#include "head.hpp"
#include "HHH.hpp"
void show_list(int fd); 
void getmygrouplist(int fd);
// 群聊
void HHH::g_chat(int fd)
{
    cout << " " << endl;
    cout << "-------------------群聊-------------------" << endl;
    cout << " " << endl;

    g_showlist(fd);

    string group;
    cout << "输入你要聊天的群聊：" << endl;
    cin.ignore();
    getline(cin, group);
    if (group.empty())
        return;

    cout << "是否需要拉取历史消息? Y or N" << endl;
    char a;
    while (1)
    {
        cin >> a;
        if (a == 'Y' || a == 'y')
        {
            json jjj = {
                {"type", "g_chatHistry"},
                {"group", group}};
            string mmm = jjj.dump();
            if (IO::send_msg(fd, mmm) == -1)
                cerr << "发送消息失败" << endl;

            string buffer;
            if (IO::recv_msg(fd, buffer) == -1)
                err_("recv_msg");
            json jj = json::parse(buffer);
            if (jj.contains("g_chatHistry"))
            {
                vector<string> re = jj["g_chatHistry"];
                if (re.empty())
                {
                    cout << "没有历史消息" << endl;
                    break;
                }
                for (const auto &message : re)
                    cout << message << endl;
            }
            break;
        }
        else if (a == 'N' || a == 'n')
            break;
    }

    getchar();

    thread recvThread;
    // 接收消息的线程函数
    auto receiveMessages = [&]()
    {
        while (!g_stop.load())
        {
            string buffer;
            cout << buffer << endl;
            if (IO::recv_msg(fd, buffer) == -1)
            {
                cerr << "接收消息失败: " << errno << " (" << strerror(errno) << ")" << endl;
                break;
            }
            try
            {
                json j = json::parse(buffer);
                string reply = j["g_chat"];
                if (reply == "exit")
                {
                    g_stop.store(true);
                    break;
                }
                if (j.contains("g_chat"))
                    cout << reply << endl;
            }
            catch (const json::parse_error &e)
            {
                cerr << "JSON 解析错误: " << e.what() << endl;
            }
        }
    };

    recvThread = thread(receiveMessages);
    string msg;
    cout << "请输入聊天消息( 'exit' 结束): " << endl;

    json m = {
        {"type", "chat"},
        {"group", group},
        {"message", "12345zxcvb"}};
    string m_str = m.dump();
    if (IO::send_msg(fd, m_str) == -1)
        cerr << "发送消息失败" << endl;

    while (1)
    {
        cin.ignore();
        getline(cin, msg);
        if (msg == "")
            continue;
        if (msg == "exit")
        {
            cout << "聊天结束......" << endl;
            json message = {
                {"type", "g_chat"},
                {"group", group},
                {"message", msg}};
            string mess = message.dump();
            if (IO::send_msg(fd, mess) == -1)
                cerr << "发送消息失败" << endl;
            break;
        }

        // 发送消息
        json message = {
            {"type", "g_chat"},
            {"group", group},
            {"message", msg}};
        string mess = message.dump();

        if (mess.length() >= 1024)
        {
            cout << "消息太长!请重新输入:" << endl;
            continue;
        }
        if (IO::send_msg(fd, mess) == -1)
            cerr << "发送消息失败" << endl;
    }
    if (recvThread.joinable())
        recvThread.join();
    getchar();
}
// 设置管理员(群主)
void HHH::g_add_manager(int fd)
{
    cout << " " << endl;
    cout << "-------------------设置管理员-------------------" << endl;
    cout << " " << endl;

    getmygrouplist(fd);

    // 设置管理员
    string group;
    string input;
    vector<string> Friends;

    cout << "输入你要设置的群聊名称:" << endl;
    cin.ignore();
    getline(cin, group);
    if (group.empty())
        return;
    json ulist =
        {
            {"type", "userlist"},
            {"group", group}};
    string b = ulist.dump();
    if (IO::send_msg(fd, b) == -1)
        cerr << "发送消息失败" << endl;
    // 获取用户列表
    string buffer;
    if (IO::recv_msg(fd, buffer) == -1)
        err_("recv_msg");
    json j = json::parse(buffer);
    if (j.contains("userlist"))
    {
        cout << "您的群聊用户列表:" << endl;
        vector<string> list = j["userlist"];
        if (list.empty())
        {
            cout << "该群聊用户列表为空！" << endl;
            return;
        }
        for (const auto &name : list)
            cout << name << endl;
    }
    cout << "输入你设置的管理员名称(','分隔):" << endl;
    getline(cin, input);
    if (input.empty())
        return;
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
        {"type", "addmanage"},
        {"group", group},
        {"members", Friends}};
    string message = Json.dump();
    if (IO::send_msg(fd, message) == -1)
        cerr << "发送消息失败" << endl;
    getchar();
}
// 删除管理员(群主)
void HHH::g_delete_manager(int fd)
{
    cout << " " << endl;
    cout << "-------------------删除管理员-------------------" << endl;
    cout << " " << endl;

    getmygrouplist(fd);

    string group;
    string input;
    vector<string> Friends;
    cout << "输入你要设置的群聊名称:" << endl;
    cin.ignore();
    getline(cin, group);
    if (group.empty())
        return;

    json ulist =
        {
            {"type", "managelist"},
            {"group", group}};
    string b = ulist.dump();
    if (IO::send_msg(fd, b) == -1)
        cerr << "发送消息失败" << endl;
    // 获取管理员列表
    string buffer;
    if (IO::recv_msg(fd, buffer) == -1)
        err_("recv_msg");
    json j = json::parse(buffer);
    if (j.contains("managelist"))
    {
        cout << "您的管理员列表:" << endl;
        vector<string> mygrouplist = j["managelist"];
        if (mygrouplist.empty())
        {
            cout << "该群聊管理员列表为空！" << endl;
            return;
        }
        for (const auto &name : mygrouplist)
            cout << name << endl;
    }

    cout << "输入你要删除的管理员名称(','分隔):" << endl;
    getline(cin, input);
    if (input.empty())
        return;
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
        {"type", "deletemanage"},
        {"group", group},
        {"members", Friends}};
    string message = Json.dump();
    if (IO::send_msg(fd, message) == -1)
        cerr << "发送消息失败" << endl;
    getchar();
}
// 移除群用户(群主/管理员)
void HHH::g_delete_people(int fd)
{
    cout << " " << endl;
    cout << "-------------------移除群用户-------------------" << endl;
    cout << " " << endl;
    g_showlist(fd);
    cout << " " << endl;
    cout << "输入你要查看的群聊:" << endl;
    string group;
    cin >> group;
    if (group.empty())
        return;

    json g =
        {
            {"type", "userlist"},
            {"group", group}};
    string str = g.dump();
    if (IO::send_msg(fd, str) == -1)
        err_("send_msg");
    // 接收群用户列表
    string buf;
    if (IO::recv_msg(fd, buf) == -1)
        err_("recv_msg");
    json i = json::parse(buf);
    if (i.contains("userlist"))
    {
        cout << "群用户列表:" << endl;
        vector<string> userlist = i["userlist"];
        if (userlist.empty())
        {
            cout << "该群聊用户列表为空！" << endl;
            return;
        }
        for (const auto &name : userlist)
            cout << name << endl;
    }

    cout << "输入你要删除的用户名称:" << endl;
    string name;
    getchar();
    cin >> name;
    if (name.empty())
        return;

    json Json = {
        {"type", "delete_people"},
        {"group", group},
        {"name", name}};
    string message = Json.dump();
    if (IO::send_msg(fd, message) == -1)
        cerr << "发送消息失败" << endl;

    // 接受回应
    string buffer;
    if (IO::recv_msg(fd, buffer) == -1)
        err_("recv_msg");
    json j = json::parse(buffer);
    string reply = j["type"];
    if (reply == "delete_people")
        cout << "您还没有权限！" << endl;
    else if (reply == "OK")
        cout << "删除用户成功！" << endl;
    getchar();
}
// 群聊申请
void HHH::g_reply(int fd)
{
    cout << " " << endl;
    cout << "-------------------入群申请-------------------" << endl;
    cout << " " << endl;

    json age = {
        {"type", "g_reply"}};
    string mess = age.dump();
    if (IO::send_msg(fd, mess) == -1)
        cerr << "发送消息失败" << endl;

    while (1)
    {
        string buf;
        if (IO::recv_msg(fd, buf) == -1)
            err_("recv_msg");

        json jj = json::parse(buf);
        if (jj.contains("g_reply"))
        {
            string name = jj["g_reply"];
            if (name == "NO")
            {
                cout << "没有新的入群申请" << endl;
                break;
            }
            string group = jj["group"];
            cout << "你有一条来自:" << name << "加入<" << group << ">群申请!" << endl;
            cout << "你是否同意该请求: T or F" << endl;
            char a;
            cin >> a;
            while (1)
            {
                if (a == 't' || a == 'T')
                {
                    json request =
                        {
                            {"type", "apply_g_reply"},
                            {"name", name},
                            {"group", group},
                            {"reply", "YES"}};
                    string request_str = request.dump();
                    if (IO::send_msg(fd, request_str) == -1)
                        err_("send_msg");
                    break;
                }
                else if (a == 'f' || a == 'F')
                {
                    json request =
                        {
                            {"type", "apply_g_reply"},
                            {"name", name},
                            {"reply", "NO"}};
                    string request_str = request.dump();
                    if (IO::send_msg(fd, request_str) == -1)
                        err_("send_msg");
                    break;
                }
                else
                    cout << "请输入正确选项:" << endl;
            }
        }
    }

    getchar();
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
    if (group.empty())
        return;

    json Json = {
        {"type", "join_group"},
        {"group", group}};
    string str = Json.dump();
    if (IO::send_msg(fd, str) == -1)
        cerr << "发送消息失败" << endl;

    string buf;
    if (IO::recv_msg(fd, buf) == -1)
        err_("recv_msg");
    json j = json::parse(buf);
    string reply = j["group"];
    if (reply == "NO")
    {
        cout << "该群聊不存在！" << endl;
        return;
    }
    cout << "发送群聊申请成功!" << endl;
    getchar();
    getchar();
}
// 查看群用户
void HHH::g_showuser(int fd)
{
    cout << " " << endl;
    cout << "-------------------查看群用户-------------------" << endl;
    cout << " " << endl;

    g_showlist(fd);
    cout << " " << endl;

    cout << "输入你要查看的群聊:" << endl;
    string group;
    cin >> group;
    if (group.empty())
        return;
    json g =
        {
            {"type", "userlist"},
            {"group", group}};
    string str = g.dump();
    if (IO::send_msg(fd, str) == -1)
        err_("send_msg");
    // 接收群用户列表
    string buf;
    if (IO::recv_msg(fd, buf) == -1)
        err_("recv_msg");
    json i = json::parse(buf);
    if (i.contains("userlist"))
    {
        cout << "群用户列表:" << endl;
        vector<string> userlist = i["userlist"];
        if (userlist.empty())
        {
            cout << "该群聊用户列表为空！" << endl;
            return;
        }
        for (const auto &name : userlist)
            cout << name << endl;
    }
    getchar();
    getchar();
}
// 查询群聊
void HHH::g_showlist(int fd)
{
    json request = {
        {"type", "grouplist"}};
    string str = request.dump();
    if (IO::send_msg(fd, str) == -1)
        err_("send_msg");
    // 接收并处理响应
    string buffer;
    if (IO::recv_msg(fd, buffer) == -1)
        err_("recv_msg");

    json response = json::parse(buffer);
    if (response.contains("grouplist"))
    {
        cout << "您的群聊列表:" << endl;
        vector<string> grouplist = response["grouplist"];
        if (grouplist.empty())
        {
            cout << "您的群聊列表为空！" << endl;
            return;
        }
        for (const auto &name : grouplist)
            cout << name << endl;
    }
    getchar();
}
// 退出群聊
void HHH::g_leave(int fd)
{
    cout << " " << endl;
    cout << "-------------------退出群聊-------------------" << endl;
    cout << " " << endl;
    g_showlist(fd);
    string group;
    cout << "请输入要退出的群聊名称:" << endl;
    getline(cin, group);
    if (group.empty())
        return;

    json Json = {
        {"type", "leave_group"},
        {"group", group}};
    string str = Json.dump();
    if (IO::send_msg(fd, str) == -1)
        cerr << "发送消息失败" << endl;
    cout << "您成功退出该群聊！" << endl;
    getchar();
}
// 我创建的群聊列表
void getmygrouplist(int fd)
{
    json mylist =
        {
            {"type", "mygrouplist"}};
    string reply = mylist.dump();
    if (IO::send_msg(fd, reply) == -1)
        cerr << "发送消息失败" << endl;
    // 接收列表
    string buffer;
    if (IO::recv_msg(fd, buffer) == -1)
        err_("recv_msg");

    json j = json::parse(buffer);
    if (j.contains("mygrouplist"))
    {
        cout << "您的群聊列表:" << endl;
        vector<string> mygrouplist = j["mygrouplist"];
        if (mygrouplist.empty())
        {
            cout << "您的群聊列表为空！" << endl;
            return;
        }
        for (const auto &name : mygrouplist)
            cout << name << endl;
    }
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
    if (group.empty())
        return;

    show_list(fd);

    cout << "请输入要添加到群聊的好友名（多个好友用逗号分隔）:" << endl;
    getline(cin, input);
    if (input.empty())
        return;

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
    if (IO::send_msg(fd, message) == -1)
        err_("发送消息失败");
    cout << "群聊创建成功!" << endl;
    getchar();
}
// 解散群聊(群主)
void HHH::g_disband(int fd)
{
    cout << " " << endl;
    cout << "-------------------解散群聊-------------------" << endl;
    cout << " " << endl;

    getmygrouplist(fd);

    string group;
    cout << "请输入要解散的群聊名称:" << endl;
    cin.ignore();
    getline(cin, group);
    if (group.empty())
        return;
    json Json = {
        {"type", "disband_group"},
        {"group", group}};
    string str = Json.dump();
    if (IO::send_msg(fd, str) == -1)
        cerr << "发送消息失败" << endl;
    cout << "您成功解散该群聊！" << endl;
    getchar();
}