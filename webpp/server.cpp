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

typedef struct sockaddr SA;
typedef struct sockaddr_in SA_IN;

static int (*__listen)(int, int) = listen;

#define HTTP_200 "HTTP/1.0 200 OK"
#define HTTP_404 "HTTP/1.0 404 Not Found\r\n\r\n"

static std::vector<std::string> tokenize(std::string &str, char delim) {
  std::size_t start;
  std::size_t end = 0;
  std::vector<std::string> out;
  while ((start = str.find_first_not_of(delim, end)) != std::string::npos) {
    end = str.find(delim, start);
    out.push_back(str.substr(start, end - start));
  }
  return out;
}

static std::string generate_ok_res(const std::string &content) {
  std::string response = HTTP_200;
  response += "\nContent-Type: text/html";
  response += "\nContent-Length: " + std::to_string(content.size());
  response += "\r\n\r\n";
  response += content;
  return response;
}

static void write_ok_res(const std::string &content, int client) {
  std::string response = generate_ok_res(content);
  const char *response_c = response.c_str();
  write(client, response_c, strlen(response_c));
  close(client);
}

static void write_404_res(int client) {
  write(client, HTTP_404, strlen(HTTP_404));
  close(client);
}

void WebServer::on(const std::string &method, const std::string &route, RouteCallback callback) {
  routes[method + " " + route] = callback;
}

void WebServer::get(const std::string &route, RouteCallback callback) {
  return on("GET", route, callback);
}

void WebServer::post(const std::string &route, RouteCallback callback) {
  return on("POST", route, callback);
}

void WebServer::make_static(const std::string &path) {
  static_paths.push_back(path);
}

std::string WebServer::get_file(const std::string &path) {
  std::ifstream file(path);
  std::string file_str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  return file_str;
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
    bool static_response = false;
    if (method == "GET") {
      const std::string &relative_path = path.substr(1);
      const std::string &parent_path = std::filesystem::path(relative_path).parent_path();
      if (has_path(parent_path)) {
        write_ok_res(get_file(relative_path), client_fd);
        static_response = true;
      }
    }
    if (!static_response) {
      if (routes.count(full_request) != 0) {
        write_ok_res(routes[full_request](), client_fd);
      } else {
        write_404_res(client_fd);
      }
    }
  }
}