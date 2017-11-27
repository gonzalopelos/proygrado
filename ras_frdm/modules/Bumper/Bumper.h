#ifndef BUMPER_H
#define BUMPER_H

#include "mbed.h"

namespace modules {

class Bumper {

private:
    InterruptIn _bumper;
    Timeout     _bumperDownTimeout;
    Timeout		_bumperUpTimeout;
    
    volatile bool _bumperCanPress;
    volatile bool _bumperCanUnpress;
    void (*_callbackDown)(void);
    void (*_callbackUp)(void);

    void click();
    void unclick();
    void callDown();
    void callUp();
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
    void attachUp(void (*method)(void) = 0);

    // ********************************************************************************
    // * Attaches the method to be called when the bumper is unpressed.
    // *
    // * @param method      A reference to the method to be called.
    // ********************************************************************************
    void attachDown(void (*method)(void) = 0);

    // ********************************************************************************
    // * The time in milliseconds in which the bumper can not be pressed again.
    // *
    // * @default          100ms
    // ********************************************************************************
    int timeout;
};

}
#endif
