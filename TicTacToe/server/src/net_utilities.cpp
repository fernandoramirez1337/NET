#include "../include/net_utilities.hpp"

#include <string>
#include <map>

std::string net::format_size(const int size, int n) {
    std::ostringstream oss;
    oss << std::setw(n) << std::setfill('0') << size;
    return oss.str();
}
// E##
std::string net::PROTOCOL::ErrorMessage(int err_n) {
    if (err_n < 0 || err_n > 99) {
        return "E00";
    }
    std::ostringstream oss;
    oss << "E" << std::setw(2) << std::setfill('0') << err_n;
    return oss.str();
}
// O
std::string net::PROTOCOL::OkMessage(){
    std::ostringstream oss;
    oss << "O";
    return oss.str();
}
// l##USERNAME
std::string net::PROTOCOL::WelcomeMessage(std::string username){
    std::ostringstream oss;
    oss << "l" << net::format_size(username.size(), 2) << username;
    return oss.str();
}
// u##USERNAME
std::string net::PROTOCOL::GoodbyeMessage(std::string username){
    std::ostringstream oss;
    oss << "u" << net::format_size(username.size(), 2) << username;
    return oss.str();
}
// t## ###LIST
std::string net::PROTOCOL::ListMessage(const std::map<std::string, int>& myMap){
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
    oss << "t" << map_size << list_size << list;
    return oss.str();
}
// b##SENDER##MSG
std::string net::PROTOCOL::BroadcastMessage(std::string msg, std::string sender){
    std::ostringstream oss;
    oss << "b" << net::format_size(sender.size(), 2) << sender << net::format_size(msg.size(), 2) << msg;
    return oss.str();
}
// m##SENDER##MSG
std::string net::PROTOCOL::PrivateMessage(std::string msg, std::string sender){
    std::ostringstream oss;
    oss << "m" << net::format_size(sender.size(), 2) << sender << net::format_size(msg.size(), 2) << msg;
    return oss.str();
}
// f##FILENAME## ##SENDER FILE
std::string net::PROTOCOL::FileMessage(std::string file_name, ssize_t file_size, std::string sender, std::string data){
    std::ostringstream oss;
    oss << "f" << net::format_size(file_name.size(), 2) << file_name << net::format_size(file_size, 15) << net::format_size(sender.size(), 2) << sender << data;
    return oss.str();
}