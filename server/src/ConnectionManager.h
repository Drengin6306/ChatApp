#pragma once

#include "Message.h"
#include <Poco/Net/TCPServerConnection.h>
#include <Poco/Net/StreamSocket.h>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <string>

class ChatConnection;

class ConnectionManager
{
public:
    static ConnectionManager &getInstance();

    void addConnection(ChatConnection *connection);
    void authenticateConnection(ChatConnection *connection, const std::string &account);
    void removeConnection(ChatConnection *connection);
    void broadcastMessage(const ChatMessage &message, ChatConnection *sender = nullptr);
    void sendMessageToUser(const ChatMessage &message);

    size_t getConnectionCount() const;

private:
    ConnectionManager() = default;
    ~ConnectionManager() = default;
    ConnectionManager(const ConnectionManager &) = delete;
    ConnectionManager &operator=(const ConnectionManager &) = delete;

    mutable std::shared_mutex connectionsMutex_;
    mutable std::unordered_map<std::string, ChatConnection *> connections_;
    mutable std::vector<ChatConnection *> unauthenticatedConnections_;
};
