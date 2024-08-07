#include "task.hpp"

RedisServer::RedisServer()
{
    context = redisConnect("127.0.0.1", 6379);
    if (context == nullptr || context->err)
    {
        cerr << "Redis 连接失败" << endl;
        if (context)
            redisFree(context);
        throw runtime_error("Redis 连接失败");
    }
    cout << "redis连接成功" << endl;
}

RedisServer::~RedisServer()
{
    if (context)
        redisFree(context);
}

bool RedisServer::setPassword(const string &username, const string &password)
{
    redisReply *reply = (redisReply *)redisCommand(context, "SET %s %s", username.c_str(), password.c_str());
    if (reply == nullptr)
    {
        cerr << "Redis SET 命令失败" << endl;
        return false;
    }

    bool success = (reply->type == REDIS_REPLY_STATUS && strcmp(reply->str, "OK") == 0);
    freeReplyObject(reply);
    return success;
}

bool RedisServer::isUser(const string &username, const string &password)
{
    redisReply *reply = (redisReply *)redisCommand(context, "GET %s", username.c_str());
    if (reply == nullptr)
    {
        cerr << "Redis GET 命令失败" << endl;
        return false;
    }

    bool registered = false;
    if (reply->type == REDIS_REPLY_STRING && password == reply->str)
        registered = true;

    freeReplyObject(reply);
    return registered;
}

bool RedisServer::friends_exit(const string &username)
{
    redisReply *reply = (redisReply *)redisCommand(context, "EXISTS %s", username.c_str());
    if (reply == nullptr)
    {
        cerr << "Redis EXISTS 命令失败" << endl;
        return false;
    }

    bool registered = false;
    if (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1)
        registered = true;

    freeReplyObject(reply);
    return registered;
}

bool RedisServer::deleteUser(const string &username)
{
    redisReply *reply = (redisReply *)redisCommand(context, "DEL %s", username.c_str());
    if (reply == nullptr)
    {
        cerr << "Redis DEL 命令失败" << endl;
        return false;
    }
    bool success = (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1);
    freeReplyObject(reply);
    return success;
}

bool RedisServer::addFriend(const string &username, const string &friendname)
{
    redisReply *reply = (redisReply *)redisCommand(context, "SADD friends:%s %s", username.c_str(), friendname.c_str());
    bool success = reply->integer == 1;
    freeReplyObject(reply);
    return success;
}
bool RedisServer::isFriend(const string &username, const string &friendname)
{
    redisReply *reply = (redisReply *)redisCommand(context, "SISMEMBER friends:%s %s", username.c_str(), friendname.c_str());
    bool is_friend = reply->integer == 1;
    freeReplyObject(reply);
    return is_friend;
}
vector<string> RedisServer::getFriends(const string &username)
{
    redisReply *reply = (redisReply *)redisCommand(context, "SMEMBERS friends:%s", username.c_str());
    vector<string> friends;
    if (reply->type == REDIS_REPLY_ARRAY)
    {
        for (size_t i = 0; i < reply->elements; i++)
            friends.push_back(reply->element[i]->str);
    }
    freeReplyObject(reply);
    return friends;
}
bool RedisServer::removeFriend(const string &username, const string &friendname)
{
    redisReply *reply = (redisReply *)redisCommand(context, "SREM friends:%s %s", username.c_str(), friendname.c_str());
    bool success = reply->integer == 1;
    freeReplyObject(reply);
    return success;
}
bool RedisServer::blockUser(const string &username, const string &blockname)
{
    redisReply *reply = (redisReply *)redisCommand(context, "SADD blocked:%s %s", username.c_str(), blockname.c_str());
    bool success = reply->integer == 1;
    freeReplyObject(reply);
    return success;
}

bool RedisServer::isBlocked(const string &username, const string &blockname)
{
    redisReply *reply = (redisReply *)redisCommand(context, "SISMEMBER blocked:%s %s", username.c_str(), blockname.c_str());
    bool is_blocked = reply->integer == 1;
    freeReplyObject(reply);
    return is_blocked;
}

vector<string> RedisServer::getBlockedUsers(const string &username)
{
    redisReply *reply = (redisReply *)redisCommand(context, "SMEMBERS blocked:%s", username.c_str());
    vector<string> blockedUsers;
    if (reply->type == REDIS_REPLY_ARRAY)
    {
        for (size_t i = 0; i < reply->elements; i++)
            blockedUsers.push_back(reply->element[i]->str);
    }
    freeReplyObject(reply);
    return blockedUsers;
}

bool RedisServer::unblockUser(const string &username, const string &blockname)
{
    redisReply *reply = (redisReply *)redisCommand(context, "SREM blocked:%s %s", username.c_str(), blockname.c_str());
    bool success = reply->integer == 1;
    freeReplyObject(reply);
    return success;
}

// 创建群聊
bool RedisServer::createGroup(const string &groupName)
{
    redisReply *reply = (redisReply *)redisCommand(context, "SADD groups %s", groupName.c_str());
    bool success = (reply->integer == 1 || reply->integer == 0);
    freeReplyObject(reply);
    return success;
}

bool RedisServer::addMemberToGroup(const string &groupName, const string &memberName)
{
    redisReply *reply = (redisReply *)redisCommand(context, "SADD %s %s", groupName.c_str(), memberName.c_str());
    bool success = (reply->integer == 1 || reply->integer == 0);
    freeReplyObject(reply);
    return success;
}

bool RedisServer::removeMemberFromGroup(const string &groupName, const string &memberName)
{
    redisReply *reply = (redisReply *)redisCommand(context, "SREM %s %s", groupName.c_str(), memberName.c_str());
    bool success = (reply->integer == 1 || reply->integer == 0); // 1 表示成员移除成功，0 表示成员不存在
    freeReplyObject(reply);
    return success;
}

vector<string> RedisServer::getGroupMembers(const string &groupName)
{
    vector<string> members;
    redisReply *reply = (redisReply *)redisCommand(context, "SMEMBERS %s", groupName.c_str());
    if (reply->type == REDIS_REPLY_ARRAY)
    {
        for (size_t i = 0; i < reply->elements; ++i)
        {
            members.push_back(reply->element[i]->str);
        }
    }
    freeReplyObject(reply);
    return members;
}

bool RedisServer::addUserToGroupList(const string &username, const string &groupName)
{
    redisReply *reply = (redisReply *)redisCommand(context, "SADD user_groups:%s %s", username.c_str(), groupName.c_str());
    bool success = (reply->integer == 1 || reply->integer == 0); // 1 表示用户添加成功，0 表示用户已在列表中
    freeReplyObject(reply);
    return success;
}

bool RedisServer::removeUserFromGroupList(const string &username, const string &groupName)
{
    redisReply *reply = (redisReply *)redisCommand(context, "SREM user_groups:%s %s", username.c_str(), groupName.c_str());
    bool success = (reply->integer == 1 || reply->integer == 0); // 1 表示群聊移除成功，0 表示群聊不在列表中
    freeReplyObject(reply);
    return success;
}

vector<string> RedisServer::getUserGroupList(const string &username)
{
    vector<string> groups;
    redisReply *reply = (redisReply *)redisCommand(context, "SMEMBERS user_groups:%s", username.c_str());
    if (reply->type == REDIS_REPLY_ARRAY)
    {
        for (size_t i = 0; i < reply->elements; ++i)
        {
            groups.push_back(reply->element[i]->str);
        }
    }
    freeReplyObject(reply);
    return groups;
}

// 群主加管理员
bool RedisServer::addAdminToGroup(const string &groupName, const string &adminName)
{
    redisReply *reply = (redisReply *)redisCommand(context, "SADD %s_admins %s", groupName.c_str(), adminName.c_str());
    if (!reply)
        return false;
    bool success = (reply->integer == 1 || reply->integer == 0);
    freeReplyObject(reply);
    return success;
}
// 群主删管理员
bool RedisServer::removeAdminFromGroup(const string &groupName, const string &adminName)
{
    redisReply *reply = (redisReply *)redisCommand(context, "SREM %s_admins %s", groupName.c_str(), adminName.c_str());
    if (!reply)
        return false;
    bool success = (reply->integer == 1 || reply->integer == 0);
    freeReplyObject(reply);
    return success;
}
// 获取管理员列表
vector<string> RedisServer::getManagers(const string &groupName)
{
    vector<string> managers;
    redisReply *reply = (redisReply *)redisCommand(context, "SMEMBERS %s_managers", groupName.c_str());
    if (reply->type == REDIS_REPLY_ARRAY)
    {
        for (size_t i = 0; i < reply->elements; ++i)
        {
            managers.push_back(reply->element[i]->str);
        }
    }
    freeReplyObject(reply);
    return managers;
}

// 获取群主和管理员列表
vector<string> RedisServer::getAdmins(const string &groupName)
{
    vector<string> admins;
    redisReply *reply = (redisReply *)redisCommand(context, "SMEMBERS %s_admins", groupName.c_str());
    if (reply->type == REDIS_REPLY_ARRAY)
    {
        for (size_t i = 0; i < reply->elements; ++i)
            admins.push_back(reply->element[i]->str);
    }
    freeReplyObject(reply);
    return admins;
}

// 获取普通用户列表
vector<string> RedisServer::getUsers(const string &groupName)
{
    vector<string> users;
    redisReply *reply = (redisReply *)redisCommand(context, "SMEMBERS %s_users", groupName.c_str());
    if (reply->type == REDIS_REPLY_ARRAY)
    {
        for (size_t i = 0; i < reply->elements; ++i)
            users.push_back(reply->element[i]->str);
    }
    freeReplyObject(reply);
    return users;
}

// 将用户添加为群聊的创建者
bool RedisServer::addMaster(const string &groupName, const string &memberName)
{
    redisReply *reply = (redisReply *)redisCommand(context, "SADD %s_master %s", memberName.c_str(), groupName.c_str());
    bool success = (reply->integer == 1 || reply->integer == 0); // 1 表示成功添加，0 表示已经存在
    freeReplyObject(reply);
    return success;
}

// 获取用户创建的群聊列表
vector<string> RedisServer::getGroupsByMaster(const string &memberName)
{
    vector<string> groups;
    redisReply *reply = (redisReply *)redisCommand(context, "SMEMBERS %s_master", memberName.c_str());
    if (reply->type == REDIS_REPLY_ARRAY)
    {
        for (size_t i = 0; i < reply->elements; ++i)
            groups.push_back(reply->element[i]->str);
    }
    freeReplyObject(reply);
    return groups;
}

bool RedisServer::deleteGroup(const std::string &groupName, const std::string &masterName)
{
    // 删除群聊
    redisReply *reply = (redisReply *)redisCommand(context, "DEL %s", groupName.c_str());
    bool success = (reply->type == REDIS_REPLY_INTEGER && reply->integer > 0); // DEL 命令返回删除的键的数量
    freeReplyObject(reply);

    // 如果群聊删除成功，更新创建者的群聊列表
    if (success)
    {
        redisReply *updateReply = (redisReply *)redisCommand(context, "SREM %s_master %s", masterName.c_str(), groupName.c_str());
        bool updateSuccess = (updateReply->integer == 1); // 1 表示移除成功，0 表示群聊不在列表中
        freeReplyObject(updateReply);
        return updateSuccess;
    }
    return false;
}