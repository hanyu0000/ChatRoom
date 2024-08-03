#include "head.hpp"
#include "serv_main.hpp"
map<string, set<string>> friends_map;
map<string, set<string>> blocked_map;
bool are_friends(const string &name_my, const string &name_f);
void add_friendship(const string &name_my, const string &name_f);
void remove_friendship(const string &name_my, const string &name_f);
bool is_blocked(const string &name_my, const string &name_f);
void block_user(const string &name_my, const string &name_f);
void unblock_user(const string &name_my, const string &name_f);

void serv_main(int my_fd, const json &request, map<int, string> &client_map, RedisServer &redis)
{
    string my_name = get_name(my_fd, client_map);
    if (my_name == "NO")
        cout << "获取用户名错误" << endl;

    cout << "处理来自 <" << my_name << ">: " << my_fd << " 的请求: " << request.dump(3) << endl;
    // 查看好友列表
    if (request.contains("show.friend.list"))
    {
        json list_json;
        if (friends_map.find(my_name) != friends_map.end())
        {
            list_json =
                {
                    {"<manage>", "<manage>"},
                    {"show.friend.list", json::array()},
                };
            cout << my_name << "有好友记录" << endl;
            for (const auto &friendName : friends_map[my_name])
                list_json["show.friend.list"].push_back(friendName);
        }
        else
        {
            list_json =
                {
                    {"<manage>", "<manage>"},
                    {"show.friend.list", "NO"},
                };
            cout << my_name << "没有任何好友记录" << endl;
        }

        string str = list_json.dump();
        if (send(my_fd, str.c_str(), str.size(), 0) == -1)
            err_("发送好友列表失败");

        cout << "给客户端发送好友列表成功!" << endl;
        return;
    }
    string f_name = request["name"].get<string>();
    int f_fd = get_fd(f_name, client_map);
    // 好友不存在
    if (!redis.friends_exit(f_name))
    {
        string que = request["choice"].get<string>();
        json a =
            {
                {"<manage>", que},
                {"no_people", " "},
            };
        string str = a.dump();
        if (send(my_fd, str.c_str(), str.size(), 0) == -1)
            err_("发送好友不存在失败");
        cout << "发送好友不存在消息" << endl;
        return;
    }
    // 好友关系不存在
    if (redis.friends_exit(f_name) && !are_friends(my_name, f_name))
    {
        // 加好友
        if (request["choice"].get<string>() == "addfriend")
        {
            cout << "好友关系不存在，可以加好友！" << endl;
            // 发送好友申请到好友客户端
            json a = {
                {"new.friend", "newfriend"},
                {"my_name", my_name},
            };
            string message = a.dump();
            if (send(f_fd, message.c_str(), message.size(), 0) == -1)
                err_("发送好友申请失败");

            cout << "发送好友申请成功!" << endl;
            return;
        }
        // 回应加好友消息
        else if (request["choice"].get<string>() == "add_friendreply")
        {
            // 回应
            string reply = request["add_friendreply"].get<string>();
            json to_f = {
                {"new.friend.reply", reply},
                {"f_name", my_name},
            };
            json to_my = {
                {"new.friend.reply", reply},
                {"f_name", f_name},
            };
            string message1 = to_f.dump();
            string message2 = to_my.dump();
            if (reply == "YES")
            {
                add_friendship(my_name, f_name);
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
        else
        {
            string aaa = request["choice"].get<string>();
            json a =
                {
                    {"<manage>", aaa},
                    {"nofriend", " "},
                };
            string str = a.dump();
            if (send(my_fd, str.c_str(), str.size(), 0) == -1)
                err_("发送不是好友消息失败");
            cout << "还不是好友!" << endl;
            return;
        }
    }
    // 好友存在,好友关系存在
    if (redis.friends_exit(f_name) && are_friends(my_name, f_name))
    {
        // 屏蔽好友
        if (request["choice"].get<string>() == "blockfriend")
        {
            json a;
            if (is_blocked(my_name, f_name))
                a["choice"] = "blockfriend_already";
            else
            {
                block_user(my_name, f_name); // 加入屏蔽列表
                a["choice"] = "blockfriend_OK";
            }

            string message_str = a.dump();
            if (send(f_fd, message_str.c_str(), message_str.size(), 0) == -1)
                err_("屏蔽好友失败");
            return;
        }
        // 取消屏蔽好友
        if (request["choice"].get<string>() == "un_blockfriend")
        {
            json a;
            if (is_blocked(my_name, f_name))
            {
                unblock_user(my_name, f_name); // 从屏蔽列表取出
                a["choice"] = "unblockfriend_OK";
            }
            else
                a["choice"] = "not_blockfriend";

            string message_str = a.dump();
            if (send(f_fd, message_str.c_str(), message_str.size(), 0) == -1)
                err_("取消屏蔽好友失败");
            return;
        }
        // 删除好友
        else if (request["choice"].get<string>() == "deletefriend")
        {
            remove_friendship(my_name, f_name);
            json b = {
                {"choice", "deletefriend_success"},
                {"my_name", my_name},
                {"f_name", f_name},
            };
            string message_str = b.dump();
            // 同时向双方发送通知
            if (send(my_fd, message_str.c_str(), message_str.size(), 0) == -1)
                err_("删除好友失败");
            if (send(f_fd, message_str.c_str(), message_str.size(), 0) == -1)
                err_("删除好友失败");
            return;
        }
        // 好友聊天
        else if (request["choice"].get<string>() == "chatfriend")
        {
            return;
        }
    }

    if (request.contains("----------group.exit-------"))
    {
        // 群管理的逻辑
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

bool are_friends(const string &name_my, const string &name_f)
{
    // 检查 name_my 是否有好友 name_f
    if (friends_map.find(name_my) != friends_map.end())
        return friends_map[name_my].find(name_f) != friends_map[name_my].end();
    return false;
}

void add_friendship(const string &name_my, const string &name_f)
{
    friends_map[name_my].insert(name_f);
    friends_map[name_f].insert(name_my); // 确保好友关系是双向的
    cout << "用户: " << name_my << " 和用户: " << name_f << " 成为了好友" << endl;
}

void remove_friendship(const string &name_my, const string &name_f)
{
    if (friends_map.find(name_my) != friends_map.end())
    {
        friends_map[name_my].erase(name_f);
    }
    if (friends_map.find(name_f) != friends_map.end())
    {
        friends_map[name_f].erase(name_my);
    }
    cout << "用户: " << name_my << " 和用户: " << name_f << " 已经取消好友关系" << endl;
}

void block_user(const string &name_my, const string &name_f)
{
    blocked_map[name_my].insert(name_f);
}
void unblock_user(const string &name_my, const string &name_f)
{
    blocked_map[name_my].erase(name_f);
}
bool is_blocked(const string &name_my, const string &name_f)
{
    if (blocked_map.find(name_my) != blocked_map.end() &&
        blocked_map[name_my].find(name_f) != blocked_map[name_my].end())
    {
        return true;
    }
    return false;
}