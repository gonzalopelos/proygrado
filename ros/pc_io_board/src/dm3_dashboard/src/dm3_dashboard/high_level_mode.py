# -*- coding: utf-8 -*-

from __future__ import print_function # to use print like in python 3.0

import rospy
from functools import partial

import threading
import os


from std_srvs.srv import *
from std_msgs.msg import String

from topic_tools.srv import MuxSelect


from rqt_robot_dashboard.widgets import MenuDashWidget
from python_qt_binding.QtCore import QSize

class HighLevelMode(MenuDashWidget):

    def __init__(self, ns):
        self.STATE_JOYSTICK_MODE = 0
        self.STATE_ZED_MODE = 1
          
        self._node_service = ns
        self.connect_service()
        
        self.working_dir = os.path.dirname(os.path.abspath(__file__))

        self._joystick_icon = [self.working_dir + '/../../images/joystick.svg']
        self._zed_icon = [self.working_dir + '/../../images/zed.svg']      

        icons = [self._joystick_icon, self._zed_icon]
        super(HighLevelMode, self).__init__(ns, icons=icons)
        self.setFixedSize(QSize(40,40))

        self.add_action("Joystick Mode", partial(self.update_state, self.STATE_JOYSTICK_MODE))
        self.add_action("Zed Mode", partial(self.update_state, self.STATE_ZED_MODE))
                
        self.setToolTip("High Level Mode")
        rospy.Subscriber('/mux_cmd_vel/selected', String, self.actual_mode)
        
        
        #self.launch_thread()
        
    """
    def check_high_level_mode(self):
        print ("hola")

    def launch_thread(self):
        threading.Thread(target=self.check_high_level_mode).start()"""
        

    def connect_service(self):
        nsM = self._node_service + "/select"

        print ("Wait for service " + nsM + " ... ", end="")
        rospy.wait_for_service(nsM)
        print ("OK")
        
        
        try:
            self._set_mode_service = rospy.ServiceProxy(nsM, MuxSelect)
            
                        
        except rospy.ServiceException, e:
            print ("Service call failed: %s"%e)
            
        
            
    def update_state(self, state):
        super(HighLevelMode, self).update_state(state)
        if state is self.STATE_JOYSTICK_MODE:
            self._set_mode_service('/dm3/cmd_vel/joy')
        else:
            self._set_mode_service('/dm3/cmd_vel/zed')
    
    def actual_mode(self, str_mode):
        if (str_mode.data == "/dm3/cmd_vel/joy"):
            super(HighLevelMode, self).update_state(self.STATE_JOYSTICK_MODE)
            self.setToolTip("Joystick mode")
        else:
            super(HighLevelMode, self).update_state(self.STATE_ZED_MODE)
            self.setToolTip("Zed mode")            

                       
    def close(self):
        self._pub.unregister()
