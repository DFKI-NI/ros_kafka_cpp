***DISCLAIMER***

***DO NOT USE IN PRODUCTION!!! ALPHA VERSION*** 


Dependencies (except ROS noetic):
```console
sudo apt install librdkafka-dev
```
If you want to use zstd compression one needs to compile librdkafka from source (on ubuntu 20.04).

Very basic bridging from ros<->kafka.
Basically two template headers based on the librdkafka tutorial.
Allows to publish serialized rosmsg on kafka topic or to consume from kafka topic to publish on ros topic.
Minimal examples can be found in the examples folder.
Custom (de-)serialization/partitioning is possible by passing a functor to the constructor.
