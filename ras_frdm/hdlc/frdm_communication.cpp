#include <fcntl.h>

#include <cerrno>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include "mbed.h"
#include "rtos.h"
#include "frdm_communication.h"
#include "Logging/Logger.h"

Serial _serial_connection(USBTX, USBRX);
DigitalOut frdm_comm_led_green(LED2);
DigitalOut frdm_comm_led_blue(LED3);
DigitalOut frdm_comm_led_red(LED4);

bool _serial_connection_callback_attached = false;

/** HDLC start/end flag sequence */
#define _YAHDLC_FLAG_SEQUENCE 0x7E

#define _HDLC_MAX_MESSAGE_LENGTH 128

const int _BUFFER_MEESSAGE_CAPACITY = 10;

typedef struct {
	int size;
	char data[_HDLC_MAX_MESSAGE_LENGTH];

} structured_data_t;

typedef struct {
	int connection_id;
	Mail<structured_data_t, 16> messages;
} read_messages_buffer_t;

read_messages_buffer_t _read_message_buffers[2];

int _connections_count = 0;

rtos::Mutex _connection_mutex;
rtos::Mutex _read_message_buffers_mutex;

rtos::Thread _reader_thread;

int started_message = 0;

char _logger_data[_HDLC_MAX_MESSAGE_LENGTH];


read_messages_buffer_t get_read_messages_buffer(int connection_id){
//	if(connection_id > _connections_count){
		//ToDo Handle Errors
//		return;
//	}
//	else {
	/*bzero(_logger_data, sizeof(_logger_data));
	sprintf(_logger_data, "get_read_messages_buffer connection id: %d\n", connection_id);
	Logger::get_instance()->write_trace(_logger_data);
*/
	return _read_message_buffers[connection_id];
//	}
}


void delete_message(int connection_id, structured_data_t * message) {

	_read_message_buffers_mutex.lock();

	read_messages_buffer_t buffer = get_read_messages_buffer(connection_id);
	buffer.messages.free(message);

	_read_message_buffers_mutex.unlock();
}

void add_message_in_buffers(structured_data_t message) {
	bzero(_logger_data, sizeof(_logger_data));
	sprintf(_logger_data, "frdm_communication - add_message_in_buffers:\n%s\n\n", message.data);
	Logger::get_instance()->write_trace("add_message_in_buffers");
	Logger::get_instance()->write_trace(_logger_data);

	_read_message_buffers_mutex.lock();

	structured_data_t * new_meesage;
	for (int index = 0; index < _connections_count; index++) {
		new_meesage = _read_message_buffers[index].messages.alloc();
		new_meesage->size = message.size;
		bzero(new_meesage->data, sizeof(new_meesage->data));
		sprintf(new_meesage->data, message.data);
		_read_message_buffers[index].messages.put(new_meesage);
	}

	_read_message_buffers_mutex.unlock();



}

structured_data_t* get_message_from_buffer(int connection_id) {

	structured_data_t* result = NULL;
	read_messages_buffer_t buffer = get_read_messages_buffer(connection_id);
	osEvent e = buffer.messages.get(500);
	if (e.status == osEventMail) {
		result = (structured_data_t*)e.value.p;
		frdm_comm_led_blue = 0;
	}


	return result;
}

void serial_connection_callback() {
	//char read_char;
	while(1){
//		frdm_comm_led_red = !frdm_comm_led_red;
		structured_data_t message;

		while (_serial_connection.readable()) {
			unsigned char read_char = _serial_connection.getc();
			if (started_message == 0 && read_char == _YAHDLC_FLAG_SEQUENCE) {
				bzero(message.data, sizeof(message.data));
				started_message = 1;
				message.size = 1;
				message.data[0] = read_char;
				frdm_comm_led_red = 0;
//				Logger::get_instance()->write_trace("Started message\n");

			} else if (started_message == 1 && read_char != _YAHDLC_FLAG_SEQUENCE) {
				message.data[message.size] = read_char;
				message.size++;
//				Logger::get_instance()->write_trace("Read message data\n");
			} else if (started_message == 1 && read_char == _YAHDLC_FLAG_SEQUENCE) {
				started_message = 0;
				message.data[message.size] = read_char;
				message.size++;
//				Logger::get_instance()->write_trace("Before add message in buffer\n");
				add_message_in_buffers(message);
				frdm_comm_led_red = 1;
			}
		}

//		frdm_comm_led_red = !frdm_comm_led_red;

	}
}

/*
 * 'open_port()' - Open serial port 1.
 *
 * Returns the file descriptor on success or -1 on error.
 */
int open_frdm_connection() {
	int connection_id = _connections_count;
	_connection_mutex.lock();

	if (_connections_count == 0) {
		_serial_connection.baud(9600);
		_serial_connection.format();
		frdm_comm_led_red = 1;

		_reader_thread.start(serial_connection_callback);
	bzero(_logger_data, sizeof(_logger_data));
	sprintf(_logger_data, "open_frdm_connection return: %d\n", connection_id);
	Logger::get_instance()->write_trace(_logger_data);

		//_serial_connection.attach(&serial_connection_callback);
	}
	_read_message_buffers[_connections_count].connection_id = connection_id;
	_connections_count++;

	_connection_mutex.unlock();

	return connection_id;
}

int get_from_fdrm(int file_descriptor, char *dataRead, unsigned int maxSize) {

	int data_read_size = 0;
	structured_data_t * message;

	_read_message_buffers_mutex.lock();

	message = get_message_from_buffer(file_descriptor);
	if (message != NULL) {
		if(message->size>0){
			if(message->size <= maxSize){
				data_read_size = sprintf(dataRead, message->data);
			}
		}
//		for (int index = 0; index < message->size; index++) {
//			dataRead[index] = message->data[index];
//		}
//		data_read_size = message->size;
//		if (data_read_size < maxSize) {
//			dataRead[data_read_size] = '\0';
//		}
	} else {
		dataRead[0] = '\0';
	}

	_read_message_buffers_mutex.unlock();

	if(message != NULL){
		delete_message(file_descriptor, message);
	}

	return data_read_size;
}

int send_to_frdm(int file_descriptor, char* data, unsigned int size) {
	int data_sent_index = 0;
//    while(_serial_connection.writeable() && (unsigned int)data_sent_index < size) {
//     _serial_connection.putc(data[data_sent_index]);
//     data_sent_index++;
//    }
	if (_serial_connection.writeable()) {
		data_sent_index = _serial_connection.puts(data);
	}

	return data_sent_index;
}
