#ifndef SERVER_HXX
#define SERVER_HXX

#include "typedef.hxx"
#include "server_toolkit.hxx"
#include "server_interface.hxx"
/* <string> <map> <vector> */
#include <netinet/in.h>
#include <cstdint>
#include <mutex>

class server {
public:
  server(const uint16_t);
  ~server();
    
private:
  void read();
  void write();
  void start();

  void handle_data(const std::string&, const struct sockaddr_in&);
  void handle_init(const std::string&, const struct sockaddr_in&);
  void handle_reset(const std::string&, const struct sockaddr_in&);

  int send_to_client(const std::string&, const struct sockaddr_in&);

  std::map<char, struct sockaddr_in> clients;
  server_interface interface;
  server_toolkit toolkit; 
  std::mutex clients_mutex;
  int udp_sock;

  int start_udp(const uint16_t);
};

#endif // SERVER_HXX