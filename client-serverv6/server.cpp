#include <iostream>

#include "client-server.h"

#include <sys/wait.h>
#include <signal.h>
#include <map>

#define BACKLOG 10 

void sigchld_handler(int s){
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

class MyServer{
    public:
        MyServer(char *);
        ~MyServer();
        void start();
        
    private: 
        std::map<std::string, int> accepted_sockets;
        int server_sockFD;
        int accept_connection();
        void start_new_thread(int);
        void receive_data(void *);
};

MyServer::MyServer(char * port){
    addrinfo hints, *servinfo, *p;
    struct sigaction sa;
    int yes = 1;
    int rv;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0){
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
        exit(1);
    }
    for(p = servinfo; p != NULL; p = p->ai_next){
        if ((server_sockFD = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("server: socket");
            continue;
        }
        if (setsockopt(server_sockFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
            perror("setsockopt");
            exit(1);
        }
        if (bind(server_sockFD, p->ai_addr, p->ai_addrlen) == -1){
            close(server_sockFD);
            perror("server: bind");
            continue;
        }
        break;
    }
    freeaddrinfo(servinfo);
    if (p == NULL){
        std::cerr << "server: failed to bind" << std::endl;
        exit(1);
    }
    if (listen(server_sockFD, BACKLOG) == -1){
        perror("listen");
        exit(1);
    }
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1){
        perror("sigaction");
        exit(1);
    }
    std::cout << "server: waiting for connections...        PORT = " << port << std::endl; 
}

MyServer::~MyServer(){
    shutdown(server_sockFD, SHUT_RDWR);
}

void MyServer::start(){
    while (1){
        int accepted_sockFD = accept_connection();
        start_new_thread(accepted_sockFD);
    }
}

int MyServer::accept_connection(){
    sockaddr_storage client_addr;
    socklen_t client_addr_size = sizeof client_addr;
    int client_sockFD = accept(server_sockFD, (sockaddr *)&client_addr, &client_addr_size);
    if (client_sockFD == -1){
        perror("accept");
    }
    //accepted_sockets.insert({"default",client_sockFD});
    return client_sockFD;
}

void MyServer::start_new_thread(int _sockFD){
    std::thread worker_thread([this,_sockFD](){receive_data((void *)_sockFD);});
    worker_thread.detach();
}

void MyServer::receive_data(void * void_sockFD){
    int _sockFD = (intptr_t)void_sockFD;
    char buffer[MAXDATASIZE];
    std::string formatted_buffer, console_message, thread_username;
    while(1){
        ssize_t numbytes;
        if ((numbytes = recv(_sockFD, buffer, 1, 0)) == -1){
            perror("recv");
            exit(1);
        }
        buffer[numbytes] = 0;

        char message_type = buffer[0];

        if (message_type == 'N') { //new user N00USERNAME
            if ((numbytes = recv(_sockFD, buffer, 2, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            int username_size = atoi(buffer);
            if ((numbytes = recv(_sockFD, buffer, username_size, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            thread_username = buffer;
            accepted_sockets.insert({thread_username, _sockFD});
            formatted_buffer = "N" + normalize(std::to_string(thread_username.size()),2) + thread_username;
            console_message = thread_username + " joined the lobby";
            std::cout << console_message << std::endl;            

            for(auto it = accepted_sockets.begin(); it != accepted_sockets.end(); it++){
                if (send(it->second, formatted_buffer.c_str(), formatted_buffer.size(), 0) == -1)
                    perror("send"); 
            }

        } else if (message_type == 'W') { //message for all
            if ((numbytes = recv(_sockFD, buffer, 3, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            int message_size = atoi(buffer);

            if ((numbytes = recv(_sockFD, buffer, message_size, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            formatted_buffer = thread_username + ": " + buffer;
            std::cout << formatted_buffer << std::endl;

            formatted_buffer = "W" + normalize(std::to_string(formatted_buffer.size()),3) + formatted_buffer;

            for(auto it = accepted_sockets.begin(); it != accepted_sockets.end(); it++){
                if (it->first != thread_username){
                    if (send(it->second, formatted_buffer.c_str(), formatted_buffer.size(), 0) == -1)
                        perror("send"); 
                }
            }

        } else if( message_type == 'M') { //message for user
            if ((numbytes = recv(_sockFD, buffer, 2, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            int receiver_size = atoi(buffer);

            if ((numbytes = recv(_sockFD, buffer, receiver_size, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            std::string receiver = buffer;

            if ((numbytes = recv(_sockFD, buffer, 3, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            int message_size = atoi(buffer);

            if ((numbytes = recv(_sockFD, buffer, message_size, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            std::string message = buffer;

            formatted_buffer = "M" + normalize(std::to_string(thread_username.size()),2) + thread_username + normalize(std::to_string(message.size()),3) + message;

            if (send(accepted_sockets[receiver], formatted_buffer.c_str(), formatted_buffer.size(),0) == -1)
                perror("send");

            std::cout << "priv from " + thread_username + " to " + receiver + ": " + message << std::endl;
        } else if ( message_type == 'Q') { //exit
            console_message = thread_username + " left the lobby.";
            formatted_buffer = "Q" + normalize(std::to_string(thread_username.size()),2) + thread_username;
            for(auto it = accepted_sockets.begin(); it != accepted_sockets.end(); it++){
                if (it->first != thread_username){
                    if (send(it->second, formatted_buffer.c_str(), formatted_buffer.size(), 0) == -1)
                        perror("send"); 
                }
            }
            accepted_sockets.erase(thread_username);
            std::cout << console_message << std::endl;
            break;

        } else if ( message_type == 'L') { //list of users
            formatted_buffer = "List of online users:";
            for(const auto& pair : accepted_sockets){
                formatted_buffer = formatted_buffer + "\n " + pair.first;
            }
            formatted_buffer = "L" + normalize(std::to_string(formatted_buffer.size()),3) + formatted_buffer;
            if (send(accepted_sockets[thread_username], formatted_buffer.c_str(), formatted_buffer.size(), 0) == -1)
                perror("send"); 

        } else if ( message_type == 'F'){
            if ((numbytes = recv(_sockFD, buffer, 2, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            int receiver_size = atoi(buffer);

            if ((numbytes = recv(_sockFD, buffer, receiver_size, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            std::string receiver = buffer;

            if ((numbytes = recv(_sockFD, buffer, 5, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;
            
            int file_name_size = atoi(buffer);

            if ((numbytes = recv(_sockFD, buffer, file_name_size, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            std::string file_name = buffer;

            if ((numbytes = recv(_sockFD, buffer, 10, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            int data_size = atoi(buffer);

            if ((numbytes = recv(_sockFD, buffer, data_size, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            std::string data = buffer;

            if ((numbytes = recv(_sockFD, buffer, 10, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            int hash = atoi(buffer);

            if ((numbytes = recv(_sockFD, buffer, 14, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            std::string the_time = buffer;

            formatted_buffer = "F" + normalize(std::to_string(thread_username.size()), 2) + thread_username + normalize(std::to_string(file_name_size),5) + file_name + normalize(std::to_string(data_size),10) + data + normalize(std::to_string(hash),10) + the_time;

            if (send(accepted_sockets[receiver], formatted_buffer.c_str(), formatted_buffer.size(), 0) == -1)
                perror("send"); 

        } else if ( message_type == 'H'){
            formatted_buffer = "HLIST OF COMMANDS:\nGET HELP: .help\nSEND GLOBAL MESSAGE: <message>\nSEND PRIVATE MESSAGE: .<receiver> <message>\nGET USERS: .list\nSEND FILES: .file <file_name> <receiver>\nEXIT: .exit";
            if (send(accepted_sockets[thread_username], formatted_buffer.c_str(), formatted_buffer.size(), 0) == -1)
                perror("send"); 
        } else if (message_type == 'E'){
            
            if ((numbytes = recv(_sockFD, buffer, 2, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            int sender_size = atoi(buffer);

            if ((numbytes = recv(_sockFD, buffer, sender_size, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            std::string sender = buffer;

            if ((numbytes = recv(_sockFD, buffer, 2, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;
            
            std::string flag = buffer;
            formatted_buffer = "E" + flag;
            if (flag == "OK"){
                std::cout << "file transfer was successful" << std::endl;
            } else if (flag == "ER"){
                std::cout << "file transfer failed :(" << std::endl;
            }
            if (send(accepted_sockets[sender], formatted_buffer.c_str(), formatted_buffer.size(), 0) == -1)
                perror("send"); 

        }
        formatted_buffer.clear();
    }

}


int main(int argc, char *argv[]){

    if (argc != 2){
        std::cerr << "usage: server port" << std::endl;
        exit(1);
    }
    MyServer server(argv[1]);
    server.start();
    //start_accepting_connections(sockFD);

    return 0;
}