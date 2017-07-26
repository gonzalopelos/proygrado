#!/usr/bin/env python
import time
import roslib
import rospy
import serial
from bencode import bencode, bdecode, BTFailure
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
DIAGNOSTIC_ADMIN_MODULE_NAME = 'Admin Module'

diagMsg = DiagnosticArray() # diagnostic message to be published
diag_pub = None # diagnostic topic

def get_status_local():
    mcc = MCC(MCC.ADMIN_PID, MCC.GET_STATUS, '')
    mcc_pub.publish(mcc)

def get_status(empty):
    get_status_local()
    return EmptyResponse()

def ping(req):
    mcc = MCC(MCC.ADMIN_PID, MCC.PING, req.str)
    mcc_pub.publish(mcc)
    return StringResponse()

def reset(empty):
    mcc = MCC(MCC.ADMIN_PID, MCC.RESET, '')
    mcc_pub.publish(mcc)
    return EmptyResponse()

def newMsg(mcc):
    try:
        l_params = bdecode(mcc.params)

        if (mcc.pid == MCC.ADMIN_PID):
            if (mcc.opcode == MCC.GET_STATUS):

                if (l_params[0] == 'STARTED'):
                    stat = DiagnosticStatus(name = DIAGNOSTIC_ADMIN_MODULE_NAME, level = WARN_LEVEL, 
                                            message = 'io board started', hardware_id = DIAGNOSTIC_IOBOARD_ID)
                    stat.values = []
                    diagMsg.status = [stat]
                    diag_pub.publish(diagMsg)
                elif (l_params[0] == 'OK'):
                    stat = DiagnosticStatus(name = DIAGNOSTIC_ADMIN_MODULE_NAME, level = OK_LEVEL, 
                                            message = 'module status ... OK', hardware_id = DIAGNOSTIC_IOBOARD_ID)
                    stat.values = []
                    diagMsg.status = [stat]
                    diag_pub.publish(diagMsg)
                elif (l_params[0] == 'PARSEERROR'):
                    stat = DiagnosticStatus(name = DIAGNOSTIC_ADMIN_MODULE_NAME, level = ERROR_LEVEL, 
                                            message = 'message parse error', hardware_id = DIAGNOSTIC_IOBOARD_ID)
                    if (len(l_params) == 6):
                        _others = l_params[5]
                    else:
                        _others = ""

                    if (len(l_params) >= 5):
                        io_board_stat.values = [KeyValue(key = 'parser_state', value = l_params[1]), KeyValue(key = 'pid', value = l_params[2]), KeyValue(key = 'opcode', value = l_params[4]), KeyValue(key = 'parsed parameters', value = _others)]
                        diagMsg.status = [io_board_stat] #, eth_stat ]
                        diag_pub.publish(diagMsg)
                    else:
                        stat = DiagnosticStatus(name = DIAGNOSTIC_ADMIN_MODULE_NAME, level = ERROR_LEVEL, 
                                                message = 'bad parameters number', hardware_id = DIAGNOSTIC_IOBOARD_ID)
                        stat.values = [KeyValue(key = 'method', value = 'newMsg'), KeyValue(key = 'opcode', value = MCC.GET_STATUS), KeyValue(key = 'status', value = 'PARSEERROR')]
                        diagMsg.status = [stat]
                        diag_pub.publish(diagMsg)
                elif (l_params[0] == 'EXEERROR'):
                    stat = DiagnosticStatus(name = DIAGNOSTIC_ADMIN_MODULE_NAME, level = ERROR_LEVEL, 
                                            message = 'execution error', hardware_id = DIAGNOSTIC_IOBOARD_ID)
                    if (len(l_params) == 6):
                        _others = l_params[5]
                    else:
                        _others = ""

                    if (len(l_params) >= 5):
                        io_board_stat.values = [KeyValue(key = 'error_code', value = l_params[1]), KeyValue(key = 'pid', value = l_params[2]), KeyValue(key = 'opcode', value = l_params[4]), KeyValue(key = 'others parameters', value = _others)]
                        diagMsg.status = [io_board_stat] #, eth_stat ]
                        diag_pub.publish(diagMsg)
                    else:
                        stat = DiagnosticStatus(name = DIAGNOSTIC_ADMIN_MODULE_NAME, level = ERROR_LEVEL, 
                                                message = 'bad parameters number', hardware_id = DIAGNOSTIC_IOBOARD_ID)
                        stat.values = [KeyValue(key = 'method', value = 'newMsg'), KeyValue(key = 'opcode', value = MCC.GET_STATUS), KeyValue(key = 'status', value = 'PARSEERROR')]
                        diagMsg.status = [stat]
                        diag_pub.publish(diagMsg)

                else:
                    stat = DiagnosticStatus(name = DIAGNOSTIC_ADMIN_MODULE_NAME, level = ERROR_LEVEL, 
                                            message = 'unknown status parameters', hardware_id = DIAGNOSTIC_IOBOARD_ID)
                    stat.values = []
                    diagMsg.status = [stat]
                    diag_pub.publish(diagMsg)
    except Exception, e:
        # Error when debencoding
        io_board_stat = DiagnosticStatus(name = DIAGNOSTIC_ADMIN_MODULE_NAME, level = ERROR_LEVEL, 
                                         message = 'binary decode exception', hardware_id = DIAGNOSTIC_IOBOARD_ID)
        io_board_stat.values = [KeyValue(key = 'method', value = 'newMsg'), KeyValue(key = 'literal', value = mcc.params)]
        diagMsg.status = [io_board_stat] #, eth_stat ]
        diag_pub.publish(diagMsg)

if __name__ == '__main__':
    node_name = 'io_admin_module'
    service_prefix = 'dm3/' + node_name + '/'

    rospy.init_node(node_name)
    diag_pub = rospy.Publisher('/diagnostics', DiagnosticArray, queue_size=10)
    mcc_pub = rospy.Publisher('/dm3/mcc2ioboard', MCC, queue_size=10)
    rospy.Subscriber("/dm3/mcc2pc", MCC, newMsg)

    rospy.Service(service_prefix + 'get_status', Empty, get_status)
    rospy.Service(service_prefix + 'ping', String, ping)
    rospy.Service(service_prefix + 'reset', Empty, reset)

    get_status_local() #obtiene el estado de la placa para eliminar inconsistencias entre el estado de la placa y el del modulo

    rospy.spin()
