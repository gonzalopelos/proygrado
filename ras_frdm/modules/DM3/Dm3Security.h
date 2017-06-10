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
		FRONT, RIGHT, BACK, LEFT
	} dm3_direction_t;

	/*Event handlers definitions*/
	typedef enum event_type { US_DIST_ALERT_EVENT = 0, BUMPER_ALERT_EVENT } event_type_t;
	typedef enum alert_type { OK, WARNING, DANGER } alert_type_t;
	typedef struct{
		int distance;
		dm3_direction_t direction;
		alert_type_t alert_type;
	}alert_data_t;

	void attach(event_type_t event, void (*function)(alert_data_t*)) {
		switch(event){
		case US_DIST_ALERT_EVENT:
			_ultrasonic_distance_alert_callback.attach(callback(function));
			break;
		case BUMPER_ALERT_EVENT:
			//ToDo attach to event handler function to callback
			break;
		}
	}

	template<typename T>
	void attach(event_type_t event, T *object, void (T::*member)(alert_data_t*)) {
		switch(event){
		case US_DIST_ALERT_EVENT:
			_ultrasonic_distance_alert_callback.attach(callback(object, member));
			break;
		case BUMPER_ALERT_EVENT:
			//ToDo attach to event handler function to callback
			break;
		}
	}

protected:
	typedef Callback<void(alert_data_t*)> alert_event_t;
	alert_event_t _ultrasonic_distance_alert_callback;
	int filter_ultrasonic_distance(int distance, int last_distance);
	void handle_ultrasonic_distance_action(dm3_direction_t direction);
	static void self_alert_call(const alert_event_t& alert_callback, alert_data_t& alert_data);
	// Front-left ultrasonic sensor ===================================
	void handle_ultrasonic_fl_distance_alert();
	// ================================================================

	// Front-right ultrasonic sensor ==================================
	void handle_ultrasonic_fr_distance_alert();
	// ================================================================

	// Bamper =========================================================
	void handle_bumper_alert();
	// ================================================================

private:
#define	ULTRASONIC_MIN_FRONT_DIST 300
#define BUMPER_DEBOUNCING_TIMEOUT (int) 200
#define ULTRASONIC_FILTER_ALPHA .2
	static Dm3Security * _dm3_security_instance;
	Dm3Security();
	virtual ~Dm3Security();

};

} /* namespace modules */

#endif /* MODULES_DM3_DM3SECURITY_H_ */
