#include "include/server.hxx"
/* <netinet/in.h> <cstdint> <string> <map> <mutex> */
#include <iostream>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "usage: server port" << std::endl;
        exit(1);
    }

    udp_server server(std::stoi(argv[1]));

    return 0;
}