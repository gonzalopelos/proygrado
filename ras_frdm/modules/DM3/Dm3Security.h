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
		ULTRASONIC = 0, BUMPER = 1, TCP_CONNECTION = 2
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

private:
#define	ULTRASONIC_MIN_FRONT_DIST 300
#define BUMPER_DEBOUNCING_TIMEOUT (int) 100
#define ULTRASONIC_FILTER_ALPHA .2
#define TCP_CONN_RESET_TO (int) 4 //time out in seconds
	static Dm3Security * _dm3_security_instance;
	Dm3Security();
	virtual ~Dm3Security();

};

} /* namespace modules */

#endif /* MODULES_DM3_DM3SECURITY_H_ */
