#!/usr/bin/env python

# -*- coding: utf-8 -*-


# Based on https://raw.githubusercontent.com/ros/diagnostics/indigo-devel/diagnostic_updater/src/example.py

"""
@author Gonzalo Tejera <gtejera [fing.edu.uy]>
"""

import roslib
import rospy
import diagnostic_updater
import diagnostic_msgs
import std_msgs
from sensor_msgs.msg import Imu

def newMsg(imu):
    imu_freq.tick(imu.header.stamp)
    updater.update()

if __name__=='__main__':
    rospy.init_node("diagnostic_updater_myahrs")

    # The Updater class advertises to /diagnostics, and has a
    # ~diagnostic_period parameter that says how often the diagnostics
    # should be published.
    updater = diagnostic_updater.Updater()
    updater.setHardwareID("MyAHRS+ IMU")
    # ROS command: rostopic echo diagnostics -c --filter "m.status[0].hardware_id == 'MyAHRS+ IMU'"

    # Some diagnostic tasks are very common, such as checking the rate
    # at which a topic is publishing, or checking that timestamps are
    # sufficiently recent. FrequencyStatus and TimestampStatus can do these
    # checks for you.
    #
    # Usually you would instantiate them via a HeaderlessTopicDiagnostic
    # (FrequencyStatus only, for topics that do not contain a header) or a
    # TopicDiagnostic (FrequencyStatus and TimestampStatus, for topics that
    # do contain a header).
    #
    # Some values are passed to the constructor as pointers. If these values
    # are changed, the FrequencyStatus/TimestampStatus will start operating
    # with the new values.
    #
    # Refer to diagnostic_updater.FrequencyStatusParam and
    # diagnostic_updater.TimestampStatusParam documentation for details on
    # what the parameters mean:
    freq_bounds = {'min':90, 'max':110} # If you update these values, the
    # HeaderlessTopicDiagnostic will use the new values.
    imu_freq = diagnostic_updater.TopicDiagnostic("/imu/data_raw", updater,
        diagnostic_updater.FrequencyStatusParam(freq_bounds, 0.1, 10),  
        diagnostic_updater.TimeStampStatusParam(min_acceptable = -1, max_acceptable = 5))

    # If we know that the state of the node just changed, we can force an
    # immediate update.
    updater.force_update()
    rospy.Subscriber("/imu/data_raw", Imu, newMsg)

    rospy.spin()
