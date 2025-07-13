#include "ChatConnection.h"
#include "ConnectionManager.h"
#include "Message.h"
#include "UserManager.h"
#include <Poco/Net/NetException.h>
#include <Poco/Logger.h>
#include <Poco/StreamCopier.h>
#include <Poco/DateTime.h>
#include <Poco/DateTimeFormatter.h>
#include <iostream>
#include <sstream>

ChatConnection::ChatConnection(const Poco::Net::StreamSocket &socket)
    : TCPServerConnection(socket), isConnected_(true), isAuthenticated_(false)
{
    clientAddress_ = socket.peerAddress().toString();

    auto &logger = Poco::Logger::get("ChatConnection");
    logger.information("New connection from: " + clientAddress_);
}

ChatConnection::~ChatConnection()
{
    auto &logger = Poco::Logger::get("ChatConnection");
    logger.information("Connection closed: " + clientAddress_);
}

void ChatConnection::run()
{
    auto &logger = Poco::Logger::get("ChatConnection");
    auto &connectionManager = ConnectionManager::getInstance();

    try
    {
        connectionManager.addConnection(this);

        while (isConnected_)
        {
            auto message = receiveMessage();
            if (!message)
            {
                logger.information("Connection " + clientAddress_ + " closed by client.");
                break;
            }

            // 处理不同类型的消息
            switch (message->getType())
            {
            case MessageType::LOGIN_REQUEST:
                if (isAuthenticated_)
                {
                    logger.warning("Received login request from already authenticated user: " + account_);
                    continue;
                }
                handleLoginRequest(static_cast<LoginRequest &>(*message));
                break;
            case MessageType::REGISTER_REQUEST:
                if (isAuthenticated_)
                {
                    logger.warning("Received register request from already authenticated user: " + account_);
                    continue;
                }
                handleRegisterRequest(static_cast<RegisterRequest &>(*message));
                break;
            case MessageType::BROADCAST_MESSAGE:
            case MessageType::PRIVATE_MESSAGE:
                handleChatMessage(static_cast<ChatMessage &>(*message));
                break;
            case MessageType::USER_STATUS_UPDATE:
                handleUserStatusUpdate(static_cast<UserStatusUpdate &>(*message));
                break;
            default:
                logger.warning("Unknown message type received: " + std::to_string(static_cast<int>(message->getType())));
                break;
            }
        }
    }
    catch (const Poco::Net::NetException &e)
    {
        logger.error("Network error in connection " + clientAddress_ + ": " + e.displayText());
    }
    catch (const std::exception &e)
    {
        logger.error("Error in connection " + clientAddress_ + ": " + e.what());
    }

    // 清理连接
    isConnected_ = false;
    connectionManager.removeConnection(this);
    logger.information("Connection " + clientAddress_ + " closed.");
}

// 接受消息并返回一个 Message 对象
std::unique_ptr<Message> ChatConnection::receiveMessage()
{
    auto &logger = Poco::Logger::get("ChatConnection");
    if (!isConnected_)
    {
        return nullptr;
    }

    try
    {
        // 接收4字节的消息长度
        uint32_t messageLength = 0;
        int bytesRead = 0;

        // 确保读取完整的4字节长度
        while (bytesRead < 4)
        {
            int received = socket().receiveBytes(
                reinterpret_cast<char *>(&messageLength) + bytesRead,
                4 - bytesRead);

            if (received <= 0)
            {
                if (bytesRead == 0 && received == 0)
                {
                    return nullptr;
                }
                throw std::runtime_error("连接中断，无法接收完整的消息长度");
            }

            bytesRead += received;
        }

        messageLength = ntohl(messageLength);

        if (messageLength == 0)
        {
            return nullptr; // 空消息
        }

        if (messageLength > 10 * 1024 * 1024) // 10MB上限
        {
            throw std::runtime_error("消息过大: " + std::to_string(messageLength) + " 字节");
        }

        std::vector<char> buffer(messageLength + 1);
        bytesRead = 0;

        while (bytesRead < messageLength)
        {
            int received = socket().receiveBytes(
                buffer.data() + bytesRead,
                messageLength - bytesRead);

            if (received <= 0)
            {
                throw std::runtime_error("连接中断，无法接收完整的消息");
            }

            bytesRead += received;
        }
        buffer[messageLength] = '\0';
        std::string jsonData(buffer.data(), messageLength);

        logger.debug("接收到JSON (" + std::to_string(messageLength) + " 字节): " + jsonData);
        return Message::parseMessage(jsonData);
    }
    catch (const std::exception &e)
    {
        logger.error("接收消息失败: " + std::string(e.what()));
        isConnected_ = false;
        return nullptr;
    }
}

void ChatConnection::sendMessage(const Message &message)
{
    if (!isConnected_)
        return;

    try
    {
        std::string jsonData = message.serialize();
        auto &logger = Poco::Logger::get("ChatConnection");

        logger.debug("发送JSON (" + std::to_string(jsonData.length()) + " 字节): " + jsonData);

        uint32_t messageLength = htonl(static_cast<uint32_t>(jsonData.length()));

        int sent = socket().sendBytes(&messageLength, sizeof(messageLength));
        if (sent != sizeof(messageLength))
        {
            throw std::runtime_error("发送消息长度失败");
        }

        sent = socket().sendBytes(jsonData.c_str(), jsonData.length());
        if (sent != jsonData.length())
        {
            throw std::runtime_error("发送消息内容不完整");
        }
    }
    catch (const std::exception &e)
    {
        auto &logger = Poco::Logger::get("ChatConnection");
        logger.error("发送消息失败: " + std::string(e.what()));
        isConnected_ = false;
    }
}

std::string ChatConnection::getClientAddress() const
{
    return clientAddress_;
}

void ChatConnection::handleLoginRequest(const LoginRequest &loginRequest)
{
    auto &logger = Poco::Logger::get("ChatConnection");
    auto response = std::make_unique<LoginResponse>();

    account_ = loginRequest.getAccount();
    auto &userManager = UserManager::getInstance();
    if (userManager.authenticateUser(account_, loginRequest.getPassword()))
    {
        if (response)
        {
            response->setStatus(MessageStatus::SUCCESS);
            response->setAccount(account_);
            response->setMessage("登录成功");
        }
        isAuthenticated_ = true;
        logger.information("User " + account_ + " logged in successfully.");
    }
    else
    {
        logger.error("Authentication failed for user " + account_);
        if (response)
        {
            response->setStatus(MessageStatus::FAILED);
            response->setMessage("登录失败");
        }
    }
    // 发送认证结果
    if (response)
    {
        sendMessage(*response);
    }
}

void ChatConnection::handleRegisterRequest(const RegisterRequest &registerRequest)
{
    auto &logger = Poco::Logger::get("ChatConnection");
    auto response = std::make_unique<RegisterResponse>();

    std::string username = registerRequest.getUsername();
    std::string password = registerRequest.getPassword();
    auto &userManager = UserManager::getInstance();
    account_ = userManager.registerUser(username, password);
    if (!account_.empty())
    {
        if (response)
        {
            response->setStatus(MessageStatus::SUCCESS);
            response->setMessage("注册成功，您的账号是: " + account_ + "，请登录你的账号");
        }
        logger.information("User " + account_ + " registered successfully.");
    }
    else
    {
        if (response)
        {
            response->setStatus(MessageStatus::FAILED);
            response->setMessage("注册失败");
        }
    }
    // 发送注册结果
    if (response)
    {
        sendMessage(*response);
    }
}

void ChatConnection::handleChatMessage(const ChatMessage &chatMessage)
{
    auto &logger = Poco::Logger::get("ChatConnection");

    if (chatMessage.isPrivateMessage() && chatMessage.getType() == MessageType::PRIVATE_MESSAGE)
    {
        auto &connectionManager = ConnectionManager::getInstance();
        connectionManager.sendMessageToUser(chatMessage);
        logger.information("Private message from " + chatMessage.getSender() + " to " + chatMessage.getReceiver() + ": " + chatMessage.getContent());
    }
    else if (chatMessage.isBroadcastMessage() && chatMessage.getType() == MessageType::BROADCAST_MESSAGE)
    {
        auto &connectionManager = ConnectionManager::getInstance();
        connectionManager.broadcastMessage(chatMessage, this);
        logger.information("Broadcast message from " + chatMessage.getSender() + "[" + clientAddress_ + "]" + ": " + chatMessage.getContent());
    }
    else
    {
        logger.warning("Received unsupported chat message type from " + clientAddress_);
    }
}

void ChatConnection::handleUserStatusUpdate(const UserStatusUpdate &userStatusUpdate)
{
    auto &logger = Poco::Logger::get("ChatConnection");
    auto &connectionManager = ConnectionManager::getInstance();

    if (userStatusUpdate.getAction() == "leave")
    {
        logger.information("User " + account_ + " has left the chat.");
        isConnected_ = false;
        connectionManager.removeConnection(this);
    }
    else if (userStatusUpdate.getAction() == "logout")
    {
        logger.information("User " + account_ + " has logged out.");
        connectionManager.unauthenticateConnection(this);
        isAuthenticated_ = false;
        account_.clear();
    }
    else
    {
        logger.warning("Received unsupported user status update: " + userStatusUpdate.getAction());
    }
}