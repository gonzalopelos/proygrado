/*
 * Dm3a.cpp
 *
 *  Created on: Oct 27, 2015
 *      Author: jvisca
 */

#include <stddef.h>
#include "Dm3.h"

AnalogIn pot(POT_IN);
AnalogIn batt(BATT_IN);

I2C i2c(I2C_SDA, I2C_SCL);	// PINES PTA25 y PTA24
DigitalOut pio_reverse[4] = {PTC8, PTC1, PTB19, PTB18};	// Ultimos 4 pines fila interior de J1
DigitalOut pio_brake(PTC3);								// Primer pin fila exterior de J1
DigitalOut pio_enable(PTC2);							// Segundo pin fila exterior de J1
DigitalOut pio_horn(PTA2);								// Tercer pin fila exterior de J1
Serial serial(USBTX, USBRX);

char i2c_cmd[2];

Dm3* Dm3::m_pInstance = NULL;

Dm3::Dm3() {
	i2c_cmd[0] = MOTOR_DATA_ADDRESS;
	for (int i=0;i<4;i++) {
		Dm3::motor_i2c(i, 0.0);
		pio_reverse[i] = 0;
	}
	pio_enable = 0;
	pio_brake = 0;
	pio_horn = 0;
}

Dm3 *Dm3::Instance() {
	if (!m_pInstance)   // Only allow one instance of class to be generated.
		m_pInstance = new Dm3();
	return m_pInstance;
}

int Dm3::motor_i2c(uint8_t motor, float speed) {
	speed=speed * Dm3::motor_mult[motor];
	if (speed<0) {
		pio_reverse[motor] = 1;
		speed=-speed;
	} else {
		pio_reverse[motor] = 0;
	}
    int outvel = 0;
    if (speed>MIN_VEL_THRESHOLD) {
    	outvel = MIN_VEL_I2C + ((MAX_VEL_I2C-MIN_VEL_I2C) * speed / 100);
        if (outvel>MAX_VEL_I2C) outvel=MAX_VEL_I2C;
    }

	i2c_cmd[1] = (uint8_t)outvel;

	/*
	 *
	 if (motor==0) {
		serial.putc(0);
		serial.putc(i2c_cmd[1]);
	}
	*/

	//FIXME
	return i2c.write(motor_i2c_address[motor], i2c_cmd, 2);
	//return 0;
}
float Dm3::get_pot() {
	//FIXME
	return pot;
	//return 0.5;
}
float Dm3::get_batt() {
	//FIXME
	return batt;
	//return 0.5;
}

int Dm3::brake(int mode) {
	pio_brake.write(1-mode); // el freno se maneja por logica negada desde la para de E/S al rele
	return 1-pio_brake.read();
}

int Dm3::brake() {
	return 1-pio_brake.read();
}

int Dm3::enable(int mode) {
	pio_enable = mode;
	return pio_enable;
}

int Dm3::enable() {
	return pio_enable;
}

int Dm3::horn(int mode) {
	pio_horn = mode;
	return pio_horn;
}

int Dm3::horn() {
	return pio_horn;
}
