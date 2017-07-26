#!/usr/bin/env python

import sys
import rospy
from diagnostic_msgs.msg import DiagnosticArray, DiagnosticStatus, KeyValue

from dm3_core.srv import *
from dm3_core.msg import MCC
from std_srvs.srv import Empty
from bencode import bencode, bdecode, BTFailure

# Possible levels of operations to DiagnosticStatus
OK_LEVEL = 0
WARN_LEVEL = 1
ERROR_LEVEL = 2
STALE_LEVEL = 3

DIAGNOSTIC_IOBOARD_ID = 'IO Board'
DIAGNOSTIC_ADMIN_MOD_NAME = 'Admin Module'
DIAGNOSTIC_DM3_MOD_NAME = 'DM3 Module'
DIAGNOSTIC_MOTOR_MOD_NAME = 'Motor Module'

diag_msg = DiagnosticArray() # diagnostic message to be published

def mmc_response(mcc):
    if (mcc.pid == MCC.ADMIN_PID):
        stat_msg = DiagnosticStatus(name = DIAGNOSTIC_ADMIN_MOD_NAME, level = OK_LEVEL, 
                                    message = 'new message', hardware_id = DIAGNOSTIC_IOBOARD_ID)
    elif (mcc.pid == MCC.DM3MOTORS_PID):
        stat_msg = DiagnosticStatus(name = DIAGNOSTIC_MOTOR_MOD_NAME, level = OK_LEVEL, 
                                    message = 'new message', hardware_id = DIAGNOSTIC_IOBOARD_ID)
    elif (mcc.pid == MCC.DM3_PID):
        stat_msg = DiagnosticStatus(name = DIAGNOSTIC_DM3_MOD_NAME, level = OK_LEVEL, 
                                    message = 'new message', hardware_id = DIAGNOSTIC_IOBOARD_ID)

    else:
        stat_msg = DiagnosticStatus(name = DIAGNOSTIC_DM3_MOD_NAME, level = ERROR_LEVEL, 
                                    message = 'invalid pid:' + str(mcc.pid), hardware_id = DIAGNOSTIC_IOBOARD_ID)


    stat_msg.values = [KeyValue(key = 'opcode', value = str(mcc.opcode)),
                        KeyValue(key = 'params', value = mcc.params)]
    diag_msg.status = [stat_msg]
    diag_pub.publish(diag_msg)

if __name__ == "__main__":
    rospy.init_node('io_board_modules_alive', anonymous=True)
    diag_pub = rospy.Publisher('/diagnostics', DiagnosticArray, queue_size=10)
    rospy.Subscriber('/dm3/mcc2pc', MCC, mmc_response)

    rospy.spin()
