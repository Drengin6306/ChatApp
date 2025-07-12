#pragma once

#include "Message.h"
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
    bool isConnected() const { return connected_; }
    bool isAuthenticated() const { return authenticated_; }
    void setAccount(const std::string &account) { account_ = account; }
    void setAuthenticated(bool authenticated) { authenticated_ = authenticated; }
    void setConnected(bool connected) { connected_ = connected; }

private:
    void connectToServer(const std::string &host, int port);
    void login(const std::string &account, const std::string &password);
    void logout();
    void registerUser(const std::string &username, const std::string &password);
    void sendBroadcastMessage(const std::string &input);
    void sendPrivateMessage(const std::string &input);
    void showHelp();
    void sendMessage(const Message &message);
    void startMessageReceiver();
    void handleUserInput();
    void disconnect();

    std::unordered_map<std::string, std::string> userMap_;
    std::shared_ptr<Poco::Net::StreamSocket> socket_;
    std::unique_ptr<Poco::Thread> receiverThread_;
    std::string account_;
    bool connected_;
    bool authenticated_;
};
