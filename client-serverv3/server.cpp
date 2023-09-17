#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include <pthread.h>

#include <vector>
#include <cstdint>
#include <string>
#include <map>

//#define PORT "3490"

#define BACKLOG 10 

#define MAXDATASIZE 100

std::map<std::string, int> aS;

int findNthOccur(std::string str,
                 char ch, int N)
{
    int occur = 0;
 
    // Loop to find the Nth
    // occurrence of the character
    for (int i = 0; i < str.length(); i++) {
        if (str[i] == ch) {
            occur += 1;
        }
        if (occur == N)
            return i;
    }
    return -1;
}

//get sockaddr, IPv4 or IPv6:
void *get_in_adrr(sockaddr *sa){
    if (sa->sa_family == AF_INET){
        return &(((sockaddr_in*)sa)->sin_addr);
    }
    return &(((sockaddr_in6*)sa)->sin6_addr);
}

// receives the servers listening socket and establishes incoming connections
std::string accept_connection(int _sockFD);
// thread function, receives incoming data from clients
void * receive_data(void * void_sockFD);
// servers main function
void start_accepting_connections(int _sockFD);
//  starts thread with last accepted connection
void start_new_thread(std::string new_user);
// sends received data to other clients
void send_data_except(char * buffer);
// sends received data to one client
void send_data_only(char * buffer);
// removes connection from vector
void remove_connection(int _sockFD);
void decide(char * buffer);


std::string accept_connection(int _sockFD){
    sockaddr_storage client_addr;
    socklen_t client_addr_size = sizeof client_addr;
    int client_sockFD = accept(_sockFD, (struct sockaddr *)&client_addr, &client_addr_size);
    if (client_sockFD == -1){
        perror("accept");
    }

    char buffer[MAXDATASIZE];
    ssize_t numbytes;
    if ((numbytes = recv(client_sockFD, buffer, MAXDATASIZE-1, 0)) == -1){
            perror("recv");
            exit(1);
        }
    buffer[numbytes] = 0;
    std::string formatted_buffer = buffer, delimiter = " ";

    std:: string username = formatted_buffer.substr(0, formatted_buffer.find(delimiter));
    std::cout << formatted_buffer << std::endl;
    aS.insert({username, client_sockFD});
    for (auto it = aS.begin(); it != aS.end(); it++){
        if (send(it->second, formatted_buffer.c_str(), MAXDATASIZE-1, 0) == -1)
            perror("send");
    }
    return username;

}

void* receive_data(void *void_sockFD){
    int _sockFD = (intptr_t)void_sockFD;
    char buffer[MAXDATASIZE];
    std::string formatted_buffer, console_message;

    while(formatted_buffer != "end"){
        ssize_t numbytes;
        if ((numbytes = recv(_sockFD, buffer, MAXDATASIZE-1, 0)) == -1){
            perror("recv");
            exit(1);
        }
        formatted_buffer.clear();
        formatted_buffer = buffer;

        if (numbytes>0){
 
            std::cout << formatted_buffer << std::endl;
            decide(buffer);
        }

        if(numbytes==0)
            break;
    }

    remove_connection(_sockFD);
}

void start_accepting_connections(int _sockFD){
    while (1){
        std::string new_user = accept_connection(_sockFD);

        start_new_thread(new_user);
    }
}

void start_new_thread(std::string new_user){
    pthread_t id;
    pthread_create(&id, NULL, receive_data, (void *)aS[new_user]);
}

void send_data_except(char * buf){
    
    std::string formatted_buffer = buf;
    std::string delimiter = ": ";
    std::string username = formatted_buffer.substr(0, formatted_buffer.find(delimiter));
    for (auto it = aS.begin(); it != aS.end(); it++){
        if (it->first !=  username){
            if (send(it->second, buf, MAXDATASIZE-1, 0) == -1)
                perror("send");
        }
    }
}

void send_data_only(char * buf){
    std::string formatted_buffer = buf;
    std::string delimiter1 = ": .";
    std::string delimiter2 = " ";
    std::string sender = formatted_buffer.substr(0, formatted_buffer.find(delimiter1));
    std::string username = formatted_buffer.substr(sender.size()+3, findNthOccur(formatted_buffer, ' ', 2)-sender.size()-3);
    std::string message = formatted_buffer.substr(sender.size()+username.size()+4, formatted_buffer.size()); 
    formatted_buffer = "from " + sender + ": " + message;
    //std::cout << sender <<"."<< username <<  "." << message << std::endl;
    if (send(aS[username], formatted_buffer.c_str(), MAXDATASIZE-1, 0) == -1)
                perror("send");
    
}

void decide(char * buf){
    std::string formatted_buffer = buf;
    std::string temp = ": .";
        if (formatted_buffer.find(temp) != std::string::npos){
            send_data_only(buf);
        } else {
            send_data_except(buf);
        }
}

void remove_connection(int _sockFD){

    

}

void sigchld_handler(int s){
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

int main(int argc, char *argv[]){
    int sockFD, numbytes;
    struct addrinfo hints, *servinfo, *p;
    struct sigaction sa;    
    int yes = 1;
    int rv;

    if (argc != 2){
        std::cerr << "usage: server port" << std::endl;
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; 

    if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0){
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
        return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next){
        if ((sockFD = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("server: socket");
            continue;
        }
        if (setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
            perror("setsockopt");
            exit(1);
        }
        if (bind(sockFD, p->ai_addr, p->ai_addrlen) == -1){
            close(sockFD);
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

    if (listen(sockFD, BACKLOG) == -1){
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

    std::cout << "server: waiting for connections...      PORT = " << argv[1] << std::endl;

    start_accepting_connections(sockFD);

    shutdown(sockFD, SHUT_RDWR);

    return 0;
}