#include "head.hpp"
#include "TUI.hpp"
void main_menu(int fd, string name)
{
    char a;
    while (true)
    {
        cout << "" << endl;
        cout << "" << endl;
        cout << "按 < 1 > 进入管理页面             < 2 > 回到登录页面" << endl;
        cout << "" << endl;
        cout << "" << endl;
        cin >> a;
        if (a == '1')
        {
            HHH jjj;
            jjj.run(fd);
        }
        else if (a == '2')
        {
            json name_json = {
                {"name/fd", name},
            };
            string str = name_json.dump();
            if (write(fd, str.c_str(), str.size()) == -1)
                err_("write");
            return;
        }
        else
            cout << "请输入正确数字:" << endl;
    }
}

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
string username;
string TUI::run(int fd)
{
    int choice;
    while (_running)
    {
        menu();
        cout << "" << endl;
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
    return username;
}

void TUI::menu()
{
    cout << " " << endl;
    cout << " " << endl;
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
    cout << " " << endl;
    cout << " " << endl;
    cout << "---------------欢迎登录!---------------" << endl;
    cout << " " << endl;
    cout << " " << endl;
    string name;
    string pwd;
    cout << "请输入您的登录名:" << endl;
    cin.ignore();
    getline(cin, name);
    cout << "请输入您的密码:" << endl;
    getline(cin, pwd);

    int _loginSuccess = read_response(fd, name, pwd);
    if (_loginSuccess == 404)
        return;
    if (_loginSuccess)
    {
        cout << " " << endl;
        cout << " " << endl;
        cout << "用户: " << name << "登录成功!!!" << endl;
        username = name;
        _running = false;
    }
    else
    {
        cout << "用户名或密码错误！请重新输入！" << endl;
        return;
    }
}
// 注册页面
void TUI::doregister(int fd)
{
    cout << " " << endl;
    cout << " " << endl;
    cout << "---------------欢迎注册!---------------" << endl;
    cout << " " << endl;
    cout << " " << endl;
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
        if (buf == "exitOK")
        {
            cout << "该用户已经存在!请重新输入用户名:" << endl;
            return;
        }
        else if (buf == "setOK")
        {
            cout << "注册成功!" << endl;
            cout << "您可以进行登录了!" << endl;
            return;
        }
        else if (buf == "setNO")
        {
            cout << "注册失败，请检查用户名或密码并重试!" << endl;
            return;
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
    if (_loginSuccess == 404)
        return;
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
                    {"------xxxxx------", "------xxxxx------"},
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
        if (buf == "loading")
        {
            cout << "该用户已经登录！请输入你自己的账号:" << endl;
            return 404;
        }
        else if (buf == "IS USER")
            return 1;
        else if (buf == "NO USER")
        {
            cout << "用户还未注册，请先注册:" << endl;
            return 404;
        }
    }
    return -100;
}