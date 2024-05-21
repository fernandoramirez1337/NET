#ifndef SERVER_HXX
#define SERVER_HXX

#include "server_toolkit.hxx"
/* <netinet/in.h> <cstdint> <string> <map> */
#include <mutex>

#ifndef MAXLINE
    #define MAXLINE 1024
#endif
#ifndef PAD
    #define PAD 46
#endif

class udp_server {
public:
    udp_server(uint16_t);
    ~udp_server();
    
private:
    void read_write();
    void start();

    void handle_private_message(const std::string&, const struct sockaddr_in&);
    void handle_broadcast(const std::string&, const struct sockaddr_in&);
    void handle_login(const std::string&, const struct sockaddr_in&);
    void handle_list(const struct sockaddr_in&);
    void handle_logout(const std::string&);
    void handle_error(const std::string&);
    void handle_ok();

    void send_to_client(const std::string&, const struct sockaddr_in&);

    std::map<std::string, struct sockaddr_in> clients;
    udp_server_toolkit toolkit; 
    std::mutex clients_mutex;
    int udp_sock;
};

#endif // SERVER_HXX