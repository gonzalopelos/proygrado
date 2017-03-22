#ifndef __DACINTERFACE__
#define __DACINTERFACE__


#define VERSION_DACINTERFACE_HDR "0.40"
#include "getVersion.h"

class DACInterface: public virtual getVersion{
private: 


public : 
    DACInterface():getVersion( VERSION_DACINTERFACE_HDR ,VERSION_DACINTERFACE_HDR , __TIME__, __DATE__){};   //Create an I2C Master interface
    
virtual int     setDACvalue(int dacvalue, int ch=0){   return 0;};
virtual int     getDACvalue (int& dacvalue , int ch=0){return 0;};
virtual int     setVoltage (float voltage, int ch=0){    return 0;};
virtual int     getVoltage(float &voltage, int ch=0){return 0;};
virtual int     update() {return 0;};  // general update  by example readout all registers in the device to the class storage. 
    
}; 

#endif
