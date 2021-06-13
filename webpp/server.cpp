#include "server.hpp"

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#include <unistd.h>

#include <iostream>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <streambuf>
#include <unordered_map>

typedef struct sockaddr SA;
typedef struct sockaddr_in SA_IN;

static int (*__listen)(int, int) = listen;

#define HTTP_200 "HTTP/1.0 200 OK"
#define HTTP_404 "HTTP/1.0 404 Not Found\r\n\r\n"

static std::vector<std::string> tokenize(const std::string &str, char delim) {
  std::size_t start;
  std::size_t end = 0;
  std::vector<std::string> out;
  while ((start = str.find_first_not_of(delim, end)) != std::string::npos) {
    end = str.find(delim, start);
    out.push_back(str.substr(start, end - start));
  }
  return out;
}

static std::string get_file(const std::string &path) {
  std::ifstream file(path);
  std::string file_str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  return file_str;
}

static std::string generate_ok_res(const std::string &content, const HeadersMap &headers) {
  std::string response = HTTP_200;
  for (const auto &it : headers) {
    response += "\n" + it.first + ": " + it.second;
  }
  response += "\r\n\r\n"; // end headers
  response += content;
  return response;
}

static void write_ok_res(const std::string &content, const HeadersMap &headers, int client) {
  std::string response = generate_ok_res(content, headers);
  const char *response_c = response.c_str();
  write(client, response_c, strlen(response_c));
  close(client);
}

static void write_404_res(int client) {
  write(client, HTTP_404, strlen(HTTP_404));
  close(client);
}

bool Query::has(const std::string &key) const {
  return query.count(key) != 0;
}

std::string Query::get(const std::string &key) const {
  if (!has(key)) return "";
  return query.at(key);
}

void Response::append(const std::string &str) {
  buffer += str;
  headers["Content-Length"] = std::to_string(buffer.size());
}

void Response::add_header(const std::string &key, const std::string &value) {
  headers[key] = value;
}

void Response::append_file(const std::string &path) {
  append(get_file(path));
}

void WebServer::on(const std::string &method, const std::string &route, const RouteCallback &callback) {
  routes[method + " " + route] = callback;
}

void WebServer::get(const std::string &route, const RouteCallback &callback) {
  return on("GET", route, callback);
}

void WebServer::post(const std::string &route, const RouteCallback &callback) {
  return on("POST", route, callback);
}

void WebServer::make_static(const std::string &path) {
  static_paths.push_back(path);
}

bool WebServer::has_path(const std::string &path) {
  return std::find(static_paths.begin(), static_paths.end(), path) != static_paths.end();
}

void WebServer::listen(const uint16_t port) {
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  int option = 1;
  setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  SA_IN address;
  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(socket_fd, (SA *)&address, sizeof(address));
  __listen(socket_fd, 1000);
  std::cout << "Listening on port " << port << "...\n";
  while (true) {
    SA client_info;
    socklen_t info_len;
    int client_fd = accept(socket_fd, (SA *)&client_info, &info_len);
    struct sockaddr_in *inaddr = (struct sockaddr_in *)&client_info;
    char *client_ip = inet_ntoa(inaddr->sin_addr);
    char buffer[1024 * 10];
    memset(buffer, 0, sizeof(buffer));
    int r = read(client_fd, buffer, sizeof(buffer) - 1);
    std::string data = buffer;
    std::vector<std::string> request = tokenize(tokenize(data, '\n')[0], ' ');
    const std::string &method = request[0];
    const std::string &path = request[1];
    const std::string &full_request = method + " " + path;
    const std::vector<std::string> &components = tokenize(full_request, '?');
    if (components.size() > 2) {
      // malformed request
      write_404_res(client_fd);
      continue;
    }
    bool has_query = components.size() == 2;
    bool static_response = false;
    const std::string &request_path = components[0];
    QueryMap req_query;
    if (has_query) {
      const std::string &query = components[1];
      const std::vector<std::string> &query_pairs = tokenize(query, '&');
      for (auto &pair : query_pairs) {
        const std::vector<std::string> pair_components = tokenize(pair, '=');
        if (pair_components.size() != 2) continue;
        req_query[pair_components[0]] = pair_components[1];
      }
    }
    Response res;
    if (method == "GET") {
      const std::string &relative_path = path.substr(1);
      const std::string &parent_path = std::filesystem::path(relative_path).parent_path();
      if (has_path(parent_path)) {
        res.append_file(relative_path);
        write_ok_res(res.buffer, res.headers, client_fd);
        static_response = true;
      }
    }
    if (!static_response) {
      if (routes.count(request_path) != 0) {
        Request req(req_query);
        routes[request_path](req, res);
        write_ok_res(res.buffer, res.headers, client_fd);
      } else {
        write_404_res(client_fd);
      }
    }
  }
}