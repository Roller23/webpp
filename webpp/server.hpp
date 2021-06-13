#if !defined(__WEBPP_SERVER_)
#define __WEBPP_SERVER_

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

class Query {
  private:
    const std::unordered_map<std::string, std::string> query;
  public:
    Query(const std::unordered_map<std::string, std::string> &_query) : query(_query) {}
    bool has(const std::string &key) const;
    std::string get(const std::string &key) const;
};

class Request {
  public:
    Query query;
    Request(const std::unordered_map<std::string, std::string> &_query) : query(_query) {}
};

typedef std::function<std::string(const Request)> RouteCallback;

class WebServer {
  private:
    std::unordered_map<std::string, RouteCallback> routes;
    std::vector<std::string> static_paths;
    int socket_fd;
    bool has_path(const std::string &path);
  public:
    void make_static(const std::string &path);
    std::string get_file(const std::string &path);
    void on(const std::string &method, const std::string &route, RouteCallback callback);
    void get(const std::string &route, RouteCallback callback);
    void post(const std::string &route, RouteCallback callback);
    void listen(const uint16_t port);
};

#endif // __WEBPP_SERVER_