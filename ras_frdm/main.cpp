#define MAIN_AM

#ifdef MAIN_AM
#include "mbed.h"
//#include "rtos.h"
//#include "Dm3Security.h"
//#include "Mcc.h"
//#include "Bumper.h"
//#include "PIDModule.h"
//#include "dm3.h"
#include "Watchdog.h"

using namespace modules;
//Mcc mcc;
DigitalOut led_red(LED_RED);
DigitalOut led_blue(LED_BLUE);
DigitalOut led_green(LED_GREEN);
//Bumper bump(SW2);
//InterruptIn sw2(SW2);
//volatile int flag = 0;
/*
void heartbeat_task() {
	while (true) {
		led_green = !led_green;
		Thread::wait(500);
	}
}
*/

//void sw2_isr();	// Prototipo de la funcion

Watchdog wdt;
int main() {
	led_red 	= 1;
	led_blue 	= 1;
	led_green 	= 1;
	printf("INICIO\n");
	int i = 0;
	wdt.kick(10.0);	// 10 segundos sin kick resetea la placa.

	while(1) {
		printf("Loop.....\n");
		wait(0.1);
		if (i == 20){
			while(1);	//Trancar, debe dar timeout el WDT, resetear la placa y comenzar nuevamente por inicio.
		}
		wdt.kick();
		i++;
	}
}

/*
int main(){
	led_red 	= 1;
	led_blue 	= 1;
	led_green 	= 0;
	Dm3Security dm3_security;
	//Thread heartbeat(heartbeat_task);

	while (1){

	}
}
*/
/*
int main(){
	bump.attach(&sw2_isr);
	led = 1;

    while (1) {
		if (flag == 1){
			printf("ENTRO\n");
			flag = 0;
			led = 0;
			wait(1);
		}
		else{
			printf("AFUERA\n");
			led = 1;
		}
    }
}

void sw2_isr()
{
	flag = 1;
}
*/
#endif

#ifndef MAIN_AM

#include "mbed.h"
#include "rtos.h"
#include "modules/DM3/Dm3Security.h"
//#include "modules/Ultrasonic/Ultrasonic.h"
//#include "modules/Mcc/Mcc.h"
//#include "modules/Admin/Admin.h"
//#include "modules/Motor/MotorModule.h"
//#include "modules/DM3/Dm3Module.h"
//#include "hdlc/frdm_communication.h"
//#include "modules/Logging/Logger.h"
//#include "modules/Ethernet/Communication.h"
//Ticker ticker_msg_rate;
DigitalOut led_green(LED_GREEN);
DigitalOut led_red(LED_RED);
DigitalOut led_blue(LED_BLUE);

using namespace modules;
//Mcc mcc;

void heartbeat_task() {
	while (true) {
		led_green = !led_green;
		Thread::wait(500);
	}
}

//================================================
// TEST ULTRASONIC VARIABLES
//int _distance = 0;
//void dist(int distance)
//{
//	_distance = distance;
//}
//Ultrasonic mu(PTA1, PTA2, .1, 1, &dist);
//================================================

int main() {

	led_red = 1;
	led_blue = 1;
	led_green = 1;
	Dm3Security dm3_security;
	Thread heartbeat(heartbeat_task);

//	wait(2);
//	Admin admin_module;
//	MotorModule motorModule;
//	motorModule.init();
//	Dm3Module dm3Module;
//
//	Thread heartbeat(heartbeat_task);
//
//	while(1){
//		mcc.send_message(0,1,"test",4);
//		wait(1);
//		mcc.tick();
//	}

//==================================================
// ULTRASONIC LOGIC
//	mu.startUpdates();//start mesuring the distance
//	while(1)
//	{
////  	Do something else here
//		printf("the distance is: %dmm\n", _distance);
//		wait(1);
////		the class checks if dist needs to be called.
//	}
//}


//==================================================
//	Interruptions POC - START
//==================================================
//InterruptIn Interrupt(PTA1);
//
//
//void blink()
//{
//    wait(.4);
//    led_red=1;
//    led_blue=0;
//    wait(.4);
//    led_blue=1;
//    wait(.4);
//}
//
//int main()
//{
//	led_red = 1;
//	led_blue = 1;
//	led_green = 1;
//    Interrupt.fall(&blink);
//    led_blue=1;
//    while (1)
//    {
//        led_red=!led_red;
//        wait(.2);
//    }
//}
//==================================================
//	Interruptions POC - END
//==================================================

}
#endif

