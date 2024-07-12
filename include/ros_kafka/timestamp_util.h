#ifndef TIMESTAMP_UTIL_
#define TIMESTAMP_UTIL_

// see https://en.cppreference.com/w/cpp/types/void_t
// primary template handles types that have no nested ::type member:
template< class, class = void >
struct is_stamped_msg : std::false_type { };
 
// specialization recognizes types that do have a nested ::type member:
template< class msgT >
struct is_stamped_msg<msgT, std::void_t<decltype(msgT::header)>> : std::true_type { };


/// use traits to use timestamp if present else now 
template<typename msgT,
typename std::enable_if_t< is_stamped_msg<msgT>::value >* = nullptr >
        ros::Time getTimestampOrNow(msgT msg) { return msg.header.stamp; } 

template<typename msgT,
typename std::enable_if_t< !is_stamped_msg<msgT>::value >* = nullptr >
    ros::Time getTimestampOrNow(msgT msg) { return ros::Time::now(); } 

#endif
