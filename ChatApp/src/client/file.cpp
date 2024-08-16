#include "head.hpp"
#include "HHH.hpp"
namespace fs = filesystem;
void group_file_send(int fd, const string &group, const string &filename);
void file_send(int fd, const string &name, const string &filename);
void file_recv(int fd, const string &directory);
// 文件传输
void HHH::file_pass(int fd)
{
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
            string filename;
            cout << "请输入要传输的文件路径名：" << endl;
            cin.ignore();
            getline(cin, filename);
            cout << "请选择:" << endl;
            cout << "       A.私聊      B.群聊   " << endl;
            char b;
            while (1)
            {
                cin >> b;
                if (b == 'A' || b == 'a')
                {
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
                    cout << "请输入要发送的好友名：" << endl;
                    cin.ignore();
                    getline(cin, name);
                    if (name.empty())
                        return;
                    if (find(chatlist.begin(), chatlist.end(), name) != chatlist.end())
                        cout << "你选择了 " << name << endl;
                    else
                    {
                        cout << "好友不在列表中!" << endl;
                        return;
                    }
                    file_send(fd, name, filename);
                    break;
                }
                else if (b == 'B' || b == 'b')
                {
                    json request = {
                        {"type", "grouplist"}};
                    string str = request.dump();
                    if (IO::send_msg(fd, str) == -1)
                        err_("send_msg");
                    string buffer;
                    if (IO::recv_msg(fd, buffer) == -1)
                        err_("recv_msg");
                    json response = json::parse(buffer);
                    vector<string> grouplist = response["grouplist"];
                    if (response.contains("grouplist"))
                    {
                        cout << "您的群聊列表:" << endl;
                        if (grouplist.empty())
                        {
                            cout << "您的群聊列表为空！" << endl;
                            return;
                        }
                        for (const auto &name : grouplist)
                            cout << name << endl;
                    }
                    string group;
                    cout << "请输入要发送的群组名：" << endl;
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
                    group_file_send(fd, group, filename);
                    break;
                }
                else
                    cout << "请输入正确选项:" << endl;
            }
            break;
        }
        else if (a == '2')
        {

            json jj = {
                {"type", "charge_file"}};
            string m = jj.dump();
            if (IO::send_msg(fd, m) == -1)
                cerr << "发送消息失败" << endl;

            string buffer;
            if (IO::recv_msg(fd, buffer) == -1)
                err_("recv_msg");
            json j = json::parse(buffer);
            string reply = j["have_file"];
            if (reply == "NO")
            {
                cout << "您还没有新文件！" << endl;
                break;
            }
            else
            {
                cout << "您有来自:" << reply << "的文件！" << endl;
                cout << "您是否接收该文件? Y or N" << endl;
                char b;
                while (1)
                {
                    cin >> b;
                    if (b == 'Y' || b == 'y')
                    {
                        string directory;
                        cout << "请输入存储的文件夹：" << endl;
                        cin.ignore();
                        getline(cin, directory);

                        json mess = {
                            {"type", "send_file"},
                            {"name", reply},
                            {"YYY", "YYY"}};
                        string aa = mess.dump();
                        if (IO::send_msg(fd, aa) == -1)
                            cerr << "发送消息失败" << endl;

                        cout << "文件正在后台接收！" << endl;
                        file_recv(fd, directory);
                        break;
                    }
                    else if (b == 'N' || b == 'n')
                    {
                        json mess = {
                            {"type", "send_file"},
                            {"NNN", "NNN"}};
                        string aa = mess.dump();
                        if (IO::send_msg(fd, aa) == -1)
                            cerr << "发送消息失败" << endl;
                        break;
                    }
                    else
                        cout << "请输入正确选项:" << endl;
                }
                break;
            }
        }
        else if (a == '3')
            break;
        else
            cout << "请输入正确数字:" << endl;
    }
}
void group_file_send(int fd, const string &group, const string &filename)
{
    int file_fd = open(filename.c_str(), O_RDONLY);
    if (file_fd == -1)
    {
        cout << "文件路径错误！" << endl;
        cout << "请重新输入文件路径！" << endl;
        return;
    }

    struct stat info;
    if (fstat(file_fd, &info) == -1)
        err_("fstat");

    off_t filesize = info.st_size;
    json j = {
        {"type", "recv_file"},
        {"group", group},
        {"filename", fs::path(filename).filename().string()},
        {"filesize", filesize}};
    string message = j.dump();
    if (IO::send_msg(fd, message) == -1)
        err_("send_msg");

    off_t sum = 0;
    while (sum < filesize)
    {
        ssize_t len = sendfile(fd, file_fd, &sum, filesize - sum);
        if (len == -1)
            err_("sendfile");
        cout << "统计发送了" << sum << "字节，一共有" << filesize << "字节！" << endl;
    }
    close(file_fd);
    cout << "文件发送完成!" << endl;
}
void file_send(int fd, const string &name, const string &filename)
{
    int file_fd = open(filename.c_str(), O_RDONLY);
    if (file_fd == -1)
    {
        cout << "文件路径错误！" << endl;
        cout << "请重新输入文件路径！" << endl;
        return;
    }

    struct stat info;
    if (fstat(file_fd, &info) == -1)
        err_("fstat");

    off_t filesize = info.st_size;
    json j = {
        {"type", "recv_file"},
        {"name", name},
        {"filename", fs::path(filename).filename().string()},
        {"filesize", filesize}};
    string message = j.dump();
    if (IO::send_msg(fd, message) == -1)
        err_("send_msg");

    off_t sum = 0;
    while (sum < filesize)
    {
        ssize_t len = sendfile(fd, file_fd, &sum, filesize - sum);
        if (len == -1)
            err_("sendfile");
        cout << "统计发送了" << sum << "字节，一共有" << filesize << "字节！" << endl;
    }
    close(file_fd);
    cout << "文件发送完成!" << endl;
}
// 接收文件
void file_recv(int fd, const string &directory)
{
    string buf;
    if (IO::recv_msg(fd, buf) == -1)
        err_("recv_msg");
    json j = json::parse(buf);
    string type = j["type"];
    if (type == "sendfile")
    {
        string filename = j["filename"];
        size_t filesize = j["filesize"];
        cout << "文件大小:" << filesize << endl;

        string filepath = directory + "/" + filename; // 文件路径
        cout << "文件路径:" << filepath << endl;

        // 检查并创建目录
        struct stat st;
        if (stat(directory.c_str(), &st) == -1)
            if (mkdir(directory.c_str(), 0755) == -1)
                err_("创建目录失败");

        int file_fd = open(filepath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (file_fd == -1)
        {
            cout << "文件路径错误！" << endl;
            cout << "请重新输入文件路径！" << endl;
            return;
        }

        char buffer[4 << 20];
        off_t sum = 0;
        ssize_t len;
        while (sum < filesize)
        {
            len = recv(fd, buffer, sizeof(buffer), 0);
            if (len > 0)
            {
                if (write(file_fd, buffer, len) != len)
                    err_("write file");
                sum += len;
                cout << "统计收到了" << sum << "字节，一共有" << filesize << "字节！" << endl;
            }
            else if (len == 0)
            {
                cout << "连接关闭！" << endl;
                break;
            }
            else if (errno == EAGAIN)
                this_thread::sleep_for(chrono::milliseconds(10));
            else
                err_("recv_file");
        }
        close(file_fd);
    }
    cout << "文件接收完成!" << endl;
}