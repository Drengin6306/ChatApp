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

        // 用户认证
        while (!isAuthenticated_)
        {
            // 处理登录请求
            char buffer[1024];
            int bytesReceived = socket().receiveBytes(buffer, sizeof(buffer) - 1);
            if (bytesReceived <= 0)
            {
                break; // 连接已关闭
            }

            buffer[bytesReceived] = '\0';
            std::string requestData(buffer);
            auto message = Message::parseMessage(requestData);
            if (message)
            {
                if (message->getType() == MessageType::LOGIN_REQUEST)
                {
                    handleLoginRequest(static_cast<LoginRequest &>(*message));
                }
                else if (message->getType() == MessageType::REGISTER_REQUEST)
                {
                    handleRegisterRequest(static_cast<RegisterRequest &>(*message));
                }
                else
                {
                    logger.error("Received unexpected message type: " + std::to_string(static_cast<int>(message->getType())));
                }
            }
            else
            {
                logger.error("Failed to parse message from " + clientAddress_);
            }
        }

        // 读取客户端消息
        char buffer[1024];
        std::string messageBuffer;

        while (isConnected_)
        {
            int bytesReceived = socket().receiveBytes(buffer, sizeof(buffer) - 1);
            if (bytesReceived <= 0)
            {
                break; // 连接已关闭
            }

            buffer[bytesReceived] = '\0';
            messageBuffer += buffer;

            // 处理完整的消息行
            size_t pos;
            while ((pos = messageBuffer.find('\n')) != std::string::npos)
            {
                std::string line = messageBuffer.substr(0, pos);
                messageBuffer.erase(0, pos + 1);

                // 移除回车符
                if (!line.empty() && line.back() == '\r')
                {
                    line.pop_back();
                }

                if (line.empty())
                    continue;

                logger.information("Received from " + clientAddress_ + ": " + line); // 处理退出命令
                if (line == "quit" || line == "QUIT")
                {
                    isConnected_ = false;
                    break;
                }
            }

            if (!isConnected_)
            {
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
}

void ChatConnection::sendMessage(const std::string &message)
{
    if (!isConnected_)
        return;

    try
    {
        int sent = socket().sendBytes(message.c_str(), message.length());
        if (sent < 0)
        {
            throw std::runtime_error("Failed to send message");
        }
    }
    catch (const std::exception &e)
    {
        auto &logger = Poco::Logger::get("ChatConnection");
        logger.error("Failed to send message to " + clientAddress_ + ": " + e.what());
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
    std::string responseData;

    account_ = loginRequest.getAccount();
    auto &userManager = UserManager::getInstance();
    if (userManager.authenticateUser(account_, loginRequest.getPassword()))
    {
        auto response = new LoginResponse();
        if (response)
        {
            response->setStatus(MessageStatus::SUCCESS);
            response->setMessage("登录成功");
        }
        responseData = response->serialize();
        isAuthenticated_ = true;
        logger.information("User " + account_ + " logged in successfully.");
    }
    else
    {
        logger.error("Authentication failed for user " + account_);
        auto response = new LoginResponse();
        if (response)
        {
            response->setStatus(MessageStatus::FAILED);
            response->setMessage("登录失败");
        }
        responseData = response->serialize();
    }
    // 发送认证结果
    if (!responseData.empty())
    {
        sendMessage(responseData);
    }
}

void ChatConnection::handleRegisterRequest(const RegisterRequest &registerRequest)
{
    auto &logger = Poco::Logger::get("ChatConnection");
    std::string responseData;

    std::string username = registerRequest.getUsername();
    std::string password = registerRequest.getPassword();
    auto &userManager = UserManager::getInstance();
    account_ = userManager.registerUser(username, password);
    if (!account_.empty())
    {
        auto response = new RegisterResponse();
        if (response)
        {
            response->setStatus(MessageStatus::SUCCESS);
            response->setMessage("注册成功，您的账号是: " + account_);
        }
        responseData = response->serialize();
        isAuthenticated_ = true;
        logger.information("User " + account_ + " registered successfully.");
    }
    else
    {
        auto response = new RegisterResponse();
        if (response)
        {
            response->setStatus(MessageStatus::FAILED);
            response->setMessage("注册失败");
        }
        responseData = response->serialize();
    }
    // 发送注册结果
    if (!responseData.empty())
    {
        sendMessage(responseData);
    }
}

void ChatConnection::handleChatMessage(const ChatMessage &chatMessage)
{
    auto &logger = Poco::Logger::get("ChatConnection");

    if (chatMessage.isPrivateMessage() && chatMessage.getType() == MessageType::PRIVATE_MESSAGE)
    {
    }
    else if (chatMessage.isBroadcastMessage() && chatMessage.getType() == MessageType::BROADCAST_MESSAGE)
    {
        auto &connectionManager = ConnectionManager::getInstance();
        std::string formattedMessage = formatMessage(chatMessage.getContent());
        ChatMessage broadcastMessage = ChatMessage(chatMessage.getSender(), formattedMessage);
        broadcastMessage.setReceiver("");
        connectionManager.broadcastMessage(broadcastMessage, this);
        logger.information("Broadcast message from " + chatMessage.getSender() + "[" + clientAddress_ + "]" + ": " + chatMessage.getContent());
    }
    else
    {
        logger.warning("Received unsupported chat message type from " + clientAddress_);
    }
}

std::string ChatConnection::formatMessage(const std::string &content) const
{
    Poco::LocalDateTime now;
    std::string timestamp = Poco::DateTimeFormatter::format(now, "%H:%M");

    std::ostringstream oss;
    oss << "[" << timestamp << "] " << UserManager::getInstance().getUserByAccount(account_).username + "(" + clientAddress_ + ")" << ": " << content;
    return oss.str();
}
