#include "../include/server.hpp"
#include "../include/net_utilities.hpp"
#include "../include/tictactoe.hpp"
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
const int ERR_LOGIN_DUPLICATES = 1;
const int ERR_LOGIN = 2;
const int ERR_PRIV = 11;
const int ERR_PRIV_RECV = 12;
const int ERR_FILE = 21;
const int ERR_FILE_RECV = 22;
const int STANDARD_MESSAGE_SIZE = 2;
const int STANDARD_FILE_SIZE = 15;

void sigchld_handler(int s) {
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

SERVER::SERVER(const char *port) : game('O') {
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
    init_handle_map();
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

void SERVER::session_(const int _session_socket){
    std::thread worker_thread([this, _session_socket](){read_write_(reinterpret_cast<void *>(_session_socket));});
    worker_thread.detach();
}

void SERVER::read_write_(const void *_void_socket) {
    int session_socket = (intptr_t)_void_socket;
    std::string session_username;
    char type_buffer[1];
    
    while (true) {
        type_buffer[0] = '\n';
        if (recv(session_socket, type_buffer, 1, 0) == -1) {
            perror("recv");
        }
        char message_type = type_buffer[0];
        auto it = handle_map.find(message_type);
        if (it != handle_map.end()) {
            std::cout << "-> " << message_type;
            it->second(session_socket, session_username);
        } else { } // unknown message type
    }
}

std::string SERVER::recv_data(const int _socket, const int _size_size) {
    std::unique_ptr<char[]> size_buffer(new char[_size_size]);
    if (recv(_socket, size_buffer.get(), _size_size, 0) == -1) {
        perror("recv");
    }
    std::cout << std::string(size_buffer.get(), _size_size);
    int data_size = atoi(size_buffer.get());

    std::unique_ptr<char[]> buffer(new char[data_size]);
    ssize_t numbytes = recv(_socket, buffer.get(), data_size, 0);
    if (numbytes == -1) {
        perror("recv");
    }
    std::cout << std::string(buffer.get(), data_size);
    return std::string(buffer.get(), numbytes);
}

void SERVER::send_data(const int _socket, const std::string _message) {
        if (send(_socket, _message.c_str(), _message.size(), 0) == -1)
            perror("send");
        else
            std::cout << "<- " << _message << std::endl;
    }

void SERVER::init_handle_map() {
    add_handler(LOGIN, [this](int _sock, std::string& _username) { handle_login(_sock, _username); });
    add_handler(OK, [this](int _sock, std::string& _username) { handle_ok(); });
    add_handler(ERROR, [this](int _sock, std::string& _username) { handle_error(_sock, _username); });
    add_handler(LOGOUT, [this](int _sock, std::string& _username) { handle_logout(_username); });
    add_handler(LIST, [this](int _sock, std::string& _username) { handle_list(_sock); });
    add_handler(BROADCAST, [this](int _sock, std::string& _username) { handle_broadcast(_sock, _username); });
    add_handler(PRIVATE_MESSAGE, [this](int _sock, std::string& _username) { handle_private_message(_sock, _username); });
    add_handler(FILE_TRANSFER, [this](int _sock, std::string& _username) { handle_file_transfer(_sock, _username); });
    add_handler(GAME, [this] (int _sock, std::string & _username) { handle_tictactoe(_sock, _username); });
}

void SERVER::add_handler(const char _message_type, const handler_function _handler) {
    auto result = handle_map.emplace(_message_type, _handler);
    if (!result.second) { std::cerr << "Error: Unable to add handler to the map" << std::endl; }
}

void SERVER::handle_login(const int _socket, std::string& _this_username) {
    std::string username = recv_data(_socket, STANDARD_MESSAGE_SIZE);
    std::string password = recv_data(_socket, STANDARD_MESSAGE_SIZE);
    std::cout << std::endl;
    bool is_duplicate = CLIENTS.find(username) != CLIENTS.end();

    if (is_duplicate) {
        std::string error_message = protocol.ErrorMessage(ERR_LOGIN_DUPLICATES);
        send_data(_socket, error_message);
    } else {
        _this_username = username;
        CLIENTS.emplace(username, _socket);
        std::string ok_message = protocol.OkMessage();
        send_data(_socket, ok_message);

        std::string welcome_message = protocol.WelcomeMessage(username);
        for (const auto& client : CLIENTS) {
            if (client.first != username) { send_data(client.second, welcome_message); }
        }
    }
}

void SERVER::handle_ok() {
    std::cout << std::endl;
}

void SERVER::handle_error(const int _socket, const std::string& _session_username) {
    char error_type_buffer[2];
    if (recv(_socket, error_type_buffer, 2, 0) == -1) { perror("recv"); }
    std::cout << "RECEIVED ERROR " << std::string(error_type_buffer, 2) << " FROM " << _session_username <<std::endl;
}

void SERVER::handle_logout(const std::string& _session_username) {
    std::cout << std::endl;
    auto it = CLIENTS.find(_session_username);
    if (it != CLIENTS.end()) {
        std::string goodbye_message = protocol.GoodbyeMessage(_session_username);
        for (auto it_client = CLIENTS.begin(); it_client != CLIENTS.end();) {
            if (it_client->first != _session_username) {
                send_data(it_client->second, goodbye_message);
                ++it_client;
            } else { it_client = CLIENTS.erase(it_client); }
        }
    }
}

void SERVER::handle_list(const int _socket) {
    std::cout << std::endl;
    std::string list_message = protocol.ListMessage(CLIENTS);
    send_data(_socket, list_message);
}

void SERVER::handle_broadcast(const int _socket, const std::string& _session_username) {
    std::string broadcast = recv_data(_socket, STANDARD_MESSAGE_SIZE);
    std::cout << std::endl;
    std::string broadcast_message = protocol.BroadcastMessage(broadcast, _session_username);
    for (const auto& client : CLIENTS) {
        if (client.first != _session_username) { send_data(client.second, broadcast_message); }
    }
}

void SERVER::handle_private_message(int _socket, const std::string& _session_username) {
    std::string receiver = recv_data(_socket, STANDARD_MESSAGE_SIZE);
    std::string message = recv_data(_socket, STANDARD_MESSAGE_SIZE);
    std::cout << std::endl;
    std::string private_message = protocol.PrivateMessage(message, _session_username);

    auto receiver_iter = CLIENTS.find(receiver);
    if (receiver_iter != CLIENTS.end()) { send_data(receiver_iter->second, private_message); } 
    else { send_data(_socket, protocol.ErrorMessage(ERR_PRIV)); }
}

void SERVER::handle_file_transfer(const int _socket, const std::string& _session_username) {
    std::string file_name = recv_data(_socket, STANDARD_MESSAGE_SIZE);
    char file_size_buffer[STANDARD_FILE_SIZE];
    if (recv(_socket, file_size_buffer, STANDARD_FILE_SIZE, 0) == -1) { perror("recv"); }
    std::cout << std::string(file_size_buffer, STANDARD_FILE_SIZE);
    ssize_t file_size = std::atoi(file_size_buffer);
    std::string receiver = recv_data(_socket, STANDARD_MESSAGE_SIZE);

    std::unique_ptr<unsigned char[]> file_buffer(new unsigned char[file_size]);
    ssize_t numbytes = recv(_socket, file_buffer.get(), file_size, 0);
    if (numbytes == -1) { perror("recv"); }
    std::cout.write(reinterpret_cast<const char*>(file_buffer.get()), numbytes);
    std::cout << std::endl;

    auto receiver_iter = CLIENTS.find(receiver);
    if (receiver_iter != CLIENTS.end()) {
        std::string file_message = protocol.FileMessage(file_name, file_size, _session_username, std::string(reinterpret_cast<char*>(file_buffer.get()), file_size));
        send_data(receiver_iter->second, file_message);
    } else { send_data(_socket, protocol.ErrorMessage(ERR_PRIV)); }
}

void SERVER::handle_tictactoe(int _socket, const std::string& _session_username){
    char TTT_command_buffer[STANDARD_MESSAGE_SIZE];
    if (recv(_socket, TTT_command_buffer, STANDARD_MESSAGE_SIZE, 0) == -1) { perror("recv"); }
    std::string TTT_command(TTT_command_buffer,STANDARD_MESSAGE_SIZE);
    std::cout << TTT_command << std::endl;
    std::string TTT_broadcast_message = protocol.BroadcastMessage(game.makeMove(TTT_command), _session_username);
    for (const auto& client : CLIENTS) { send_data(client.second, TTT_broadcast_message); }
}