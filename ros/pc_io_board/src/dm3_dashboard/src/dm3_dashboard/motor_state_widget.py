import rospy
from functools import partial

#from kobuki_msgs.msg import MotorPower

from rqt_robot_dashboard.widgets import IconToolButton
from python_qt_binding.QtCore import QSize

from dm3_core.srv import *
from dm3_core.msg import MCC

from bencode import bencode, bdecode, BTFailure

class MotorStateWidget(IconToolButton):
    def __init__(self, ns):
        self._disable_icon = ['bg-grey.svg', 'ic-motors.svg']
        self._disable_break_icon = ['bg-grey.svg', 'ic-motors.svg', 'ol-stop-badge.svg']
        self._disable_stale_icon = ['bg-grey.svg', 'ic-motors.svg', 'ol-stale-badge.svg']
        self._enable_icon = ['bg-green.svg', 'ic-motors.svg']
        self._enable_break_icon = ['bg-green.svg', 'ic-motors.svg', 'ol-stop-badge.svg']
        self._break_motors_ns = ns

        self.STATE_DISABLE_STALE = 0
        self.STATE_DISABLE = 1
        self.STATE_DISABLE_BREAK = 2
        self.STATE_ENABLE = 3
        self.STATE_ENABLE_BREAK = 4

        self._breaks_on = False
        self._enable = False

        icons = [self._disable_stale_icon, self._disable_icon, self._disable_break_icon, self._enable_icon, self._enable_break_icon]
        super(MotorStateWidget, self).__init__(ns, icons=icons)
        self.setFixedSize(QSize(40,40))

        super(MotorStateWidget, self).update_state(self.STATE_DISABLE_STALE)
        self.setToolTip("Motors idle")

        self.clicked.connect(self.get_status)
        self.connect_service()

    def update_state(self):
        if self._breaks_on and self._enable:
            super(MotorStateWidget, self).update_state(self.STATE_ENABLE_BREAK)
            self.setToolTip("Motors enabled and breaked")
        elif not self._breaks_on and self._enable:
            super(MotorStateWidget, self).update_state(self.STATE_ENABLE)
            self.setToolTip("Motors enabled")
        elif self._breaks_on and not self._enable:
            super(MotorStateWidget, self).update_state(self.STATE_DISABLE_BREAK)
            self.setToolTip("Motors disabled and breaked")
        elif not self._breaks_on and not self._enable:
            super(MotorStateWidget, self).update_state(self.STATE_DISABLE)
            self.setToolTip("Motors disabled")

    def mmc_response(self, mcc):
        if (mcc.pid == MCC.DM3MOTORS_PID):
            if mcc.opcode == MCC.ENABLE:
                self._enable = bdecode(mcc.params)[0]==1
            elif mcc.opcode == MCC.BREAK:
                self._breaks_on = bdecode(mcc.params)[0]==1
            self.update_state()

    def connect_service(self):
        rospy.Subscriber('/dm3/mcc2pc', MCC, self.mmc_response)

    def get_status(self):
        # TODO get_status
        super(MotorStateWidget, self).update_state(self.STATE_DISABLE_STALE)
        self.setToolTip("Motors idle")

    def close(self):
        #self._pub.unregister()
        pass
