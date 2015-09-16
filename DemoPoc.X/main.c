/*
 * File:   main.c
 * Author: root
 *
 * Created on September 9, 2015, 6:36 PM
 */

#include "configs.h"
#include <xc.h>


void sleep(void)
{
    int clock = 0;
    int timeout = 1000000;
    while(clock < timeout)
    {
        clock = clock + 1;
    }
}


void main(void) {
    TRISD = 0;
    while (1) {
        LATDbits.LATD0 = 1;
        sleep();
        LATDbits.LATD0 = 0;
        sleep();
    }

}

