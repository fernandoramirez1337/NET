#include <iostream>

#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <cstdint>

#include <vector>
#include <string>
//#include <format>

#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>



//#define PORT "3490"

#define MAXDATASIZE 100
#define MAXUSERNAMESIZE 5

//get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

//receives a socket and creates a thread
void start_new_thread(int _sockFD);
//thread function, connects with server and receives data
void * receive_data(void * _sockFD);
// receives socket for sending data and username
void send_data(int _sockFD, std::string user);

void start_new_thread(int _sockFD) {

    pthread_t id ;
    pthread_create(&id,NULL,receive_data,(void *)_sockFD);
}

void * receive_data(void * void_sockFD) {
    int _sockFD = (intptr_t)void_sockFD;
    char buf[MAXDATASIZE];

    while (1)
    {
        ssize_t numbytes;
        if ((numbytes = recv(_sockFD, buf, MAXDATASIZE-1, 0)) == -1){
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

    close(_sockFD);
}

void send_data(int _sockFD, std::string user) {

    std::cout << std::endl;

    std::string buffer, formatted_buffer, console_message;

    console_message = user + " joined the room";
    int console_message_size = console_message.size();

    if (send(_sockFD, console_message.c_str(), console_message_size, 0) == -1)
        perror("send");

    while(buffer != "end")
    {
        std::getline(std::cin, buffer);
        
        formatted_buffer.clear();
        formatted_buffer = user + ": " + buffer;
        
        int buffer_size = buffer.size();

        if (buffer_size > 0)
        {
            if (send(_sockFD, formatted_buffer.c_str(), MAXDATASIZE-1, 0) == -1)
                    perror("send");
        }
    }

    console_message = user + " ended its connection.";
    console_message_size = console_message.size();
    if (send(_sockFD, console_message.c_str(), console_message_size, 0) == -1)
        perror("send");
}

int main(int argc, char *argv[]){
    int sockFD;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char server[INET6_ADDRSTRLEN];

    std::string username(argv[3]);

    if (argc != 4){
        std::cerr << "usage: client hostname port username" << std::endl;
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0){
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
        return 1;
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
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), server, sizeof server);
    std::cout << "client: connecting to " << server << std::endl;

    freeaddrinfo(servinfo);

    start_new_thread(sockFD);

    send_data(sockFD,username);

    std::cout << "Connection ended successfully" << std::endl;
    close(sockFD);

    return 0;
}