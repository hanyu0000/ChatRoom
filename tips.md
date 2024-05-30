# 项目整体规划
### 1.初步设置

选择和配置项目构建工具（CMake）。

设置项目目录结构。

配置日志系统（GLog）和单元测试框架（GTest）。

### 2.模块开发

账号管理模块：包括登录、注册、注销、找回密码、验证码验证等功能。

好友管理模块：包括好友添加、删除、查询、状态显示、屏蔽、聊天等功能。

群组管理模块：包括群组创建、解散、加入、退出、成员管理、聊天等功能。

聊天功能模块：包括私聊和群聊的消息记录、在线聊天、离线通知、文件传输等功能。
### 3.数据库设计和集成

配置和连接 Redis 数据库。

实现数据的序列化和反序列化（JSON/Proto）。
### 4.网络通信

使用 Epoll ET 模式实现 I/O 多路复用。

实现 TCP 心跳检测以保持连接稳定性。
用户文档和测试

### 5.编写用户文档和开发文档。
进行功能测试和压力测试，确保系统能够支持大量客户端同时访问。

## 详细任务分析
### 阶段一：初步设置
1.配置项目构建工具

创建 CMakeLists.txt 文件，设置项目基础配置。

配置编译选项和依赖库。

2.设置项目目录结构

创建目录结构：src/, include/, tests/, build/ 等。

确保源代码和头文件分开存放，便于管理和维护。

3.配置日志系统和单元测试框架

在 CMakeLists.txt 中添加 GLog 和 GTest 的配置。

创建基本的日志记录和测试框架。

### 阶段二：模块开发
1.账号管理模块

功能：登录、注册、注销、找回密码、验证码验证、数据加密。

任务：

设计 AccountManager 类及其接口。

实现登录、注册、注销、找回密码、验证码验证等功能。

实现密码加密（使用 SHA256 或其他加密算法）。

编写单元测试，测试每个功能。

2.好友管理模块

功能：好友添加、删除、查询、状态显示、屏蔽、聊天。

任务：

设计 FriendManager 类及其接口。

实现添加、删除、查询好友的功能。

实现好友在线状态显示和屏蔽功能。

实现好友间的聊天功能。

编写单元测试，测试每个功能。

3.群组管理模块

功能：群组创建、解散、加入、退出、成员管理、聊天。

任务：

设计 GroupManager 类及其接口。

实现群组的创建和解散。

实现用户申请加入、退出和查看群组。

实现群组成员的管理功能（添加、删除管理员）。

实现群组内的聊天功能。

编写单元测试，测试每个功能。

4.聊天功能模块

功能：私聊和群聊的消息记录、在线聊天、离线通知、文件传输。

任务：

设计 ChatManager 类及其接口。

实现消息记录查看功能。

实现用户间的在线聊天功能。

实现离线消息通知功能。

实现文件传输和断点续传功能。

编写单元测试，测试每个功能。

### 阶段三：数据库设计和集成
1.配置和连接 Redis 数据库

安装和配置 Redis。

编写代码连接 Redis 数据库。

2.实现数据的序列化和反序列化

选择序列化工具（JSON 或 Proto）。

编写序列化和反序列化代码，确保数据能够正确存储和读取。

### 阶段四：网络通信
1.实现 I/O 多路复用（Epoll ET 模式）

设计网络通信的基础结构。

使用 Epoll ET 模式实现高效的 I/O 多路复用。

2.实现 TCP 心跳检测

设计心跳包机制，定期发送心跳包以保持连接。

实现心跳包的发送和接收逻辑，检测连接是否断开。

### 阶段五：用户文档和测试
1.编写用户文档和开发文档

编写详细的用户文档，说明系统的使用方法。

编写开发文档，记录设计思路和实现细节。

2.进行功能测试和压力测试

编写自动化测试脚本，测试各个功能模块。

进行压力测试，确保系统能够支持大量客户端同时访问。

## 具体实施计划
第1周：

配置项目构建工具和目录结构。

设置日志系统和单元测试框架。

第2-3周：

开发账号管理模块，并编写单元测试。

完成账号管理模块的测试和调试。

第4-5周：

开发好友管理模块，并编写单元测试。

完成好友管理模块的测试和调试。

第6-7周：

开发群组管理模块，并编写单元测试。

完成群组管理模块的测试和调试。

第8-9周：

开发聊天功能模块，并编写单元测试。

完成聊天功能模块的测试和调试。

第10周：

配置和连接 Redis 数据库。

实现数据的序列化和反序列化。

第11周：

实现 I/O 多路复用（Epoll ET 模式）。

实现 TCP 心跳检测。

第12周：

编写用户文档和开发文档。

进行功能测试和压力测试。

## 详细实施步骤
### 设置项目构建工具
CMakeLists.txt：
```cmake
cmake_minimum_required(VERSION 3.10)
project(ChatApp)

set(CMAKE_CXX_STANDARD 17)

# 添加源文件
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")

# 添加可执行文件
add_executable(ChatApp ${SOURCES} ${HEADERS})

# 链接库
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
target_link_libraries(ChatApp ${OPENSSL_LIBRARIES})

# 日志库
find_package(glog REQUIRED)
include_directories(${GLOG_INCLUDE_DIRS})
target_link_libraries(ChatApp ${GLOG_LIBRARIES})

# 添加测试
enable_testing()
add_subdirectory(tests)
```
### 账号管理模块
AccountManager.h：

```cpp
#pragma once
#include <string>

class AccountManager {
public:
    bool login(const std::string& username, const std::string& password);
    bool registerUser(const std::string& username, const std::string& password, const std::string& email);
    bool logout(const std::string& username);
    bool recoverPassword(const std::string& email);
    bool verifyCaptcha(const std::string& captcha);

private:
    std::string encryptPassword(const std::string& password);
};
```
AccountManager.cpp：

```cpp
#include "AccountManager.h"
#include <iostream>
#include <openssl/sha.h> // For password encryption

bool AccountManager::login(const std::string& username, const std::string& password) {
    // 实现登录逻辑
    std::string encryptedPassword = encryptPassword(password);
    // 验证用户名和加密密码
    return true;
}

bool AccountManager::registerUser(const std::string& username, const std::string& password, const std::string& email) {
    // 实现注册逻辑
    std::string encryptedPassword = encryptPassword(password);
    // 保存用户数据到数据库
    return true;
}

bool AccountManager::logout(const std::string& username) {
    // 实现注销逻辑
    return true;
}

bool AccountManager::recoverPassword(const std::string& email) {
    // 实现找回密码逻辑
    return true;
}

bool AccountManager::verifyCaptcha(const std::string& captcha) {
    // 实现验证码验证逻辑
    return true;
}

std::string AccountManager::encryptPassword(const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, password.c_str(), password.size());
    SHA256_Final(hash, &sha256);
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}
```
### 好友管理模块
FriendManager.h：

```cpp
#pragma once
#include <string>
#include <vector>

class FriendManager {
public:
    bool addFriend(const std::string& user, const std::string& friendUser);
    bool removeFriend(const std::string& user, const std::string& friendUser);
    std::vector<std::string> listFriends(const std::string& user);
    bool blockFriend(const std::string& user, const std::string& friendUser);
    bool sendMessage(const std::string& user, const std::string& friendUser, const std::string& message);
};
```
FriendManager.cpp：

```cpp
#include "FriendManager.h"
#include <iostream>
#include <vector>

bool FriendManager::addFriend(const std::string& user, const std::string& friendUser) {
    // 实现添加好友逻辑
    return true;
}

bool FriendManager::removeFriend(const std::string& user, const std::string& friendUser) {
    // 实现删除好友逻辑
    return true;
}

std::vector<std::string> FriendManager::listFriends(const std::string& user) {
    // 实现列出好友逻辑
    return std::vector<std::string>();
}

bool FriendManager::blockFriend(const std::string& user, const std::string& friendUser) {
    // 实现屏蔽好友逻辑
    return true;
}

bool FriendManager::sendMessage(const std::string& user, const std::string& friendUser, const std::string& message) {
    // 实现发送消息逻辑
    return true;
}
```
通过以上的详细计划和任务分析，你可以按步骤实施每个阶段的任务。希望这些信息能帮助你清晰地了解项目需求和实现路径。





