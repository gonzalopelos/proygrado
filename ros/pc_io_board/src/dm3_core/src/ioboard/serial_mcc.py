#!/usr/bin/env python
import time
import roslib
import rospy
import serial
from bencode import bencode, bdecode, BTFailure
import std_srvs.srv
from diagnostic_msgs.msg import DiagnosticArray, DiagnosticStatus, KeyValue
import diagnostic_updater
from dm3_core.srv import *
from dm3_core.msg import *

# Possible levels of operations to DiagnosticStatus
OK_LEVEL = 0
WARN_LEVEL = 1
ERROR_LEVEL = 2
STALE_LEVEL = 3

DIAGNOSTIC_IOBOARD_ID = 'IO Board'
DIAGNOSTIC_SERIAL_COMM_NAME = 'serial_mcc' # no cambiar pues updater usa ese nombre

# Time out for the IO Board serial communication
SERIAL_TIME_OUT = 2
SERIAL_PORT = '/dev/dm3/mbed-serial'
SERIAL_BAUD = 9600
SERIAL_RECONNECT_TIME = 2

ser = None # serial comminucation
readIn = '' # characters readed from serial comm
diagMsg = DiagnosticArray() # diagnostic message to be published
diag_pub = None # diagnostic topic

def send(module, operation, params_bc):
    global ser
    try:
        params = bdecode(params_bc)
        data = bencode([module, operation, params])

        try:
            ser.write(data + "\n")
            #print "serial sended:" + data + "\n"
        except Exception, e:
            #print "Service call failed: %s"%e
            io_board_stat = DiagnosticStatus(name = DIAGNOSTIC_SERIAL_COMM_NAME, level = ERROR_LEVEL, 
                                             message = 'serial write exception', hardware_id = DIAGNOSTIC_IOBOARD_ID)
            io_board_stat.values = [KeyValue(key = 'method', value = 'send'), KeyValue(key = 'port', value = SERIAL_PORT)]
            diagMsg.status = [io_board_stat] #, eth_stat ]
            diag_pub.publish(diagMsg)
    except Exception, e:
        #print "Service call failed: %s"%e
        io_board_stat = DiagnosticStatus(name = DIAGNOSTIC_SERIAL_COMM_NAME, level = ERROR_LEVEL, 
                                         message = 'binary decode exception', hardware_id = DIAGNOSTIC_IOBOARD_ID)
        io_board_stat.values = [KeyValue(key = 'method', value = 'send'), KeyValue(key = 'literal', value = params_bc)]
        diagMsg.status = [io_board_stat] #, eth_stat ]
        diag_pub.publish(diagMsg)

    
    
def newMCC(mcc):
    send(mcc.pid, mcc.opcode, mcc.params)
    #print 'New MCC: [' + str(req.mcc.pid) + ", " + str(req.mcc.opcode) + ", " + str(req.mcc.params) + "]."

def connectSerial():
    global ser

    reconnect_rate = rospy.Rate(SERIAL_RECONNECT_TIME)
    while (True):
        try:
            if(ser == None):
                # configure the serial connections (the parameters differs on the device you are connecting to)
                ser = serial.Serial(
                    port = SERIAL_PORT,
                    timeout = SERIAL_TIME_OUT, #None#,
                    baudrate = SERIAL_BAUD
                    #parity=serial.PARITY_NONE,
                    #stopbits=serial.STOPBITS_ONE,
                    #bytesize=serial.EIGHTBITS
                    )
                
                #ser.isOpen()
                #print "Connected!!"
                readIn = '' 
                io_board_stat = DiagnosticStatus(name = DIAGNOSTIC_SERIAL_COMM_NAME, level = OK_LEVEL, 
                                                    message = 'serial_found', hardware_id = DIAGNOSTIC_IOBOARD_ID)
                io_board_stat.values = [KeyValue(key = 'port', value = SERIAL_PORT)]
                diagMsg.status = [io_board_stat] #, eth_stat ]
                diag_pub.publish(diagMsg)
                break;
            else:
                ser.close()
                ser = None
                io_board_stat = DiagnosticStatus(name = DIAGNOSTIC_SERIAL_COMM_NAME, level = ERROR_LEVEL, 
                                                    message = 'serial_io_error', hardware_id = DIAGNOSTIC_IOBOARD_ID)
                io_board_stat.values = [KeyValue(key = 'method', value = 'connectSerial')]                 
                diagMsg.status = [io_board_stat] #, eth_stat ]
                diag_pub.publish(diagMsg)

        except serial.SerialException as e:
            #print e
            io_board_stat = DiagnosticStatus(name = DIAGNOSTIC_SERIAL_COMM_NAME, level = ERROR_LEVEL, 
                                                message = 'serial_not_found', hardware_id = DIAGNOSTIC_IOBOARD_ID)
            io_board_stat.values = [KeyValue(key = 'method', value = 'connectSerial'), KeyValue(key = 'port', value = SERIAL_PORT)]
            diagMsg.status = [io_board_stat] #, eth_stat ]
            diag_pub.publish(diagMsg)
            reconnect_rate.sleep()

def proccess(bc_msg):
    global mcc_pub, diag_pub, diagMsg, imu_freq, updater
    try:
        decode = bdecode(bc_msg)
        #print "llego: " + str(decode[2])
        pid = decode[0]
        opcode = decode[1]
        l_params = decode[2]
        mcc = MCC(pid, opcode, bencode(l_params))
        mcc_pub.publish(mcc)

        #print "envio" 
    except BTFailure, e:
        io_board_stat = DiagnosticStatus(name = DIAGNOSTIC_SERIAL_COMM_NAME, level = ERROR_LEVEL, 
                                    message = 'error decoding binary code', hardware_id = DIAGNOSTIC_IOBOARD_ID)
        io_board_stat.values = [KeyValue(key = 'method', value = 'proccess'), KeyValue(key = 'literal', value = bc_msg)]
         
        diagMsg.status = [io_board_stat] #, eth_stat ]
        diag_pub.publish(diagMsg)
    imu_freq.tick(rospy.Time.now())
    updater.update()

def readSerial():
    global ser, readIn, diag_pub, diagMsg
    newData = ''
    # If you want to wait until data arrives, just do a read(1) with timeout None
    try:
        newData = ser.readline()
        # Limpiar datos viejos 
        readIn = newData
        # FIX solo me quedo con la ultima linea leida
        # readIn = newData
        #readIn += ser.read(1)
        first, sep, readIn = readIn.partition("\n")
        #print "Buffer: " + readIn

        if (sep!= ''):
            proccess(first)
        #descarta si no tiene EOL   
        #else:
        #    readIn = first # no new line found
    except Exception, e:
        #print "Service call failed: %s"%e
        io_board_stat = DiagnosticStatus(name = DIAGNOSTIC_SERIAL_COMM_NAME, level = ERROR_LEVEL, 
                                         message = 'read exception', hardware_id = DIAGNOSTIC_IOBOARD_ID)
        io_board_stat.values = [KeyValue(key = 'method', value = 'readSerial'), KeyValue(key = 'port', value = SERIAL_PORT)]
        diagMsg.status = [io_board_stat] #, eth_stat ]
        diag_pub.publish(diagMsg)
        
        # Try to reconnect serial comunication
        connectSerial()

if __name__ == '__main__':
    rospy.init_node('serial_mcc')
    diag_pub = rospy.Publisher('/diagnostics', DiagnosticArray, queue_size=10)
    mcc_pub = rospy.Publisher('/dm3/mcc2pc', MCC, queue_size=10)
    #rospy.Service('send_mcc', SendMCC, newMCC)
    rospy.Subscriber('/dm3/mcc2ioboard', MCC, newMCC)

    connectSerial()

    # updater se usa para enviar estadisticas del topico mcc2pc al topico diadnostics
    updater = diagnostic_updater.Updater()
    updater.setHardwareID(DIAGNOSTIC_IOBOARD_ID)
    freq_bounds = {'min':5, 'max':20} # If you update these values, the
    imu_freq = diagnostic_updater.TopicDiagnostic("/dm3/mcc2pc", updater,
        diagnostic_updater.FrequencyStatusParam(freq_bounds, 0.1, 10),  
        diagnostic_updater.TimeStampStatusParam(min_acceptable = -1, max_acceptable = 5))
    updater.force_update()

    while not rospy.is_shutdown():
        readSerial()
        