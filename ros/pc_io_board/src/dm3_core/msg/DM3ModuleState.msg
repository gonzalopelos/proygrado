# DM3 Module State Messages

###### CONSTANTS ######
float32 INI_BATTERY_LEVEL = -1
uint16 INI_SIREN_REMAINIG = 0

###### MESSAGE ######

Header header

###################
# Core Packet
###################
float32 battery_level   # battery level in 0..100
uint16 siren_remaining 	# siren on remaining time