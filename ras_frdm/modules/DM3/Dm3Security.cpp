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


// Front-left ultrasonic sensor variables =========================
Thread _ultrasonic_fl_alert_thread;
int _ultrasonic_fl_distance;
int _ultrasonic_fl_last_distance;

void update_fl_dist(int distance);
void handle_ultrasonic_fl_distance_alert();

Ultrasonic ultrasonic_fl(PTA1, PTA2, .1, 1, &update_fl_dist);
// ================================================================


// Front-right ultrasonic sensor variables =========================
Thread _ultrasonic_fr_alert_thread;
int _ultrasonic_fr_distance;
int _ultrasonic_fr_last_distance;

void update_fr_dist(int distance);
void handle_ultrasonic_fr_distance_alert();

Ultrasonic ultrasonic_fr(PTD1, PTD3, .1, 1, &update_fr_dist);
// ================================================================

int filter_distance(int distance, int last_distance);

Dm3Security::Dm3Security() {
	// TODO Auto-generated constructor stub
	ultrasonic_fl.startUpdates();
	_ultrasonic_fl_last_distance = -1;
	_ultrasonic_fl_alert_thread.start(&handle_ultrasonic_fl_distance_alert);
	ultrasonic_fr.startUpdates();
	_ultrasonic_fr_last_distance = -1;
	_ultrasonic_fr_alert_thread.start(&handle_ultrasonic_fr_distance_alert);
}

//ToDo: Agregar filtro por software para sacar ruido leer diapo de FRA diapositiva 1 hay formula, aplicar filtro de ventana o ponderaciones.
//ToDO: REvisar wathchdog


Dm3Security::~Dm3Security() {
	// TODO Auto-generated destructor stub

}

void update_fl_dist(int distance)
{
	_ultrasonic_fl_distance = distance;
   //ToDo Logic to check distance ranges
	_ultrasonic_fl_alert_thread.signal_set(0x1);
}

void handle_ultrasonic_fl_distance_alert(){
//	int risk_state = 0;
	DigitalOut led_blue(LED_BLUE);
	led_blue = 1;
	while(true) {
		_ultrasonic_fl_alert_thread.signal_wait(0x1, 0);
		_ultrasonic_fl_last_distance = filter_distance(_ultrasonic_fl_distance, _ultrasonic_fl_last_distance);
		if(_ultrasonic_fl_distance < ULTRASONIC_MIN_FRONT_DIST){
			led_blue = 0;
		} else {
			led_blue = 1;
		}
	}
}

void update_fr_dist(int distance)
{
	_ultrasonic_fr_distance = distance;
   //ToDo Logic to check distance ranges
	_ultrasonic_fr_alert_thread.signal_set(0x1);
}

void handle_ultrasonic_fr_distance_alert(){
//	int risk_state = 0;
	DigitalOut led_red(LED_RED);
	led_red = 1;
	while(true) {
		_ultrasonic_fr_alert_thread.signal_wait(0x1, 0);
		_ultrasonic_fr_last_distance = filter_distance(_ultrasonic_fr_distance, _ultrasonic_fr_last_distance);
		if(_ultrasonic_fr_last_distance < ULTRASONIC_MIN_FRONT_DIST){
			led_red = 0;
		} else {
			led_red = 1;
		}
	}
}

int filter_distance(int dist, int last_dist){
	int result = dist;
	float alpha = 0.2;
	if(last_dist != -1){
		result = last_dist + alpha * (dist - last_dist);
	}
	return result;
}


} /* namespace modules */
