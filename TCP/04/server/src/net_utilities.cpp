#include "../include/net_utilities.hpp"

//void net::add_padding(){};

//void net::remove_padding(){};

std::string net::format_size(const int size, int n) {
    std::ostringstream oss;
    oss << std::setw(n) << std::setfill('0') << size;
    return oss.str();
}

int format_size(const std::string& size) {
    return std::stoi(size);
}