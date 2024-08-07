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
#include <hiredis/hiredis.h>
#include <condition_variable>
#include <nlohmann/json.hpp>
using namespace nlohmann;

using namespace std;

class RedisServer
{
public:
    RedisServer();
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

    // 将用户添加为群聊的创建者
    bool addMaster(const string &groupName, const string &memberName);
    // 获取用户创建的群聊列表
    vector<string> getGroupsByMaster(const string &memberName);
    // 群主加管理员
    bool addAdminToGroup(const string &groupName, const string &adminName);
    // 群主删管理员
    bool removeAdminFromGroup(const string &groupName, const string &adminName);
    // 管理员列表
    vector<string> getManagers(const string &groupName);
    // 获取群主和管理员列表
    vector<string> getAdmins(const string &groupName);
    // 获取普通用户列表
    vector<string> getUsers(const string &groupName);

    // 创建群聊
    bool createGroup(const string &groupName);
    // 删除群聊并更新创建者的群聊列表
    bool deleteGroup(const string &groupName, const string &masterName);
    // 加入群聊
    bool addMemberToGroup(const string &groupName, const string &memberName);
    // 退出群聊
    bool removeMemberFromGroup(const string &groupName, const string &memberName);
    // 群聊成员
    vector<string> getGroupMembers(const string &groupName);

    // 加入用户列表
    bool addUserToGroupList(const string &username, const string &groupName);
    // 退出用户列表
    bool removeUserFromGroupList(const string &username, const string &groupName);
    // 用户列表
    vector<string> getUserGroupList(const string &username);

private:
    redisContext *context;
};

void doregister(int fd, json j); // 注册
void logout(int fd, json j);     // 注销
void isUser(int fd, json j);     // 是否注册
void f_chatlist(int fd, json j); // 好友列表
void f_chat(int fd, json j);     // 好友聊天
void f_block(int fd, json j);    // 好友屏蔽
void f_unblock(int fd, json j);  // 好友取消屏蔽
void f_add(int fd, json j);      // 好友添加
void f_addreply(int fd, json j); // 好友添加回复
void f_delete(int fd, json j);   // 好友删除
void g_showuser(int fd, json j); // 查看群用户
void g_showlist(int fd, json j); // 查询群聊
void g_create(int fd, json j);   // 创建群聊
void g_disband(int fd, json j);  // 删除群聊
void g_leave(int fd, json j);    // 退出群聊
void g_join(int fd, json j);     // 加入群聊

extern map<int, string> client_map;
string get_name(int fd, const map<int, string> &client_map);
int get_fd(const string &username, const map<int, string> &client_map);
void fd_user(int fd, string &name);
bool is_name_present(const string &name);

#endif