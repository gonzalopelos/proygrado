//
// Created by Gonzalo Pelós on 12/18/16.
//

#include "../../include/dm3_core/Tcp_Controller.h"
#include <cstdio>
#include <strings.h>
//#include <zconf.h>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <fcntl.h>/* Added for the nonblocking socket */
#include "../../include/dm3_core/Logger.h"

Tcp_Controller * Tcp_Controller::_Tcp_Controller_instance = NULL;
struct hostent * server;
struct sockaddr_in serv_addr;
int sockfd;

Tcp_Controller *Tcp_Controller::get_instance() {
    if(_Tcp_Controller_instance == NULL){
        _Tcp_Controller_instance = new Tcp_Controller();
    }
    return _Tcp_Controller_instance;
}

int Tcp_Controller::send_all(char *data, int length) {
    int result = 0;
    do{
        result += (int) write(sockfd, data, (size_t) length);
        if(result < 0) {
            Logger::log_error((char*)"ERROR writing data in socket");
            close(sockfd);
            init_eth_interface();
        }
    } while(result < length);

    return result;
}

int Tcp_Controller::receive(char *data, int max_length) {
    int result = 0;
    int receive_timeout = 2; // time out in seconds
    memset(data, 0, sizeof(*data));
    result = recv_to(sockfd, data, 1, 0, 2);
    if (result < 0) {
        if (result == -1){
            Logger::log_error("ERROR reading from socket");
            close(sockfd);
            init_eth_interface();
        }
    }
    
    return result;
}

Tcp_Controller::~Tcp_Controller() {
    if(sockfd>0){
        close(sockfd);
    }
    free(_Tcp_Controller_instance);
}

Tcp_Controller::Tcp_Controller() {
    init_eth_interface();
}

void Tcp_Controller::init_eth_interface() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        Logger::log_error("ERROR opening socket");
    }
    server = gethostbyname(ETH_Tcp_Controller_IP);//gethostbyname(argv[1]);
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
    serv_addr.sin_port = htons(ETH_Tcp_Controller_SERVER_PORT);

    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        Logger::log_error("ERROR connecting");
    }
    // struct timeval timeout;
    // timeout.tv_sec = 1;
    // timeout.tv_usec = 0;
    // setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}

/*
   Params:
      fd       -  (int) socket file descriptor
      buffer - (char*) buffer to hold data
      len     - (int) maximum number of bytes to recv()
      flags   - (int) flags (as the fourth param to recv() )
      to       - (int) timeout in milliseconds
   Results:
      int      - The same as recv, but -2 == TIMEOUT
   Notes:
      You can only use it on file descriptors that are sockets!
      'to' must be different to 0
      'buffer' must not be NULL and must point to enough memory to hold at least 'len' bytes
      I WILL mix the C and C++ commenting styles...
*/
int Tcp_Controller::recv_to(int fd, char *buffer, int len, int flags, int to) {

   fd_set readset;
   int result, iof = -1;
   struct timeval tv;

   // Initialize the set
   FD_ZERO(&readset);
   FD_SET(fd, &readset);
   
   // Initialize time out struct
   tv.tv_sec = 0;
   tv.tv_usec = to * 1000;
   // select()
   result = select(fd+1, &readset, NULL, NULL, &tv);

   // Check status
   if (result < 0){
      return -1;
   }else if(result == 0){
    //    char c;
    //    ssize_t x = recv(fd, &c, 1, MSG_PEEK);
    //    if(x <= 0){
    //        return -1;
    //    }
   }
   else if (result > 0 && FD_ISSET(fd, &readset)) {
      // Set non-blocking mode
      if ((iof = fcntl(fd, F_GETFL, 0)) != -1)
         fcntl(fd, F_SETFL, iof | O_NONBLOCK);
      // receive
      result = recv(fd, buffer, len, flags);
      // set as before
      if (iof != -1)
         fcntl(fd, F_SETFL, iof);
      return result;
   }
   return -2;
}