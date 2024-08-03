#ifndef TUI_H
#define TUI_H
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <atomic>
#include <memory>
#include <cstdio>
#include <limits>
#include <vector>
#include <thread>
#include <future>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/uio.h>
#include <functional>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <json.hpp>
#include <hiredis/hiredis.h>
#include <condition_variable>
using json = nlohmann::json;
using namespace std;

string get_name(int fd, const map<int, string> &client_map);
int get_fd(const string &username, const map<int, string> &client_map);
void serv_main(int my_fd, const json &request,map<int, string> &client_map, RedisServer &redisServer);

class RedisServer
{
public:
    RedisServer(const string &hostname, int port)
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

    ~RedisServer()
    {
        if (context)
            redisFree(context);
    }

    // 存储用户信息
    bool setPassword(const string &username, const string &password)
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
    // 判断用户是否注册
    bool isUser(const string &username, const string &password)
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
    // 判断用户名是否存在
    bool friends_exit(const string &username)
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
    // 删除用户信息
    bool deleteUser(const string &username)
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

private:
    redisContext *context;
};

#endif