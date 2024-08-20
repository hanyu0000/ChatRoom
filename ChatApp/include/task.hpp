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

    // 存储好友关系
    bool addFriend(const string &username, const string &friendname);
    // 获取好友列表
    vector<string> getFriends(const string &username);
    // 删除好友
    bool removeFriend(const string &username, const string &friendname);

    // 存储屏蔽关系
    bool blockUser(const string &username, const string &blockname);
    // 取消屏蔽用户
    bool unblockUser(const string &username, const string &blockname);
    // 判断用户是否被另一个用户屏蔽
    bool isUserBlocked(const string &username, const string &blockname);
    // 获取屏蔽列表
    vector<string> getBlockedUsers(const string &username);

    // 创建的群聊
    void mycreateGroup(const string &groupName, const string &creator);
    // 获取群聊列表
    vector<string> getGroupsByUser(const string &username);
    // 群聊-群主
    void setGroupMaster(const string &group, const string &username);
    // 判断用户是否是群主
    bool isGroupMaster(const string &group, const string &username);
    // 删除群聊的群主
    void deleteGroupMaster(const string &group);
    // 删除我的群聊
    void deleteGroupByUser(const string &groupName, const string &username);

    // 群主加管理员
    void addAdminToGroup(const string &groupName, const string &adminName);
    // 群主删管理员
    void removeAdminFromGroup(const string &groupName, const string &adminName);
    // 管理员列表
    vector<string> getManagers(const string &groupName);
    // 判断是不是管理员
    bool isGroupManager(const string &groupName, const string &username);

    // 删除用户信息
    bool deleteUser(const string &username);
    // 清空用户的好友列表
    bool removeAllFriends(const string &username);
    // 清空用户的屏蔽好友列表
    bool removeAllBlockedUsers(const string &username);
    // 删除用户的群组列表
    bool removeUserGroupList(const string &username);
    // 删除用户创建的所有群聊
    bool removeGroupsByMaster(const string &memberName);
    // 删除用户的群列表
    void removeUserFromAllGroups(const string &username);
    // 删除用户创建的所有聊天记录
    bool removeChatRecordsByUser(const string &username);
    // 获取用户参与的所有聊天对
    vector<string> getChatPartners(const string &username);

    // 创建群聊
    bool createGroup(const string &groupName);
    // 从集合中删除群聊
    void deleteGroup(const string &groupName);
    // 判断群聊是否存在
    bool isGroupExists(const string &groupName);

    // 删除群聊并更新创建者的群聊列表
    bool deleteGroup(const string &groupName, const string &masterName);
    // 加入群聊
    bool addMemberToGroup(const string &groupName, const string &memberName);
    // 退出群聊
    bool removeMemberFromGroup(const string &groupName, const string &memberName);
    // 群聊成员
    vector<string> getGroupMembers(const string &groupName);

    // 加用户群列表
    bool addUserToGroupList(const string &username, const string &groupName);
    // 删用户群列表
    bool removeUserFromGroupList(const string &username, const string &groupName);
    // 用户群列表
    vector<string> getUserGroupList(const string &username);
    // 删除群聊并从所有用户的群组列表中移除该群聊
    bool deleteGroupFromUserLists(const string &groupName);

    // 存储聊天记录
    void storeChatRecord(const string &user1, const string &user2, const string &message);
    // 获取聊天记录
    vector<string> getChatRecord(const string &user1, const string &user2);

    // 存储离线消息
    void storeOfflineMessage(const string &sender, const string &receiver, const string &message);
    // 获取离线消息,并清空
    vector<string> getOfflineMessages(const string &receiver, const string &sender);
    // 判断用户离线消息是否为空
    bool hasOfflineMessageFromSender(const string &receiver, const string &sender);

    // 存储好友申请
    void storeFriendRequest(const string &receiver, const string &sender);
    // 取出并处理好友申请
    string getAndRemoveFriendRequest(const string &receiver);
    // 存储群聊申请
    void storeGroupRequest(const string &receiver, const string &sender);
    // 取出并处理申请
    pair<string, string> getAndRemoveGroupRequest(const string &receiver);
    // 删除申请
    void removeGroupRequest(const string &receiver, const string &message);

    // 存储群聊消息
    void storeGroupMessage(const string &groupName, const string &message);
    // 删除群聊消息
    void deleteAllGroupMessages(const string &groupName);
    // 获取群聊记录
    vector<string> getGroupMessages(const string &groupName);

    // 存文件路径名
    void storeFilePath(const string &sender, const string &receiver, const string &filepath);
    // 取文件路径
    pair<string, string> getFilePath(const string &receiver);
    // 删除文件路径
    void deleteFilePath(const string &sender, const string &receiver);
    // 获取群聊的群主
    string getGroupMaster(const string &group);
    // 检查 sender 是否已经向 receiver 发送了好友申请
    bool hasFriendRequest(const string &receiver, const string &sender);
    // 删除某人的好友申请
    void removeFriendRequest(const string &receiver, const string &sender);

private:
    redisContext *context;
};
void recv_file(int fd, json j);
void send_file(int fd, json j);
void login(int fd, json j);
void return_last(int fd, json j);   // 返回上一级页面
void doregister(int fd, json j);    // 注册
void logout(int fd, json j);        // 注销
void isUser(int fd, json j);        // 是否注册
void f_chatlist(int fd, json j);    // 好友列表
void my_group_list(int fd, json j); // 我创建的群聊列表
void f_chat(int fd, json j);        // 好友聊天
void g_chat(int fd, json j);
void is_Blocked(int fd, json j);
void g_chatHistry(int fd, json j);
void charge_file(int fd, json j);
void delete_people(int fd, json j);
void g_reply(int fd, json j);
void f_chat_leave(int fd, json j);
void g_addreply(int fd, json j);
void newfriend_leave(int fd, json j);
void add_manager(int fd, json j);    // 设置管理员
void delete_manager(int fd, json j); // 删除管理员
void managelist(int fd, json j);     // 管理员列表
void f_chatHistry(int fd, json j);   // 聊天记录
void f_block(int fd, json j);        // 好友屏蔽
void f_unblock(int fd, json j);      // 好友取消屏蔽
void block_list(int fd, json j);     // 屏蔽列表
void f_add(int fd, json j);          // 好友添加
void f_addreply(int fd, json j);     // 好友添加回复
void f_delete(int fd, json j);       // 好友删除
void g_showuser(int fd, json j);     // 查看群用户
void g_showlist(int fd, json j);     // 查询群聊
void g_create(int fd, json j);       // 创建群聊
void g_disband(int fd, json j);      // 删除群聊
void g_leave(int fd, json j);        // 退出群聊
void g_join(int fd, json j);         // 加入群聊

extern map<int, string> client_map;
string get_name(int fd, const map<int, string> &client_map);
int get_fd(const string &username, const map<int, string> &client_map);
void fd_user(int fd, string &name);
bool is_name_present(const string &name);

#endif