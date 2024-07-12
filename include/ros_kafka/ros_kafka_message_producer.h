#ifndef KAFKA_MESSAGE_PRODUCER_
#define KAFKA_MESSAGE_PRODUCER_

// ROS
#include <ros/ros.h>
#include <ros/serialization.h>

// kafka
#include <librdkafka/rdkafkacpp.h>

#include <string>
#include <unistd.h>

#include <boost/array.hpp>
#include <memory.h>
#include <chrono>

#include "ros_kafka_common_config.h"
#include "ros_kafka_serialization.h"
#include "unassigned_partitioner.h"

#include "timestamp_util.h"


// Slightly modified dr from examples. 
//class ExampleDeliveryReportCb : public RdKafka::DeliveryReportCb {
//    private:
//        int i = 0;
//        std::chrono::steady_clock::time_point begin;
//    public:
//        void setChrono()
//        {
//         begin = std::chrono::steady_clock::now();
//        }
//        void dr_cb (RdKafka::Message &message) {
//
//            std::chrono::steady_clock::time_point end  = std::chrono::steady_clock::now();
//            /* If message.err() is non-zero the message delivery failed permanently
//             * for the message. */
//            if (message.err())
//            {
//                std::cerr << "% Message delivery failed: " << message.errstr() << std::endl;
//                }
//            else
//            {
//                ROS_INFO("Send frame %d in %ld", i, std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count());
//            }
//        }
//};

template <typename msgT>
class RosKafkaMessageProducer
{
    public:

        RosKafkaMessageProducer(ros::NodeHandle nh, const std::string& rosTopic) :
            RosKafkaMessageProducer(nh,
                             rosTopic,
                             &RosKafkaSerialization::serializeFromRosMsg<msgT>,
                             &UAPartitioner::UAPartition<msgT>)
        {}

        RosKafkaMessageProducer(ros::NodeHandle nh,
                             const std::string& rosTopic,
                             std::function<uint32_t (const msgT&, boost::shared_array<uint8_t>&)> serializer):
            RosKafkaMessageProducer(nh,
                             rosTopic,
                             serializer,
                             &UAPartitioner::UAPartition<msgT>)
        {}


        RosKafkaMessageProducer(ros::NodeHandle nh,
                             const std::string& rosTopic,
                             std::function<uint32_t (const msgT&, boost::shared_array<uint8_t>&)> serializer,
                             std::function<uint32_t (const msgT&)> partitioner)
            :
                nh_(nh),
                rosTopic_(rosTopic),
                serializer_(serializer),
                partitioner_(partitioner)
        {


            if(rosTopic != "")
            {
                /// TODO queue size configurable
                sub_ = nh.subscribe(rosTopic, std::numeric_limits<uint32_t>::max(), &RosKafkaMessageProducer::produceKafkaMsg, this);
            }


            // TODO should this be in commonconfig?
            if(nh.getParam("kafka_topic", kafkaTopic_))
            {   
                ROS_INFO("Producing to %s", kafkaTopic_.c_str());
            }
            else
            {
                ROS_INFO("NO KAFKA TOPIC DEFINED!");
            }
            std::string errstr;
            RosKafkaCommonConfig confHelper(nh_);
            std::unique_ptr<RdKafka::Conf> conf = confHelper.getConfig();
            producer_ = std::unique_ptr<RdKafka::Producer>(RdKafka::Producer::create(conf.get(), errstr));
            conf.reset();

            if (!producer_) {
                std::cerr << "Failed to create producer: " << errstr << std::endl;
            }
        }

        void produceKafkaMsg(const msgT& msg)
        {
            boost::shared_array<uint8_t> buffer;
            // For measuring
            //msgT msg2 = msg;
            //msg2.header.stamp = ros::Time::now();
            //ros::Time ts = msg2.header.stamp;
            //uint32_t serSize = serializer_(msg2, buffer);

            // Production
            uint32_t serSize = serializer_(msg, buffer);

            /// use traits to use timestamp if present else now 
            ros::Time ts = getTimestampOrNow(msg);

            // Convert to ms
            long ms_ts = (long) ts.sec * 1000 + (long) ts.nsec / 1000000;
        bool retry = false;
        do
        {
            RdKafka::ErrorCode err =
                producer_->produce(
                    kafkaTopic_,
                    partitioner_(msg),
                    // TODO possible to transfer ownership?! 
                    RdKafka::Producer::RK_MSG_COPY,
                    buffer.get(), serSize,
                    /* Key */
                    NULL, 0,
                    ms_ts,
                    /* Message headers, if any */
                    NULL,
                    /* Per-message opaque value passed to
                     * delivery report */
                    NULL);

            if (err != RdKafka::ERR_NO_ERROR) {

                std::string errStr(RdKafka::err2str(err));
                ROS_INFO("Failed to produce %s", errStr.c_str());
                if (err == RdKafka::ERR__QUEUE_FULL) {
                    /// TODO SEE https://github.com/edenhill/librdkafka/issues/2247
                    // TODO configurable behaviour.
                    producer_->poll(1000);
                }

                // TODO configurable
                retry = true;
            }
        }
        while(retry);

        /* A producer application should continually serve
         * the delivery report queue by calling poll()
         * at frequent intervals.
         * Either put the poll call in your main loop, or in a
         * dedicated thread, or call it after every produce() call.
         * Just make sure that poll() is still called
         * during periods where you are not producing any messages
         * to make sure previously produced messages have their
         * delivery report callback served (and any other callbacks
         * you register). */
        producer_->poll(0);
        }

        
        void poll()
        {
            producer_->poll(0);
        }

        RosKafkaMessageProducer(RosKafkaMessageProducer&) = delete;
        // Should/could this be kept and default?!
        RosKafkaMessageProducer(RosKafkaMessageProducer&&) = delete;
        RosKafkaMessageProducer& operator=(RosKafkaMessageProducer&) = delete;

        ~RosKafkaMessageProducer()
        {
            // TODO make configurable
            producer_->flush(1 * 1000 /* wait for max 1 seconds */);
        }


    private:

        std::function<uint32_t (const msgT&, boost::shared_array<uint8_t>&)> serializer_;
        std::function<uint32_t (const msgT& )> partitioner_;
        ros::NodeHandle nh_;
        std::string rosTopic_;
        ros::Subscriber sub_;
        std::string kafkaTopic_;
        std::unique_ptr<RdKafka::Producer> producer_;
//        ExampleDeliveryReportCb ex_dr_cb;
//        std::unique_ptr<RdKafka::DeliveryReportCb> msg_opaque_;
};

#endif
