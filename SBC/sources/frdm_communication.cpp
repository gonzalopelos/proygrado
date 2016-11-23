//
// Created by proygrado on 7/24/16.
//
#include <fcntl.h>
#include <stdio.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <zconf.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <sys/termios.h>
#include "frdm_communication.h"

const char* frdm_devicePath = "/dev/ttyACM0";

// Hold the original termios attributes so we can reset them
//static struct termios gOriginalTTYAttrs;

// Given the path to a serial device, open the device and configure it.
// Return the file descriptor associated with the device.
//static int openSerialPort(const char *bsdPath)
//{
//    int             fileDescriptor = -1;
//    int             handshake;
//    struct termios  options;
//
//    // Open the serial port read/write, with no controlling terminal, and don't wait for a connection.
//    // The O_NONBLOCK flag also causes subsequent I/O on the device to be non-blocking.
//    // See open(2) <x-man-page://2/open> for details.
//
//    fileDescriptor = open(bsdPath, O_RDWR | O_NOCTTY | O_NONBLOCK);
//    if (fileDescriptor == -1) {
//        printf("Error opening serial port %s - %s(%d).\n",
//               bsdPath, strerror(errno), errno);
//        goto error;
//        /*if (fileDescriptor != -1) {
//            close(fileDescriptor);
//        }
//        return -1;*/
//    }
//
//    // Note that open() follows POSIX semantics: multiple open() calls to the same file will succeed
//    // unless the TIOCEXCL ioctl is issued. This will prevent additional opens except by root-owned
//    // processes.
//    // See tty(4) <x-man-page//4/tty> and ioctl(2) <x-man-page//2/ioctl> for details.
//
//    if (ioctl(fileDescriptor, TIOCEXCL) == -1) {
//        printf("Error setting TIOCEXCL on %s - %s(%d).\n",
//               bsdPath, strerror(errno), errno);
//        goto error;
//        /*if (fileDescriptor != -1) {
//            close(fileDescriptor);
//        }
//        return -1;*/
//    }
//
//    // Now that the device is open, clear the O_NONBLOCK flag so subsequent I/O will block.
//    // See fcntl(2) <x-man-page//2/fcntl> for details.
//
//    if (fcntl(fileDescriptor, F_SETFL, 0) == -1) {
//        printf("Error clearing O_NONBLOCK %s - %s(%d).\n",
//               bsdPath, strerror(errno), errno);
//        goto error;
//        /*if (fileDescriptor != -1) {
//            close(fileDescriptor);
//        }
//        return -1;*/
//    }
//
//
//
//    // Get the current options and save them so we can restore the default settings later.
//    if (tcgetattr(fileDescriptor, &gOriginalTTYAttrs) == -1) {
//        printf("Error getting tty attributes %s - %s(%d).\n",
//               bsdPath, strerror(errno), errno);
//
//        goto error;
//        /*if (fileDescriptor != -1) {
//            close(fileDescriptor);
//        }
//        return -1;*/
//    }
//
//    // The serial port attributes such as timeouts and baud rate are set by modifying the termios
//    // structure and then calling tcsetattr() to cause the changes to take effect. Note that the
//    // changes will not become effective without the tcsetattr() call.
//    // See tcsetattr(4) <x-man-page://4/tcsetattr> for details.
//
//    options = gOriginalTTYAttrs;
//
//    // Print the current input and output baud rates.
//    // See tcsetattr(4) <x-man-page://4/tcsetattr> for details.
//
//    printf("Current input baud rate is %d\n", (int) cfgetispeed(&options));
//    printf("Current output baud rate is %d\n", (int) cfgetospeed(&options));
//
//    // Set raw input (non-canonical) mode, with reads blocking until either a single character
//    // has been received or a one second timeout expires.
//    // See tcsetattr(4) <x-man-page://4/tcsetattr> and termios(4) <x-man-page://4/termios> for details.
//
//    cfmakeraw(&options);
//    options.c_cc[VMIN] = 0;
//    options.c_cc[VTIME] = 10;
//
//    // The baud rate, word length, and handshake options can be set as follows:
//
//    cfsetspeed(&options, B9600);       // Set 9600 baud
//    options.c_cflag |= (CS8     |    // Use 8 bit words
//                        CLOCAL  );
////                        PARENB     |    // Parity enable (even parity if PARODD not also set)
////                        CCTS_OFLOW |    // CTS flow control of output
////                        CRTS_IFLOW);    // RTS flow control of input
//
//    // disable parity generation and 2 stop bits
//    options.c_cflag &= ~(PARENB | CSTOPB);
//
//    // Enable Raw Input
//
//    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
//
//    // Disable Software Flow control
//    options.c_iflag &= ~(IXON | IXOFF | IXANY);
//
//    // Chose raw (not processed) output
//    options.c_oflag &= ~OPOST;
//
//    // Mask the character size bits
//    options.c_oflag &= ~CSIZE;
//
//
//    // The IOSSIOSPEED ioctl can be used to set arbitrary baud rates
//    // other than those specified by POSIX. The driver for the underlying serial hardware
//    // ultimately determines which baud rates can be used. This ioctl sets both the input
//    // and output speed.
///*
//    speed_t speed = 14400; // Set 14400 baud
//    if (ioctl(fileDescriptor, speed) == -1) {
//        printf("Error calling ioctl(..., IOSSIOSPEED, ...) %s - %s(%d).\n",
//               bsdPath, strerror(errno), errno);
//    }
//*/
//    // Print the new input and output baud rates. Note that the IOSSIOSPEED ioctl interacts with the serial driver
//    // directly bypassing the termios struct. This means that the following two calls will not be able to read
//    // the current baud rate if the IOSSIOSPEED ioctl was used but will instead return the speed set by the last call
//    // to cfsetspeed.
//
//    printf("Input baud rate changed to %d\n", (int) cfgetispeed(&options));
//    printf("Output baud rate changed to %d\n", (int) cfgetospeed(&options));
//
//    // Cause the new options to take effect immediately.
//    if (tcsetattr(fileDescriptor, TCSANOW, &options) == -1) {
//        printf("Error setting tty attributes %s - %s(%d).\n",
//               bsdPath, strerror(errno), errno);
//
//        goto error;
//        /*if (fileDescriptor != -1) {
//            close(fileDescriptor);
//        }
//        return -1;*/
//    }
//
//    // To set the modem handshake lines, use the following ioctls.
//    // See tty(4) <x-man-page//4/tty> and ioctl(2) <x-man-page//2/ioctl> for details.
//
//    // Assert Data Terminal Ready (DTR)
//    if (ioctl(fileDescriptor, TIOCSDTR) == -1) {
//        printf("Error asserting DTR %s - %s(%d).\n",
//               bsdPath, strerror(errno), errno);
//    }
//
//    // Clear Data Terminal Ready (DTR)
//    if (ioctl(fileDescriptor, TIOCCDTR) == -1) {
//        printf("Error clearing DTR %s - %s(%d).\n",
//               bsdPath, strerror(errno), errno);
//    }
//
//    // Set the modem lines depending on the bits set in handshake
//    handshake = TIOCM_DTR | TIOCM_RTS | TIOCM_CTS | TIOCM_DSR;
//    if (ioctl(fileDescriptor, TIOCMSET, &handshake) == -1) {
//        printf("Error setting handshake lines %s - %s(%d).\n",
//               bsdPath, strerror(errno), errno);
//    }
//
//    // To read the state of the modem lines, use the following ioctl.
//    // See tty(4) <x-man-page//4/tty> and ioctl(2) <x-man-page//2/ioctl> for details.
//
//    // Store the state of the modem lines in handshake
//    if (ioctl(fileDescriptor, TIOCMGET, &handshake) == -1) {
//        printf("Error getting handshake lines %s - %s(%d).\n",
//               bsdPath, strerror(errno), errno);
//    }
//
//    printf("Handshake lines currently set to %d\n", handshake);
//
////    unsigned long mics = 1UL;
//
//    // Set the receive latency in microseconds. Serial drivers use this value to determine how often to
//    // dequeue characters received by the hardware. Most applications don't need to set this value: if an
//    // app reads lines of characters, the app can't do anything until the line termination character has been
//    // received anyway. The most common applications which are sensitive to read latency are MIDI and IrDA
//    // applications.
//    /*if (ioctl(fileDescriptor, mics) == -1) {
//        // set latency to 1 microsecond
//        printf("Error setting read latency %s - %s(%d).\n",
//               bsdPath, strerror(errno), errno);
//        goto error;
//    }*/
//    // Success
//    return fileDescriptor;
//
//    // Failure path
//    error:
//    {
//        if (fileDescriptor != -1) {
//            close(fileDescriptor);
//        }
//    };
//
//    return -1;
//}
//
//// Given the file descriptor for a serial device, close that device.
//int closeSerialPort(int fileDescriptor)
//{
//    // Block until all written output has been sent from the device.
//    // Note that this call is simply passed on to the serial device driver.
//    // See tcsendbreak(3) <x-man-page://3/tcsendbreak> for details.
//    if (tcdrain(fileDescriptor) == -1) {
//        printf("Error waiting for drain - %s(%d).\n",
//               strerror(errno), errno);
//    }
//
//    // Traditionally it is good practice to reset a serial port back to
//    // the state in which you found it. This is why the original termios struct
//    // was saved.
//    if (tcsetattr(fileDescriptor, TCSANOW, &gOriginalTTYAttrs) == -1) {
//        printf("Error resetting tty attributes - %s(%d).\n",
//               strerror(errno), errno);
//    }
//
//    return close(fileDescriptor);
//}

/*
 * 'open_port()' - Open serial port 1.
 *
 * Returns the file descriptor on success or -1 on error.
 */

int open_frdm_connection(){
    int res;
    if((res = open(frdm_devicePath, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK)) == -1) {
        char errorDesc[200];
        sprintf(errorDesc, "Failed to open port %s:%d:%s\n", frdm_devicePath, errno, strerror(errno));
        perror(errorDesc);
        res = -1;
    }
    else {
        fcntl(res, F_SETFL, 0);
    }
    return res;
    //return openSerialPort(frdm_devicePath);
}

int get_from_fdrm(int file_descriptor, char *dataRead, unsigned int maxSize) {
    int res = -1;
    // Most USB serial port implementations use a max of 4096 for the buffer

    ssize_t data_read_size = read(file_descriptor, dataRead, maxSize);

    /* 0 means no data - errno can indicate 2 or 11 and not be a problem, other values are bad */
    /* -1 means error, check errno */
    /* >0 means you got data at inPtr, which is the start of inBuf and the value indicates how much data you got */
    res = static_cast<int>(data_read_size);

    return res;
}

int send_to_frdm(int file_descriptor, char* data, unsigned int size){
    int res = -1;
    ssize_t data_written_size = write(file_descriptor, data, size);
    res = static_cast<int>(data_written_size);
    return res;
}

int close_frdm_connection(int file_descriptor){
    return close(file_descriptor);
    //return closeSerialPort(file_descriptor);
}
