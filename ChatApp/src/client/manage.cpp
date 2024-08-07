#include "head.hpp"
#include "HHH.hpp"
// 消息列表
void HHH::message(int fd)
{
    string buffer;
    if (Util::recv_msg(fd, buffer) == -1)
        err_("recv_msg");
    json j = json::parse(buffer);
    cout << buffer << endl;
}
// 好友聊天
void HHH::f_chat(int fd)
{
    cout << " " << endl;
    cout << "-------------------好友聊天-------------------" << endl;
    cout << " " << endl;
    // 请求好友列表
    json request =
        {
            {"type", "chatlist"}};
    string request_str = request.dump();
    if (Util::send_msg(fd, request_str) == -1)
        err_("send_msg");
    // 接收好友列表
    string buffer;
    if (Util::recv_msg(fd, buffer) == -1)
        err_("Util::recv_msg");
    json j = json::parse(buffer);
    if (j.contains("chatlist"))
    {
        cout << "您的好友列表:" << endl;
        vector<string> chatlist = j["chatlist"];
        for (const auto &name : chatlist)
            cout << name << endl;
    }
    // 输入要聊天的好友
    cout << "请输入你要聊天的好友:" << endl;
    string name;
    cin >> name;
    // 发送聊天消息
    json message = {
        {"type", "chat"},
        {"name", name},
        {"message", "hello!"}}; 
    string message_str = message.dump();
    if (Util::send_msg(fd, message_str) == -1)
        err_("send_msg");
}
// 好友申请
void HHH::f_reply(int fd)
{
    cout << " " << endl;
    cout << "-------------------好友申请-------------------" << endl;
    cout << " " << endl;

    string buffer;
    if (Util::recv_msg(fd, buffer) == -1)
        err_("recv_msg");
    json j = json::parse(buffer);
    if (j.contains("newfriend"))
    {
        string name = j["newfriend"];
        cout << "你有一条来自:" << name << "的好友申请!" << endl;
        cout << "你是否同意该请求: T or F" << endl;
        char a;
        cin >> a;
        while (1)
        {
            if (a == 't' || a == 'T')
            {
                json request =
                    {
                        {"type", "addfriendreply"},
                        {"name", name},
                        {"reply", "YES"}};
                string request_str = request.dump();
                if (Util::send_msg(fd, request_str) == -1)
                    err_("send_msg");
                break;
            }
            else if (a == 'f' || a == 'F')
            {
                json request =
                    {
                        {"type", "addfriendreply"},
                        {"name", name},
                        {"reply", "NO"}};
                string request_str = request.dump();
                if (Util::send_msg(fd, request_str) == -1)
                    err_("send_msg");
                break;
            }
            else
                cout << "请输入正确选项:" << endl;
        }
    }
    else if (j.contains("newfriendreply"))
    {
        string reply = j["newfriendreply"];
        string name = j["newfriend"];
        cout << name << reply << "你的好友申请" << endl;
    }
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
            {"type", "blockfriend"},
            {"name", name}};
    string str = a.dump();
    if (Util::send_msg(fd, str) == -1)
        err_("Util::send_msg");
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
            {"type", "unblockfriend"},
            {"name", name}};
    string str = a.dump();
    if (Util::send_msg(fd, str) == -1)
        err_("Util::send_msg");
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
            {"type", "addfriend"},
            {"name", name}};
    string str = a.dump();
    if (Util::send_msg(fd, str) == -1)
        err_("send_msg");
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
            {"type", "deletefriend"},
            {"name", name}};
    string str = a.dump();
    if (Util::send_msg(fd, str) == -1)
        err_("send_msg");
    
}
// 创建群聊(群主)
void HHH::g_create(int fd)
{
    cout << " " << endl;
    cout << "-------------------创建群聊-------------------" << endl;
    cout << " " << endl;
    string group;
    string input;
    vector<string> Friends;

    cout << "请输入群聊名称:" << endl;
    cin.ignore();
    getline(cin, group);

    cout << "请输入要添加到群聊的好友名（多个好友用逗号分隔）:" << endl;
    getline(cin, input);

    size_t pos = 0;
    while ((pos = input.find(',')) != string::npos)
    {
        string friendName = input.substr(0, pos);
        Friends.push_back(friendName);
        input.erase(0, pos + 1);
    }
    if (!input.empty())
        Friends.push_back(input);

    json Json = {
        {"type", "create_group"},
        {"group", group},
        {"members", Friends}};
    string message = Json.dump();
    if (Util::send_msg(fd, message) == -1)
    {
        cerr << "发送消息失败" << endl;
        return;
    }

    // 接收服务器响应
    string response;
    if (Util::recv_msg(fd, response) == -1)
    {
        cerr << "接收消息失败" << endl;
        return;
    }
    try
    {
        json j = json::parse(response);
        if (j.contains("type") && j["type"] == "create_group")
        {
            string status = j["status"];
            if (status == "success")
                cout << "群聊创建成功!" << endl;
            else
                cout << "群聊创建失败!" << endl;
        }
    }
    catch (const json::parse_error &e)
    {
        cerr << "JSON 解析失败: " << e.what() << endl;
    }
}
// 查看群用户
void HHH::g_showuser(int fd)
{
    cout << " " << endl;
    cout << "-------------------查看群用户-------------------" << endl;
    cout << " " << endl;
    // 展示群聊列表
    json glist =
        {
            {"type", "grouplist"}};
    string list = glist.dump();
    if (Util::send_msg(fd, list) == -1)
        err_("Util::send_msg");

    // 接收群聊列表
    string buffer;
    if (Util::recv_msg(fd, buffer) == -1)
        err_("Util::recv_msg");

    json j = json::parse(buffer);
    if (j.contains("grouplist"))
    {
        cout << "您的群聊列表:" << endl;
        vector<string> grouplist = j["grouplist"];
        for (const auto &name : grouplist)
            cout << name << endl;
    }

    cout << "输入你要查看的群聊:" << endl;
    string group;
    cin >> group;
    json g =
        {
            {"type", "userlist"},
            {"group", group}};
    string str = g.dump();
    if (Util::send_msg(fd, str) == -1)
        err_("Util::send_msg");

    // 接收群用户列表
    string buf;
    if (Util::recv_msg(fd, buf) == -1)
        err_("Util::recv_msg");

    json i = json::parse(buf);
    if (i.contains("userlist"))
    {
        cout << "群用户列表:" << endl;
        vector<string> userlist = i["userlist"];
        for (const auto &name : userlist)
            cout << name << endl;
    }
}
// 查询群聊
void HHH::g_showlist(int fd)
{
    cout << " " << endl;
    cout << "-----------------群聊列表-----------------" << endl;
    cout << " " << endl;
    // 发送请求
    json request = {
        {"type", "grouplist"}};
    string str = request.dump();
    if (Util::send_msg(fd, str) == -1)
        err_("Util::send_msg");

    // 接收并处理响应
    string buffer;
    if (Util::recv_msg(fd, buffer) == -1)
        err_("Util::recv_msg");

    json response = json::parse(buffer);
    if (response.contains("grouplist"))
    {
        cout << "您的群聊列表:" << endl;
        vector<string> grouplist = response["grouplist"];
        for (const auto &name : grouplist)
            cout << name << endl;
    }
}
// 设置管理员(群主)
void HHH::g_add_manager(int fd)
{
    cout << " " << endl;
    cout << "-------------------设置管理员-------------------" << endl;
    cout << " " << endl;
    json mylist =
        {
            {"type", "mygrouplist"}};
    string a = mylist.dump();
    if (write(fd, a.c_str(), a.size()) == -1)
        err_("write");

    string buffer;
    int r = read(fd, &buffer[0], buffer.size());
    buffer.resize(r);
    json j = json::parse(buffer);
    if (j.contains("mygrouplist"))
    {
        cout << "您的群聊列表:" << endl;
        vector<string> mygrouplist = j["mygrouplist"];
        for (const auto &name : mygrouplist)
            cout << name << endl;
    }
    // 设置管理员
    string group;
    string input;
    vector<string> Friends;
    cout << "输入你要设置的群聊名称:" << endl;
    cin.ignore();
    getline(cin, group);

    json ulist =
        {
            {"type", "userlist"},
            {"group", group}};
    string b = ulist.dump();
    if (write(fd, b.c_str(), b.size()) == -1)
        err_("write");

    string buf;
    r = read(fd, &buf[0], buf.size());
    buffer.resize(r);
    json i = json::parse(buf);
    if (i.contains("userlist"))
    {
        cout << "群用户列表:" << endl;
        vector<string> userlist = i["userlist"];
        for (const auto &name : userlist)
            cout << name << endl;
    }

    cout << "输入你设置的管理员名称:" << endl;
    getline(cin, input);
    size_t pos = 0;
    while ((pos = input.find(',')) != string::npos)
    {
        string friendName = input.substr(0, pos);
        Friends.push_back(friendName);
        input.erase(0, pos + 1);
    }
    json Json = {
        {"type", "addmanage"},
        {"group", group},
        {"members", Friends}};
    string message = Json.dump();
    if (send(fd, message.c_str(), message.size(), 0) == -1)
        cerr << "发送消息失败" << endl;
    cout << "是否退出页面:" << endl;
}
// 删除管理员(群主)
void HHH::g_delete_manager(int fd)
{
    cout << " " << endl;
    cout << "-------------------删除管理员-------------------" << endl;
    cout << " " << endl;
    json mylist =
        {
            {"type", "mygrouplist"}};
    string a = mylist.dump();
    if (write(fd, a.c_str(), a.size()) == -1)
        err_("write");

    string buffer;
    int r = read(fd, &buffer[0], buffer.size());
    buffer.resize(r);
    json j = json::parse(buffer);
    if (j.contains("mygrouplist"))
    {
        cout << "您的群聊列表:" << endl;
        vector<string> mygrouplist = j["mygrouplist"];
        for (const auto &name : mygrouplist)
            cout << name << endl;
    }
    // 删除管理员
    string group;
    string input;
    vector<string> Friends;
    cout << "输入你要设置的群聊名称:" << endl;
    cin.ignore();
    getline(cin, group);

    json ulist =
        {
            {"type", "userlist"},
            {"group", group}};
    string b = ulist.dump();
    if (write(fd, b.c_str(), b.size()) == -1)
        err_("write");

    string buf;
    r = read(fd, &buf[0], buf.size());
    buffer.resize(r);
    json i = json::parse(buf);
    if (i.contains("userlist"))
    {
        cout << "群用户列表:" << endl;
        vector<string> userlist = i["userlist"];
        for (const auto &name : userlist)
            cout << name << endl;
    }

    cout << "输入你要删除的管理员名称:" << endl;
    getline(cin, input);
    size_t pos = 0;
    while ((pos = input.find(',')) != string::npos)
    {
        string friendName = input.substr(0, pos);
        Friends.push_back(friendName);
        input.erase(0, pos + 1);
    }
    json Json = {
        {"type", "deletemanage"},
        {"group", group},
        {"members", Friends}};
    string message = Json.dump();
    if (send(fd, message.c_str(), message.size(), 0) == -1)
        cerr << "发送消息失败" << endl;
    cout << "是否退出页面:" << endl;
}
// 移除群用户(群主/管理员)
void HHH::g_delete_people(int fd)
{
    cout << " " << endl;
    cout << "-------------------移除群用户-------------------" << endl;
    cout << " " << endl;
    json mylist =
        {
            {"type", "mygrouplist"}};
    string a = mylist.dump();
    if (write(fd, a.c_str(), a.size()) == -1)
        err_("write");

    string buffer;
    int r = read(fd, &buffer[0], buffer.size());
    buffer.resize(r);
    json j = json::parse(buffer);
    if (j.contains("mygrouplist"))
    {
        cout << "您的群聊列表:" << endl;
        vector<string> mygrouplist = j["mygrouplist"];
        for (const auto &name : mygrouplist)
            cout << name << endl;
    }

    string group;
    string input;
    vector<string> Friends;
    cout << "输入你要设置的群聊名称:" << endl;
    cin.ignore();
    getline(cin, group);

    json b =
        {
            {"type", "g_people"}, // 普通成员
            {"group", group}};
    string str = b.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");

    string buf;
    r = read(fd, &buf[0], buf.size());
    buf.resize(r);
    json i = json::parse(buf);
    if (i.contains("managelist"))
    {
        cout << "您的管理列表:" << endl;
        vector<string> people_list = i["people_list"];
        for (const auto &name : people_list)
            cout << name << endl;
    }
    cout << "输入你要删除的用户名称:" << endl;
    getline(cin, input);
    size_t pos = 0;
    while ((pos = input.find(',')) != string::npos)
    {
        string friendName = input.substr(0, pos);
        Friends.push_back(friendName);
        input.erase(0, pos + 1);
    }
    json Json = {
        {"type", "deletemanage"},
        {"group", group},
        {"members", Friends}};
    string message = Json.dump();
    if (send(fd, message.c_str(), message.size(), 0) == -1)
        cerr << "发送消息失败" << endl;
    cout << "是否退出页面:" << endl;
}
// 解散群聊(群主)
void HHH::g_disband(int fd)
{
    cout << " " << endl;
    cout << "-------------------解散群聊-------------------" << endl;
    cout << " " << endl;
    json mylist =
        {
            {"type", "mygrouplist"}};
    string a = mylist.dump();
    if (write(fd, a.c_str(), a.size()) == -1)
        err_("write");

    string buffer;
    int r = read(fd, &buffer[0], buffer.size());
    buffer.resize(r);
    json j = json::parse(buffer);
    if (j.contains("mygrouplist"))
    {
        cout << "您的群聊列表:" << endl;
        vector<string> mygrouplist = j["mygrouplist"];
        for (const auto &name : mygrouplist)
            cout << name << endl;
    }

    string group;
    cout << "请输入要解散的群聊名称:" << endl;
    cin.ignore();
    getline(cin, group);

    json Json = {
        {"type", "disband_group"},
        {"group", group}};
    string str = Json.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");
}
// 加入群聊
void HHH::g_join(int fd)
{
    cout << " " << endl;
    cout << "-------------------加入群聊-------------------" << endl;
    cout << " " << endl;
    string group;
    cout << "请输入要加入的群聊名称:" << endl;
    cin.ignore();
    getline(cin, group);

    json Json = {
        {"type", "join_group"},
        {"group", group}};
    string str = Json.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");
}
// 退出群聊
void HHH::g_leave(int fd)
{
    cout << " " << endl;
    cout << "-------------------退出群聊-------------------" << endl;
    cout << " " << endl;
    string group;
    cout << "请输入要退出的群聊名称:" << endl;
    cin.ignore();
    getline(cin, group);

    json Json = {
        {"type", "leave_group"},
        {"group", group}};
    string str = Json.dump();
    if (Util::send_msg(fd, str) == -1)
    {
        cerr << "发送消息失败" << endl;
        return;
    }
}