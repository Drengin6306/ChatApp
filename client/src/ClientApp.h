#pragma once

#include <Poco/Net/StreamSocket.h>
#include <Poco/Thread.h>
#include <memory>
#include <string>

class MessageHandler;

class ClientApp
{
public:
    ClientApp();
    virtual ~ClientApp();

    int run(const std::string &host = "localhost", int port = 9999);

private:
    void connectToServer(const std::string &host, int port);
    void startMessageReceiver();
    void handleUserInput();
    void disconnect();

    std::shared_ptr<Poco::Net::StreamSocket> socket_;
    std::unique_ptr<MessageHandler> messageHandler_;
    std::unique_ptr<Poco::Thread> receiverThread_;
    bool connected_;
};
