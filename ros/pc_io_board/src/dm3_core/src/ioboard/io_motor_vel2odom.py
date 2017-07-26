#!/usr/bin/env python
import time
import roslib
import rospy
import numpy as np
from math import pi, atan, cos, sin
from geometry_msgs.msg import Twist, Quaternion
from nav_msgs.msg import Odometry
from std_srvs.srv import *
from diagnostic_msgs.msg import DiagnosticArray, DiagnosticStatus, KeyValue
from dm3_core.srv import *
from dm3_core.msg import *
from tf.transformations import quaternion_about_axis

precision = 0.005

R = 0.82
S = 0.70
vx, vy, az = 0.0, 0.0, 0.0
x, y, az = 0.0, 0.0, 0.0

# other method
d, tita = 0.0, 0.0


def get_wheel_speed(motor_state):
    global vx, vy, az, x, y, az, d, tita

    v1 = motor_state.wheel_speed_1[0]
    v2 = motor_state.wheel_speed_1[1]

    # other method
    d = (v1 + v2) / 2.0 if (abs(v1+v2) > precision) else 0.0
    tita = (v2 - v1) / R if (abs(v1-v2) > precision) else 0.0

    # convert wheel speed to Vx, Vy and Az
    # vx = (v1 + v2) / 2.0 if (abs(v1+v2) > precision) else 0.0
    # vy = R * (v2 - v1) / S  if (abs(v1-v2) > precision) else 0.0

    # az = atan( vy / vx ) if vx != 0 else 0.0
    # if abs(az) > 3*pi/8:
    #     az = 0
    # print v1, v2, vx, vy, az*180/pi


if __name__ == '__main__':
    node_name = 'io_motor_vel2odom'

    rospy.init_node(node_name)
    motor_state = rospy.Subscriber('/dm3/motors_state', MotorsState, get_wheel_speed)
    odom_pub = rospy.Publisher('/wheel_odometry', Odometry, queue_size=10)

    msg_odom = Odometry()
    
    last_time = rospy.Time.now()

    report_status_rate = rospy.Rate(1)
    while not rospy.is_shutdown():

        current_time = rospy.Time.now()

        msg_odom.header.frame_id = "odom_wheel"
        msg_odom.header.stamp = current_time
        msg_odom.child_frame_id = ""

        dt = current_time.secs - last_time.secs
        # conversion coord to map frame
        # delta_x = (vx * cos(az) - vy * sin(az)) * dt
        # delta_y = (vx * sin(az) + vy * cos(az)) * dt
        # delta_az = az * dt

        # x += delta_x
        # y += delta_y
        # az += delta_az

        # other method
        az += tita
        x += d * cos(az)
        y += d * sin(az)


        msg_odom.pose.pose.position.x = x
        msg_odom.pose.pose.position.y = y
        msg_odom.pose.pose.position.z = 0.0
        msg_odom.pose.pose.orientation = Quaternion(*quaternion_about_axis(az,(0,0,1)))
        msg_odom.pose.covariance = [0.01, 0.0, 0.0, 0.0, 0.0, 0.0, 
                                    0.0, 0.01, 0.0, 0.0, 0.0, 0.0, 
                                    0.0, 0.0, 0.01, 0.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.01, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0.01, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0.0, 0.01]

        msg_odom.twist.twist.linear.x = vx
        msg_odom.twist.twist.linear.y = vy
        msg_odom.twist.twist.linear.z = 0
        msg_odom.twist.twist.angular.x = 0
        msg_odom.twist.twist.angular.y = 0
        msg_odom.twist.twist.angular.z = az
        msg_odom.twist.covariance = [0.01, 0.0, 0.0, 0.0, 0.0, 0.0, 
                                    0.0, 0.01, 0.0, 0.0, 0.0, 0.0, 
                                    0.0, 0.0, 0.01, 0.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.01, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0.01, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0.0, 0.01]


        odom_pub.publish(msg_odom)

        last_time = current_time

        report_status_rate.sleep()
