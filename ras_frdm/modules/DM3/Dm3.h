/*
 * Dm3a.h
 *
 *  Created on: Oct 27, 2015
 *      Author: jvisca
 */

#include <stdint.h>
#include "mbed.h"

#ifndef MODULES_DM3_H_
#define MODULES_DM3_H_

#define CALIBRATION_DATA

#define MIN_VEL_I2C 0x60
#define MAX_VEL_I2C 0xc9
#define MIN_VEL_THRESHOLD 1.0

#define POT_IN PTB2
#define BATT_IN PTB3

#define HALL0_IO PTC5	// Primer pin fila interior de J1
#define HALL1_IO PTC7	// Segundo pin fila interior de J1
#define HALL2_IO PTC0	// Tercer pin fila interior de J1
#define HALL3_IO PTC9	// Cuarto pin fila interior de J1

#define HALL0_INT PTD2	// Pin nro 7 fila exterior de J2
#define HALL1_INT PTD0	// Pin nro 8 fila exterior de J2
#define HALL2_INT PTD4	// Pin nro 9 fila exterior de J2
#define HALL3_INT PTC12	// Pin nro 10 fila exterior de J2

#define MOTOR_DATA_ADDRESS 0x40

class Dm3 {
public:
   int motor_i2c_address[4] = {0x48<<1, 0x4c<<1, 0x4f<<1, 0x4e<<1};
   float motor_mult[4] = {1.0,-1.0,-0.2,0.2};

   static Dm3* Instance();
   int motor_i2c(uint8_t motor, float speed);
   float get_pot();
   float get_batt();
   AnalogIn get_hall(int i);

   int brake(int);
   int brake();

   int enable(int);
   int enable();

   int horn(int);
   int horn();

private:
   Dm3();
   static Dm3* m_pInstance;
};

#endif /* MODULES_DM3_H_ */
