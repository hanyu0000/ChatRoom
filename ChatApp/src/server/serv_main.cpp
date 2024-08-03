#include "head.hpp"
#include "serv_main.hpp"

map<string, set<string>> friends_map;
map<string, set<string>> blocked_map;
void add_friendship(const string &name_my, const string &name_f);
bool are_friends(const string &name_my, const string &name_f);
void remove_friendship(const string &name_my, const string &name_f);
void block_user(const string &name_my, const string &name_f);
void unblock_user(const string &name_my, const string &name_f);
bool is_blocked(const string &name_my, const string &name_f);

void serv_main(int my_fd, const json &request, map<int, string> &client_map, RedisServer &redisServer)
{
    string my_name = get_name(my_fd, client_map);
    if (my_name == "NO")
        cout << "获取用户名错误" << endl;
    else
        cout << "处理来自 <" << my_name << ">: " << my_fd << " 的请求: " << request.dump(3) << endl;

    if (request["----------friend.exit-------"])
    {
        string name = request["name"].get<string>();
        if (redisServer.friends_exit(name)) // 好友存在
        {
            string f_name = request["name"].get<string>();
            int f_fd = get_fd(f_name, client_map);
            if (are_friends(my_name, f_name)) // 好友关系存在
            {
                if (request["----------choice------------"].get<string>() == "blockfriend")
                {
                    json a;
                    if (is_blocked(my_name, f_name)) // 已经屏蔽好友
                        a["----------choice------------"] = "blockfriend_already";
                    else
                    {
                        a["----------choice------------"] = "blockfriend_success";
                        block_user(my_name, f_name);
                    }
                    string message_str = a.dump();
                    if (send(f_fd, message_str.c_str(), message_str.size(), 0) == -1)
                        err_("屏蔽好友失败");
                }
                else if (request["----------choice------------"].get<string>() == "deletefriend")
                {
                    remove_friendship(my_name, f_name);// 删除好友
                    json b = {
                        {"----------choice------------", "deletefriend_success"},
                        {"my_name", my_name},
                        {"f_name", f_name},
                    };
                    string message_str = b.dump();
                    // 同时向双方发送通知
                    if (send(my_fd, message_str.c_str(), message_str.size(), 0) == -1)
                        err_("删除好友失败");
                    if (send(f_fd, message_str.c_str(), message_str.size(), 0) == -1)
                        err_("删除好友失败");
                }
                else if (request["----------choice------------"].get<string>() == "chatfriend")
                { // 好友聊天
                }
            }
            else // 好友关系不存在
            {
                // 发送好友申请到好友客户端
                json u_i = {
                    {"---------new.friend-------", "add-friend"},
                    {"my_name", my_name},
                };
                string message_str = u_i.dump();
                if (send(f_fd, message_str.c_str(), message_str.size(), 0) == -1)
                    err_("发送好友申请失败");

                // 接受or拒绝
                string reply = request["reply"].get<string>();
                json p_to_p = {
                    {"---------new.friend-------", reply},
                    {"my_name", my_name},
                    {"f_name", f_name},
                };
                string message_str = u_i.dump();
                // 同时向双方发送好友通知
                if (send(my_fd, message_str.c_str(), message_str.size(), 0) == -1)
                    err_("send_my");
                if (send(f_fd, message_str.c_str(), message_str.size(), 0) == -1)
                    err_("send_f");

                if (reply == "YES")
                    // 维护一个好友关系
                    add_friendship(my_name, f_name);
                else if (reply == "NO")
                    cout << "维护好友关系错误!" << endl;
            }
        }
        // 好友不存在
        else
        {
            json a =
                {
                    {"----------friend.exit-------", "该用户不存在，请输入正确的用户名！"},
                    {"name", name},
                };
            string str = a.dump();
            if (send(my_fd, str.c_str(), str.size(), 0) == -1)
                err_("发送好友不存在失败");
        }
    }
    else if (request.contains("----------show.friend.list-------")) // 好友列表
    {
        json response_json;
        if (friends_map.find(my_name) != friends_map.end())
        {
            cout << my_name << "的好友列表:" << endl;
            response_json =
                {
                    {"----------manage------------", "manage"},
                    {"----------show.friend.list-------", "----"},
                    {"friends", friends_map[my_name]},
                };
            for (const auto &friendName : friends_map[my_name])
                cout << friendName << endl;
        }
        else
        {
            response_json =
                {
                    {"----------manage------------", "manage"},
                    {"----------show.friend.list-------", "----"},
                    {"friends", "NoFriends"},
                };
            cout << my_name << "没有任何好友记录." << endl;
        }
        string str = response_json.dump();
        if (send(my_fd, str.c_str(), str.size(), 0) == -1)
            err_("发送好友列表失败");
    }
    else if (request.contains("----------group.exit-------"))
    {
        // 群管理的逻辑
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

bool are_friends(const string &name_my, const string &name_f)
{
    // 检查 name_my 是否有好友 name_f
    if (friends_map.find(name_my) != friends_map.end())
        return friends_map[name_my].find(name_f) != friends_map[name_my].end();
    return false;
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