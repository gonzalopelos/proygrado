#!/usr/bin/env python

import rospy
import sensor_msgs.msg
from std_msgs.msg import String
from geometry_msgs.msg import Twist
import dynamic_reconfigure.client
import time

QUEUE_SIZE = 10

MIN_DIST_VALUE = 0.002 	# no usar range.range_min pues devuelve cero (m)
MAX_DIST_VALUE = 5	# distancia maxima del laser (m)

MIN_POWER_MOTORS = 20
MAX_POWER_MOTORS = 100

MIN_ANGLE = -1; MAX_ANGLE = -MIN_ANGLE # angulo minimo y maximo para el scanneo


TOPIC_VEL = '/dm3/cmd_vel'
PI = 3.1416

# create a twist message
twist = Twist()
twist.linear.y = 0; twist.linear.z = 0;
twist.angular.x = 0; twist.angular.y = 0;


# publish to cmd_vel
p = rospy.Publisher(TOPIC_VEL, Twist, queue_size = QUEUE_SIZE)

def recta():

    # set speeds for describe a square path
    twist.linear.x = 50;
    twist.angular.z = 0;
    p.publish(twist)

    time.sleep(50)    

def detener():
    twist.linear.x = 0;
    twist.angular.z = 0;
    p.publish(twist)

def listener():
    # In ROS, nodes are uniquely named. If two nodes with the same
    # node are launched, the previous one is kicked off. The
    # anonymous=True flag means that rospy will choose a unique
    # name for our 'listener' node so that multiple listeners can
    # run simultaneously.
    rospy.init_node('listener', anonymous=True)

    # spin() simply keeps python from exiting until this node is stopped
    #rospy.spin()

if __name__ == '__main__':
    listener()
    recta()
    detener()
