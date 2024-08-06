#include "head.hpp"
#include "serv_main.hpp"

void group_main(int my_fd, const json &request, map<int, string> &client_map, RedisServer &redis)
{
    string my_name = get_name(my_fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << my_fd << " 的请求: " << request.dump(3) << endl;
    string ggg = request["ggg"].get<string>();
    // 用户的群聊列表
    if (ggg == "grouplist")
    {
        vector<string> grouplist = redis.getUserGroupList(my_name);
        cout << "用户 < " << my_name << " > 的群聊列表: " << endl;
        for (const auto &grp : grouplist)
            cout << grp << endl;
        json to_me = {
            {"grouplist", grouplist},
        };
        string message = to_me.dump();
        if (send(my_fd, message.c_str(), message.size(), 0) == -1)
            err_("send_f");
        return;
    }
    string group = request["group"].get<string>();
    cout << group << endl;
    // 创建群聊
    if (ggg == "create_group")
    {
        redis.createGroup(group);
        vector<string> members = request["members"];
        redis.addMemberToGroup(group, my_name);
        redis.addUserToGroupList(my_name, group);
        for (const auto &member : members)
        {
            cout << "添加成员: " << member << endl;
            redis.addMemberToGroup(group, member);   // 向群聊添加成员
            redis.addUserToGroupList(member, group); // 群聊添加至成员列表
        }
        return;
    }
    else if (ggg == "disband_group")
    {
        redis.deleteGroup(group); // 删除群聊
        return;
    }
    else if (ggg == "userlist")
    {
        vector<string> userlist = redis.getGroupMembers(group);
        cout << "群聊 < " << group << " > 的用户列表: " << endl;
        for (const auto &grp : userlist)
            cout << grp << endl;
        json to_me = {
            {"grouplist", userlist},
        };
        string message = to_me.dump();
        if (send(my_fd, message.c_str(), message.size(), 0) == -1)
            err_("send_f");
        return;
    }
    else if (ggg == "leave_group")
    {
        redis.removeMemberFromGroup(group, my_name); // 退出群聊
        return;
    }
    else if (ggg == "join_group")
    {
        redis.addMemberToGroup(group, my_name); // 加入群聊
        return;
    }
}
void serv_main(int my_fd, const json &request, map<int, string> &client_map, RedisServer &redis)
{
    string my_name = get_name(my_fd, client_map);
    string choice = request["choice"].get<string>();
    cout << "处理来自 <" << my_name << ">: " << my_fd << " 的请求: " << request.dump(3) << endl;
    if (choice == "chatlist")
    {
        vector<string> chatlist = redis.getFriends(my_name);
        for (const auto &name : chatlist)
            cout << my_name << "的好友名单:" << name << endl;
        json to_my = {
            {"chatlist", chatlist},
        };
        string message = to_my.dump();
        if (send(my_fd, message.c_str(), message.size(), 0) == -1)
            err_("send_f");
        return;
    }

    string f_name = request["name"].get<string>();
    int f_fd = get_fd(f_name, client_map);
    bool userexit = redis.friends_exit(f_name);
    cout << "userexit:" << userexit << endl;
    if (!userexit) // 用户不存在
    {
        json a =
            {
                {"nopeople", f_name},
            };
        string str = a.dump();
        if (send(my_fd, str.c_str(), str.size(), 0) == -1)
            err_("发送好友不存在失败");
        return;
    }
    bool friexit = redis.isFriend(my_name, f_name);
    cout << "friexit:" << friexit << endl;

    // 加好友
    if (choice == "addfriend")
    {
        json a = {
            {"newfriend", my_name},
        };
        string message = a.dump();
        if (send(f_fd, message.c_str(), message.size(), 0) == -1)
            err_("发送好友申请失败");
        cout << "发送好友申请成功!" << endl;
        return;
    }
    // 回应加好友消息
    else if (choice == "reply")
    {
        string reply = request["reply"].get<string>();
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
        if (send(my_fd, message2.c_str(), message2.size(), 0) == -1)
            err_("send_my");
        return;
    }
    // 好友聊天
    else if (choice == "chat")
    {
        string reply = request["message"].get<string>();
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
        if (send(my_fd, message2.c_str(), message2.size(), 0) == -1)
            err_("send_my");
        return;
    }
    // 屏蔽好友
    else if (choice == "blockfriend")
    {
        json a;
        if (redis.isBlocked(my_name, f_name)) // 已经在屏蔽列表
        {
            a = {
                {"blockfriend", "already"},
                {"f_name", f_name},
            };
        }
        else
        {
            redis.blockUser(my_name, f_name);
            a = {
                {"blockfriend", "blockOK"},
                {"f_name", f_name},
            };
        }
        string message_str = a.dump();
        if (send(my_fd, message_str.c_str(), message_str.size(), 0) == -1)
            err_("屏蔽好友失败");
        return;
    }
    // 取消屏蔽好友
    else if (choice == "unblockfriend")
    {
        json a;
        if (redis.isBlocked(my_name, f_name))
        {
            redis.unblockUser(my_name, f_name); // 从屏蔽列表取出
            a = {
                {"unblockfriend", "OK"},
                {"f_name", f_name},
            };
        }
        else
        {
            a = {
                {"unblockfriend", "NO"},
                {"f_name", f_name},
            };
        }
        string message_str = a.dump();
        if (send(my_fd, message_str.c_str(), message_str.size(), 0) == -1)
            err_("取消屏蔽好友失败");
        return;
    }
    // 删除好友
    else if (choice == "deletefriend")
    {
        redis.removeFriend(my_name, f_name);
        redis.removeFriend(f_name, my_name);
        json a = {
            {"deletefriend", "YES"},
            {"f_name", f_name},
        };
        string message = a.dump();
        if (send(my_fd, message.c_str(), message.size(), 0) == -1)
            err_("删除好友失败");
        return;
    }
}

string get_name(int fd, const map<int, string> &client_map)
{
    auto it = client_map.find(fd);
    if (it != client_map.end())
        return it->second;
    else
        return "NO";
}

int get_fd(const string &name, const map<int, string> &client_map)
{
    for (const auto &pair : client_map)
    {
        if (pair.second == name)
            return pair.first;
    }
    return -1;
}