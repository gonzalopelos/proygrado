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
#include "../Motor/MotorModule.h"
#include "../Ethernet/Communication.h"
#include "Bumper.h"

namespace modules {

Mutex _dm3_security_state_mutex;
Dm3 * _dm3_instance = NULL;
MotorModule* _motor_module_instance = NULL;

Dm3Security* Dm3Security::_dm3_security_instance = NULL;

// Front-right ultrasonic sensor variables =========================
Thread _ultrasonic_fr_alert_thread;
int _ultrasonic_fr_distance;
int _ultrasonic_fr_last_distance;
void update_fr_dist(int distance);
Ultrasonic ultrasonic_fr(PTD1, PTD3, .1, 1, &update_fr_dist);
// ================================================================

// Front-left ultrasonic sensor ===================================
Thread _ultrasonic_fl_alert_thread;
int _ultrasonic_fl_distance;
int _ultrasonic_fl_last_distance;
void update_fl_dist(int distance);
Ultrasonic ultrasonic_fl(PTA1, PTA2, .1, 1, &update_fl_dist);
// ================================================================

// Bumper =========================================================
void handle_bumper_down();
void handle_bumper_up();
void handle_bumper_down_alert();
void handle_bumper_up_alert();
Thread _bumper_down_alert_thread;
Thread _bumper_up_alert_thread;
Bumper bumper(SW2);
// ================================================================

// TCP Connection =================================================
Thread _tcp_connection_checks_thread;


Dm3Security* Dm3Security::get_instance(){
	if(_dm3_security_instance == NULL){
		_dm3_security_instance = new Dm3Security();
	}

	return _dm3_security_instance;
}

Dm3Security::Dm3Security() {
	// TODO Auto-generated constructor stub
	// Sensores de contacto
	bumper.timeout = BUMPER_DEBOUNCING_TIMEOUT;
	bumper.attachDown(&handle_bumper_down);
	bumper.attachUp(&handle_bumper_up);
	_bumper_down_alert_thread.start(callback(this, &Dm3Security::handle_bumper_down_alert));
	_bumper_up_alert_thread.start(callback(this, &Dm3Security::handle_bumper_up_alert));

	// Sensores ultrasonicos
	ultrasonic_fl.startUpdates();
	_ultrasonic_fl_last_distance = -1;
	_ultrasonic_fl_alert_thread.start(callback(this, &Dm3Security::handle_ultrasonic_fl_distance_alert));
//	ultrasonic_fr.startUpdates();
	_ultrasonic_fr_last_distance = 9999999;
	_ultrasonic_fr_alert_thread.start(callback(this, &Dm3Security::handle_ultrasonic_fr_distance_alert));
	_dm3_instance = Dm3::Instance();
	_motor_module_instance = MotorModule::get_instance();

	//TCP connection alerts
	_tcp_connection_checks_thread.start(callback(this, &Dm3Security::handle_tcp_connection_alert));
}

//ToDo: Agregar filtro por software para sacar ruido leer diapo de FRA diapositiva 1 hay formula, aplicar filtro de ventana o ponderaciones.
//ToDO: REvisar wathchdog


Dm3Security::~Dm3Security() {
	free(_motor_module_instance);
	free(_dm3_security_instance);
}
    
void handle_bumper_down(){
	_bumper_down_alert_thread.signal_set(0x1);
}

void handle_bumper_up(){
	_bumper_up_alert_thread.signal_set(0x1);
}

void Dm3Security::handle_bumper_down_alert(){
	alert_data alert_data;
	while(true){
		_bumper_down_alert_thread.signal_wait(0x1, osWaitForever);
		alert_data.direction 	= FRONT;
		alert_data.level 		= DANGER;
		self_alert_call(_bumper_pressed_alert_callback, alert_data);
	}
}

void Dm3Security::handle_bumper_up_alert(){
	alert_data alert_data;
	while(true){
		_bumper_up_alert_thread.signal_wait(0x1, osWaitForever);
		alert_data.direction 	= FRONT;
		alert_data.level 		= OK;
		self_alert_call(_bumper_unpressed_alert_callback, alert_data);
	}
}

inline void update_fl_dist(int distance)
{
	_ultrasonic_fl_distance = distance;
   //ToDo Logic to check distance ranges
	_ultrasonic_fl_alert_thread.signal_set(0x1);
}

void Dm3Security::handle_ultrasonic_fl_distance_alert(){
	while(true) {
		_ultrasonic_fl_alert_thread.signal_wait(0x1, osWaitForever);
		_ultrasonic_fl_last_distance = filter_ultrasonic_distance(_ultrasonic_fl_distance, _ultrasonic_fl_last_distance);
		handle_ultrasonic_distance_action(FRONT);
	}
}

inline void update_fr_dist(int distance)
{
	_ultrasonic_fr_distance = distance;
   //ToDo Logic to check distance ranges
	_ultrasonic_fr_alert_thread.signal_set(0x1);
}

void Dm3Security::handle_ultrasonic_fr_distance_alert(){
//	int risk_state = 0;
	while(true) {
		_ultrasonic_fr_alert_thread.signal_wait(0x1, osWaitForever);
		_ultrasonic_fr_last_distance = filter_ultrasonic_distance(_ultrasonic_fr_distance, _ultrasonic_fr_last_distance);
		handle_ultrasonic_distance_action(FRONT);
	}
}

int Dm3Security::filter_ultrasonic_distance(int dist, int last_dist){
	int result = dist;
	if(last_dist != -1){
		result = last_dist + ULTRASONIC_FILTER_ALPHA * (dist - last_dist);
	}
	return result;
}

void Dm3Security::handle_ultrasonic_distance_action(dm3_direction_t direction) {
	int distance_min = 0;
	DigitalOut led_red(LED_RED);
	led_red = 1;
	bool disable = false;
	alert_data alert_data;
	switch (direction) {
		case FRONT:
			distance_min = _ultrasonic_fl_last_distance < _ultrasonic_fr_last_distance ? _ultrasonic_fl_last_distance : _ultrasonic_fr_last_distance;
			alert_data.distance = distance_min;
			alert_data.direction = FRONT;
			if(distance_min < ULTRASONIC_MIN_FRONT_DIST){
				float** vels = _motor_module_instance->get_current_vels();
				for (int motor = 0; motor < MOTORS_PER_CHASIS; ++motor) {
					if(vels[0][motor] > 0){
						disable = true;
						break;
					}
				}
				//delete aux data
				for (int chasis = 0; chasis < (int)NUMBER_CHASIS; ++chasis) {
					delete[] vels[chasis];
				}
				delete[] vels;

				if(disable){
					led_red = 0;
					alert_data.level = DANGER;
				}else{
					led_red = 1;
					alert_data.level = WARNING;
				}
			}else{
				led_red = 1;
				alert_data.level = OK;
			}
			self_alert_call(_ultrasonic_distance_alert_callback, alert_data);
			break;
		case RIGHT:
			break;
		case BACK:
			break;
		case LEFT:
			break;
	}

}

void Dm3Security::self_alert_call(const alert_event_t& alert_callback, alert_data& alert_data) {
	if(alert_callback){
		alert_callback.call(&alert_data);
	}
}

/*dm3_security_state_t Dm3Security::get_dm3_security_state() {
	dm3_security_state_t result;
	_dm3_security_state_mutex.lock();
	result = _dm3_swcurity_state;
	_dm3_security_state_mutex.unlock();
	return result;
}*/
/*
void Dm3Security::update_dm3_security_state(dm3_security_state_t state) {
	_dm3_security_state_mutex.lock();
	switch (state) {
		case ENABLED:
			if(_dm3_swcurity_state != ENABLED){
				_dm3_swcurity_state = ENABLED;
				_dm3_instance->enable(1);
			}
			break;
		case DISABLED_BUMPER:
			//ToDo: rise event
			break;
		case DISABLED_FRONT_DISTANCE:
		case DISABLED_RIGHT_DISTANCE:
		case DISABLED_BACK_DISTANCE:
		case DISABLED_LEFT_DISTANCE:
			if(_dm3_swcurity_state == ENABLED){
				_dm3_swcurity_state = state;
//				_dm3_instance->enable(0);

			}
			break;
	}
	_dm3_security_state_mutex.unlock();
}*/

void Dm3Security::handle_tcp_connection_alert() {
	Communication * tcp_comm;
	tcp_comm = Communication::get_instance();
	alert_data data;
	data.distance = 0;
	data.direction = FRONT;
	bool throw_alert = false;
	int lost_conn_count = 0;
	float time_to_sleep_rate = 0.5; //in secconds
	float time_to_sleep = 1000 * time_to_sleep_rate;

	while(true){
		if(!tcp_comm->is_client_connected()){
			lost_conn_count++;
			if(lost_conn_count >= TCP_CONN_RESET_TO / time_to_sleep_rate){ /**TCP_CONN_RESET_TO is in seconds**/
				data.level = DANGER;
				throw_alert = true;
			}
//			//printf("TCP CONNECTION: DSICONECTED\n");
		}else{
			data.level = OK;
//			//printf("TCP CONNECTION: CONECTED\n");
		}
		if(throw_alert){
			throw_alert = false;
			lost_conn_count = 0;
			self_alert_call(_tcp_connection_alert_callback, data);
		}
		Thread::wait(time_to_sleep);
	}

}

void Dm3Security::disable_dm3() {
	_motor_module_instance->update_motors_status(0);
}

void Dm3Security::enable_dm3() {
	_motor_module_instance->update_motors_status(1);
}

} /* namespace modules */

