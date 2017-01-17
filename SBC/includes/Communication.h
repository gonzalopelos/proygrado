//
// Created by Gonzalo Pelós on 12/18/16.
//

#ifndef SBC_COMMUNICATION_H
#define SBC_COMMUNICATION_H

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#define ETH_COMMUNICATION_IP       		"192.168.1.52"
#define ETH_COMMUNICATION_SERVER_PORT   5001

class Communication {
public:
    static Communication* get_instance();

    int send_all(char* data, int length);
    int receive(char *data, int max_length);
    virtual ~Communication();


protected:
    static Communication * _communication_instance;
    Communication();
    void init_eth_interface();

};

#endif //SBC_COMMUNICATION_H
