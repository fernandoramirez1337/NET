#ifndef CLIENT_SERVER
#define CLIENT_SERVER

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

#define MAXDATASIZE 100000



std::string normalize(const std::string& input, size_t width) {
    std::string result;
    size_t zeros = width - input.length();
    
    if (zeros > 0) {
        result.append(zeros, '0');
    }
    result.append(input);
    return result;
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

#endif
