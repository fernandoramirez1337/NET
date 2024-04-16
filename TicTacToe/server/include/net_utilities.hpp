#ifndef NET_UTILITIES_HPP
#define NET_UTILITIES_HPP

#include <string>
#include <sstream>
#include <iomanip>
#include <map>


namespace net {

    std::string format_size(const int, int);
    class PROTOCOL {
        public:
        std::string ErrorMessage(int);
        std::string OkMessage();
        std::string WelcomeMessage(std::string);
        std::string GoodbyeMessage(std::string);
        std::string ListMessage(const std::map<std::string, int>&);
        std::string BroadcastMessage(std::string, std::string);
        std::string PrivateMessage(std::string, std::string);
        std::string FileMessage(std::string, ssize_t, std::string, std::string);
    };    
}

#endif // NET_UTILITIES_HPP
