#include "head.hpp"
#include "TUI.hpp"

typedef void (TUI::*FuncPointer)(int fd);
typedef struct TUI_table
{
    int choice;
    FuncPointer pfunc;
} table;
table gtable[] = {
    {1, &TUI::dologin},
    {2, &TUI::doregister},
    {3, &TUI::dologout},
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
                cout << "输入无效，请重新输入:" << endl;
        }
        else
        {
            cout << "输入无效，请重新输入:" << endl;
            cin.clear();                                         // 清除错误标志
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 忽略错误输入
        }
    }
}

void TUI::menu()
{
    cout << "********************************" << endl;
    cout << "* 欢迎来到聊天室!请输入你的选项 *" << endl;
    cout << "*                               *" << endl;
    cout << "*           1.登录              *" << endl;
    cout << "*           2.注册              *" << endl;
    cout << "*           3.注销              *" << endl;
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
        if (buf == "IS")
            return 1;
        else if (buf == "NO")
        {
            cout << "用户还未注册，请先注册" << endl;
            run(fd);
        }
    }
    return -100;
}

void HHH::menu()
{
    cout << "****************************************" << endl;
    cout << "*                                      *" << endl;
    cout << "*                                      *" << endl;
    cout << "*        1.查询好友      2.好友聊天    *" << endl;
    cout << "*        3.添加好友      4.删除好友    *" << endl;
    cout << "*        5.屏蔽好友                    *" << endl;
    cout << "*        6.查询群聊      7.创建群聊     *" << endl;
    cout << "*        8.加入群聊      9.查看群聊     *" << endl;
    cout << "*        10.退出群聊     11.解散群聊    *" << endl;
    cout << "*                                       *" << endl;
    cout << "*                                       *" << endl;
    cout << "****************************************" << endl;
}
void HHH::f_showlist(int fd)
{
    return;
}
void HHH::f_chat(int fd)
{
    return;
}
void HHH::f_block(int fd)
{
    return;
}
void HHH::f_add(int fd)
{
    cout << "---------------欢迎添加好友---------------" << endl;
    string name;
    cout << "请输入您的好友名称:" << endl;
    cin.ignore();
    getline(cin, name);

    json u_i =
        {
            {"---------name.exit-----------", "----"},
            {"name", name},
        };
    string str = u_i.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
        err_("write");

    string a(128, '\0');
    int l = read(fd, &a[0], a.size() - 1);
    a.resize(l);
    if (a == "OK")//用户已经注册
    {
        json new_ui =
            {
                {"----------manage----------", "manage"},
                {"------choice------", "add-friend"},
                {"name", name},
            };
        string str = new_ui.dump();
        if (write(fd, str.c_str(), str.size()) == -1)
            err_("write");

        string b(128, '\0');
        if (read(fd, &b[0], b.size() - 1) == -1)
            err_("read");
        cout << "好友申请发送成功！可以进行聊天了" << endl;
        run(fd);
    }
    else if (a == "NO")
        cout << "该用户还未注册，请输入正确用户名" << endl;
    else
        cout << "接收数据错误！" << endl;
}

void HHH::f_remove(int fd)
{
    return;
}

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
    {4, &HHH::f_remove},
    {5, &HHH::f_block},
};

int HHH_len = sizeof(gtab) / sizeof(gtab[0]);
void HHH::run(int fd)
{
    int choice;
    while (_running)
    {
        menu();
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