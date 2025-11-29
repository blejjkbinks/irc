#include "Server.hpp"
#include <cstdlib>
#include <iostream>

int main(int argc, char **argv) {
  int port;
  std::string password;
  if (argc != 3) {
    port = 6667;
    password = "password";
  } else {
    port = std::atoi(argv[1]);
    password = argv[2];
  }

  Server s(port, password);
  s.start();

  return (0);
}