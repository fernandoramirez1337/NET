#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <cstdint>
#include <string>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include <sstream>
#include <iomanip>
#include <fstream>

std::string format_size(const int size, int n) {
    std::ostringstream oss;
    oss << std::setw(n) << std::setfill('0') << size;
    return oss.str();
}

class client{
    public:
    client(char *, char *);
    ~client();

    private:
    int sockFD, m;
    bool signal_received = false;
    std::string latest_m_data;
    void *get_in_addr(struct sockaddr *);
    void start_session_();
    void read_();
    void write_();
    int send_message(const std::string&);
};

void *client::get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) { return &(((struct sockaddr_in*)sa)->sin_addr); }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

client::client(char * hostname, char * port){
    addrinfo hints, *servinfo, *p;
    int rv;
    char server[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0){
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
        exit(1);
    }

    for(p = servinfo; p != NULL; p = p->ai_next){
        if ((sockFD = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("client: socket");
            continue;
        }
        if (connect(sockFD, p->ai_addr, p->ai_addrlen) == -1){
            close(sockFD);
            perror("client: connect");
            continue;
        }
        break;
    }
    if (p == NULL){
        std::cerr << "client: failed to connect" << std::endl;
        exit(2);
    }
    inet_ntop(p->ai_family, get_in_addr((sockaddr *)p->ai_addr), server, sizeof server);
    std::cout << "client: connecting to " << server << std::endl;
    freeaddrinfo(servinfo);

    start_session_();
}

client::~client(){
    close(sockFD);
}

void client::start_session_(){
    std::thread worker_thread([this](){read_();});
    worker_thread.detach();
    write_();
}

void client::read_() {
    while (true) {
        char type_buffer[3];
        ssize_t numbytes = read(sockFD, type_buffer, 2);
        type_buffer[numbytes] = '\0';
        if (type_buffer[1] == '1') {
            char size_buffer[6];
            numbytes = read(sockFD, size_buffer, 5);
            size_buffer[numbytes] = '\0';
            int data_size = std::stoi(size_buffer);
            char data_buffer[6];
            numbytes = read(sockFD, data_buffer, data_size);
            data_buffer[numbytes] = '\0';
            m++;
            std::cout << data_buffer << std::endl;
            latest_m_data.clear();
            latest_m_data = std::string(data_buffer,data_size);
        }
        else if (type_buffer[1] == '2') {
            char size_buffer[6];
            numbytes = read(sockFD, size_buffer, 5);
            size_buffer[numbytes] = '\0';
            int data_size = std::stoi(size_buffer);
            char data_buffer[data_size + 1];
            numbytes = read(sockFD, data_buffer, data_size);
            data_buffer[numbytes] = '\0';
        }
        else if (type_buffer[1] == '3') {
            signal_received = true;
            break;
        }
    }
}

int client::send_message(const std::string& message) {
    return send(sockFD, message.c_str(), message.size(), 0);
}

void client::write_() {
    while (true) {
        if (signal_received) {
            std::string one = "01000102321061973";
            send_message(one);

            std::string two = "02";
            two += format_size(m,5);
            two += latest_m_data;
            send_message(two);
            
            std::string three = "03";
            send_message(three);
            break;
        }
    }
}

int main() {
    char* ip = "localhost";
    char* port = "5000";
    client test(ip,port);
    //client profe("51.75.130.125","43210");
    
    return 0;
}
