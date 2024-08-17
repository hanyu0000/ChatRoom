#include "head.hpp"
#include "HHH.hpp"
// 群聊
void HHH::g_chat(int fd)
{
    cout << "-------------------群聊-------------------" << endl;
    cout << " " << endl;
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
    vector<string> grouplist = response["grouplist"];
    cout << "您的群聊列表:" << endl;
    if (grouplist.empty())
    {
        cout << "您的群聊列表为空！" << endl;
        return;
    }
    for (const auto &name : grouplist)
        cout << name << endl;

    string group;
    cout << "输入你要聊天的群聊：" << endl;
    cin.ignore();
    getline(cin, group);
    if (group.empty())
        return;
    if (find(grouplist.begin(), grouplist.end(), group) != grouplist.end())
        cout << "你选择的群聊是: " << group << endl;
    else
    {
        cout << "该群聊不在列表中!" << endl;
        return;
    }

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
                    cout << "没有历史消息" << endl;
                else
                {
                    for (const auto &message : re)
                        cout << message << endl;
                }
                break;
            }
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
    cout << "请输入聊天消息( 'exit' 结束): " << endl;
    json m = {
        {"type", "g_chat"},
        {"group", group},
        {"message", "12345zxcvb"}};
    string m_str = m.dump();
    if (IO::send_msg(fd, m_str) == -1)
        cerr << "发送消息失败" << endl;

    string msg;
    while (1)
    {
        getline(cin, msg);
        if (msg == "")
            continue;
        if (msg.length() >= 256)
        {
            cout << "消息太长!请重新输入:" << endl;
            continue;
        }
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

        json message = {
            {"type", "g_chat"},
            {"group", group},
            {"message", msg}};
        string mess = message.dump();
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
    cout << "-------------------设置管理员-------------------" << endl;
    cout << " " << endl;
    json mylist =
        {
            {"type", "mygrouplist"}};
    string reply = mylist.dump();
    if (IO::send_msg(fd, reply) == -1)
        cerr << "发送消息失败" << endl;
    // 接收列表
    string buf;
    if (IO::recv_msg(fd, buf) == -1)
        err_("recv_msg");
    json oo = json::parse(buf);
    vector<string> mygrouplist = oo["mygrouplist"];
    cout << "您的群聊列表:" << endl;
    if (mygrouplist.empty())
    {
        cout << "您的群聊列表为空！" << endl;
        return;
    }
    for (const auto &name : mygrouplist)
        cout << name << endl;

    string group;
    cout << "输入你要设置的群聊名称:" << endl;
    cin.ignore();
    getline(cin, group);
    if (group.empty())
        return;
    if (find(mygrouplist.begin(), mygrouplist.end(), group) != mygrouplist.end())
        cout << "你选择的群聊是: " << group << endl;
    else
    {
        cout << "该群聊不在列表中!" << endl;
        return;
    }

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
    vector<string> list;
    string master;
    cout << "您的群聊用户列表:" << endl;
    master = j["master"];
    cout << "群主" << master << endl;
    list = j["userlist"];
    if (list.empty())
    {
        cout << "该群聊用户列表为空！" << endl;
        return;
    }
    for (const auto &name : list)
        cout << name << endl;
    string input;
    cout << "输入你设置的管理员名称(','分隔):" << endl;
    getline(cin, input);
    if (input.empty())
        return;

    vector<string> Friends;
    size_t pos = 0;
    while ((pos = input.find(',')) != string::npos)
    {
        string friendName = input.substr(0, pos);
        if (friendName == master)
            cout << friendName << " 是群主，不能设置为管理员，已忽略..." << endl;
        else if (find(list.begin(), list.end(), friendName) != list.end())
            Friends.push_back(friendName);
        else
            cout << friendName << " 不在群聊用户列表中，已忽略...." << endl;
        input.erase(0, pos + 1);
    }
    // 检查最后一个输入的名称
    if (!input.empty())
        if (find(list.begin(), list.end(), input) != list.end())
            Friends.push_back(input);

    if (!Friends.empty())
    {
        json Json = {
            {"type", "addmanage"},
            {"group", group},
            {"members", Friends}};
        string message = Json.dump();
        if (IO::send_msg(fd, message) == -1)
            cerr << "发送消息失败" << endl;
    }
    getchar();
}
// 删除管理员(群主)
void HHH::g_delete_manager(int fd)
{
    cout << "-------------------删除管理员-------------------" << endl;
    cout << " " << endl;
    json request = {
        {"type", "grouplist"}};
    string str = request.dump();
    if (IO::send_msg(fd, str) == -1)
        err_("send_msg");
    // 接收并处理响应
    string buf;
    if (IO::recv_msg(fd, buf) == -1)
        err_("recv_msg");
    json response = json::parse(buf);
    vector<string> grouplist = response["grouplist"];
    cout << "您的群聊列表:" << endl;
    if (grouplist.empty())
    {
        cout << "您的群聊列表为空！" << endl;
        return;
    }
    for (const auto &name : grouplist)
        cout << name << endl;

    string group;
    string input;
    cout << "输入你要设置的群聊名称:" << endl;
    cin.ignore();
    getline(cin, group);
    if (group.empty())
        return;
    if (find(grouplist.begin(), grouplist.end(), group) != grouplist.end())
        cout << "你选择的群聊是: " << group << endl;
    else
    {
        cout << "该群聊不在列表中!" << endl;
        return;
    }

    json ulist =
        {
            {"type", "managelist"},
            {"group", group}};
    string b = ulist.dump();
    if (IO::send_msg(fd, b) == -1)
        cerr << "发送消息失败" << endl;

    string buffer;
    if (IO::recv_msg(fd, buffer) == -1)
        err_("recv_msg");
    json j = json::parse(buffer);
    vector<string> mygrouplist = j["managelist"];
    cout << "您的管理员列表:" << endl;
    if (mygrouplist.empty())
    {
        cout << "该群聊管理员列表为空！" << endl;
        return;
    }
    for (const auto &name : mygrouplist)
        cout << name << endl;

    cout << "输入你要删除的管理员名称(','分隔):" << endl;
    getline(cin, input);
    if (input.empty())
        return;

    vector<string> Friends;
    size_t pos = 0;
    while ((pos = input.find(',')) != string::npos)
    {
        string friendName = input.substr(0, pos);
        if (find(mygrouplist.begin(), mygrouplist.end(), friendName) != mygrouplist.end())
            Friends.push_back(friendName);
        else
            cout << friendName << " 不在管理员列表中，已忽略..." << endl;
        input.erase(0, pos + 1);
    }
    // 检查最后一个输入的名称
    if (!input.empty())
    {
        if (find(mygrouplist.begin(), mygrouplist.end(), input) != mygrouplist.end())
            Friends.push_back(input);
        else
            cout << input << " 不在管理员列表中，已忽略..." << endl;
    }
    // 如果有有效的管理员名称被添加，则发送删除请求
    if (!Friends.empty())
    {
        json Json = {
            {"type", "deletemanage"},
            {"group", group},
            {"members", Friends}};
        string message = Json.dump();
        if (IO::send_msg(fd, message) == -1)
            cerr << "发送消息失败" << endl;
    }
    getchar();
}
// 移除群用户(群主/管理员)
void HHH::g_delete_people(int fd)
{
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

    string buf;
    if (IO::recv_msg(fd, buf) == -1)
        err_("recv_msg");
    json i = json::parse(buf);
    cout << "群用户列表:" << endl;
    vector<string> userlist = i["userlist"];
    if (userlist.empty())
    {
        cout << "该群聊用户列表为空！" << endl;
        return;
    }
    for (const auto &name : userlist)
        cout << name << endl;

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
        cout << "该群聊不存在或已经被删除！" << endl;
        return;
    }
    cout << "发送群聊申请成功!" << endl;
    getchar();
}
// 查看群用户
void HHH::g_showuser(int fd)
{
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

    string buf;
    if (IO::recv_msg(fd, buf) == -1)
        err_("recv_msg");
    json i = json::parse(buf);
    cout << "群用户列表:" << endl;
    vector<string> userlist = i["userlist"];
    if (userlist.empty())
    {
        cout << "该群聊用户列表为空！" << endl;
        return;
    }
    for (const auto &name : userlist)
        cout << name << endl;
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
    cout << "您的群聊列表:" << endl;
    vector<string> grouplist = response["grouplist"];
    if (grouplist.empty())
    {
        cout << "您的群聊列表为空！" << endl;
        return;
    }
    for (const auto &name : grouplist)
        cout << name << endl;
    getchar();
}
// 退出群聊
void HHH::g_leave(int fd)
{
    cout << "-------------------退出群聊-------------------" << endl;
    cout << " " << endl;
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
    cout << "您的群聊列表:" << endl;
    vector<string> grouplist = response["grouplist"];
    if (grouplist.empty())
    {
        cout << "您的群聊列表为空！" << endl;
        return;
    }
    for (const auto &name : grouplist)
        cout << name << endl;

    string group;
    cout << "请输入要退出的群聊名称:" << endl;
    cin.ignore();
    getline(cin, group);
    if (group.empty())
        return;
    if (find(grouplist.begin(), grouplist.end(), group) == grouplist.end())
    {
        cout << "您输入的群聊名称无效，不在群聊列表中!" << endl;
        return;
    }

    json Json = {
        {"type", "leave_group"},
        {"group", group}};
    string str_ = Json.dump();
    if (IO::send_msg(fd, str_) == -1)
        cerr << "发送消息失败" << endl;
    string mess;
    if (IO::recv_msg(fd, mess) == -1)
        err_("recv_msg");
    json qqq = json::parse(mess);
    string reply = qqq["master"];
    if (reply == "OK")
        cout << "您成功退出该群聊！" << endl;
    else
        cout << "您是群主不能退群!" << endl;
    getchar();
}
// 创建群聊(群主)
void HHH::g_create(int fd)
{
    cout << "-------------------创建群聊-------------------" << endl;
    cout << " " << endl;
    string group;
    string input;
    cout << "请输入群聊名称:" << endl;
    cin.ignore();
    getline(cin, group);
    if (group.empty())
        return;

    json request =
        {
            {"type", "chatlist"}};
    string request_str = request.dump();
    if (IO::send_msg(fd, request_str) == -1)
        err_("send_msg");
    // 接收好友列表
    string buffer;
    if (IO::recv_msg(fd, buffer) == -1)
        err_("recv_msg");
    json qqq = json::parse(buffer);
    vector<string> chatlist;
    if (qqq.contains("chatlist"))
    {
        chatlist = qqq["chatlist"];
        if (chatlist.empty())
        {
            cout << "您的好友列表为空！" << endl;
            return;
        }
    }
    cout << "您的好友列表:" << endl;
    chatlist = qqq["chatlist"];
    for (const auto &name : chatlist)
        cout << name << endl;

    cout << "请输入要添加到群聊的好友名（多个好友用逗号分隔）:" << endl;
    getline(cin, input);
    if (input.empty())
        return;

    // 使用逗号分割输入的好友名
    vector<string> friends_to_add;
    stringstream ss(input);
    string friend_name;
    while (getline(ss, friend_name, ','))
    {
        friend_name.erase(remove(friend_name.begin(), friend_name.end(), ' '), friend_name.end());

        if (find(chatlist.begin(), chatlist.end(), friend_name) != chatlist.end())
            friends_to_add.push_back(friend_name); // 好友存在，添加到列表中
        else
        {
            cout << "好友 " << friend_name << " 不在列表中!" << endl;
            return;
        }
    }

    if (!friends_to_add.empty())
    {
        json Json = {
            {"type", "create_group"},
            {"group", group},             // 你的群聊名称变量
            {"members", friends_to_add}}; // 将选定的好友添加到群聊成员中

        string message = Json.dump();
        if (IO::send_msg(fd, message) == -1)
            err_("发送消息失败");
        cout << "群聊创建成功!" << endl;
    }
    getchar();
}
// 解散群聊(群主)
void HHH::g_disband(int fd)
{
    cout << "-------------------解散群聊-------------------" << endl;
    cout << " " << endl;
    json request = {
        {"type", "grouplist"}};
    string ooo = request.dump();
    if (IO::send_msg(fd, ooo) == -1)
        err_("send_msg");
    // 接收并处理响应
    string buffer;
    if (IO::recv_msg(fd, buffer) == -1)
        err_("recv_msg");
    json response = json::parse(buffer);
    vector<string> grouplist = response["grouplist"];
    cout << "您的群聊列表:" << endl;
    if (grouplist.empty())
    {
        cout << "您的群聊列表为空！" << endl;
        return;
    }
    for (const auto &name : grouplist)
        cout << name << endl;

    string group;
    cout << "请输入要解散的群聊名称:" << endl;
    cin.ignore();
    getline(cin, group);
    if (group.empty())
        return;

    if (find(grouplist.begin(), grouplist.end(), group) != grouplist.end())
        cout << "你选择的群聊是: " << group << endl;
    else
    {
        cout << "该群聊不在列表中!" << endl;
        return;
    }

    json Json = {
        {"type", "disband_group"},
        {"group", group}};
    string str = Json.dump();
    if (IO::send_msg(fd, str) == -1)
        cerr << "发送消息失败" << endl;
    cout << "您成功解散该群聊！" << endl;
    getchar();
}