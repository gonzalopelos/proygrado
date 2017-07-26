from __future__ import print_function # to use print like in python 3.0

import rospy
from functools import partial

from dm3_core.srv import Bool

from std_srvs.srv import *


from rqt_robot_dashboard.widgets import MenuDashWidget
from python_qt_binding.QtCore import QSize

class MotorControlWidget(MenuDashWidget):

    def __init__(self, ns):
        self.STATE_DISABLE = 0
        self.STATE_ENABLE = 1
        self.STATE_NOT_BREAKED = 2        
        self.STATE_BREAKED = 3      
        self.STATE_REVERSE = 4
        self.STATE_TEST = 5
          
        self._node_service = ns
        self.connect_service()

        self._off_icon = ['bg-grey.svg', 'ic-motors.svg']
        self._green_icon = ['bg-green.svg', 'ic-motors.svg']
        self._red_icon = ['bg-red.svg', 'ic-motors.svg']
        self._orange_icon = ['bg-orange.svg', 'ic-motors.svg']
        self._yellow_icon = ['bg-yellow.svg', 'ic-motors.svg']
        

        icons = [self._off_icon, self._green_icon, self._red_icon, self._orange_icon, self._yellow_icon]
        super(MotorControlWidget, self).__init__(ns, icons=icons)
        self.setFixedSize(QSize(40,40))

        self.add_action("Disable", partial(self.update_state, self.STATE_DISABLE))
        self.add_action("Enable", partial(self.update_state, self.STATE_ENABLE))
        self.add_action("!Break", partial(self.update_state, self.STATE_NOT_BREAKED))
        self.add_action("Break", partial(self.update_state, self.STATE_BREAKED))
        self.add_action("Reverse", partial(self.update_state, self.STATE_REVERSE))        
        self.add_action("Test", partial(self.update_state, self.STATE_TEST))
        
        self.setToolTip("Motors disabled")
        self._break_service(False)
        self._enable_service(False)
        

    def connect_service(self):
        nsb = self._node_service + "/break"
        nse = self._node_service + "/enable"        
        nsr = self._node_service + "/reverse"
        nst = self._node_service + "/test"

        print ("Wait for service " + nsb + " ... ", end="")
        rospy.wait_for_service(nsb)
        print ("OK")
        print ("Wait for service " + nse + " ... ", end="")
        rospy.wait_for_service(nse)
        print ("OK")
        print ("Wait for service " + nsr + " ... ", end="")
        rospy.wait_for_service(nsr)
        print ("OK")
        print ("Wait for service " + nst + " ... ", end="")
        rospy.wait_for_service(nst)
        print ("OK")
        
        try:
            self._break_service = rospy.ServiceProxy(nsb, Bool)
            self._enable_service = rospy.ServiceProxy(nse, Bool)            
            self._reverse_service = rospy.ServiceProxy(nsr, Bool)
            self._test_service = rospy.ServiceProxy(nst, Empty)
                        
        except rospy.ServiceException, e:
            print ("Service call failed: %s"%e)

    def update_state(self, state):
        super(MotorControlWidget, self).update_state(state)
        
        if state is self.STATE_ENABLE:
            self._enable_service(True)
            self.setToolTip("Motors enabled")
        elif state is self.STATE_BREAKED:
            self._break_service(True)
            self.setToolTip("Motors breaked")
        elif state is self.STATE_REVERSE:
            self._reverse_service(True)            
            self.setToolTip("Motors reversed")
        elif state is self.STATE_TEST:
            self._enable_service(True)
            self._test_service()            
            self.setToolTip("Motors testing")
        elif state is self.STATE_DISABLE:
            self._enable_service(False)
            self.setToolTip("Motors disenabled")
        elif state is self.STATE_NOT_BREAKED:
            self._break_service(False)
            self.setToolTip("Motors not breaked")
        else:
            self._break_service(True)
            self._enable_service(False)
            self.setToolTip("Motors disabled")

    def close(self):
        self._pub.unregister()
    
