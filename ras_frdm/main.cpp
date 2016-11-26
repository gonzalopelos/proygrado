//#define MAIN_AM

#ifdef MAIN_AM
#include "mbed.h"
#include "PCF8591.h"
#include "PIDModule.h"

I2C i2c(PTE25, PTE24); // SDA, SCL para K64F
PCF8591 DAC_I2C(&i2c, PCF8591_SA0); // I2C bus, Default PCF8591 Slaveaddress
Serial pc(USBTX, USBRX);

DigitalOut led(LED_RED);
DigitalOut ledGreen(LED_GREEN);

int main()
{
	led 		= 0;
	ledGreen	= 0;
    while (true) {
    	int command = pc.getc();
    	switch(command){
			case 1:
				led 		= !led;
				ledGreen 	= !led;
				break;
			case 6:

				break;
			default:
				ledGreen = !ledGreen;
    	}
        wait(0.4f);
    }
}

/*
int main() {
	PIDModule pid[1];
	uint8_t count = 0;
	float vel = 0;
    while(1) {
    	wait(0.1);
    	count++;

    	vel = pid[1].compute(0, 10);

    	DAC_I2C.write(count);
    }
}
*/

#endif

#ifndef MAIN_AM

//#include "mbed.h"
//#include "hdlc/frdm_communication.h"
//#include "rtos.h"

//DigitalOut led1(LED1);
//DigitalOut led2(LED2);
//DigitalOut led3(LED3);
//DigitalOut led4(LED4);
//
////Serial pc(USBTX, USBRX);
//Serial my_serial(USBTX, USBRX);

//Thread hdlc_secondary_station_thread;
//Thread pc_serial;
//Thread pc_serial_2;
//typedef struct {
//	char data[256];
//	int size;
//}message_data_t;

//Queue<message_data_t, 16> messages;
//MemoryPool<message_data_t, 16> messages_pool;

//int messages_count = 0;

//void pc_callback() {
//	led3 = !led3;
//	while(my_serial.readable()){
//		my_serial.getc();
//	}

//	message_data_t * me = messages_pool.alloc();
//	me->size = 0;
//	while(pc.readable()) {
//		if(me->size < 256) {
//			me->data[me->size] = pc.getc();
//			me->size++;
//		}
//	}
//	led4 = !led4;
//	wait(0.5);
//	led4 = !led4;
//	messages.put(me);
//	messages_count++;
//}


//int main() {
//	led1 = 1;
//	led2 = 1;
//	led3 = 1;
//	led4 = 1;

	/*int connection = open_frdm_connection();
	char data[256];
	int data_size;
	int data_sent_size;
    while (1) {
        wait(0.5);
        data_size = get_from_fdrm(connection, data, 255);
        if(data_size > 0){
        	led1 = !led1;
			wait(1);
			led1 = !led1;
        	data_sent_size = send_to_frdm(connection, data, data_size);
        	if(data_sent_size == data_size){
        		led4= !led4;
        		wait(1);
        		led4 = !led4;

        	}
        }
    }*/
	//my_serial.baud(9600);
	//pc.format();
//	my_serial.attach(&pc_callback);

	//message_data_t* my_message;
//	while(1) {
//		wait(0.5);
//		led2 = !led2;
	//	if(messages_count > 1){
//			led1 = !led1;
//			wait(0.5);
//			led1 = !led1;
//
//			osEvent e =  messages.get();
//			if(e.status == osEventMessage) {
//				led2 = !led2;
//				wait(0.5);
//				led2 = !led2;
//
//				my_message = (message_data_t *)e.value.p;
//				if(my_message->size > 0) {
//					led3 = !led3;
//					wait(0.5);
//					led3 = !led3;
//					pc.puts(my_message->data);
//					messages_pool.free(my_message);
//					messages_count--;
//				}
//			}
//		}
//	}

//}
#include "mbed.h"
#include "rtos.h"

#include "EthernetInterface.h"

#include "Logging/Logger.h"

#define MBED_DEV_IP       "192.168.1.52"
#define MBED_DEV_MASK     "255.255.255.0"
#define MBED_DEV_GW       "0.0.0.0"
#define ECHO_SERVER_PORT   5000

//DigitalOut led1(LED1);
//DigitalOut led2(LED2);
//DigitalOut led3(LED3);
//
//Serial pc(USBTX, USBRX);

typedef struct {
	char data[256];
	int size;
}message_data_t;

Queue<message_data_t, 16> messages;
MemoryPool<message_data_t, 16> messages_pool;
Mutex message_count_mutex;
Thread _reader_thread;

int messages_count = 0;


void reader_func();
void start_reader();

/*void callback() {
    led2 = !led2;   // on
	message_data_t * me = messages_pool.alloc();
	me->size = 0;
	while(pc->readable()) {
		if(me->size < 256) {
			me->data[me->size] = pc->getc();
			me->size++;
		}
	}
	messages.put(me);
	message_count_mutex.lock();
	messages_count++;
	message_count_mutex.unlock();
}*/
/*

void reader_func(){
	while(1){
		led2 = !led2;

		message_data_t * me = messages_pool.alloc();
		me->size = 0;
		while(pc.readable()) {
			if(me->size < 256) {
				me->data[me->size] = pc.getc();
				me->size++;
			}
		}
		if(me->size > 0){
			messages.put(me);
			message_count_mutex.lock();
			messages_count++;
			message_count_mutex.unlock();
		} else {
			messages_pool.free(me);
		}
	}
}
*/

int main() {
/*
    led1 = 0;
    led2 = 1;   // Off
    led3 = 1;
    pc.baud(9600);
    start_reader();
    message_data_t* my_message;
    while(1) {
        led1 = !led1;

        message_count_mutex.lock();
		if(messages_count > 0) {
			led3 = !led3;
			//wait(0.5);
			if(messages_count == 2) {
				//wait(0.5);
				led3 = !led3;
				osEvent e =  messages.get();
				if(e.status == osEventMessage) {
					my_message = (message_data_t *)e.value.p;
					if(my_message->size > 0) {
						pc.puts(my_message->data);
						messages_pool.free(my_message);
						messages_count--;
					}
				}
			}
		}
		message_count_mutex.unlock();
    }
*/

/*	Serial pc(USBTX, USBRX);
	EthernetInterface eth;
	DigitalOut led_green(LED_GREEN);
	led_green = 0;
	eth.init(MBED_DEV_IP, MBED_DEV_MASK, MBED_DEV_GW); //Assign a device ip, mask and gateway
	eth.connect(100);
	pc.printf("IP Address is %s\n", eth.getIPAddress());
	led_green = !led_green;
	TCPSocketServer server;
	server.bind(ECHO_SERVER_PORT);
	server.listen();

	while (true) {
		pc.printf("\nWait for new connection...\n");
		TCPSocketConnection client;
		server.accept(client);
		client.set_blocking(false, 1500); // Timeout after (1.5)s
		led_green = !led_green;
		pc.printf("Connection from: %s\n", client.get_address());
		char buffer[256];
		while (true) {
			int n = client.receive(buffer, sizeof(buffer));
			if(n>0){
				client.send_all(buffer, n);
			}
		}
		led_green = !led_green;
		client.close();
	}*/

	DigitalOut led_green(LED_GREEN);
	DigitalOut led_red(LED_RED);
	DigitalOut led_blue(LED_BLUE);
	led_red = 1;
	led_blue = 1;
	led_green = 0;
	Logger* _logger = Logger::get_instance();
	int counter = 0;
	char data[128];
	while(true){
		wait(1.5);
		led_green = !led_green;
		bzero(data, sizeof(data));
		sprintf(data, "test from main %d\n", counter);
		_logger->write_trace(data);
		counter ++;
	}

}



inline void start_reader() {

	_reader_thread.start(reader_func);
	//_reader_thread.join();
}

#endif



