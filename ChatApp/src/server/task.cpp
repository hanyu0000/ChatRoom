#include "head.hpp"
#include "task.hpp"
// 实例化 RedisServer 对象
RedisServer redis;
// 注册
void doregister(int fd, json j)
{
    string name = j["name"].get<string>();
    string pwd = j["pwd"].get<string>();

    string reply;
    bool flag = redis.friends_exit(name);
    string buf = flag ? "exitOK" : "exitNO";
    if (buf == "exitNO") // 用户未注册
    {
        bool success = redis.setPassword(name, pwd);
        reply = success ? "setOK" : "setNO";
    }
    else
        reply = buf; // 用户已注册

    cout << reply << endl;
    json u_i =
        {
            {"register", reply}};
    string str = u_i.dump();
    if (Util::send_msg(fd, str) == -1)
        err_("send_msg");
}
// 注销
void logout(int fd, json j)
{
    string name = j["name"].get<string>();
    string pwd = j["pwd"].get<string>();

    bool success = redis.deleteUser(name);
    string reply = success ? "deleteOK" : "deleteNO";
    cout << reply << endl;
    json u_i =
        {
            {"logout", reply}};
    string str = u_i.dump();
    if (Util::send_msg(fd, str) == -1)
        err_("send_msg");
}
// 是否注册
void isUser(int fd, json j)
{
    string name = j["name"].get<string>();
    string pwd = j["pwd"].get<string>();

    string reply;
    if (is_name_present(name))
    {
        cout << "该用户已经登录" << endl;
        reply = "loading";
    }
    else
    {
        bool registered = redis.isUser(name, pwd);
        reply = registered ? "IS USER" : "NO USER";
        fd_user(fd, name);
    }
    cout << reply << endl;
    json u_i =
        {
            {"isUser", reply}};
    string str = u_i.dump();
    if (Util::send_msg(fd, str) == -1)
        err_("send_msg");
}

// 好友列表
void f_chatlist(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;

    vector<string> chatlist = redis.getFriends(my_name);
    for (const auto &name : chatlist)
        cout << my_name << "的好友名单:" << name << endl;
    json to_my = {
        {"chatlist", chatlist}};
    string message = to_my.dump();
    if (Util::send_msg(fd, message) == -1)
        err_("send_msg");
}
// 好友聊天
void f_chat(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;
    string f_name = j["name"].get<string>();
    int f_fd = get_fd(f_name, client_map);

    string reply = j["message"].get<string>();
    json to_f = {
        {"chat", reply},
        {"f_name", my_name}};
    json to_my = {
        {"chat", reply},
        {"f_name", f_name}};
    string message1 = to_f.dump();
    string message2 = to_my.dump();
    if (Util::send_msg(fd, message1) == -1)
        err_("send_msg");
    if (Util::send_msg(fd, message2) == -1)
        err_("send_msg");
}
// 好友屏蔽
void f_block(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;
    string f_name = j["name"].get<string>();
    int f_fd = get_fd(f_name, client_map);

    redis.blockUser(my_name, f_name);
    json a = {
        {"blockfriend", "blockOK"},
        {"f_name", f_name}};
    string message = a.dump();
    if (Util::send_msg(fd, message) == -1)
        err_("send_msg");
}
// 取消屏蔽
void f_unblock(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;
    string f_name = j["name"].get<string>();
    int f_fd = get_fd(f_name, client_map);

    redis.unblockUser(my_name, f_name); // 从屏蔽列表取出
    json a = {
        {"unblockfriend", "OK"},
        {"f_name", f_name}};
    string message = a.dump();
    if (Util::send_msg(fd, message) == -1)
        err_("send_msg");
}
// 好友添加 addfriend
void f_add(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;
    string f_name = j["name"].get<string>();
    int f_fd = get_fd(f_name, client_map);

    // 判断用户是否存在
    bool userexit = redis.friends_exit(f_name);
    cout << "userexit:" << userexit << endl;
    if (!userexit) // 用户不存在
    {
        json a =
            {
                {"nopeople", f_name}};
        string str = a.dump();
        if (Util::send_msg(fd, str) == -1)
            err_("send_msg");
        return;
    }

    json b = {
        {"newfriend", my_name}};
    string message = b.dump();
    if (Util::send_msg(fd, message) == -1)
        err_("send_msg");
    cout << "发送好友申请成功!" << endl;
}
// 好友添加回复
void f_addreply(int fd, json j)
{
    string f_name = get_name(fd, client_map);
    cout << "处理来自 <" << f_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;
    string my_name = j["name"].get<string>();
    int my_fd = get_fd(my_name, client_map); // 发出申请

    string reply = j["reply"].get<string>();
    cout << reply << endl;

    if (reply == "同意")
    {
        json to_my = {
            {"newfriendreply", reply},
            {"f_name", f_name}};
        string message = to_my.dump();
        cout << "同意加好友" << endl;
        redis.addFriend(my_name, f_name);
        redis.addFriend(f_name, my_name);
        if (Util::send_msg(fd, message) == -1)
            err_("send_msg");
    }
    else if (reply == "不同意")
    {
        json to_my = {
            {"newfriendreply", reply},
            {"f_name", f_name}};
        string message = to_my.dump();
        cout << "不同意加好友" << endl;
        if (Util::send_msg(fd, message) == -1)
            err_("send_msg");
    }
}
// 好友删除
void f_delete(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;
    string f_name = j["name"].get<string>();
    int f_fd = get_fd(f_name, client_map);

    bool userexit = redis.friends_exit(f_name);
    cout << "userexit:" << userexit << endl;
    if (!userexit)
    {
        json a =
            {
                {"nopeople", f_name}};
        string str = a.dump();
        if (Util::send_msg(fd, str) == -1)
            err_("send_msg");
        return;
    }

    redis.removeFriend(my_name, f_name);
    redis.removeFriend(f_name, my_name);
    json a = {
        {"deletefriend", "YES"},
        {"f_name", f_name}};
    string message = a.dump();
    if (Util::send_msg(fd, message) == -1)
        err_("send_msg");
}
// 查看群用户
void g_showuser(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;
    string group = j["group"].get<string>();
    cout << group << endl;

    vector<string> userlist = redis.getGroupMembers(group);
    cout << "群聊 < " << group << " > 的用户列表: " << endl;
    for (const auto &grp : userlist)
        cout << grp << endl;

    json response = {
        {"type", "userlist"},
        {"userlist", userlist}};
    string message = response.dump();
    if (Util::send_msg(fd, message) == -1)
        err_("send_msg");
}
// 查询群聊
void g_showlist(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;

    vector<string> grouplist = redis.getUserGroupList(my_name);
    cout << "用户 < " << my_name << " > 的群聊列表: " << endl;

    for (const auto &grp : grouplist)
        cout << grp << endl;

    json response = {
        {"type", "grouplist"},
        {"grouplist", grouplist}};
    string message = response.dump();
    if (Util::send_msg(fd, message) == -1)
        err_("send_msg");
}
// 创建群聊
void g_create(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;

    string group = j["group"].get<string>();
    cout << "创建群聊: " << group << endl;

    redis.createGroup(group);        // 创建群聊
    redis.addMaster(group, my_name); // 群聊群主

    vector<string> members = j["members"];
    for (const auto &member : members)
    {
        cout << "添加成员: " << member << endl;
        redis.addMemberToGroup(group, member);   // 向群聊添加成员
        redis.addUserToGroupList(member, group); // 群聊添加至成员列表
    }

    // 发送确认消息到客户端
    json response = {
        {"type", "create_group"},
        {"status", "success"}};
    string response_str = response.dump();
    if (Util::send_msg(fd, response_str) == -1)
        cerr << "发送确认消息失败" << endl;
}
// 删除群聊
void g_disband(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;

    string group = j["group"].get<string>();
    cout << group << endl;
    redis.deleteGroup(group, my_name); // 删除群聊
    vector<string> members = redis.getGroupMembers(group);
    for (const auto &member : members)
        redis.removeUserFromGroupList(member, group);
}
// 加入群聊
void g_join(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;

    string group = j["group"].get<string>();
    cout << group << endl;
    // 获取管理列表
    vector<string> members = redis.getAdmins(group);
    for (const auto &member : members)
    {
        string request = "User " + my_name + " 请求加入群组 " + group;
        int manager_fd = get_fd(member, client_map);
        if (manager_fd != -1)
            Util::send_msg(manager_fd, request);
    }
    if (j["reply" == "YES"])
    {
        redis.addUserToGroupList(my_name, group);
        redis.addMemberToGroup(group, my_name); // 加入群聊
    }
}
// 退出群聊
void g_leave(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;

    string group = j["group"].get<string>();
    cout << "用户 <" << my_name << "> 退出群聊: " << group << endl;

    redis.removeUserFromGroupList(my_name, group);
    redis.removeMemberFromGroup(group, my_name); // 退出群聊
}

string get_name(int fd, const map<int, string> &client_map)
{
    auto it = client_map.find(fd);
    if (it != client_map.end())
        return it->second;
    return "";
}
int get_fd(const string &username, const map<int, string> &client_map)
{
    for (const auto &pair : client_map)
        if (pair.second == username)
            return pair.first;
    return -1;
}