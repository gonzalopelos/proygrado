#include "mbed.h"

DigitalOut gpo(D0);
DigitalOut led(LED_RED);

int main()
{
	led = 1;
	while (true) {
        wait(0.8f);
        led = !led; // toggle led
    }
}
