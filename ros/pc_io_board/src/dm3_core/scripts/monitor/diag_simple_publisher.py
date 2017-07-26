#!/usr/bin/env python
import time
import roslib
import rospy
from geometry_msgs.msg import Twist
from diagnostic_msgs.msg import DiagnosticArray, DiagnosticStatus, KeyValue
import serial
from bencode import bencode, bdecode, BTFailure
import std_srvs.srv
from dm3.srv import *

# Possible levels of operations to DiagnosticStatus
OK_LEVEL = 0
WARN_LEVEL = 1
ERROR_LEVEL = 2
STALE_LEVEL = 3

# ejemplo de mensaje https://github.com/yujinrobot/kobuki_core/blob/indigo/kobuki_driver/src/driver/event_manager.cpp
diagMsg = DiagnosticArray() # diagnostic message to be published

if __name__ == '__main__':
    rospy.init_node('diag_simple_publisher')
    pub = rospy.Publisher('/diagnostics', DiagnosticArray, queue_size=10)

    while not rospy.is_shutdown():
            io_board_stat = DiagnosticStatus(name = "/DM3/Motor State", level = OK_LEVEL, 
                                             message = 'read exception')
            io_board_stat.values = [KeyValue(key = 'port', value = SERIAL_PORT)]
            diagMsg.status = [io_board_stat] #, eth_stat ]
            pub.publish(diagMsg)
