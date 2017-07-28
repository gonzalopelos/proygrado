#include "Bumper.h"

using namespace modules;

Bumper::Bumper(PinName bumperPin) :
    _bumper(bumperPin) {
        _bumperCanPress = true;
        _bumperCanUnpress = true;
        _callbackDown = NULL;
        _callbackUp = NULL;
        timeout = 100;
        _bumper.fall(this, &Bumper::click);
        _bumper.rise(this, &Bumper::unclick);
}

void Bumper::attachDown(void (*method)(void)) {
    _callbackDown = method;
}

void Bumper::attachUp(void (*method)(void)) {
    _callbackUp = method;
}

void Bumper::click() {
    if (_bumperCanPress) {
        _bumperCanPress = false;
        _bumperDownTimeout.attach_us(this, &Bumper::reset, timeout * 1000);
        callDown();
    }
}

void Bumper::unclick() {
    if (_bumperCanUnpress) {
        _bumperCanUnpress = false;
        _bumperUpTimeout.attach_us(this, &Bumper::reset, timeout * 1000);
        callUp();
    }
}

void Bumper::callDown() {
	(*_callbackDown)();
}

void Bumper::callUp() {
	(*_callbackUp)();
}

void Bumper::reset() {
    _bumperCanPress 	= true;
    _bumperCanUnpress 	= true;
}
