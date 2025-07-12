#include "ClientApp.h"
#include "MessageHandler.h"
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/NetException.h>
#include <iostream>
#include <string>
#include <memory>
#include <limits>

ClientApp::ClientApp() : connected_(false), authenticated_(false)
{
}

ClientApp::~ClientApp()
{
    disconnect();
}

int ClientApp::run(const std::string &host, int port)
{
    std::cout << "欢迎来到聊天室！" << std::endl;
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
        else if (input == "help" || input == "HELP")
        {
            showHelp();
        }
        else if (input.substr(0, 5) == "login")
        {
            std::string account, password;
            std::cout << "请输入账号: ";
            std::cin >> account;
            std::cout << "请输入密码: ";
            std::cin >> password;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (account.empty() || password.empty())
            {
                std::cerr << "账号和密码不能为空" << std::endl;
                continue;
            }
            login(account, password);
        }
        else if (input.substr(0, 8) == "register")
        {
            std::string username, password;
            std::cout << "请输入用户名: ";
            std::cin >> username;
            std::cout << "请输入密码: ";
            std::cin >> password;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            if (username.empty() || password.empty())
            {
                std::cerr << "用户名和密码不能为空" << std::endl;
                continue;
            }
            registerUser(username, password);
        }
        else if (input.substr(0, 6) == "logout")
        {
            logout();
        }
        else if (input.substr(0, 2) == "\\b")
        {
            if (!authenticated_)
            {
                std::cerr << "请先登录或注册账号" << std::endl;
                continue;
            }
            // 广播消息
            std::string content = input.substr(2);
            size_t firstNonSpace = content.find_first_not_of(" \t");
            if (firstNonSpace != std::string::npos)
            {
                content = content.substr(firstNonSpace);
            }
            else
            {
                std::cerr << "消息不能为空" << std::endl;
                continue;
            }
            sendMessage(ChatMessage(this->account_, content));
        }
        else if (input.substr(0, 2) == "\\p")
        {
            if (!authenticated_)
            {
                std::cerr << "请先登录或注册账号" << std::endl;
                continue;
            }
            // 私聊消息
            std::string command = input.substr(2);

            size_t firstNonSpace = command.find_first_not_of(" \t");
            if (firstNonSpace != std::string::npos)
            {
                command = command.substr(firstNonSpace);

                size_t spacePos = command.find(' ');
                if (spacePos != std::string::npos)
                {
                    std::string receiver = command.substr(0, spacePos);

                    size_t messageStart = command.find_first_not_of(" \t", spacePos + 1);
                    if (messageStart != std::string::npos)
                    {
                        std::string messageContent = command.substr(messageStart);
                        sendMessage(ChatMessage(this->account_, receiver, messageContent));
                    }
                    else
                    {
                        std::cerr << "消息内容不能为空" << std::endl;
                    }
                }
                else
                {
                    std::cerr << "私聊格式错误，请使用 \\p <账号> <消息>" << std::endl;
                }
            }
            else
            {
                std::cerr << "私聊格式错误，请使用 \\p <账号> <消息>" << std::endl;
            }
        }
        else
        {
            std::cout << "未知命令: " << input << std::endl;
            std::cout << "请输入 'help' 查看可用命令" << std::endl;
        }
    }
}

void ClientApp::disconnect()
{
    if (receiverThread_ && receiverThread_->isRunning())
    {
        receiverThread_->join();
    }

    if (socket_)
    {
        try
        {
            socket_->close();
        }
        catch (const Poco::Net::NetException &e)
        {
            std::cerr << "关闭连接时出错: " << e.displayText() << std::endl;
        }
    }

    connected_ = false;
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

void ClientApp::registerUser(const std::string &username, const std::string &password)
{
    RegisterRequest registerRequest(username, password);
    sendMessage(registerRequest);
}

void ClientApp::logout()
{
    if (!authenticated_)
    {
        std::cerr << "您尚未登录" << std::endl;
        return;
    }

    authenticated_ = false;
    account_.clear();
    std::cout << "已登出" << std::endl;
}

void ClientApp::showHelp()
{
    std::cout << "\n可用命令:\n";
    std::cout << "  login     - 登录系统\n";
    std::cout << "  register  - 注册新账号\n";
    std::cout << "  logout    - 登出系统\n";
    std::cout << "  \\b <message>        - 发送广播消息\n";
    std::cout << "  \\p <account> <message> - 发送私聊消息\n";
    std::cout << "  help      - 显示帮助信息\n";
    std::cout << "  quit/exit - 退出程序\n";
}