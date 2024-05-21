#ifndef SERVER_TOOLKIT_HXX
#define SERVER_TOOLKIT_HXX

#include <netinet/in.h>
#include <cstdint>
#include <string>
#include <map>

#ifndef MAXLINE
    #define MAXLINE 1024
#endif
#ifndef PAD
    #define PAD 46
#endif

class udp_server_toolkit {
public:
    enum MessageType {
        PRIVATE_MESSAGE = 'M',
        BROADCAST = 'B',
        LOGOUT = 'U',
        LOGIN = 'L',
        ERROR = 'E',
        LIST = 'T',
        OK = 'O',
    };

    std::string generate_list_message(const std::map<std::string, struct sockaddr_in>&);
    std::string generate_broadcast_message(const std::string&, const std::string&);
    std::string generate_private_message(const std::string&, const std::string&);
    std::string generate_logout_message(const std::string&);
    std::string generate_login_message(const std::string&);
    std::string generate_error_message(uint8_t);
    std::string generate_ok_message();

    std::string unpad_message(const std::string&);
};

#endif // SERVER_TOOLKIT_HXX
