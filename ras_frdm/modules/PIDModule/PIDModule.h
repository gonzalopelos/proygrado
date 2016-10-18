/*
 * PIDModule.h
 *
 *  Created on: 7 de abr. de 2016
 *      Author: bruno
 */

#ifndef _PIDMODULE_H_
#define _PIDMODULE_H_


class PIDModule {
	double output = 0.0;
	double input = 0.0;
	double iTerm = 0.0;
	void initialize();
	void setMode(int Mode);
	void setOutputLimits(double Min, double Max);
public:
	PIDModule();
	virtual ~PIDModule();
	void new_set_vel();
	float compute(float input, float setPoint);
	void setSampleTime(int t);
	void setTunings(double Kp, double Ki, double Kd);

};

#endif /* MODULES_PIDMODULE_H_ */
