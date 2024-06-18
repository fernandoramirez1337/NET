#include "../include/client.hxx"
/* <string> <cstdint> <netinet/in.h> */
#include <arpa/inet.h>
#include <stdexcept>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <netdb.h>
#include <vector>
#include <thread>
#include <ncurses.h>

#ifndef SPACE
  #define SPACE 32
#endif
#ifndef ATSIGN
  #define ATSIGN 64
#endif

int client::start_udp(const std::string& server_ip, const uint16_t server_port) {
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  if (getaddrinfo(server_ip.c_str(),nullptr,&hints,&res) != 0) {
    throw std::runtime_error("getaddrinfo failed");
    return 1;
  }

  udp_sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (udp_sock < 0) {
    freeaddrinfo(res);
    throw std::runtime_error("socket creation failed: " + std::string(strerror(errno)));
    return 1;
  }

  server_addr = *reinterpret_cast<struct sockaddr_in*>(res->ai_addr);
  server_addr.sin_port = htons(server_port);
  addrlen = sizeof(server_addr);
  freeaddrinfo(res);
  return 0;
}

client::client(const std::string& server_ip, const uint16_t server_port) {
  start_udp(server_ip, server_port);
  try_login();
  start_ncurses();
  start();
}

client::~client() {
  close(udp_sock);
}

void client::write() {
  int ch_buffer, direction(0), last_direction(1);
  bool running = true;

  while (running) {     
    ch_buffer = getch();
    switch (ch_buffer) {
      case KEY_UP:
      case 'w':
      case 'W':
        direction = toolkit.directions::UP;
        break;
      case KEY_DOWN:
      case 's':
      case 'S':
        direction = toolkit.directions::DOWN;
        break;
      case KEY_LEFT:
      case 'a':
      case 'A':
        direction = toolkit.directions::LEFT;
        break;
      case KEY_RIGHT:
      case 'd':
      case 'D':
        direction = toolkit.directions::RIGHT;
        break;
      default:
        break;   
    }
    if (direction != last_direction)
      send_to_server(toolkit.generate_data(direction, client_character));
    last_direction = direction;
  }
}

void client::read() {
  char message_type;
  std::string message;
    
  while (true) {
    message.clear();
    message = recv_from_server();
    toolkit.unpad_(message);
    message_type = message[0];
    switch(message_type) {
      case toolkit.from_server_flag::DATA:
        toolkit.handle_data(message);
        break;
      case toolkit.from_server_flag::WIN:
        toolkit.handle_win(message);
        break;
      case toolkit.from_server_flag::LOSE:
        toolkit.handle_lose(message);
        break;
      case toolkit.from_server_flag::YES:
        toolkit.handle_yes();
        break;
      case toolkit.from_server_flag::NO:
        toolkit.handle_no();
        break;    
      default:
        break;
    }
  }
  close(udp_sock);
}

void client::start() {
  session();
}

void client::session() {
  std::thread worker_thread([this](){read();});
  worker_thread.detach();
  write();
}

int client::send_to_server(const std::string& data) {
  return sendto(udp_sock, data.c_str(), MAXLINE, MSG_CONFIRM, reinterpret_cast<const struct sockaddr*>(&server_addr), addrlen);
}

std::string client::recv_from_server() {
  std::vector<char> buffer(MAXLINE);
  ssize_t n = recvfrom(udp_sock, buffer.data(), MAXLINE, MSG_WAITALL, reinterpret_cast<struct sockaddr*>(&server_addr), &addrlen);
  if (n < 0) {
    return "?";
  }
  return std::string(buffer.begin(), buffer.begin() + n);
}

void client::try_login() {
  std::string login_message, response_message;
  char character;

  initscr(); 
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  printw("Type your character (aA-zZ 0-9): ");
  refresh();

  while (true) {
    int ch = getch(); 
    if (ch == ERR) {
      continue;
    }
    character = static_cast<char>(ch);

    printw("\nYour snake: %c\n", character);
    refresh();

    login_message = toolkit.generate_init(character);
    if (send_to_server(login_message) == -1) {
      perror("send");
      break;
    }

    response_message = recv_from_server();
    clear();

    if (response_message[0] == toolkit.from_server_flag::YES) {
      client_character = character;
      toolkit.handle_yes();
      break;  // Successful login, break out of loop
    } else if (response_message[0] == toolkit.from_server_flag::NO) {
      toolkit.handle_no();
      printw("Character already taken! [%c]\n", character);
      printw("Type your character (aA-zZ 0-9): ");
      refresh();  // Refresh screen to display debug message
    } else {
        std::cerr << "Unexpected response from server." << std::endl;
        break;
    }
  }
}


int client::start_ncurses() {
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0);
  timeout(100);
  return 0;
}