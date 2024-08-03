#include "head.hpp"
#include "TUI.hpp"
typedef void (HHH::*Pointer)(int fd);
typedef struct HHH_table
{
    int choice;
    Pointer p;
} tab;
tab gtab[] = {
    {1, &HHH::f_showlist},
    {2, &HHH::f_chat},
    {3, &HHH::f_add},
    {4, &HHH::f_delete},
    {5, &HHH::f_block},
    {6, &HHH::g_showlist},
    {7, &HHH::g_create},
    {8, &HHH::g_join},
    {9, &HHH::g_showuser},
    {10, &HHH::g_leave},
    {11, &HHH::g_disband},
};
int HHH_len = sizeof(gtab) / sizeof(gtab[0]);
void HHH::run(int fd)
{
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
}
void HHH::menu()
{
    cout << "" << endl;
    cout << "" << endl;
    cout << "----------------------------------------------------" << endl;
    cout << "           1.好友列表        2.好友聊天          " << endl;
    cout << "           3.添加好友        4.删除好友          " << endl;
    cout << "           5.屏蔽好友                           " << endl;
    cout << "           6.查询群聊        7.创建群聊           " << endl;
    cout << "           8.加入群聊        9.查看群用户          " << endl;
    cout << "           10.退出群聊       11.解散群聊         " << endl;
    cout << "----------------------------------------------------" << endl;
    cout << "" << endl;
    cout << "" << endl;
}
// 好友列表
void HHH::f_showlist(int fd)
{
    cout << " " << endl;
    cout << "-----------------好友列表-----------------" << endl;
    cout << " " << endl;

    json a =
        {
            {"----------manage------------", "manage"},
            {"----------show.friend.list-------", "----"},
        };
    string str = a.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");

    // string json_str = receiveFromServer(sock);
    // json friend_list_json = json::parse(json_str);
    // string name = friend_list_json["name"];
    // cout << name << "的好友列表:" << endl;
    // for (const auto &friend_name : friend_list_json["friends"])
    // cout << friend_name << endl;
    running = false;
}
// 好友聊天
void HHH::f_chat(int fd)
{
    cout << " " << endl;
    cout << "-------------------好友聊天-------------------" << endl;
    cout << " " << endl;
    string name;
    cout << "请输入您的好友名称:" << endl;
    cin.ignore();
    getline(cin, name);

    json a =
        {
            {"----------manage------------", "manage"},
            {"----------friend.exit-------", "----"}, // 好友是否存在,好友关系是否存在
            {"----------choice------------", "chatfriend"},
            {"name", name},
        };
    string str = a.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");

    running = false;
}
// 好友屏蔽
void HHH::f_block(int fd)
{
    cout << " " << endl;
    cout << "----------------好友屏蔽----------------" << endl;
    cout << " " << endl;
    string name;
    cout << "请输入您的好友名称:" << endl;
    cin.ignore();
    getline(cin, name);

    json a =
        {
            {"----------manage------------", "manage"},
            {"----------friend.exit-------", "----"},
            {"----------choice------------", "blockfriend"},
            {"name", name},
        };
    string str = a.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");

    running = false;
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
            {"----------manage------------", "manage"},
            {"----------friend.exit-------", "----"},
            {"----------choice------------", "addfriend"},
            {"name", name},
        };
    string str = a.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");

    running = false;
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
            {"----------manage------------", "manage"},
            {"----------friend.exit-------", "----"},
            {"----------choice------------", "deletefriend"},
            {"name", name},
        };
    string str = a.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");

    running = false;
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
            {"----------manage------------", "manage"},
            {"----------choice------------", "create_group"},
            {"friend_nameS", friend_names},
        };
    string str = a.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");

    running = false;
}
// 查询群聊
void HHH::g_showlist(int fd)
{
    cout << " " << endl;
    cout << "-----------------群聊列表-----------------" << endl;
    cout << " " << endl;

    json a =
        {
            {"----------manage------------", "manage"},
            {"----------show.group.list-------", "----"},
        };
    string str = a.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");

    running = false;
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
            {"----------manage------------", "manage"},
            {"----------group.exit-------", group_name},
            {"----------choice------------", "join_group"},
            {"group_name", group_name},
        };
    string str = a.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");

    running = false;
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
            {"----------manage------------", "manage"},
            {"----------group.exit-------", group_name},
            {"----------choice------------", "show_group_users"},
            {"group_name", group_name},
        };
    string str = a.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");

    running = false;
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
            {"----------manage------------", "manage"},
            {"----------group.exit-------", group_name},
            {"----------choice------------", "leave_group"},
            {"group_name", group_name},
        };
    string str = a.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");

    running = false;
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
            {"----------manage------------", "manage"},
            {"----------group.exit-------", group_name},
            {"----------choice------------", "disband_group"},
            {"group_name", group_name},
        };
    string str = a.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");

    running = false;
}