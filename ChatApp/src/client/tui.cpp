#include <iostream>
#include <cstdio>
#include <cstring>
#include <limits>
#include "tui_main.hpp"
using namespace std;

class Base
{
public:
    Base()
    {
        _phead = new Node();
    }

    ~Base() // 析沟函数：释放链表中的所有节点，防止内存泄漏
    {
        Node *ptr = _phead;
        while (ptr != nullptr)
        {
            Node *next = ptr->getNext();
            delete ptr;
            ptr = next;
        }
    }

    bool CheckUserDate(char *name, char *pwd) // 判断用户名、密码是否匹配
    {
        Node *ptr = _phead->getNext();
        while (ptr != nullptr)
        {
            if ((strcmp(name, ptr->_name) == 0 && strcmp(pwd, ptr->_pwd) == 0))
                return true;
            ptr = ptr->getNext();
        }
        return false;
    }

    bool QueryName(char *name) // 名字是否匹配
    {
        Node *ptr = _phead->getNext();
        while (ptr != nullptr)
        {
            if (strcmp(name, ptr->_name) == 0)
                return true;
            ptr = ptr->getNext();
        }
        return false;
    }

    void WriteUserDate(char *name, char *pwd)
    {
        Node *p = new Node(name, pwd);
        p->setNext(_phead->getNext());
        _phead->setNext(p);
    }

    void DeleteUserDate(char *name, char *pwd)
    {
        Node *prev = _phead;
        Node *cur = prev->getNext();

        while (cur != nullptr)
        {
            if (strcmp(cur->_name, name) == 0 && strcmp(cur->_pwd, pwd) == 0)
            {
                prev->setNext(cur->getNext());
                delete cur;
                return;
            }
            prev = cur;
            cur = cur->getNext();
        }
    }

private:
    struct Node
    {
        Node() : _pnext(nullptr) {} // 构造函数

        Node(char *n, char *p) : _pnext(nullptr)
        {
            strcpy(_name, n);
            strcpy(_pwd, p);
        }

        char _name[50];
        char _pwd[50];
        Node *_pnext;

        void setNext(Node *p)
        {
            _pnext = p;
        }

        Node *getNext()
        {
            return _pnext;
        }
    };
    Node *_phead;
};

UI _userInfo;
class TUI
{
public:
    TUI() : _running(true), _loginSuccess(false) {}

    void run();

    void menu()
    {
        cout << "********************************" << endl;
        cout << "* 欢迎来到聊天室!请输入你的选项 *" << endl;
        cout << "*           1.登录              *" << endl;
        cout << "*           2.注册              *" << endl;
        cout << "*           3.注销              *" << endl;
        cout << "*           4.退出              *" << endl;
        cout << "********************************" << endl;
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
            cout << "登录成功!" << endl;
            strcpy(_userInfo.name, _name);
            strcpy(_userInfo.pwd, _pwd);
            cout << "********1.进入聊天室*********" << endl;
            cout << "********2.停留登录页面********" << endl;
            char a;
            cin >> a;
            if (a == '1')
            {
                _loginSuccess = true;
                _running = false; // 停止运行
            }
            else
                cout << "登录成功！" << endl;
        }
        else
            cout << "用户名或密码错误！请重新输入！" << endl;
    }
    // 注册页面
    void doregister()
    {
        cout << "欢迎注册!" << endl;
        cout << "请输入您的注册名称:" << endl;
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
typedef void (TUI::*FuncPointer)();

typedef struct _table
{
    int choice;
    FuncPointer pfunc;
} table;
table gtable[] = {
    {1, &TUI::dologin},
    {2, &TUI::doregister},
    {3, &TUI::dologout},
    {4, &TUI::doexit}};

int len = sizeof(gtable) / sizeof(gtable[0]);

void TUI::run()
{
    int choice;
    while (_running)
    {
        menu();
        cout << "请选择：" << endl;
        if (cin >> choice)
        {
            bool found = false;
            for (int i = 0; i < len; i++)
            {
                if (choice == gtable[i].choice)
                {
                    (this->*gtable[i].pfunc)(); // 解引用成员函数指针,调用该成员函数
                    found = true;
                    break;
                }
            }
            if (!found)
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

UI tui_main()
{
    TUI tui;
    tui.run();
    if (tui.isLoginSuccess())
        return _userInfo;
    return UI{};
}