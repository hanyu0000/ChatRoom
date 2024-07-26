#include <iostream>
#include <cstdio>
#include <cstring>
#include <limits>
#include "base.h"
#include "gui_main.h"
using namespace std;

UI _userInfo;

class GUI
{
public:
    GUI() : _running(true), _loginSuccess(false) {}

    void run();

    void menu()
    {
        cout << "******************************" << endl;
        cout << "  欢迎来到聊天室!请输入你的选项：  " << endl;
        cout << "         1.登录        " << endl;
        cout << "         2.注册        " << endl;
        cout << "         3.注销        " << endl;
        cout << "         4.退出        " << endl;
        cout << "******************************" << endl;
    }
    // 登陆页面
    void dologin()
    {
        cout << "请输入您的登录名:" << endl;
        cin.ignore(); // 忽略之前输入留下的换行符
        cin.getline(_name, sizeof(_name));
        cout << "请输入您的密码:" << endl;
        cin.getline(_pwd, sizeof(_pwd));
        if (!(_db.QueryName(_name)))
            cout << "该用户还没有注册" << endl;
        if (_db.CheckUserDate(_name, _pwd))
        {
            cout << "登陆成功!" << endl;
            strcpy(_userInfo.name, _name);
            strcpy(_userInfo.pwd, _pwd);
            cout << "********1.进入聊天室*********" << endl;
            cout << "********2.停留登陆页面********" << endl;
            char a;
            cin >> a;
            if (a == '1')
            {
                _loginSuccess = true;
                _running = false; // 停止运行
            }
            else
                cout << "登陆成功！"<< endl;
        }
        else
            cout << "用户名或密码错误！请重新输入！" << endl;
    }
    // 注册页面
    void doregister()
    {
        cout << "欢迎注册!" << endl;
        cout << "请输入您的注册名称" << endl;
        cin.ignore();
        cin.getline(_name, sizeof(_name));
        cout << "请输入您的密码:" << endl;
        cin.getline(_pwd, sizeof(_pwd));
        if (_db.QueryName(_name))
            cout << "该用户已经注册过" << endl;
        else
        {
            _db.WriteUserDate(_name, _pwd);
            cout << "用户名" << _name << "注册成功" << endl;
        }
    }
    // 注销
    void dologout()
    {
        cout << "请输入您的用户名称" << endl;
        cin.ignore();
        cin.getline(_name, sizeof(_name));
        if (!(_db.QueryName(_name)))
        {
            cout << "该用户不存在" << endl;
            return;
        }
        cout << "请输入您的密码：" << endl;
        cin.getline(_pwd, sizeof(_pwd));
        cout << "确认要注销你的账号吗?T or F" << endl;
        char in;
        cin >> in;
        cout << "你的选择是:" << in << endl;
        if (in == 'T' || in == 't')
            _db.DeleteUserDate(_name, _pwd);
        cout << "用户" << _name << "注销成功！" << endl;
    }
    // 退出页面
    void doexit()
    {
        _running = false;
        exit(0);
    }

    bool isLoginSuccess() const
    {
        return _loginSuccess;
    }

private:
    char _name[50];
    char _pwd[50];
    bool _running;
    bool _loginSuccess;
    Base _db;
};
// FuncPointer 是一个指向 CGUI 成员函数的指针类型
typedef void (GUI::*FuncPointer)();

typedef struct _table
{
    int choice;
    FuncPointer pfunc;
} table;
table gtable[] = {
    {1, &GUI::dologin},
    {2, &GUI::doregister},
    {3, &GUI::dologout},
    {4, &GUI::doexit}};

int tlen = sizeof(gtable) / sizeof(gtable[0]);

void GUI::run()
{
    int choice;
    while (_running)
    {
        menu();
        cout << "请选择：" << endl;
        if (cin >> choice)
        {
            bool found = false;
            for (int i = 0; i < tlen; i++)
            {
                if (choice == gtable[i].choice)
                {
                    (this->*gtable[i].pfunc)(); // 解引用成员函数指针,调用该成员函数
                    found = true;
                    break;
                }
            }
            if (!found)
                cout << "无效的选择，请重新输入。" << endl;
        }
        else
        {
            cout << "输入无效，请重新输入。" << endl;
            cin.clear();                                         // 清除错误标志
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 忽略错误输入
        }
    }
}

UI gui_main()
{
    GUI gui;
    gui.run();
    if (gui.isLoginSuccess())
    {
        return _userInfo;
    }
    return UI{};
}