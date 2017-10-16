#include "Communication.h"
#include "errno.h"


Communication * Communication::_communication_instance = NULL;

Communication* Communication::get_instance() {
	if(_communication_instance == NULL){
		_communication_instance = new Communication();
	}

	return _communication_instance;
}


int Communication::send_all(char* data, int length) {
	int result = 0;
	result = getSocketClient()->send_all(data, length);

	if(result <= 0){
		if(errno != ERR_OK){
			//printf("\n\nSEND RESULT: %d, %s\n\n", result, strerror(errno));
		}
	}

	return result;
}


int Communication::receive(char* data, int length) {
	int result = 0;
	bzero(data, sizeof(data));

	result = getSocketClient()->receive(data, length);

	if(result <= 0){
		if(errno != ERR_OK){
			//printf("\n\nRECEIVE RESULT: %d, %s\n\n", result, strerror(errno));
		}
	}

	return result;
}


Communication::~Communication() {
	_socket_client.close(true);
	_socket_server.close(true);
	free(_communication_instance);
}


Communication::Communication() {
	init_eth_interface();
}

void Communication::init_eth_interface() {
	_eth_interface.init(ETH_COMMUNICATION_IP, ETH_COMMUNICATION_MASK, ETH_COMMUNICATION_GW); //Assign a device ip, mask and gateway
	_eth_interface.connect(100);
	_socket_server.bind(ETH_COMMUNICATION_SERVER_PORT);
	_socket_server.listen(1);
	_socket_server.set_blocking(false, 1000);
//	getSocketClient();
}

bool Communication::is_client_connected() {
//	printf("\n\nCLIENT IS CONNECTED: %s\n\n", _socket_client.is_connected() ? "TRUE":"FALSE");
	return  _socket_client.is_connected();//_client_connected &&
}

TCPSocketConnection* Communication::getSocketClient() {
	if(!is_client_connected()){
		_socket_client.close();
//		printf("\n\ngetSocketClient is_connected = false\n\n");

		if(_socket_server.accept(_socket_client) == 0)
		{
			_socket_client.set_blocking(false, 1000);
//		}else{
//			printf("\nsocket_server.accept ERROR\n");
		}
//		//printf("sale del getSocketClient()\n");
	}
	return &_socket_client;
}
