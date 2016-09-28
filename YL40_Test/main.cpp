 #include "mbed.h"
 #include "PCF8591.h"

 // I2C Communication
  I2C i2c_adcdac(PTE25,PTE24); // SDA, SCL for LPC1768
  //I2C i2c_adcdac(P0_10,P0_11); // SDA, SCL for LPC812
 
  //Declare a composite ADC and DAC device that may be used through public methods
  PCF8591 capteur0(&i2c_adcdac,PCF8591_SA0); // I2C bus, Default PCF8591 Slaveaddress
 
  //Declare independent ADC and DAC objects that may be used similar to mbed AnalogIn and AnalogOut pins
  //PCF8591_AnalogOut anaOut(&i2c_bus);
  //
  //PCF8591_AnalogIn anaIn(&i2c_bus, PCF8591_ADC0);
 
  
int main() {
    uint8_t count = 0; 
    uint8_t analog;  
    printf("TEST DAC\r\n");
    while(1) {
        wait(1);    
        count++;       
 
        // Composite device methods    
        capteur0.write(count);                 // write D/A value
        analog = capteur0.read(PCF8591_ADC0);  // read A/D value for Channel 0
        printf("AO=%d AI=%d \r\n",count,analog);
        // read A/D value for Channel 0 in (Volts*10)
    } 
 }