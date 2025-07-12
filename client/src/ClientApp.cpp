#include "ClientApp.h"
#include "MessageHandler.h"
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/NetException.h>
#include <iostream>
#include <string>
#include <memory>

ClientApp::ClientApp() : connected_(false), authenticated_(false)
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
}

void ClientApp::startMessageReceiver()
{
    if (!connected_)
    {
        std::cerr << "未连接到服务器，无法启动消息接收器" << std::endl;
        return;
    }
    std::shared_ptr<ClientApp> self(this, [](ClientApp *p) {});
    MessageHandler::getInstance().initialize(socket_, self);
    receiverThread_ = std::make_unique<Poco::Thread>();
    receiverThread_->start(MessageHandler::getInstance());
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

void ClientApp::sendMessage(const Message &message)
{
    if (!connected_ || !socket_)
    {
        std::cerr << "未连接到服务器，无法发送消息" << std::endl;
        return;
    }

    try
    {
        std::string jsonData = message.serialize();
        uint32_t messageLength = htonl(static_cast<uint32_t>(jsonData.length()));

        int sent = socket_->sendBytes(&messageLength, sizeof(messageLength));
        if (sent != sizeof(messageLength))
        {
            throw std::runtime_error("发送消息长度失败");
        }

        sent = socket_->sendBytes(jsonData.c_str(), jsonData.length());
        if (sent != jsonData.length())
        {
            throw std::runtime_error("发送消息内容不完整");
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
}

void ClientApp::login(const std::string &account, const std::string &password)
{
    LoginRequest loginRequest(account, password);
    sendMessage(loginRequest);
}

void ClientApp::showHelp()
{
    std::cout << "\n可用命令:\n";
    std::cout << "  login     - 登录系统\n";
    std::cout << "  register  - 注册新账号\n";
    std::cout << "  logout    - 登出系统\n";
    std::cout << "  help      - 显示帮助信息\n";
    std::cout << "  quit/exit - 退出程序\n";
    std::cout << "  <其他文本> - 发送聊天消息\n\n";
}