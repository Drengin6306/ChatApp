#include "ServerApp.h"
#include "ChatConnection.h"
#include <Poco/Net/TCPServerParams.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Util/PropertyFileConfiguration.h>
#include <Poco/Logger.h>
#include <Poco/PatternFormatter.h>
#include <Poco/FormattingChannel.h>
#include <Poco/ConsoleChannel.h>
#include <Poco/AutoPtr.h>
#include <iostream>

// 连接工厂实现
Poco::Net::TCPServerConnection *ChatConnectionFactory::createConnection(const Poco::Net::StreamSocket &socket)
{
    return new ChatConnection(socket);
}

ServerApp::ServerApp()
    : helpRequested_(false), port_(9999), host_("0.0.0.0"), maxConnections_(100)
{
}

ServerApp::~ServerApp()
{
}

void ServerApp::initialize(Poco::Util::Application &self)
{
    // 先调用父类初始化
    ServerApplication::initialize(self);

    // 加载配置文件
    loadConfiguration();

    // 简化日志设置
    Poco::Logger::root().setLevel(Poco::Message::PRIO_INFORMATION);

    auto &logger = Poco::Logger::get("ServerApp");
    logger.information("服务器初始化完成");
}

void ServerApp::uninitialize()
{
    auto &logger = Poco::Logger::get("ServerApp");
    logger.information("服务器正在关闭...");

    ServerApplication::uninitialize();
}

void ServerApp::defineOptions(Poco::Util::OptionSet &options)
{
    ServerApplication::defineOptions(options);

    options.addOption(
        Poco::Util::Option("help", "h", "显示帮助信息")
            .required(false)
            .repeatable(false)
            .callback(Poco::Util::OptionCallback<ServerApp>(this, &ServerApp::handleOption)));

    options.addOption(
        Poco::Util::Option("port", "p", "设置监听端口")
            .required(false)
            .repeatable(false)
            .argument("PORT")
            .callback(Poco::Util::OptionCallback<ServerApp>(this, &ServerApp::handleOption)));
}

void ServerApp::handleOption(const std::string &name, const std::string &value)
{
    ServerApplication::handleOption(name, value);

    if (name == "help")
    {
        helpRequested_ = true;
        displayHelp();
        stopOptionsProcessing();
    }
    else if (name == "port")
    {
        port_ = std::stoi(value);
    }
}

void ServerApp::displayHelp()
{
    Poco::Util::HelpFormatter helpFormatter(options());
    helpFormatter.setCommand(commandName());
    helpFormatter.setUsage("选项");
    helpFormatter.setHeader("聊天室服务器");
    helpFormatter.format(std::cout);
}

void ServerApp::loadConfiguration()
{
    try
    {
        // 加载配置文件
        Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> pConfig =
            new Poco::Util::PropertyFileConfiguration("config/server.properties");
        Poco::Util::LayeredConfiguration &config = Poco::Util::Application::config();
        config.add(pConfig, "file", 100, false);

        // 从配置文件读取设置（如果可用）
        if (config.hasProperty("server.port"))
        {
            port_ = config.getInt("server.port", 9999);
        }
        if (config.hasProperty("server.host"))
        {
            host_ = config.getString("server.host", "0.0.0.0");
        }
        if (config.hasProperty("server.maxConnections"))
        {
            maxConnections_ = config.getInt("server.maxConnections", 100);
        }

        auto &logger = Poco::Logger::get("ServerApp");
        logger.information("配置文件加载成功");
    }
    catch (const Poco::FileNotFoundException &e)
    {
        auto &logger = Poco::Logger::get("ServerApp");
        logger.warning("配置文件未找到，使用默认设置: " + std::string(e.what()));
    }
    catch (const std::exception &e)
    {
        auto &logger = Poco::Logger::get("ServerApp");
        logger.warning("无法加载配置文件，使用默认设置: " + std::string(e.what()));
    }
    auto &logger = Poco::Logger::get("ServerApp");
    logger.information("监听地址: " + host_);
    logger.information("端口: " + std::to_string(port_));
    logger.information("最大连接数: " + std::to_string(maxConnections_));
}

int ServerApp::main(const std::vector<std::string> &args)
{
    if (helpRequested_)
    {
        return Poco::Util::Application::EXIT_OK;
    }

    auto &logger = Poco::Logger::get("ServerApp");

    try
    {
        // 创建服务器套接字
        Poco::Net::ServerSocket serverSocket(port_);

        // 设置服务器参数
        Poco::AutoPtr<Poco::Net::TCPServerParams> params = new Poco::Net::TCPServerParams;
        params->setMaxThreads(maxConnections_);
        params->setThreadIdleTime(Poco::Timespan(10, 0)); // 10秒空闲时间

        // 创建服务器
        server_ = std::make_unique<Poco::Net::TCPServer>(
            new ChatConnectionFactory(),
            serverSocket,
            params);

        // 启动服务器
        server_->start();

        logger.information("聊天服务器启动成功");
        logger.information("按 Ctrl+C 停止服务器");

        // 等待终止信号
        waitForTerminationRequest();

        logger.information("收到终止信号，正在关闭服务器...");

        // 停止服务器
        server_->stop();

        logger.information("服务器已停止");
    }
    catch (const std::exception &e)
    {
        logger.fatal("服务器启动失败: " + std::string(e.what()));
        return Poco::Util::Application::EXIT_SOFTWARE;
    }

    return Poco::Util::Application::EXIT_OK;
}
