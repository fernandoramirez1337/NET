#include "../include/net_utilities.hpp"

#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <string>

std::string net::format_size(const int size, int n) {
    std::ostringstream oss;
    oss << std::setw(n) << std::setfill('0') << size;
    return oss.str();
}

std::vector<std::vector<unsigned char>> net::readAndDivideFile(const std::string& filename, size_t chunkSize) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return {};
    }

    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<unsigned char> fileData(fileSize);
    file.read(reinterpret_cast<char*>(fileData.data()), fileSize);
    std::vector<std::vector<unsigned char>> chunks;

    size_t offset = 0;
    while (offset < fileSize) {
        size_t remainingSize = fileSize - offset;
        size_t chunkSizeBytes = std::min(remainingSize, chunkSize);
        
        std::vector<unsigned char> chunk(fileData.begin() + offset, fileData.begin() + offset + chunkSizeBytes);
        chunks.push_back(std::move(chunk));

        offset += chunkSizeBytes;
    }
    return chunks;
}
// E##
std::string net::PROTOCOL::ErrorMessage(int err_n) {
    if (err_n < 0 || err_n > 99) {
        return "E00";
    }
    std::ostringstream oss;
    oss << "E" << std::setw(2) << std::setfill('0') << err_n;
    return oss.str();
}
// O
std::string net::PROTOCOL::OkMessage(){
    std::ostringstream oss;
    oss << "O";
    return oss.str();
}
// L##USERNAME##PASSWORD
std::string net::PROTOCOL::LoginMessage(std::string username, std::string password){
    std::ostringstream oss;
    oss << "L" << net::format_size(username.size(), 2) << username << net::format_size(password.size(), 2) << password;
    return oss.str();
}
// U
std::string net::PROTOCOL::LogoutMessage(){
    std::ostringstream oss;
    oss << "U";
    return oss.str();
}
// T
std::string net::PROTOCOL::ListMessage(){
    std::ostringstream oss;
    oss << "T";
    return oss.str();
}
// B##MSG
std::string net::PROTOCOL::BroadcastMessage(std::string msg){
    std::ostringstream oss;
    oss << "B" << net::format_size(msg.size(), 2) << msg;
    return oss.str();
}
// M##RECEIVER##MSG
std::string net::PROTOCOL::PrivateMessage(std::string msg, std::string receiver){
    std::ostringstream oss;
    oss << "M" << net::format_size(receiver.size(), 2) << receiver << net::format_size(msg.size(), 2) << msg;
    return oss.str();
}
// F##FILENAME####RECEIVERFILE
std::vector<std::string> net::PROTOCOL::FileMessages(std::string file_name, std::string receiver) {
    std::vector<std::string> messages;
    const int chunkSize = 1024;
    std::vector<std::vector<unsigned char>> chunks = net::readAndDivideFile(file_name, chunkSize);
    std::ostringstream oss;

    for (size_t i = 0; i < chunks.size(); ++i) {
        oss.str("");
        oss << "F" << net::format_size(file_name.size(), 2) << file_name
            << net::format_size(chunks[i].size(), 15)
            << net::format_size(receiver.size(), 2) << receiver;

        for (unsigned char val : chunks[i]) {
            oss << val;
        }
        messages.push_back(oss.str());
    }
    return messages;
}
//G##
std::string net::PROTOCOL::TicTacToeMessage(std::string msg){
    std::ostringstream oss;
    if (msg.size() == 2) {
        oss << "G" << msg;
    } else { oss << ""; }
    return oss.str();
}
