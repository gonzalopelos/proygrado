#include "ros/ros.h"
#include "tcp_comm/mcc2ioboard.h"
#include <strings.h>


bool sendmcc(tcp_comm::mcc2ioboard::Request  &req,
         tcp_comm::mcc2ioboard::Response &res)
{
//   res.sum = req.a + req.b;
//   ROS_INFO("request: x=%ld, y=%ld", (long int)req.a, (long int)req.b);
//   ROS_INFO("sending back response: [%ld]", (long int)res.sum);
    std::string data = req.request.params;
    res.result = data + "- from server";    
    return true;
}



int main(int argc, char **argv)
{
  ros::init(argc, argv, "pc2ioboard_server");
  ros::NodeHandle n;

  ros::ServiceServer service = n.advertiseService("mcc2ioboard", sendmcc);
  ROS_INFO("Ready to send mcc messages");
  ros::spin();

  return 0;
}