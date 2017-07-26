#!/usr/bin/env python

import rospy
from geometry_msgs.msg import PoseWithCovarianceStamped, TwistWithCovariance
from nav_msgs.msg import Odometry

# create a twist message
empty_twc = TwistWithCovariance()
odom = Odometry();
odom.twist = empty_twc
QUEUE_SIZE = 10
p = rospy.Publisher('/zed/vo_covariance', Odometry, queue_size = QUEUE_SIZE)

def callback(data):
    odom.header = data.header
    odom.pose.pose = data.pose.pose
    odom.pose.pose.position.z = 0
    odom.pose.covariance =  [0.005, 0.0, 0.0, 0.0, 0.0, 0.0, 
                            0.0, 0.005, 0.0, 0.0, 0.0, 0.0, 
                            0.0, 0.0, 0.005, 0.0, 0.0, 0.0,
                            0.0, 0.0, 0.0, 0.005, 0.0, 0.0,
                            0.0, 0.0, 0.0, 0.0, 0.005, 0.0,
                            0.0, 0.0, 0.0, 0.0, 0.0, 0.005]
    odom.twist.twist      = data.twist.twist
    odom.twist.covariance = [0.005, 0.0, 0.0, 0.0, 0.0, 0.0, 
                            0.0, 0.005, 0.0, 0.0, 0.0, 0.0, 
                            0.0, 0.0, 0.005, 0.0, 0.0, 0.0,
                            0.0, 0.0, 0.0, 0.005, 0.0, 0.0,
                            0.0, 0.0, 0.0, 0.0, 0.005, 0.0,
                            0.0, 0.0, 0.0, 0.0, 0.0, 0.005]
    p.publish(odom)

def listener():
   
    rospy.init_node('add_covariance', anonymous=True)

    # suscribe to scan topic
    rospy.Subscriber("/zed/vo_zed", Odometry, callback)

    # spin() simply keeps python from exiting until this node is stopped
    rospy.spin()

if __name__ == '__main__':
    listener()
