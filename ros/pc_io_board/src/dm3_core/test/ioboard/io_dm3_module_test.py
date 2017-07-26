#!/usr/bin/env python

import sys
import rospy

from dm3.srv import *
from dm3.msg import MCC
from std_srvs.srv import Empty
from geometry_msgs.msg import Twist
from bencode import bencode, bdecode, BTFailure

b = True
i = 0

service_prefix = 'dm3/io_dm3_module/'

def get_status_local():
    name_service = service_prefix + 'get_status'

    rospy.wait_for_service(name_service)
    try:
        get_status_remote = rospy.ServiceProxy(name_service, Empty)
    	get_status_remote()
    except rospy.ServiceException, e:
        print "Service call failed: %s"%e

def siren_local():
    global i

    name_service = service_prefix + 'siren'

    rospy.wait_for_service(name_service)
    try:
        siren_remote = rospy.ServiceProxy(name_service, Int)
        siren_remote(i)
    except rospy.ServiceException, e:
        print "Service call failed: %s"%e

    i = i + 1

if __name__ == "__main__":
    frec = 40 # hasta 40 parece roportar la MBED
    rospy.init_node('io_dm3_module_test', anonymous=True)
    rate = rospy.Rate(frec) # 10hz
    print "publish mcc message @ " + str (frec) + "Hz"

    while not rospy.is_shutdown():	
        get_status_local()
        #siren_local()

        rate.sleep()
