#!/usr/bin/env python

import roslib #; roslib.load_manifest('pr2_motors_analyzer')

import rospy
from diagnostic_msgs.msg import DiagnosticArray, DiagnosticStatus, KeyValue

if __name__ == '__main__':
    rospy.init_node('motor_power_sim')

    pub = rospy.Publisher('/diagnostics', DiagnosticArray, queue_size=10)
    
    array = DiagnosticArray()
    # Fake power board status, estop is on
    power_stat = DiagnosticStatus(name = 'Power board 1000', level = 0, 
                                  message = 'Running')
    power_stat.values = [ KeyValue(key = 'Runstop hit', value = 'False'),
                          KeyValue(key = 'Estop hit', value = 'False')]
    # Fake EtherCAT Master status, all OK
    eth_stat = DiagnosticStatus(name='EtherCAT Master', level = 0,
                                message = 'OK')

    array.status = [ power_stat, eth_stat ]

    my_rate = rospy.Rate(1.0)
    while not rospy.is_shutdown():
        pub.publish(array)
        my_rate.sleep()
