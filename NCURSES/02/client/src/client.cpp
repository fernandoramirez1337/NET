#include "../include/client.hpp"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <iostream>
#include <unistd.h>
#include <cstdint>
#include <cstring>
#include <netdb.h>
#include <vector>
#include <thread>
#include <string>

client::client(const std::string& server_ip, const uint16_t server_port) {
  toolkit.startUdp(server_ip, server_port, udpSock, serverAddr);
  addrLen = sizeof(serverAddr);
  interface.startScreen();
  tryLogin();
  startSession();
}

client::~client() {
  close(udpSock);
}

void client::write() {
  int ch_buffer, direction(0), last_direction(1);
  bool running = true;

  while (running) {     
    ch_buffer = interface.getChar();
    switch (ch_buffer) {
      case 0403: //ncurses
        direction = toolkit.directions::UP;
        break;
      case 0402:
        direction = toolkit.directions::DOWN;
        break;
      case 0405:
        direction = toolkit.directions::LEFT;
        break;
      case 0404:
        direction = toolkit.directions::RIGHT;
        break;
      default:
        break;   
    }
    //if (direction != last_direction)
    sendToServer(toolkit.generateData(direction, clientCharacter));
    last_direction = direction;
  }
}

void client::read() {
  char message_type;
  std::string message;
    
  while (true) {
    message.clear();
    message = recvFromServer();
    toolkit.unpad(message);
    message_type = message[0];
    switch(message_type) {
      case toolkit.fromServerFlag::DATA:
        interface.clearScreen();
        interface.drawBorder(HEIGHT,WIDTH);
        toolkit.handleData(message);
        interface.refreshScreen();
        break;
      case toolkit.fromServerFlag::WIN:
        toolkit.handleWin(message);
        break;
      case toolkit.fromServerFlag::LOSE:
        toolkit.handleLose(message);
        break;
      case toolkit.fromServerFlag::YES:
        toolkit.handleYes();
        break;
      case toolkit.fromServerFlag::NO:
        toolkit.handleNo();
        break;    
      default:
        break;
    }
  }
  close(udpSock);
}

void client::startSession() {
  std::thread worker_thread([this](){write();});
  worker_thread.detach();
  read();
}

int client::sendToServer(const std::string& data) {
  return sendto(udpSock, data.c_str(), MAXLINE, MSG_CONFIRM, reinterpret_cast<const struct sockaddr*>(&serverAddr), addrLen);
}

std::string client::recvFromServer() {
  std::vector<char> buffer(MAXLINE);
  ssize_t n = recvfrom(udpSock, buffer.data(), MAXLINE, MSG_WAITALL, reinterpret_cast<struct sockaddr*>(&serverAddr), &addrLen);
  if (n < 0) {
    return "?";
  }
  return std::string(buffer.begin(), buffer.begin() + n);
}

void client::tryLogin() {
  std::string login_message, response_message;
  char character;

  interface.printText("Type your character (aA-zZ 0-9): ");

  interface.refreshScreen();

  while (true) {
    int ch = interface.getChar(); 
    if (ch == -1) {
      continue;
    }
    character = static_cast<char>(ch);

    interface.refreshScreen();

    login_message = toolkit.generateInit(character);
    if (sendToServer(login_message) == -1) {
      perror("send");
      break;
    }

    response_message = recvFromServer();
    interface.clearScreen();

    if (response_message[0] == toolkit.fromServerFlag::YES) {
      clientCharacter = character;
      toolkit.handleYes();
      break;
    } else if (response_message[0] == toolkit.fromServerFlag::NO) {
      toolkit.handleNo();
      interface.printText("Character already taken!\n");
      //interface.printText(&character);
      interface.printText("Type your character (aA-zZ 0-9): ");
      interface.refreshScreen();
    } else {
        std::cerr << "Unexpected response from server." << std::endl;
        break;
    }
  }
}



