#!/usr/bin/env python
import time
import math
import roslib
import rospy
from geometry_msgs.msg import Twist
from std_srvs.srv import *
from diagnostic_msgs.msg import DiagnosticArray, DiagnosticStatus, KeyValue

from dynamixel_hr_ros.msg import CommandSpeed

d_p = 0.81/0.88 # width / length TODO

dxl_speed_pub = None

# Identificadores de los motores en el bus DLX
ID_MOTOR1_CHASIS1 = 18
ID_MOTOR2_CHASIS1 = 1
ID_MOTOR1_CHASIS2 = 5
ID_MOTOR2_CHASIS2 = 3

MAX_VEL = 5.0 # metros por segundo
MIN_VEL = 0.2 # metros por segundo

MAX_POW = 11 # potencia maxima
MIN_POW = 1

def setVel(twist):
    fx = twist.linear.x * math.cos(twist.angular.z)
    fy = twist.linear.x * math.sin(twist.angular.z)
    ls = int(fx + d_p*fy) # velocidad izquierda
    rs = int(fx - d_p*fy)
    if  (ls>MAX_VEL):
        ls = MAX_VEL
        rospy.loginfo("Velocidad izquierdsa muy alta")

    if  (rs>MAX_VEL):
        rs = MAX_VEL
        rospy.loginfo("Velocidad derecha muy alta")

    lp = int(ls / MAX_VEL * MAX_POW) # potencia izquierda
    rp = int(rs / MAX_VEL * MAX_POW)

    #speed_cmd = CommandSpeed([ID_MOTOR1_CHASIS1, ID_MOTOR2_CHASIS1], [lp, -rp])
    speed_cmd = CommandSpeed([ID_MOTOR1_CHASIS1, ID_MOTOR2_CHASIS1, ID_MOTOR1_CHASIS2, ID_MOTOR2_CHASIS2], [lp, -rp, lp, -rp])

    dxl_speed_pub.publish(speed_cmd)

if __name__ == '__main__':
    node_name = 'motor_dynamixel_controller'
    service_prefix = 'dm3/' + node_name + '/'

    rospy.init_node(node_name)
    diag_pub = rospy.Publisher('/diagnostics', DiagnosticArray, queue_size=10)
    dxl_speed_pub = rospy.Publisher('/dxl/command_speed', CommandSpeed, queue_size=10)

    rospy.Subscriber("/dm3/cmd_vel", Twist, setVel)

    rospy.spin()
