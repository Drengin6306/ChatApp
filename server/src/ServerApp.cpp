#include "ServerApp.h"
#include "ChatConnection.h"
#include <Poco/Net/TCPServerParams.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Util/PropertyFileConfiguration.h>
#include <Poco/Logger.h>
#include <Poco/AutoPtr.h>
#include <Poco/File.h>
#include <iostream>

// 连接工厂实现
Poco::Net::TCPServerConnection *ChatConnectionFactory::createConnection(const Poco::Net::StreamSocket &socket)
{
    return new ChatConnection(socket);
}

ServerApp::ServerApp()
    : port_(9999), host_("0.0.0.0"), maxConnections_(100)
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

void ServerApp::loadConfiguration()
{
    try
    {
        std::string configPath = "config/server.properties";

        Poco::File configFile(configPath);
        if (configFile.exists())
        {
            Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> pConfig =
                new Poco::Util::PropertyFileConfiguration(configPath);

            Poco::Util::LayeredConfiguration &config = Poco::Util::Application::config();
            config.add(pConfig, "file", 100, false);

            // 读取配置值
            port_ = config.getInt("server.port", 9999);
            host_ = config.getString("server.host", "0.0.0.0");
            maxConnections_ = config.getInt("server.maxConnections", 100);

            auto &logger = Poco::Logger::get("ServerApp");
            logger.information("配置文件加载成功");
        }
        else
        {
            auto &logger = Poco::Logger::get("ServerApp");
            logger.warning("配置文件未找到，使用默认设置");
        }
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
