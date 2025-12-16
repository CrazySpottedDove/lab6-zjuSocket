#pragma once
#include <cstdint>
namespace zjuSocket {
/**
 * @brief 消息类型枚举
 *
 */
enum class MessageType : uint8_t
{
    /**
     * @brief 断开连接
     * @note data 字段为空
     */
    DISCONNECT,
    /**
     * @brief 请求服务器时间
     * @note data 字段为空
     */
    GET_TIME,
    /**
     * @brief 服务器对时间请求的回应
     * @note data 字段为时间字符串，格式为 "YYYY-MM-DD HH:MM:SS"
     *
     */
    ANSWER_GET_TIME,
    /**
     * @brief 请求客户端名称
     * @note data 字段为空
     *
     */
    GET_NAME,
    /**
     * @brief 服务器对名称请求的回应
     * @note data 字段为客户端名称字符串
     *
     */
    ANSWER_GET_NAME,
    /**
     * @brief 请求客户端列表
     * @note data 字段为空
     */
    GET_CLIENT_LIST,
    /**
     * @brief 服务器对客户端列表请求的回应
     * @note data 字段为以逗号分隔的客户端名称字符串
     */
    ANSWER_GET_CLIENT_LIST,
    /**
     * @brief 发送消息
     * @note data 字段为消息内容字符串
     */
    SEND_MESSAGE,
    /**
     * @brief 服务器对发送消息请求的回应
     * @note data 字段："Ok" 表示发送成功，"Not Found" 表示目标客户端不存在，"Error" 表示发送失败
     */
    ANSWER_SEND_MESSAGE,
    /**
     * @brief 转发消息
     * @note data 字段为消息内容字符串
     */
    REPOST
};

constexpr char ANSWER_SEND_MESSAGE_NOT_FOUND[] = "Not Found";
constexpr char ANSWER_SEND_MESSAGE_OK[]        = "Ok";
constexpr char ANSWER_SEND_MESSAGE_ERROR[]     = "Error";

constexpr int MESSAGE_MAX_LENGTH    = 1024;
constexpr int MESSAGE_HEADER_LENGTH = sizeof(MessageType) / sizeof(uint8_t);

/**
 * @brief 服务器和客户端之间传递的信息结构体
 *
 */
struct Message
{
    MessageType type;
    char        data[MESSAGE_MAX_LENGTH - 1];
};
}   // namespace zjuSocket