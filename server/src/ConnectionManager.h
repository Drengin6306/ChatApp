#pragma once

#include <Poco/Net/TCPServerConnection.h>
#include <Poco/Net/StreamSocket.h>
#include <vector>
#include <mutex>
#include <string>

class ChatConnection;

class ConnectionManager
{
public:
    static ConnectionManager &getInstance();

    void addConnection(ChatConnection *connection);
    void removeConnection(ChatConnection *connection);
    void broadcastMessage(const std::string &message, ChatConnection *sender = nullptr);

    size_t getConnectionCount() const;

private:
    ConnectionManager() = default;
    ~ConnectionManager() = default;
    ConnectionManager(const ConnectionManager &) = delete;
    ConnectionManager &operator=(const ConnectionManager &) = delete;

    mutable std::mutex connectionsMutex_;
    mutable std::vector<ChatConnection *> connections_;
};
