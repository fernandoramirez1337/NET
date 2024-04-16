#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "net_utilities.hpp"

#include <string>
#include <vector>

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

    void handleLoginLogout(char);
    void handleListUsers();
    void handleMessage(char);
    void handleErrorMessage();
    void handleFileMessage();

    std::string recv_string(int, int);
    void *get_in_addr(struct sockaddr *);
};

#endif // CLIENT_HPP
