#include "../modules/DM3/Dm3.h"


#include <stddef.h>

// NEED REVIEW ->
//#include "I2C.h"
#include "I2CInterface.h"
#include "../libs_I2C/MBEDI2CInterface.h" 
//#include "DACInterface.h" 
//#include "dev_interface_def.h"
#include "../libs_I2C/mcp4728.h"


//MBEDI2CInterface mbedi2c(p28,p27);
MBEDI2CInterface mbedi2c(PTE25, PTE24);	   // 2017.03.18 AM - PTE25 = SDA y PTE24 = SDL en FRDM K64F
MBEDI2CInterface* mbedi2cp =  &mbedi2c ;
I2CInterface* i2cdev = mbedi2cp;
const float Vdd = 5.0;

MCP4728 dac(i2cdev, 0, Vdd);
// I2C i2c(p28, p27);

// NEED REVIEW

AnalogIn pot(POT_IN);
AnalogIn batt(BATT_IN);

//we are not usig p5 anymore because malfunction
//DigitalOut pio_reverse[4]={p9, p6, p7, p8};
DigitalOut pio_reverse[4]={PTB18, PTB19, PTC1, PTC8};	// 2017.03.18 AM - Ultimos 4 pines fila interior de J1
//DigitalOut pio_brake(p29);
DigitalOut pio_brake(PTC17);							// 2017.03.18 AM - Pin nro 1 fila exterior de J1
//DigitalOut pio_enable(p30);
DigitalOut pio_enable(PTC16);							// 2017.03.18 AM - Pin nro 2 fila exterior de J1
DigitalOut pio_horn(PTB20);								// 2017.06.13 GP - Pin nro 9 fila interior de J4

//Serial serial(p13, p14); // tx, rx
//Serial serial(USBTX, USBRX);	// 2017.03.18 AM

char i2c_cmd[2];

Dm3* Dm3::m_pInstance = NULL;
int _logical_pio_enable;
int _logical_pio_brake;

Dm3::Dm3() {
	// ->
	// i2c_cmd[0] = MOTOR_DATA_ADDRESS;
	// for (int i=0;i<4;i++) {
	// 	Dm3::motor_i2c(i, 0.0);
	// 	pio_reverse[i] = 0;
	// }
	// NEED REVIEW ->
	//     dac.setRefIntern(); // set to use internal voltage
    dac.setRefExtern();
    dac.setPowerModeNormal();

	i2c_cmd[0] = MOTOR_DATA_ADDRESS;
	for (int i=0;i<4;i++) {
		dac.setDACvalue(0, i);
		// if ( dac.setDACvalue(0, i) )
			// TODO: report error in serial Dm3::motor_i2c(i, 0.0);
	// NEED REVIEW
		pio_reverse[i] = 0;
	}
	pio_enable = 0;
	pio_brake = 0;
	_logical_pio_enable = 0;
	_logical_pio_brake = 0;
 	//pio_horn = 0;
}

Dm3 *Dm3::Instance() {
	if (!m_pInstance)   // Only allow one instance of class to be generated.
		m_pInstance = new Dm3();
	return m_pInstance;
}

int Dm3::motor_i2c(uint8_t motor, float speed) {
	speed = speed * Dm3::motor_mult[motor];
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

	i2c_cmd[1] = (uint8_t)round(outvel);

	/*
	 *
	 if (motor==0) {
		serial.putc(0);
		serial.putc(i2c_cmd[1]);
	}
	*/

	//FIXME
	int result;
	result =  dac.setDACvalue(outvel, motor); // i2c.write(motor_i2c_address[motor], i2c_cmd, 2);

	if(result != 0){
	   printf("\n\nset_motors_power ERROR: %d\n\n", result);
	}

	return result;
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
	pio_brake = 1 - mode; // el freno se maneja por logica negada desde la para de E/S al rele
	_logical_pio_brake = 1 - mode;
	//FixMe return 1 - pio_brake
	return 1 - _logical_pio_brake;
}

int Dm3::brake() {
	//FixMe return 1 - pio_brake
	return 1 - _logical_pio_brake;
}

int Dm3::enable(int mode) {
	pio_enable = mode;
	_logical_pio_enable = mode;
	//FixMe return pio_enable;
	return _logical_pio_enable;
}

int Dm3::enable() {
//FixMe	return pio_enable;
	return _logical_pio_enable;
}

int Dm3::horn(int mode) {
	pio_horn = mode;
	return pio_horn;
}

int Dm3::horn() {
	return pio_horn;
}

