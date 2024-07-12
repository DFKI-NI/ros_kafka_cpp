#ifndef ROS_KAFKA_COMMON_CONFIG_
#define ROS_KAFKA_COMMON_CONFIG_

class RosKafkaCommonConfig{
    public:
        RosKafkaCommonConfig(ros::NodeHandle nh) : 
            nh_(nh),
            conf_(nullptr)
        {
            configFromRosParams();
        }
        

        std::unique_ptr<RdKafka::Conf> getConfig()
        {
            if(!conf_)
                configFromRosParams();

            return std::move(conf_);
        }
    private:
        std::unique_ptr<RdKafka::Conf> conf_;
        ros::NodeHandle nh_;

        void configFromRosParams()
        {
            conf_ = std::unique_ptr<RdKafka::Conf>(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));
            std::string errstr;
            std::string user, password, security_protocol, host;
            if (nh_.getParam("host", host))
            {
                if (conf_->set("bootstrap.servers", host, errstr) !=
                        RdKafka::Conf::CONF_OK) {
                    std::cerr << errstr << std::endl;
                }
            }
            

            /// SECURITY ///
            if (nh_.getParam("security_protocol", security_protocol))
            {
                if (conf_->set("security.protocol", security_protocol, errstr) !=
                        RdKafka::Conf::CONF_OK) {
                    std::cerr << errstr << std::endl;
                }

                /// SASL ///
                if (security_protocol.find("sasl") != std::string::npos) {
                    /// TODO check if supported from builtin features.
                    /// TODO configurable
                    if (conf_->set("sasl.mechanism", "PLAIN", errstr) !=
                            RdKafka::Conf::CONF_OK) {
                        std::cerr << errstr << std::endl;
                    }

                    if (nh_.getParam("user", user))
                    {
                        if (conf_->set("sasl.username", std::getenv(user.c_str()), errstr) !=
                                RdKafka::Conf::CONF_OK) {
                            std::cerr << errstr << std::endl;
                        }
                    }

                    if (nh_.getParam("password", password))
                    {
                        if (conf_->set("sasl.password", std::getenv(user.c_str()), errstr) !=
                                RdKafka::Conf::CONF_OK) {
                            std::cerr << errstr << std::endl;
                        }
                    }
                }

                //// SSL ///
                if (security_protocol.find("ssl") != std::string::npos) {
                    /// TODO check if supported
                    std::string ca_cert, client_cert, client_key, key_pw_var, endpoint_identification;

                    if (nh_.getParam("ca_cert", ca_cert))
                    {
                        if (conf_->set("ssl.ca.location", ca_cert, errstr) !=
                                RdKafka::Conf::CONF_OK) {
                            std::cerr << errstr << std::endl;
                        }
                    }

                    if (nh_.getParam("client_cert", client_cert))
                    {
                        std::cout << " client cert " << client_cert << std::endl;
                        if (conf_->set("ssl.certificate.location", client_cert, errstr) !=
                                RdKafka::Conf::CONF_OK) {
                            std::cerr << errstr << std::endl;
                        }
                    }

                    if (nh_.getParam("client_key", client_key))
                    {
                        std::cout << "client key" << client_key << std::endl;
                        if (conf_->set("ssl.key.location", client_key, errstr) !=
                                RdKafka::Conf::CONF_OK) {
                            std::cerr << errstr << std::endl;
                        }
                    }

                    if (nh_.getParam("key_pw_var", key_pw_var))
                    {
                        if (conf_->set("ssl.key.password", std::getenv(key_pw_var.c_str()), errstr) !=
                                RdKafka::Conf::CONF_OK) {
                            std::cerr << errstr << std::endl;
                        }
                    }

                    if (nh_.getParam("endpoint_identification", endpoint_identification))
                    {
                        if (conf_->set("ssl.endpoint.identification.algorithm", endpoint_identification, errstr) !=
                                RdKafka::Conf::CONF_OK) {
                            std::cerr << errstr << std::endl;
                        }
                    }
                }
            }

            /// read arbitrary config from parameters namespace e.g. loaded from yaml ///
            std::map<std::string,std::string> params;
            if(nh_.getParam("parameters", params))
            {
                for(auto const& it : params)
                {
                    if (conf_->set(it.first, it.second, errstr) !=
                            RdKafka::Conf::CONF_OK) {
                        ROS_ERROR("INVALID PARAM/VALUE: %s", errstr.c_str());
                        std::cerr << errstr << std::endl;
                    }

                }
            }
        }
};

#endif

