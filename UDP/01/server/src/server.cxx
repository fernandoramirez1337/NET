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

#define ERR_LOGIN_DUPLICATES 1
#define ERR_LOGIN 2
#define ERR_PRIV 11

udp_server::udp_server(uint16_t port) {
    struct sockaddr_in server_addr;

    // Create socket
    if ((udp_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Prepare server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind socket to address
    if (bind(udp_sock, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server: waiting for connections... PORT = " << port << std::endl;
    start();
}

udp_server::~udp_server() {
    shutdown(udp_sock, SHUT_RDWR);
    close(udp_sock);
}

void udp_server::start() {
    read_write();
}

void udp_server::read_write() {
    struct sockaddr_in client_addr; socklen_t addr_len; 
    std::vector<char> buffer(MAXLINE);

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
            std::string data = toolkit.unpad_message(buffer.data());
            std::cout << "-> " << data << std::endl;
            char message_type = data[0];

            switch (message_type) {
                case toolkit.LOGIN:
                    handle_login(data, client_addr);
                    break;
                case toolkit.OK:
                    handle_ok();
                    break;
                case toolkit.ERROR:
                    handle_error(data);
                    break;
                case toolkit.LOGOUT:
                    handle_logout(data);
                    break;
                case toolkit.LIST:
                    handle_list(client_addr);
                    break;
                case toolkit.BROADCAST:
                    handle_broadcast(data, client_addr);
                    break;
                case toolkit.PRIVATE_MESSAGE:
                    handle_private_message(data, client_addr);
                    break;
                default:
                    std::cerr << "Unknown message type received" << std::endl;
                    break;
            }
        }
    }
}

void udp_server::send_to_client(const std::string& message, const struct sockaddr_in& client_addr) {
    if (sendto(udp_sock, message.c_str(), MAXLINE, MSG_CONFIRM, (const struct sockaddr *)&client_addr, sizeof(client_addr)) == -1) {
        perror("sendto error");
    } else {
        std::cout << "<- " << toolkit.unpad_message(message) << std::endl;
    }
}

void udp_server::handle_login(const std::string& data, const struct sockaddr_in& client_addr) {
    ssize_t username_size, password_size; std::string username, password;
    bool is_duplicate(true);
    username_size = std::stoi(data.substr(1, 2));
    username = data.substr(3, username_size);
    password_size = std::stoi(data.substr(3 + username_size, 2));
    password = data.substr(5 + username_size, password_size);

    std::lock_guard<std::mutex> lock(clients_mutex);
    is_duplicate = clients.find(username) != clients.end();

    if (is_duplicate) {
        send_to_client(toolkit.generate_error_message(ERR_LOGIN_DUPLICATES), client_addr);
    } else {
        clients.emplace(username, client_addr);
        send_to_client(toolkit.generate_ok_message(), client_addr);

        std::string welcome_message = toolkit.generate_login_message(username);
        for (const auto& client : clients) {
            if (client.first != username) {
                send_to_client(welcome_message, client.second);
            }
        }
    }
}

void udp_server::handle_ok() {
    // Handle OK messages if necessary
}

void udp_server::handle_error(const std::string& data) {
    uint8_t error_number = std::stoi(data.substr(1, 2));
    std::cerr << "Received error with number: " << error_number << std::endl;
}

void udp_server::handle_logout(const std::string& data) {
    ssize_t username_size; std::string username;
    username_size = std::stoi(data.substr(1, 2));
    username = data.substr(3, username_size);

    std::lock_guard<std::mutex> lock(clients_mutex);
    auto it = clients.find(username);
    if (it != clients.end()) {
        clients.erase(it);
        std::string goodbye_message = toolkit.generate_logout_message(username);
        for (const auto& client : clients) {
            send_to_client(goodbye_message, client.second);
        }
    }
}

void udp_server::handle_list(const struct sockaddr_in& client_addr) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    send_to_client(toolkit.generate_list_message(clients), client_addr);
}

void udp_server::handle_broadcast(const std::string& data, const struct sockaddr_in& client_addr) {
    ssize_t message_size; std::string message, username;
    message_size = std::stoi(data.substr(1, 2));
    message = data.substr(3, message_size);
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        for (const auto& client : clients) {
            if (client.second.sin_addr.s_addr == client_addr.sin_addr.s_addr &&
                client.second.sin_port == client_addr.sin_port) {
                username = client.first;
                break;
            }
        }
    }
    std::string broadcast_message = toolkit.generate_broadcast_message(username, message);
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (const auto& client : clients) {
        send_to_client(broadcast_message, client.second);
    }
}

void udp_server::handle_private_message(const std::string& data, const struct sockaddr_in& client_addr) {
    ssize_t receiver_size, message_size; std::string receiver, message, sender;
    receiver_size = std::stoi(data.substr(1, 2));
    receiver = data.substr(3, receiver_size);
    message_size = std::stoi(data.substr(3 + receiver_size, 2));
    message = data.substr(5 + receiver_size, message_size);
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        for (const auto& client : clients) {
            if (client.second.sin_addr.s_addr == client_addr.sin_addr.s_addr &&
                client.second.sin_port == client_addr.sin_port) {
                sender = client.first;
                break;
            }
        }
    }
    std::lock_guard<std::mutex> lock(clients_mutex);
    auto receiver_iter = clients.find(receiver);
    if (receiver_iter != clients.end()) {
        send_to_client(toolkit.generate_private_message(sender, message), receiver_iter->second);
    } else {
        send_to_client(toolkit.generate_error_message(ERR_PRIV), client_addr);
    }
}
