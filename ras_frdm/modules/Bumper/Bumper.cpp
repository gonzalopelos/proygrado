#include "Bumper.h"

using namespace modules;

Bumper::Bumper(PinName bumperPin) :
    _bumper(bumperPin) {
        _bumperCanPress = true;
        _callback = NULL;
        timeout = 200;
        _bumper.fall(this, &Bumper::click);
}

void Bumper::attach(void (*method)(void)) {
    _callback = method;
}

void Bumper::click() {
    if (_bumperCanPress) {
        _bumperCanPress = false;
        _bumperDownTimeout.attach_us(this, &Bumper::reset, timeout * 1000);
        call();
    }
}

void Bumper::call() {
	(*_callback)();
}

void Bumper::reset() {
    _bumperCanPress = true;
}
