# ChatApp - 多人聊天室

一个基于 C++ 和 Poco 库开发的多人聊天室应用程序，支持 Windows 和 Linux 平台。

## 功能特性

- 多客户端同时连接
- 实时消息广播
- 简单的文本聊天
- 优雅的连接管理
- 跨平台支持（Windows/Linux）

## 项目结构

```
ChatApp/
├── CMakeLists.txt              # 顶层CMake配置文件
├── CMakePresets.json           # CMake预设配置
├── CMakeUserPresets.json       # 用户级CMake预设（可选）
├── vcpkg.json                  # vcpkg依赖清单
├── vcpkg-configuration.json    # vcpkg注册表配置
├── README.md                   # 项目说明
├── config/                     # 配置文件目录
├── server/                     # 服务器端代码
├── client/                     # 客户端代码
└── build/                      # 编译输出目录
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

#### 使用 CMake 预设编译（推荐）
```cmd
# 配置项目
cmake --preset=windows-vs2022

# 构建 Debug 版本
cmake --build --preset=windows-debug

# 构建 Release 版本
cmake --build --preset=windows-release
```

#### 手动配置
```cmd
cmake -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake -S. -Bbuild -G "Visual Studio 17 2022"
cmake --build build --config Release
```

### Linux 平台

#### 使用 CMake 预设编译（推荐）
```bash
# 配置并构建 Debug 版本
cmake --preset=linux-debug
make -C build

# 配置并构建 Release 版本
cmake --preset=linux-release
make -C build
```

#### 手动配置
```bash
cmake -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## 启动应用程序

### 启动服务器

#### Windows
```cmd
# Debug 版本
.\bin\chat_server.exe

# Release 版本
.\bin\chat_server.exe
```

#### Linux
```bash
# Debug 或 Release 版本
./build/bin/chat_server
```

### 启动客户端

#### Windows
```cmd
# Debug 版本
.\build\bin\chat_client.exe

# Release 版本
.\build\bin\chat_client.exe
```

#### Linux
```bash
# Debug 或 Release 版本
./build/bin/chat_client
```

## 使用说明

1. 首先启动服务器，默认监听端口 9999
2. 启动一个或多个客户端连接到服务器
3. 在客户端输入消息，按回车发送
4. 输入 "quit" 退出客户端

## 配置

服务器配置文件位于 `config/server.properties`，可以修改端口和其他设置：

```properties
# 服务器配置
server.port=9999
server.host=0.0.0.0
server.maxConnections=100
```

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

### vcpkg 清单模式

项目使用 vcpkg 的清单模式（manifest mode），依赖项在 `vcpkg.json` 中声明：

```json
{
  "dependencies": [
    {
      "name": "poco",
      "version>=": "1.12.4"
    }
  ]
}
```

编译时会自动安装到项目本地，不会影响全局环境。

### 常见问题解决

#### Windows 平台
- 确保安装了 Visual Studio 2022 和 C++ 开发工具
- 设置正确的 `VCPKG_ROOT` 环境变量

#### Linux 平台
- 确保安装了必要的开发工具：
  ```bash
  # Ubuntu/Debian
  sudo apt install build-essential cmake git

  # CentOS/RHEL
  sudo yum groupinstall "Development Tools"
  sudo yum install cmake git
  ```

## 贡献

欢迎提交 Issue 和 Pull Request 来改进这个项目。

