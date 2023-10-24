#ifndef UDP_UTILITIES_HPP
#define UDP_UTILITIES_HPP

#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <cstdint>

#include <vector>
#include <string>

#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>

#include <fstream>

#include <sys/wait.h>

#define MAXDATASIZE 1024

namespace udp_util{
    void sigchld_handler(int);

    void *get_in_addr(struct sockaddr *);

    int checksum(const std::string&);

    std::string normalize(const std::string&, size_t, char);

    std::string normalize(const std::string&, char);

    struct udp_datagram{
        std::pair<std::string,std::uint8_t> id;
        std::string username, payload, free, timestamp;
        size_t username_size, payload_size, checksum;
        udp_datagram(const std::string&);
    };
    
}

#endif