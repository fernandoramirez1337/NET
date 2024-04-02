#include <iostream>  // Necessary for input/output operations
#include <cstdlib>   // Necessary for general utilities such as memory management and conversions
#include <unistd.h>  // Necessary for POSIX operating system API
#include <cerrno>    // Necessary for error handling
#include <cstring>   // Necessary for string manipulation functions
#include <sys/types.h>  // Necessary for various data types used in system calls
#include <sys/socket.h> // Necessary for socket-related functions and structures
#include <netinet/in.h> // Necessary for internet address structures and constants
#include <netdb.h>      // Necessary for network database operations
#include <arpa/inet.h>  // Necessary for manipulating IP addresses
#include <sys/wait.h>   // Necessary for waiting for process status changes
#include <signal.h>     // Necessary for signal handling

#include <thread>      // Necessary for multithreading support

#include <vector>      // Necessary for dynamic arrays
#include <cstdint>     // Necessary for fixed-size integer types
#include <string>      // Necessary for string operations
#include <map>         // Necessary for associative array (map) data structure

#define BACKLOG 10 
#define MAXDATASIZE 100000

void sigchld_handler(int s){
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

class SERVER{
    public:
        SERVER(char *); //OPEN LISTEN - socket bind listen
        ~SERVER();

    private:
        std::map<std::string, int> CLIENTS;
        int sockFD;
        
        int accept_();
        void read_write_(void *);
        //void close_();

        void start_();
        void session_(int);
};

SERVER::SERVER(char * port){
    addrinfo hints, *servinfo, *p;
    struct sigaction sa;
    int yes = 1, rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0){
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
        exit(1);
    }
    for (p = servinfo; p != NULL; p = p->ai_next){
        if ((sockFD = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) ==  -1){
            perror("server: socket");
            continue;
        }
        if (setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
            perror("setsockopt");
            exit(1);
        }
        if (bind(sockFD, p->ai_addr, p->ai_addrlen) == -1){
            close(sockFD);
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
    if (listen(sockFD, BACKLOG) == -1){
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
    start_();
}

SERVER::~SERVER(){
    shutdown(sockFD, SHUT_RDWR);
}

void SERVER::start_(){
    while(1){
        int accepted_connection = accept_();
        session_(accepted_connection);
    }
}

int SERVER::accept_(){
    sockaddr_storage client_addr;
    socklen_t client_addr_size = sizeof client_addr;
    int client_sockFD = accept(sockFD, (sockaddr *)&client_addr, &client_addr_size);
    if (client_sockFD == -1){
        perror("accept");
    }
    return client_sockFD;
}

void SERVER::session_(int _sockFD){
    std::thread worker_thread([this, _sockFD](){read_write_(reinterpret_cast<void *>(_sockFD));});
    worker_thread.detach();
}

void SERVER::read_write_(void * void_sockFD){
    int _sockFD = (intptr_t)void_sockFD;
    char buffer[MAXDATASIZE];
    std::string f_buffer, console_message;
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

        if (message_type == 'N') {  // new user
            std::string username = buffer;
            CLIENTS.insert({username, _sockFD});
            console_message = username + " joined the lobby";
            for(auto it = CLIENTS.begin(); it != CLIENTS.end(); it++){
                if (send(it->second, console_message.c_str(), console_message.size(), 0) == -1)
                    perror("send"); 
            }
            std::cout << console_message << std::endl;
            

        } else if (message_type == 'W') {   // message for all
            f_buffer = buffer;
            std::string dots = ":", sender, message;
            int delimiter = f_buffer.find(dots);
            sender = f_buffer.substr(0, delimiter);
            message = f_buffer.substr(delimiter+1, f_buffer.size());
            f_buffer = sender + ": " + message;
            for(auto it = CLIENTS.begin(); it != CLIENTS.end(); it++){
                if (it->first != sender){
                    if (send(it->second, f_buffer.c_str(), f_buffer.size(), 0) == -1)
                        perror("send"); 
                }
            }
            std::cout << f_buffer << std::endl;
        } else if( message_type == 'M') {   // message for user
            
            std::string dots = ":", sender, receiver, message;
            receiver = buffer;
            if ((numbytes = recv(_sockFD, buffer, 2, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            int buffer_size = atoi(buffer);
            if ((numbytes = recv(_sockFD, buffer, buffer_size, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            f_buffer = buffer;
            int delimiter = f_buffer.find(dots);
            sender = f_buffer.substr(0, delimiter);
            message = f_buffer.substr(delimiter + 1, f_buffer.size());
            f_buffer = "priv from " + sender + ": " + message;
            if (send(CLIENTS[receiver], f_buffer.c_str(), f_buffer.size(),0) == -1)
                perror("send");
            std::cout << "priv from " + sender + " to " + receiver + ": " + message << std::endl;
        } else if ( message_type == 'Q') {  // exit

        }
        f_buffer.clear();
    }
}

int main(int argc, char *argv[]){

    if (argc != 2){
        std::cerr << "usage: server port" << std::endl;
        exit(1);
    }
    
    SERVER server(argv[1]);

    return 0;
}