#include "UserManager.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>
#include <random>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Stringifier.h>
#include <Poco/SHA2Engine.h>
#include <Poco/DigestEngine.h>
#include <Poco/Random.h>

UserManager::UserManager() : usersFilePath_("config/users.json")
{
    initializeRandomGenerator();
    loadUsersFromFile();
}

UserManager &UserManager::getInstance()
{
    static UserManager instance;
    return instance;
}

void UserManager::loadUsersFromFile()
{
    std::lock_guard<std::mutex> lock(usersMutex_);

    std::ifstream file(usersFilePath_);
    if (!file.is_open())
    {
        users_.clear();
        return;
    }

    try
    {
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(file);
        Poco::JSON::Object::Ptr json = result.extract<Poco::JSON::Object::Ptr>();

        if (json->has("users"))
        {
            Poco::JSON::Array::Ptr usersArray = json->getArray("users");
            for (const auto &userVar : *usersArray)
            {
                Poco::JSON::Object::Ptr userJson = userVar.extract<Poco::JSON::Object::Ptr>();
                User user = User::fromJson(userJson);
                users_.push_back(user);
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error loading users file: " << e.what() << std::endl;
        users_.clear();
    }
}

void UserManager::saveUsersToFile()
{
    std::lock_guard<std::mutex> lock(usersMutex_);

    Poco::JSON::Object j;
    Poco::JSON::Array::Ptr usersArray = new Poco::JSON::Array;

    for (const auto &user : users_)
    {
        usersArray->add(user.toJson());
    }

    j.set("users", usersArray);

    std::ofstream file(usersFilePath_);
    if (file.is_open())
    {
        try
        {
            std::ostringstream oss;
            j.stringify(oss, 2);
            file << oss.str();
            file.flush();
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error saving users file: " << e.what() << std::endl;
        }
    }
    else
    {
        std::cerr << "Failed to open users file for writing: " << usersFilePath_ << std::endl;
    }
}

void UserManager::initializeRandomGenerator()
{
    // 使用当前时间作为随机种子
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    randomGenerator_.seed(seed);
}

// 生成基于哈希的用户ID
std::string UserManager::generateHashBasedUserId(const std::string &username)
{
    std::string userId;
    int maxAttempts = 50; // 最多尝试50次
    int attempts = 0;

    do
    {
        auto now = std::chrono::high_resolution_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();

        uint64_t randomNum = randomGenerator_();

        // 组合输入：用户名 + 时间戳 + 随机数 + 尝试次数
        std::string input = username + std::to_string(timestamp) + std::to_string(randomNum) + std::to_string(attempts);

        Poco::SHA2Engine sha256;
        sha256.update(input);
        auto digest = sha256.digest();

        // 将哈希值转换为9位数字账号
        uint64_t hashValue = 0;
        for (size_t i = 0; i < 8 && i < digest.size(); ++i)
        {
            hashValue = (hashValue << 8) | digest[i];
        }

        // 映射到9位数范围 (100000000 - 999999999)
        uint64_t accountNumber = (hashValue % 900000000) + 100000000;
        userId = std::to_string(accountNumber);

        attempts++;

        if (attempts >= maxAttempts)
        {
            uint64_t extendedAccount = (hashValue % 9000000000ULL) + 1000000000ULL;
            userId = std::to_string(extendedAccount);
        }

    } while (userIdExists(userId));

    return userId;
}

bool UserManager::userIdExists(const std::string &userId)
{
    for (const auto &user : users_)
    {
        if (user.userId == userId)
        {
            return true;
        }
    }
    return false;
}

std::string UserManager::registerUser(const std::string &username, const std::string &password)
{
    std::lock_guard<std::mutex> lock(usersMutex_);

    // 验证用户名和密码格式
    if (username.empty() || password.length() < 6)
    {
        return "";
    }

    // 生成基于哈希的随机用户ID
    std::string userId = generateHashBasedUserId(username);

    if (userId.empty())
    {
        std::cerr << "Failed to generate unique user ID" << std::endl;
        return "";
    }

    // 创建新用户
    User newUser;
    newUser.username = username;
    newUser.userId = userId;
    newUser.passwordHash = hashPassword(password);
    newUser.isOnline = false;

    users_.push_back(newUser);
    saveUsersToFile();

    return userId;
}

bool UserManager::authenticateUser(const std::string &userId, const std::string &password)
{
    std::lock_guard<std::mutex> lock(usersMutex_);

    for (auto &user : users_)
    {
        if (user.userId == userId)
        {
            std::string hashedPassword = hashPassword(password);
            user.isOnline = true;
            return hashedPassword == user.passwordHash;
        }
    }

    return false;
}

std::vector<std::string> UserManager::getOnlineUsers()
{
    std::lock_guard<std::mutex> lock(usersMutex_);

    std::vector<std::string> onlineUsers;
    for (const auto &user : users_)
    {
        if (user.isOnline)
        {
            onlineUsers.push_back(user.userId);
        }
    }
    return onlineUsers;
}

User UserManager::getUserByUserId(const std::string &userId)
{
    std::lock_guard<std::mutex> lock(usersMutex_);

    for (const auto &user : users_)
    {
        if (user.userId == userId)
        {
            return user;
        }
    }

    return User{};
}

std::string UserManager::hashPassword(const std::string &password)
{
    Poco::SHA2Engine sha256;
    sha256.update(password);
    return Poco::DigestEngine::digestToHex(sha256.digest());
}

// User转换为JSON对象
Poco::JSON::Object User::toJson() const
{
    Poco::JSON::Object json;
    json.set("username", username);
    json.set("user_id", userId);
    json.set("password_hash", passwordHash);
    return json;
}

// 从JSON对象创建User实例
User User::fromJson(const Poco::JSON::Object::Ptr &j)
{
    User user;
    user.username = j->getValue<std::string>("username");
    user.userId = j->getValue<std::string>("user_id");
    user.passwordHash = j->getValue<std::string>("password_hash");
    return user;
}