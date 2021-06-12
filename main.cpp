#include "webpp/server.hpp"
#include <string>
#include <iostream>

int main(void) {
  WebServer server;
  server.make_static("public");
  server.get("/", []() {
    return std::string("Hello world");
  });
  server.get("/brownie", [&]() {
    return server.get_file("resources/brownie.txt");
  });
  server.listen(3333);
  return 0;
}