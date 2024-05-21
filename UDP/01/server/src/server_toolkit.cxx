#include "../include/server_toolkit.hxx"
/* <netinet/in.h> <cstdint> <string> <map> */
#include <algorithm>
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

std::string udp_server_toolkit::unpad_message(const std::string& msg) {
    auto end = std::find_if(msg.rbegin(), msg.rend(), [](char ch) {
        return ch != (char)PAD;
    }).base();
    return std::string(msg.begin(), end);
}

std::string udp_server_toolkit::generate_error_message(uint8_t err_n) {
    std::ostringstream oss;
    if (err_n > 99 || err_n < 0) {
        oss << "E00";
    } else {
        oss << "E";
        oss << std::setw(2) << std::setfill('0') << static_cast<int>(err_n);
    }
    return pad_message(oss.str());
}

std::string udp_server_toolkit::generate_ok_message() {
    std::ostringstream oss;
    oss << "O";
    return pad_message(oss.str());
}

std::string udp_server_toolkit::generate_login_message(const std::string& username) {
    std::ostringstream oss;
    oss << "l";
    oss << std::setw(2) << std::setfill('0') << username.size();
    oss << username;
    return pad_message(oss.str());
}

std::string udp_server_toolkit::generate_logout_message(const std::string& username) {
    std::ostringstream oss;
    oss << "u";
    oss << std::setw(2) << std::setfill('0') << username.size();
    oss << username;
    return pad_message(oss.str());
}

std::string udp_server_toolkit::generate_list_message(const std::map<std::string, struct sockaddr_in>& clients) {
    ssize_t map_size, list_size; std::string list; std::ostringstream oss;
    auto it = clients.begin();
    if (it != clients.end()) {
        oss << it->first;
        ++it;
    }
    for (; it != clients.end(); ++it) {
        oss << "," << it->first;
    }
    map_size = clients.size();
    list_size = oss.str().size();
    list = oss.str();
    oss.str("");
    oss << "t";
    oss << std::setw(2) << std::setfill('0') << map_size;
    oss << std::setw(3) << std::setfill('0') << list_size;
    oss << list;
    return pad_message(oss.str());
}

std::string udp_server_toolkit::generate_broadcast_message(const std::string& sender, const std::string& message) {
    std::ostringstream oss;
    oss << "b";
    oss << std::setw(2) << std::setfill('0') << sender.size();
    oss << sender;
    oss << std::setw(2) << std::setfill('0') << message.size();
    oss << message;
    return pad_message(oss.str());
}

std::string udp_server_toolkit::generate_private_message(const std::string& sender, const std::string& message) {
    std::ostringstream oss;
    oss << "m";
    oss << std::setw(2) << std::setfill('0') << sender.size();
    oss << sender;
    oss << std::setw(2) << std::setfill('0') << message.size();
    oss << message;
    return pad_message(oss.str());
}
