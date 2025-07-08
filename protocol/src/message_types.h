#pragma once

#include <cstdint>

// 消息类型枚举
enum class MessageType : uint8_t
{
    // 认证相关
    REGISTER_REQUEST = 1,
    REGISTER_RESPONSE = 2,
    LOGIN_REQUEST = 3,
    LOGIN_RESPONSE = 4,
    LOGOUT = 5,

    // 消息相关
    BROADCAST_MESSAGE = 10,
    PRIVATE_MESSAGE = 11,
    MESSAGE_ACK = 12,

    // 用户相关
    USER_LIST_REQUEST = 20,
    USER_LIST_RESPONSE = 21,
    USER_STATUS_UPDATE = 22,

    // 系统相关
    HEARTBEAT = 30,
    ERROR = 31
};

// 消息状态
enum class MessageStatus : uint8_t
{
    SUCCESS = 0,
    ERROR = 1,
    TIMEOUT = 2,
    USER_NOT_FOUND = 3,
    USER_ALREADY_EXISTS = 4,
    INVALID_FORMAT = 5,
    UNAUTHORIZED = 6
};