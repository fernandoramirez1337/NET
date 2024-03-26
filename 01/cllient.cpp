#include <iostream>
#include <cstdio>    
#include <cstdlib>   
#include <unistd.h>  
#include <cerrno>    
#include <cstring>   
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h> 

#include <pthread.h> 

#include <cstdint>  // For exact-width integer types like intptr_t

#define PORT "3490"
#define MAXDATASIZE 100

void* get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

class client{
    std::string
};