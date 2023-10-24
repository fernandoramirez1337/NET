#ifndef UDP_UTILITIES_HPP
#define UDP_UTILITIES_HPP

#include <string>
#include <cstdint>
#include <utility>

#define MAXDATASIZE 1024

namespace udp_util{
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