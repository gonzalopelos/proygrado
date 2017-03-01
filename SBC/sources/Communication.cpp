//
// Created by Gonzalo Pelós on 12/18/16.
//

#include "../includes/Communication.h"
#include <cstdio>
#include <strings.h>
//#include <zconf.h>
#include <cstdlib>
#include <unistd.h>
#include <cstring>

Communication * Communication::_communication_instance = NULL;
struct hostent * server;
struct sockaddr_in serv_addr;
int sockfd;

Communication *Communication::get_instance() {
    if(_communication_instance == NULL){
        _communication_instance = new Communication();
    }
    return _communication_instance;
}

int Communication::send_all(char *data, int length) {
    int result = 0;
    do{
        result += (int) write(sockfd, data, (size_t) length);
        if(result < 0) {
            perror("ERROR writing data in socket");
            close(sockfd);
            init_eth_interface();
        }
    } while(result < length);

    return result;
}

int Communication::receive(char *data, int max_length) {
    int result = 0;
    do {
        // 2017.02.28 AM - Cambio bzero por memset.
        //bzero(data, sizeof(*data));
        memset(data, 0, sizeof(*data));
        result = (int) read(sockfd, data, (size_t) max_length);
        if (result < 0) {
            perror("ERROR reading from socket");
            close(sockfd);
            init_eth_interface();
        }
    } while (result < 0);

//    printf("Comm::recievied data: %s\n", data);

    return result;
}

Communication::~Communication() {
    free(_communication_instance);
}

Communication::Communication() {
    init_eth_interface();
}

void Communication::init_eth_interface() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        perror("ERROR opening socket");
    }
    server = gethostbyname("192.168.1.52");//gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
    }

    // 2017.02.28 AM - Cambio bzero por memset.
    //bzero((char *) &serv_addr, sizeof(serv_addr));
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    // 2017.02.28 AM - Cambio bcopy por memcpy.
    //bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
    serv_addr.sin_port = htons(ETH_COMMUNICATION_SERVER_PORT);


    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
    }
}
