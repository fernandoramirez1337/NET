#include "../include/server_toolkit.hpp"
/* <netinet/in.h> <cstdint> <string> <map> */

int serverToolkit::pad(std::string& message) {
  ssize_t message_size = message.size();
  if (message_size >= MAXLINE) return 1;
  message.append(MAXLINE - message_size, PAD);
  return 0;
}

int serverToolkit::unpad(std::string& message) {
  ssize_t message_end = message.find_last_not_of(PAD);
  if (message_end == std::string::npos) return 1; 
  message.resize(message_end + 1);
  return 0;
}

std::string serverToolkit::generateData(board& ss){
  std::string message(1, static_cast<char>(toClientFlag::DATA));
  for (const auto& row : ss) 
  for (const auto& ch : row)
  message += ch;
  
  pad(message);
  return message;
}

std::string serverToolkit::generateWin(char character){
  std::string message(1, static_cast<char>(toClientFlag::WIN));
  message += character;
  pad(message);
  return message;
}

std::string serverToolkit::generateLose(char character){
  std::string message(1, static_cast<char>(toClientFlag::LOSE));
  message += character;
  pad(message);
  return message;
}
std::string serverToolkit::generateYes(){
  std::string message(1, static_cast<char>(toClientFlag::YES));
  pad(message);
  return message;
}
std::string serverToolkit::generateNo(){
  std::string message(1, static_cast<char>(toClientFlag::NO));
  pad(message);
  return message;
}