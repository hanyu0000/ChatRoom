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
map<int, string> client_map;
void fd_user(int fd, string &name);
bool is_name_present(const string &name);
string get_name(int fd, const map<int, string> &client_map);
int get_fd(const string &username, const map<int, string> &client_map);


void doregister(int fd, json j); // 注册
void logout(int fd, json j);     // 注销
void isUser(int fd, json j);     // 是否注册
void f_chatlist(int fd, json j); // 好友列表
void f_chat(int fd, json j);     // 好友聊天
void f_block(int fd, json j);    // 好友屏蔽
void f_unblock(int fd, json j);  // 好友取消屏蔽
void f_add(int fd, json j);      // 好友添加
void f_delete(int fd, json j);   // 好友删除
void g_showuser(int fd, json j); // 查看群用户
void g_showlist(int fd, json j); // 查询群聊
void g_create(int fd, json j);   // 创建群聊
void g_disband(int fd, json j);  // 删除群聊
void g_leave(int fd, json j);    // 退出群聊
void g_join(int fd, json j);     // 加入群聊

#endif