#include "../include/client.hpp"
#include "../include/net_utilities.hpp"
#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <cstdint>
#include <string>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <vector>


CLIENT::CLIENT(char * hostname, char * port){
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

    try_login();
    start_session_();
}

CLIENT::~CLIENT(){
    close(sockFD);
}

void CLIENT::write_() {
    std::cout << std::endl;
    std::string buffer, formatted_message, space = " ";
    bool running = true;
    
    while(running) {
        std::getline(std::cin, buffer);
        if (buffer.empty()) continue;

        if (buffer[0] == '@') { // private
            size_t delimiter = buffer.find(space); 
            formatted_message = protocol.PrivateMessage(buffer.substr(delimiter + 1), buffer.substr(1, delimiter - 1));
        } else if (buffer == ".list") { // list
            formatted_message = protocol.ListMessage();
        } else if (buffer == ".logout") { // logout
            formatted_message = protocol.LogoutMessage();
            running = false;
        } else if (buffer.substr(0, 5) == ".file") { // file
            size_t last_space = buffer.find_last_of(' '); 
            size_t prev_space = buffer.find_last_of(' ', last_space - 1);
            std::string receiver = buffer.substr(last_space + 1);
            std::string file_name = buffer.substr(prev_space + 1, last_space - prev_space - 1);
            std::vector<std::string> formatted_messages = protocol.FileMessages(file_name, receiver);
            
            for (const auto& message : formatted_messages) {
                if (send(sockFD, message.c_str(), message.size(), 0) == -1)
                    perror("send");
            }
        } else { // public
            formatted_message = protocol.BroadcastMessage(buffer);
        }
        
        if (!formatted_message.empty()) {
            if (send(sockFD, formatted_message.c_str(), formatted_message.size(), 0) == -1)
                perror("send");
        }
    }
}

void CLIENT::read_() {
    char type_buffer[1], message_type;
    while (true) {
        if (recv(sockFD, type_buffer, 1, 0) == -1) {
            perror("recv");
        }

        message_type = type_buffer[0];
        switch(message_type) {
            case LOGIN: // someone login
            case LOGOUT: // someone logout
                handleLoginLogout(message_type);
                break;

            case LIST:
                handleListUsers();
                break;

            case BROADCAST:
            case PRIVATE_MESSAGE:
                handleMessage(message_type);
                break;

            case ERROR:
                handleErrorMessage();
                break;

            case OK:
                // No action needed for 'O'
                break;

            case FILE_TRANSFER:
                handleFileMessage();
                break;

            default:
                // Unknown message type
                break;
        }
    }
    close(sockFD);
}

void CLIENT::start_session_(){
    std::thread worker_thread([this](){read_();});
    worker_thread.detach();
    write_();
}

void CLIENT::try_login() {
    std::string buffer, user, pass, login_message;
    size_t at_position;
    while (true) {
        std::cout << "Type your credentials in the format USER@PASS: ";
        std::getline(std::cin, buffer);
        at_position = buffer.find('@');

        if (at_position != std::string::npos) {
            user = buffer.substr(0, at_position);
            pass = buffer.substr(at_position + 1);
            login_message = protocol.LoginMessage(user, pass);

            if (send(sockFD, login_message.c_str(), login_message.size(), 0) == -1) {
                perror("send");
            }

            char response_buffer[1];
            if (recv(sockFD, response_buffer, 1, 0) == -1) {
                perror("recv");
            }

            if (response_buffer[0] == 'O') {
                std::cout << "Login successful" << std::endl;
                break;
            } else if (response_buffer[0] == 'E') {
                handleErrorMessage();
                std::cerr << "Username already taken." << std::endl;
            } 
        } else {
            std::cerr << "Invalid input format." << std::endl;
        }
    }
}

std::string CLIENT::recv_string(int _sockFD, int size_size) {
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

void *CLIENT::get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void CLIENT::handleLoginLogout(char message_type) {
    std::string action = (message_type == 'l') ? " is here!" : " left the room.";
    std::string username = recv_string(sockFD, 2);
    std::cout << username << action << std::endl;
}
void CLIENT::handleListUsers() {
    char n_users_buffer[2];
    if (recv(sockFD, n_users_buffer, 2, 0) == -1) {
        perror("recv");
    }
    int n_users = std::atoi(n_users_buffer);
    std::string user_list = recv_string(sockFD, 3);
    std::cout << user_list << std::endl;
}
void CLIENT::handleMessage(char message_type) {
    std::string sender = recv_string(sockFD, 2);
    std::string msg = recv_string(sockFD, 2);
    std::cout << (message_type == 'b' ? sender : "priv from " + sender) << ": " << msg << std::endl;
}
void CLIENT::handleErrorMessage() {
    char buffer[3];
    if (recv(sockFD, buffer, 2, 0) == -1) {
        perror("recv");
    }
    buffer[2] = '\0';
    std::cout << "ERROR " << buffer << std::endl;
}
void CLIENT::handleFileMessage() {
    std::string file_name = recv_string(sockFD, 2);
    char file_size_buffer[15];
    if (recv(sockFD, file_size_buffer, 15, 0) == -1) {
        perror("recv");
        return;
    }
    ssize_t file_size = std::atol(file_size_buffer);
    std::string sender = recv_string(sockFD, 2);

    std::cout << "Receiving " << file_name << " from " << sender << std::endl;

    std::ofstream file(file_name, std::ios::binary | std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Error opening file for writing: " << file_name << std::endl;
        return;
    }

    std::vector<unsigned char> buffer(1024);
    ssize_t remaining_bytes = file_size;
    while (remaining_bytes > 0) {
        ssize_t bytes_to_receive = std::min(static_cast<ssize_t>(buffer.size()), remaining_bytes);
        ssize_t bytes_received = recv(sockFD, reinterpret_cast<char*>(buffer.data()), bytes_to_receive, 0);
        if (bytes_received == -1) {
            perror("recv");
            file.close();
            return;
        }
        file.write(reinterpret_cast<const char*>(buffer.data()), bytes_received);
        remaining_bytes -= bytes_received;
    }
    file.close();
}