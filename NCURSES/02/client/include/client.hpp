#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "typedef.hpp"
#include "client_toolkit.hpp"
#include "client_interface.hpp"

#include <netinet/in.h>

class client {
public:
  client(const std::string&, const uint16_t);
  ~client();

private:
  void startSession();
  void write();
  void read();

  struct sockaddr_in serverAddr;
  clientInterface interface;
  clientToolkit toolkit;
  char clientCharacter;
  socklen_t addrLen;
  int udpSock;

  int sendToServer(const std::string&);
  std::string recvFromServer();
  void tryLogin();
};

#endif // CLIENT_HPP