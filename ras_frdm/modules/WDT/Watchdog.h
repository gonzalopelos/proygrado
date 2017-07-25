#ifndef WATCHDOG_K64F_H
#define WATCHDOG_K64F_H

#include "mbed.h"

namespace modules{

class Watchdog {

public:
	// Crear watchdog, WDTseconds = timeout en segundods, hasta 356 segundos.
    void kick(int WDTseconds);
    
    // Patear el perro.
    void kick();
    
private:
    // Desabilitar WDT
    void DisableWDOG();
    
    // Habilitar WDT
    void EnableWDOG();

};

}

#endif
