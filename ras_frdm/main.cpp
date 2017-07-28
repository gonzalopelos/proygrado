#include "mbed.h"
#include "rtos.h"
#include "modules/DM3/Dm3Security.h"
#include "modules/Mcc/Mcc.h"
#include "modules/Admin/Admin.h"
#include "modules/Motor/MotorModule.h"
#include "modules/DM3/Dm3Module.h"
#include "Watchdog.h"
#include <stdio.h>
#include <stdlib.h>

using namespace modules;
Watchdog wdt;

Mcc mcc;

Ticker ticker_msg_rate;
DigitalOut led_green(LED_GREEN);
DigitalOut led_red(LED_RED);
DigitalOut led_blue(LED_BLUE);

void heartbeat_task() {
	while (true) {
		wdt.kick();	// Refresh WDT
		led_green = !led_green;
		Thread::wait((wdt.getTimeOutValue() * 1000) / 2);	// Wait WDT (toVal / 2) expressed in milliseconds.
	}
}

//================================================
/*
 * MEMORY MONITOR
 */
/* Using malloc() to determine free memory.*/

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
	// Levantar hilo con heartbeat, contiene el kick del WDT.
	Thread heartbeat;
	heartbeat.start(&heartbeat_task);

	Admin* admin_module = Admin::get_instance();
	MotorModule* motorModule_instance = MotorModule::get_instance();
	motorModule_instance->init();
	Dm3Module dm3Module;

	memory_monitor_thread.start(&memory_monitor);
	Thread potpoll; // polling de potenciometro
	potpoll.start(callback(&MotorModule::potpoll_task, motorModule_instance));
	Thread siren; // controlador de sirena
	siren.start(callback(&dm3Module, &Dm3Module::siren_task));
	Thread report_vel;
	report_vel.start(callback(&MotorModule::rated_report_vel_task, &motorModule_instance));
	Thread report_pow;
	report_pow.start(callback(&MotorModule::rated_report_pow_task, &motorModule_instance));
	Thread report_pot;
	report_pot.start(callback(&MotorModule::rated_report_pot_task, &motorModule_instance));
	Thread update_current_vels;
	update_current_vels.start(callback(&MotorModule::compute_speed_task, motorModule_instance));
	//Thread security_stop_task(motorModule.security_stop_task);
//	Thread battery_report_task;
//	battery_report_task.start(callback(&Dm3Module::battery_report_task, &dm3Module));

	while(1){
		mcc.tick();
	}
}


