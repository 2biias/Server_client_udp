#include<sys/socket.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<netdb.h>
#include<string>
#include<string.h>
#include<iostream>
#include<unistd.h>
#include<arpa/inet.h>
#include<vector>
#include<fstream>

#define MSGLEN 1

void error(const char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[])
{

    if(argc < 4){
        std::cout << "Usage: " << argv[0] << " hostname port U/L\nU: Get uptime.\nL: Get CPU-load" << std::endl;
        exit(0);
    }

    //Chech if U(u) or L(l)
    char req[MSGLEN+1];
    req[0] = argv[3][0];
    if(!(req[0] == 'U' || req[0] == 'u' || req[0] == 'L' || req[0] == 'l')){
        error("Bad request");
    }

    //Setting port number, socketfd, host entry and address struct
    int portno = atoi(argv[2]), udp_client_fd = 0, bytes=0;
    struct sockaddr_in server_address;
    int addresslen = sizeof(server_address);

    //Setting up UDP socket
    if((udp_client_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        error("ERROR opening socket");

    //IPV4 address, and port of server
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(portno);

    //Converting hostname to address
    if(inet_pton(AF_INET, argv[1], &server_address.sin_addr) < 0){
        error("ERROR converting hostname to address type");
    }

    //Sending request to server
    if((bytes = sendto(udp_client_fd, req, MSGLEN, MSG_CONFIRM,
                      (struct sockaddr*)&server_address, addresslen)) < 0){
        error("Error sending request");
    }

    //Listening for reply
    char rep[256];
    if((bytes = recvfrom(udp_client_fd, rep, 255, MSG_WAITALL,
                         (struct sockaddr*)&server_address, (socklen_t*)&addresslen)) < 0){
        error("Error getting reply");
    }

    std::cout << "Reply from server:\n" << (req[0] == 'U' || req[0] == 'u' ? "Uptime: ": "CPU-load: ")
              << rep << std::endl;
}
