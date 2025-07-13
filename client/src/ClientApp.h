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

    ClientApp(const ClientApp &) = delete;
    ClientApp &operator=(const ClientApp &) = delete;

    bool isConnected() const { return connected_; }
    bool isAuthenticated() const { return authenticated_; }
    void setAccount(const std::string &account) { account_ = account; }
    void setAuthenticated(bool authenticated) { authenticated_ = authenticated; }
    void setConnected(bool connected) { connected_ = connected; }
    void connectToServer(const std::string &host, int port);
    std::shared_ptr<Poco::Net::StreamSocket> getSocket() const { return socket_; }
    void startMessageReceiver();
    void handleUserInput();
    void disconnect();

private:
    void login(const std::string &account, const std::string &password);
    void logout();
    void registerUser(const std::string &username, const std::string &password);
    void sendBroadcastMessage(const std::string &input);
    void sendPrivateMessage(const std::string &input);
    void showHelp();
    void sendMessage(const Message &message);

    std::unordered_map<std::string, std::string> userMap_;
    std::shared_ptr<Poco::Net::StreamSocket> socket_;
    std::unique_ptr<Poco::Thread> receiverThread_;
    std::string account_;
    bool connected_;
    bool authenticated_;
};
