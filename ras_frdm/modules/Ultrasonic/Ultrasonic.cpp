#include "Ultrasonic.h"

using namespace modules;

    Ultrasonic::Ultrasonic(PinName trigPin, PinName echoPin, float updateSpeed, float timeout):_trig(trigPin), _echo(echoPin)
    {
        _updateSpeed = updateSpeed;
        _timeout = timeout;
    }

    Ultrasonic::Ultrasonic(PinName trigPin, PinName echoPin, float updateSpeed, float timeout, void onUpdate(int))
    :_trig(trigPin), _echo(echoPin)
    {
        _onUpdateMethod=onUpdate;
        _updateSpeed = updateSpeed;
        _timeout = timeout;
        _t.start ();
    }
    void Ultrasonic::_startT()
    {
        if(_t.read()>600)
        {
            _t.reset ();
        }
        start = _t.read_us ();
    }

    void Ultrasonic::_updateDist()
    {
        end = _t.read_us ();
        done = 1;
        _distance = (end - start)/6;
        _tout.detach();
        _tout.attach(callback(this,&Ultrasonic::_startTrig), _updateSpeed);
    }
    void Ultrasonic::_startTrig(void)
    {
            _tout.detach();
            _trig=1;
            wait_us(10);
            done = 0;
            _echo.rise(callback(this,&Ultrasonic::_startT));
            _echo.fall(callback(this,&Ultrasonic::_updateDist));
            _echo.enable_irq ();
            _tout.attach(callback(this,&Ultrasonic::_startTrig),_timeout);
            _trig=0;
    }

    int Ultrasonic::getCurrentDistance(void)
    {
        return _distance;
    }
    void Ultrasonic::pauseUpdates(void)
    {
        _tout.detach();
        _echo.rise(NULL);
        _echo.fall(NULL);
    }
    void Ultrasonic::startUpdates(void)
    {
    	printf("start::startUpdates\n");
        _startTrig();
        printf("end::startUpdates\n");
    }
    void Ultrasonic::attachOnUpdate(void method(int))
    {
        _onUpdateMethod = method;
    }
    void Ultrasonic::changeUpdateSpeed(float updateSpeed)
    {
        _updateSpeed = updateSpeed;
    }
    float Ultrasonic::getUpdateSpeed()
    {
        return _updateSpeed;
    }
    int Ultrasonic::isUpdated(void)
    {
        d=done;
        done = 0;
        return d;
    }
    void Ultrasonic::checkDistance(void)
    {
    	printf("checkDistance\n");
        if(isUpdated())
        {
        	printf("start::checkDistance isUpdated\n");
            (*_onUpdateMethod)(_distance);
            printf("end::checkDistance isUpdated\n");
        }
        else{
        	printf("checkDistance isUpdated return false\n");
        }
    }
