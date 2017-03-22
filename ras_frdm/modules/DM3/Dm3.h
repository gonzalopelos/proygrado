/*
 * Dm3a.h
 *
 *  Created on: Oct 27, 2015
 *      Author: jvisca
 */

#ifndef MODULES_DM3_H_
#define MODULES_DM3_H_


#define CALIBRATION_DATA

// #define MIN_VEL_I2C 0x60
// #define MAX_VEL_I2C 0xc9
// NEED REVIEW ->
// New Range Tested with Motors
#define MIN_VEL_I2C 1200 // By Gonzalo 1443 // 1.70 V
//#define MAX_VEL_I2C 3900 // 4.60 V
#define MAX_VEL_I2C 2713 // 3.2 V CUENTA MAGICA LUCAS
// NEED REVIEW

#define MIN_VEL_THRESHOLD 1.0

#define POT_IN PTB2	   // 2017.03.18 AM
#define BATT_IN PTB3	// 2017.03.18 AM

#define HALL0_IO PTC5	// 2017.03.18 AM - Pin nro 1 fila interior de J1
#define HALL1_IO PTC7	// 2017.03.18 AM - Pin nro 2 fila interior de J1
#define HALL2_IO PTC0	// 2017.03.18 AM - Pin nro 3 fila interior de J1
#define HALL3_IO PTC9	// 2017.03.18 AM - Pin nro 4 fila interior de J1

#define HALL0_INT PTD2	// 2017.03.18 AM - Pin nro 7 fila exterior de J2
#define HALL1_INT PTD0	// 2017.03.18 AM - Pin nro 8 fila exterior de J2
#define HALL2_INT PTD4	// 2017.03.18 AM - Pin nro 9 fila exterior de J2
#define HALL3_INT PTC12	// 2017.03.18 AM - Pin nro 10 fila exterior de J2
#define HALL4_INT PTC3	// 2017.03.18 AM - Pin nro 1 fila exterior de J1
#define HALL5_INT PTC2	// 2017.03.18 AM - Pin nro 2 fila exterior de J1

#define MOTOR_DATA_ADDRESS 0x40

#include <stdint.h>
#include "mbed.h"

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
