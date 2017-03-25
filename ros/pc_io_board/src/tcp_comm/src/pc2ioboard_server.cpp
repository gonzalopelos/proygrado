#include "ros/ros.h"
#include "tcp_comm/mcc2ioboard.h"
#include <strings.h>
#include "../include/tcp_comm/Tcp_Controller.h"

Tcp_Controller *tcp_controller_instance;

bool sendmcc(tcp_comm::mcc2ioboard::Request &req,
             tcp_comm::mcc2ioboard::Response &res)
{
  //   res.sum = req.a + req.b;
  //   ROS_INFO("request: x=%ld, y=%ld", (long int)req.a, (long int)req.b);
  //   ROS_INFO("sending back response: [%ld]", (long int)res.sum);
  std::string data = req.request.params;
  tcp_controller_instance->send_all(&data[0u], data.size() + 1);
  res.result = data + "- from server";
  return true;
}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "pc2ioboard_server");
  ros::NodeHandle n;

  tcp_controller_instance = Tcp_Controller::get_instance();

  ros::ServiceServer service = n.advertiseService("mcc2ioboard", sendmcc);

  ROS_INFO("Ready to send mcc messages");
  ROS_INFO("Send ping message to ioboard...\n");
  tcp_controller_instance->send_all("li0ei1el4:pingee\n", 17);
  char data[256];
  int data_length = 0;
  while (ros::ok())
  {
    data_length = tcp_controller_instance->receive(data, 17);
    if (data_length > 0)
    {
      ROS_INFO("Data recieved from ioboard: %s\n", data);
      if (data_length >= 17)
      {
        tcp_controller_instance->send_all(data, 17);
      }
    }
    ros::spin();
  }

  return 0;
}