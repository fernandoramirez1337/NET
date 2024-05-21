#ifndef NET_UTILITIES_HPP
#define NET_UTILITIES_HPP

#include <string>
#include <sstream>
#include <iomanip>


namespace net {
    //void add_padding();
    //void remove_padding();
    // int to string according to protocol 
    std::string format_size(const int, int);
    // string to int - std::stoi
    int format_size(const std::string&);
}

#endif // NET_UTILITIES_HPP
