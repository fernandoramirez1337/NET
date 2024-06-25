#ifndef SERVER_HPP
#define SERVER_HPP

#include "typedef.hpp"
#include "server_toolkit.hpp"
#include "server_interface.hpp"

#include <netinet/in.h> // sockaddr_in
#include <cstdint>      // uint16_t
#include <string>       // string
#include <mutex>        // mutex
#include <map>          // map

class server {
public:
  server(const uint16_t);
  ~server();
    
private:
  void read();
  void write();
  void start();

  void handleData(const std::string&, const struct sockaddr_in&);
  void handleInit(const std::string&, const struct sockaddr_in&);
  void handleReset(const std::string&, const struct sockaddr_in&);

  int sendToClient(const std::string&, const struct sockaddr_in&);
  void broadcast();

  std::map<char, struct sockaddr_in> clients;
  serverInterface interface;
  std::mutex clientsMutex;
  serverToolkit toolkit; 
  int udpSock;

  int startUdp(const uint16_t);
};

#endif // SERVER_HXX