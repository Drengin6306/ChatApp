#include "ConnectionManager.h"
#include "ChatConnection.h"
#include <Poco/Logger.h>
#include <algorithm>

ConnectionManager &ConnectionManager::getInstance()
{
    static ConnectionManager instance;
    return instance;
}

void ConnectionManager::addConnection(ChatConnection *connection)
{
    std::unique_lock<std::shared_mutex> lock(connectionsMutex_);

    unauthenticatedConnections_.push_back(connection);

    auto &logger = Poco::Logger::get("ConnectionManager");
    logger.information("New connection added. Total connections: " + std::to_string(getConnectionCount()));
}

void ConnectionManager::authenticateConnection(ChatConnection *connection, const std::string &account)
{
    std::unique_lock<std::shared_mutex> lock(connectionsMutex_);

    // 将连接从未认证列表中移除
    auto it = std::remove(unauthenticatedConnections_.begin(), unauthenticatedConnections_.end(), connection);
    if (it != unauthenticatedConnections_.end())
    {
        unauthenticatedConnections_.erase(it, unauthenticatedConnections_.end());
    }

    // 检查是否已存在同一账号的连接
    auto existingIt = connections_.find(account);
    if (existingIt != connections_.end())
    {
        ChatConnection *oldConnection = existingIt->second;
        auto &logger = Poco::Logger::get("ConnectionManager");
        logger.warning("Account " + account + " already logged in. Kicking out old connection from " + oldConnection->getClientAddress());

        try
        {
            // 向旧连接发送被踢出的消息
            auto kickoutMsg = std::make_unique<ErrorMessage>();
            kickoutMsg->setErrorMessage("您的账号在另一设备登录，当前会话已断开");
            oldConnection->sendMessage(*kickoutMsg);
            oldConnection->setDisconnected();
        }
        catch (const std::exception &e)
        {
            logger.error("Failed to notify old connection: " + std::string(e.what()));
        }
    }

    // 添加到已认证列表中
    connections_[account] = connection;

    auto &logger = Poco::Logger::get("ConnectionManager");
    logger.information("Connection authenticated for account: " + account);
}

void ConnectionManager::removeConnection(ChatConnection *connection)
{
    std::unique_lock<std::shared_mutex> lock(connectionsMutex_);

    // 从已认证连接中移除
    if (connection->isAuthenticated())
    {
        auto it = std::find_if(connections_.begin(), connections_.end(),
                               [connection](const auto &pair)
                               { return pair.second == connection; });
        if (it != connections_.end())
        {
            connections_.erase(it);
        }
        auto &logger = Poco::Logger::get("ConnectionManager");
        logger.information("Connection removed for authenticated user: " + connection->getClientAddress() + " Total connections: " + std::to_string(getConnectionCount()));
    }
    // 从未认证连接中移除
    else
    {
        auto it = std::remove(unauthenticatedConnections_.begin(), unauthenticatedConnections_.end(), connection);
        if (it != unauthenticatedConnections_.end())
        {
            unauthenticatedConnections_.erase(it, unauthenticatedConnections_.end());
        }
        auto &logger = Poco::Logger::get("ConnectionManager");
        logger.information("Connection removed for unauthenticated user: " + connection->getClientAddress() + " Total connections: " + std::to_string(getConnectionCount()));
    }
}

void ConnectionManager::broadcastMessage(const ChatMessage &message, ChatConnection *sender)
{
    std::shared_lock<std::shared_mutex> lock(connectionsMutex_);

    auto &logger = Poco::Logger::get("ConnectionManager");
    logger.information("Broadcasting message to " + std::to_string(connections_.size()) + " connections");

    for (const auto &pair : connections_)
    {
        ChatConnection *connection = pair.second;
        if (connection != sender && connection->isConnected())
        {
            try
            {
                connection->sendMessage(message);
            }
            catch (const std::exception &e)
            {
                logger.error("Failed to send message to " + connection->getClientAddress() + ": " + e.what());
            }
        }
    }
}
void ConnectionManager::sendMessageToUser(const ChatMessage &message)
{
    std::shared_lock<std::shared_mutex> lock(connectionsMutex_);

    auto &logger = Poco::Logger::get("ConnectionManager");
    logger.information("Sending message to user: " + message.getReceiver());

    auto it = connections_.find(message.getReceiver());
    if (it != connections_.end())
    {
        ChatConnection *connection = it->second;
        if (connection->isConnected())
        {
            try
            {
                connection->sendMessage(message);
            }
            catch (const std::exception &e)
            {
                logger.error("Failed to send message to " + connection->getClientAddress() + ": " + e.what());
            }
        }
        else
        {
            logger.warning("Connection for user " + message.getReceiver() + " is not connected.");
        }
    }
    else
    {
        logger.warning("No connection found for user: " + message.getReceiver());
    }
}

size_t ConnectionManager::getConnectionCount() const
{
    std::shared_lock<std::shared_mutex> lock(connectionsMutex_);
    return connections_.size() + unauthenticatedConnections_.size();
}
