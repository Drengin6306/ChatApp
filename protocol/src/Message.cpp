#include "Message.h"
#include <chrono>
#include <random>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Stringifier.h>
#include <Poco/Dynamic/Var.h>
#include <sstream>

struct IDTimestamp
{
    uint32_t id;
    uint64_t timestamp_sec;
};

class IDGenerator
{
public:
    static IDTimestamp generate()
    {
        static thread_local std::random_device rd;
        static thread_local std::mt19937 gen(rd());
        static thread_local std::uniform_int_distribution<uint32_t> dis(1, UINT32_MAX);

        // 获取当前时间戳
        auto now_sec = std::chrono::duration_cast<std::chrono::seconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count();

        // 生成ID：时间戳(24位) + 随机数(8位)
        uint32_t timestamp_part = (static_cast<uint32_t>(now_sec) & 0xFFFFFF) << 8;
        uint32_t random_part = dis(gen) & 0xFF;
        uint32_t id = timestamp_part | random_part;

        return {id, static_cast<uint64_t>(now_sec)};
    }

    static uint32_t generateID()
    {
        return generate().id;
    }
};

// Message基类实现
Message::Message(MessageType type) : type_(type)
{
    auto [id, timestamp] = IDGenerator::generate();
    id_ = id;
    timestamp_ = timestamp;
}

Poco::JSON::Object::Ptr Message::toJSON() const
{
    Poco::JSON::Object::Ptr json = new Poco::JSON::Object();
    json->set("type", static_cast<int>(type_));
    json->set("id", id_);
    json->set("timestamp", timestamp_);
    return json;
}

bool Message::fromJSON(const Poco::JSON::Object::Ptr &json)
{
    try
    {
        type_ = static_cast<MessageType>(json->getValue<int>("type"));
        id_ = json->getValue<uint32_t>("id");
        timestamp_ = json->getValue<uint64_t>("timestamp");
        return true;
    }
    catch (const std::exception &)
    {
        return false;
    }
}

// RegisterRequest实现
RegisterRequest::RegisterRequest() : Message(MessageType::REGISTER_REQUEST), username_(""), password_("")
{
}

RegisterRequest::RegisterRequest(const std::string &username, const std::string &password)
    : Message(MessageType::REGISTER_REQUEST), username_(username), password_(password)
{
}

std::string RegisterRequest::serialize() const
{
    auto json = toJSON();
    std::ostringstream oss;
    Poco::JSON::Stringifier::stringify(json, oss);
    return oss.str();
}

bool RegisterRequest::deserialize(const std::string &data)
{
    try
    {
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(data);
        Poco::JSON::Object::Ptr json = result.extract<Poco::JSON::Object::Ptr>();
        return fromJSON(json);
    }
    catch (const std::exception &)
    {
        return false;
    }
}

Poco::JSON::Object::Ptr RegisterRequest::toJSON() const
{
    auto json = Message::toJSON();
    json->set("username", username_);
    json->set("password", password_);
    return json;
}

bool RegisterRequest::fromJSON(const Poco::JSON::Object::Ptr &json)
{
    if (!Message::fromJSON(json))
    {
        return false;
    }

    try
    {
        username_ = json->getValue<std::string>("username");
        password_ = json->getValue<std::string>("password");
        return true;
    }
    catch (const std::exception &)
    {
        return false;
    }
}

// RegisterResponse实现
RegisterResponse::RegisterResponse() : Message(MessageType::REGISTER_RESPONSE), status_(MessageStatus::SUCCESS), message_("注册成功")
{
}

RegisterResponse::RegisterResponse(MessageStatus status, const std::string &message)
    : Message(MessageType::REGISTER_RESPONSE), status_(status), message_(message)
{
}

std::string RegisterResponse::serialize() const
{
    auto json = toJSON();
    std::ostringstream oss;
    Poco::JSON::Stringifier::stringify(json, oss);
    return oss.str();
}

bool RegisterResponse::deserialize(const std::string &data)
{
    try
    {
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(data);
        Poco::JSON::Object::Ptr json = result.extract<Poco::JSON::Object::Ptr>();
        return fromJSON(json);
    }
    catch (const std::exception &)
    {
        return false;
    }
}

Poco::JSON::Object::Ptr RegisterResponse::toJSON() const
{
    auto json = Message::toJSON();
    json->set("status", static_cast<int>(status_));
    json->set("message", message_);
    return json;
}

bool RegisterResponse::fromJSON(const Poco::JSON::Object::Ptr &json)
{
    if (!Message::fromJSON(json))
    {
        return false;
    }

    try
    {
        status_ = static_cast<MessageStatus>(json->getValue<int>("status"));
        message_ = json->getValue<std::string>("message");
        return true;
    }
    catch (const std::exception &)
    {
        return false;
    }
}

// LoginRequest实现
LoginRequest::LoginRequest() : Message(MessageType::LOGIN_REQUEST), account_(""), password_("")
{
}

LoginRequest::LoginRequest(const std::string &account, const std::string &password)
    : Message(MessageType::LOGIN_REQUEST), account_(account), password_(password)
{
}

std::string LoginRequest::serialize() const
{
    auto json = toJSON();
    std::ostringstream oss;
    Poco::JSON::Stringifier::stringify(json, oss);
    return oss.str();
}

bool LoginRequest::deserialize(const std::string &data)
{
    try
    {
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(data);
        Poco::JSON::Object::Ptr json = result.extract<Poco::JSON::Object::Ptr>();
        return fromJSON(json);
    }
    catch (const std::exception &)
    {
        return false;
    }
}

Poco::JSON::Object::Ptr LoginRequest::toJSON() const
{
    auto json = Message::toJSON();
    json->set("account", account_);
    json->set("password", password_);
    return json;
}

bool LoginRequest::fromJSON(const Poco::JSON::Object::Ptr &json)
{
    if (!Message::fromJSON(json))
    {
        return false;
    }

    try
    {
        account_ = json->getValue<std::string>("account");
        password_ = json->getValue<std::string>("password");
        return true;
    }
    catch (const std::exception &)
    {
        return false;
    }
}

// LoginResponse实现
LoginResponse::LoginResponse() : Message(MessageType::LOGIN_RESPONSE), status_(MessageStatus::SUCCESS), message_("登录成功")
{
}

LoginResponse::LoginResponse(MessageStatus status, const std::string &account, const std::string &message)
    : Message(MessageType::LOGIN_RESPONSE), status_(status), account_(account), message_(message)
{
}

std::string LoginResponse::serialize() const
{
    auto json = toJSON();
    std::ostringstream oss;
    Poco::JSON::Stringifier::stringify(json, oss);
    return oss.str();
}

bool LoginResponse::deserialize(const std::string &data)
{
    try
    {
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(data);
        Poco::JSON::Object::Ptr json = result.extract<Poco::JSON::Object::Ptr>();
        return fromJSON(json);
    }
    catch (const std::exception &)
    {
        return false;
    }
}

Poco::JSON::Object::Ptr LoginResponse::toJSON() const
{
    auto json = Message::toJSON();
    json->set("status", static_cast<int>(status_));
    json->set("message", message_);
    return json;
}

bool LoginResponse::fromJSON(const Poco::JSON::Object::Ptr &json)
{
    if (!Message::fromJSON(json))
    {
        return false;
    }

    try
    {
        status_ = static_cast<MessageStatus>(json->getValue<int>("status"));
        message_ = json->getValue<std::string>("message");
        return true;
    }
    catch (const std::exception &)
    {
        return false;
    }
}

// ChatMessage实现
ChatMessage::ChatMessage() : Message(MessageType::BROADCAST_MESSAGE), sender_(""), receiver_(""), content_("")
{
}

ChatMessage::ChatMessage(const std::string &sender, const std::string &content)
    : Message(MessageType::BROADCAST_MESSAGE), sender_(sender), receiver_(""), content_(content)
{
}

ChatMessage::ChatMessage(const std::string &sender, const std::string &receiver, const std::string &content)
    : Message(MessageType::PRIVATE_MESSAGE), sender_(sender), receiver_(receiver), content_(content)
{
}

std::string ChatMessage::serialize() const
{
    auto json = toJSON();
    std::ostringstream oss;
    Poco::JSON::Stringifier::stringify(json, oss);
    return oss.str();
}

bool ChatMessage::deserialize(const std::string &data)
{
    try
    {
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(data);
        Poco::JSON::Object::Ptr json = result.extract<Poco::JSON::Object::Ptr>();
        return fromJSON(json);
    }
    catch (const std::exception &)
    {
        return false;
    }
}

Poco::JSON::Object::Ptr ChatMessage::toJSON() const
{
    auto json = Message::toJSON();
    json->set("sender", sender_);
    json->set("content", content_);
    if (!receiver_.empty())
    {
        json->set("receiver", receiver_);
    }
    return json;
}

bool ChatMessage::fromJSON(const Poco::JSON::Object::Ptr &json)
{
    if (!Message::fromJSON(json))
    {
        return false;
    }

    try
    {
        sender_ = json->getValue<std::string>("sender");
        content_ = json->getValue<std::string>("content");

        // receiver是可选字段
        if (json->has("receiver"))
        {
            receiver_ = json->getValue<std::string>("receiver");
        }
        else
        {
            receiver_.clear();
        }

        return true;
    }
    catch (const std::exception &)
    {
        return false;
    }
}

// UserListResponse实现
UserListResponse::UserListResponse() : Message(MessageType::USER_LIST_RESPONSE)
{
}

std::string UserListResponse::serialize() const
{
    auto json = toJSON();
    std::ostringstream oss;
    Poco::JSON::Stringifier::stringify(json, oss);
    return oss.str();
}

bool UserListResponse::deserialize(const std::string &data)
{
    try
    {
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(data);
        Poco::JSON::Object::Ptr json = result.extract<Poco::JSON::Object::Ptr>();
        return fromJSON(json);
    }
    catch (const std::exception &)
    {
        return false;
    }
}

Poco::JSON::Object::Ptr UserListResponse::toJSON() const
{
    auto json = Message::toJSON();
    Poco::JSON::Array::Ptr usersArray = new Poco::JSON::Array;
    for (const auto &user : users_)
    {
        usersArray->add(user);
    }
    json->set("users", usersArray);
    return json;
}

bool UserListResponse::fromJSON(const Poco::JSON::Object::Ptr &json)
{
    if (!Message::fromJSON(json))
    {
        return false;
    }

    try
    {
        Poco::JSON::Array::Ptr usersArray = json->getArray("users");
        users_.clear();
        for (size_t i = 0; i < usersArray->size(); ++i)
        {
            users_.push_back(usersArray->getElement<std::string>(i));
        }
        return true;
    }
    catch (const std::exception &)
    {
        return false;
    }
}

// UserStatusUpdate实现
UserStatusUpdate::UserStatusUpdate() : Message(MessageType::USER_STATUS_UPDATE), action_("")
{
}

UserStatusUpdate::UserStatusUpdate(const std::string &action)
    : Message(MessageType::USER_STATUS_UPDATE), action_(action)
{
}

std::string UserStatusUpdate::serialize() const
{
    auto json = toJSON();
    std::ostringstream oss;
    Poco::JSON::Stringifier::stringify(json, oss);
    return oss.str();
}

bool UserStatusUpdate::deserialize(const std::string &data)
{
    try
    {
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(data);
        Poco::JSON::Object::Ptr json = result.extract<Poco::JSON::Object::Ptr>();
        return fromJSON(json);
    }
    catch (const std::exception &)
    {
        return false;
    }
}

Poco::JSON::Object::Ptr UserStatusUpdate::toJSON() const
{
    auto json = Message::toJSON();
    json->set("action", action_);
    return json;
}

bool UserStatusUpdate::fromJSON(const Poco::JSON::Object::Ptr &json)
{
    if (!Message::fromJSON(json))
    {
        return false;
    }

    try
    {
        action_ = json->getValue<std::string>("action");
        return true;
    }
    catch (const std::exception &)
    {
        return false;
    }
}

// ErrorMessage实现
ErrorMessage::ErrorMessage() : Message(MessageType::ERROR_MESSAGE), error_code_(0), error_message_("")
{
}

ErrorMessage::ErrorMessage(int error_code, const std::string &error_message)
    : Message(MessageType::ERROR_MESSAGE), error_code_(error_code), error_message_(error_message)
{
}

std::string ErrorMessage::serialize() const
{
    auto json = toJSON();
    std::ostringstream oss;
    Poco::JSON::Stringifier::stringify(json, oss);
    return oss.str();
}

bool ErrorMessage::deserialize(const std::string &data)
{
    try
    {
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(data);
        Poco::JSON::Object::Ptr json = result.extract<Poco::JSON::Object::Ptr>();
        return fromJSON(json);
    }
    catch (const std::exception &)
    {
        return false;
    }
}

Poco::JSON::Object::Ptr ErrorMessage::toJSON() const
{
    auto json = Message::toJSON();
    json->set("error_code", error_code_);
    json->set("error_message", error_message_);
    return json;
}

bool ErrorMessage::fromJSON(const Poco::JSON::Object::Ptr &json)
{
    if (!Message::fromJSON(json))
    {
        return false;
    }

    try
    {
        error_code_ = json->getValue<int>("error_code");
        error_message_ = json->getValue<std::string>("error_message");
        return true;
    }
    catch (const std::exception &)
    {
        return false;
    }
}

// 工厂方法实现
std::unique_ptr<Message> Message::createMessage(MessageType type)
{
    switch (type)
    {
    case MessageType::REGISTER_REQUEST:
        return std::make_unique<RegisterRequest>();
    case MessageType::REGISTER_RESPONSE:
        return std::make_unique<RegisterResponse>();
    case MessageType::LOGIN_REQUEST:
        return std::make_unique<LoginRequest>();
    case MessageType::LOGIN_RESPONSE:
        return std::make_unique<LoginResponse>();
    case MessageType::BROADCAST_MESSAGE:
    case MessageType::PRIVATE_MESSAGE:
        return std::make_unique<ChatMessage>();
    case MessageType::USER_LIST_RESPONSE:
        return std::make_unique<UserListResponse>();
    case MessageType::USER_STATUS_UPDATE:
        return std::make_unique<UserStatusUpdate>();
    case MessageType::ERROR_MESSAGE:
        return std::make_unique<ErrorMessage>();
    default:
        return nullptr;
    }
}

std::unique_ptr<Message> Message::parseMessage(const std::string &data)
{
    try
    {
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(data);
        Poco::JSON::Object::Ptr json = result.extract<Poco::JSON::Object::Ptr>();

        int typeInt = json->getValue<int>("type");
        MessageType type = static_cast<MessageType>(typeInt);
        switch (type)
        {
        case MessageType::REGISTER_REQUEST:
        {
            auto message = std::make_unique<RegisterRequest>();
            message->deserialize(data);
            return message;
        }
        break;
        case MessageType::REGISTER_RESPONSE:
        {
            auto message = std::make_unique<RegisterResponse>();
            message->deserialize(data);
            return message;
        }
        break;
        case MessageType::LOGIN_REQUEST:
        {
            auto message = std::make_unique<LoginRequest>();
            message->deserialize(data);
            return message;
        }
        break;
        case MessageType::LOGIN_RESPONSE:
        {
            auto message = std::make_unique<LoginResponse>();
            message->deserialize(data);
            return message;
        }
        break;
        case MessageType::BROADCAST_MESSAGE:
        case MessageType::PRIVATE_MESSAGE:
        {
            auto message = std::make_unique<ChatMessage>();
            message->deserialize(data);
            return message;
        }
        break;
        case MessageType::USER_LIST_RESPONSE:
        {
            auto message = std::make_unique<UserListResponse>();
            message->deserialize(data);
            return message;
        }
        break;
        case MessageType::USER_STATUS_UPDATE:
        {
            auto message = std::make_unique<UserStatusUpdate>();
            message->deserialize(data);
            return message;
        }
        break;
        case MessageType::ERROR_MESSAGE:
        {
            auto message = std::make_unique<ErrorMessage>();
            message->deserialize(data);
            return message;
        }
        break;
        default:
            break;
        }
    }
    catch (const std::exception &)
    {
        // 解析失败
    }

    return nullptr;
}
