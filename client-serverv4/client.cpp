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
#include <thread>



//#define PORT "3490"

#define MAXDATASIZE 100

//get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

class MyClient{
    private:
        std::string username;
        int client_sockFD;
        void start_new_thread();
        void * receive_data();
        void send_data();
    public:
        void start();
        MyClient(char *, char *, char *);
        ~MyClient();
};

MyClient::MyClient(char * hostname, char * port, char * user){
    username = user;
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
        if ((client_sockFD = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("client: socket");
            continue;
        }
        if (connect(client_sockFD, p->ai_addr, p->ai_addrlen) == -1){
            close(client_sockFD);
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
}

MyClient::~MyClient(){
    close(client_sockFD);
}

void MyClient::start_new_thread(){
    std::thread worker_thread([this](){receive_data();});
    worker_thread.detach();
}

void * MyClient::receive_data(){
    char buf[MAXDATASIZE];

    while (1)
    {
        ssize_t numbytes;
        if ((numbytes = recv(client_sockFD, buf, MAXDATASIZE-1, 0)) == -1){
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

    close(client_sockFD);
}

void MyClient::send_data(){
    std::cout << std::endl;
    std::string buffer, formatted_buffer, receiver, receiver_size, message, message_size, space = " ";

    int username_size = username.size();
    buffer = std::to_string(username_size);
    if (username_size < 10)
        buffer = "0" + buffer;
    buffer = "N" + buffer + username;

    if (send(client_sockFD, buffer.c_str(), buffer.size(), 0) == -1)
        perror("send");
    
    while(1){
        std::getline(std::cin, buffer);

        formatted_buffer.clear();

        
        if (buffer[0] == '.'){ //private
            int delimiter = buffer.find(space); 
            receiver = buffer.substr(1, delimiter-1);
            receiver_size = std::to_string(receiver.size());
            if (receiver.size() < 10)
                receiver_size = "0" + receiver_size;
            message = username +":"+ buffer.substr(delimiter+1, buffer.size());
            message_size = std::to_string(message.size());
            if (message.size() < 10)
                message_size = "0" + message_size;
            formatted_buffer = "M" + receiver_size + receiver + message_size + message;
        } else { //public
            message = username +":"+ buffer;
            message_size = std::to_string(message.size());
            if (message.size() < 10)
                message_size = "0" + message_size;
            formatted_buffer = "W" + message_size + message;
        }
        if (send(client_sockFD, formatted_buffer.c_str(), formatted_buffer.size(), 0) == -1)
            perror("send");
    }

}

void MyClient::start(){
    start_new_thread();
    send_data();
}

int main(int argc, char *argv[]){

    if (argc != 4){
        std::cerr << "usage: client hostname port username" << std::endl;
        exit(1);
    }

    MyClient client(argv[1],argv[2],argv[3]);
    client.start();

    return 0;
}
