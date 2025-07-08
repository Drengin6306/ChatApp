#pragma once

#include <Poco/Util/ServerApplication.h>
#include <Poco/Net/TCPServer.h>
#include <Poco/Net/TCPServerConnectionFactory.h>
#include <memory>

class ChatConnection;

class ChatConnectionFactory : public Poco::Net::TCPServerConnectionFactory
{
public:
    Poco::Net::TCPServerConnection *createConnection(const Poco::Net::StreamSocket &socket) override;
};

class ServerApp : public Poco::Util::ServerApplication
{
public:
    ServerApp();
    virtual ~ServerApp();

protected:
    void initialize(Poco::Util::Application &self) override;
    void uninitialize() override;
    int main(const std::vector<std::string> &args) override;

private:
    void loadConfiguration();

    std::unique_ptr<Poco::Net::TCPServer> server_;
    bool helpRequested_;
    int port_;
    std::string host_;
    int maxConnections_;
};
