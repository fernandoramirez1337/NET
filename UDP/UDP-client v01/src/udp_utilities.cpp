#include "../include/udp_utilities.hpp"

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

    void sigchld_handler(int s){
        int saved_errno = errno;
        while (waitpid(-1, NULL, WNOHANG) > 0);
        errno = saved_errno;
    }

    std::string normalize(const std::string& input, size_t width, char c = '0') {
        if (width <= input.length()) {
            return input;
        }
        return std::string(width - input.length(), c) + input;
    }

    std::string normalize(const std::string& input, char c = ' ') {
        size_t firstNonSpace = input.find_first_not_of(' ');

        if (firstNonSpace == std::string::npos) {
            return "";
        }
        return input.substr(firstNonSpace);
    }

    void *get_in_addr(struct sockaddr *sa){
        if (sa->sa_family == AF_INET) {
            return &(((struct sockaddr_in*)sa)->sin_addr);
        }
        return &(((struct sockaddr_in6*)sa)->sin6_addr);
    }

    udp_datagram::udp_datagram(const std::string& input){
        if (input.length() != MAXDATASIZE){
            return;
        }
        id.first = input.substr(0,2);
        id.second = std::stoi(input.substr(2,2));
        username_size = std::stoi(input.substr(4,2));
        username = normalize(input.substr(6,8));
        payload_size = std::stoi(input.substr(14,4));
        payload = normalize(input.substr(18,888));
        free = input.substr(906,94);
        checksum = std::stoi(input.substr(1000,10));
        timestamp = input.substr(1010,14);
    }

    int checksum(const std::string& str) {
        int checksum = 0;

        for (char c : str) {
            checksum += static_cast<int>(c);
        }
        return checksum;
    }
}