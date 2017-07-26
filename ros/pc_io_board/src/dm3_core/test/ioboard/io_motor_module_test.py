#!/usr/bin/env python

import sys
import rospy

from dm3_core.srv import *
from dm3_core.msg import MCC
from std_srvs.srv import Empty
from geometry_msgs.msg import Twist
from bencode import bencode, bdecode, BTFailure

b = True
i = 0

service_prefix = 'dm3/io_motor_module/'

def get_status_local():
    global i

    name_service = service_prefix + 'get_status'

    rospy.wait_for_service(name_service)
    try:
        get_status_remote = rospy.ServiceProxy(name_service, Empty)
    	get_status_remote()
    except rospy.ServiceException, e:
        print "Service call failed: %s"%e

def enable_toggle_local():
    global b

    name_service = service_prefix + 'enable'

    rospy.wait_for_service(name_service)
    try:
        enable_remote = rospy.ServiceProxy(name_service, Bool)
    	enable_remote(b)
    except rospy.ServiceException, e:
        print "Service call failed: %s"%e
    b = not b

def enable_local():
    name_service = service_prefix + 'enable'

    rospy.wait_for_service(name_service)
    try:
        enable_remote = rospy.ServiceProxy(name_service, Bool)
    	enable_remote(b)
    except rospy.ServiceException, e:
        print "Service call failed: %s"%e

def break_local():
    global b

    name_service = service_prefix + 'break'
    rospy.wait_for_service(name_service)
    try:
        break_remote = rospy.ServiceProxy(name_service, Bool)
        break_remote(b)
    except rospy.ServiceException, e:
        print "Service call failed: %s"%e
    b = not b

def reverse_local():
    global b

    name_service = service_prefix + 'reverse'
    rospy.wait_for_service(name_service)
    try:
        reverse_remote = rospy.ServiceProxy(name_service, Bool)
    	reverse_remote(b)
    except rospy.ServiceException, e:
        print "Service call failed: %s"%e
    b = not b

def test_local():
    name_service = service_prefix + 'test'

    rospy.wait_for_service(name_service)
    try:
        test_remote = rospy.ServiceProxy(name_service, Empty)
        test_remote()
    except rospy.ServiceException, e:
        print "Service call failed: %s"%e

def set_vector_mode_local():
    global i

    name_service = service_prefix + 'set_vector_mode'
    rospy.wait_for_service(name_service)
    try:
        set_vector_mode_remote = rospy.ServiceProxy(name_service, Int)
        set_vector_mode_remote(i)
    except rospy.ServiceException, e:
        print "Service call failed: %s"%e
    i=i+1

def set_vel_local():
    global vel_pub, i

    twist = Twist()
    twist.linear.x = i
    twist.angular.z = -i
    vel_pub.publish(twist)

    i = i + 1

if __name__ == "__main__":
    frec = 1 # hasta 40 parece soportar la MBED
    rospy.init_node('io_motor_module_test', anonymous=True)
    rate = rospy.Rate(frec)
    print "publish mcc message @ " + str (frec) + "Hz"

    vel_pub = rospy.Publisher('/dm3/cmd_vel', Twist, queue_size=10)

    while not rospy.is_shutdown():	
        #get_status_local()
        #enable_toggle_local()
        #enable_local()
        #reverse_local()
        #break_local()
        #test_local()
        #set_vel_local()
        set_vector_mode_local()

        rate.sleep()