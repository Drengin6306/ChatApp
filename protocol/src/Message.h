#pragma once

#include "message_types.h"
#include <string>
#include <memory>
#include <vector>
#include <Poco/JSON/Object.h>

// 消息基类
class Message
{
public:
    Message(MessageType type);
    virtual ~Message() = default;

    // 基本属性
    MessageType getType() const { return type_; }
    uint32_t getId() const { return id_; }
    uint64_t getTimestamp() const { return timestamp_; }

    void setId(uint32_t id) { id_ = id; }

    // 序列化/反序列化
    virtual std::string serialize() const = 0;
    virtual bool deserialize(const std::string &data) = 0;

    // 创建消息的工厂方法
    static std::unique_ptr<Message> createMessage(MessageType type);
    static std::unique_ptr<Message> parseMessage(const std::string &data);

protected:
    MessageType type_;
    uint32_t id_;
    uint64_t timestamp_;

    // JSON 相关
    virtual Poco::JSON::Object::Ptr toJSON() const;
    virtual bool fromJSON(const Poco::JSON::Object::Ptr &json);
};

// 注册请求消息
class RegisterRequest : public Message
{
public:
    RegisterRequest();
    RegisterRequest(const std::string &username, const std::string &password);

    std::string serialize() const override;
    bool deserialize(const std::string &data) override;

    void setUsername(const std::string &username) { username_ = username; }
    void setPassword(const std::string &password) { password_ = password; }

    const std::string &getUsername() const { return username_; }
    const std::string &getPassword() const { return password_; }

protected:
    Poco::JSON::Object::Ptr toJSON() const override;
    bool fromJSON(const Poco::JSON::Object::Ptr &json) override;

private:
    std::string username_;
    std::string password_;
};

// 注册响应消息
class RegisterResponse : public Message
{
public:
    RegisterResponse();
    RegisterResponse(MessageStatus status, const std::string &message = "");

    std::string serialize() const override;
    bool deserialize(const std::string &data) override;

    void setStatus(MessageStatus status) { status_ = status; }
    void setMessage(const std::string &message) { message_ = message; }

    MessageStatus getStatus() const { return status_; }
    const std::string &getMessage() const { return message_; }

protected:
    Poco::JSON::Object::Ptr toJSON() const override;
    bool fromJSON(const Poco::JSON::Object::Ptr &json) override;

private:
    MessageStatus status_;
    std::string message_;
};
// 登录请求消息
class LoginRequest : public Message
{
public:
    LoginRequest();
    LoginRequest(const std::string &account, const std::string &password);

    std::string serialize() const override;
    bool deserialize(const std::string &data) override;

    void setAccount(const std::string &account) { account_ = account; }
    void setPassword(const std::string &password) { password_ = password; }

    const std::string &getAccount() const { return account_; }
    const std::string &getPassword() const { return password_; }

protected:
    Poco::JSON::Object::Ptr toJSON() const override;
    bool fromJSON(const Poco::JSON::Object::Ptr &json) override;

private:
    std::string account_;
    std::string password_;
};

// 登录响应消息
class LoginResponse : public Message
{
public:
    LoginResponse();
    LoginResponse(MessageStatus status, const std::string &account, const std::string &message = "");

    std::string serialize() const override;
    bool deserialize(const std::string &data) override;

    void setStatus(MessageStatus status) { status_ = status; }
    void setMessage(const std::string &message) { message_ = message; }

    MessageStatus getStatus() const { return status_; }
    const std::string &getMessage() const { return message_; }

protected:
    Poco::JSON::Object::Ptr toJSON() const override;
    bool fromJSON(const Poco::JSON::Object::Ptr &json) override;

private:
    MessageStatus status_;
    std::string account_;
    std::string message_;
};

// 聊天消息
class ChatMessage : public Message
{
public:
    ChatMessage();
    ChatMessage(const std::string &sender, const std::string &content);
    ChatMessage(const std::string &sender, const std::string &receiver, const std::string &content);

    std::string serialize() const override;
    bool deserialize(const std::string &data) override;

    void setSender(const std::string &sender) { sender_ = sender; }
    void setReceiver(const std::string &receiver) { receiver_ = receiver; }
    void setContent(const std::string &content) { content_ = content; }

    const std::string &getSender() const { return sender_; }
    const std::string &getReceiver() const { return receiver_; }
    const std::string &getContent() const { return content_; }

    // 辅助方法
    bool isPrivateMessage() const { return !receiver_.empty(); }
    bool isBroadcastMessage() const { return receiver_.empty(); }

protected:
    Poco::JSON::Object::Ptr toJSON() const override;
    bool fromJSON(const Poco::JSON::Object::Ptr &json) override;

private:
    std::string sender_;
    std::string receiver_;
    std::string content_;
};

// 用户列表响应消息
class UserListResponse : public Message
{
public:
    UserListResponse();

    std::string serialize() const override;
    bool deserialize(const std::string &data) override;

    void setUsers(const std::vector<std::string> &users) { users_ = users; }
    void addUser(const std::string &user) { users_.push_back(user); }
    const std::vector<std::string> &getUsers() const { return users_; }

protected:
    Poco::JSON::Object::Ptr toJSON() const override;
    bool fromJSON(const Poco::JSON::Object::Ptr &json) override;

private:
    std::vector<std::string> users_;
};

// 用户状态更新消息
class UserStatusUpdate : public Message
{
public:
    UserStatusUpdate();
    UserStatusUpdate(const std::string &username, const std::string &action);

    std::string serialize() const override;
    bool deserialize(const std::string &data) override;

    void setUsername(const std::string &username) { username_ = username; }
    void setAction(const std::string &action) { action_ = action; }

    const std::string &getUsername() const { return username_; }
    const std::string &getAction() const { return action_; }

protected:
    Poco::JSON::Object::Ptr toJSON() const override;
    bool fromJSON(const Poco::JSON::Object::Ptr &json) override;

private:
    std::string username_;
    std::string action_; // "join", "leave"
};

// 错误消息
class ErrorMessage : public Message
{
public:
    ErrorMessage();
    ErrorMessage(int error_code, const std::string &error_message);

    std::string serialize() const override;
    bool deserialize(const std::string &data) override;

    void setErrorCode(int code) { error_code_ = code; }
    void setErrorMessage(const std::string &message) { error_message_ = message; }

    int getErrorCode() const { return error_code_; }
    const std::string &getErrorMessage() const { return error_message_; }

protected:
    Poco::JSON::Object::Ptr toJSON() const override;
    bool fromJSON(const Poco::JSON::Object::Ptr &json) override;

private:
    int error_code_;
    std::string error_message_;
};
