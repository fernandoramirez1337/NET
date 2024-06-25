#include "../include/client_toolkit.hpp"
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdexcept>


#include <string>
#include <cstdint>

int clientToolkit::pad(std::string& message) {
  ssize_t message_size = message.size();
  if (message_size >= MAXLINE) return EXIT_FAILURE;
  message.append(MAXLINE - message_size, PAD);
  return EXIT_SUCCESS;
}

int clientToolkit::unpad(std::string& message) {
  ssize_t message_end = message.find_last_not_of(PAD);
  if (message_end == std::string::npos) return EXIT_FAILURE; 
  message.resize(message_end + 1);
  return EXIT_SUCCESS;
}

void clientToolkit::handleData(const std::string& data) {
  std::string characters = data.substr(1, 800);
  int x = 0, y = 0; const char space = ' ';

  for (const auto& character : characters) {
    if (character != space)
    interface.drawChar(y + 1, x + 1, character);
    x++;
    if (x >= WIDTH) {
      x = 0;
      y++;
      if (y >= HEIGHT)
      break;
    }
  }
}

void clientToolkit::handleWin(const std::string& data){
  char winner = data[1];
}

void clientToolkit::handleLose(const std::string& data){
  char loser = data[1];
}

void clientToolkit::handleYes(){}
void clientToolkit::handleNo(){}

std::string clientToolkit::generateData(const int direction, const char character) {
  std::string message(1, static_cast<char>(toServerFlag::data));
  message += std::to_string(direction);
  message += character;
  pad(message);
  return message;
}

std::string clientToolkit::generateInit(const char character){
  std::string message(1, static_cast<char>(toServerFlag::init));
  message += character;
  pad(message);
  return message;
}

std::string clientToolkit::generateReset(){
  std::string message(1, static_cast<char>(toServerFlag::reset));
  pad(message);
  return message;
}

int clientToolkit::startUdp(const std::string& server_ip, const uint16_t server_port, int& udpSock, struct sockaddr_in& serverAddr) {
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  if (getaddrinfo(server_ip.c_str(),nullptr,&hints,&res) != 0) {
    throw std::runtime_error("getaddrinfo failed");
    return EXIT_FAILURE;
  }

  udpSock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (udpSock < 0) {
    freeaddrinfo(res);
    throw std::runtime_error("socket creation failed: " + std::string(strerror(errno)));
    return EXIT_FAILURE;
  }

  serverAddr = *reinterpret_cast<struct sockaddr_in*>(res->ai_addr);
  serverAddr.sin_port = htons(server_port);
  freeaddrinfo(res);
  return EXIT_SUCCESS;
}