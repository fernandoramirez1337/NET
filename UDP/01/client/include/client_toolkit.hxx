#ifndef CLIENT_TOOLKIT_HXX
#define CLIENT_TOOLKIT_HXX

#include <cstdint>
#include <string>

#ifndef MAXLINE
    #define MAXLINE 1024
#endif
#ifndef PAD
    #define PAD 46
#endif

class udp_client_toolkit {
public:
    enum message_flag {
        PRIVATE_MESSAGE = 'm',
        BROADCAST = 'b',
        LOGOUT = 'u',
        LOGIN = 'l',
        ERROR = 'E',
        LIST = 't',
        OK = 'O',
    };

    void handle_list_message(const std::string&);
    void handle_sys_message(const std::string&);
    void handle_message(const std::string&);
    void handle_error_message(uint8_t);

    std::string generate_private_message(const std::string&, const std::string&);
    std::string generate_login_message(const std::string&, const std::string&);
    std::string generate_broadcast_message(const std::string&);
    std::string generate_logout_message(const std::string&);
    std::string generate_error_message(uint8_t);
    std::string generate_list_message();
    std::string generate_ok_message();

    std::string unpad_message(const std::string&);    
};

static std::string help_message = R"(
Usage:
- Broadcast message: <message>
Example: Hello everyone!

- Private message: @<receiver> <message>
Example: @John Hey, how are you?

- List of users: .list
Example: .list

- Help message: .help

Note:
- Make sure to use proper syntax for each command.
- Replace <message> and <receiver> with your actual message and recipient name respectively.
- Private messages should be addressed to a specific user using @<receiver>.
)";

#endif // CLIENT_TOOLKIT_HXX
