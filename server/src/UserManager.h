#pragma once
#include <string>
#include <map>
#include <unordered_map>
#include <mutex>
#include <vector>
#include <Poco/JSON/Object.h>
#include <random>

struct User
{
    std::string username;
    std::string account;
    std::string passwordHash;

    Poco::JSON::Object toJson() const;
    static User fromJson(const Poco::JSON::Object::Ptr &j);
};

class UserManager
{
public:
    static UserManager &getInstance();

    // 用户认证
    bool authenticateUser(const std::string &account, const std::string &password);

    // 用户注册 - 返回生成的account
    std::string registerUser(const std::string &username, const std::string &password);

    // 用户管理
    User getUserByAccount(const std::string &account);
    bool setUserStatus(const std::string &account, bool online);

    // 密码相关
    std::string hashPassword(const std::string &password);

private:
    UserManager();
    std::vector<User> users_;                              // 只存储在线用户
    std::unordered_map<std::string, size_t> accountIndex_; // account到users_索引的映射(仅在线用户)
    std::mutex usersMutex_;
    std::string usersFilePath_;

    // 随机数生成器用于生成随机种子
    std::mt19937_64 randomGenerator_;

    User loadUserFromFile(const std::string &account);
    std::vector<User> loadAllUsersFromFile();
    void appendUserToFile(const User &user);
    std::string generateHashBasedAccount(const std::string &username);
    bool accountExistsInFile(const std::string &account);
    User *findUserFromFile(const std::string &account);
    void initializeRandomGenerator();
    void rebuildAccountIndex();
    User *findUserByAccount(const std::string &account);
    void addUserToOnlineList(const User &user);
    void removeUserFromOnlineList(const std::string &account);
    bool accountExists(const std::string &account);
};