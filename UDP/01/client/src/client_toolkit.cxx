#include "../include/client_toolkit.hxx"
/* <string> <cstdint> */
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>

std::string pad_message(const std::string& msg) {
    if (msg.size() < MAXLINE) {
        std::ostringstream oss;
        oss << msg;
        oss << std::setw(MAXLINE - msg.size()) << std::setfill((char)PAD) << "";
        return oss.str();
    }
    return msg;
}

std::string udp_client_toolkit::unpad_message(const std::string& msg) {
    auto end = std::find_if(msg.rbegin(), msg.rend(), [](char ch) {
        return ch != (char)PAD;
    }).base();
    return std::string(msg.begin(), end);
}

void udp_client_toolkit::handle_sys_message(const std::string& message){
    std::string action, username; ssize_t username_size;
    username_size = std::stoi(message.substr(1,2));
    username = message.substr(3, username_size);
    action = (message[0] == 'l') ? " is here!" : " left the room.";
    std::cout << username << action << std::endl;
}

void udp_client_toolkit::handle_list_message(const std::string& message){
    std::string list; ssize_t list_size;
    list_size = std::stoi(message.substr(3,3));
    list = message.substr(6, list_size);
    std::cout << "list of users in chat:" << std::endl << list << std::endl;
}

void udp_client_toolkit::handle_message(const std::string& message){
    std::string chat_type, sender, data; ssize_t sender_size, data_size;
    chat_type = (message[0] == 'b') ? "global" : "private";
    sender_size = std::stoi(message.substr(1,2));
    sender = message.substr(3, sender_size);
    data_size = std::stoi(message.substr(3 + sender_size, 2));
    data = message.substr(5 + sender_size, data_size);
    std::cout << sender << "@" << chat_type << ": " << data << std::endl;
}

void udp_client_toolkit::handle_error_message(uint8_t err_n){
    std::cout << "SERVER: ERROR" << static_cast<int>(err_n) << std::endl;
}

std::string udp_client_toolkit::generate_error_message(uint8_t err_n) {
    std::ostringstream oss;
    if (err_n < 0 || err_n > 99) {
        oss << "E00";
    } else {
        oss << "E"; 
        oss << std::setw(2) << std::setfill('0') << static_cast<int>(err_n);
    }
    return pad_message(oss.str());
}

std::string udp_client_toolkit::generate_ok_message(){
    std::ostringstream oss;
    oss << "O";
    return pad_message(oss.str());
}

std::string udp_client_toolkit::generate_login_message(const std::string& username, const std::string& password) {
    std::ostringstream oss;
    oss << "L";
    oss << std::setw(2) << std::setfill('0') << username.size();
    oss << username;
    oss << std::setw(2) << std::setfill('0') << password.size();
    oss << password;
    return pad_message(oss.str());
}

std::string udp_client_toolkit::generate_logout_message(const std::string& username){
    std::ostringstream oss;
    oss << "U";
    oss << std::setw(2) << std::setfill('0') << username.size();
    oss << username;
    return pad_message(oss.str());
}

std::string udp_client_toolkit::generate_list_message(){
    std::ostringstream oss;
    oss << "T";
    return pad_message(oss.str());
}

std::string udp_client_toolkit::generate_broadcast_message(const std::string& message){
    std::ostringstream oss;
    oss << "B";
    oss << std::setw(2) << std::setfill('0') << message.size();
    oss << message;
    return pad_message(oss.str());
}

std::string udp_client_toolkit::generate_private_message(const std::string& message, const std::string& receiver){
    std::ostringstream oss;
    oss << "M";
    oss << std::setw(2) << std::setfill('0') << receiver.size();
    oss << receiver;
    oss << std::setw(2) << std::setfill('0') << message.size();
    oss << message;
    return pad_message(oss.str());
}
