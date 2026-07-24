#include "mbed.h"

Serial pc(USBTX, USBRX);
DigitalOut led(LED_RED);
DigitalOut ledGreen(LED_GREEN);

int main()
{
	led = 0;
	ledGreen = 0;
    while (true) {
    	int command = pc.getc();
    	switch(command){
    	case 1:
    		led = !led;
    		ledGreen = !led;
    		break;
    	default:
    		ledGreen = !ledGreen;
    	}

        wait(0.4f);
    }
}
