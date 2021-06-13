#include "webpp/server.hpp"
#include <string>
#include <iostream>

int main(void) {
  WebServer server;
  server.make_static("public");
  server.get("/", [](const Request &req, Response &res) {
    res.append("Hello world");
  });
  server.get("/brownie", [](const Request &req, Response &res) {
    std::cout << "has cookie: " << req.query.has("cookie") << std::endl;
    std::cout << "cookie: " << req.query.get("cookie") << std::endl;
    res.append_file("resources/brownie.txt");
  });
  server.listen(3333);
  return 0;
}