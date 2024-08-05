#include "head.hpp"
#include "TUI.hpp"
int process_response(string &response, int fd);
typedef void (HHH::*Pointer)(int fd);
typedef struct HHH_table
{
    int choice;
    Pointer p;
} tab;
tab gtab[] = {
    {1, &HHH::message_s},
    {2, &HHH::f_chat},
    {3, &HHH::f_add},
    {4, &HHH::f_delete},
    {5, &HHH::f_block},
    {6, &HHH::f_unblock},

    {7, &HHH::g_showlist},
    {8, &HHH::g_showuser},
    {9, &HHH::g_join},
    {10, &HHH::g_leave},
    {11, &HHH::g_create},
    {12, &HHH::g_disband},
    {13, &HHH::exit},
};
int HHH_len = sizeof(gtab) / sizeof(gtab[0]);
void HHH::run(int fd)
{
    thread listener(&HHH::listen_serv, this, fd);
    int choice;
    while (running)
    {
        menu();
        cout << "" << endl;
        cout << "请选择：" << endl;
        if (cin >> choice)
        {
            bool flag = false;
            for (int i = 0; i < HHH_len; i++)
            {
                if (choice == gtab[i].choice)
                {
                    (this->*gtab[i].p)(fd); // 解引用成员函数指针,调用该成员函数
                    flag = true;
                    break;
                }
            }
            if (!flag)
                cout << "输入无效，请重新输入:" << endl;
        }
        else
        {
            cout << "输入无效，请重新输入:" << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
    listener.join();
}
void HHH::menu()
{
    cout << "" << endl;
    cout << "" << endl;
    cout << "----------------------------------------------------" << endl;
    cout << "           1.消息列表        2.好友聊天          " << endl;
    cout << "           3.添加好友        4.删除好友          " << endl;
    cout << "           5.屏蔽好友        6.取消屏蔽好友           " << endl;
    cout << "           7.查询群聊        8.查看群用户           " << endl;
    cout << "           9.加入群聊        10.退出群聊          " << endl;
    cout << "           11.创建群聊       12.解散群聊         " << endl;
    cout << "                     13.退出此页面          " << endl;
    cout << "----------------------------------------------------" << endl;
    cout << "" << endl;
    cout << "" << endl;
}

void HHH::message_s(int fd)
{
    while (running)
    {
        string message;
        {
            unique_lock<mutex> lock(queue_mutex);
            queue_cond.wait(lock, [this]
                            { return !message_queue.empty() || !running; });
            if (!running && message_queue.empty())
                break;
            message = message_queue.front();
            message_queue.pop();
        }
        if (process_response(message, fd))
            break;
    }
    return;
}
void HHH::listen_serv(int fd)
{
    while (running)
    {
        string buffer(128, '\0');
        int r = read(fd, &buffer[0], buffer.size());
        if (r > 0)
        {
            buffer.resize(r);
            {
                lock_guard<mutex> lock(queue_mutex);
                message_queue.push(buffer);
            }
            queue_cond.notify_one();
        }
        else if (r == 0)
        {
            cout << "服务器已关闭连接!" << endl;
            running = false;
            break;
        }
        else
        {
            cerr << "客户端读取数据失败!" << endl;
            break;
        }
    }
}

int process_response(string &response, int fd)
{
    try
    {
        json j = json::parse(response);
        cout << "处理消息: " << j.dump(3) << endl;
        if (j.contains("newfriend"))
        {
            string frind = j["newfriend"];
            if (j.contains("already"))
            {
                cout << "用户: " << frind << "和你已是好友关系！" << endl;
                return 1;
            }
            cout << "你有一条来自 < " << frind << " > 的好友申请" << endl;
            cout << "是否同意该用户的好友申请? T or F" << endl;
            char a;
            while (1)
            {
                cin >> a;
                json mas;
                if (a == 'T' || a == 't')
                {
                    mas =
                        {
                            {"choice", "newfriendreply"},
                            {"reply", "YES"},
                        };
                    string message = mas.dump();
                    if (send(fd, message.c_str(), message.size(), 0) == -1)
                        err_("send");
                    cout << "同意好友申请发送成功" << endl;
                    break;
                }
                else if (a == 'F' || a == 'f')
                {
                    mas =
                        {
                            {"choice", "newfriendreply"},
                            {"reply", "NO"},
                        };
                    string message = mas.dump();
                    if (send(fd, message.c_str(), message.size(), 0) == -1)
                        err_("send");
                    cout << "不同意好友申请发送成功" << endl;
                    break;
                }
                else
                    cout << "请按要求输入正确选项:" << endl;
            }
            return 1;
        }
        else if (j.contains("newfriendreply"))
        {
            string value = j["newfriendreply"];
            if (value == "YES")
            {
                string f_name = j["f_name"];
                cout << "您成功与用户: " << f_name << " 建立好友关系" << endl;
            }
            if (value == "NO")
            {
                string f_name = j["f_name"];
                cout << "用户:" << f_name << "不同意你的好友申请！" << endl;
            }
            return 1;
        }
        else if (j.contains("blockfriend"))
        {
            string reply = j["blockfriend"];
            string name = j["f_name"];
            if (reply == "already")
                cout << "用户: " << name << "已被你屏蔽！" << endl;
            else if (reply == "OK")
                cout << "已将用户: " << name << "屏蔽!" << endl;
            return 1;
        }
        else if (j.contains("unblockfriend"))
        {
            string reply = j["unblockfriend"];
            string name = j["f_name"];
            if (reply == "NO")
                cout << "你还未将用户: " << name << "屏蔽！" << endl;
            else if (reply == "OK")
                cout << "已将用户: " << name << "取消屏蔽！" << endl;
            return 1;
        }
        else if (j.contains("deletefriend"))
        {
            string reply = j["deletefriend"];
            string name = j["f_name"];
            if (reply == "f")
                cout << "你已将用户: " << name << "删除!" << endl;
            else if (reply == "my")
                cout << "用户: " << name << "已经把你删除！" << endl;
            return 1;
        }
        if (j.contains("chat")) // 聊天
        {
            string f_name = j["chat"];
            string message = j["message"];
            cout << f_name << " : " << message << endl;

            char a;
            cout << "      1.回消息       2.退出页面" << endl;
            while (1)
            {
                cin >> a;
                if (a == '1')
                {
                    cout << "请输入消息内容:" << endl;
                    cin.ignore();
                    string reply;
                    getline(cin, message);

                    json a =
                        {
                            {"<choice>", "chat"},
                            {"name", f_name},
                            {"message", reply},
                        };
                    string str = a.dump();
                    if (write(fd, str.c_str(), str.size()) == -1)
                        err_("write");
                }
                else if (a == '2')
                    break;
                else
                    cout << "请输入正确数字:" << endl;
            }
            return 1;
        }
        else if (j.contains("nopeople"))
        {
            string name = j["nopeople"];
            cout << "用户:" << name << "不存在，请重新输入正确的用户名!" << endl;
            return 1;
        }
        else
            cout << "hjkfgjkvgklmvfg" << endl;
    }
    catch (const json::parse_error &e)
    {
        cerr << "JSON 解析错误: " << e.what() << endl;
    }
    return 1;
}
// 好友聊天
void HHH::f_chat(int fd)
{
    cout << " " << endl;
    cout << "-------------------好友聊天-------------------" << endl;
    cout << " " << endl;
    json a =
        {
            {"<choice>", "chat"},
        };
    string str = a.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");
    return;
}
// 好友屏蔽
void HHH::f_block(int fd)
{
    cout << " " << endl;
    cout << "----------------好友屏蔽----------------" << endl;
    cout << " " << endl;
    string name;
    cout << "请输入您要屏蔽的好友名称:" << endl;
    cin.ignore();
    getline(cin, name);

    json a =
        {
            {"<choice>", "blockfriend"},
            {"name", name},
        };
    string str = a.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");
}
// 取消屏蔽
void HHH::f_unblock(int fd)
{
    cout << " " << endl;
    cout << "----------------取消屏蔽----------------" << endl;
    cout << " " << endl;
    string name;
    cout << "请输入您要取消屏蔽的好友名称:" << endl;
    cin.ignore();
    getline(cin, name);

    json a =
        {
            {"<choice>", "unblockfriend"},
            {"name", name},
        };
    string str = a.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");
}
// 好友添加
void HHH::f_add(int fd)
{
    cout << " " << endl;
    cout << "----------------添加好友----------------" << endl;
    cout << " " << endl;
    string name;
    cout << "请输入您的好友名称:" << endl;
    cin.ignore();
    getline(cin, name);

    json a =
        {
            {"choice", "addfriend"},
            {"name", name},
        };
    string str = a.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");
    cout << "发送好友申请成功！" << endl;
}
// 删除好友
void HHH::f_delete(int fd)
{
    cout << " " << endl;
    cout << "----------------删除好友----------------" << endl;
    cout << " " << endl;
    string name;
    cout << "请输入您的好友名称:" << endl;
    cin.ignore();
    getline(cin, name);

    json a =
        {
            {"<choice>", "deletefriend"},
            {"name", name},
        };
    string str = a.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");
}

// 创建群聊
void HHH::g_create(int fd)
{
    cout << " " << endl;
    cout << "-------------------创建群聊-------------------" << endl;
    cout << " " << endl;
    vector<string> friend_names;
    string name;

    cout << "请输入好友名称：" << endl;
    cin.ignore();
    while (1)
    {
        getline(cin, name);
        if (name == "\r")
            break;
        if (!name.empty())
            friend_names.push_back(name);
        else
            cout << "名称不能为空，请重新输入：" << endl;
    }

    json a =
        {
            {"<choice>", "create_group"},
            {"friend_nameS", friend_names},
        };
    string str = a.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");
}
// 查询群聊
void HHH::g_showlist(int fd)
{
    cout << " " << endl;
    cout << "-----------------群聊列表-----------------" << endl;
    cout << " " << endl;

    json a =
        {
            {"<choice>", "<manage>"},
            {"show.group.list", "----"},
        };
    string str = a.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");
}
// 加入群聊
void HHH::g_join(int fd)
{
    cout << " " << endl;
    cout << "-------------------加入群聊-------------------" << endl;
    cout << " " << endl;
    string group_name;
    cout << "请输入群聊名称:" << endl;
    cin.ignore();
    getline(cin, group_name);

    json a =
        {
            {"<choice>", "join_group"},
            {"group_name", group_name},
        };
    string str = a.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");
}
// 查看群用户
void HHH::g_showuser(int fd)
{
    cout << " " << endl;
    cout << "-------------------查看群用户-------------------" << endl;
    cout << " " << endl;
    string group_name;
    cout << "请输入群聊名称:" << endl;
    cin.ignore();
    getline(cin, group_name);

    json a =
        {
            {"<choice>", "show_group_users"},
            {"group_name", group_name},
        };
    string str = a.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");
}
// 退出群聊
void HHH::g_leave(int fd)
{
    cout << " " << endl;
    cout << "-------------------退出群聊-------------------" << endl;
    cout << " " << endl;
    string group_name;
    cout << "请输入群聊名称:" << endl;
    cin.ignore();
    getline(cin, group_name);

    json a =
        {
            {"<choice>", "leave_group"},
            {"group_name", group_name},
        };
    string str = a.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");
}
// 解散群聊
void HHH::g_disband(int fd)
{
    cout << " " << endl;
    cout << "-------------------解散群聊-------------------" << endl;
    cout << " " << endl;
    string group_name;
    cout << "请输入群聊名称:" << endl;
    cin.ignore();
    getline(cin, group_name);

    json a =
        {
            {"<choice>", "disband_group"},
            {"group_name", group_name},
        };
    string str = a.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");
}
void HHH::exit(int fd)
{
    running = false;
}