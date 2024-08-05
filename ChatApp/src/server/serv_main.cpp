#include "head.hpp"
#include "serv_main.hpp"

void serv_main(int my_fd, const json &request, map<int, string> &client_map, RedisServer &redis)
{
    string my_name = get_name(my_fd, client_map);
    string f_name = request["name"].get<string>();
    int f_fd = get_fd(f_name, client_map);

    cout << "处理来自 <" << my_name << ">: " << my_fd << " 的请求: " << request.dump(3) << endl;

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
        cout << "发送好友不存在消息" << endl;
        return;
    }

    bool friexit = redis.isFriend(my_name, f_name); // 是否存在好友关系
    cout << "friexit:" << friexit << endl;
    string choice = request["choice"].get<string>();
    // 加好友
    if (choice == "addfriend")
    {
        if (!friexit)
        {
            cout << "好友关系不存在，可以加好友！" << endl;
            // 发送好友申请到好友客户端
            json a = {
                {"newfriend", my_name},
            };
            string message = a.dump();
            if (send(f_fd, message.c_str(), message.size(), 0) == -1)
                err_("发送好友申请失败");
            cout << "发送好友申请成功!" << endl;
            return;
        }
        else
        {
            json a = {
                {"newfriend", my_name},
                {"already", "yet"},
            };
            string message = a.dump();
            if (send(f_fd, message.c_str(), message.size(), 0) == -1)
                err_("发送已是好友信息失败");
            cout << "发送已是好友信息成功!" << endl;
            return;
        }
    }
    // 回应加好友消息
    if (choice == "newfriendreply")
    {
        string reply = request["reply"].get<string>();
        json to_f = {
            {"newfriendreply", reply},
            {"f_name", my_name},
        };
        json to_my = {
            {"newfriendreply", reply},
            {"f_name", f_name},
        };
        string message1 = to_f.dump();
        string message2 = to_my.dump();
        if (reply == "YES")
        {
            redis.addFriend(my_name, f_name);
            redis.addFriend(f_name, my_name);
            cout << "同意加好友" << endl;
            if (send(f_fd, message1.c_str(), message1.size(), 0) == -1)
                err_("send_f");
            if (send(my_fd, message2.c_str(), message2.size(), 0) == -1)
                err_("send_my");
        }
        else
        {
            cout << "不同意加好友" << endl;
            if (send(my_fd, message2.c_str(), message2.size(), 0) == -1)
                err_("send_my");
        }
        return;
    }

    // 好友聊天
    if (choice == "chat")
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
        if (send(f_fd, message_str.c_str(), message_str.size(), 0) == -1)
            err_("屏蔽好友失败");
        cout << "屏蔽好友成功!" << endl;
        return;
    }
    // 取消屏蔽好友
    if (choice == "unblockfriend")
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
        if (send(f_fd, message_str.c_str(), message_str.size(), 0) == -1)
            err_("取消屏蔽好友失败");
        cout << "取消屏蔽好友成功!" << endl;
        return;
    }
    // 删除好友
    else if (choice == "deletefriend")
    {
        redis.removeFriend(my_name, f_name);
        redis.removeFriend(f_name, my_name);
        json a = {
            {"deletefriend", "f"},
            {"f_name", f_name},
        };
        json b = {
            {"deletefriend", "my"},
            {"f_name", my_name},
        };
        string message_1 = a.dump();
        string message_2 = b.dump();
        if (send(my_fd, message_1.c_str(), message_1.size(), 0) == -1)
            err_("删除好友失败");
        if (send(f_fd, message_2.c_str(), message_2.size(), 0) == -1)
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