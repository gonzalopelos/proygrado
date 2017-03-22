#ifndef __MBEDI2CINTERFACE_H  
#define __MBEDI2CINTERFACE_H  

#define VERSION_MBEDI2CInterface_HDR "1.00" 


class MBEDI2CInterface :public I2CInterface {
    
    I2C i2cdev;
    public :
        
    MBEDI2CInterface(PinName sda, PinName scl): 
    getVersion( VERSION_MBEDI2CInterface_HDR,VERSION_MBEDI2CInterface_HDR, __TIME__, __DATE__),i2cdev(sda,scl){
        // no init code yet 
    } ;
    // next could perhaps more efficient  but not yet investigated 
virtual void    frequency (int hz){return i2cdev.frequency(hz) ;};
virtual int     read (int address, char *data, int length, bool repeated=false){
            return i2cdev.read ( address, data, length, repeated);};
 virtual int     read (int ack){return i2cdev.read ( ack);};// Read a single byte from the I2C bus.
 virtual int     write (int address, const char *data, int length, bool repeated=false){
            return i2cdev.write ( address, data, length, repeated);
        }
virtual int     write (int data){return i2cdev.write (data);};//  Write single byte out on the I2C bus.
virtual void    start (void){i2cdev.start ();};// Creates a start condition on the I2C bus.
virtual void    stop (void){i2cdev.stop();};// Creates a stop condition on the I2C bus.
virtual int     transfer (int address, const char *tx_buffer, int tx_length, char *rx_buffer, int rx_length, void* callbackfunction,  bool repeated=false){
            return  -1;  // seems transfer not supported in mbed or not correctly called below
           //  return i2cdev.transfer (address, tx_buffer,  tx_length, rx_buffer,  rx_length, callback,  event,  repeated);
        };    
    
    
    } ;





#endif