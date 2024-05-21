#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <string>

class SERVER {
public:
    SERVER(char *);
    ~SERVER();

private:
    std::map<std::string, int> CLIENTS;
    int sockFD;

    int accept_();
    void read_write_(void *);
    void start_();
    void session_(int);

    std::string recv_string(int, int);
};

#endif // SERVER_HPP
