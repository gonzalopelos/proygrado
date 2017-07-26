#!/usr/bin/env python

import rospy
import sensor_msgs.msg
from std_msgs.msg import String
from geometry_msgs.msg import Twist
import dynamic_reconfigure.client

QUEUE_SIZE = 10

MIN_DIST_VALUE = 0.002 	# no usar range.range_min pues devuelve cero (m)
MAX_DIST_VALUE = 5	# distancia maxima del laser (m)

MIN_POWER_MOTORS = 20
MAX_POWER_MOTORS = 100

MIN_ANGLE = -1; MAX_ANGLE = -MIN_ANGLE # angulo minimo y maximo para el scanneo

SAFE_DISTANCE = 1 # distancia a partir de la cual no es seguro mover el robot en metros

# coeficientes de la recta para convertir linealmente distancias del laser a potencias de motores
A = (MAX_POWER_MOTORS-MIN_POWER_MOTORS)/(MAX_DIST_VALUE - MIN_DIST_VALUE)
B = (MIN_POWER_MOTORS * MAX_DIST_VALUE - MAX_POWER_MOTORS * MIN_DIST_VALUE) / (MAX_DIST_VALUE - MIN_DIST_VALUE)

# create a twist message
twist = Twist()
twist.linear.y = 0; twist.linear.z = 0;
twist.angular.x = 0; twist.angular.y = 0;

# publish to cmd_vel
p = rospy.Publisher('laser_cmd_vel', Twist, queue_size = QUEUE_SIZE)

def callback(data):

    #minValue = data.ranges.min()
    #minIndex = np.where(data.ranges == minValue)
    minValue = data.range_max
    minIndex = -1

    #rospy.loginfo(rospy.get_caller_id() + "Range in  (%d, %d)", data.range_min, data.range_max)
    #rospy.loginfo(rospy.get_caller_id() + "Data range len is %d", len(data.ranges))

    for index, elem in enumerate(data.ranges):
        if (minValue>elem and elem>MIN_DIST_VALUE):
            minValue = elem
            minIndex = index
    egoAng = -(data.angle_min + minIndex * data.angle_increment)
    #rospy.loginfo(rospy.get_caller_id() + "Min value %f is in %d with absolute angle %f", minValue, minIndex, egoAng)
    # http://docs.ros.org/api/sensor_msgs/html/msg/LaserScan.html

    if (minValue<SAFE_DISTANCE): # si hay algo cerca detengo
	linearSpeed = 0
    elif (minValue == data.range_max): # si no veo nadd detengo
	linearSpeed = 0
    else:
        linearSpeed = A * minValue + B

    # fill the details in twist
    twist.linear.x = linearSpeed;
    twist.angular.z = egoAng;
    p.publish(twist)

def listener():
    # In ROS, nodes are uniquely named. If two nodes with the same
    # node are launched, the previous one is kicked off. The
    # anonymous=True flag means that rospy will choose a unique
    # name for our 'listener' node so that multiple listeners can
    # run simultaneously.
    rospy.init_node('listener', anonymous=True)

    # set laser parameters
    client = dynamic_reconfigure.client.Client('hokuyo_node')
    params = {'min_ang' : MIN_ANGLE, 'max_ang' : MAX_ANGLE}
    rospy.loginfo(rospy.get_caller_id() + "Angle range in  (%d, %d)", MIN_ANGLE, MAX_ANGLE)
    config = client.update_configuration(params)

    # suscribe to scan topic
    rospy.Subscriber("scan", sensor_msgs.msg.LaserScan, callback)

    # spin() simply keeps python from exiting until this node is stopped
    rospy.spin()

if __name__ == '__main__':
    listener()
