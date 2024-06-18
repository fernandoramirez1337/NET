#include "../include/client_toolkit.hxx"
#include <ncurses.h>
/* <string> <cstdint> */

int client_toolkit::pad_(std::string& message) {
  ssize_t message_size = message.size();
  if (message_size >= MAXLINE) return 1;
  message.append(MAXLINE - message_size, PAD);
  return 0;
}

int client_toolkit::unpad_(std::string& message) {
  ssize_t message_end = message.find_last_not_of(PAD);
  if (message_end == std::string::npos) return 1; 
  message.resize(message_end + 1);
  return 0;
}

void client_toolkit::handle_data(const std::string& data) {
    std::string characters = data.substr(1, 800);  // Extract 800 characters starting from the beginning
    int x = 0, y = 0;  // Initialize coordinates

    for (const auto& character : characters) {
        if (character != ' ') {
            mvprintw(y+1, x+1, "%c", character);  // Print character at current coordinates
        }
        x++;  // Move to the next column
        if (x >= 40) {  // If the end of the row is reached, move to the next row
            x = 0;
            y++;
            if (y >= 20) {  // If the end of the screen is reached, break the loop
                break;
            }
        }
    }
}

void client_toolkit::handle_win(const std::string& data){
  char winner = data[1];
}

void client_toolkit::handle_lose(const std::string& data){
  char loser = data[1];
}

void client_toolkit::handle_yes(){}
void client_toolkit::handle_no(){}

std::string client_toolkit::generate_data(const int direction, const char character) {
  std::string message(1, static_cast<char>(to_server_flag::data));
  message += std::to_string(direction);
  message += character;
  pad_(message);
  return message;
}

std::string client_toolkit::generate_init(const char character){
  std::string message(1, static_cast<char>(to_server_flag::init));
  message += character;
  pad_(message);
  return message;
}

std::string client_toolkit::generate_reset(){
  std::string message(1, static_cast<char>(to_server_flag::reset));
  pad_(message);
  return message;
}