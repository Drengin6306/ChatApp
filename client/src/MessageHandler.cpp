#include "MessageHandler.h"
#include <Poco/Net/NetException.h>
#include <iostream>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>

MessageHandler &MessageHandler::getInstance()
{
    static MessageHandler instance;
    return instance;
}

void MessageHandler::initialize(std::shared_ptr<Poco::Net::StreamSocket> socket, std::shared_ptr<ClientApp> clientApp)
{
    socket_ = socket;
    clientApp_ = clientApp;
    running_ = true;
}

MessageHandler::~MessageHandler()
{
    stop();
}

void MessageHandler::run()
{
    while (running_)
    {
        try
        {
            auto message = receiveMessage();
            if (!message)
            {
                if (!running_)
                    break;
                continue;
            }
            MessageType type = message->getType();
            switch (type)
            {
            case MessageType::LOGIN_RESPONSE:
                handleLoginResponse(static_cast<LoginResponse &>(*message));
                break;
            case MessageType::REGISTER_RESPONSE:
                handleRegisterResponse(static_cast<RegisterResponse &>(*message));
                break;
            case MessageType::BROADCAST_MESSAGE:
            case MessageType::PRIVATE_MESSAGE:
                handleChatMessage(static_cast<ChatMessage &>(*message));
                break;
            default:
                std::cerr << "未知消息类型: " << static_cast<int>(type) << std::endl;
                break;
            }
        }
        catch (const Poco::Exception &e)
        {
            std::cerr << "接收消息失败: " << e.displayText() << std::endl;
            auto clientApp = clientApp_.lock();
            if (clientApp)
            {
                clientApp->setConnected(false);
            }
            running_ = false;
            break;
        }
    }
}

void MessageHandler::stop()
{
    running_ = false;
}

void MessageHandler::handleLoginResponse(const LoginResponse &response)
{
    auto clientApp = clientApp_.lock();
    if (clientApp)
    {
        if (response.getStatus() == MessageStatus::SUCCESS)
        {
            clientApp->setAuthenticated(true);
            clientApp->setAccount(response.getAccount());
            clientApp->setUsername(response.getUsername());
            std::cout << response.getMessage() << std::endl;
        }
        else
        {
            clientApp->setAuthenticated(false);
            std::cerr << response.getMessage() << std::endl;
        }
    }
}

void MessageHandler::handleRegisterResponse(const RegisterResponse &response)
{
    auto clientApp = clientApp_.lock();
    if (clientApp)
    {
        if (response.getStatus() == MessageStatus::SUCCESS)
        {
            std::cout << response.getMessage() << std::endl;
        }
        else
        {
            std::cerr << response.getMessage() << std::endl;
        }
    }
}

void MessageHandler::handleChatMessage(const ChatMessage &message)
{
    if (auto clientApp = clientApp_.lock())
    {
        // 使用结构化绑定简化代码
        auto [sender, sender_username, content, timestamp, isPrivate] =
            std::tuple{message.getSender(), message.getSenderUsername(), message.getContent(),
                       message.getTimestamp(), message.isPrivateMessage()};

        // 处理时间戳
        auto timePoint = std::chrono::system_clock::time_point{std::chrono::seconds{timestamp}};
        auto tm = std::chrono::system_clock::to_time_t(timePoint);
        std::tm localTime = *std::localtime(&tm);

        std::stringstream ss;
        ss << "[" << std::put_time(&localTime, "%H:%M") << "] ";

        if (isPrivate)
        {
            ss << "[私信] ";
        }
        else
        {
            ss << "[广播] ";
        }
        ss << std::endl;
        ss << sender_username << "(" << sender << ")" << ": ";
        ss << content;

        std::cout << ss.str() << std::endl;
    }
}

// 接受消息并返回一个 Message 对象
std::unique_ptr<Message> MessageHandler::receiveMessage()
{
    auto clientApp = clientApp_.lock();
    if (!clientApp || !socket_ || !clientApp->isConnected())
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
            int received = socket_->receiveBytes(
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
            int received = socket_->receiveBytes(
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

        return Message::parseMessage(jsonData);
    }
    catch (const Poco::TimeoutException &e)
    {
        return nullptr;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        running_ = false;
        clientApp->setConnected(false);
        return nullptr;
    }
}