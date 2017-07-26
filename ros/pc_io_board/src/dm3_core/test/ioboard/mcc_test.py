#!/usr/bin/env python

import sys
import rospy

from dm3.srv import *
from dm3.msg import MCC

from bencode import bencode, bdecode, BTFailure

i=0
pub = rospy.Publisher('/dm3/mcc2IOBoard', MCC, queue_size=10)


def sendMCC():
    global i, pub


    try:
        mcc = MCC(0,1,bencode(["hola" + str(i)]))
        pub.publish(mcc)
        i=i+1
        
    except rospy.ServiceException, e:
        print "Service call failed: %s"%e

if __name__ == "__main__":
	frec = 40 # hasta 40 parece roportar la MBED
	rospy.init_node('mcc_test', anonymous=True)
	rate = rospy.Rate(frec) # 10hz
	print "publish mcc message @ " + str (frec) + "Hz"
	while not rospy.is_shutdown():	
		sendMCC()
		rate.sleep()
