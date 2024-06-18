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

class client {
public:
  client(const std::string&, const uint16_t);
  ~client();

private:
  void session();
  void start();
  void write();
  void read();

  struct sockaddr_in server_addr;
  client_toolkit toolkit;
  char client_character;
  socklen_t addrlen;
  int udp_sock;

  int send_to_server(const std::string&);
  std::string recv_from_server();
  void try_login();
  int start_udp(const std::string&, const uint16_t);
  void start_ncurses();
  void draw_screen();
};

#endif // CLIENT_HXX