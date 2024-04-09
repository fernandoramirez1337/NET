#include "../include/server.hpp"
#include "../include/net_utilities.hpp"
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
#include <stdexcept> // For std::runtime_error
#include <sstream>

#define BACKLOG 10
#define MAXDATASIZE 100000
#define ERR_LOGIN_DUPLICATES (int)01
#define ERR_LOGIN (int)02
#define ERR_PRIV (int)11

void sigchld_handler(int s);
std::string generate_error_message(int err_n);
std::string generate_ok_message();
std::string generate_welcome_message(std::string username);
std::string generate_goodbye_message(std::string username);
std::string generate_list_message(const std::map<std::string, int>& myMap);
std::string generate_broadcast_message(std::string msg, std::string sender);
std::string generate_private_message(std::string msg, std::string sender);

void sigchld_handler(int s) {
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

// E
std::string generate_error_message(int err_n) {
    if (err_n < 0 || err_n > 99) {
        throw std::invalid_argument("Number must be in the range 0-99");
    }
    std::ostringstream oss;
    oss << "E" << std::setw(2) << std::setfill('0') << err_n << "\n";
    return oss.str();
}
// O
std::string generate_ok_message(){
    std::ostringstream oss;
    oss << "O\n";
    return oss.str();
}
// l##USERNAME
std::string generate_welcome_message(std::string username){
    std::ostringstream oss;
    oss << "l" << net::format_size(username.size(), 2) << username << "\n";
    return oss.str();
}
// u##USERNAME
std::string generate_goodbye_message(std::string username){
    std::ostringstream oss;
    oss << "u" << net::format_size(username.size(), 2) << username << "\n";
    return oss.str();
}
// t## ###LIST
std::string generate_list_message(const std::map<std::string, int>& myMap){
    std::ostringstream oss;
    auto it = myMap.begin();
    if (it != myMap.end()) {
        oss << it->first;
        ++it;
    }
    for (; it != myMap.end(); ++it) {
        oss << "," << it->first;
    }
    std::string map_size = net::format_size(myMap.size(), 2);
    std::string list_size = net::format_size(oss.str().size(), 3);
    std::string list = oss.str();
    oss.str("");
    oss << "t" << map_size << list_size << list << "\n";
    return oss.str();
}
// b##SENDER##MSG
std::string generate_broadcast_message(std::string msg, std::string sender){
    std::ostringstream oss;
    oss << "b" << net::format_size(sender.size(), 2) << sender << net::format_size(msg.size(), 2) << msg << '\n';
    return oss.str();
}
// m##SENDER##MSG
std::string generate_private_message(std::string msg, std::string sender){
    std::ostringstream oss;
    oss << "m" << net::format_size(sender.size(), 2) << sender << net::format_size(msg.size(), 2) << msg << '\n';
    return oss.str();
}

SERVER::SERVER(char *port) {
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

SERVER::~SERVER() {
    shutdown(sockFD, SHUT_RDWR);
}

void SERVER::start_() {
    while(1){
        int accepted_connection = accept_();
        session_(accepted_connection);
    }
}

int SERVER::accept_() {
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

void SERVER::read_write_(void *void_sockFD) {
    int _sockFD = (intptr_t)void_sockFD;
    char buffer[MAXDATASIZE];
    std::string this_username;
    while(1){
        ssize_t numbytes = ssize_t(0);
        if ((numbytes = recv(_sockFD, buffer, 1, 0)) == -1){
            perror("recv");
            exit(1);
        }
        char message_type = buffer[0]; 

        if (message_type == 'L') {  // login code
            std::string username = recv_string(_sockFD, 2);
            std::string password = recv_string(_sockFD, 2);
            std::cout << "-> L " << username << " " << password << std::endl;
            // check for duplicates
            bool is_duplicate = false;
            for(auto it = CLIENTS.begin(); it != CLIENTS.end(); it++){
                if (it->first == username){
                    std::string error_message = generate_error_message(ERR_LOGIN_DUPLICATES);
                    if (send(_sockFD, error_message.c_str(), error_message.size(), 0) == -1)
                        perror("send"); 
                    is_duplicate = true;
                    std::cout << "<- " << error_message;
                    break;
                }
            }
            // add client to list if not a duplicate
            if (!is_duplicate) {
                this_username = username;
                CLIENTS.insert({username, _sockFD});
                std::string ok_message = generate_ok_message();
                if (send(_sockFD, ok_message.c_str(), ok_message.size(), 0) == -1)
                    perror("send"); 
                std::cout << "<- " << ok_message;

                std::string welcome_message = generate_welcome_message(username);
                for(auto it = CLIENTS.begin(); it != CLIENTS.end(); it++){
                    if (send(it->second, welcome_message.c_str(), welcome_message.size(), 0) == -1)
                        perror("send"); 
                    std::cout << "<- " << welcome_message;
                }
            }
        } else if (message_type == 'O') {   // ok code
            std::cout << " -> O " << std::endl;
        } else if (message_type == 'E') {   // error code
            std::cout << " -> E " << std::endl;
        } else if (message_type == 'U') {   // logout code
            std::cout << "-> U "<< std::endl;
            // iterate over the map to find the key associated with the value to delete
            auto it = CLIENTS.begin();
            for (; it != CLIENTS.end(); ++it) {
                if (it->second == _sockFD) {
                    break;
                }
            }
            // check if the value was found
            if (it != CLIENTS.end()) {
                CLIENTS.erase(it);
                std::string goodbye_message = generate_goodbye_message(this_username);
                for(auto it_ = CLIENTS.begin(); it_ != CLIENTS.end(); it_++){
                    if (send(it_->second, goodbye_message.c_str(), goodbye_message.size(), 0) == -1)
                        perror("send"); 
                    std::cout << "<- " << goodbye_message ;

                }
                /*
                std::string ok_message = generate_ok_message();
                if (send(_sockFD, ok_message.c_str(), ok_message.size(), 0) == -1)
                    perror("send"); 
                    std::cout << "<- " << ok_message;
                CLIENTS.erase(it);
                */
            } else {
                //error logout
            }
        } else if (message_type == 'T') {
            std::string list_message = generate_list_message(CLIENTS);
            if (send(_sockFD, list_message.c_str(), list_message.size(), 0) == -1)
                perror("send"); 
            std::cout << "<- " << list_message;

        } else if (message_type == 'B') {
            std::string broadcast = recv_string(_sockFD, 2);
            std::string broadcast_message = generate_broadcast_message(broadcast, this_username);
            std::cout << "-> B " << broadcast << this_username << std::endl;
            for(auto it = CLIENTS.begin(); it != CLIENTS.end(); it++){
                if (it->first != this_username){
                    if (send(it->second, broadcast_message.c_str(), broadcast_message.size(), 0) == -1)
                        perror("send"); 
                    std::cout << "<- " << broadcast_message;

                }
            }

        } else if (message_type == 'M') {
            std::string receiver = recv_string(_sockFD, 2);
            std::string message = recv_string(_sockFD, 2);
            std::string private_message = generate_private_message(message, this_username);
            std::cout << "-> M " << receiver << " " << message << " " << this_username << std::endl;
            auto receiver_iter = CLIENTS.find(receiver);
            if (receiver_iter != CLIENTS.end()) { // Receiver exists
                if (send(receiver_iter->second, private_message.c_str(), private_message.size(), 0) == -1)
                    perror("send");
                std::cout << "<- " << private_message;
            } else { // Receiver does not exist
                std::string error_message = generate_error_message(ERR_PRIV);
                std::cout << "<- " << error_message;
                if (send(_sockFD, error_message.c_str(), error_message.size(), 0) == -1)
                        perror("send"); 
                // Handle the case where the receiver does not exist
            }
        } else if (message_type == 'F') {
            // file code
        } else {
            // default code
        }
    }
}

std::string SERVER::recv_string(int _sockFD, int size_size) {
    char buffer[MAXDATASIZE];

    ssize_t numbytes = recv(_sockFD, buffer, size_size, 0);
    if (numbytes == -1) {
        perror("recv");
        throw std::runtime_error("Failed to receive data size");
    }
    buffer[numbytes] = '\0';
    int data_size = atoi(buffer);

    if (data_size >= MAXDATASIZE) {
        throw std::runtime_error("Received data size exceeds buffer size");
    }

    numbytes = recv(_sockFD, buffer, data_size, 0);
    if (numbytes == -1) {
        perror("recv");
        throw std::runtime_error("Failed to receive data");
    }
    buffer[numbytes] = '\0';

    return std::string(buffer);
}
