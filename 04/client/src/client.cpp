#include "../include/client.hpp"
#include "../include/net_utilities.hpp"
#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <cstdint>
#include <vector>
#include <string>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include <stdexcept> // For std::runtime_error
#include <sstream>
#include <iomanip>


#define MAXDATASIZE 100000

void *get_in_addr(struct sockaddr *sa);
std::string generate_error_message(int err_n);
std::string generate_ok_message();
std::string generate_login_message(std::string username, std::string password);
std::string generate_logout_message();
std::string generate_list_message();
std::string generate_broadcast_message(std::string msg);
std::string generate_private_message(std::string msg, std::string sender);

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
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
// L##USERNAME##PASSWORD
std::string generate_login_message(std::string username, std::string password){
    std::ostringstream oss;
    oss << "L" << net::format_size(username.size(), 2) << username << net::format_size(password.size(), 2) << password << "\n";
    return oss.str();
}
// U
std::string generate_logout_message(){
    std::ostringstream oss;
    oss << "U" << "\n";
    return oss.str();
}
// T
std::string generate_list_message(){
    std::ostringstream oss;
    oss << "T" << "\n";
    return oss.str();
}
// B##MSG
std::string generate_broadcast_message(std::string msg){
    std::ostringstream oss;
    oss << "B" << net::format_size(msg.size(), 2) << msg << '\n';
    return oss.str();
}
// M##RECEIVER##MSG
std::string generate_private_message(std::string msg, std::string receiver){
    std::ostringstream oss;
    oss << "M" << net::format_size(receiver.size(), 2) << receiver << net::format_size(msg.size(), 2) << msg << '\n';
    return oss.str();
}

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

    start_();
}

CLIENT::~CLIENT(){
    close(sockFD);
}

void CLIENT::write_(){
    std::cout << std::endl;
    std::string buffer, f_buffer, receiver, message, space = " ";
    bool mamamia = 1;
    while(mamamia){
        std::getline(std::cin, buffer);

        f_buffer.clear();
        //buffer.clear();

        if (buffer[0] == '@'){              // private
            int delimiter = buffer.find(space); 
            receiver = buffer.substr(1, delimiter-1);
            message = buffer.substr(delimiter+1, buffer.size());
            f_buffer = generate_private_message(message, receiver);
        } else if (buffer == ".list") {         // list
            f_buffer = generate_list_message();

        } else if (buffer == ".logout") {    // logout
            f_buffer = generate_logout_message();
            mamamia = 0;

        } else {                            // public
            f_buffer = generate_broadcast_message(buffer);
        }
        if (send(sockFD, f_buffer.c_str(), f_buffer.size(), 0) == -1)
            perror("send");
    }
}

void CLIENT::read_(){
    char buffer[MAXDATASIZE];
    std::string f_buffer;

    while (1)
    {
        ssize_t numbytes = ssize_t(0);
        if ((numbytes = recv(sockFD, buffer, 1, 0)) == -1){
            perror("recv");
            exit(1);
        }
        char message_type = buffer[0];

        if (message_type == 'l') { // someone login
        f_buffer = recv_string(sockFD, 2);
        std::cout << f_buffer << " is here!" << std::endl;

        } else if (message_type == 'u') { //someone logout
        f_buffer = recv_string(sockFD, 2);
        std::cout << f_buffer << " left the room." << std::endl;


        } else if (message_type == 't') { // list of online users
        if ((numbytes = recv(sockFD, buffer, 2, 0)) == -1){
            perror("recv");
            exit(1);
        }
        buffer[numbytes] = 0;
        int n_users = std::atoi(buffer);
        f_buffer = recv_string(sockFD, 3);
        std::cout << f_buffer << std::endl;
        
        } else if (message_type == 'b') { // broadcast message
        std::string sender = recv_string(sockFD, 2);
        std::string msg = recv_string(sockFD, 2);
        std::cout << sender << ": " << msg << std::endl;

        } else if (message_type == 'm') { // private message
        std::string sender = recv_string(sockFD, 2);
        std::string msg = recv_string(sockFD, 2);
        std::cout << "priv from "<<sender << ": " << msg << std::endl;
        
        } else if (message_type == 'E') { // error
            if ((numbytes = recv(sockFD, buffer, 2, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;
            std::cout << "ERROR " << buffer << std::endl; 
        } else if (message_type == 'O') { // ok
        } else if (message_type == 'f') { // file
        
        } else {}

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

std::string CLIENT::recv_string(int _sockFD, int size_size) {
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

void CLIENT::try_login() {
    std::string f_buffer, user, pass;
    char buffer[MAXDATASIZE];

    bool loginSuccessful = false;
    while (!loginSuccessful) {
        std::cout << "Type your credentials in the format USER@PASS: ";
        std::getline(std::cin, f_buffer);

        // Find the position of '@' character
        size_t at_position = f_buffer.find('@');

        if (at_position != std::string::npos) {
            // Extract the username and password
            user = f_buffer.substr(0, at_position);
            pass = f_buffer.substr(at_position + 1);

            f_buffer = generate_login_message(user, pass);

            // Sending login message to the server
            if (send(sockFD, f_buffer.c_str(), f_buffer.size(), 0) == -1) {
                perror("send");
                throw std::runtime_error("Failed to send data to server");
            }

            // Receiving response from the server
            int numbytes = recv(sockFD, buffer, MAXDATASIZE, 0);
            if (numbytes == -1) {
                perror("recv");
                throw std::runtime_error("Failed to receive data from server");
            }
            buffer[numbytes] = '\0';

            if (buffer[0] == 'O') {
                std::cout << "Login successful" << std::endl;
                loginSuccessful = true;
            } else {
                std::cerr << "Username already taken. Please type in the format USER@PASS." << std::endl;
            }
        } else {
            std::cerr << "Invalid input format. Please type in the format USER@PASS." << std::endl;
        }
    }
}