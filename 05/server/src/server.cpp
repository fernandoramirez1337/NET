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
#include <sstream>

#define BACKLOG 10
#define ERR_LOGIN_DUPLICATES (int)01
#define ERR_LOGIN (int)02
#define ERR_PRIV (int)11
#define MSG_LENGTH (int)2

void sigchld_handler(int s) {
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
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
    std::string this_username;
    char type_buffer[1];
    
    while (true) {
        type_buffer[0] = '\n';
        if (recv(_sockFD, type_buffer, 1, 0) == -1) {
            perror("recv");
        }
        char message_type = type_buffer[0];

        switch (message_type) {
            case LOGIN:
                handleLogin(_sockFD, this_username);
                break;
            case OK:
                handleOk();
                break;
            case ERROR:
                handleError();
                break;
            case LOGOUT:
                handleLogout(this_username);
                break;
            case LIST:
                handleList(_sockFD);
                break;
            case BROADCAST:
                handleBroadcast(_sockFD, this_username);
                break;
            case PRIVATE_MESSAGE:
                handlePrivateMessage(_sockFD, this_username);
                break;
            case FILE_TRANSFER:
                handleFileTransfer(_sockFD, this_username);
                break;
            default:
                // Handle unknown message type
                break;
        }
    }
}

std::string SERVER::recv_string(int _sockFD, int size_size) {
    std::unique_ptr<char[]> size_buffer(new char[size_size]);
    if (recv(_sockFD, size_buffer.get(), size_size, 0) == -1) {
        perror("recv");
    }
    int data_size = atoi(size_buffer.get());

    std::unique_ptr<char[]> buffer(new char[data_size]);
    ssize_t numbytes = recv(_sockFD, buffer.get(), data_size, 0);
    if (numbytes == -1) {
        perror("recv");
    }
    return std::string(buffer.get(), numbytes);
}

void SERVER::send_message(int _sockFD, const std::string& message) {
        if (send(_sockFD, message.c_str(), message.size(), 0) == -1)
            perror("send");
        else
            std::cout << "<- " << message << std::endl;
    }

void SERVER::handleLogin(int _sockFD, std::string& this_username) {
    std::string username = recv_string(_sockFD, MSG_LENGTH);
    std::string password = recv_string(_sockFD, MSG_LENGTH);
    std::cout << "-> L" << net::format_size(username.size(), 2) << username << net::format_size(password.size(), 2) << password << std::endl;

    bool is_duplicate = CLIENTS.find(username) != CLIENTS.end();

    if (is_duplicate) {
        std::string error_message = protocol.ErrorMessage(ERR_LOGIN_DUPLICATES);
        send_message(_sockFD, error_message);
    } else {
        this_username = username;
        CLIENTS.emplace(username, _sockFD);
        std::string ok_message = protocol.OkMessage();
        send_message(_sockFD, ok_message);

        std::string welcome_message = protocol.WelcomeMessage(username);
        for (const auto& client : CLIENTS) {
            if (client.first != username) {
                send_message(client.second, welcome_message);
            }
        }
    }
}

void SERVER::handleOk() {
    std::cout << "-> O " << std::endl;
}

void SERVER::handleError() {
    std::cout << "-> E " << std::endl;
}

void SERVER::handleLogout(std::string& this_username) {
    std::cout << "-> U " << std::endl;

    auto it = CLIENTS.find(this_username);
    if (it != CLIENTS.end()) {
        CLIENTS.erase(it);
        std::string goodbye_message = protocol.GoodbyeMessage(this_username);
        for (const auto& client : CLIENTS) {
            send_message(client.second, goodbye_message);
        }
    }
}

void SERVER::handleList(int _sockFD) {
    std::cout << "-> T" << std::endl;
    std::string list_message = protocol.ListMessage(CLIENTS);
    send_message(_sockFD, list_message);
}

void SERVER::handleBroadcast(int _sockFD, std::string& this_username) {
    std::string broadcast = recv_string(_sockFD, MSG_LENGTH);
    std::string broadcast_message = protocol.BroadcastMessage(broadcast, this_username);
    std::cout << "-> B" << net::format_size(broadcast.size(), 2) << broadcast << std::endl;
    for (const auto& client : CLIENTS) {
        if (client.first != this_username) {
            send_message(client.second, broadcast_message);
        }
    }
}

void SERVER::handlePrivateMessage(int _sockFD, std::string& this_username) {
    std::string receiver = recv_string(_sockFD, MSG_LENGTH);
    std::string message = recv_string(_sockFD, MSG_LENGTH);
    std::string private_message = protocol.PrivateMessage(message, this_username);
    std::cout << "-> M" << net::format_size(receiver.size(), 2) << receiver << net::format_size(message.size(), 2) << message << std::endl;

    auto receiver_iter = CLIENTS.find(receiver);
    if (receiver_iter != CLIENTS.end()) {
        send_message(receiver_iter->second, private_message);
    } else {
        std::string error_message = protocol.ErrorMessage(ERR_PRIV);
        send_message(_sockFD, error_message);
    }
}

void SERVER::handleFileTransfer(int _sockFD, std::string& this_username) {
    std::string file_name = recv_string(_sockFD, MSG_LENGTH);
    char file_size_buffer[15];
    if (recv(_sockFD, file_size_buffer, 15, 0) == -1) {
        perror("recv");
    }
    ssize_t file_size = std::atoi(file_size_buffer);
    std::string receiver = recv_string(_sockFD, MSG_LENGTH);

    std::unique_ptr<unsigned char[]> file_buffer(new unsigned char[file_size]);
    ssize_t numbytes = recv(_sockFD, file_buffer.get(), file_size, 0);
    if (numbytes == -1) {
        perror("recv");
    }

    auto receiver_iter = CLIENTS.find(receiver);
    if (receiver_iter != CLIENTS.end()) {
        std::string file_message = protocol.FileMessage(file_name, file_size, this_username, std::string(reinterpret_cast<char*>(file_buffer.get()), file_size));
        send_message(receiver_iter->second, file_message);
    } else {
        std::string error_message = protocol.ErrorMessage(ERR_PRIV);
        send_message(_sockFD, error_message);
    }
}