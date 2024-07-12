#ifndef ROS_KAFKA_SERIALIZATION_H_
#define ROS_KAFKA_SERIALIZATION_H_

// ROS
#include <ros/ros.h>
#include <ros/serialization.h>
#include <sensor_msgs/Image.h>

// kafka
#include <librdkafka/rdkafkacpp.h>


namespace RosKafkaSerialization{
template <typename msgT>
boost::shared_ptr<msgT> deserializeToRosMsg(RdKafka::Message* kafka_msg)
{
    boost::shared_ptr<msgT> ros_msg = boost::shared_ptr<msgT>(new msgT());
    kafka_msg->payload();
    ros::serialization::IStream stream(static_cast<uint8_t *>(kafka_msg->payload()), kafka_msg->len());
    ros::serialization::deserialize(stream, *ros_msg);
    return ros_msg;
}


template <>
boost::shared_ptr<sensor_msgs::Image> deserializeToRosMsg(RdKafka::Message* kafka_msg)
{
    std::cout << "des img" << std::endl;
    boost::shared_ptr<sensor_msgs::Image> ros_msg = boost::shared_ptr<sensor_msgs::Image>(new sensor_msgs::Image());
    kafka_msg->payload();
    ros::serialization::IStream stream(static_cast<uint8_t *>(kafka_msg->payload()), kafka_msg->len());
    ros::serialization::deserialize(stream, *ros_msg);
    std::cout << ros_msg->data.size() << std::endl;
    return ros_msg;
}

template <typename msgT>
uint32_t serializeFromRosMsg(const msgT& msg, boost::shared_array<uint8_t>& buffer)
{
    uint32_t serSize = ros::serialization::serializationLength(msg);
    // Maybe should be shared_ptr with array but we are using ros..
    buffer =  boost::shared_array<uint8_t>(new uint8_t[serSize]);
    ros::serialization::OStream stream(buffer.get(), serSize);
    ros::serialization::serialize(stream, msg);
    return serSize;
}

template <>
uint32_t serializeFromRosMsg(const sensor_msgs::Image& msg, boost::shared_array<uint8_t>& buffer)
{
    uint32_t serSize = ros::serialization::serializationLength(msg);
    // Maybe should be shared_ptr with array but we are using ros..
    buffer =  boost::shared_array<uint8_t>(new uint8_t[serSize]);
    ros::serialization::OStream stream(buffer.get(), serSize);
    ros::serialization::serialize(stream, msg);
    return serSize;
}
}

#endif
