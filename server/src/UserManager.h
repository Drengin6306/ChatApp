#pragma once
#include <string>
#include <map>
#include <mutex>
#include <vector>
#include <Poco/JSON/Object.h>
#include <random>

struct User
{
    std::string username;
    std::string userId;
    std::string passwordHash;
    bool isOnline;

    Poco::JSON::Object toJson() const;
    static User fromJson(const Poco::JSON::Object::Ptr &j);
};

class UserManager
{
public:
    static UserManager &getInstance();

    // 用户认证
    bool authenticateUser(const std::string &userId, const std::string &password);

    // 用户注册 - 返回生成的userId
    std::string registerUser(const std::string &username, const std::string &password);

    // 用户管理
    User getUserByUserId(const std::string &userId);
    bool setUserOnline(const std::string &userId, bool online);

    // 获取在线用户列表
    std::vector<std::string> getOnlineUsers();

    // 密码相关
    std::string hashPassword(const std::string &password);

private:
    UserManager();
    std::vector<User> users_;
    std::mutex usersMutex_;
    std::string usersFilePath_;

    // 随机数生成器用于生成随机种子
    std::mt19937_64 randomGenerator_;

    void loadUsersFromFile();
    void saveUsersToFile();
    std::string generateHashBasedUserId(const std::string &username);
    bool userIdExists(const std::string &userId);
    void initializeRandomGenerator();
};