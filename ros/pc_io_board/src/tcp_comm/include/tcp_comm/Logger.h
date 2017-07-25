#ifndef TCP_COMM_LOGGER_H
#define TCP_COMM_LOGGER_H
#include "ros/ros.h"
#include <cerrno>
class Logger{
    public:
    static void log_error(const char* message){
        char buffer[200];
        ROS_ERROR("%s: %s\n", message, strerror(errno));
    }
};

#endif //TCP_COMM_LOGGER_H