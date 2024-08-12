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

// 存储用户名-密码
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
// 是否注册
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
// 加好友判断用户名是否存在
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
// 删除用户
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
// 存储好友关系
bool RedisServer::addFriend(const string &username, const string &friendname)
{
    redisReply *reply = (redisReply *)redisCommand(context, "SADD friends:%s %s", username.c_str(), friendname.c_str());
    bool success = reply->integer == 1;
    freeReplyObject(reply);
    return success;
}
// 好友列表
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
// 清空用户的好友列表
bool RedisServer::removeAllFriends(const string &username)
{
    // 获取用户的所有好友
    vector<string> friends = getFriends(username);
    if (friends.empty())
        return true; // 如果没有好友，返回成功
    // 从 Redis 中删除所有好友
    redisReply *reply = (redisReply *)redisCommand(context, "DEL friends:%s", username.c_str());
    bool success = reply->integer == 1;
    freeReplyObject(reply);
    return success;
}
// 删除好友
bool RedisServer::removeFriend(const string &username, const string &friendname)
{
    redisReply *reply = (redisReply *)redisCommand(context, "SREM friends:%s %s", username.c_str(), friendname.c_str());
    bool success = reply->integer == 1;
    freeReplyObject(reply);
    return success;
}
// 屏蔽好友
bool RedisServer::blockUser(const string &username, const string &blockname)
{
    redisReply *reply = (redisReply *)redisCommand(context, "SADD blocked:%s %s", username.c_str(), blockname.c_str());
    bool success = reply->integer == 1;
    freeReplyObject(reply);
    return success;
}
// 判断用户是否被另一个用户屏蔽
bool RedisServer::isUserBlocked(const string &username, const string &blockname)
{
    redisReply *reply = (redisReply *)redisCommand(context, "SISMEMBER blocked:%s %s", blockname.c_str(), username.c_str());
    bool isBlocked = reply->integer == 1;
    freeReplyObject(reply);
    return isBlocked;
}
// 取消屏蔽好友
bool RedisServer::unblockUser(const string &username, const string &blockname)
{
    redisReply *reply = (redisReply *)redisCommand(context, "SREM blocked:%s %s", username.c_str(), blockname.c_str());
    bool success = reply->integer == 1;
    freeReplyObject(reply);
    return success;
}
// 屏蔽好友列表
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
// 清空用户的屏蔽好友列表
bool RedisServer::removeAllBlockedUsers(const string &username)
{
    // 使用 DEL 命令删除屏蔽好友列表
    redisReply *reply = (redisReply *)redisCommand(context, "DEL blocked:%s", username.c_str());
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
// 删除群聊
bool RedisServer::deleteGroup(const string &groupName, const string &masterName)
{
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
// 存储群聊
void RedisServer::mycreateGroup(const string &groupName, const string &creator)
{
    // 将群聊名称存储在 Redis 中，使用集合来存储用户创建的群聊名称
    redisReply *reply = (redisReply *)redisCommand(context, "SADD user_groups:%s %s", creator.c_str(), groupName.c_str());
    if (reply == nullptr || reply->type == REDIS_REPLY_ERROR)
    {
        cerr << "Failed to create group " << groupName << endl;
        if (reply)
            freeReplyObject(reply);
        throw runtime_error("Failed to create group");
    }
    freeReplyObject(reply);
    cout << "Group " << groupName << " created successfully by " << creator << endl;
}
// 查询群聊
vector<string> RedisServer::getGroupsByUser(const string &username)
{
    vector<string> groups;
    redisReply *reply = (redisReply *)redisCommand(context, "SMEMBERS user_groups:%s", username.c_str());
    if (reply->type == REDIS_REPLY_ARRAY)
    {
        for (size_t i = 0; i < reply->elements; ++i)
            groups.push_back(reply->element[i]->str);
    }
    freeReplyObject(reply);
    return groups;
}
// 加群成员
bool RedisServer::addMemberToGroup(const string &groupName, const string &memberName)
{
    redisReply *reply = (redisReply *)redisCommand(context, "SADD %s %s", groupName.c_str(), memberName.c_str());
    bool success = (reply->integer == 1 || reply->integer == 0);
    freeReplyObject(reply);
    return success;
}
// 移除群成员
bool RedisServer::removeMemberFromGroup(const string &groupName, const string &memberName)
{
    redisReply *reply = (redisReply *)redisCommand(context, "SREM %s %s", groupName.c_str(), memberName.c_str());
    bool success = (reply->integer == 1 || reply->integer == 0); // 1 表示成员移除成功，0 表示成员不存在
    freeReplyObject(reply);
    return success;
}
// 获取群成员
vector<string> RedisServer::getGroupMembers(const string &groupName)
{
    vector<string> members;
    redisReply *reply = (redisReply *)redisCommand(context, "SMEMBERS %s", groupName.c_str());
    if (reply->type == REDIS_REPLY_ARRAY)
    {
        for (size_t i = 0; i < reply->elements; ++i)
            members.push_back(reply->element[i]->str);
    }
    freeReplyObject(reply);
    return members;
}

// 用户-群列表
bool RedisServer::addUserToGroupList(const string &username, const string &groupName)
{
    redisReply *reply = (redisReply *)redisCommand(context, "SADD user_groups:%s %s", username.c_str(), groupName.c_str());
    bool success = (reply->integer == 1 || reply->integer == 0); // 1 表示用户添加成功，0 表示用户已在列表中
    freeReplyObject(reply);
    return success;
}
// 删
bool RedisServer::removeUserFromGroupList(const string &username, const string &groupName)
{
    redisReply *reply = (redisReply *)redisCommand(context, "SREM user_groups:%s %s", username.c_str(), groupName.c_str());
    bool success = (reply->integer == 1 || reply->integer == 0); // 1 表示群聊移除成功，0 表示群聊不在列表中
    freeReplyObject(reply);
    return success;
}
// 获取用户的群列表
vector<string> RedisServer::getUserGroupList(const string &username)
{
    vector<string> groups;
    redisReply *reply = (redisReply *)redisCommand(context, "SMEMBERS user_groups:%s", username.c_str());
    if (reply->type == REDIS_REPLY_ARRAY)
    {
        for (size_t i = 0; i < reply->elements; ++i)
            groups.push_back(reply->element[i]->str);
    }
    freeReplyObject(reply);
    return groups;
}
// 删除用户的所有群列表
void RedisServer::removeUserFromAllGroups(const string &username)
{
    vector<string> groups = getUserGroupList(username);
    // 遍历每个群并将用户从群中删除
    for (const string &group : groups)
    {
        redisReply *reply = (redisReply *)redisCommand(context, "SREM user_groups:%s %s", group.c_str(), username.c_str());
        if (reply->type != REDIS_REPLY_INTEGER || reply->integer != 1)
            cerr << "Failed to remove user " << username << " from group " << group << endl;
        freeReplyObject(reply);
    }
}
// 删除用户的群组列表
bool RedisServer::removeUserGroupList(const string &username)
{
    // 使用 DEL 命令删除用户的群组列表
    redisReply *reply = (redisReply *)redisCommand(context, "DEL user_groups:%s", username.c_str());
    bool success = reply->integer == 1;
    freeReplyObject(reply);
    return success;
}
// 获取管理员列表
vector<string> RedisServer::getManagers(const string &groupName)
{
    vector<string> managers;
    string key = groupName + "_admins"; // 确保与存入时的键名一致
    redisReply *reply = (redisReply *)redisCommand(context, "SMEMBERS %s", key.c_str());
    // 检查 reply 对象和错误
    if (reply == nullptr || reply->type != REDIS_REPLY_ARRAY)
    {
        cerr << "获取 " << groupName << " 管理员列表失败: ";
        if (context->err)
            cerr << context->errstr << endl;
        else
            cerr << "未知错误" << endl;
        if (reply)
            freeReplyObject(reply);
        throw runtime_error("获取管理员列表失败");
    }
    for (size_t i = 0; i < reply->elements; ++i)
        if (reply->element[i]->str) // 确保元素不为空
            managers.push_back(reply->element[i]->str);

    freeReplyObject(reply);
    return managers;
}
bool RedisServer::isGroupManager(const string &groupName, const string &username)
{
    vector<string> managers = getManagers(groupName);
    // 检查用户是否在管理员列表中
    return find(managers.begin(), managers.end(), username) != managers.end();
}

// 群主加管理员
void RedisServer::addAdminToGroup(const string &groupName, const string &adminName)
{
    redisReply *reply = (redisReply *)redisCommand(context, "SADD %s_admins %s", groupName.c_str(), adminName.c_str());
    if (reply == nullptr)
    {
        cerr << "Redis 命令执行失败" << endl;
        return;
    }
    if (reply->integer != 1 && reply->integer != 0)
        cerr << "群聊管理员添加失败: " << adminName << endl;
    else
        cout << "群聊管理员已添加: " << adminName << endl;
    freeReplyObject(reply);
}
// 群主删管理员
void RedisServer::removeAdminFromGroup(const string &groupName, const string &adminName)
{
    redisReply *reply = (redisReply *)redisCommand(context, "SREM %s_admins %s", groupName.c_str(), adminName.c_str());
    if (reply == nullptr)
    {
        cerr << "Redis 命令执行失败" << endl;
        return;
    }
    if (reply->integer != 1 && reply->integer != 0)
        cerr << "群聊管理员删除失败: " << adminName << endl;
    else
        cout << "群聊管理员已删除: " << adminName << endl;
    freeReplyObject(reply);
}
// 删除用户创建的所有群聊
bool RedisServer::removeGroupsByMaster(const string &username)
{
    vector<string> groups = getGroupsByUser(username);
    for (const string &group : groups)
    {
        redisReply *reply = (redisReply *)redisCommand(context, "SREM user_groups:%s %s", username.c_str(), group.c_str());
        if (reply == nullptr || reply->type == REDIS_REPLY_ERROR)
        {
            cerr << "Failed to remove group " << group << " for user " << username << endl;
            if (reply)
                freeReplyObject(reply);
            throw runtime_error("Failed to remove group");
        }
        freeReplyObject(reply);
    }
    return 1;
}
// 存储聊天记录
void RedisServer::storeChatRecord(const string &user1, const string &user2, const string &message)
{
    // 确保 user1 和 user2 的顺序不影响聊天记录
    string key1 = "chat:" + user1 + ":" + user2;
    string key2 = "chat:" + user2 + ":" + user1;
    // 存储消息到两个键中，以确保不考虑顺序
    redisReply *reply1 = (redisReply *)redisCommand(context, "RPUSH %s %s", key1.c_str(), message.c_str());
    if (reply1 == nullptr)
    {
        cerr << "Redis 命令失败: RPUSH" << endl;
        return;
    }
    freeReplyObject(reply1);

    redisReply *reply2 = (redisReply *)redisCommand(context, "RPUSH %s %s", key2.c_str(), message.c_str());
    if (reply2 == nullptr)
    {
        cerr << "Redis 命令失败: RPUSH" << endl;
        return;
    }
    freeReplyObject(reply2);
}
// 获取聊天记录
vector<string> RedisServer::getChatRecord(const string &user1, const string &user2)
{
    vector<string> messages;
    string key = "chat:" + user1 + ":" + user2;
    //  从 Redis 中获取消息
    redisReply *reply1 = (redisReply *)redisCommand(context, "LRANGE %s 0 -1", key.c_str());
    if (reply1 != nullptr)
    {
        for (size_t i = 0; i < reply1->elements; ++i)
            messages.push_back(reply1->element[i]->str);
        freeReplyObject(reply1);
    }
    else
        cerr << "Redis 命令失败: LRANGE " << key << endl;
    return messages;
}
// 删除用户创建的所有聊天记录
bool RedisServer::removeChatRecordsByUser(const string &username)
{
    // 获取用户参与的所有聊天对
    vector<string> chatPartners = getChatPartners(username); // 需要实现 getChatPartners 方法
    if (chatPartners.empty())
        return true; // 如果没有聊天对，返回成功
    // 删除每个聊天对的记录
    for (const string &partner : chatPartners)
    {
        string key1 = "chat:" + username + ":" + partner;
        string key2 = "chat:" + partner + ":" + username;

        // 删除聊天记录
        redisReply *reply1 = (redisReply *)redisCommand(context, "DEL %s", key1.c_str());
        if (reply1 == nullptr)
        {
            cerr << "Redis 命令失败: DEL" << endl;
            return false; // 如果删除失败，返回失败
        }
        freeReplyObject(reply1);

        redisReply *reply2 = (redisReply *)redisCommand(context, "DEL %s", key2.c_str());
        if (reply2 == nullptr)
        {
            cerr << "Redis 命令失败: DEL" << endl;
            return false; // 如果删除失败，返回失败
        }
        freeReplyObject(reply2);
    }
    return true;
}
// 获取用户参与的所有聊天对
vector<string> RedisServer::getChatPartners(const string &username)
{
    vector<string> partners;
    redisReply *reply = (redisReply *)redisCommand(context, "KEYS chat:%s:*", username.c_str());
    if (reply->type == REDIS_REPLY_ARRAY)
    {
        for (size_t i = 0; i < reply->elements; ++i)
        {
            string key = reply->element[i]->str;
            size_t pos = key.find_last_of(':');
            if (pos != string::npos)
            {
                string partner = key.substr(pos + 1);
                if (partner != username) // 忽略自己
                    partners.push_back(partner);
            }
        }
    }
    freeReplyObject(reply);
    return partners;
}
// 存储离线消息
void RedisServer::storeOfflineMessage(const string &sender, const string &receiver, const string &message)
{
    string key = receiver + ":offline_messages";
    // 使用 LPUSH 将消息存储在列表中
    string fullMessage = sender + ":" + message;
    redisReply *reply = (redisReply *)redisCommand(context, "LPUSH %s %s", key.c_str(), fullMessage.c_str());
    if (reply == nullptr)
        cerr << "Redis 存储离线消息失败" << endl;
    else
        cout << "离线消息已存储: " << fullMessage << endl;
    freeReplyObject(reply);
}
// 获取离线消息,并清空
vector<string> RedisServer::getOfflineMessages(const string &receiver, const string &sender)
{
    // 键名 "receiver:offline_messages"
    string key = receiver + ":offline_messages";
    vector<string> messages;
    redisReply *reply = (redisReply *)redisCommand(context, "LRANGE %s 0 -1", key.c_str());
    if (reply == nullptr || reply->type != REDIS_REPLY_ARRAY)
        cerr << "Redis 获取离线消息失败" << endl;
    else
    {
        string senderPrefix = sender + ":";
        for (size_t i = 0; i < reply->elements; ++i)
        {
            string message(reply->element[i]->str);
            // 检查是否有来自特定发送者的消息
            if (message.compare(0, senderPrefix.length(), senderPrefix) == 0)
                messages.push_back(message);
        }
    }
    // 删除离线消息
    redisCommand(context, "DEL %s", key.c_str());
    freeReplyObject(reply);
    return messages;
}
// 判断用户离线消息是否为空
bool RedisServer::hasOfflineMessageFromSender(const string &receiver, const string &sender)
{
    // 键名是 "receiver:offline_messages"
    string key = receiver + ":offline_messages";
    // 使用 LRANGE 获取列表中的所有消息
    redisReply *reply = (redisReply *)redisCommand(context, "LRANGE %s 0 -1", key.c_str());
    if (reply == nullptr || reply->type != REDIS_REPLY_ARRAY)
    {
        cerr << "Redis 检查离线消息失败" << endl;
        freeReplyObject(reply);
        return false;
    }
    // 遍历所有消息，检查是否有来自特定发送者的消息
    bool hasMessageFromSender = false;
    string senderPrefix = sender + ":"; // 消息前缀 "sender:"
    for (size_t i = 0; i < reply->elements; ++i)
    {
        string message(reply->element[i]->str);
        if (message.compare(0, senderPrefix.length(), senderPrefix) == 0)
        {
            hasMessageFromSender = true;
            break;
        }
    }
    freeReplyObject(reply);
    return hasMessageFromSender;
}
// 存储好友申请
void RedisServer::storeFriendRequest(const string &receiver, const string &sender)
{
    string key = "friend_requests:" + receiver;
    redisReply *reply = (redisReply *)redisCommand(context, "RPUSH %s %s", key.c_str(), sender.c_str());
    if (reply == nullptr)
    {
        cerr << "Redis 命令失败: RPUSH" << endl;
        return;
    }
    freeReplyObject(reply);
}
// 取出并处理好友申请
string RedisServer::getAndRemoveFriendRequest(const string &receiver)
{
    string key = "friend_requests:" + receiver;
    redisReply *reply = (redisReply *)redisCommand(context, "LPOP %s", key.c_str());
    string sender;

    if (reply != nullptr && reply->type == REDIS_REPLY_STRING)
        sender = reply->str;
    else if (reply == nullptr || reply->type == REDIS_REPLY_NIL)
        sender = "NO";
    else
        cerr << "Redis 命令失败: LPOP" << endl;

    if (reply)
        freeReplyObject(reply);
    return sender;
}
// 存储群聊消息
void RedisServer::storeGroupMessage(const string &groupName, const string &message)
{
    string key = groupName + ":messages";
    redisReply *reply = (redisReply *)redisCommand(context, "RPUSH %b %b", key.data(), key.size(), message.data(), message.size());
    if (reply == nullptr || context->err)
    {
        cerr << "Redis 存储群聊消息失败: " << (context ? context->errstr : "未知错误") << endl;
        if (reply)
            freeReplyObject(reply);
        throw runtime_error("存储群聊消息失败");
    }
    cout << "群聊消息已存储: " << message << endl;
    freeReplyObject(reply);
}
// 删除群聊消息
void RedisServer::deleteAllGroupMessages(const string &groupName)
{
    string key = groupName + ":messages";
    redisReply *reply = (redisReply *)redisCommand(context, "DEL %b", key.data(), key.size());
    if (reply == nullptr || context->err)
    {
        cerr << "Redis 删除群聊消息失败: " << (context ? context->errstr : "未知错误") << endl;
        if (reply)
            freeReplyObject(reply);
        throw runtime_error("删除群聊消息失败");
    }
    cout << "所有群聊消息已删除" << endl;
    freeReplyObject(reply);
}
// 获取群聊记录
vector<string> RedisServer::getGroupMessages(const string &groupName)
{
    vector<string> messages;
    string key = groupName + ":messages";
    redisReply *reply = (redisReply *)redisCommand(context, "LRANGE %b 0 -1", key.data(), key.size());
    if (reply == nullptr || context->err)
    {
        cerr << "Redis 获取群聊记录失败: " << (context ? context->errstr : "未知错误") << endl;
        if (reply)
            freeReplyObject(reply);
        throw runtime_error("获取群聊记录失败");
    }
    // 遍历回复中的元素并将消息存储到 vector 中
    for (size_t i = 0; i < reply->elements; ++i)
        messages.push_back(reply->element[i]->str);
    // 释放回复对象
    freeReplyObject(reply);
    return messages;
}

// 用户当前聊天对象
void RedisServer::setCurrentChatPartner(const string &user, const string &chatPartner)
{
    string key = "current_chat:" + user;
    redisCommand(context, "SET %s %s", key.c_str(), chatPartner.c_str());
}
string RedisServer::getCurrentChatPartner(const string &user)
{
    // 获取用户的当前聊天对象
    string key = "current_chat:" + user;
    redisReply *reply = (redisReply *)redisCommand(context, "GET %s", key.c_str());
    if (reply->type == REDIS_REPLY_STRING)
    {
        string chatPartner = reply->str;
        freeReplyObject(reply);
        return chatPartner;
    }
    else
    {
        freeReplyObject(reply);
        return "NO"; // 如果没有找到聊天对象，返回空字符串
    }
}
// 清除用户的当前聊天对象
void RedisServer::clearCurrentChatPartner(const string &user)
{
    string key = "current_chat:" + user;
    redisCommand(context, "DEL %s", key.c_str());
}
// 存储申请
void RedisServer::storeGroupRequest(const string &receiver, const string &message)
{
    string key = "g_requests:" + receiver;
    // 存储消息到 Redis
    redisReply *reply = (redisReply *)redisCommand(context, "RPUSH %s %s", key.c_str(), message.c_str());
    if (reply == nullptr)
    {
        cerr << "Redis 命令失败: RPUSH" << endl;
        return;
    }
    freeReplyObject(reply);
}
// 取出并处理申请
pair<string, string> RedisServer::getAndRemoveGroupRequest(const string &receiver)
{
    string key = "g_requests:" + receiver;
    redisReply *reply = (redisReply *)redisCommand(context, "LPOP %s", key.c_str());
    string sender = "NO";
    string group;

    if (reply != nullptr)
    {
        if (reply->type == REDIS_REPLY_STRING)
        {
            // 解析消息内容
            string message = reply->str;
            size_t delimiterPos = message.find(':');
            if (delimiterPos != string::npos)
            {
                sender = message.substr(0, delimiterPos);
                group = message.substr(delimiterPos + 1);
            }
            else
                cerr << "消息格式无效: " << message << endl;
        }
        else if (reply->type == REDIS_REPLY_NIL)
            sender = "NO";
        else
            cerr << "Redis 命令失败: LPOP" << endl;
        freeReplyObject(reply);
    }
    else
        cerr << "Redis 命令失败: LPOP" << endl;

    return {sender, group};
}
// 删除申请
void RedisServer::removeGroupRequest(const string &receiver, const string &message)
{
    string key = "g_requests:" + receiver;
    // 从 Redis 列表中删除特定的消息
    redisReply *reply = (redisReply *)redisCommand(context, "LREM %s 0 %s", key.c_str(), message.c_str());
    if (reply == nullptr)
    {
        cerr << "Redis 命令失败: LREM" << endl;
        return;
    }
    freeReplyObject(reply);
}

// 存文件路径名
void RedisServer::storeFilePath(const string &sender, const string &receiver, const string &filepath)
{
    string key = sender + ":files:" + receiver;
    redisReply *reply = (redisReply *)redisCommand(context, "HSET %s filepath %s", key.c_str(), filepath.c_str());
    if (!reply)
    {
        cerr << "执行 Redis 命令失败" << endl;
        throw runtime_error("Redis 命令执行失败");
    }
    freeReplyObject(reply);
}
// 取文件路径
pair<string, string> RedisServer::getFilePath(const string &receiver)
{
    pair<string, string> file_info = {"", ""}; // 默认初始化为空的 pair
    string key_pattern = "*:files:" + receiver;
    redisReply *reply = (redisReply *)redisCommand(context, "KEYS %s", key_pattern.c_str());
    if (!reply)
    {
        cerr << "执行 Redis 命令失败" << endl;
        throw runtime_error("Redis 命令执行失败");
    }
    if (reply->type == REDIS_REPLY_ARRAY && reply->elements > 0)
    {
        for (size_t i = 0; i < reply->elements; ++i)
        {
            string key = reply->element[i]->str;
            size_t pos = key.find(":files:");
            if (pos != string::npos)
            {
                string sender = key.substr(0, pos);
                // 获取文件路径
                redisReply *path_reply = (redisReply *)redisCommand(context, "HGET %s filepath", key.c_str());
                if (path_reply && path_reply->type == REDIS_REPLY_STRING)
                {
                    file_info = {path_reply->str, sender};
                    freeReplyObject(path_reply);
                    break;
                }
                freeReplyObject(path_reply);
            }
        }
    }
    else
        cout << "没有找到匹配的文件路径！" << endl;

    freeReplyObject(reply);
    return file_info;
}
// 群聊-群主
void RedisServer::setGroupMaster(const string &group, const string &username)
{
    redisReply *reply = (redisReply *)redisCommand(context, "SET %s_master %s", group.c_str(), username.c_str());
    if (reply == nullptr)
    {
        cerr << "Redis SET 命令失败" << endl;
        return;
    }

    if (reply->type == REDIS_REPLY_STATUS && strcmp(reply->str, "OK") == 0)
        cout << "群聊主成功设置: " << group << " -> " << username << endl;
    else
        cerr << "设置群聊主失败: " << group << endl;

    freeReplyObject(reply);
}
bool RedisServer::groupExists(const string &group) const
{
    redisReply *reply = (redisReply *)redisCommand(context, "EXISTS %s_master", group.c_str());
    if (reply == nullptr)
    {
        cerr << "Redis EXISTS 命令失败" << endl;
        return false;
    }

    bool exists = (reply->integer == 1);
    if (exists)
        cout << "群聊存在: " << group << endl;
    else
        cout << "群聊不存在: " << group << endl;

    freeReplyObject(reply);
    return exists;
}
// 判断用户是否是群主
bool RedisServer::isGroupMaster(const string &group, const string &username)
{
    // 执行 Redis GET 命令获取群聊的群主信息
    redisReply *reply = (redisReply *)redisCommand(context, "GET %s_master", group.c_str());
    if (reply == nullptr)
    {
        cerr << "Redis GET 命令失败" << endl;
        return false;
    }
    // 检查返回值是否为字符串类型
    bool isMaster = false;
    if (reply->type == REDIS_REPLY_STRING)
        isMaster = (strcmp(reply->str, username.c_str()) == 0);
    else
        cerr << "群聊主信息不存在或格式错误" << endl;
    freeReplyObject(reply);
    return isMaster;
}