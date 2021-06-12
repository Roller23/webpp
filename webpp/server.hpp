#if !defined(__WEBPP_SERVER_)
#define __WEBPP_SERVER_

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

typedef std::function<std::string()> RouteCallback;

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