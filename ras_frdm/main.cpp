#include "mbed.h"
#include "PCF8591.h"
#include "PIDModule.h"

I2C i2c(PTE25, PTE24); // SDA, SCL para K64F
PCF8591 DAC_I2C(&i2c, PCF8591_SA0); // I2C bus, Default PCF8591 Slaveaddress
  
int main() {
	PIDModule pid[1];
	uint8_t count = 0;
	float vel = 0;
    while(1) {
    	wait(0.1);
    	count++;

    	vel = pid[1].compute(0, 10);

    	DAC_I2C.write(count);
    }
}
