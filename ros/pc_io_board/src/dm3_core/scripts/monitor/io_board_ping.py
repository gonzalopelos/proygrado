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
DIAGNOSTIC_IOBOARD_NAME = 'Admin Module'

service_prefix = 'dm3/io_admin_module/'
diag_msg = DiagnosticArray() # diagnostic message to be published
ping_stat = DiagnosticStatus(name = DIAGNOSTIC_IOBOARD_NAME, level = OK_LEVEL, 
                             message = 'alive', hardware_id = DIAGNOSTIC_IOBOARD_ID)
i = 0

def ping_local():
    global i

    name_service = service_prefix + 'ping'

    rospy.wait_for_service(name_service)
    try:
        ping_remote = rospy.ServiceProxy(name_service, String)
    	ping_remote(bencode(["alive msg",  str(i)]))
    except rospy.ServiceException, e:
        print "Service call failed: %s"%e
    i = i + 1

def mmc_response(mcc):
    if (mcc.pid == MCC.ADMIN_PID and mcc.opcode == MCC.PING):
        ping_stat.values = [KeyValue(key = 'ping number', value = bdecode(mcc.params)[0])]
        diag_msg.status = [ping_stat] #, eth_stat ]
        diag_pub.publish(diag_msg)

if __name__ == "__main__":
    frec = 1
    rospy.init_node('io_board_ping', anonymous=True)
    diag_pub = rospy.Publisher('/diagnostics', DiagnosticArray, queue_size=10)
    rospy.Subscriber('/dm3/mcc2pc', MCC, mmc_response)
    rate = rospy.Rate(frec)

    while not rospy.is_shutdown():	
        ping_local()
        rate.sleep()
