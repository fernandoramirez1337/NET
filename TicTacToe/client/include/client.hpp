#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "net_utilities.hpp"

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <unordered_map>

class CLIENT {
public:
    CLIENT(char *, char *);
    ~CLIENT();

private:
    std::string username;
    int sockFD;
    net::PROTOCOL protocol;

    void write_();
    void read_();
    void start_session_();

    void try_login();

    enum MessageType {
        LOGIN = 'l',
        OK = 'O',
        ERROR = 'E',
        LOGOUT = 'u',
        LIST = 't',
        BROADCAST = 'b',
        PRIVATE_MESSAGE = 'm',
        FILE_TRANSFER = 'f'
    };

    using command_action = std::function<std::string(const std::string&)>;
    std::map<std::string, command_action> command_actions;
    int send_message(const std::string&);

    using handler_function = std::function<void(char)>;
    std::unordered_map<char, handler_function> handle_map;
    void init_handle_map();
    void add_handler(const char, const handler_function);

    void handle_login_logout(char);
    void handle_list_users();
    void handle_message(char);
    void handle_error_message();
    void handle_file_message();

    std::string recv_string(int, int);
    void *get_in_addr(struct sockaddr *);
};

#endif // CLIENT_HPP
