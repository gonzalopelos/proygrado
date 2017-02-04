#include "Communication.h"


Communication * Communication::_communication_instance = NULL;
DigitalOut communication_led_red(LED_RED);

Communication* Communication::get_instance() {
	if(_communication_instance == NULL){
		_communication_instance = new Communication();
	}

	return _communication_instance;
}


int Communication::send_all(char* data, int length) {
	int result = 0;

	result = getSocketClient()->send_all(data, length);

	return result;
}


int Communication::receive(char* data, int length) {
	int result = 0;
	bzero(data, sizeof(data));
	result = getSocketClient()->receive(data, length);

	return result;
}


Communication::~Communication() {
	free(_communication_instance);
}


Communication::Communication() {
	init_eth_interface();
	communication_led_red = 0;
}

void Communication::init_eth_interface() {
	_eth_interface.init(ETH_COMMUNICATION_IP, ETH_COMMUNICATION_MASK, ETH_COMMUNICATION_GW); //Assign a device ip, mask and gateway
	_eth_interface.connect(100);
	_socket_server.bind(ETH_COMMUNICATION_SERVER_PORT);
	_socket_server.listen(1);

}

TCPSocketConnection* Communication::getSocketClient() {
	if(!_socket_client.is_connected()){
		_socket_client.close();

		communication_led_red = !communication_led_red;

		_socket_server.accept(_socket_client);
		_socket_client.set_blocking(false, 1000);
	}
	return &_socket_client;
}
