#include "../include/server.hpp"
#include <netinet/in.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <cstdint> 
#include <cerrno>
#include <vector>
#include <thread>
#include <chrono>
#include <string> 
#include<mutex>
#include <map> 

int server::startUdp(const uint16_t port) {
  struct sockaddr_in server_addr;

  if ((udpSock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("Socket creation failed");
    return EXIT_FAILURE;
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  if (bind(udpSock, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("Bind failed");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

server::server(const uint16_t port) {
  startUdp(port);
  interface.startScreen();
  start();
}

server::~server() {
  shutdown(udpSock, SHUT_RDWR);
  close(udpSock);
}

void server::start() {
  std::thread worker_thread([this](){read();});
  worker_thread.detach();
  write();
}

void server::write() {
  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    interface.updateScreen();
    interface.drawScreen();
    broadcast();
  }
}

void server::read() {
  struct sockaddr_in client_addr; socklen_t addr_len; 
  std::vector<char> buffer(MAXLINE);
  std::string message;

  while (true) {
    memset(&client_addr, 0, sizeof(client_addr));
    addr_len = sizeof(client_addr);
    buffer.clear();
    ssize_t n = recvfrom(udpSock, buffer.data(), MAXLINE, MSG_WAITALL, (struct sockaddr *)&client_addr, &addr_len);

    if (n == -1) {
      perror("recvfrom error");
      continue;
    }

    if (n > 0) {
      message.clear(); message = buffer.data(); toolkit.unpad(message);
      char message_type = message[0];

      switch (message_type) {
        case toolkit.fromClientFlag::data:
          handleData(message, client_addr);
          break;
        case toolkit.fromClientFlag::init:
          handleInit(message, client_addr);
          break;
        //case toolkit.from_client_flag::reset:
        //  handle_reset(message, client_addr);
        //  break;
        default:
          break;
      }
    }
  }
}

int server::sendToClient(const std::string& message, const struct sockaddr_in& client_addr) {
  if (sendto(udpSock, message.c_str(), MAXLINE, MSG_CONFIRM, (const struct sockaddr *)&client_addr, sizeof(client_addr)) == -1)
  return EXIT_FAILURE;
  return EXIT_SUCCESS;
}

void server::handleInit(const std::string& data, const struct sockaddr_in& client_addr) {
  char character = data[1];
  bool is_duplicate(true);

  std::lock_guard<std::mutex> lock(clientsMutex);
  is_duplicate = clients.find(character) != clients.end();

  if (is_duplicate) {
    sendToClient(toolkit.generateNo(), client_addr);
  } else {
    clients.emplace(character, client_addr);
    spaceship client_spaceship;
    interface.initSpaceship(client_spaceship, character);
    interface.spaceships.emplace(character, client_spaceship);
    sendToClient(toolkit.generateYes(), client_addr);
  }
}

void server::handleData(const std::string& data, const struct sockaddr_in& client_addr) {
  char new_direction = data[1];
  char user = data[2];
  switch (new_direction) {
    case serverToolkit::directions::UP:
      interface.spaceships[user].body.y--;
      break;
    case serverToolkit::directions::DOWN:
      interface.spaceships[user].body.y++;
      break;
    case serverToolkit::directions::LEFT:
      interface.spaceships[user].body.x+=2;
      break;
    case serverToolkit::directions::RIGHT:
      interface.spaceships[user].body.x--;
      break;
    default:
      break;
  }
}

void server::handleReset(const std::string& data, const struct sockaddr_in& client_addr) {
  std::cout<< data << std::endl;
}

void server::broadcast() {
  board tmp = interface.getBoard();
  std::string message = toolkit.generateData(tmp);
  for (auto& client : clients)
    sendToClient(message, client.second);
}