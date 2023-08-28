//
// Created by fzy on 23-8-25.
//

#ifndef TINYNETFLOW_SOMEIPPROTOCOL_H
#define TINYNETFLOW_SOMEIPPROTOCOL_H

#include <cstdint>

/** SOME/IP协议的各种枚举类
 * 参考自 interface/vsomeip/enumeration_types.hpp */
namespace netflow::net::someip {
/**********************         SOME/IP         *************************************/
enum class SomeIpStateType : uint8_t {
    ST_REGISTERED = 0x0,
    ST_DEREGISTERED = 0x1
};

// SIP_RPC_684
enum class SomeIpMessageType : uint8_t {
    MT_REQUEST = 0x00,
    MT_REQUEST_NO_RETURN = 0x01,
    MT_NOTIFICATION = 0x02,
    MT_REQUEST_ACK = 0x40,
    MT_REQUEST_NO_RETURN_ACK = 0x41,
    MT_NOTIFICATION_ACK = 0x42,
    MT_RESPONSE = 0x80,
    MT_ERROR = 0x81,
    MT_RESPONSE_ACK = 0xC0,
    MT_ERROR_ACK = 0xC1,
    MT_UNKNOWN = 0xFF
};

// SIP_RPC_371
enum class SomeIpReturnCode : uint8_t {
    E_OK = 0x00,
    E_NOT_OK = 0x01,
    E_UNKNOWN_SERVICE = 0x02,
    E_UNKNOWN_METHOD = 0x03,
    E_NOT_READY = 0x04,
    E_NOT_REACHABLE = 0x05,
    E_TIMEOUT = 0x06,
    E_WRONG_PROTOCOL_VERSION = 0x07,
    E_WRONG_INTERFACE_VERSION = 0x08,
    E_MALFORMED_MESSAGE = 0x09,
    E_WRONG_MESSAGE_TYPE = 0x0A,
    E_UNKNOWN = 0xFF
};

enum class SomeIpRoutingState : uint8_t {
    RS_RUNNING = 0x00,
    RS_SUSPENDED = 0x01,
    RS_RESUMED = 0x02,
    RS_SHUTDOWN = 0x03,
    RS_DIAGNOSIS = 0x04,
    RS_UNKNOWN = 0xFF
};

enum class SomeIpOfferType : uint8_t {
    OT_LOCAL = 0x00,
    OT_REMOTE = 0x01,
    OT_ALL = 0x02,
};

enum class SomeIpEventType : uint8_t {
    ET_EVENT = 0x00,
    ET_SELECTIVE_EVENT = 0x01,
    ET_FIELD = 0x02,
    ET_UNKNOWN = 0xFF
};

enum class SomeIpSecurityMode : uint8_t {
    SM_OFF = 0x00,
    SM_ON = 0x01,
    SM_AUDIT = 0x02
};

enum class SomeIpSecurityUpdateState : uint8_t {
    SU_SUCCESS = 0x00,
    SU_NOT_ALLOWED = 0x01,
    SU_UNKNOWN_USER_ID = 0x02,
    SU_INVALID_FORMAT = 0x03
};

enum class SomeIpReliabilityType : uint8_t {
    RT_RELIABLE = 0x01,
    RT_UNRELIABLE = 0x02,
    RT_BOTH = 0x3, // RT_RELIABLE | RT_UNRELIABLE
    RT_UNKNOWN = 0xFF
};

enum class SomeIpAvailabilityState : uint8_t {
    AS_UNAVAILABLE = 0x00, // unseen
    AS_OFFERED = 0x01,     // seen, but not requested/not yet usable
    AS_AVAILABLE = 0x02,   // seen and usable
    AS_UNKNOWN = 0xFF
};
/**********************  end of SOME/IP         *************************************/

/**********************     SOME/IP-SD          *************************************/

enum class SomeIpSdOptionType : uint8_t {
    CONFIGURATION = 0x1,
    LOAD_BALANCING = 0x2,
    PROTECTION = 0x3,
    IP4_ENDPOINT = 0x4,
    IP6_ENDPOINT = 0x6,
    IP4_MULTICAST = 0x14,
    IP6_MULTICAST = 0x16,
    SELECTIVE = 0x20,
    UNKNOWN = 0xFF
};

enum class SomeIpSdEntryType: uint8_t {
    FIND_SERVICE = 0x00,
    OFFER_SERVICE = 0x01,
    STOP_OFFER_SERVICE = 0x01,
    REQUEST_SERVICE = 0x2,
    FIND_EVENT_GROUP = 0x4,
    PUBLISH_EVENTGROUP = 0x5,
    STOP_PUBLISH_EVENTGROUP = 0x5,
    SUBSCRIBE_EVENTGROUP = 0x06,
    STOP_SUBSCRIBE_EVENTGROUP = 0x06,
    SUBSCRIBE_EVENTGROUP_ACK = 0x07,
    STOP_SUBSCRIBE_EVENTGROUP_ACK = 0x07,
    UNKNOWN = 0xFF
};

enum class SomeIpSdL4Protocol : uint8_t {
    TCP = 0x06,
    UDP = 0x11,
    UNKNOWN = 0xFF
};
/**********************  end of SOME/IP-SD     *************************************/
}  // namespace netflow::net::someip


#endif //TINYNETFLOW_SOMEIPPROTOCOL_H
