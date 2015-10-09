/*
 * File:   main.c
 * Author: root
 *
 * Created on September 9, 2015, 6:36 PM
 */

#include "configs.h"
#include <xc.h>

unsigned long _maxTimeOut;
int _analogValue;

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
    //ver p[agina 288 de la cartilla https://drive.google.com/open?id=0B5b9cF_5KTdQd3g5VUFWa3VheFU
    //ver http://www.learningaboutelectronics.com/Articles/PIC-analog-to-digital-conversion-ADC.php
    
    TRISA = 0; 
    TRISB = 0;
    TRISAbits.RA3 = 1; 
    ANSELAbits.ANSA3=1;
    
    ADCON2bits.ADFM=1;
    ADCON2bits.ACQT=1;
    ADCON2bits.ADCS=2;
    
    ADCON1bits.PVCFG0 =0;
    ADCON1bits.NVCFG1 =0;
    
    ADCON0bits.CHS = 3;
    
    ADCON0bits.ADON = 1;
    
    _maxTimeOut = 131072;
}

void main(void) {
    initialize();
    LATBbits.LATB2 = 0;
    while (1) {

        ADCON0bits.GODONE=1;
        
        while(ADCON0bits.GODONE==1);
        
        _analogValue = ADRESL + (ADRESH *256);
        
        if (_analogValue > 700){
            LATBbits.LATB2 = 1;
        }
        else{
            LATBbits.LATB2 = 0;
        }
        
    }

}




