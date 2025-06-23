#pragma once

#include <Poco/Net/TCPServerConnection.h>
#include <Poco/Net/StreamSocket.h>
#include <string>

class ChatConnection : public Poco::Net::TCPServerConnection
{
public:
    ChatConnection(const Poco::Net::StreamSocket &socket);
    virtual ~ChatConnection();

    void run() override;
    void sendMessage(const std::string &message);

    std::string getClientAddress() const;

private:
    std::string clientAddress_;
    bool isConnected_;

    void handleClientMessage(const std::string &message);
    std::string formatMessage(const std::string &content) const;
};
