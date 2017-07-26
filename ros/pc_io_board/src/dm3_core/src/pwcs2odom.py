#!/usr/bin/env python

import rospy
from geometry_msgs.msg import PoseWithCovarianceStamped, TwistWithCovariance
from nav_msgs.msg import Odometry

# create a twist message
empty_twc = TwistWithCovariance()
odom = Odometry();
odom.twist = empty_twc
QUEUE_SIZE = 10
p = rospy.Publisher('odom', Odometry, queue_size = QUEUE_SIZE)

def callback(data):
    odom.header = data.header
    odom.pose = data.pose
    p.publish(odom)

def listener():
    # In ROS, nodes are uniquely named. If two nodes with the same
    # node are launched, the previous one is kicked off. The
    # anonymous=True flag means that rospy will choose a unique
    # name for our 'listener' node so that multiple listeners can
    # run simultaneously.
    rospy.init_node('pwcs2odom', anonymous=True)

    # suscribe to scan topic
    rospy.Subscriber("robot_pose_ekf/odom_combined", PoseWithCovarianceStamped, callback)

    # spin() simply keeps python from exiting until this node is stopped
    rospy.spin()

if __name__ == '__main__':
    listener()
