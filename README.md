***DISCLAIMER***

***DO NOT USE IN PRODUCTION!!!*** 


Dependencies (except obviously ROS):
```console
sudo apt install librdkafka-dev
```
If you want to use zstd compression one needs to compile librdkafka from source (on ubuntu 20.04).

Very basic bridgin from ros<->kafka.
Basically two template headers based on the librdkafka tutorial.
Publishes serialized rosmsg on kafka topic assumes stamped message.
Or consumes from kafka topic to publish on ros topic.
See ScanProducer.cpp for the most simple example for producing to kafka
and RosClusterConsumer reads a MarkerArray from a kafka topic.
Custom (de-)serialization/partitioning is possible by passing a functor to the constructor.
