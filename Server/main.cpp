#include<iostream>
#include<sys/socket.h>
#include<sys/stat.h>
#include<netinet/in.h>
#include<fcntl.h>
#include<netdb.h>
#include<string>
#include<string.h>
#include<unistd.h>
#include<vector>
#include<fstream>

#define MSGLEN 1
#define MAXCHUNKSIZE 1024

void error(const char* msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[])
{
    if(argc < 2){
        std::cout << "Usage: " << argv[0] << " Port" << std::endl;
        error("Exiting");
    }

    //Setting udp_server_fd, buffer and address struct
    int portno = atoi(argv[1]), udp_server_fd, bytes;
    char buffer[MSGLEN+1];
    struct sockaddr_in server_address, client_address;
    int addresslen = sizeof(server_address);
    int clientlen = sizeof(client_address);

    //Creating udp socket
    if((udp_server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0)
        error("ERROR creating socket");

    //IPV4 address, and port
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(portno);

    //Binding address to socket
    if(bind(udp_server_fd, (struct sockaddr*) &server_address, addresslen) < 0)
        error("Error binding address to socket");


    //Entries for /proc/uptime and /proc/loadavg
    std::vector<char> filebuffer(MAXCHUNKSIZE, 0);
    std::ifstream file;

    //Wait to receive request
    while(true){
        std::cout << "Server wating for request" << std::endl;

        if((bytes = recvfrom(udp_server_fd, (char*)buffer, MSGLEN, MSG_WAITALL,
                             (struct sockaddr*)&client_address, (socklen_t*)&clientlen)) < 0){
            error("Error recieving request from client");
        }

        std::cout << "Server recieved: " << buffer[0] << std::endl;
        char req = buffer[0];

        if(req == 'U' || req == 'u' || req == 'l' || req == 'L'){

            if(req == 'U' || req == 'u'){
                std::cout << "Uptime requested, sending reply to client" << std::endl;
                file.open("/proc/uptime", std::ifstream::in);
            }else{
                std::cout << "CPU-load requested, sending reply to client" << std::endl;
                file.open("/proc/loadavg", std::ifstream::in);
            }

            //Read until newline
            file.getline(filebuffer.data(), MAXCHUNKSIZE);

            std::cout << "Sending " << file.gcount() << " bytes" << std::endl;

            //Send to client
            if((sendto(udp_server_fd, filebuffer.data(), file.gcount(), MSG_CONFIRM, (struct sockaddr*)&client_address, clientlen)) < 0){
                error("Error seding requested file to client");
            }
            file.close();

        }else{
            //Reply with bad response
            char bad_req[] = "Bad request recieved";
            if((sendto(udp_server_fd, bad_req, strlen(bad_req), MSG_CONFIRM,
                       (struct sockaddr*)&client_address, clientlen)) < 0){
                error("Error seding requested file to client");
            }
        }
    }
}
