#pragma once

#include "Message.h"
#include <Poco/Net/TCPServerConnection.h>
#include <Poco/Net/StreamSocket.h>
#include <string>

class ChatConnection : public Poco::Net::TCPServerConnection
{
public:
    ChatConnection(const Poco::Net::StreamSocket &socket);
    virtual ~ChatConnection();

    void run() override;
    void sendMessage(const Message &message);
    bool isConnected() const { return isConnected_; }
    bool isAuthenticated() const { return isAuthenticated_; }
    void setDisconnected() { isConnected_ = false; }

    std::string getClientAddress() const;

private:
    std::string clientAddress_;
    std::string account_;
    bool isConnected_;
    bool isAuthenticated_;

    void handleChatMessage(const ChatMessage &chatMessage);
    void handleLoginRequest(const LoginRequest &loginRequest);
    void handleRegisterRequest(const RegisterRequest &registerRequest);
    std::unique_ptr<Message> receiveMessage();
};
