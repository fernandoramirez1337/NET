#include <iostream>
#include <ctime>

#include "../client-server.h"

//#define PORT "3490"

class MyClient{
    private:
        std::string username;
        int client_sockFD;
        void receiving_thread();
        void * receive_data();
        void read_and_send_data();
        std::string my_protocol(std::string input);
        bool validate_hash(std::string data, int hash, std::string my_time);
    public:
        void start();
        MyClient(char *, char *, char *);
        ~MyClient();
};

MyClient::MyClient(char * hostname, char * port, char * user){
    username = user;
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
        if ((client_sockFD = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("client: socket");
            continue;
        }
        if (connect(client_sockFD, p->ai_addr, p->ai_addrlen) == -1){
            close(client_sockFD);
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
    std::cout << "type .help for instructions " << std::endl;
    freeaddrinfo(servinfo);
}

MyClient::~MyClient(){
    close(client_sockFD);
}

void MyClient::receiving_thread(){
    std::thread worker_thread([this](){receive_data();});
    worker_thread.detach();
}

void * MyClient::receive_data(){
    char buffer[MAXDATASIZE];
    std::string formatted_buffer;

    while (1)
    {
        ssize_t numbytes;
        if ((numbytes = recv(client_sockFD, buffer, 1, 0)) == -1){
            perror("recv");
            exit(1);
        }
        buffer[numbytes] = 0;

        char message_type = buffer[0];

        if(message_type == 'N'){
            if ((numbytes = recv(client_sockFD, buffer, 2, 0)) == -1){
            perror("recv");
            exit(1);
            }
            buffer[numbytes] = 0;

            int new_user_size = atoi(buffer);
            if ((numbytes = recv(client_sockFD, buffer, new_user_size, 0)) == -1){
            perror("recv");
            exit(1);
            }
            buffer[numbytes] = 0;

            formatted_buffer = buffer;
            formatted_buffer = formatted_buffer + " joined the lobby.";
            std::cout << formatted_buffer << std::endl;
        } else if(message_type == 'M'){
            if ((numbytes = recv(client_sockFD, buffer, 2, 0)) == -1){
            perror("recv");
            exit(1);
            }
            buffer[numbytes] = 0;

            int sender_size = atoi(buffer);
            if ((numbytes = recv(client_sockFD, buffer, sender_size, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            std::string sender = buffer;

            if ((numbytes = recv(client_sockFD, buffer, 3, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            int message_size = atoi(buffer);
            if ((numbytes = recv(client_sockFD, buffer, message_size, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            std::string message = buffer;
            std::cout << "priv from " << sender << ": " << message << std::endl;
        } else if (message_type == 'W'){
            if ((numbytes = recv(client_sockFD, buffer, 3, 0)) == -1){
            perror("recv");
            exit(1);
            }
            buffer[numbytes] = 0;

            int message_size = atoi(buffer);
            if ((numbytes = recv(client_sockFD, buffer, message_size, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            std::cout << buffer << std::endl;
            
        } else if (message_type == 'F'){
            if ((numbytes = recv(client_sockFD, buffer, 2, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            int sender_size = atoi(buffer);

            if ((numbytes = recv(client_sockFD, buffer, sender_size, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            std::string sender = buffer;

            if ((numbytes = recv(client_sockFD, buffer, 5, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            int file_name_size = atoi(buffer);
            if ((numbytes = recv(client_sockFD, buffer, file_name_size, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            std::string file_name = buffer;

            if ((numbytes = recv(client_sockFD, buffer, 10, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            int data_size = atoi(buffer);
            if ((numbytes = recv(client_sockFD, buffer, data_size, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            std::string data = buffer;

            if ((numbytes = recv(client_sockFD, buffer, 10, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            int hash = atoi(buffer);

            if ((numbytes = recv(client_sockFD, buffer, 14, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            std::string sender_time = buffer;
            formatted_buffer = "E" + normalize(std::to_string(sender_size),2) + sender;
            if (validate_hash(data,hash,sender_time) == 0){
                std::cout << "file received from " << sender << std::endl;
                formatted_buffer += "OK";
                std::ofstream outputFile(file_name, std::ios::binary);
                outputFile << data;
                outputFile.close();
            } else {
                std::cout << "file received from " << sender << " was corrupted" <<std::endl;
                formatted_buffer += "ER";
            }

            if (send(client_sockFD, formatted_buffer.c_str(), formatted_buffer.size(), 0) == -1)
                perror("send");


        } else if (message_type == 'H'){
            if ((numbytes = recv(client_sockFD, buffer, 180, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            std::cout << buffer << std::endl;
            
        } else if (message_type == 'L'){
            if ((numbytes = recv(client_sockFD, buffer, 3, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            int list_size = atoi(buffer);
            if ((numbytes = recv(client_sockFD, buffer, list_size, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            std::cout << buffer << std::endl;

        } else if (message_type == 'Q'){
            if ((numbytes = recv(client_sockFD, buffer, 2, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            int quit_size = atoi(buffer);
            if ((numbytes = recv(client_sockFD, buffer, quit_size, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;

            std::cout << buffer << " left the lobby." << std::endl;
        } else if (message_type == 'E'){
            if ((numbytes = recv(client_sockFD, buffer, 2, 0)) == -1){
                perror("recv");
                exit(1);
            }
            buffer[numbytes] = 0;
            std::string flag = buffer;
            if (flag == "ER"){
                formatted_buffer = "Operation failed.";
            } else if (flag == "OK"){
                formatted_buffer = "Operation was successful";
            }
            std::cout << formatted_buffer << std::endl;

        }

        if(numbytes==0)
            break;
    }

    close(client_sockFD);
}

void MyClient::read_and_send_data(){
    std::cout << std::endl;
    std::string buffer, formatted_buffer;

    buffer = "N" + normalize(std::to_string(username.size()),2) + username;

    if (send(client_sockFD, buffer.c_str(), buffer.size(), 0) == -1)
        perror("send");
    
    do{
        std::getline(std::cin, buffer);

        formatted_buffer.clear();

        formatted_buffer = my_protocol(buffer);

        if (send(client_sockFD, formatted_buffer.c_str(), formatted_buffer.size(), 0) == -1)
            perror("send");
    }while(buffer != ".exit");

}

void MyClient::start(){
    receiving_thread();
    read_and_send_data();
}

std::string MyClient::my_protocol(std::string input) {
    std::string output;
    if (input == ".help") {                         //.help command
        output = "H00";
    } else if (input == ".list"){                   //.list command
        output = "L00";
    } else if (input == ".exit"){                   //.exit command
        output = "Q00";
    } else if (input.substr(0,5) == ".file"){       //.file command
        size_t last_space = input.find_last_of(' '); 
        size_t prev_space = input.find_last_of(' ', last_space - 1);
        std::string receiver = input.substr(last_space + 1);
        std::string file_name = input.substr(prev_space + 1, last_space - prev_space - 1);
        std::string receiver_size = normalize(std::to_string(receiver.size()), 2);
        std::string file_name_size = normalize(std::to_string(file_name.size()), 5);
        std::time_t current_time = std::time(nullptr);
        std::string time_string = std::ctime(&current_time);
        if (time_string.length() > 14) {
            time_string = time_string.substr(time_string.length() - 14);
        }

        std::ifstream file(file_name, std::ios::binary);

        if (!file.is_open()) {
            std::cerr << "Failed to open the file." << std::endl;
        }

        char character; int hash = 0; std::string file_data;
        while (file.get(character)){
            file_data += character;
            hash += static_cast<int>(character);
        }
        file.close();
        std::string file_data_size = normalize(std::to_string(file_data.size()),10);

        for (char c : time_string) {
            hash += static_cast<int>(c);
        }
    
        output = "F" + receiver_size + receiver + file_name_size + file_name + file_data_size + file_data + normalize(std::to_string(hash),10) + time_string;
        
    } else if (input.substr(0,1) == "."){           //private message command
        size_t space = input.find(' ');
        std::string receiver = input.substr(1, space -1);
        std::string receiver_size = normalize(std::to_string(receiver.size()),2);
        std::string message = input.substr(space + 1);
        std::string message_size = normalize(std::to_string(message.size()),3);
        
        output = "M" + receiver_size + receiver + message_size + message;
    } else {                                        //public message command
        std::string message = input;
        std::string message_size = normalize(std::to_string(message.size()),3);
        
        output = "W" + message_size + message;
    }
    return output;
}

bool MyClient::validate_hash(std::string data,int hash,std::string my_time){
    int sum = 0;
    for(char character : data){
        sum += static_cast<int>(character);
    }
    for(char character : my_time){
        sum += static_cast<int>(character);
    }
    if (hash == sum){
        return 0;
    } return 1;
}

int main(int argc, char *argv[]){

    if (argc != 4){
        std::cerr << "usage: client hostname port username" << std::endl;
        exit(1);
    }

    MyClient client(argv[1],argv[2],argv[3]);
    client.start();

    return 0;
}
