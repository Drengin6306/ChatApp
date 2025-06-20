# ChatApp - 多人聊天室

一个基于 C++ 和 Poco 库开发的多人聊天室应用程序。

## 功能特性

- 多客户端同时连接
- 实时消息广播
- 简单的文本聊天
- 优雅的连接管理

## 项目结构

```
ChatApp/
├── CMakeLists.txt              # 顶层CMake配置文件
├── README.md                   # 项目说明
├── config/                     # 配置文件目录
├── server/                     # 服务器端代码
├── client/                     # 客户端代码
└── build/                      # 编译输出目录
```

## 依赖要求

- CMake 3.15+
- C++17 编译器
- Poco C++ 库
  ```bash
  sudo apt install libpoco-dev
  ```

## 编译和运行

### 编译项目

```bash
mkdir -p build
cd build
cmake ..
make
```

### 启动服务器

```bash
./server/chat_server
```

### 启动客户端

```bash
./client/chat_client
```

## 使用说明

1. 首先启动服务器，默认监听端口 9999
2. 启动一个或多个客户端连接到服务器
3. 在客户端输入消息，按回车发送
4. 输入 "quit" 退出客户端

## 配置

服务器配置文件位于 `config/server.properties`，可以修改端口和其他设置。
