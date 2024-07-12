#ifndef UNASSIGNED_PARTITIONER_H_
#define UNASSIGNED_PARTITIONER_H_

// ROS
#include <ros/ros.h>
#include <ros/serialization.h>
#include <sensor_msgs/Image.h>

// kafka
#include <librdkafka/rdkafkacpp.h>


namespace UAPartitioner{
template <typename msgT>
    int32_t UAPartition(const msgT& msg)
    {
        return RdKafka::Topic::PARTITION_UA;
    }
}

#endif
