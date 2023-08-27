#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <malloc.h>

#include <pthread.h>

//#define PORT "3490"

#define BACKLOG 10 

#define MAXDATASIZE 100

struct acceptedSocket{
    int sockFD;
    struct sockaddr_storage addr;
};

//get sockaddr, IPv4 or IPv6:
void *get_in_adrr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

struct acceptedSocket * accept_connection(int _sockFD);
void * receive_data(void * void_sockFD);
void start_accepting_connections(int _sockFD);
void receive_sock(struct acceptedSocket *p_socket);
void send_other_clients(char *buffer,int _sockFD);

struct acceptedSocket acceptedSockets[BACKLOG];
int acceptedSocketCount = 0;

struct acceptedSocket * accept_connection(int _sockFD){
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size = sizeof client_addr;
    int new_sockFD = accept(_sockFD, (struct sockaddr *)&client_addr, &client_addr_size);
    if (new_sockFD == -1){
        perror("accept");
    }

    struct acceptedSocket* accepted_sockFD = malloc(sizeof (struct acceptedSocket));
    accepted_sockFD->addr = client_addr;
    accepted_sockFD->sockFD = new_sockFD;

    return accepted_sockFD;
}

void* receive_data(void *void_sockFD){
    int _sockFD = (int)void_sockFD;
    char buf[MAXDATASIZE];

    while(1){
        ssize_t numbytes;
        if ((numbytes = recv(_sockFD, buf, MAXDATASIZE-1, 0)) == -1){
            perror("recv");
            exit(1);
        }

        if (numbytes>0){
            buf[numbytes] = 0;
            printf("%s\n",buf);

            send_other_clients(buf, _sockFD);
        }

        if(numbytes==0)
            break;
    }
    close(_sockFD);
}

void start_accepting_connections(int _sockFD){
    while (1){
        struct acceptedSocket* new_sockFD = accept_connection(_sockFD);
        acceptedSockets[acceptedSocketCount++] = *new_sockFD;

        receive_sock(new_sockFD);
    }
}

void receive_sock(struct acceptedSocket *p_socket){
    pthread_t id;
    pthread_create(&id, NULL, receive_data, (void *)p_socket->sockFD);
}

void send_other_clients(char *buf, int _sockFD){
    for (int i = 0; i<acceptedSocketCount; i++){
        if (acceptedSockets[i].sockFD != _sockFD){
            if (send(acceptedSockets[i].sockFD, buf, MAXDATASIZE-1, 0) == -1)
                perror("send");
        }
    }
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
        fprintf(stderr, "usage: server port\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; 

    if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
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
        fprintf(stderr, "server: failed to bind\n");
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

    printf("server: waiting for connections...      PORT = %s\n", argv[1]);

    start_accepting_connections(sockFD);

    shutdown(sockFD, SHUT_RDWR);

    return 0;
}