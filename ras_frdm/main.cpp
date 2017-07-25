#define MAIN_AM

#ifdef MAIN_AM
/*
#include "mbed.h"
#include "USBHostSerial.h"

DigitalOut led(LED_RED);
Serial pc(USBTX, USBRX);

int main() {
	pc.printf("ARRANCO");
	led = 0;
	USBHostSerial serial;
	while(1) {
		led = !led;
		// try to connect a serial device
		pc.printf("INTENDANDO CONECTAR\n");
		while(!serial.connect()){
			wait(0.5);
			pc.printf("INTENDANDO CONECTAR\n");
		}

		// in a loop, print all characters received
		// if the device is disconnected, we try to connect it again
		pc.printf("CONECTO\n");
		while (1) {
			// if device disconnected, try to connect it again
			if (!serial.connected())
				break;

			pc.printf("IMPRIMIENDO\n");
			serial.putc('A');
			wait(0.5);
		}
	}
}
*/

#include "mbed.h"
#include "rtos.h"
//#include "Dm3Security.h"
//#include "Mcc.h"
//#include "Bumper.h"
//#include "PIDModule.h"
//#include "dm3.h"
#include "Watchdog.h"
#include "fsl_rcm.h"

using namespace modules;
//Mcc mcc;
DigitalOut led_red(LED_RED);
DigitalOut led_blue(LED_BLUE);
DigitalOut led_green(LED_GREEN);
//Bumper bump(SW2);
//InterruptIn sw2(SW2);
//volatile int flag = 0;

Watchdog wdt;

void reportResetStatus(){
	uint32_t resetStatus = RCM_GetPreviousResetSources(RCM) & (kRCM_SourceWdog | kRCM_SourcePin | kRCM_SourcePor);
	switch(resetStatus){
		case kRCM_SourceWdog:
			printf("WATCHDOG TIMEOUT RESET\r\n");
			break;
		case kRCM_SourcePin:
			printf("EXTERNAL PIN RESET\r\n");
			break;
		case kRCM_SourcePor:
			printf("POWER ON RESET\r\n");
			break;
		default:
			printf("OTHER SOURCE RESET\r\n");
			break;
	}
}

int main() {
	reportResetStatus();
	wait(1.0);

	led_red 	= 1;
	led_blue 	= 1;
	led_green 	= 1;
	printf("INICIO\r\n");
	wait(1.0);

	wdt.kick(1.0);	// 1 segundos sin kick resetea la placa.

	while(1) {
		printf("MAIN\r\n");
		wdt.kick();
		wait(2.0);
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

//================================================
/*
 * MEMORY MONITOR
 */
/* Using malloc() to determine free memory.*/

#include <stdio.h>
#include <stdlib.h>
#define FREEMEM_CELL 100
struct elem { /* Definition of a structure that is FREEMEM_CELL bytes  in size.) */
    struct elem *next;
    char dummy[FREEMEM_CELL-2];
};

int FreeMem(void) {
    int counter;
    struct elem *head, *current, *nextone;
    current = head = (struct elem*) malloc(sizeof(struct elem));
    if (head == NULL)
        return 0;      /*No memory available.*/
    counter = 0;
   // __disable_irq();
    do {
        counter++;
        current->next = (struct elem*) malloc(sizeof(struct elem));
        current = current->next;
    } while (current != NULL);
    /* Now counter holds the number of type elem
       structures we were able to allocate. We
       must free them all before returning. */
    current = head;
    do {
        nextone = current->next;
        free(current);
        current = nextone;
    } while (nextone != NULL);
   // __enable_irq();

    return counter*FREEMEM_CELL;
}

Thread memory_monitor_thread;
void memory_monitor(){
	int last_free_memory;
	int actual_free_memory;
	last_free_memory = FreeMem();
	while(true){
		actual_free_memory = FreeMem();
		if(actual_free_memory < last_free_memory){
			printf("FREE MEMORY: %dB\n", actual_free_memory);
		}
		last_free_memory = actual_free_memory;
		Thread::wait(1000);
	}
}
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
	memory_monitor_thread.start(&memory_monitor);
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
//		wait(1);
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

