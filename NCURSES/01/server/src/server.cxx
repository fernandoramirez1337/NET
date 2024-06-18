#include "../include/server.hxx"
/* <netinet/in.h> <cstdint> <string> <map> <mutex> */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <vector>
#include <thread>
#include <chrono>

int server::start_udp(const uint16_t port) {
  struct sockaddr_in server_addr;

  if ((udp_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("Socket creation failed");
    return EXIT_FAILURE;
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  if (bind(udp_sock, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("Bind failed");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

server::server(const uint16_t port) {
  start_udp(port);
  interface.start_screen();
  start();
}

server::~server() {
  shutdown(udp_sock, SHUT_RDWR);
  close(udp_sock);
}

void server::start() {
  std::thread worker_thread([this](){read();});
  worker_thread.detach();
  write();
}

void server::write() {
  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    interface.update_screen();
    interface.draw_screen();
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
    ssize_t n = recvfrom(udp_sock, buffer.data(), MAXLINE, MSG_WAITALL, (struct sockaddr *)&client_addr, &addr_len);

    if (n == -1) {
      perror("recvfrom error");
      continue;
    }

    if (n > 0) {
      message.clear(); message = buffer.data(); toolkit.unpad_(message);
      char message_type = message[0];

      switch (message_type) {
        case toolkit.from_client_flag::data:
          handle_data(message, client_addr);
          break;
        case toolkit.from_client_flag::init:
          handle_init(message, client_addr);
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

int server::send_to_client(const std::string& message, const struct sockaddr_in& client_addr) {
  if (sendto(udp_sock, message.c_str(), MAXLINE, MSG_CONFIRM, (const struct sockaddr *)&client_addr, sizeof(client_addr)) == -1)
  return EXIT_FAILURE;
  return EXIT_SUCCESS;
}

void server::handle_init(const std::string& data, const struct sockaddr_in& client_addr) {
  char character = data[1];
  bool is_duplicate(true);

  std::lock_guard<std::mutex> lock(clients_mutex);
  is_duplicate = clients.find(character) != clients.end();

  if (is_duplicate) {
    send_to_client(toolkit.generate_no(), client_addr);
  } else {
    clients.emplace(character, client_addr);
    snake client_snake;
    interface.init_snake(client_snake, character);
    interface.snakes.emplace(character, client_snake);
    send_to_client(toolkit.generate_yes(), client_addr);
  }
}

void server::handle_data(const std::string& data, const struct sockaddr_in& client_addr) {
  char new_direction = data[1];
  char user = data[2];
  switch (new_direction) {
    case server_toolkit::directions::UP:
      if (interface.snakes[user].direction.y != 1) interface.snakes[user].direction = {0, -1};
      break;
    case server_toolkit::directions::DOWN:
      if (interface.snakes[user].direction.y != -1) interface.snakes[user].direction = {0, 1};
      break;
    case server_toolkit::directions::LEFT:
      if (interface.snakes[user].direction.x != 1) interface.snakes[user].direction = {-1, 0};
      break;
    case server_toolkit::directions::RIGHT:
      if (interface.snakes[user].direction.x != -1) interface.snakes[user].direction = {1, 0};
      break;
    default:
      break;
  }
}

void server::handle_reset(const std::string& data, const struct sockaddr_in& client_addr) {
  std::cout<< data << std::endl;
}
