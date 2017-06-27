//#define MAIN_AM

#ifdef MAIN_AM
#include "mbed.h"
#include "USBSerial.h"

//Virtual serial port over USB
USBSerial serial;

int main(void) {

    while(1)
    {
        serial.printf("I am a virtual serial port\r\n");
        wait(1);
    }
}
/*
#include "mbed.h"
#include "rtos.h"
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

void heartbeat_task() {
	while (1) {
		printf("HILO\n");
		Thread::wait(2000);
	}
}

//void sw2_isr();	// Prototipo de la funcion

Watchdog wdt;
int main() {
	led_red 	= 1;
	led_blue 	= 1;
	led_green 	= 1;
	printf("INICIO\n");
	wait(1.0);

	wdt.kick(1.0);	// 10 segundos sin kick resetea la placa.
	Thread heartbeat(heartbeat_task);
	while(1) {
		printf("MAIN\n");
		wdt.kick();
	}
}
*/
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
#include "modules/Mcc/Mcc.h"
#include "modules/Admin/Admin.h"
#include "modules/Motor/MotorModule.h"
#include "modules/DM3/Dm3Module.h"
Ticker ticker_msg_rate;
DigitalOut led_green(LED_GREEN);
DigitalOut led_red(LED_RED);
DigitalOut led_blue(LED_BLUE);

using namespace modules;
Mcc mcc;

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

//	wait(2);
	Admin* admin_module = Admin::get_instance();
	MotorModule* motorModule_instance = MotorModule::get_instance();
	motorModule_instance->init();
	Dm3Module dm3Module;

	Thread heartbeat;
	heartbeat.start(&heartbeat_task);
	Thread potpoll; // polling de potenciometro
//	potpoll.start(callback(&MotorModule::potpoll_task, &motorModule));
	Thread siren; // controlador de sirena
	siren.start(callback(&dm3Module, &Dm3Module::siren_task));
	Thread report_vel;
//	report_vel.start(callback(&MotorModule::rated_report_vel_task, &motorModule));
	Thread report_pow;
//	report_pow.start(callback(&MotorModule::rated_report_pow_task, &motorModule));
	Thread report_pot;
//	report_pot.start(callback(&MotorModule::rated_report_pot_task, &motorModule));
	  //Thread security_stop_task(motorModule.security_stop_task);
	Thread battery_report_task;
//	battery_report_task.start(callback(&Dm3Module::battery_report_task, &dm3Module));


//	mcc.send_message(0,1,"test",4);
	while(1){
		wait(1);
		mcc.tick();
	}

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

