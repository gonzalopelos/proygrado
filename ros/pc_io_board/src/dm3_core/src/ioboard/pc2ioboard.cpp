#include "ros/ros.h"
// #include "dm3_core/ioboard/mcc2ioboard.h"
#include <strings.h>
#include "../../include/dm3_core/Tcp_Controller.h"
#include "../../include/dm3_core/Mcc_Controller.h"
// #include <dm3_core/MCC.h>
#include "dm3_core/SendMCC.h"

Tcp_Controller *tcp_controller_instance;
ros::Publisher mcc_publisher;
ros::Subscriber mcc_subscriber;
Mcc_Controller mcc_instance;
OpcodeCallback opcode_callbacks[1];
int pid;

// bool sendmcc(tcp_comm::mcc2ioboard::Request &req,
//              tcp_comm::mcc2ioboard::Response &res)
// {
//   std::string data = req.request.params;
//   char* mcc_message = mcc_instance.create_mcc_message(req.request.pid, req.request.opcode, &data[0u]);
//   if(strlen(mcc_message) > 3){
//     // tcp_controller_instance->send_all((char*)"li0ei1e4:pinge\n", 15);
//     tcp_controller_instance->send_all(mcc_message, strlen(mcc_message));
//   }
//   res.result = mcc_message;
//   return true;
// }

void mcc2ioboard_callback(const dm3_core::MCC::ConstPtr& msg)
{
  ROS_INFO("I heard: [%d] [%d] [%s]", msg->pid, msg->opcode, msg->params.c_str());
  // if(msg->pid == 1 && msg->opcode == 2){
  //   for(int index = 0; index < 60; index++){
  //     char * mcc_message = mcc_instance.create_mcc_message(msg->pid, msg->opcode, msg->params.c_str());
  //     if(strlen(mcc_message) > 3){
  //       // tcp_controller_instance->send_all((char*)"li0ei1e4:pinge\n", 15);
  //       tcp_controller_instance->send_all(mcc_message, strlen(mcc_message));
  //     }
  //     sleep(1);
  //   }
  // }
  // else{
    char * mcc_message = mcc_instance.create_mcc_message(msg->pid, msg->opcode, msg->params.c_str());
    if(strlen(mcc_message) > 3){
      // tcp_controller_instance->send_all((char*)"li0ei1e4:pinge\n", 15);
      tcp_controller_instance->send_all(mcc_message, strlen(mcc_message));
    }
  // }
}

void process_incomming_from_ioboard(){
  char data[1];
  int data_length = 0;
  
  memset(data, 0, 1);
  data_length = tcp_controller_instance->receive(data, 1);
  if(data_length > 0) {
    mcc_instance.process_incomming(data[0]);
  }
}

int mcc_from_ioboard_callback(unsigned int  pid, unsigned int opcode, char* bc_params){
  dm3_core::MCC mcc_message;
  mcc_message.pid = pid;
  mcc_message.opcode = opcode;
  mcc_message.params = bc_params;
  mcc_publisher.publish(mcc_message);
  return 1;
}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "pc2ioboard_server");
  ros::NodeHandle n;

  tcp_controller_instance = Tcp_Controller::get_instance();

  mcc_publisher = n.advertise<dm3_core::MCC>("/dm3/mcc2pc", 10);
  mcc_subscriber = n.subscribe("/dm3/mcc2ioboard",10, mcc2ioboard_callback);
//   ros::ServiceServer service = n.advertiseService("mcc2ioboard", sendmcc);
  opcode_callbacks[0] = &mcc_from_ioboard_callback;
  pid = mcc_instance.register_opcode_callbacks(opcode_callbacks,1);
  ROS_INFO("Ready to send mcc messages");
  // ROS_INFO("Send ping message to ioboard...\n");
  
  

  while (ros::ok())
  {
    process_incomming_from_ioboard();
    ros::spinOnce();
  }

  return 0;
}