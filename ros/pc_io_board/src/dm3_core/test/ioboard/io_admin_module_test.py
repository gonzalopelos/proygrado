#!/usr/bin/env python

import sys
import rospy

from dm3.srv import *
from dm3.msg import MCC
from std_srvs.srv import Empty
from bencode import bencode, bdecode, BTFailure

i=0

service_prefix = 'dm3/io_admin_module/'

def get_status_local():
    name_service = service_prefix + 'get_status'

    rospy.wait_for_service(name_service)
    try:
        get_status_remote = rospy.ServiceProxy(name_service, Empty)
    	get_status_remote()
    except rospy.ServiceException, e:
        print "Service call failed: %s"%e

def ping_local():
    global i

    name_service = service_prefix + 'ping'

    rospy.wait_for_service(name_service)
    try:
        ping_remote = rospy.ServiceProxy(name_service, String)
    	ping_remote(bencode(["hola",  str(i)]))
    except rospy.ServiceException, e:
        print "Service call failed: %s"%e
    i = i + 1

def reset_local():
    name_service = service_prefix + 'reset'

    rospy.wait_for_service(name_service)
    try:
        reset_remote = rospy.ServiceProxy(name_service, Empty)
    	reset_remote()
    except rospy.ServiceException, e:
        print "Service call failed: %s"%e

if __name__ == "__main__":
	frec = 40 # hasta 40 parece roportar la MBED
	rospy.init_node('io_admin_module_test', anonymous=True)
	rate = rospy.Rate(frec) # 10hz
	print "publish mcc message @ " + str (frec) + "Hz"
	while not rospy.is_shutdown():	
		#get_status_local()
		#ping_local()
		reset_local()
		rate.sleep()
