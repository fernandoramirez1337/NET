#ifndef CLIENT_HXX
#define CLIENT_HXX

#include "client_toolkit.hxx"
/* <string> <cstdint> */
#include <netinet/in.h>

#ifndef MAXLINE
    #define MAXLINE 1024
#endif
#ifndef PAD
    #define PAD 46
#endif

class udp_client {
public:
    udp_client(const std::string&, uint16_t);
    ~udp_client();

private:
    void session();
    void start();
    void write();
    void read();

    struct sockaddr_in server_addr;
    std::string client_username;
    udp_client_toolkit toolkit;
    socklen_t addrlen;
    int udp_sock;

    int send_to_server(const std::string&);
    std::string recv_from_server();
    void try_login();
};

#endif // CLIENT_HXX