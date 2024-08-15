#include "head.hpp"
#include "HHH.hpp"
namespace fs = filesystem;
void show_list(int fd); // 好友列表
void getmygrouplist(int fd);
void HHH::last(int fd)
{
    json j = {
        {"type", "return"}};
    string m = j.dump();
    if (IO::send_msg(fd, m) == -1)
        cerr << "发送消息失败" << endl;
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
    }
    close(file_fd);
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
    }
    close(file_fd);
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
            }
            else if (len == 0)
            {
                cout << "连接关闭！" << endl;
                break;
            }
            else if (errno == EAGAIN)
                this_thread::sleep_for(chrono::milliseconds(50));
            else
                err_("recv_file");
        }
        close(file_fd);
    }
}
void group_send_file_thread(int fd, const string &name, const string &filename)
{
    group_file_send(fd, name, filename);
}
void send_file_thread(int fd, const string &name, const string &filename)
{
    file_send(fd, name, filename);
}
void recv_file_thread(int fd, const string &directory)
{
    file_recv(fd, directory);
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
                    show_list(fd); // 好友列表
                    string name;
                    cout << "请输入要发送的好友名：" << endl;
                    cin.ignore();
                    getline(cin, name);
                    if (name.empty())
                        return;

                    // 创建并启动一个新线程来发送文件
                    cout << "文件正在后台发送..." << endl;
                    thread file_thread(send_file_thread, fd, name, filename);
                    file_thread.join();
                    cout << "文件发送完成!" << endl;
                    break;
                }
                else if (b == 'B' || b == 'b')
                {
                    g_showlist(fd); // 群组列表

                    string group;
                    cout << "请输入要发送的群组名：" << endl;
                    getline(cin, group);
                    if (group.empty())
                        return;

                    cout << "文件正在后台发送..." << endl;
                    thread file_thread(group_send_file_thread, fd, group, filename);
                    file_thread.join();
                    cout << "文件发送完成!" << endl;
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
                        thread file_thread(recv_file_thread, fd, directory);
                        file_thread.join();
                        cout << "文件发送完成!" << endl;
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