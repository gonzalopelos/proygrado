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

double input;  		// la velocidad actual del motor
double output; 		// la función de salida del PID que controla la planta (potencia del motor)
double setPoint;	// la velocidad deseada del motor
double lastInput; // el termino integral que se va calculando iterativamente y la velocidad inicia para calcular la derivada
double kp, ki, kd;
int sampleTime = 1000; // Tiempo de muestreo 1 segundo.
double outMin, outMax; // los limites para la salidad del PID (potencias de motor {20,80}). 20 o cero?
int inRegulate = 0;


#define MANUAL 0
#define AUTOMATIC 1

void PIDModule::initialize() {
	lastInput = input;
	iTerm = output;
	if(iTerm> outMax) iTerm= outMax;
	else if(iTerm< outMin) iTerm= outMin;
}

void PIDModule::new_set_vel() {
	iTerm = 0;
}

void PIDModule::setTunings(double Kp, double Ki, double Kd) {
	double sampleTimeInSec = ((double)sampleTime)/1000;
	kp = Kp;
	ki = Ki * sampleTimeInSec;
	kd = Kd / sampleTimeInSec;
}

void PIDModule::setSampleTime(int newSampleTime) {
	if (newSampleTime > 0) {
		double ratio = (double)newSampleTime / (double)sampleTime;
		ki *= ratio;
		kd /= ratio;
		sampleTime = (unsigned long)newSampleTime;
	}
}

void PIDModule::setOutputLimits(double Min, double Max) {
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
	float error = setPoint - input;
	iTerm+= (ki * error);
	if(iTerm> outMax) iTerm= outMax;
	else if(iTerm< outMin) iTerm= outMin;
	float dInput = (input - lastInput);
	// Calculamos la función de salida del PID.
	output = kp * error + iTerm - kd * dInput;
	if(output > outMax) output = outMax;
	else if(output < outMin) output = outMin;
	// Guardamos el valor de algunas variables para el próximo recálculo.

	lastInput = input;
	return output;
}





PIDModule::PIDModule() {
	// TODO Auto-generated constructor stub
	initialize();
	setTunings(10.0, 0.2, 0.2);
	setMode(1);
	setOutputLimits(0,100);
}

PIDModule::~PIDModule() {
	// TODO Auto-generated destructor stub
}

