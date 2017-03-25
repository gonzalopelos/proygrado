/*
 * PIDModule.h
 *
 *  Created on: 7 de abr. de 2016
 *      Author: bruno
 */

#include <math.h>
#ifndef MODULES_PIDMODULE_H_
#define MODULES_PIDMODULE_H_
#define UMBRAL_VEL_RESET 0.2



class PIDModule {
	float output = 0.0;
	float iTerm = 0.0;
	float lastInput;
	float outMin, outMax;
	float sampleTime = 1000;
	float kp, ki, kd;
	int inRegulate = 0;
	float lastSetPoint = 0;
	float error = 0;
	float dInput = 0;
	void initialize();
	void setMode(int Mode);
	void setOutputLimits(float Min, float Max);
	//sacar!!
	float setPoint_deb=0.0;
	float input_deb=0.0;
	//Sacar!!
public:
	PIDModule();
	virtual ~PIDModule();
	void new_set_vel();
	float compute(float input, float setPoint);
	void setSampleTime(int t);
	void setTunings(float Kp, float Ki, float Kd);
	float getKp();
	float getKi();
	float getKd();
	float getITerm();
	float getDInput();
	float getError();
	//Sacar!!
	float getSetPointDeb();
	float getInputDeb();
	//Sacar!!
};

#endif /* MODULES_PIDMODULE_H_ */
