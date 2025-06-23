#include "ChatConnection.h"
#include "ConnectionManager.h"
#include <Poco/Net/NetException.h>
#include <Poco/Logger.h>
#include <Poco/StreamCopier.h>
#include <Poco/DateTime.h>
#include <Poco/DateTimeFormatter.h>
#include <iostream>
#include <sstream>

ChatConnection::ChatConnection(const Poco::Net::StreamSocket &socket)
    : TCPServerConnection(socket), isConnected_(true)
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

        // 发送欢迎消息
        sendMessage("=== 欢迎来到聊天室 ===");
        sendMessage("输入消息开始聊天，输入 'quit' 退出");

        // 广播新用户加入消息
        std::string joinMessage = "[系统] 用户 " + clientAddress_ + " 加入了聊天室";
        connectionManager.broadcastMessage(joinMessage, this);

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

                handleClientMessage(line);
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

    // 广播用户离开消息
    std::string leaveMessage = "[系统] 用户 " + clientAddress_ + " 离开了聊天室";
    connectionManager.broadcastMessage(leaveMessage, this);
}

void ChatConnection::sendMessage(const std::string &message)
{
    if (!isConnected_)
        return;

    try
    {
        std::string messageWithNewline = message + "\n";
        int sent = socket().sendBytes(messageWithNewline.c_str(), messageWithNewline.length());
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

void ChatConnection::handleClientMessage(const std::string &message)
{
    auto &connectionManager = ConnectionManager::getInstance();

    // 格式化消息并广播
    std::string formattedMessage = formatMessage(message);
    connectionManager.broadcastMessage(formattedMessage, this);
}

std::string ChatConnection::formatMessage(const std::string &content) const
{
    Poco::LocalDateTime now;
    std::string timestamp = Poco::DateTimeFormatter::format(now, "%H:%M:%S");

    std::ostringstream oss;
    oss << "[" << timestamp << "] " << clientAddress_ << ": " << content;
    return oss.str();
}
