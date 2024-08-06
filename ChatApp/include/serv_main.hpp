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

class RedisServer
{
public:
    RedisServer(const string &hostname, int port);
    ~RedisServer();
    // 存储用户信息
    bool setPassword(const string &username, const string &password);
    // 判断用户是否注册
    bool isUser(const string &username, const string &password);
    // 判断用户名是否存在
    bool friends_exit(const string &username);
    // 删除用户信息
    bool deleteUser(const string &username);

    // 存储好友关系
    bool addFriend(const string &username, const string &friendname);
    // 判断是否是好友
    bool isFriend(const string &username, const string &friendname);
    // 获取好友列表
    vector<string> getFriends(const string &username);
    // 删除好友
    bool removeFriend(const string &username, const string &friendname);

    // 存储屏蔽关系
    bool blockUser(const string &username, const string &blockname);
    // 判断是否屏蔽某用户
    bool isBlocked(const string &username, const string &blockname);
    // 获取屏蔽列表
    vector<string> getBlockedUsers(const string &username);
    // 取消屏蔽用户
    bool unblockUser(const string &username, const string &blockname);

    // 群聊管理
    bool createGroup(const string &groupName);
    bool deleteGroup(const string &groupName);
    bool addMemberToGroup(const string &groupName, const string &memberName);
    bool removeMemberFromGroup(const string &groupName, const string &memberName);
    vector<string> getGroupMembers(const string &groupName);

    // 用户群聊列表管理
    bool addUserToGroupList(const string &username, const string &groupName);
    bool removeUserFromGroupList(const string &username, const string &groupName);
    vector<string> getUserGroupList(const string &username);

private:
    redisContext *context;
};

string get_name(int fd, const map<int, string> &client_map);
int get_fd(const string &username, const map<int, string> &client_map);
void serv_main(int my_fd, const json &request, map<int, string> &client_map, RedisServer &redis);
void group_main(int my_fd, const json &request, map<int, string> &client_map, RedisServer &redis);

#endif