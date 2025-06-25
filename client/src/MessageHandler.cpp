#include "MessageHandler.h"
#include <Poco/Net/NetException.h>
#include <iostream>
#include <string>

MessageHandler::MessageHandler(std::shared_ptr<Poco::Net::StreamSocket> socket)
    : socket_(socket), running_(true)
{
}

MessageHandler::~MessageHandler()
{
    stop();
}

void MessageHandler::run()
{
    try
    {
        char buffer[1024];
        std::string messageBuffer;

        while (running_)
        {
            int bytesReceived = socket_->receiveBytes(buffer, sizeof(buffer) - 1);
            if (bytesReceived <= 0)
            {
                break; // 连接已关闭
            }

            buffer[bytesReceived] = '\0';
            messageBuffer += buffer;

            // 处理完整的消息行
            size_t pos;
            while ((pos = messageBuffer.find('\n')) != std::string::npos)
            {
                std::string line = messageBuffer.substr(0, pos);
                messageBuffer.erase(0, pos + 1);

                // 移除回车符
                if (!line.empty() && line.back() == '\r')
                {
                    line.pop_back();
                }

                if (!line.empty())
                {
                    std::cout << line << std::endl;
                }
            }
        }
    }
    catch (const Poco::Net::NetException &e)
    {
        if (running_)
        {
            std::cerr << "网络错误: " << e.displayText() << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        if (running_)
        {
            std::cerr << "接收消息时发生错误: " << e.what() << std::endl;
        }
    }

    running_ = false;
}

void MessageHandler::stop()
{
    running_ = false;
}
