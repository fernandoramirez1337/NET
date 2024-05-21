#include "../include/client.hxx"
/* <string> <cstdint> <netinet/in.h> */
#include <arpa/inet.h>
#include <stdexcept>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <netdb.h>
#include <vector>
#include <thread>

#ifndef SPACE
    #define SPACE 32
#endif
#ifndef ATSIGN
    #define ATSIGN 64
#endif


udp_client::udp_client(const std::string& server_ip, uint16_t server_port){
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP
    if (getaddrinfo(server_ip.c_str(),nullptr,&hints,&res) != 0) {
        throw std::runtime_error("getaddrinfo failed");
    }

    udp_sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (udp_sock < 0) {
        freeaddrinfo(res);
        throw std::runtime_error("socket creation failed: " + std::string(strerror(errno)));
    }

    server_addr = *reinterpret_cast<struct sockaddr_in*>(res->ai_addr);
    server_addr.sin_port = htons(server_port);
    addrlen = sizeof(server_addr);
    freeaddrinfo(res);

    try_login();
    start();
}

udp_client::~udp_client(){
    close(udp_sock);
}

void udp_client::write(){
    const std::string list_cmd = ".list", help_cmd = ".help", logout_cmd = ".logout";
    std::string cin_buffer;
    bool running = true;

    while (running) {
        cin_buffer.clear();
        std::getline(std::cin, cin_buffer);
        if (cin_buffer.empty()) continue;

        if (cin_buffer[0] == (char)ATSIGN) {
            size_t first_space = cin_buffer.find((char)SPACE);
            send_to_server(toolkit.generate_private_message(cin_buffer.substr(first_space + 1), cin_buffer.substr(1, first_space - 1)));
        } else if (cin_buffer == list_cmd) {
            send_to_server(toolkit.generate_list_message());
        } else if (cin_buffer == help_cmd) {    
            std::cout << help_message << std::endl;
        } else if (cin_buffer == logout_cmd) {
            send_to_server(toolkit.generate_logout_message(client_username));
            running = false;
        } else {
            send_to_server(toolkit.generate_broadcast_message(cin_buffer));
        }
    }
}
void udp_client::read(){
    char message_type;
    std::string message;
    
    while (true) {
        message.clear();
        message = toolkit.unpad_message(recv_from_server());
        message_type = message[0];
        switch(message_type) {
            case toolkit.LOGIN: 
            case toolkit.LOGOUT: 
                toolkit.handle_sys_message(message);
                break;

            case toolkit.LIST:
                toolkit.handle_list_message(message);
                break;

            case toolkit.BROADCAST:
            case toolkit.PRIVATE_MESSAGE:
                toolkit.handle_message(message);
                break;

            case toolkit.ERROR:
                toolkit.handle_error_message(std::stoi(message.substr(1,2)));
                break;

            case toolkit.OK:
                // No action needed for 'O'
                break;

            default:
                // Unknown message type
                break;
        }
    }
    close(udp_sock);
}
void udp_client::start(){
    session();
}
void udp_client::session(){
    std::thread worker_thread([this](){read();});
    worker_thread.detach();
    write();
}

int udp_client::send_to_server(const std::string& data) {
    return sendto(udp_sock, data.c_str(), MAXLINE, MSG_CONFIRM, reinterpret_cast<const struct sockaddr*>(&server_addr), addrlen);
}

std::string udp_client::recv_from_server() {
    std::vector<char> buffer(MAXLINE);
    ssize_t n = recvfrom(udp_sock, buffer.data(), MAXLINE, MSG_WAITALL, reinterpret_cast<struct sockaddr*>(&server_addr), &addrlen);
    if (n < 0) {
        return "?";
    }
    return std::string(buffer.begin(), buffer.begin() + n);
}

void udp_client::try_login() {
    std::string buffer, user, pass, login_message;
    size_t at_position;
    while (true) {
        std::cout << "Type your credentials in the format USER@PASS: ";
        std::getline(std::cin, buffer);
        at_position = buffer.find('@');

        if (at_position != std::string::npos) {
            user = buffer.substr(0, at_position);
            pass = buffer.substr(at_position + 1);
            login_message = toolkit.generate_login_message(user, pass);
            if (send_to_server(login_message) == -1) { perror("send"); }

            std::string response_buffer;
            response_buffer = recv_from_server();

            if (response_buffer[0] == 'O') {
                std::cout << "Login successful" << std::endl;
                client_username = user;
                break;
            } else if (response_buffer[0] == 'E') {
                toolkit.handle_error_message(std::stoi(response_buffer.substr(1,2)));
                std::cerr << "Username already taken." << std::endl;
            } 
        } else {
            std::cerr << "Invalid input format." << std::endl;
        }
    }
}