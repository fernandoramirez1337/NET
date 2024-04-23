#ifndef SERVER_HPP
#define SERVER_HPP

#include "tictactoe.hpp"
#include "net_utilities.hpp"

#include <map>
#include <unordered_map>
#include <string>
#include <functional>

class SERVER {
public:
    SERVER(const char *);
    ~SERVER();

private:
    std::map<std::string, int> CLIENTS;
    int sockFD;
    net::PROTOCOL protocol;
    TicTacToe game;

    void start_();
    int accept_();
    void session_(const int);
    void read_write_(const void *);

    enum MessageType {
        LOGIN = 'L',
        OK = 'O',
        ERROR = 'E',
        LOGOUT = 'U',
        LIST = 'T',
        BROADCAST = 'B',
        PRIVATE_MESSAGE = 'M',
        FILE_TRANSFER = 'F',
        GAME = 'G'
    };

    std::string recv_data(const int, const int);
    void send_data(const int, const std::string);

    using handler_function = std::function<void(int, std::string&)>;
    std::unordered_map<char,handler_function> handle_map;
    void init_handle_map();
    void add_handler(const char, const handler_function);

    void handle_login(const int, std::string&);
    void handle_ok();
    void handle_error(const int, const std::string&);
    void handle_logout(const std::string&);
    void handle_list(const int);
    void handle_broadcast(const int, const std::string&);
    void handle_private_message(const int, const std::string&);
    void handle_file_transfer(const int, const std::string&);
    void handle_tictactoe(const int, const std::string&);
};

#endif // SERVER_HPP
