#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <pthread.h>

//#define PORT "3490"

#define MAXDATASIZE 100

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void start_listening(int _sockFD);
void * receive_data(void * _sockFD);
void read_send(int _sockFD, char * user);

void start_listening(int _sockFD) {

    pthread_t id ;
    pthread_create(&id,NULL,receive_data,(void *)_sockFD);
}

void * receive_data(void * void_sockFD) {
    int _sockFD = (int)void_sockFD;
    char buf[MAXDATASIZE];

    while (1)
    {
        ssize_t numbytes;
        if ((numbytes = recv(_sockFD, buf, MAXDATASIZE-1, 0)) == -1){
            perror("recv");
            exit(1);
        }

        if(numbytes>0)
        {
            buf[numbytes] = 0;
            printf("%s\n", buf);
        }

        if(numbytes==0)
            break;
    }

    close(_sockFD);
}

void read_send(int _sockFD, char * user) {

    char *line = NULL;
    size_t line_size= 0;
    printf("\n");


    char buf[MAXDATASIZE];

    sprintf(buf,"%s joined the room",user);
    if (send(_sockFD, buf, MAXDATASIZE-1, 0) == -1)
        perror("send");

    while(1)
    {
        ssize_t  char_count = getline(&line,&line_size,stdin);
        line[char_count-1]=0;

        sprintf(buf,"%s: %s",user,line);

        if(char_count>0)
        {
            if(strcmp(line,"end")==0){
                sprintf(buf,"%s left the room",user);
                if (send(_sockFD, buf, MAXDATASIZE-1, 0) == -1)
                    perror("send");
                break;
            }

            if (send(_sockFD, buf, MAXDATASIZE-1, 0) == -1)
                    perror("send");
        }
    }
}

int main(int argc, char *argv[]){
    int sockFD;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char server[INET6_ADDRSTRLEN];
    size_t user_len = strlen(argv[3]);
    char * username = malloc(user_len+1);
    strcpy(username, argv[3]);
    

    if (argc != 4){
        fprintf(stderr, "usage: client hostname port username\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
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
        fprintf(stderr, "client:failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), server, sizeof server);
    printf("client: connecting to %s\n", server);

    freeaddrinfo(servinfo);

    start_listening(sockFD);

    read_send(sockFD,username);

    printf("connection ended.\n");
    close(sockFD);
    free(username);

    return 0;
}