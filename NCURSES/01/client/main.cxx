#include "include/client.hxx"
/* <string> <cstdint> <netinet/in.h> */
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc != 3){
    std::cerr << "usage: client hostname port" << std::endl;
    exit(1);
  }

  client client_(std::string(argv[1]),std::stoi(argv[2])); 
  return 0;
}