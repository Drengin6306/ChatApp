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
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    connections_.push_back(connection);

    auto &logger = Poco::Logger::get("ConnectionManager");
    logger.information("New connection added. Total connections: " + std::to_string(connections_.size()));
}

void ConnectionManager::removeConnection(ChatConnection *connection)
{
    std::lock_guard<std::mutex> lock(connectionsMutex_);

    connections_.erase(
        std::remove(connections_.begin(), connections_.end(), connection),
        connections_.end());

    auto &logger = Poco::Logger::get("ConnectionManager");
    logger.information("Connection removed. Remaining connections: " + std::to_string(connections_.size()));
}

void ConnectionManager::broadcastMessage(const ChatMessage &message, ChatConnection *sender)
{
    std::lock_guard<std::mutex> lock(connectionsMutex_);

    auto &logger = Poco::Logger::get("ConnectionManager");
    logger.information("Broadcasting message to " + std::to_string(connections_.size()) + " connections");

    for (auto it = connections_.begin(); it != connections_.end();)
    {
        ChatConnection *connection = *it;

        // 不发送给发送者自己
        if (connection != sender)
        {
            try
            {
                connection->sendMessage(message);
                ++it;
            }
            catch (const std::exception &e)
            {
                logger.error("Failed to send message to connection: " + std::string(e.what()));
                it = connections_.erase(it);
            }
        }
        else
        {
            ++it;
        }
    }
}

size_t ConnectionManager::getConnectionCount() const
{
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    return connections_.size();
}
