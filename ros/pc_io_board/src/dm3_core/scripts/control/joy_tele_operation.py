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

QUEUE_SIZE = 10

MIN_VEL_VALUE = 0.002
MAX_VEL_VALUE = 5

MIN_POWER_MOTORS = 20
MAX_POWER_MOTORS = 100
MAX_VEL_M_S = 2.0 # max speed in m/s

MAX_STEER_RADIANS = 1.5

ROLL_IND = 0
PITCH_IND = 1
STATE_BUTTON_IND = 0
SAFE_BUTTON_IND = 1
# array index that code the control position in the y-axis, 1 up, 0 released and -1 down
CONTROL_Y_AXIS = 7
CONTROL_X_AXIS = 6

MIN_ROT = -0.4; MAX_ROT = -MIN_ROT

A = (MAX_POWER_MOTORS-MIN_POWER_MOTORS)/(MAX_ROT - MIN_ROT)
B = (MIN_POWER_MOTORS * MAX_ROT - MAX_POWER_MOTORS * MIN_ROT) / (MAX_ROT - MIN_ROT)

# create a twist message
twist = Twist()
twist.linear.y = 0; twist.linear.z = 0;
twist.angular.x = 0; twist.angular.y = 0;

# publish to cmd_vel
p = rospy.Publisher('/dm3/cmd_vel/joy', Twist, queue_size = QUEUE_SIZE)

# state machine to control autonomous or telepo 
TELEOP_STATE = 0
TELEOP_WR_STATE = 1 # wait until button release
AUTONO_STATE = 2
AUTONO_WR_STATE = 3
state_mode = TELEOP_STATE

SAFE_RELEASE_STATE = 0
SAFE_PRESS_STATE = 1
safe_state = SAFE_RELEASE_STATE
set_drive_mode_remote = None

def changeCmdVelMux(newTopic):
    try:
      rospy.wait_for_service('mux_cmd_vel/select', 5)
    except rospy.ROSException, e:
      rospy.loginfo(rospy.get_caller_id() + "Service call failed: %s", e)

    select_srv = rospy.ServiceProxy('mux_cmd_vel/select', MuxSelect)
    select_srv(newTopic)

def set_drive_mode(mode):
    global set_drive_mode_remote

    name_service = '/dm3/io_motor_module/set_drive_mode'
    rospy.wait_for_service(name_service)
    try:
        set_drive_mode_remote = rospy.ServiceProxy(name_service, Int)
        set_drive_mode_remote(mode)
    except rospy.ServiceException, e:
        print "Service call failed: %s"%e

def enable_motors(enable):
    name_service = 'dm3/io_motor_module/enable'
    rospy.wait_for_service(name_service)
    try:
        enable_remote = rospy.ServiceProxy(name_service, Bool)
        enable_remote(enable)
    except rospy.ServiceException, e:
        print "Service call failed: %s"%e

def callback(joy):
    global state_mode, safe_state, set_drive_mode_remote

    #rospy.loginfo(rospy.get_caller_id() + "Min value %f is in %d with absolute angle %f", minValue, minIndex, egoAng)

    #linearSpeed = -math.copysign(1, joy.axes[PITCH_IND]) * math.sqrt(math.pow(joy.axes[ROLL_IND], 2)+math.pow(joy.axes[PITCH_IND], 2))

    # si esta presionado del boton de habilitacion de control teleoperado
    if (joy.buttons[STATE_BUTTON_IND]==1):
      if (state_mode==AUTONO_STATE):
        state_mode = AUTONO_WR_STATE
      elif (state_mode==TELEOP_STATE):
        state_mode = TELEOP_WR_STATE

    # armo el vertor de control en base a la informacion del joy
    if (joy.buttons[SAFE_BUTTON_IND]==1):
      egoAng = -joy.axes[ROLL_IND]/MAX_ROT*math.pi/2
      linearSpeed = joy.axes[PITCH_IND]/MAX_ROT
      signoLS = math.copysign(1,linearSpeed)
      linearSpeed = -math.copysign(abs(linearSpeed) * MAX_VEL_M_S, signoLS)  # escalo la velocidad para que varie en el rango 20-
      safe_state = SAFE_PRESS_STATE
      twist.linear.x = linearSpeed;
      twist.angular.z = egoAng;
      p.publish(twist)
    # si estaba mandando comandos de velocidad manda un comando con velocidad cero solo una vez
    elif (safe_state == SAFE_PRESS_STATE):
      safe_state = SAFE_RELEASE_STATE
      twist.linear.x = 0;
      twist.angular.z = 0;
      p.publish(twist)

    # Cuando se suelta el boton de modo, ajusta el modo de operacion # no sacar segundo and controla que mande solo una vez parar
    if ((joy.buttons[STATE_BUTTON_IND]==0) and ((state_mode==AUTONO_WR_STATE) or (state_mode==TELEOP_WR_STATE))):
      if (state_mode==AUTONO_WR_STATE):
        state_mode = TELEOP_STATE
        changeCmdVelMux('/dm3/cmd_vel/joy')
      else:
        state_mode = AUTONO_STATE
        changeCmdVelMux('/dm3/cmd_vel/zed')
      # siempre que cambie de modo manda velocidad cero
      twist.linear.x = 0;
      twist.angular.z = 0;
      p.publish(twist)


    if (joy.axes[CONTROL_Y_AXIS] == 1):
      set_drive_mode_remote(0) # modo vectorial de jorge
    elif (joy.axes[CONTROL_Y_AXIS] == -1):
      set_drive_mode_remote(1) # modo vectorial de pancho
    
    if (joy.axes[CONTROL_X_AXIS] == -1):
      enable_motors(True)
    elif (joy.axes[CONTROL_X_AXIS] == 1):
      enable_motors(False)


def listener():
    # In ROS, nodes are uniquely named. If two nodes with the same
    # node are launched, the previous one is kicked off. The
    # anonymous=True flag means that rospy will choose a unique
    # name for our 'listener' node so that multiple listeners can
    # run simultaneously.
    rospy.init_node('joy_tele_operation', anonymous=True)

    # suscribe to scan topic
    rospy.Subscriber("joy", sensor_msgs.msg.Joy, callback)

    # spin() simply keeps python from exiting until this node is stopped
    rospy.spin()

if __name__ == '__main__':
    set_drive_mode(0)
    listener()
