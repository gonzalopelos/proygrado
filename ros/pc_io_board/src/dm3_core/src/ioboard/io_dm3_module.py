#!/usr/bin/env python
import time
import roslib
import rospy
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
DIAGNOSTIC_DM3_MODULE_NAME = 'DM3 Module'

diagMsg = DiagnosticArray() # diagnostic message to be published
diag_pub = None # diagnostic topic

# DM3 module local state
_bat_level = DM3ModuleState.INI_BATTERY_LEVEL
_siren_remaining = DM3ModuleState.INI_SIREN_REMAINIG

def get_status_local():
    mcc = MCC(MCC.DM3_PID, MCC.GET_STATUS, '')
    mcc_pub.publish(mcc)

def get_status(empty):
    get_status_local()
    return EmptyResponse()

def siren(req):
    mcc = MCC(MCC.DM3_PID, MCC.SIREN, bencode([req.i]))
    mcc_pub.publish(mcc)

def get_battery_level(empty):
    mcc = MCC(MCC.DM3_PID, MCC.GET_BAT, bencode([]))
    mcc_pub.publish(mcc)
    return EmptyResponse()

def newMsg(mcc):
    global _bat_level, _siren_remaining

    l_params = bdecode(mcc.params)

    if (mcc.pid == MCC.DM3_PID):
        if (mcc.opcode == MCC.GET_STATUS):
            # rostopic pub /dm3/mcc2pc dm3_core/MCC '{pid: 2, opcode: 0, params: "l2:OKe"}' -1 -v
            if (l_params[0] == 'OK'):
                stat = DiagnosticStatus(name = DIAGNOSTIC_DM3_MODULE_NAME, level = OK_LEVEL, 
                                        message = 'module status ... OK', hardware_id = DIAGNOSTIC_IOBOARD_ID)
                stat.values = []
                diagMsg.status = [stat]
                diag_pub.publish(diagMsg)
            # rostopic pub /dm3/mcc2pc dm3_core/MCC '{pid: 2, opcode: 0, params: "l12:ADCREADERRORe"}' -1 -v
            elif (l_params[0] == 'ADCREADERROR'):
                stat = DiagnosticStatus(name = DIAGNOSTIC_DM3_MODULE_NAME, level = ERROR_LEVEL, 
                                        message = 'ADC read error', hardware_id = DIAGNOSTIC_IOBOARD_ID)
                stat.values = []
                diagMsg.status = [stat]
                diag_pub.publish(diagMsg)
            # rostopic pub /dm3/mcc2pc dm3_core/MCC '{pid: 2, opcode: 0, params: "l4:NONEe"}' -1 -v
            else:
                stat = DiagnosticStatus(name = DIAGNOSTIC_DM3_MODULE_NAME, level = ERROR_LEVEL, 
                                        message = 'unknown status parameters', hardware_id = DIAGNOSTIC_IOBOARD_ID)
                stat.values = []
                diagMsg.status = [stat]
                diag_pub.publish(diagMsg)
        # rostopic pub /dm3/mcc2pc dm3_core/MCC '{pid: 2, opcode: 1, params: "li8ee"}' -1 -v
        elif (mcc.opcode == MCC.SIREN):
            _siren_remaining = l_params[0]
        # rostopic pub /dm3/mcc2pc dm3_core/MCC '{pid: 2, opcode: 2, params: "l4:21.3e"}' -1 -v
        elif (mcc.opcode == MCC.GET_BAT):
            _bat_level = float(l_params[0])
        else:
            # rostopic pub /dm3/mcc2pc dm3_core/MCC '{pid: 2, opcode: 100, params: "l4:NONEe"}' -1 -v
            stat = DiagnosticStatus(name = DIAGNOSTIC_DM3_MODULE_NAME, level = ERROR_LEVEL, 
                                    message = 'unknown operation code', hardware_id = DIAGNOSTIC_IOBOARD_ID)
            stat.values = []
            diagMsg.status = [stat]
            diag_pub.publish(diagMsg)

if __name__ == '__main__':
    node_name = 'io_dm3_module'
    service_prefix = 'dm3/' + node_name + '/'

    rospy.init_node(node_name)
    diag_pub = rospy.Publisher('/diagnostics', DiagnosticArray, queue_size=10)
    mcc_pub = rospy.Publisher('/dm3/mcc2ioboard', MCC, queue_size=10)

    # service register
    rospy.Service(service_prefix + 'get_status', Empty, get_status)
    rospy.Service(service_prefix + 'siren', Int, siren)
    rospy.Service(service_prefix + 'get_battery_level', Empty, get_battery_level)

    rospy.Subscriber("/dm3/mcc2pc", MCC, newMsg)

    state_pub = rospy.Publisher('/dm3/dm3_module_state', DM3ModuleState, queue_size=10)
    report_status_rate = rospy.Rate(1)

    dm3_state = DM3ModuleState()

    get_status_local() #obtiene el estado de la placa para eliminar inconsistencias entre el estado de la placa y el del modulo

    while not rospy.is_shutdown():
        dm3_state.battery_level = _bat_level
        dm3_state.siren_remaining = _siren_remaining

        state_pub.publish(dm3_state)
        report_status_rate.sleep()
