#include "include/client.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
    if (argc != 3){
        std::cerr << "usage: client hostname port" << std::endl;
        exit(1);
    }

    CLIENT client(argv[1],argv[2]); 

    return 0;
}
