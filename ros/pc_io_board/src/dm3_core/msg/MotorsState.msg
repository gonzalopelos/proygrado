# DM3 Motor State Messages


###### CONSTANTS ######
uint8 MIN_POWER = 0
uint8 MAX_POWER = 100

###### MESSAGE ######

Header header

###################
# Core Packet
###################
bool enabled      		# are the motors enabled
bool braked      		# are the motors breaked
bool reversed      		# are the motors reversed
uint8 drive_mode  		# actual drive vector mode
uint8 controller_type   # controller type in {UNSENSORED, PID}

###################
# Chasis wheel power
###################
float32[] wheel_power_1     # [left_power, right_power] set on chasis 1
float32[] wheel_power_2		# [left_power, right_power] set on chasis 2

###################
# Chasis wheel speed
###################
float32[] wheel_speed_1     # actual speed [left_speed, right_speed] set on chasis 1
float32[] wheel_speed_2		# actual speed [left_speed, right_speed] set on chasis 2

float32[] wheel_target_speed_1     # idem tarject speed set on chasis 1
float32[] wheel_target_speed_2

float32[] pidk #actual pid parameters [kp, ki, kd]	 