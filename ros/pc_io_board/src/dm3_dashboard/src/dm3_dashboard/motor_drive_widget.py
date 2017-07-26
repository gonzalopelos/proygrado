import rospy
from functools import partial

import os

from rqt_robot_dashboard.widgets import IconToolButton
from python_qt_binding.QtCore import QSize

from dm3_core.srv import *
from dm3_core.msg import MCC

from bencode import bencode, bdecode, BTFailure

class MotorDriveWidget(IconToolButton):
    def __init__(self, ns):
        self.working_dir = os.path.dirname(os.path.abspath(__file__))
        self._none_mode_icon = ['bg-grey.svg', 'ic-steering-wheel.svg']
        self._jorge_mode_icon = [self.working_dir + '/../../images/jorge.svg']
        self._pancho_mode_icon = [self.working_dir + '/../../images/pancho.svg']
        self._raw_mode_icon = ['bg-red.svg', 'ic-steering-wheel.svg']
        # Los numeros de estado se corresponden con el indice del array icons segun el orden en que se colocan los icones en el.
        self.STATE_NONE = 3 # los primeros tres valores se reservan por los codigos MCC para DRIVE_MODE
        self._state = self.STATE_NONE

        icons = [self._jorge_mode_icon, self._pancho_mode_icon, self._raw_mode_icon, self._none_mode_icon]
        self._tool_tip = ["Jorge motors drive mode.", "Panch motors drive mode.", "Raw motors drive mode.", "Unknown mode"]

        super(MotorDriveWidget, self).__init__(ns, icons=icons)
        self.setFixedSize(QSize(40,40))

        super(MotorDriveWidget, self).update_state(self.STATE_NONE)
        self.setToolTip("Unknown mode")

        self.clicked.connect(self.get_status)
        self.connect_service()

    def update_state(self):
        self.setToolTip(self._tool_tip[self._state])
        super(MotorDriveWidget, self).update_state(self._state)

    def mmc_response(self, mcc):
        if ((mcc.pid == MCC.DM3MOTORS_PID) and (mcc.opcode == MCC.DRIVE_MODE)):
            self._state = bdecode(mcc.params)[0]
            self.update_state()

    def connect_service(self):
        self._pub = rospy.Subscriber('/dm3/mcc2pc', MCC, self.mmc_response)

    def get_status(self):
        # TODO get_status
        self._state = self.STATE_NONE
        self.update_state()

    def close(self):
        self._pub.unregister()
        #pass
