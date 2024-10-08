#include "head.hpp"
#include "HHH.hpp"
// 退出管理页面
void HHH::last(int fd)
{
    json j = {
        {"type", "return"}};
    string m = j.dump();
    if (IO::send_msg(fd, m) == -1)
        cerr << "发送消息失败" << endl;
}
// 好友聊天
void HHH::f_chat(int fd)
{
    cout << "-------------------好友聊天-------------------" << endl;
    cout << " " << endl;
    json request =
        {
            {"type", "chatlist"}};
    string request_str = request.dump();
    if (IO::send_msg(fd, request_str) == -1)
        err_("send_msg");
    // 接收好友列表
    string bbb;
    if (IO::recv_msg(fd, bbb) == -1)
        err_("recv_msg");
    json qqq = json::parse(bbb);
    vector<string> chatlist = qqq["chatlist"];
    if (chatlist.empty())
    {
        cout << "您的好友列表为空！" << endl;
        return;
    }
    cout << "您的好友列表:" << endl;
    chatlist = qqq["chatlist"];
    for (const auto &name : chatlist)
        cout << name << endl;

    string name;
    cout << "请输入你要聊天的好友: " << endl;
    cin.ignore();
    getline(cin, name);
    if (name.empty())
        return;
    // 检查输入的好友是否存在于好友列表中
    if (find(chatlist.begin(), chatlist.end(), name) != chatlist.end())
        cout << "你选择了与 " << name << " 进行聊天" << endl;
    else
    {
        cout << "好友不在列表中!" << endl;
        return;
    }

    json mm = {
        {"type", "chatblocked"},
        {"name", name}};
    string _str = mm.dump();
    if (IO::send_msg(fd, _str) == -1)
        cerr << "发送消息失败" << endl;

    string buffer;
    if (IO::recv_msg(fd, buffer) == -1)
        err_("recv_msg");
    json jj = json::parse(buffer);
    if (jj.contains("chatblocked"))
    {
        string reply = jj["chatblocked"];
        if (reply == "YES")
        {
            cout << "您已经被对方屏蔽！" << endl;
            return;
        }
    }

    // 离线消息
    json age = {
        {"type", "f_chat_leave"},
        {"name", name}};
    string mess = age.dump();
    if (IO::send_msg(fd, mess) == -1)
        cerr << "发送消息失败" << endl;

    string buf;
    if (IO::recv_msg(fd, buf) == -1)
        err_("recv_msg");
    json j = json::parse(buf);
    if (j.contains("f_chat_leave") && j["f_chat_leave"].is_array())
    {
        cout << "您有新的离线消息:" << endl;
        vector<string> reply = j["f_chat_leave"].get<vector<string>>();
        for (const auto &message : reply)
            cout << message << endl;
    }
    cout << "是否需要拉取历史消息? Y or N" << endl;
    char a;
    while (1)
    {
        cin >> a;
        if (a == 'Y' || a == 'y')
        {
            json jjj = {
                {"type", "f_chatHistry"},
                {"name", name}};
            string mmm = jjj.dump();
            if (IO::send_msg(fd, mmm) == -1)
                cerr << "发送消息失败" << endl;
            string buffer;
            if (IO::recv_msg(fd, buffer) == -1)
                err_("recv_msg");

            json jj = json::parse(buffer);
            if (jj.contains("f_chatHistry"))
            {
                vector<string> re = jj["f_chatHistry"];
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

    thread recvThread;
    // 接收消息的线程函数
    auto receiveMessages = [&]()
    {
        while (!f_stop.load())
        {
            string buffer;
            if (IO::recv_msg(fd, buffer) == 0)
            {
                cerr << "接收消息失败: " << errno << " (" << strerror(errno) << ")" << endl;
                break;
            }
            try
            {
                json j = json::parse(buffer);
                string reply = j["chat"];
                if (reply == "exit")
                {
                    g_stop.store(true);
                    break;
                }
                string f_name = j["f_name"];
                cout << f_name << ": " << reply << endl;
            }
            catch (const json::parse_error &e)
            {
                cerr << "JSON 解析错误: " << e.what() << endl;
            }
        }
    };
    // 启动接收消息的线程
    recvThread = thread(receiveMessages);
    json m = {
        {"type", "chat"},
        {"name", name},
        {"message", "12345zxcvb"}};
    string m_str = m.dump();
    if (IO::send_msg(fd, m_str) == -1)
        cerr << "发送消息失败" << endl;
    cout << "请输入消息(输入 'exit' 退出):" << endl;
    while (1)
    {
        string msg;
        getline(cin, msg);
        if (msg.length() >= 256)
        {
            cout << "消息太长!请重新输入:" << endl;
            continue;
        }
        if (msg == "exit")
        {
            json message = {
                {"type", "chat"},
                {"name", name},
                {"message", "exit"}};
            string message_str = message.dump();
            if (IO::send_msg(fd, message_str) == -1)
                cerr << "发送消息失败" << endl;
            cout << "聊天结束......" << endl;
            break;
        }
        json message = {
            {"type", "chat"},
            {"name", name},
            {"message", msg}};
        string message_str = message.dump();
        if (IO::send_msg(fd, message_str) == -1)
            cerr << "发送消息失败" << endl;
    }
    if (recvThread.joinable())
        recvThread.join();
}
// 好友添加
void HHH::f_add(int fd)
{
    cout << "----------------添加好友----------------" << endl;
    cout << " " << endl;
    string name;
    cout << "请输入您的好友名称:" << endl;
    cin.ignore();
    getline(cin, name);
    if (name.empty())
        return;

    json a =
        {
            {"type", "addfriend"},
            {"name", name}};
    string str = a.dump();
    if (IO::send_msg(fd, str) == -1)
        err_("send_msg");

    string buf;
    if (IO::recv_msg(fd, buf) == -1)
        err_("recv_msg");
    json j = json::parse(buf);
    cout << "1" << endl;
    if (j.contains("nopeople"))
        cout << "该用户不存在!" << endl;
    else if (j.contains("me"))
        cout << "请不要输入自己姓名!" << endl;
    else
        cout << "好友申请发送成功！等待对方同意" << endl;
    return;
}
// 好友申请回复
void HHH::f_reply(int fd)
{
    cout << "-------------------新好友消息-------------------" << endl;
    cout << " " << endl;
    json age = {
        {"type", "newfriend_leave"}};
    string mess = age.dump();
    if (IO::send_msg(fd, mess) == -1)
        cerr << "发送消息失败" << endl;

    while (1)
    {
        string buf;
        if (IO::recv_msg(fd, buf) == -1)
            err_("recv_msg");

        json j = json::parse(buf);
        if (j.contains("newfriend_leave"))
        {
            string name = j["newfriend_leave"];
            if (name == "NO")
            {
                cout << "您没有新的好友申请" << endl;
                break;
            }
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
                    if (IO::send_msg(fd, request_str) == -1)
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
// 好友屏蔽
void HHH::f_block(int fd)
{
    cout << "----------------好友屏蔽----------------" << endl;
    cout << " " << endl;
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
    string name;
    cout << "请输入您要屏蔽的好友名称:" << endl;
    cin.ignore();
    getline(cin, name);
    if (name.empty())
        return;
    if (find(chatlist.begin(), chatlist.end(), name) != chatlist.end())
        cout << "你选择了 " << name << " 进行屏蔽" << endl;
    else
    {
        cout << "好友不在列表中!" << endl;
        return;
    }

    json a =
        {
            {"type", "blockfriend"},
            {"name", name}};
    string str = a.dump();
    if (IO::send_msg(fd, str) == -1)
        err_("send_msg");
    cout << "好友屏蔽成功！" << endl;
    getchar();
}
// 取消屏蔽
void HHH::f_unblock(int fd)
{
    cout << "----------------取消屏蔽好友----------------" << endl;
    cout << " " << endl;
    json request =
        {
            {"type", "block_list"}};
    string request_str = request.dump();
    if (IO::send_msg(fd, request_str) == -1)
        err_("send_msg");
    string buffer;
    if (IO::recv_msg(fd, buffer) == -1)
        err_("recv_msg");
    json j = json::parse(buffer);

    vector<string> block_list;
    if (j.contains("block_list"))
    {
        block_list = j["block_list"];
        if (block_list.empty())
        {
            cout << "您的屏蔽列表为空！" << endl;
            return;
        }
    }
    cout << "您的屏蔽列表:" << endl;
    block_list = j["block_list"];
    for (const auto &name : block_list)
        cout << name << endl;

    string name;
    cout << "请输入您要取消屏蔽的好友名称:" << endl;
    cin.ignore();
    getline(cin, name);
    if (name.empty())
        return;

    if (find(block_list.begin(), block_list.end(), name) != block_list.end())
        cout << "你选择了 " << name << " 取消屏蔽" << endl;
    else
    {
        cout << "好友不在列表中!" << endl;
        return;
    }

    json a =
        {
            {"type", "unblockfriend"},
            {"name", name}};
    string str = a.dump();
    if (IO::send_msg(fd, str) == -1)
        err_("send_msg");
    cout << "取消好友屏蔽成功！" << endl;
    getchar();
}
// 删除好友
void HHH::f_delete(int fd)
{
    cout << "----------------删除好友----------------" << endl;
    cout << " " << endl;
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

    string name;
    cout << "请输入您要删除的好友名称:" << endl;
    cin.ignore();
    getline(cin, name);
    if (name.empty())
        return;

    if (find(chatlist.begin(), chatlist.end(), name) != chatlist.end())
        cout << "你选择了删除 " << name << endl;
    else
    {
        cout << "好友不在列表中!" << endl;
        return;
    }

    json a =
        {
            {"type", "deletefriend"},
            {"name", name}};
    string str = a.dump();
    if (IO::send_msg(fd, str) == -1)
        err_("send_msg");
    cout << "好友删除成功！" << endl;
    getchar();
}