#include "mbed.h"

DigitalOut ledGreen(LED_GREEN);
DigitalOut ledRed(LED_RED);
DigitalOut ledBlue(LED_BLUE);

int main()
{
	ledRed 	= 1;
	ledBlue = 1;
    while (true) {
    	ledGreen = !ledGreen;
        wait(0.5f);
    }
}
