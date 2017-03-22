#ifndef __I2CINTERFACE__
#define __I2CINTERFACE__

#include "getVersion.h"

#define I2CINTERFACE_HDR_VER "0.33" 

class I2CInterface : public virtual getVersion{
private: 
protected :
     void*  callback();

public : 
    I2CInterface():getVersion( I2CINTERFACE_HDR_VER ,I2CINTERFACE_HDR_VER , __TIME__, __DATE__){};   //Create an I2C Master interface
virtual void    frequency (int hz){};//  Set the frequency of the I2C interface.
virtual int     read (int address, char *data, int length, bool repeated=false){
            return 0;};//Read from an I2C slave.
virtual int     read (int ack){return 0;};// Read a single byte from the I2C bus.
virtual int     write (int address, const char *data, int length, bool repeated=false){
            return 0;
        };// Write to an I2C slave.
virtual int     write (int data){return 0;};//  Write single byte out on the I2C bus.
virtual void    start (void){};// Creates a start condition on the I2C bus.
virtual void    stop (void){};// Creates a stop condition on the I2C bus.
virtual int     transfer (int address, const char *tx_buffer, int tx_length, char *rx_buffer, int rx_length, void*  callbackptr, bool repeated=false){
            return 0;
        };     //   Start non-blocking I2C transfer.       not yet clear how to deal with the callback
             // proposol here is for the implementation a spefic call back function ,that includes the event type  
    
}; 

#endif
