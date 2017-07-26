#!/usr/bin/env python

import rospy
import sensor_msgs.msg
from std_msgs.msg import String
from geometry_msgs.msg import Twist
from topic_tools.srv import MuxSelect
from dm3_core.msg import MCC
from dm3_core.srv import Int
from dm3_core.srv import Bool

import socket
import math

MAX_ANGLE = 1.0
QUEUE_SIZE = 10

# publish to cmd_vel
p = rospy.Publisher('/dm3/cmd_vel/backward_control', Twist, queue_size = QUEUE_SIZE)


def changeCmdVelMux(newTopic):
    try:
      rospy.wait_for_service('mux_not_backward/select', 5)
    except rospy.ROSException, e:
      rospy.loginfo(rospy.get_caller_id() + "Service call failed: %s", e)

    select_srv = rospy.ServiceProxy('mux_not_backward/select', MuxSelect)
    select_srv(newTopic)


def callback(twist):   
    if (twist.linear.x < 0):
      changeCmdVelMux('/dm3/cmd_vel/backward_control')
      twist.linear.x = 0.5;
      if(twist.angular.z < 0):
        twist.angular.z = MAX_ANGLE
      else:
        twist.angular.z = -MAX_ANGLE
      p.publish(twist)
    else:
      changeCmdVelMux('/dm3/cmd_vel')
    
      



def listener():
    # In ROS, nodes are uniquely named. If two nodes with the same
    # node are launched, the previous one is kicked off. The
    # anonymous=True flag means that rospy will choose a unique
    # name for our 'listener' node so that multiple listeners can
    # run simultaneously.
    rospy.init_node('backward_control', anonymous=True)

    # suscribe to scan topic
    rospy.Subscriber("/dm3/cmd_vel", Twist, callback)

    # spin() simply keeps python from exiting until this node is stopped
    rospy.spin()

if __name__ == '__main__':
    listener()
