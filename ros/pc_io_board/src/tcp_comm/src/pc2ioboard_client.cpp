#include "ros/ros.h"
#include "tcp_comm/mcc2ioboard.h"
#include <cstdlib>
#include <strings.h>


int main(int argc, char **argv)
{
  ros::init(argc, argv, "pc2ioboard_client");
  if (argc != 4)
  {
    ROS_INFO("usage: pc2ioboard_client pid opcode params");
    return 1;
  }

  ros::NodeHandle n;
  ros::ServiceClient client = n.serviceClient<tcp_comm::mcc2ioboard>("mcc2ioboard");
  tcp_comm::mcc2ioboard srv;
  srv.request.request.pid = atoll(argv[1]);
  srv.request.request.opcode = atoll(argv[2]);
  srv.request.request.params = argv[3];
  if (client.call(srv))
  {
    ROS_INFO("Mcc2ioboard response: %s", srv.response.result.c_str());
  }
  else
  {
    ROS_ERROR("Failed to call service mcc2ioboard");
    return 1;
  }

  return 0;
}