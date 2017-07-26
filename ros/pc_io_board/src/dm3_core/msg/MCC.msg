# MCC message format

# PIDs
uint8 ADMIN_PID = 0
uint8 DM3MOTORS_PID = 1
uint8 DM3_PID = 2

# OP_CODEs
uint8 GET_STATUS = 0
uint8 PING = 1
uint8 PONG = 1
uint8 RESET = 2 
uint8 ENABLE = 1 
uint8 SET_TARGET_VEL = 2
uint8 BREAK = 3
uint8 REVERSE = 4
uint8 TEST = 5
uint8 DRIVE_MODE = 6
uint8 SET_POWER_MOTOR = 7
uint8 SET_CONTROLLER = 8
uint8 SET_PID_PARAMETERS = 9
uint8 GET_VEL_CHANGE = 10
uint8 GET_POT_CHANGE = 11

uint8 SIREN = 1
uint8 GET_BAT = 2

uint8 pid
uint8 opcode
string params # parameter list binary coded (bencode)
