/*
 * Dm3Security.cpp
 *
 *  Created on: May 18, 2017
 *      Author: gonzalopelos
 */

#include <Dm3Security.h>
#include <mbed.h>
#include <rtos.h>
#include "../Ultrasonic/Ultrasonic.h"

namespace modules {

Thread _ultrasonic_alert_thread;
int _ultrasonic_front_distance;


void update_front_dist(int distance);
void handle_ultrasonic_distance_alert();

Ultrasonic ultrasonic_front(PTA1, PTA2, .1, 1, &update_front_dist);

Dm3Security::Dm3Security() {
	// TODO Auto-generated constructor stub
	ultrasonic_front.startUpdates();
	_ultrasonic_alert_thread.start(&handle_ultrasonic_distance_alert);
}

//ToDo: Agregar filtro por software para sacar ruido leer diapo de FRA diapositiva 1 hay formula, aplicar filtro de ventana o ponderaciones.
//ToDO: REvisar wathchdog


Dm3Security::~Dm3Security() {
	// TODO Auto-generated destructor stub

}

void update_front_dist(int distance)
{
	_ultrasonic_front_distance = distance;
   //ToDo Logic to check distance ranges
	_ultrasonic_alert_thread.signal_set(0x1);
}

void handle_ultrasonic_distance_alert(){
//	int risk_state = 0;
	DigitalOut led_blue(LED_BLUE);
	led_blue = 1;
	while(true) {
		_ultrasonic_alert_thread.signal_wait(0x1, 0);

		if(_ultrasonic_front_distance < ULTRASONIC_MIN_FRONT_DIST){
			led_blue = 0;
		} else {
			led_blue = 1;
		}
	}
}


} /* namespace modules */
