#ifndef BUMPER_H
#define BUMPER_H

#include "mbed.h"

namespace modules {

class Bumper {

private:
    InterruptIn _bumper;
    Timeout     _bumperDownTimeout;
    
    volatile bool _bumperCanPress;
    void (*_callback)(void);

    void click();
    void call();
    void reset();


public:
    // ********************************************************************************
    // * Constructor
    // *
    // * @param bumperPin   The pin which is connected to the bumper.
    // ********************************************************************************
    Bumper(PinName bumperPin);

    // ********************************************************************************
    // * Attaches the method to be called when the bumper is pressed.
    // *
    // * @param method      A reference to the method to be called.
    // ********************************************************************************
    void attach(void (*method)(void) = 0);

    // ********************************************************************************
    // * The time in milliseconds in which the bumper can not be pressed again.
    // *
    // * @default          20ms
    // ********************************************************************************
    int timeout;
};

}
#endif
