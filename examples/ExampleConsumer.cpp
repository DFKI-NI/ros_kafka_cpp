#include <ros/ros.h>
#include <ros/serialization.h>

#include <ros_kafka/ros_kafka_message_consumer.h>
#include <ros_kafka/timestamp_util.h>

#include <std_msgs/String.h>
#include <geometry_msgs/PointStamped.h>


int main(int argc, char **argv)
{
  ros::init(argc, argv, "ConsumerExample");

  ros::NodeHandle nh("~");
  ros::Publisher pub = nh.advertise<std_msgs::String>("chatter", 50);

  RosKafkaMessageConsumer<std_msgs::String, ros::Publisher> consumer(pub, nh, &RosKafkaSerialization::deserializeToRosMsg<std_msgs::String>);

  while(ros::ok())
  {
      consumer.consume();
      ros::spinOnce();
  }

  return 0;
}
