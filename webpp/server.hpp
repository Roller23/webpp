#if !defined(__WEBPP_SERVER_)
#define __WEBPP_SERVER_

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

typedef std::unordered_map<std::string, std::string> HeadersMap;
typedef std::unordered_map<std::string, std::string> QueryMap;

class WebServer;

class Query {
  private:
    const QueryMap query;
  public:
    Query(const QueryMap &_query) : query(_query) {}
    bool has(const std::string &key) const;
    std::string get(const std::string &key) const;
};

class Request {
  public:
    Query query;
    Request(const QueryMap &_query) : query(_query) {}
};

class Response {
  friend class WebServer;
  private:
    std::string buffer = "";
    HeadersMap headers;
  public:
    void append(const std::string &str);
    void add_header(const std::string &key, const std::string &value);
    void append_file(const std::string &path);
    Response() {
      headers["Content-Type"] = "text/html; charset=utf-8";
      headers["Content-Length"] = "0";
    }
};

typedef std::function<void(const Request &, Response &)> RouteCallback;

class WebServer {
  private:
    std::unordered_map<std::string, RouteCallback> routes;
    std::vector<std::string> static_paths;
    int socket_fd;
    bool has_path(const std::string &path);
  public:
    void make_static(const std::string &path);
    void on(const std::string &method, const std::string &route, const RouteCallback &callback);
    void get(const std::string &route, const RouteCallback &callback);
    void post(const std::string &route, const RouteCallback &callback);
    void listen(const uint16_t port);
};

#endif // __WEBPP_SERVER_