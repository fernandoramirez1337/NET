#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class CLIENT {
public:
    CLIENT(char *, char *);
    ~CLIENT();

private:
    std::string username;
    int sockFD;

    void write_();
    void read_();
    void start_();
    void session_();

    std::string recv_string(int, int);
    void try_login();
};

#endif // CLIENT_HPP
