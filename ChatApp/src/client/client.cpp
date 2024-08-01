#include "head.hpp"
#include "_main.hpp"
string my_name;
class TUI
{
public:
    TUI() : _running(true) {}

    void run(int fd);
    void menu();
    void dologin(int fd);
    void doregister(int fd);
    void dologout(int fd);
    void exit(int fd);

private:
    bool _running;
    int read_response(int fd, const string &name, const string &pwd);
};

void TUI::menu()
{
    cout << "********************************" << endl;
    cout << "* 欢迎来到聊天室!请输入你的选项 *" << endl;
    cout << "*                               *" << endl;
    cout << "*           1.登录              *" << endl;
    cout << "*           2.注册              *" << endl;
    cout << "*           3.注销              *" << endl;
    cout << "*        4.停留此页面           *" << endl;
    cout << "*                               *" << endl;
    cout << "********************************" << endl;
}
// 登陆页面
void TUI::dologin(int fd)
{
    cout << "---------------欢迎登录!---------------" << endl;
    string name;
    string pwd;
    cout << "请输入您的登录名:" << endl;
    cin.ignore(); // 忽略之前输入留下的换行符
    getline(cin, name);
    cout << "请输入您的密码:" << endl;
    getline(cin, pwd);

    int _loginSuccess = read_response(fd, name, pwd);

    if (_loginSuccess)
    {
        cout << "登录成功!" << endl;
        my_name = name;
        _running = false;
    }
    else
    {
        cout << "用户名或密码错误！请重新输入！" << endl;
        run(fd);
    }
}
// 注册页面
void TUI::doregister(int fd)
{
    cout << "---------------欢迎注册!---------------" << endl;
    string name;
    string pwd;
    cout << "请输入您的注册名称:" << endl;
    cin.ignore();
    getline(cin, name);
    cout << "请输入您的密码:" << endl;
    getline(cin, pwd);

    json u_i =
        {
            {"------sssss------", "------sssss------"},
            {"name", name},
            {"pwd", pwd},
        };
    string str = u_i.dump();                      // 将 JSON 对象转换为字符串
    if (write(fd, str.c_str(), str.size()) == -1) // 将 JSON 字符串写入文件描述符
        err_("write");

    string buf(128, '\0');
    int r = read(fd, &buf[0], buf.size() - 1);
    if (r == -1)
        err_("read");
    if (r == 0)
        cout << "服务器关闭连接......" << endl;
    if (r > 0)
    {
        buf.resize(r);
        if (buf == "setOK")
        {
            cout << "注册成功!" << endl;
            cout << "您可以进行登录了!" << endl;
            run(fd);
        }
        else if (buf == "setNO")
        {
            cout << "注册失败，请检查用户名或密码并重试!" << endl;
            run(fd);
        }
    }
}
// 注销
void TUI::dologout(int fd)
{
    string name;
    string pwd;
    cout << "请输入您的用户名称" << endl;
    cin.ignore();
    getline(cin, name);
    cout << "请输入您的密码:" << endl;
    getline(cin, pwd);

    int _loginSuccess = read_response(fd, name, pwd);
    cout << "确认要注销你的账号吗? T or F" << endl;
    cout << "你要注销的账户名为：" << name << "密码为：" << pwd << endl;
    if (_loginSuccess)
    {
        char in;
        cin >> in;
        cout << "你的选择是:" << in << endl;
        if (in == 'T' || in == 't')
        {
            json u_i =
                {
                    {"xxxxx", "xxxxx"},
                    {"name", name},
                    {"pwd", pwd},
                };

            string str = u_i.dump();                      // 将 JSON 对象转换为字符串
            if (write(fd, str.c_str(), str.size()) == -1) // 将 JSON 字符串写入文件描述符
                err_("write");

            string buf(128, '\0');
            int r = read(fd, &buf[0], buf.size() - 1);
            if (r == -1)
                err_("read");
            if (r > 0) // 如果读取到数据，则处理响应
            {
                buf[r] = '\0';
                cout << "从服务端接收到: " << buf << endl;
            }
        }
    }
}
void TUI::exit(int fd)
{
    run(fd);
}
int TUI::read_response(int fd, const string &name, const string &pwd)
{
    string buf(128, '\0');
    json u_i =
        {
            {"---charge_user---", "---charge_user---"},
            {"name", name},
            {"pwd", pwd},
        };
    string str = u_i.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");

    int r = read(fd, &buf[0], buf.size() - 1);
    if (r == -1)
        err_("read");
    if (r == 0)
        cout << "服务器关闭连接......" << endl;
    if (r > 0)
    {
        buf.resize(r);
        cout << "从服务端接收到: " << buf << endl;

        if (buf == "IS")
        {
            cout << "IS" << endl;
            return 1;
        }
        else if (buf == "NO")
        {
            cout << "用户还未注册，请先注册" << endl;
            run(fd);
        }
    }
    return -100;
}

typedef void (TUI::*FuncPointer)(int fd);
typedef struct _table
{
    int choice;
    FuncPointer pfunc;
} table;

table gtable[] = {
    {1, &TUI::dologin},
    {2, &TUI::doregister},
    {3, &TUI::dologout},
    {4, &TUI::exit},
};
int g_len = sizeof(gtable) / sizeof(gtable[0]);
void TUI::run(int fd)
{
    int choice;
    while (_running)
    {
        menu();
        cout << "请选择：" << endl;
        if (cin >> choice)
        {
            bool flag = false;
            for (int i = 0; i < g_len; i++)
            {
                if (choice == gtable[i].choice)
                {
                    (this->*gtable[i].pfunc)(fd); // 解引用成员函数指针,调用该成员函数
                    flag = true;
                    break;
                }
            }
            if (!flag)
                cout << "无效的选择，请重新输入:" << endl;
        }
        else
        {
            cout << "输入无效，请重新输入:" << endl;
            cin.clear();                                         // 清除错误标志
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 忽略错误输入
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " <server-ip>" << endl;
        exit(1);
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0); // 创建socket设备，返回设备文件描述符
    if (fd == -1)
        err_("socket");

    struct sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_port = htons(8080);
    inet_pton(AF_INET, argv[1], &serv.sin_addr);

    // 使用fd向服务器发起连接
    if (connect(fd, (struct sockaddr *)&serv, sizeof(serv)) == -1)
        err_("connect");

    // 三次握手成功

    TUI tui;
    tui.run(fd);
    string name = my_name;
    while (1)
    {
        cout << " -------------------------------------------------" << endl;
        cout << " -------------------------------------------------" << endl;
        cout << " ------------欢迎" << name << ":" << fd << "来到聊天室！！！------------" << endl;
        cout << " -------------------------------------------------" << endl;
        cout << " -------------------------------------------------" << endl;

        while (1)
        {
            char in;
            cout << "       1.好友管理       2.群组管理" << endl;
            cout << "请输入你的选项： " << endl;
            cin >> in;
            if (in == '1')
                _friend(fd);
            else if (in == '2')
                _group(fd);
            else
                cout << "输入错误，请输入正确选项" << endl;
        }
        getchar();
    }
    close(fd);
    return 0;
}