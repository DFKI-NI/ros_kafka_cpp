#ifndef KAFKA_MESSAGE_CONSUMER_
#define KAFKA_MESSAGE_CONSUMER_

// ROS
#include <ros/ros.h>
#include <ros/serialization.h>

// kafka
#include <librdkafka/rdkafkacpp.h>

// ros kafka bridge
#include "ros_kafka_common_config.h"
#include "ros_kafka_serialization.h"

template <typename msgT, typename publisherT>
class RosKafkaMessageConsumer
{
    public:
        RosKafkaMessageConsumer(
                publisherT pub,
                ros::NodeHandle nh,
                std::function<boost::shared_ptr<msgT>(RdKafka::Message*)> deserializer =
                &RosKafkaSerialization::deserializeToRosMsg<msgT>)
            : 
                nh_(nh),
                deserializer_(deserializer),
                pub_(pub)

    {
        std::string errstr;
        RosKafkaCommonConfig confHelper(nh_);
        std::unique_ptr<RdKafka::Conf> conf = confHelper.getConfig();

        std::string group_id;

        if (nh.getParam("group_id", group_id))
        {
            if (conf->set("group.id",  group_id, errstr) != RdKafka::Conf::CONF_OK) {
                ROS_ERROR("ERROR group id: %s", errstr.c_str());
            }
        }
        else
        {
            /// TODO exceptions
            ROS_ERROR("NO GROUP ID");
        }

        // Create the cosumer
        consumer_ =
            std::unique_ptr<RdKafka::KafkaConsumer>(RdKafka::KafkaConsumer::create(conf.get(), errstr));

        conf.reset();

        if (!consumer_) {
            ROS_ERROR("Failed to create consumer: %s", errstr.c_str());
        }else{
            // TODO should this be in commonconfig?
            if(nh.getParam("kafka_topic", kafkaTopic_))
            {   
                ROS_INFO("Consuming from %s", kafkaTopic_.c_str());
                consumer_->subscribe({kafkaTopic_});
            }
            else
            {
                ROS_INFO("NO KAFKA TOPIC DEFINED!");
            }
        }
       
    }

        void consume(RdKafka::Message* kafka_msg)
        {
            
          if(kafka_msg->err())
          {
              // HANDLE ERRORS BUT THIS MAY JUST BE A LOT "TIMED OUT" Messages...
              return;
          }
          else
          {
              boost::shared_ptr<msgT> ros_msg = deserializer_(kafka_msg);
              pub_.publish(ros_msg);
          }
              delete kafka_msg;
        }

        void consume()
        {
            // TODO This should this be configurable
            RdKafka::Message* msg = consumer_->consume(20);
            if(msg)
                consume(msg);

        }

    private:
        std::unique_ptr<RdKafka::KafkaConsumer> consumer_;
        ros::NodeHandle nh_;
        std::string kafkaTopic_;
        publisherT pub_;
        std::function<boost::shared_ptr<msgT>(RdKafka::Message* kafka_msg)> deserializer_;

};

#endif
