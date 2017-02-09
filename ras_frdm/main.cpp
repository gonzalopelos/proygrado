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
#include "rtos.h"
#include "modules/Mcc/Mcc.h"

//#include "hdlc/frdm_communication.h"
//#include "modules/Logging/Logger.h"
//#include "modules/Ethernet/Communication.h"
DigitalOut led_green(LED_GREEN);
Mcc mcc;
void heartbeat_task() {
    while (true) {
    	led_green = !led_green;
        Thread::wait(500);
    }
}

int main() {
//	Thread _pc_thread;
//	main_pc.baud(9600);
//	main_pc.format();
	//int connection_id = 0;
	//connection_id = open_frdm_connection()
	DigitalOut led_red(LED_RED);
	DigitalOut led_blue(LED_BLUE);
	led_red = 1;
	led_blue = 1;
	led_green = 1;
	wait(2);
	mcc.send_message(0,0,"test",4);

	Thread heartbeat(heartbeat_task);

	while(1){
//		wait(0.4);
//		led_blue = !led_blue;
	}

}


#endif



