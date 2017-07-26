#ifndef DM3_CORE_LOGGER_H
#define DM3_CORE_LOGGER_H
#include "ros/ros.h"
#include <cerrno>
class Logger{
    public:
    static void log_error(const char* message){
        char buffer[200];
        ROS_ERROR("%s: %s\n", message, strerror(errno));
    }
};

#endif //DM3_CORE_LOGGER_H