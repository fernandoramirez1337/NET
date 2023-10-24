#include <iostream>
#include "../include/udp_client.hpp"

int main(int argc, char * argv[]){
    if (argc != 3){
        std::cerr << "usage: client port username" <<std::endl;
        exit(1);
    }
    udp_client client(argv[1], argv[2]);
    client.start();
    return 0;
}