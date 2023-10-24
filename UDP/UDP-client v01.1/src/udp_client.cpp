#include "../include/udp_client.hpp"

#include <iostream>

#include <string>
#include <cstring>

#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>

#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)

udp_client::udp_client(char * port , char * username){
    this->username = username;
    struct sockaddr_in this_addr;
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        perror("socket");
        exit(1);
    }
    this_addr.sin_family = AF_INET;
    this_addr.sin_port = htons(atoi(port));
    this_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(this_addr.sin_zero), 8);
    if (bind(sock, (struct sockaddr *)&this_addr, sizeof(struct sockaddr)) == -1){
        perror("Bind");
        exit(1);
    }
    std::cout << "Waiting for messages in port " << port << std::endl << std::endl;
}

udp_client::~udp_client(){
    close(sock);
}

void udp_client::receive_thread(){
    std::thread worker_thread([this](){receive();});
    worker_thread.detach();
}

void * udp_client::receive(){
    char buffer[MAXDATASIZE];
    std::string formatted_buffer;
    struct sockaddr_in tmp_addr;
    socklen_t addr_len = sizeof(tmp_addr);
    while (1){
        ssize_t numbytes;
        if ((numbytes = recvfrom(sock, buffer, MAXDATASIZE, 0, (struct sockaddr *)&tmp_addr, &addr_len)) == -1){
            perror("recvfrom");
            exit(1);
        }
        buffer[numbytes] = 0;
        formatted_buffer = buffer;
        udp_util::udp_datagram received(formatted_buffer);
        std::cout << "(" << received.username <<"@"<< inet_ntoa(tmp_addr.sin_addr) << ":" << ntohs(tmp_addr.sin_port) << "):  " << received.payload << std::endl;
    }
}

void udp_client::send(){
    char buffer[MAXDATASIZE];
    std::cout << std::endl;
    std::string formatted_buffer, message, port;
    struct sockaddr_in receiver_addr;
    receiver_addr.sin_family = AF_INET;
    //receiver_addr.sin_port = htons(5000);
    receiver_addr.sin_addr.s_addr = INADDR_ANY;
    do{
        std::getline(std::cin, formatted_buffer);
        size_t wall = formatted_buffer.find_first_of(' ');
        port = formatted_buffer.substr(0, wall);
        message = formatted_buffer.substr(wall + 1, formatted_buffer.size());
        receiver_addr.sin_port = htons(std::stoi(port));
        formatted_buffer.clear();
        formatted_buffer = protocol(message);
        if(sendto(sock, formatted_buffer.c_str(), MAXDATASIZE, 0,(struct sockaddr *)&receiver_addr, sizeof(struct sockaddr)) == -1) perror("sendto");
    }   while(buffer != ".exit");
}

void udp_client::start(){
    receive_thread();
    send();
}

std::string udp_client::protocol(std::string input){
    std::time_t current_time = std::time(nullptr);
    std::string output, payload = input, free(94,'x'), time_string = std::ctime(&current_time);
    if (time_string.length() > 14) time_string = time_string.substr(time_string.length() - 14);
    output = "MS00" + udp_util::normalize(std::to_string(username.size()), 2, '0') + udp_util::normalize(username, 8, ' ') + udp_util::normalize(std::to_string(payload.size()), 4, '0') + udp_util::normalize(payload, 888, ' ') + free + udp_util::normalize(std::to_string(udp_util::checksum(payload)),10, '0') + time_string;
    return output;
}