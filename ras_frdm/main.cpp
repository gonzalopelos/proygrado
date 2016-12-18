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


#include "mbed.h"

//#include "hdlc/frdm_communication.h"
#include "modules/Logging/Logger.h"
#include "modules/Ethernet/Communication.h"

//typedef struct {
//	char data[256];
//	int size;
//}message_data_t;
//Serial main_pc(USBTX, USBRX);

//void pc_thred_func(){
//	DigitalOut led_red(LED_RED);
//	led_red = 0;
//	char c;
//	int serial_readable;
//	char data[19];
//	while(1){
//		serial_readable = main_pc.readable();
//		while(serial_readable){
//			led_red = !led_red;
//			c = main_pc.getc();
//			sprintf(data, "serial readable: %d\n", serial_readable);
//			main_pc.puts(data);
////			Logger::get_instance()->write_trace(&c);
//			main_pc.putc(c);
//		}
//	}
//}


int main() {
//	Thread _pc_thread;
//	main_pc.baud(9600);
//	main_pc.format();
	//int connection_id = 0;
	//connection_id = open_frdm_connection();
	DigitalOut led_green(LED_GREEN);
	DigitalOut led_red(LED_RED);
	DigitalOut led_blue(LED_BLUE);
	led_red = 1;
	led_blue = 1;
	led_green = 1;
	Communication* comm = Communication::get_instance();
	comm->send_all("Hello",5);
	char data[256];
	int data_size;
	//Logger::get_instance()->write_trace("Estoy en el main");
	wait(3);
//	_pc_thread.start(pc_thred_func);
	while(1){

		//led_green = !led_green;
		bzero(data, 256);
		//wait(1.5);
		data_size = comm->receive(data, 255);
		if (data_size > 0){
			comm->send_all(data, data_size);
		} else if (data_size < 0){
			led_green = !led_green;
		}
//		data_size = get_from_fdrm(connection_id, data, 255);
//		if(data_size > 0){
//			send_to_frdm(connection_id, data,data_size);
//		}
//		Logger::get_instance()->write_trace("Estoy en el main\n");
	}

}


#endif



