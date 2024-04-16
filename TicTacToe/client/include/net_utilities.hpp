#ifndef NET_UTILITIES_HPP
#define NET_UTILITIES_HPP

#include <string>
#include <vector>

namespace net {
    std::string format_size(const int, int);
    std::vector<std::vector<unsigned char>> readAndDivideFile(const std::string&, size_t);
    class PROTOCOL{
        public:
        std::string ErrorMessage(int);
        std::string OkMessage();
        std::string LoginMessage(std::string, std::string);
        std::string LogoutMessage();
        std::string ListMessage();
        std::string BroadcastMessage(std::string);
        std::string PrivateMessage(std::string, std::string);
        std::vector<std::string> FileMessages(std::string, std::string);
        std::string TTTMessage(std::string);
    };
}

#endif // NET_UTILITIES_HPP
