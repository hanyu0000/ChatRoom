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
    return;
}
void TUI::menu()
{
    cout << " " << endl;
    cout << " " << endl;
    cout << "********************************" << endl;
    cout << "* 欢迎来到聊天室!请输入你的选项: *" << endl;
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

    int login = read_response(fd, name, pwd);
    if (login == 404)
        return;
    if (login == 99)
        return;
    if (login)
    {
        cout << " " << endl;
        cout << "用户: " << name << "登录成功!!!" << endl;
        _running = false;
        system("clear");
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
            {"type", "register"},
            {"name", name},
            {"pwd", pwd}
        };
    string str = u_i.dump();
    if (Util::send_msg(fd, str) == -1)
        err_("send_msg");

    string strResponse;
    if (Util::recv_msg(fd, strResponse) == -1)
        err_("recv_msg");
    try
    {
        json response = json::parse(strResponse);
        if (response.contains("register"))
        {
            string reply = response["register"];
            if (reply == "exitOK")
            {
                cout << "该用户已经存在!请重新输入用户名:" << endl;
                return;
            }
            else if (reply == "setOK")
            {
                cout << "注册成功!" << endl;
                cout << "您可以进行登录了!" << endl;
                return;
            }
            else if (reply == "setNO")
            {
                cout << "注册失败，请检查用户名或密码并重试!" << endl;
                return;
            }
        }
    }
    catch (const json::parse_error &e)
    {
        cerr << "JSON 解析失败: " << e.what() << endl;
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

    int login = read_response(fd, name, pwd);
    if (login == 404)
        return;
    cout << "确认要注销你的账号吗? T or F" << endl;
    cout << "你要注销的账户名为：" << name << "密码为：" << pwd << endl;
    if (login)
    {
        char in;
        cin >> in;
        cout << "你的选择是:" << in << endl;
        if (in == 'T' || in == 't')
        {
            json u_i =
                {
                    {"type", "logout"},
                    {"name", name},
                    {"pwd", pwd}
                };
            string str = u_i.dump();
            if (Util::send_msg(fd, str) == -1)
                err_("send_msg");

            string strResponse;
            if (Util::recv_msg(fd, strResponse) == -1)
                err_("recv_msg");
            try
            {
                json response = json::parse(strResponse);
                if (response.contains("logout"))
                {
                    string reply = response["logout"];
                    cout << "从服务端接收到: " << reply << endl;
                }
            }
            catch (const json::parse_error &e)
            {
                cerr << "JSON 解析失败: " << e.what() << endl;
            }
        }
    }
}
int TUI::read_response(int fd, const string &name, const string &pwd)
{
    string buf;
    json u_i =
        {
            {"type", "isUser"},
            {"name", name},
            {"pwd", pwd}
        };
    string str = u_i.dump();
    if (Util::send_msg(fd, str) == -1)
        err_("send_msg");

    string strResponse;
    if (Util::recv_msg(fd, strResponse) == -1)
        err_("recv_msg");
    try
    {
        json response = json::parse(strResponse);
        if (response.contains("isUser"))
        {
            string reply = response["isUser"];
            if (reply == "loading")
            {
                cout << "该用户已经登录！请输入你自己的账号:" << endl;
                return 99;
            }
            else if (reply == "IS USER")
                return 1;
            else if (reply == "NO USER")
            {
                cout << "用户还未注册，请先注册:" << endl;
                return 404;
            }
        }
    }
    catch (const json::parse_error &e)
    {
        cerr << "JSON 解析失败: " << e.what() << endl;
    }
    return -100;
}