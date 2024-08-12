#include "head.hpp"
#include "HHH.hpp"
void show_list(int fd);
void getmygrouplist(int fd);
void file_pase(int fd, string &name);
void file_recv(int fd);
void file_pase(int fd, string &name)
{
    char filename[1024] = {"/home/zxc/wk/wl.h"};
    cout << "请输入要传输的文件路径名：" << endl;
    // cin.ignore();
    // cin >> filename;
    cout << filename << endl;

    int file_fd = open(filename, O_RDONLY);
    if (file_fd == -1)
        err_("open file fail");

    struct stat info;
    if (fstat(file_fd, &info) == -1) // 用于获取文件的状态信息
    {
        close(file_fd);
        err_("fstat");
    }

    off_t filesize = info.st_size; // st_size;  文件字节数(文件大小)
    json j = {
        {"type", "file"},
        {"name", name},
        {"filename", filename},
        {"filesize", filesize}};
    string message = j.dump();
    if (Util::send_msg(fd, message) == -1)
        err_("send_msg");

    off_t len = 0; // 每次发送的字节数
    off_t sum = 0; // 累计发送的字节数
    while (sum < filesize)
    {
        // sendfile 函数用于将文件的内容直接从文件描述符发送到套接字，而不经过用户空间的缓冲区
        len = sendfile(fd, file_fd, nullptr, filesize - sum);
        if (len == -1)
        {
            perror("sendfile");
            break;
        }
        else if (len == 0)
        {
            cout << "没有数据发送" << endl;
            break;
        }
        sum += len;
    }
    close(file_fd);
    cout << "文件发送完成！" << endl;
}
void file_recv(int fd)
{
    char buffer[4096];
    ssize_t len;

    // 接收文件大小
    string buffer_size;
    if (Util::recv_msg(fd, buffer_size) == -1)
        err_("recv_msg");
    json j = json::parse(buffer_size);
    size_t filesize = j["filesize"];
    // 获取文件名并打开文件准备写入
    string filename = j["have_file"];
    int file_fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_fd == -1)
        err_("打开文件失败");

    size_t received_size = 0;
    while (received_size < filesize)
    {
        len = recv(fd, buffer, sizeof(buffer), 0);
        if (len <= 0)
        {
            if (len == 0)
                cout << "文件接收完成！" << endl;
            else
                perror("接收文件内容失败");
            break;
        }
        if (write(file_fd, buffer, len) != len)
            err_("写入文件失败");
        received_size += len;
    }
    close(file_fd);
    cout << "文件成功保存为 " << filename << endl;
}
// 文件传输
void HHH::file_pass(int fd)
{
    cout << " " << endl;
    cout << "-------------------文件传输-------------------" << endl;
    cout << " " << endl;

    cout << "请选择： " << endl;
    cout << "    1.发送文件        2.接受文件        3.退出" << endl;
    char a;
    while (1)
    {
        cin >> a;
        if (a == '1')
        {
            cout << "       A.私聊      B.群聊   " << endl;
            char b;
            while (1)
            {
                cin >> b;
                if (b == 'A' || b == 'a')
                {
                    show_list(fd); // 好友列表
                    string name;
                    cout << "请输入要发送的好友名：" << endl;
                    cin.ignore();
                    getline(cin, name);
                    if (name.empty())
                        return;

                    thread fileThread(file_pase, fd, ref(name));
                    fileThread.detach();
                    break;
                }
                else if (b == 'B' || b == 'b')
                {
                    g_showlist(fd); // 群组列表
                }
                else
                    cout << "请输入正确选项:" << endl;
            }
            break;
        }
        else if (a == '2')
        {
            json jj = {
                {"type", "send_file"}};
            string m = jj.dump();
            if (Util::send_msg(fd, m) == -1)
                cerr << "发送消息失败" << endl;
            string buffer;
            if (Util::recv_msg(fd, buffer) == -1)
                err_("recv_msg");
            json j = json::parse(buffer);
            string reply = j["have_file"];
            if (reply == "NO")
            {
                cout << "您还没有新文件！" << endl;
                break;
            }
            else
                cout << "您有来自:" << reply << "的文件！" << endl;
            cout << "您是否接收该文件? Y or N" << endl;
            char b;
            while (1)
            {
                cin >> b;
                if (b == 'Y' || b == 'y')
                {
                    json mess = {
                        {"type", "send_file"},
                        {"YYY", "YYY"}};
                    string aa = mess.dump();
                    if (Util::send_msg(fd, aa) == -1)
                        cerr << "发送消息失败" << endl;
                    // 接受文件(线程)
                    thread newthread(file_recv, fd);
                    newthread.detach();
                    break;
                }
                else if (b == 'N' || b == 'n')
                {
                    json mess = {
                        {"type", "send_file"},
                        {"NNN", "NNN"}};
                    string aa = mess.dump();
                    if (Util::send_msg(fd, aa) == -1)
                        cerr << "发送消息失败" << endl;
                    break;
                }
                else
                    cout << "请输入正确选项:" << endl;
            }
            break;
        }
        else if (a == '3')
            break;
        else
            cout << "请输入正确数字:" << endl;
    }
    getchar();
}
// 群聊
void HHH::g_chat(int fd)
{
    cout << " " << endl;
    cout << "-------------------群聊-------------------" << endl;
    cout << " " << endl;

    g_showlist(fd);

    cout << "输入你要聊天的群聊：" << endl;
    string group;
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
            if (Util::send_msg(fd, mmm) == -1)
                cerr << "发送消息失败" << endl;

            string buffer;
            if (Util::recv_msg(fd, buffer) == -1)
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
        else
            cout << "请输入正确选项:" << endl;
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
            if (Util::recv_msg(fd, buffer) == -1)
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
                else
                    cerr << "收到未知格式的消息" << endl;
            }
            catch (const json::parse_error &e)
            {
                cerr << "JSON 解析错误: " << e.what() << endl;
            }
        }
    };
    // 启动接收消息的线程
    recvThread = thread(receiveMessages);
    string msg;
    cout << "请输入聊天消息( 'exit' 结束): " << endl;
    while (1)
    {
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
            if (Util::send_msg(fd, mess) == -1)
                cerr << "发送消息失败" << endl;
            break;
        }
        // 发送消息
        json message = {
            {"type", "g_chat"},
            {"group", group},
            {"message", msg}};
        string mess = message.dump();
        if (Util::send_msg(fd, mess) == -1)
            cerr << "发送消息失败" << endl;
    }
    if (recvThread.joinable())
        recvThread.join();
}
// 好友聊天
void HHH::f_chat(int fd)
{
    cout << " " << endl;
    cout << "-------------------好友聊天-------------------" << endl;
    cout << " " << endl;

    show_list(fd);

    string name;
    cout << "请输入你要聊天的好友: " << endl;
    cin.ignore();
    getline(cin, name);
    if (name.empty())
        return;
    // 离线消息
    json age = {
        {"type", "f_chat_leave"},
        {"name", name}};
    string mess = age.dump();
    if (Util::send_msg(fd, mess) == -1)
        cerr << "发送消息失败" << endl;

    string buf;
    if (Util::recv_msg(fd, buf) == -1)
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
            // 历史消息
            json jjj = {
                {"type", "f_chatHistry"},
                {"name", name}};
            string mmm = jjj.dump();
            if (Util::send_msg(fd, mmm) == -1)
                cerr << "发送消息失败" << endl;

            string buffer;
            if (Util::recv_msg(fd, buffer) == -1)
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
        else
            cout << "请输入正确选项:" << endl;
    }
    getchar();

    thread recvThread;
    // 接收消息的线程函数
    auto receiveMessages = [&]()
    {
        while (!f_stop.load())
        {
            string buffer;
            cout << buffer << endl;
            if (Util::recv_msg(fd, buffer) == -1)
            {
                cerr << "接收消息失败: " << errno << " (" << strerror(errno) << ")" << endl;
                break;
            }
            try
            {
                json j = json::parse(buffer);
                string f_name = j["f_name"];
                string reply = j["chat"];
                if (reply == "exit")
                {
                    f_stop.store(true);
                    break;
                }
                else if (reply == "blocked")
                {
                    cout << "您已经被对方屏蔽！" << endl;
                    break;
                }
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
    string msg;
    cout << "请输入聊天消息( 'exit' 结束): " << endl;
    while (1)
    {
        getline(cin, msg);
        if (msg == "")
            continue;
        if (msg == "exit")
        {
            cout << "聊天结束......" << endl;
            json message = {
                {"type", "chat"},
                {"name", name},
                {"message", msg}};
            string message_str = message.dump();
            if (Util::send_msg(fd, message_str) == -1)
                cerr << "发送消息失败" << endl;
            break;
        }
        // 发送消息
        json message = {
            {"type", "chat"},
            {"name", name},
            {"message", msg}};
        string message_str = message.dump();
        if (Util::send_msg(fd, message_str) == -1)
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
    if (Util::send_msg(fd, b) == -1)
        cerr << "发送消息失败" << endl;
    // 获取用户列表
    string buffer;
    if (Util::recv_msg(fd, buffer) == -1)
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
    if (Util::send_msg(fd, message) == -1)
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
    if (Util::send_msg(fd, b) == -1)
        cerr << "发送消息失败" << endl;
    // 获取管理员列表
    string buffer;
    if (Util::recv_msg(fd, buffer) == -1)
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
    if (Util::send_msg(fd, message) == -1)
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
    if (Util::send_msg(fd, str) == -1)
        err_("send_msg");
    // 接收群用户列表
    string buf;
    if (Util::recv_msg(fd, buf) == -1)
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
    if (Util::send_msg(fd, message) == -1)
        cerr << "发送消息失败" << endl;

    // 接受回应
    string buffer;
    if (Util::recv_msg(fd, buffer) == -1)
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
    if (Util::send_msg(fd, mess) == -1)
        cerr << "发送消息失败" << endl;

    while (1)
    {
        string buf;
        if (Util::recv_msg(fd, buf) == -1)
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
                    if (Util::send_msg(fd, request_str) == -1)
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
                    if (Util::send_msg(fd, request_str) == -1)
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
    if (Util::send_msg(fd, str) == -1)
        cerr << "发送消息失败" << endl;

    string buf;
    if (Util::recv_msg(fd, buf) == -1)
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
// 好友申请回复
void HHH::f_reply(int fd)
{
    cout << " " << endl;
    cout << "-------------------新好友消息-------------------" << endl;
    cout << " " << endl;

    json age = {
        {"type", "newfriend_leave"}};
    string mess = age.dump();
    if (Util::send_msg(fd, mess) == -1)
        cerr << "发送消息失败" << endl;
    while (1)
    {
        string buf;
        if (Util::recv_msg(fd, buf) == -1)
            err_("recv_msg");

        json jj = json::parse(buf);
        if (jj.contains("newfriend_leave"))
        {
            string name = jj["newfriend_leave"];
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
    }

    getchar();
}
// 好友屏蔽
void HHH::f_block(int fd)
{
    cout << " " << endl;
    cout << "----------------好友屏蔽----------------" << endl;
    cout << " " << endl;

    show_list(fd);

    string name;
    cout << "请输入您要屏蔽的好友名称:" << endl;
    cin.ignore();
    getline(cin, name);
    if (name.empty())
        return;

    json a =
        {
            {"type", "blockfriend"},
            {"name", name}};
    string str = a.dump();
    if (Util::send_msg(fd, str) == -1)
        err_("send_msg");
    cout << "好友屏蔽成功！" << endl;
    getchar();
}
// 取消屏蔽
void HHH::f_unblock(int fd)
{
    cout << " " << endl;
    cout << "----------------取消屏蔽好友----------------" << endl;
    cout << " " << endl;

    json request =
        {
            {"type", "block_list"}};
    string request_str = request.dump();
    if (Util::send_msg(fd, request_str) == -1)
        err_("send_msg");
    // 接收列表
    string buffer;
    if (Util::recv_msg(fd, buffer) == -1)
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

    json a =
        {
            {"type", "unblockfriend"},
            {"name", name}};
    string str = a.dump();
    if (Util::send_msg(fd, str) == -1)
        err_("send_msg");
    cout << "取消好友屏蔽成功！" << endl;
    getchar();
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
    if (name.empty())
        return;

    json a =
        {
            {"type", "addfriend"},
            {"name", name}};
    string str = a.dump();
    if (Util::send_msg(fd, str) == -1)
        err_("send_msg");
    cout << "好友申请发送成功！等待对方同意" << endl;
    getchar();
}
// 删除好友
void HHH::f_delete(int fd)
{
    cout << " " << endl;
    cout << "----------------删除好友----------------" << endl;
    cout << " " << endl;
    show_list(fd);
    string name;
    cout << "请输入您要删除的好友名称:" << endl;
    cin.ignore();
    getline(cin, name);
    if (name.empty())
        return;

    json a =
        {
            {"type", "deletefriend"},
            {"name", name}};
    string str = a.dump();
    if (Util::send_msg(fd, str) == -1)
        err_("send_msg");
    cout << "好友删除成功！" << endl;
    getchar();
}
// 好友列表
void show_list(int fd)
{
    json request =
        {
            {"type", "chatlist"}};
    string request_str = request.dump();
    if (Util::send_msg(fd, request_str) == -1)
        err_("send_msg");
    // 接收好友列表
    string buffer;
    if (Util::recv_msg(fd, buffer) == -1)
        err_("recv_msg");
    try
    {
        json j = json::parse(buffer);
        vector<string> chatlist;
        if (j.contains("chatlist"))
        {
            chatlist = j["chatlist"];
            if (chatlist.empty())
            {
                cout << "您的好友列表为空！" << endl;
                return;
            }
        }
        cout << "您的好友列表:" << endl;
        chatlist = j["chatlist"];
        for (const auto &name : chatlist)
            cout << name << endl;
    }
    catch (const json::parse_error &e)
    {
        cerr << "JSON解析错误: " << e.what() << endl;
    }
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
    if (Util::send_msg(fd, str) == -1)
        err_("send_msg");
    // 接收群用户列表
    string buf;
    if (Util::recv_msg(fd, buf) == -1)
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
    if (Util::send_msg(fd, str) == -1)
        err_("send_msg");
    // 接收并处理响应
    string buffer;
    if (Util::recv_msg(fd, buffer) == -1)
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
    if (Util::send_msg(fd, str) == -1)
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
    if (Util::send_msg(fd, reply) == -1)
        cerr << "发送消息失败" << endl;
    // 接收列表
    string buffer;
    if (Util::recv_msg(fd, buffer) == -1)
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
    if (Util::send_msg(fd, message) == -1)
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
    if (Util::send_msg(fd, str) == -1)
        cerr << "发送消息失败" << endl;
    cout << "您成功解散该群聊！" << endl;
    getchar();
}