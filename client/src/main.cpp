#include "ClientApp.h"
#include <iostream>
#include <string>

int main(int argc, char **argv)
{
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

    ClientApp app;
    return app.run(host, port);
}
