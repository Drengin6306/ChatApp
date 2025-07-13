# ChatApp - 多人聊天室

一个基于 C++ 和 Poco 库开发的多人聊天室应用程序，支持 Windows 和 Linux 平台。

## 功能特性

- 多客户端同时连接
- 优雅的连接管理
- 跨平台支持（Windows/Linux）
- 用户管理（支持用户注册、登录、登出、昵称管理等）
- 消息协议（统一的消息格式）

## 项目结构

```
ChatApp/
├── CMakeLists.txt              # 顶层CMake配置文件
├── CMakePresets.json           # CMake预设配置
├── CMakeUserPresets.json       # 用户级CMake预设（可选）
├── vcpkg.json                  # vcpkg依赖清单
├── vcpkg-configuration.json    # vcpkg注册表配置
├── config/                     # 配置文件目录
├── protocol/                   # 消息协议相关代码
├── server/                     # 服务器端代码
├── client/                     # 客户端代码
├── build/                      # 编译输出目录
└── README.md                   # 项目说明
```

## 依赖要求

### 通用要求
- CMake 3.20+
- C++17 编译器
- vcpkg 包管理器
- Poco C++ 库（通过 vcpkg 自动安装）

### Windows 平台
- Visual Studio 2022
- Windows 10/11

### Linux 平台
- GCC 编译器
- GNU Make
- 支持的发行版：Ubuntu、Debian、CentOS、Fedora 等

## 编译和运行

### Windows 平台

```cmd
# 配置项目
cmake --preset=windows-vs2022

# 构建 Debug 版本
cmake --build --preset=windows-debug

# 构建 Release 版本
cmake --build --preset=windows-release
```

### Linux 平台

```bash
# 配置并构建 Debug 版本
cmake --preset=linux-debug
make -C build

# 配置并构建 Release 版本
cmake --preset=linux-release
make -C build
```

## 启动应用程序

### 启动服务器

#### Windows
```cmd
.\chat_server.exe
```

#### Linux
```bash
./chat_server
```

### 启动客户端

#### Windows
```cmd
.\chat_client.exe
```

#### Linux
```bash
./chat_client
```

## 使用说明

1. 首先启动服务器，默认监听端口 9999
2. 启动一个或多个客户端连接到服务器
3. 在客户端输入命令使用

## 用户管理

- 支持用户注册、登录和登出。
- 每个用户可设置唯一昵称，昵称在聊天室内唯一。
- 服务器实现对用户的管理。
- 用户信息存储在config/users.json中

## 消息协议

- 客户端与服务器之间采用统一的消息协议进行通信，所有消息均为 JSON 格式。
- 典型消息结构如下：

```json
{
  "id": "消息ID",
  "type": "chatmessage|login|logout|register",
  "sender": "发送者账号",
  "receiver": "接收者账号（可选）",
  "content": "消息内容",
  "timestamp": "时间戳"
}
```

## 配置

服务器配置文件位于 `config/server.properties`，可以修改端口和其他设置：

## 开发说明

### CMake 预设

项目提供了多平台的 CMake 预设配置：

#### Windows 预设
- `windows-vs2022`：Windows Visual Studio 2022 配置预设

#### Linux 预设
- `linux-debug`：Linux GCC Debug 配置预设
- `linux-release`：Linux GCC Release 配置预设

#### 构建预设
- `windows-debug`：Windows Debug 构建
- `windows-release`：Windows Release 构建

### 查看可用预设

```bash
# 查看配置预设
cmake --list-presets=configure

# 查看构建预设
cmake --list-presets=build
```

## 贡献

欢迎提交 Issue 和 Pull Request 来改进这个项目。

