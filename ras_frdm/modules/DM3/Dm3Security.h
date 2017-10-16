/*
 * Dm3Security.h
 *
 *  Created on: May 18, 2017
 *      Author: gonzalopelos
 */

#ifndef MODULES_DM3_DM3SECURITY_H_
#define MODULES_DM3_DM3SECURITY_H_

#include "mbed.h"
namespace modules {

class Dm3Security {
public:
	static Dm3Security * get_instance();

	typedef enum dm3_direction{
		FRONT = 0, RIGHT = 1, BACK = 2, LEFT = 3
	} dm3_direction_t;

	/*Event handlers definitions*/
	typedef enum e_security_device_type{
		ULTRASONIC = 0, BUMPER = 1, TCP_CONNECTION = 2, SPEEDS_CHECK = 3, WATCHDOG = 4, POWER_CHECK = 5, MOTORS_STATUS = 6
	}security_device_type;
	typedef enum e_alert_level { OK = 0, WARNING = 1, DANGER = 2 } alert_level;
	typedef struct s_alert_data{
		int distance;
		dm3_direction_t direction;
		alert_level level;
	}alert_data;

	void attach(security_device_type sd_type, void (*function)(alert_data*)) {
		switch(sd_type){
		case ULTRASONIC:
			_ultrasonic_distance_alert_callback.attach(callback(function));
			break;
		case BUMPER:
			_bumper_pressed_alert_callback.attach(callback(function));
			_bumper_unpressed_alert_callback.attach(callback(function));
			break;
		case TCP_CONNECTION:
			_tcp_connection_alert_callback.attach(callback(function));
			break;
		case SPEEDS_CHECK:
			_speeds_check_alert_callback.attach(callback(function));
			break;
		case POWER_CHECK:
			_power_alert_callback.attach(callback(function));
			break;
		case MOTORS_STATUS:
			_motors_status_callback.attach(callback(function));
			break;
		}
	}

	template<typename T>
	void attach(security_device_type sd_type, T *object, void (T::*member)(alert_data*)) {
		switch(sd_type){
		case ULTRASONIC:
			_ultrasonic_distance_alert_callback.attach(callback(object, member));
			break;
		case BUMPER:
			_bumper_pressed_alert_callback.attach(callback(object, member));
			_bumper_unpressed_alert_callback.attach(callback(object, member));
			break;
		case TCP_CONNECTION:
			_tcp_connection_alert_callback.attach(callback(object, member));
			break;
		case SPEEDS_CHECK:
			_speeds_check_alert_callback.attach(callback(object, member));
			break;
		case POWER_CHECK:
			_power_alert_callback.attach(callback(object, member));
			break;
		case MOTORS_STATUS:
			_motors_status_callback.attach(callback(object, member));
			break;
		}
	}
	void disable_dm3();
	void enable_dm3();
protected:
	typedef Callback<void(alert_data*)> alert_event_t;
	alert_event_t _ultrasonic_distance_alert_callback;
	alert_event_t _tcp_connection_alert_callback;
	alert_event_t _bumper_pressed_alert_callback;
	alert_event_t _bumper_unpressed_alert_callback;
	alert_event_t _speeds_check_alert_callback;
	alert_event_t _power_alert_callback;
	alert_event_t _motors_status_callback;
	int filter_ultrasonic_distance(int distance, int last_distance);
	void handle_ultrasonic_distance_action(dm3_direction_t direction);
	static void self_alert_call(const alert_event_t& alert_callback, alert_data& alert_data);

	// Front-left ultrasonic sensor ===================================
	void handle_ultrasonic_fl_distance_alert();
	// ================================================================

	// Front-right ultrasonic sensor ==================================
	void handle_ultrasonic_fr_distance_alert();
	// ================================================================

	// Bumper =========================================================
	void handle_bumper_down_alert();
	void handle_bumper_up_alert();
	// ================================================================

	// TCP Connection =================================================
	void handle_tcp_connection_alert();
	// ================================================================

	/**
	 * chequea consistencia entre la variacón de potencia y velocidad
	 * y velocidad máxima.
	 **/
	void check_speed_and_power();

	void handle_motors_status_changed();

private:
#define	ULTRASONIC_MIN_FRONT_DIST 200
#define BUMPER_DEBOUNCING_TIMEOUT (int) 100
#define ULTRASONIC_FILTER_ALPHA .2
#define TCP_CONN_RESET_TO (int) 4 //time out in seconds
#define SPEED_MAX_VALUE (float) 2.9 // m/s
/**
 * EXCEEDS_MAX_SPEED_TIME_RATE * 100ms de esoera en el bucle = EXCEEDS_MAX_SPEED_TIME_RATE * 0.1s.
 */
#define EXCEEDS_MAX_SPEED_TIME_RATE (int) 20
/**
 * calcular este factor según el peso de carga y la inersia.
 */
#define MAX_POWS_SPEED_DESVIATION (float)25
/**
 * POWS_SPEED_INCONSISTENT_TIME_RATE * 100ms de esoera en el bucle = POWS_SPEED_INCONSISTENT_TIME_RATE * 0.1s.
 */
#define POWS_SPEED_INCONSISTENT_TIME_RATE (int) 20

	static Dm3Security * _dm3_security_instance;
	Dm3Security();
	virtual ~Dm3Security();
};

} /* namespace modules */

#endif /* MODULES_DM3_DM3SECURITY_H_ */
