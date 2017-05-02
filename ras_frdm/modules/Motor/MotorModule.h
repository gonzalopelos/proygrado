#ifndef __task_motor_h
#define __task_motor_h

#include "EmBencode.h"
#include "dm3.h"
#include "PIDModule.h"
#include "../Mcc/Mcc.h"
#include "rtos.h"

namespace modules {

#define POT_POLL 100
#define ENCODER_POLL 10

#define KILLING_TIME_MS 1000

//#define MODULE_VERSION 1

#define MOTOR_OPCODES 12 // Uno mas de los que vienen a continuacion porque incluye a OPCODE_REPORT es global

// codigos de operacion del modulo. deben estar consecutivos pues la implementacion usa un array para los manejadores
#define OPCODE_ENABLE 1
#define OPCODE_SET_TARGET_VEL 2
#define OPCODE_BRAKE  3
#define OPCODE_REVERSE  4
#define OPCODE_TEST  5
#define OPCODE_SET_DRIVE_MODE 6
#define OPCODE_SET_POWER_MOTOR 7 // set raw power operation code
#define OPCODE_SET_CONTROLLER 8
#define OPCODE_SET_PID_PARAMETERS 9
#define OPCODE_GET_VEL_CHANGE 10	// get current velocity using hall sensor
#define OPCODE_GET_POT_CHANGE 11 // report potencimeter operation code
#define OPCODE_DEBUG 12 // report debug string

class MotorModule
{
	int pid;
	OpcodeCallback opcode_callbacks[MOTOR_OPCODES];
	void hall_raised_n(int);

public:
	MotorModule ();
	//void potpoll();
	static void init();
	static void potpoll_task(void const *argument);
	static void compute_speed_task(void const *argument);
	static void controller_update_task(void const *argument);
	static void rated_report_vel_task(void const *argument);
	static void rated_report_pow_task(void const *argument);
	static void rated_report_pot_task(void const *argument);
	static void security_stop_task(void const *argument);
};

}

#endif
