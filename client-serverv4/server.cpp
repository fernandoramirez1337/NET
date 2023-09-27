#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include <thread>

#include <vector>
#include <cstdint>
#include <string>
#include <map>

#define BACKLOG 10 

#define MAXDATASIZE 100

std::map<std::string, int> aS;

//get sockaddr, IPv4 or IPv6:
void *get_in_adrr(sockaddr *sa){
    if (sa->sa_family == AF_INET){
        return &(((sockaddr_in*)sa)->sin_addr);
    }
    return &(((sockaddr_in6*)sa)->sin6_addr);
}

void sigchld_handler(int s){
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

class MyServer{
    public:
        MyServer(char *);
        ~MyServer();
        void start();
        
    private: 
        std::map<std::string, int> accepted_sockets;
        int server_sockFD;
        int accept_connection();
        void start_new_thread(int);
        void receive_data(void *);
};

MyServer::MyServer(char * port){
    addrinfo hints, *servinfo, *p;
    struct sigaction sa;
    int yes = 1;
    int rv;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0){
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
        exit(1);
    }
    for(p = servinfo; p != NULL; p = p->ai_next){
        if ((server_sockFD = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("server: socket");
            continue;
        }
        if (setsockopt(server_sockFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
            perror("setsockopt");
            exit(1);
        }
        if (bind(server_sockFD, p->ai_addr, p->ai_addrlen) == -1){
            close(server_sockFD);
            perror("server: bind");
            continue;
        }
        break;
    }
    freeaddrinfo(servinfo);
    if (p == NULL){
        std::cerr << "server: failed to bind" << std::endl;
        exit(1);
    }
    if (listen(server_sockFD, BACKLOG) == -1){
        perror("listen");
        exit(1);
    }
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1){
        perror("sigaction");
        exit(1);
    }
    std::cout << "server: waiting for connections...        PORT = " << port << std::endl; 
}

MyServer::~MyServer(){
    shutdown(server_sockFD, SHUT_RDWR);
}

void MyServer::start(){
    while (1){
        int accepted_sockFD = accept_connection();
        start_new_thread(accepted_sockFD);
    }
}

int MyServer::accept_connection(){
    sockaddr_storage client_addr;
    socklen_t client_addr_size = sizeof client_addr;
    int client_sockFD = accept(server_sockFD, (sockaddr *)&client_addr, &client_addr_size);
    if (client_sockFD == -1){
        perror("accept");
    }
    //accepted_sockets.insert({"default",client_sockFD});
    return client_sockFD;
}

void MyServer::start_new_thread(int _sockFD){
    std::thread worker_thread([this,_sockFD](){receive_data((void *)_sockFD);});
    worker_thread.detach();
}

void MyServer::receive_data(void * void_sockFD){
    int _sockFD = (intptr_t)void_sockFD;
    char buffer[MAXDATASIZE];
    std::string formatted_buffer, console_message;
    while(1){
        ssize_t numbytes;
        if ((numbytes = recv(_sockFD, buffer, 3, 0)) == -1){
            perror("recv");
            exit(1);
        }
        char message_type = buffer[0]; buffer[0] = '0'; buffer[numbytes] = 0;
        int message_size = atoi(buffer);
        if ((numbytes = recv(_sockFD, buffer, message_size, 0)) == -1){
            perror("recv");
            exit(1);
        }
        buffer[numbytes] = 0;

        if (message_type == 'N') { //new user
            std::string username = buffer;
            accepted_sockets.insert({username, _sockFD});
            console_message = username + " joined the lobby";
            for(auto it = accepted_sockets.begin(); it != accepted_sockets.end(); it++){
                if (send(it->second, console_message.c_str(), console_message.size(), 0) == -1)
                    perror("send"); 
            }
            std::cout << console_message << std::endl;
            

        } else if (message_type == 'W') { //message for all
            formatted_buffer = buffer;
            std::string dots = ":", sender, message;
            int delimiter = formatted_buffer.find(dots);
            sender = formatted_buffer.substr(0, delimiter);
            message = formatted_buffer.substr(delimiter+1, formatted_buffer.size());
            formatted_buffer = sender + ": " + message;
            for(auto it = accepted_sockets.begin(); it != accepted_sockets.end(); it++){
                if (it->first != sender){
                    if (send(it->second, formatted_buffer.c_str(), formatted_buffer.size(), 0) == -1)
                        perror("send"); 
                }
            }
            std::cout << formatted_buffer << std::endl;
        } else if( message_type == 'M') { //message for user
            
            std::string dots = ":", sender, receiver, message;
            receiver = buffer;
            if ((numbytes = recv(_sockFD, buffer, 2, 0)) == -1){
                perror("recv");
                exit(1);
            }
            int buffer_size = atoi(buffer);
            if ((numbytes = recv(_sockFD, buffer, buffer_size, 0)) == -1){
                perror("recv");
                exit(1);
            }
            formatted_buffer = buffer;
            int delimiter = formatted_buffer.find(dots);
            sender = formatted_buffer.substr(0, delimiter);
            message = formatted_buffer.substr(delimiter + 1, formatted_buffer.size());
            formatted_buffer = "priv from " + sender + ": " + message;
            if (send(accepted_sockets[receiver], formatted_buffer.c_str(), formatted_buffer.size(),0) == -1)
                perror("send");
            std::cout << "priv from " + sender + " to " + receiver + ": " + message << std::endl;
        } else if ( message_type == 'Q') { //exit

        }
    }

}


int main(int argc, char *argv[]){

    if (argc != 2){
        std::cerr << "usage: server port" << std::endl;
        exit(1);
    }
    MyServer server(argv[1]);
    server.start();
    //start_accepting_connections(sockFD);

    return 0;
}