import roslib
import rospy

import diagnostic_msgs

from rqt_robot_dashboard.dashboard import Dashboard
from rqt_robot_dashboard.widgets import MonitorDashWidget, ConsoleDashWidget, MenuDashWidget, IconToolButton
from QtGui import QMessageBox, QAction
from python_qt_binding.QtCore import QSize
from bencode import bencode, bdecode, BTFailure
from dm3_core.msg import *

from .battery_widget import BatteryWidget
from .motor_control_widget import MotorControlWidget
from .motor_state_widget import MotorStateWidget
from .motor_drive_widget import MotorDriveWidget
from .high_level_mode import HighLevelMode

class DM3Dashboard(Dashboard):
    def setup(self, context):
        self.message = None
        self._dashboard_message = None
        self._last_dashboard_message_time = 0.0
        self._motor_widget = MotorStateWidget('DM3 Motors State')       
        self._motor_drive_widget = MotorDriveWidget('DM3 Motors Drive')       
        self._laptop_bat = BatteryWidget("Laptop")
        self._dm3_battery = BatteryWidget("DM3")
        self._mcc2pc_sub = rospy.Subscriber('/dm3/mcc2pc', MCC, self.dashboard_callback)

    def get_widgets(self):
        nodeMC = 'dm3/io_motor_module'
        nodeHM = 'mux_cmd_vel'
        #list of buttons (each button is a list)   
        monitor = [MonitorDashWidget(self.context)]
        console = [ConsoleDashWidget(self.context)]
        motors = [self._motor_widget, self._motor_drive_widget, MotorControlWidget(nodeMC)]
        batteries = [self._laptop_bat, self._dm3_battery]
        modes = [HighLevelMode(nodeHM)]
        return [monitor, console, motors, batteries, modes]

    def dashboard_callback(self, msg):
        self._dashboard_message = msg
        self._last_dashboard_message_time = rospy.get_time()
        
	if (msg.pid == MCC.DM3_PID and msg.opcode == MCC.GET_BAT):
        	bat_level = float(bdecode(msg.params)[0])
		self._dm3_battery.update_perc(bat_level)
                # This should be self._last_dashboard_message_time?
                # Is it even used graphically by the widget
                self._dm3_battery.update_time(bat_level)

    def shutdown_dashboard(self):
        self._dashboard_agg_sub.unregister()
