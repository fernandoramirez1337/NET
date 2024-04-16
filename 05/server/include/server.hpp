#ifndef SERVER_HPP
#define SERVER_HPP

#include "net_utilities.hpp"

#include <map>
#include <string>

class SERVER {
public:
    SERVER(char *);
    ~SERVER();

private:
    std::map<std::string, int> CLIENTS;
    int sockFD;
    net::PROTOCOL protocol;

    int accept_();
    void read_write_(void *);
    void start_();
    void session_(int);

    enum MessageType {
        LOGIN = 'L',
        OK = 'O',
        ERROR = 'E',
        LOGOUT = 'U',
        LIST = 'T',
        BROADCAST = 'B',
        PRIVATE_MESSAGE = 'M',
        FILE_TRANSFER = 'F'
    };

    std::string recv_string(int, int);
    void send_message(int, const std::string&);

    void handleLogin(int, std::string&);
    void handleOk();
    void handleError();
    void handleLogout(std::string&);
    void handleList(int);
    void handleBroadcast(int, std::string&);
    void handlePrivateMessage(int, std::string&);
    void handleFileTransfer(int, std::string&);

};

#endif // SERVER_HPP
