#include "ClientApp.h"
#include "MessageHandler.h"
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/NetException.h>
#include <iostream>
#include <string>

ClientApp::ClientApp() : connected_(false)
{
}

ClientApp::~ClientApp()
{
    disconnect();
}

int ClientApp::run(const std::string &host, int port)
{
    std::cout << "=== 聊天室客户端 ===" << std::endl;
    std::cout << "正在连接到服务器 " << host << ":" << port << "..." << std::endl;

    try
    {
        connectToServer(host, port);
        startMessageReceiver();
        handleUserInput();
    }
    catch (const Poco::Net::NetException &e)
    {
        std::cerr << "网络错误: " << e.displayText() << std::endl;
        return 1;
    }
    catch (const std::exception &e)
    {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }

    disconnect();
    std::cout << "客户端已退出" << std::endl;
    return 0;
}

void ClientApp::connectToServer(const std::string &host, int port)
{
    socket_ = std::make_shared<Poco::Net::StreamSocket>();

    Poco::Net::SocketAddress address(host, port);
    socket_->connect(address);

    connected_ = true;
    std::cout << "连接成功！" << std::endl;
    std::cout << "输入消息并按回车发送，输入 'quit' 退出" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
}

void ClientApp::startMessageReceiver()
{
    messageHandler_ = std::make_unique<MessageHandler>(socket_);
    receiverThread_ = std::make_unique<Poco::Thread>();
    receiverThread_->start(*messageHandler_);
}

void ClientApp::handleUserInput()
{
    std::string input;

    while (connected_ && std::getline(std::cin, input))
    {
        if (input == "quit" || input == "QUIT")
        {
            break;
        }

        if (input.empty())
        {
            continue;
        }

        try
        {
            // 添加换行符并发送
            std::string message = input + "\n";
            int sent = socket_->sendBytes(message.c_str(), message.length());
            if (sent < 0)
            {
                std::cerr << "发送消息失败" << std::endl;
                break;
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "发送消息失败: " << e.what() << std::endl;
            break;
        }
    }
}

void ClientApp::disconnect()
{
    if (!connected_)
        return;

    connected_ = false;

    if (messageHandler_)
    {
        messageHandler_->stop();
    }

    if (socket_)
    {
        try
        {
            // 发送退出消息
            std::string quitMessage = "quit\n";
            socket_->sendBytes(quitMessage.c_str(), quitMessage.length());
        }
        catch (...)
        {
            // 忽略发送退出消息时的错误
        }

        socket_->close();
    }

    if (receiverThread_)
    {
        try
        {
            receiverThread_->join();
        }
        catch (...)
        {
            // 忽略线程结束时的错误
        }
    }
}
