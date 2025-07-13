#include "ClientApp.h"
#include "MessageHandler.h"
#include <iostream>
#include <string>
#include <Poco/Net/NetException.h>
#include <csignal>

std::shared_ptr<ClientApp> clientApp;
void signalHandler(int signum)
{
    if (clientApp)
    {
        clientApp->disconnect();
        std::cout << "客户端已退出" << std::endl;
    }
    std::_Exit(0);
}

int main(int argc, char **argv)
{
    std::signal(SIGINT, signalHandler);
    std::string host = "localhost";
    int port = 9999;

    // 简单的命令行参数解析
    if (argc >= 2)
    {
        host = argv[1];
    }
    if (argc >= 3)
    {
        try
        {
            port = std::stoi(argv[2]);
        }
        catch (const std::exception &e)
        {
            std::cerr << "无效的端口号: " << argv[2] << std::endl;
            return 1;
        }
    }

    std::cout << "欢迎来到聊天室，输入 'help' 查看可用命令" << std::endl;
    std::cout << "正在连接到服务器 " << host << ":" << port << "..." << std::endl;

    auto clientApp = std::make_shared<ClientApp>();
    try
    {
        clientApp->connectToServer(host, port);
        MessageHandler::getInstance().initialize(clientApp->getSocket(), clientApp);
        clientApp->startMessageReceiver();
        clientApp->handleUserInput();
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
    clientApp->disconnect();
    std::cout << "客户端已退出" << std::endl;

    return 0;
}
