/*
 * Communication.h
 *
 *  Created on: Dec 17, 2016
 *      Author: gonzalopelos
 */

#ifndef MODULES_ETHERNET_COMMUNICATION_H_
#define MODULES_ETHERNET_COMMUNICATION_H_

#include "../EthernetInterface/EthernetInterface.h"
#define ETH_COMMUNICATION_IP       		"192.168.1.52"
#define ETH_COMMUNICATION_MASK     		"255.255.255.0"
#define ETH_COMMUNICATION_GW       		"0.0.0.0"
#define ETH_COMMUNICATION_SERVER_PORT   5001

class Communication {
public:
	static Communication* get_instance();

	int send_all(char* data, int length);
	int receive(char* data, int length);


protected:
	static Communication* _communication_instance;
	virtual ~Communication();
	EthernetInterface _eth_interface;
	TCPSocketServer _socket_server;
	TCPSocketConnection _socket_client;

	Communication();
	void init_eth_interface();
	TCPSocketConnection* getSocketClient();

};


#endif /* MODULES_ETHERNET_COMMUNICATION_H_ */
