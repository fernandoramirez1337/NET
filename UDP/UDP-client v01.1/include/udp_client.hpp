#ifndef UDP_CLIENT_HPP
#define UDP_CLIENT_HPP

#include "udp_utilities.hpp"

#include <string>
//#include <map>

class udp_client {
private:
    std::string username;
    int sock;
    void receive_thread();
    void * receive();
    void send();
    std::string protocol(std::string);
    //std::map<std::string, int> directory;
    
public:
    void start();
    udp_client(char *, char *);
    ~udp_client();
};

#endif