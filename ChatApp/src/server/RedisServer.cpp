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