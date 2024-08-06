#include "head.hpp"
#include "serv_main.hpp"
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
            {"register", reply},
        };
    string str = u_i.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");
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
            {"logout", reply},
        };
    string str = u_i.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");
}
// 是否注册
void isUser(int fd, json j)
{
    string name = j["name"].get<string>();
    string pwd = j["pwd"].get<string>();

    string reply;
    if (is_name_present(name)) // 用户已经登录
        reply = "loading";
    else
    {
        bool registered = redis.isUser(name, pwd);
        reply = registered ? "IS USER" : "NO USER";
        fd_user(fd, name);
    }
    cout << reply << endl;
    json u_i =
        {
            {"isUser", reply},
        };
    string str = u_i.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");
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
        {"chatlist", chatlist},
    };
    string message = to_my.dump();
    if (send(fd, message.c_str(), message.size(), 0) == -1)
        err_("send_f");
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
        {"f_name", my_name},
    };
    json to_my = {
        {"chat", reply},
        {"f_name", f_name},
    };
    string message1 = to_f.dump();
    string message2 = to_my.dump();
    if (send(f_fd, message1.c_str(), message1.size(), 0) == -1)
        err_("send_f");
    if (send(fd, message2.c_str(), message2.size(), 0) == -1)
        err_("send_my");
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
        {"f_name", f_name},
    };
    string message_str = a.dump();
    if (send(fd, message_str.c_str(), message_str.size(), 0) == -1)
        err_("屏蔽好友失败");
}
// 好友取消屏蔽
void f_unblock(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;
    string f_name = j["name"].get<string>();
    int f_fd = get_fd(f_name, client_map);

    redis.unblockUser(my_name, f_name); // 从屏蔽列表取出
    json a = {
        {"unblockfriend", "OK"},
        {"f_name", f_name},
    };
    string message_str = a.dump();
    if (send(fd, message_str.c_str(), message_str.size(), 0) == -1)
        err_("取消屏蔽好友失败");
}
// 好友添加
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
                {"nopeople", f_name},
            };
        string str = a.dump();
        if (send(fd, str.c_str(), str.size(), 0) == -1)
            err_("发送好友不存在失败");
        return;
    }

    json b = {
        {"newfriend", my_name},
    };
    string message = b.dump();
    if (send(f_fd, message.c_str(), message.size(), 0) == -1)
        err_("发送好友申请失败");
    cout << "发送好友申请成功!" << endl;

    // 回复
    string reply = j["reply"].get<string>();
    cout << reply << endl;
    json to_f = {
        {"reply", reply},
        {"f_name", my_name},
    };
    string message1 = to_f.dump();

    json to_my = {
        {"reply", reply},
        {"f_name", f_name},
    };
    string message2 = to_my.dump();
    if (reply == "YES")
    {
        cout << "同意加好友" << endl;
        redis.addFriend(my_name, f_name);
        redis.addFriend(f_name, my_name);
        if (send(f_fd, message1.c_str(), message1.size(), 0) == -1)
            err_("send_f");
    }
    if (send(fd, message2.c_str(), message2.size(), 0) == -1)
        err_("send_my");
}
// 好友删除
void f_delete(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;
    string f_name = j["name"].get<string>();
    int f_fd = get_fd(f_name, client_map);

    redis.removeFriend(my_name, f_name);
    redis.removeFriend(f_name, my_name);
    json a = {
        {"deletefriend", "YES"},
        {"f_name", f_name},
    };
    string message = a.dump();
    if (send(fd, message.c_str(), message.size(), 0) == -1)
        err_("删除好友失败");
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
    json to_me = {
        {"grouplist", userlist},
    };
    string message = to_me.dump();
    if (send(fd, message.c_str(), message.size(), 0) == -1)
        err_("send_f");
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
    json to_me = {
        {"grouplist", grouplist},
    };
    string message = to_me.dump();
    if (send(fd, message.c_str(), message.size(), 0) == -1)
        err_("send_f");
}
// 创建群聊
void g_create(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;

    string group = j["group"].get<string>();
    cout << group << endl;

    redis.createGroup(group);//创建群聊
    vector<string> members = j["members"];
    redis.addMemberToGroup(group, my_name);
    redis.addUserToGroupList(my_name, group);
    for (const auto &member : members)
    {
        cout << "添加成员: " << member << endl;
        redis.addMemberToGroup(group, member);   // 向群聊添加成员
        redis.addUserToGroupList(member, group); // 群聊添加至成员列表
    }
}
// 删除群聊
void g_disband(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;

    string group = j["group"].get<string>();
    cout << group << endl;
    redis.deleteGroup(group); // 删除群聊
}
// 退出群聊
void g_leave(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;

    string group = j["group"].get<string>();
    cout << group << endl;
    redis.removeMemberFromGroup(group, my_name); // 退出群聊
}
// 加入群聊
void g_join(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;

    string group = j["group"].get<string>();
    cout << group << endl;
    redis.addMemberToGroup(group, my_name); // 加入群聊
}