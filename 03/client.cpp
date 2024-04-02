#include <iostream>  // Needed for input/output operations
#include <cstdlib>   // Needed for general utilities such as exit()
#include <cerrno>    // Needed for error numbers
#include <cstring>   // Needed for string manipulation functions like memset()
#include <cstdint>   // Needed for fixed-size integer types

#include <vector>    // Needed for vector containers
#include <string>    // Needed for string manipulation

#include <unistd.h>       // Needed for POSIX operating system API (e.g., close())
#include <netdb.h>        // Needed for networking functions and structures
#include <sys/types.h>    // Needed for data types used in system calls
#include <netinet/in.h>   // Needed for Internet address family structures
#include <sys/socket.h>   // Needed for socket-related functions and structures
#include <arpa/inet.h>    // Needed for IP address conversion functions
#include <thread>         // Needed for multithreading support

#define MAXDATASIZE 100000

//get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

class CLIENT{
    public:
    CLIENT(char *, char *, char *);
    ~CLIENT();

    private:
    std::string username;
    int sockFD;
    
    void write_();
    void read_();
    //void close_();

    void start_();
    void session_();
};

CLIENT::CLIENT(char * hostname, char * port, char * user) : username(user) {
    addrinfo hints, *servinfo, *p;
    int rv;
    char server[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0){
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
        exit(1);
    }

    for(p = servinfo; p != NULL; p = p->ai_next){
        if ((sockFD = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("client: socket");
            continue;
        }
        if (connect(sockFD, p->ai_addr, p->ai_addrlen) == -1){
            close(sockFD);
            perror("client: connect");
            continue;
        }
        break;
    }
    if (p == NULL){
        std::cerr << "client: failed to connect" << std::endl;
        exit(2);
    }
    inet_ntop(p->ai_family, get_in_addr((sockaddr *)p->ai_addr), server, sizeof server);
    std::cout << "client: connecting to " << server << std::endl;
    freeaddrinfo(servinfo);

    int username_size = username.size();
    std::string buffer = std::to_string(username_size);
    if (username_size < 10)
        buffer = "0" + buffer;
    buffer = "N" + buffer + username;

    if (send(sockFD, buffer.c_str(), buffer.size(), 0) == -1)
        perror("send");

    start_();
}

CLIENT::~CLIENT(){
    close(sockFD);
}

void CLIENT::write_(){
    std::cout << std::endl;
    std::string buffer, f_buffer, receiver, receiver_size, message, message_size, space = " ";
    
    while(1){
        std::getline(std::cin, buffer);

        f_buffer.clear();

        if (buffer[0] == '.'){  // private
            int delimiter = buffer.find(space); 
            receiver = buffer.substr(1, delimiter-1);
            receiver_size = std::to_string(receiver.size());
            if (receiver.size() < 10)
                receiver_size = "0" + receiver_size;
            message = username +":"+ buffer.substr(delimiter+1, buffer.size());
            message_size = std::to_string(message.size());
            if (message.size() < 10)
                message_size = "0" + message_size;
            f_buffer = "M" + receiver_size + receiver + message_size + message;
        } else {    // public
            message = username +":"+ buffer;
            message_size = std::to_string(message.size());
            if (message.size() < 10)
                message_size = "0" + message_size;
            f_buffer = "W" + message_size + message;
        }
        if (send(sockFD, f_buffer.c_str(), f_buffer.size(), 0) == -1)
            perror("send");
    }
}

void CLIENT::read_(){
    char buf[MAXDATASIZE];

    while (1)
    {
        ssize_t numbytes;
        if ((numbytes = recv(sockFD, buf, MAXDATASIZE-1, 0)) == -1){
            perror("recv");
            exit(1);
        }

        if(numbytes>0)
        {
            buf[numbytes] = 0;
            std::cout << buf << std::endl;
        }

        if(numbytes==0)
            break;
    }

    close(sockFD);
}

void CLIENT::start_(){
    session_();
}

void CLIENT::session_(){
    std::thread worker_thread([this](){read_();});
    worker_thread.detach();
    write_();
}

int main(int argc, char *argv[]){

    if (argc != 4){
        std::cerr << "usage: client hostname port username" << std::endl;
        exit(1);
    }

    CLIENT client(argv[1],argv[2],argv[3]); 

    return 0;
}