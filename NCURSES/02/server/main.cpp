#include "include/server.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "usage: server port" << std::endl;
    exit(1);
  }

  server server_(std::stoi(argv[1]));

  return 0;
}