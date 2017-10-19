/*
 * Dm3Security.cpp
 *
 *  Created on: May 18, 2017
 *      Author: gonzalopelos
 */

#include <Bumper.h>
#include <cmsis_os.h>
#include <Callback.h>
#include <Communication.h>
#include <DigitalOut.h>
#include <Dm3.h>
#include <Dm3Security.h>
#include <MotorModule.h>
#include <Mutex.h>
#include <PinNames.h>
#include <stdlib.h>
#include <Thread.h>
#include <utilities/math_helper.h>
#include <Ultrasonic.h>



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

// Speed checks ===================================================
Thread _speed_checks_thread;
float compute_speed_variation(MotorModule::motors_info motors_info, int motor);
float compute_pows_variation(MotorModule::motors_info motors_info, int motor);
float compute_pows_and_speed_diff(float speed_variation, float pows_variation);
bool check_angular_speed_direction(float speed, float pow, bool reversed);


// Motors status
Thread _motors_status_thread;




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

	//speed checks
	_speed_checks_thread.start(callback(this, &Dm3Security::check_speed_and_power));

	_motors_status_thread.start(callback(this, &Dm3Security::handle_motors_status_changed));


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
				MotorModule::motors_info motors_info = _motor_module_instance->get_motors_info();
				for (int motor = 0; motor < MOTORS_PER_CHASIS; ++motor) {
					if(motors_info.current_vels[0][motor] > 0 && !motors_info.reverse_enabled ){
						disable = true;
						break;
					}
				}
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
	bool throw_alert, connection_lost = false;
	int lost_conn_count = 0;
	float time_to_sleep_rate = 1; //in secconds
	float time_to_sleep = 1000 * time_to_sleep_rate;

	while(true){
		if(!tcp_comm->is_client_connected()){
			lost_conn_count++;
			if(lost_conn_count >= TCP_CONN_RESET_TO / time_to_sleep_rate){ /**TCP_CONN_RESET_TO is in seconds**/
				data.level = DANGER;
				throw_alert = true;
				connection_lost = true;
			}
//			printf("TCP CONNECTION: DSICONECTED\n");
		}else{
			data.level = OK;
			if(connection_lost){
				connection_lost = false;
				throw_alert = true;
			}
//			printf("TCP CONNECTION: CONECTED\n");
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

void Dm3Security::check_speed_and_power() {
	MotorModule::motors_info motors_info;

	float speed_variation;
	float pows_variation;
	bool exceeds_maximum_speed = false;
	int exceeds_maximum_speed_times = 0;
	bool power_speed_inconsistency = false;
	int power_speed_inconsistency_times = 0;
	bool angular_speed_direction_consistent = false;
	alert_data data;
	data.distance = 0;
	data.direction = FRONT;
	while(true){
		motors_info = _motor_module_instance->get_motors_info();
		/**
		 * Sólo se considera el chasis nro1 para controlar
		 * las velocidades y potencias.
		 */
		for (int motor = 0; motor < MOTORS_PER_CHASIS; ++motor) {

			/**
			 * Check angular speed direction.
			 */
			angular_speed_direction_consistent = check_angular_speed_direction(motors_info.current_vels[0][motor], motors_info.current_pow[0][motor], motors_info.reverse_enabled);
			if(!angular_speed_direction_consistent){
				printf("SPEED[%d] = %f || POWS[%d] = %f || reversed: %d\n\n",motor, motors_info.current_vels[0][motor], motor, motors_info.current_pow[0][motor], motors_info.reverse_enabled);
				break;
			}

			/**
			 * Maximum speeds checks
			 */
			if(motors_info.current_vels[0][motor] > utilities::math_helper::abs(SPEED_MAX_VALUE)){
				exceeds_maximum_speed = true;
			}

			/**
			 * Consistency check between speeds and powers
			 */
			speed_variation = compute_speed_variation(motors_info, motor);

			pows_variation = compute_pows_variation(motors_info, motor);

			if(compute_pows_and_speed_diff(speed_variation, pows_variation) > MAX_POWS_SPEED_DESVIATION){
				power_speed_inconsistency = true;
			}

//			printf("SPEED_VARIATION[%d] = %f || POWS_VARIATION[%d] = %f || VARIATION = %f\n\n",motor, speed_variation, motor, pows_variation, compute_pows_and_speed_diff(speed_variation, pows_variation));
		}

		if(!angular_speed_direction_consistent){
			data.level = DANGER;
			self_alert_call(_speeds_check_alert_callback, data);
		}else if(exceeds_maximum_speed){
			exceeds_maximum_speed_times++;
			if(exceeds_maximum_speed_times >= EXCEEDS_MAX_SPEED_TIME_RATE){
				data.level = DANGER;
				self_alert_call(_speeds_check_alert_callback, data);
				exceeds_maximum_speed_times = 0;
			}
		}else{
			data.level = OK;
			self_alert_call(_speeds_check_alert_callback, data);
			exceeds_maximum_speed_times = 0;
		}

		if(power_speed_inconsistency){
			power_speed_inconsistency_times++;
			if(power_speed_inconsistency_times >= POWS_SPEED_INCONSISTENT_TIME_RATE){
				data.level = DANGER;
				self_alert_call(_power_alert_callback, data);
				power_speed_inconsistency_times = 0;
			}
		}else{
			data.level = OK;
			self_alert_call(_power_alert_callback, data);
			power_speed_inconsistency_times = 0;
		}

		exceeds_maximum_speed = false;
		power_speed_inconsistency = false;

		Thread::wait(100);
	}


}

void Dm3Security::handle_motors_status_changed(){
	alert_data data;
	data.distance = 0;
	data.direction = FRONT;


	while(true){
		data.level = _dm3_instance->enable() == 1 ? OK : DANGER;
		self_alert_call(_motors_status_callback, data);
		Thread::wait(500);
	}


}
float compute_speed_variation(MotorModule::motors_info motors_info, int motor){
	float result = -1;

	result = utilities::math_helper::abs(motors_info.current_vels[0][motor]) * 100 / SPEED_MAX_VALUE;

	return result;
}

inline float compute_pows_variation(MotorModule::motors_info motors_info, int motor) {\
	return utilities::math_helper::abs(motors_info.current_pow[0][motor]);
}

inline float compute_pows_and_speed_diff(float speed_variation, float pows_variation) {
	return utilities::math_helper::abs(speed_variation - pows_variation);
}

inline bool check_angular_speed_direction(float speed, float pow, bool reversed) {
	/**
	 * si la velocidad es negativa es porque está en dirección contraria a lo indicado
	 * por el flag reversed.
	 */
	return speed >= 0;
}

} /* namespace modules */

