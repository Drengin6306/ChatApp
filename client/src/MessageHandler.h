#pragma once

#include "ClientApp.h"
#include <Poco/Runnable.h>
#include <Poco/Net/StreamSocket.h>
#include <memory>
#include <atomic>

class MessageHandler : public Poco::Runnable
{
public:
    static MessageHandler &getInstance();
    void initialize(std::shared_ptr<Poco::Net::StreamSocket> socket, std::shared_ptr<ClientApp> clientApp);
    ~MessageHandler();

    void run() override;
    void stop();

private:
    MessageHandler() = default;
    MessageHandler(const MessageHandler &) = delete;
    MessageHandler &operator=(const MessageHandler &) = delete;
    MessageHandler(MessageHandler &&) = delete;

    void handleLoginResponse(const LoginResponse &response);
    void handleRegisterResponse(const RegisterResponse &response);
    void handleChatMessage(const ChatMessage &message);

    std::unique_ptr<Message> receiveMessage();
    std::shared_ptr<Poco::Net::StreamSocket> socket_;
    std::atomic<bool> running_;
    std::weak_ptr<ClientApp> clientApp_;
};
