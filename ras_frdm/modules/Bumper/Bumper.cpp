#include "Bumper.h"

using namespace modules;

Bumper::Bumper(PinName bumperPin) :
    _bumper(bumperPin) {
        _bumperCanPress = true;
        _callback = NULL;
//        _callbackInstance = NULL;
//        _callbackMethod = NULL;
        timeout = 200;
        _bumper.rise(this, &Bumper::click);
}

void Bumper::attach(void (*method)(void)) {
    _callback = method;
}

/*
template<class T>
void attach(T* instance, void (T::*method)(void)) {
	_callbackInstance = (BumperCallbackInstance *)instance;
    _callbackMethod = (void (BumperCallbackInstance::*)(void))method;
}
*/

void Bumper::click() {
    if (_bumperCanPress) {
        _bumperCanPress = false;
        _bumperDownTimeout.attach_us(this, &Bumper::reset, timeout * 1000);
        call();
    }
}

void Bumper::call() {
//	if (_callback != NULL)
		(*_callback)();
//    else
//        (_callbackInstance->*_callbackMethod)();
}

void Bumper::reset() {
    _bumperCanPress = true;
}
