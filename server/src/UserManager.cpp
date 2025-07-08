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
    users_.clear();
    accountIndex_.clear();
}

UserManager &UserManager::getInstance()
{
    static UserManager instance;
    return instance;
}

User UserManager::loadUserFromFile(const std::string &account)
{
    std::ifstream file(usersFilePath_);
    if (!file.is_open())
    {
        return User{};
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
                if (user.account == account)
                {
                    return user;
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error loading user from file: " << e.what() << std::endl;
    }

    return User{};
}

std::vector<User> UserManager::loadAllUsersFromFile()
{
    std::vector<User> allUsers;

    std::ifstream file(usersFilePath_);
    if (!file.is_open())
    {
        return allUsers;
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
                allUsers.push_back(user);
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error loading users file: " << e.what() << std::endl;
    }

    return allUsers;
}

void UserManager::appendUserToFile(const User &user)
{
    Poco::JSON::Object j;
    Poco::JSON::Array::Ptr usersArray;

    // 尝试读取现有文件
    std::ifstream inFile(usersFilePath_);
    if (inFile.is_open())
    {
        try
        {
            Poco::JSON::Parser parser;
            Poco::Dynamic::Var result = parser.parse(inFile);
            Poco::JSON::Object::Ptr existingJson = result.extract<Poco::JSON::Object::Ptr>();

            if (existingJson->has("users"))
            {
                usersArray = existingJson->getArray("users");
            }
            else
            {
                usersArray = new Poco::JSON::Array;
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error reading existing users file: " << e.what() << std::endl;
            usersArray = new Poco::JSON::Array;
        }
        inFile.close();
    }
    else
    {
        usersArray = new Poco::JSON::Array;
    }

    // 添加新用户
    usersArray->add(user.toJson());
    j.set("users", usersArray);

    // 写入文件
    std::ofstream outFile(usersFilePath_);
    if (outFile.is_open())
    {
        try
        {
            std::ostringstream oss;
            j.stringify(oss, 2);
            outFile << oss.str();
            outFile.flush();
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error saving user to file: " << e.what() << std::endl;
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
std::string UserManager::generateHashBasedAccount(const std::string &username)
{
    std::string account;
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

        // 显示映射到9位数范围 (100000000 - 999999999)
        uint64_t accountNumber = (hashValue % 900000000) + 100000000;
        account = std::to_string(accountNumber);

        attempts++;

        if (attempts >= maxAttempts)
        {
            uint64_t extendedAccount = (hashValue % 9000000000ULL) + 1000000000ULL;
            account = std::to_string(extendedAccount);
        }

    } while (accountExists(account));

    return account;
}

void UserManager::rebuildAccountIndex()
{
    accountIndex_.clear();
    for (size_t i = 0; i < users_.size(); ++i)
    {
        accountIndex_[users_[i].account] = i;
    }
}

User *UserManager::findUserByAccount(const std::string &account)
{
    auto it = accountIndex_.find(account);
    if (it != accountIndex_.end() && it->second < users_.size())
    {
        return &users_[it->second];
    }
    return nullptr;
}

bool UserManager::accountExists(const std::string &account)
{
    return accountExistsInFile(account);
}

bool UserManager::accountExistsInFile(const std::string &account)
{
    User user = loadUserFromFile(account);
    return !user.account.empty();
}

User *UserManager::findUserFromFile(const std::string &account)
{
    static User foundUser = loadUserFromFile(account);
    if (!foundUser.account.empty())
    {
        return &foundUser;
    }
    return nullptr;
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
    std::string account = generateHashBasedAccount(username);

    if (account.empty())
    {
        std::cerr << "Failed to generate unique user ID" << std::endl;
        return "";
    }

    // 创建新用户
    User newUser;
    newUser.username = username;
    newUser.account = account;
    newUser.passwordHash = hashPassword(password);

    // 向文件追加新用户
    appendUserToFile(newUser);

    return account;
}

bool UserManager::authenticateUser(const std::string &account, const std::string &password)
{
    std::lock_guard<std::mutex> lock(usersMutex_);

    // 从文件中加载单个用户
    User user = loadUserFromFile(account);
    if (!user.account.empty())
    {
        std::string hashedPassword = hashPassword(password);
        if (hashedPassword == user.passwordHash)
        {
            // 验证成功，添加到在线用户列表
            addUserToOnlineList(user);
            return true;
        }
    }

    return false;
}

void UserManager::addUserToOnlineList(const User &user)
{
    accountIndex_[user.account] = users_.size();
    users_.push_back(user);
}

void UserManager::removeUserFromOnlineList(const std::string &account)
{
    auto it = accountIndex_.find(account);
    if (it != accountIndex_.end())
    {
        size_t index = it->second;

        if (index < users_.size())
        {
            accountIndex_.erase(it);

            if (index != users_.size() - 1)
            {
                users_[index] = users_.back();
                accountIndex_[users_[index].account] = index;
            }
            users_.pop_back();
        }
        else
        {
            accountIndex_.erase(it);
        }
    }
}

User UserManager::getUserByAccount(const std::string &account)
{
    std::lock_guard<std::mutex> lock(usersMutex_);

    // 首先在在线用户中查找
    User *user = findUserByAccount(account);
    if (user)
    {
        return *user;
    }

    // 如果不在线，从文件中加载
    return loadUserFromFile(account);
}

bool UserManager::setUserStatus(const std::string &account, bool online)
{
    std::lock_guard<std::mutex> lock(usersMutex_);

    if (online)
    {
        User user = loadUserFromFile(account);
        if (!user.account.empty())
        {
            // 检查是否已经在在线列表中
            if (findUserByAccount(account) == nullptr)
            {
                addUserToOnlineList(user);
            }
            return true;
        }
    }
    else
    {
        // 用户下线：从在线列表中移除
        User *user = findUserByAccount(account);
        if (user)
        {
            removeUserFromOnlineList(account);
            return true;
        }
    }

    return false;
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
    json.set("account", account);
    json.set("password_hash", passwordHash);
    return json;
}

// 从JSON对象创建User实例
User User::fromJson(const Poco::JSON::Object::Ptr &j)
{
    User user;
    user.username = j->getValue<std::string>("username");
    user.account = j->getValue<std::string>("account");
    user.passwordHash = j->getValue<std::string>("password_hash");
    return user;
}