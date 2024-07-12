#include <ros/ros.h>
#include <ros_kafka/ros_kafka_message_producer.h>
#include <std_msgs/String.h>

#include <ros/callback_queue.h>

int main(int argc, char **argv)
{
  ros::init(argc, argv, "producer");

  ros::NodeHandle n("~");
  RosKafkaMessageProducer<std_msgs::String> prod(n, "chatter");

  while(ros::ok())
  {
      prod.poll();
      //ros::spinOnce();
      // mimic spin behaviour according to wiki..
      ros::getGlobalCallbackQueue()->callAvailable(ros::WallDuration(0.1));
  }

  return 0;
}
