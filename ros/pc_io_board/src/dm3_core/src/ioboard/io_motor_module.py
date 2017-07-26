#!/usr/bin/env python
import time
import roslib
import rospy
import serial
from bencode import bencode, bdecode, BTFailure
from geometry_msgs.msg import Twist
from std_srvs.srv import *
from diagnostic_msgs.msg import DiagnosticArray, DiagnosticStatus, KeyValue
from dm3_core.srv import *
from dm3_core.msg import *

# Possible levels of operations to DiagnosticStatus
OK_LEVEL = 0
WARN_LEVEL = 1
ERROR_LEVEL = 2
STALE_LEVEL = 3

DIAGNOSTIC_IOBOARD_ID = 'IO Board'
DIAGNOSTIC_MOTOR_MODULE_NAME = 'Motor Module'

diagMsg = DiagnosticArray() # diagnostic message to be published
diag_pub = None # diagnostic topic

_enabled = False
_braked = False
_reversed = False
_drive_mode = 0
_controller_type = 0
_power = [[0, 0],[0, 0]]# arryay of [l_power, r_power] for each chasis
_current_vel = [[0, 0],[0, 0]]
_target_vel = [[0, 0],[0, 0]]# arryay of [l_target_vel, r_target_vel] for each chasis
_angle = [0,0]
_raw   = [0,0]
_pidk  = [0, 0, 0] 

def get_status_local():
    mcc = MCC(MCC.DM3MOTORS_PID, MCC.GET_STATUS, '')
    mcc_pub.publish(mcc)

def get_status(empty):
    get_status_local()
    return EmptyResponse()

def enable(req):
    mcc = MCC(MCC.DM3MOTORS_PID, MCC.ENABLE, bencode([1 if req.b else 0]))
    mcc_pub.publish(mcc)
    return BoolResponse()

def breakAll(req):
    mcc = MCC(MCC.DM3MOTORS_PID, MCC.BREAK, bencode([1 if req.b else 0]))
    mcc_pub.publish(mcc)
    return BoolResponse()

def reverse(req):
    mcc = MCC(MCC.DM3MOTORS_PID, MCC.REVERSE, bencode([1 if req.b else 0]))
    mcc_pub.publish(mcc)
    return BoolResponse()

def test(empty):
    mcc = MCC(MCC.DM3MOTORS_PID, MCC.TEST, bencode([]))
    mcc_pub.publish(mcc)
    return EmptyResponse()

def set_drive_mode(req):
    mcc = MCC(MCC.DM3MOTORS_PID, MCC.DRIVE_MODE, bencode([req.i]))
    mcc_pub.publish(mcc)
    return IntResponse()

def set_controller(req):
    mcc = MCC(MCC.DM3MOTORS_PID, MCC.SET_CONTROLLER, bencode([req.i]))
    mcc_pub.publish(mcc)
    return IntResponse()

def set_pid_parameters(req):
    mcc = MCC(MCC.DM3MOTORS_PID, MCC.SET_PID_PARAMETERS, bencode([str(req.kp), str(req.ki), str(req.kd)]))
    mcc_pub.publish(mcc)
    return PIDkResponse()

def setVel(twist):
    mcc = MCC(MCC.DM3MOTORS_PID, MCC.SET_TARGET_VEL, bencode([str(twist.linear.x), str(twist.angular.z)]))
    mcc_pub.publish(mcc)

def setPower(motor_power):
    mcc = MCC(MCC.DM3MOTORS_PID, MCC.SET_POWER_MOTOR, bencode([motor_power.chasis_id, motor_power.motor_id, str(motor_power.power)]))
    mcc_pub.publish(mcc)

def sendState():
    _motors_state = MotorsState()
    _chasis_state = ChasisState()

    _motors_state.header.stamp = rospy.Time.now()
    _motors_state.enabled = _enabled
    _motors_state.braked = _braked
    _motors_state.reversed = _reversed
    _motors_state.drive_mode = _drive_mode
    _motors_state.controller_type = _controller_type
    _motors_state.wheel_power_1 = _power[0]
    _motors_state.wheel_power_2 = _power[1]
    _motors_state.wheel_target_speed_1 = _target_vel[0]
    _motors_state.wheel_target_speed_2 = _target_vel[1]
    _motors_state.wheel_speed_1 = _current_vel[0]
    _motors_state.wheel_speed_2 = _current_vel[1]
    _chasis_state.header.stamp = rospy.Time.now()   
    _chasis_state.angle   = _angle
    _chasis_state.raw     = _raw
    _motors_state.pidk    = _pidk 

    motor_state_pub.publish(_motors_state)
    chasis_state_pub.publish(_chasis_state)

def newMsg(mcc):
    global  _enabled, _braked, _reversed, _angle, _raw, _drive_mode, _controller_type, _pidk
    _change_state = False

    try:
        l_params = bdecode(mcc.params)
    except Exception, e:
        # Error when debencoding
        io_board_stat = DiagnosticStatus(name = DIAGNOSTIC_MOTOR_MODULE_NAME, level = ERROR_LEVEL, 
                                         message = 'binary decode exception', hardware_id = DIAGNOSTIC_IOBOARD_ID)
        io_board_stat.values = [KeyValue(key = 'method', value = 'newMsg'), KeyValue(key = 'literal', value = mcc.params)]
        diagMsg.status = [io_board_stat] #, eth_stat ]
        diag_pub.publish(diagMsg)
    else:
        try:

            if (mcc.pid == MCC.DM3MOTORS_PID):
                if (mcc.opcode == MCC.GET_STATUS):
                    # rostopic pub /dm3/mcc2pc dm3_core/MCC '{pid: 1, opcode: 0, params: "l2:OK7:ENABLED6:BRAKED10:REVERSEDONe"}' -1 -v
                    if (l_params[0] == 'OK'):
                        if (len(l_params) == 4):
                            _enabled = l_params[1]=='ENABLED'; _braked = l_params[2]=='BRAKED'; _reversed = l_params[3]=='REVERSEDON'
                            stat = DiagnosticStatus(name = DIAGNOSTIC_MOTOR_MODULE_NAME, level = OK_LEVEL, 
                                                    message = 'module status ... OK', hardware_id = DIAGNOSTIC_IOBOARD_ID)
                            stat.values = [KeyValue(key = 'enable', value = l_params[1]), KeyValue(key = 'brake', value = l_params[2]), KeyValue(key = 'reverse', value = l_params[3])]
                            diagMsg.status = [stat]
                            diag_pub.publish(diagMsg)
                            _change_state = True
                        else:
                            stat = DiagnosticStatus(name = DIAGNOSTIC_MOTOR_MODULE_NAME, level = ERROR_LEVEL, 
                                                    message = 'bad parameters number', hardware_id = DIAGNOSTIC_IOBOARD_ID)
                            stat.values = [KeyValue(key = 'method', value = 'newMsg'), KeyValue(key = 'opcode', value = MCC.GET_STATUS), KeyValue(key = 'status', value = 'OK')]
                            diagMsg.status = [stat]
                            diag_pub.publish(diagMsg)

                    # rostopic pub /dm3/mcc2pc dm3_core/MCC '{pid: 1, opcode: 0, params: "l15:I2CWRITEFAILUREe"}' -1 -v
                    elif (l_params[0] == 'I2CWRITEFAILURE'):
                        stat = DiagnosticStatus(name = DIAGNOSTIC_MOTOR_MODULE_NAME, level = ERROR_LEVEL, 
                                                message = 'I2C write failure', hardware_id = DIAGNOSTIC_IOBOARD_ID)
                        stat.values = []
                        diagMsg.status = [stat]
                        diag_pub.publish(diagMsg)
                    # rostopic pub /dm3/mcc2pc dm3_core/MCC '{pid: 1, opcode: 0, params: "l4:NONEe"}' -1 -v
                    else:
                        stat = DiagnosticStatus(name = DIAGNOSTIC_MOTOR_MODULE_NAME, level = ERROR_LEVEL, 
                                                message = 'unknown status parameters', hardware_id = DIAGNOSTIC_IOBOARD_ID)
                        stat.values = []
                        diagMsg.status = [stat]
                        diag_pub.publish(diagMsg)
                # rostopic pub /dm3/mcc2pc dm3_core/MCC '{pid: 1, opcode: 1, params: "li1ee"}' -1 -v
                elif (mcc.opcode == MCC.ENABLE):
                    _enabled = l_params[0] == 1
                    _change_state = True
                # rostopic pub /dm3/mcc2pc dm3_core/MCC '{pid: 1, opcode: 3, params: "li1ee"}' -1 -v
                elif (mcc.opcode == MCC.BREAK):
                    _braked = l_params[0] == 1
                    _change_state = True
                # rostopic pub /dm3/mcc2pc dm3_core/MCC '{pid: 1, opcode: 4, params: "li1ee"}' -1 -v
                elif (mcc.opcode == MCC.REVERSE):
                    _reversed = l_params[0] == 1
                    _change_state = True
                # rostopic pub /dm3/mcc2pc dm3_core/MCC '{pid: 1, opcode: 6, params: "li1ee"}' -1 -v
                elif (mcc.opcode == MCC.DRIVE_MODE):
                    _drive_mode = int(l_params[0])
                    _change_state = True
                elif (mcc.opcode == MCC.SET_CONTROLLER):
                    _controller_type = int(l_params[0])
                    _change_state = True
                # rostopic pub /dm3/mcc2pc dm3_core/MCC '{pid: 1, opcode: 2, params: "li0e2:102:204:-1.24:0.12e"}' -1 -v
                elif (mcc.opcode == MCC.SET_POWER_MOTOR):
                    if (len(l_params) == 3):
                        chasis = int(l_params[0])
                        _power[chasis-1] = [float(l_params[1]), float(l_params[2])]
                        _change_state = True
                    # rostopic pub /dm3/mcc2pc dm3_core/MCC '{pid: 1, opcode: 2, params: "li0e2:102:20e"}' -1 -v
                    else:
                        stat = DiagnosticStatus(name = DIAGNOSTIC_MOTOR_MODULE_NAME, level = ERROR_LEVEL, 
                                                message = 'bad parameters number', hardware_id = DIAGNOSTIC_IOBOARD_ID)
                        stat.values = [KeyValue(key = 'operation', value = 'MCC.SET_POWER_MOTOR'),
                                       KeyValue(key = 'params_number', value = str(len(l_params))),
                                       KeyValue(key = 'literal', value = mcc.params)]
                        diagMsg.status = [stat]
                        diag_pub.publish(diagMsg)
                elif (mcc.opcode == MCC.SET_TARGET_VEL):
                    if (len(l_params) == 3):
                        chasis = int(l_params[0])
                        _target_vel[chasis-1] = [float(l_params[1]), float(l_params[2])]
                        _change_state = True
                    # rostopic pub /dm3/mcc2pc dm3_core/MCC '{pid: 1, opcode: 2, params: "li0e2:102:20e"}' -1 -v
                    else:
                        stat = DiagnosticStatus(name = DIAGNOSTIC_MOTOR_MODULE_NAME, level = ERROR_LEVEL, 
                                                message = 'bad parameters number', hardware_id = DIAGNOSTIC_IOBOARD_ID)
                        stat.values = [KeyValue(key = 'operation', value = 'MCC.SET_TARGET_VEL'),
                                       KeyValue(key = 'params_number', value = str(len(l_params))),
                                       KeyValue(key = 'literal', value = mcc.params)]
                        diagMsg.status = [stat]
                        diag_pub.publish(diagMsg)
                elif (mcc.opcode == MCC.GET_VEL_CHANGE):
                    if (len(l_params) == 3):
                        chasis = int(l_params[0])
                        _current_vel[chasis-1] = [float(l_params[1]), float(l_params[2])]
                        _change_state = True
                    # rostopic pub /dm3/mcc2pc dm3_core/MCC '{pid: 1, opcode: 2, params: "li0e2:102:20e"}' -1 -v
                    else:
                        stat = DiagnosticStatus(name = DIAGNOSTIC_MOTOR_MODULE_NAME, level = ERROR_LEVEL, 
                                                message = 'bad parameters number', hardware_id = DIAGNOSTIC_IOBOARD_ID)
                        stat.values = [KeyValue(key = 'operation', value = 'MCC.GET_VEL_CHANGE'),KeyValue(key = 'params_number', value = str(len(l_params)))]
                        diagMsg.status = [stat]
                        diag_pub.publish(diagMsg)
                elif (mcc.opcode == MCC.GET_POT_CHANGE):
                    if (len(l_params) == 3):
                        chasis = int(l_params[0])
                        _angle[chasis-1] = float(l_params[1])
                        _raw[chasis-1] = float(l_params[2])
                        _change_state = True
                    # rostopic pub /dm3/mcc2pc dm3_core/MCC '{pid: 1, opcode: 2, params: "li0e2:102:20e"}' -1 -v
                    else:
                        stat = DiagnosticStatus(name = DIAGNOSTIC_MOTOR_MODULE_NAME, level = ERROR_LEVEL, 
                                                message = 'bad parameters number', hardware_id = DIAGNOSTIC_IOBOARD_ID)
                        stat.values = [KeyValue(key = 'operation', value = 'MCC.GET_POT_CHANGE'),KeyValue(key = 'params_number', value = str(len(l_params)))]
                        diagMsg.status = [stat]
                        diag_pub.publish(diagMsg)
                elif (mcc.opcode == MCC.SET_PID_PARAMETERS):
                    if (len(l_params) == 3):
                        _pidk[0] = float(l_params[0])
                        _pidk[1] = float(l_params[1])
                        _pidk[2] = float(l_params[2])
                        _change_state = True
                    else:
                        stat = DiagnosticStatus(name = DIAGNOSTIC_MOTOR_MODULE_NAME, level = ERROR_LEVEL, 
                                                message = 'bad parameters number', hardware_id = DIAGNOSTIC_IOBOARD_ID)
                        stat.values = [KeyValue(key = 'operation', value = 'MCC.SET_PID_PARAMETERS'),KeyValue(key = 'params_number', value = str(len(l_params)))]
                        diagMsg.status = [stat]
                        diag_pub.publish(diagMsg)                      


                else:
                    # rostopic pub /dm3/mcc2pc dm3_core/MCC '{pid: 1, opcode: 100, params: "l4:NONEe"}' -1 -v
                    stat = DiagnosticStatus(name = DIAGNOSTIC_MOTOR_MODULE_NAME, level = ERROR_LEVEL, 
                                            message = 'unknown operation code', hardware_id = DIAGNOSTIC_IOBOARD_ID)
                    stat.values = []
                    diagMsg.status = [stat]
                    diag_pub.publish(diagMsg)
                if (not rospy.get_param("/io_motor_module/rated_state") and _change_state):
                    sendState()

        except Exception, e:
            stat = DiagnosticStatus(name = DIAGNOSTIC_MOTOR_MODULE_NAME, level = ERROR_LEVEL, 
                                    message = 'bad convertion to number', hardware_id = DIAGNOSTIC_IOBOARD_ID)
            stat.values = [KeyValue(key = 'operation', value = str(mcc.opcode)),
                           KeyValue(key = 'params', value = mcc.params)]
            diagMsg.status = [stat]
            diag_pub.publish(diagMsg)



if __name__ == '__main__':
    node_name = 'io_motor_module'
    service_prefix = 'dm3/' + node_name + '/'

    rospy.init_node(node_name)
    diag_pub = rospy.Publisher('/diagnostics', DiagnosticArray, queue_size=10)
    mcc_pub = rospy.Publisher('/dm3/mcc2ioboard', MCC, queue_size=10)

    # service register
    rospy.Service(service_prefix + 'get_status', Empty, get_status)
    rospy.Service(service_prefix + 'enable', Bool, enable)
    rospy.Service(service_prefix + 'break', Bool, breakAll)
    rospy.Service(service_prefix + 'reverse', Bool, reverse)
    rospy.Service(service_prefix + 'test', Empty, test) 
    rospy.Service(service_prefix + 'set_drive_mode', Int, set_drive_mode)
    rospy.Service(service_prefix + 'set_controller', Int, set_controller)
    rospy.Service(service_prefix + 'set_pid_parameters', PIDk, set_pid_parameters)

    rospy.Subscriber("/dm3/cmd_vel_io_board", Twist, setVel)
    rospy.Subscriber("/dm3/cmd_power", MotorPower, setPower)
    rospy.Subscriber("/dm3/mcc2pc", MCC, newMsg)

    motor_state_pub  = rospy.Publisher('/dm3/motors_state', MotorsState, queue_size=10)
    chasis_state_pub = rospy.Publisher('/dm3/chasis_state', ChasisState, queue_size=10)

    get_status_local() #obtiene el estado de la placa para eliminar inconsistencias entre el estado de la placa y el del modulo
    if (rospy.get_param("/io_motor_module/rated_state")):
        report_status_rate = rospy.Rate(1)
        while not rospy.is_shutdown():
            sendState()
            report_status_rate.sleep()
    else:
        rospy.spin()
