/*
 * File:   main.c
 * Author: root
 *
 * Created on September 9, 2015, 6:36 PM
 */

#include "configs.h"
#include <xc.h>

unsigned long _maxTimeOut;

void sleep(unsigned long timeout)
{
    unsigned long clock = 0;
    
    
    while(clock < timeout)
    {
        clock = clock + 1;
    }
}

void checkButtonStatus(void) {
    if(PORTAbits.RA2) {
        _maxTimeOut = _maxTimeOut / 2; 
    }
    
    if (_maxTimeOut == 1)
       _maxTimeOut = 131072;
}

void initialize(void) {
    //TRISD = 0; // se configuran los pines asociados al puerto D para salida
    TRISA = 0; // se configuran todos los pines asociados al puerto C para salida
    TRISAbits.RA2 = 1; // se configura el pin asociado al bit 0 del puerto A para lectura de entrada anal[ogica]
    ANSELAbits.ANSA2= 0;               // set pin as digital
    _maxTimeOut = 131072;
}

void main(void) {
    initialize();
    while (1) {
        LATAbits.LATA0 = 1;
        sleep(_maxTimeOut);
        LATAbits.LATA0 = 0;
        sleep(_maxTimeOut);
        checkButtonStatus();
    }

}




