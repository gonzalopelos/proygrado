/*
 * PIDModule.cpp
 *
 *  Created on: 7 de abr. de 2016
 *      Author: bruno
 */
/*

 *
 *  Created on: 7 de abr. de 2016
 *      Author: bruno
 */
#include "PIDModule.h"

 		// la velocidad actual del motor
		// la función de salida del PID que controla la planta (potencia del motor)
	// la velocidad deseada del motor
 // el termino integral que se va calculando iterativamente y la velocidad inicia para calcular la derivada
 // Tiempo de muestreo 1 segundo.
 // los limites para la salidad del PID (potencias de motor {20,80}). 20 o cero?



#define MANUAL 0
#define AUTOMATIC 1


void PIDModule::initialize() {
	lastInput = 0.0;
	iTerm = output;
	if(iTerm> outMax) iTerm= outMax;
	else if(iTerm< outMin) iTerm= outMin;
}

void PIDModule::new_set_vel() {
	iTerm = 0;
}

void PIDModule::setTunings(float Kp, float Ki, float Kd) {
	float sampleTimeInSec = ((float)sampleTime)/1000;
	kp = Kp;
	ki = Ki * sampleTimeInSec;
	kd = Kd / sampleTimeInSec;
}

void PIDModule::setSampleTime(int newSampleTime) {
	if (newSampleTime > 0) {
		float ratio = (float)newSampleTime / (float)sampleTime;
		ki *= ratio;
		kd /= ratio;
		sampleTime = (unsigned long)newSampleTime;
	}
}

void PIDModule::setOutputLimits(float Min, float Max) {
	if(Min > Max) return;
	outMin = Min;
	outMax = Max;
	if(output > outMax) output = outMax;
	else if(output < outMin) output = outMin;
	if(iTerm> outMax) iTerm= outMax;
	else if(iTerm< outMin) iTerm= outMin;
}

void PIDModule::setMode(int Mode) {
	int newAuto = (Mode == AUTOMATIC);
	if(newAuto && !inRegulate) { // Para cambiar de manual a automático, inicializamos algunos parámetros.
		initialize();
	}

	inRegulate = newAuto;
}

float PIDModule::compute(float input, float setPoint) {	
	if(!inRegulate) return setPoint;

	// Calculamos todos los errores.
	error = setPoint - input;
	if (fabs(lastSetPoint - setPoint) > UMBRAL_VEL_RESET){
		iTerm = 0;
	}else{
		iTerm = iTerm + (ki * error);
		if(iTerm> outMax) iTerm= outMax;
		else if(iTerm< outMin) iTerm= outMin;
	}	
	dInput = (input - lastInput);
	// Calculamos la función de salida del PID.
	output = kp * error + iTerm + kd * dInput;
	if(output > outMax) output = outMax;
	else if(output < outMin) output = outMin;
	// Guardamos el valor de algunas variables para el próximo recálculo.

	lastInput = input;
	lastSetPoint = setPoint;


	//sacar!
	input_deb = input;
	setPoint_deb = setPoint;

	//sacar!
	return output;
}

//sacar!!
float PIDModule::getInputDeb(){
	return input_deb;
}

float PIDModule::getSetPointDeb(){
	return setPoint_deb;
}
//sacar!!

float PIDModule::getKp(){
	return kp;
}

float PIDModule::getKi(){
	return ki;
}

float PIDModule::getKd(){
	return kd;
}

float PIDModule::getITerm(){
	return iTerm;
}

float PIDModule::getError(){
	return error;
}

float PIDModule::getDInput(){
	return dInput;
}
PIDModule::PIDModule() {
	// TODO Auto-generated constructor stub
	initialize();
	setTunings(0.0, 0.0, 0.0);
	setMode(1);
	setOutputLimits(0,100);
}

PIDModule::~PIDModule() {
	// TODO Auto-generated destructor stub
}

