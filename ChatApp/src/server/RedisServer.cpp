#include "serv_main.hpp"

RedisServer::RedisServer(const string &hostname, int port)
{
    context = redisConnect(hostname.c_str(), port);
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

// 群聊管理方法的实现
bool RedisServer::createGroup(const string &groupName)
{
    redisReply *reply;
    reply = (redisReply *)redisCommand(context, "SADD %s", groupName.c_str(), "");
    if (reply == nullptr || reply->type == REDIS_REPLY_ERROR)
    {
        cerr << "Error creating group: " << (reply ? reply->str : "unknown error") << endl;
        if (reply)
            freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    return true;
}

bool RedisServer::deleteGroup(const string &groupName)
{
    redisReply *reply;
    reply = (redisReply *)redisCommand(context, "DEL %s", groupName.c_str());
    if (reply == nullptr || reply->type == REDIS_REPLY_ERROR)
    {
        cerr << "Error deleting group: " << (reply ? reply->str : "unknown error") << endl;
        if (reply)
            freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    return true;
}

bool RedisServer::addMemberToGroup(const string &groupName, const string &memberName)
{
    redisReply *reply;
    reply = (redisReply *)redisCommand(context, "SADD %s %s", groupName.c_str(), memberName.c_str());
    if (reply == nullptr || reply->type == REDIS_REPLY_ERROR)
    {
        cerr << "Error adding member to group: " << (reply ? reply->str : "unknown error") << endl;
        if (reply)
            freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    return true;
}

bool RedisServer::removeMemberFromGroup(const string &groupName, const string &memberName)
{
    redisReply *reply;
    reply = (redisReply *)redisCommand(context, "SREM %s %s", groupName.c_str(), memberName.c_str());
    if (reply == nullptr || reply->type == REDIS_REPLY_ERROR)
    {
        cerr << "Error removing member from group: " << (reply ? reply->str : "unknown error") << endl;
        if (reply)
            freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    return true;
}

vector<string> RedisServer::getGroupMembers(const string &groupName)
{
    redisReply *reply;
    vector<string> members;
    reply = (redisReply *)redisCommand(context, "SMEMBERS %s", groupName.c_str());
    if (reply == nullptr || reply->type == REDIS_REPLY_ERROR)
    {
        cerr << "Error getting group members: " << (reply ? reply->str : "unknown error") << endl;
        if (reply)
            freeReplyObject(reply);
        return members;
    }
    for (size_t i = 0; i < reply->elements; ++i)
    {
        members.push_back(reply->element[i]->str);
    }
    freeReplyObject(reply);
    return members;
}

// 用户群聊列表管理方法的实现
bool RedisServer::addUserToGroupList(const string &username, const string &groupName)
{
    redisReply *reply;
    reply = (redisReply *)redisCommand(context, "SADD %s:%s", username.c_str(), groupName.c_str());
    if (reply == nullptr || reply->type == REDIS_REPLY_ERROR)
    {
        cerr << "Error adding user to group list: " << (reply ? reply->str : "unknown error") << endl;
        if (reply)
            freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    return true;
}

bool RedisServer::removeUserFromGroupList(const string &username, const string &groupName)
{
    redisReply *reply;
    reply = (redisReply *)redisCommand(context, "SREM %s:%s", username.c_str(), groupName.c_str());
    if (reply == nullptr || reply->type == REDIS_REPLY_ERROR)
    {
        cerr << "Error removing user from group list: " << (reply ? reply->str : "unknown error") << endl;
        if (reply)
            freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    return true;
}

vector<string> RedisServer::getUserGroupList(const string &username)
{
    redisReply *reply;
    vector<string> groups;
    reply = (redisReply *)redisCommand(context, "SMEMBERS %s:*", username.c_str());
    if (reply == nullptr || reply->type == REDIS_REPLY_ERROR)
    {
        cerr << "Error getting user group list: " << (reply ? reply->str : "unknown error") << endl;
        if (reply)
            freeReplyObject(reply);
        return groups;
    }
    for (size_t i = 0; i < reply->elements; ++i)
    {
        // 解析群聊名
        string groupName = reply->element[i]->str;
        size_t pos = groupName.find(':');
        if (pos != string::npos)
        {
            groups.push_back(groupName.substr(pos + 1));
        }
    }
    freeReplyObject(reply);
    return groups;
}