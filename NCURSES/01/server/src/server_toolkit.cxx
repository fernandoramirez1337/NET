#include "../include/server_toolkit.hxx"
/* <netinet/in.h> <cstdint> <string> <map> */

int server_toolkit::pad_(std::string& message) {
  ssize_t message_size = message.size();
  if (message_size >= MAXLINE) return 1;
  message.append(MAXLINE - message_size, PAD);
  return 0;
}

int server_toolkit::unpad_(std::string& message) {
  ssize_t message_end = message.find_last_not_of(PAD);
  if (message_end == std::string::npos) return 1; 
  message.resize(message_end + 1);
  return 0;
}

std::string server_toolkit::generate_data(){
  std::string message(1, static_cast<char>(to_client_flag::DATA));
  pad_(message);
  return message;
}

std::string server_toolkit::generate_win(char character){
  std::string message(1, static_cast<char>(to_client_flag::WIN));
  message += character;
  pad_(message);
  return message;
}

std::string server_toolkit::generate_lose(char character){
  std::string message(1, static_cast<char>(to_client_flag::LOSE));
  message += character;
  pad_(message);
  return message;
}
std::string server_toolkit::generate_yes(){
  std::string message(1, static_cast<char>(to_client_flag::YES));
  pad_(message);
  return message;
}
std::string server_toolkit::generate_no(){
  std::string message(1, static_cast<char>(to_client_flag::NO));
  pad_(message);
  return message;
}