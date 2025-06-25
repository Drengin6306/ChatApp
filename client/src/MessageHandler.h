#pragma once

#include <Poco/Runnable.h>
#include <Poco/Net/StreamSocket.h>
#include <memory>
#include <atomic>

class MessageHandler : public Poco::Runnable
{
public:
    MessageHandler(std::shared_ptr<Poco::Net::StreamSocket> socket);
    virtual ~MessageHandler();

    void run() override;
    void stop();

private:
    std::shared_ptr<Poco::Net::StreamSocket> socket_;
    std::atomic<bool> running_;
};
