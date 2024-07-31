#include "head.hpp"
#include "_main.hpp"

class FRI
{
public:
    FRI() : _running(true) {}

    void run(int fd, const string &name);
    void menu();
    void f_chat(int fd, const string &name);
    void f_block(int fd, const string &name);
    void f_add(int fd, const string &name);
    void f_remove(int fd, const string &name);
    void f_showlist(int fd, const string &name);

private:
    bool _running;
    void read_response(int fd, const string &name);
};
void FRI::menu()
{
    cout << "**********************************" << endl;
    cout << "*                                *" << endl;
    cout << "*   1.好友聊天       2.屏蔽好友   *" << endl;
    cout << "*   3.添加好友       4.删除好友   *" << endl;
    cout << "*   5.查询好友列表                *" << endl;
    cout << "*                                *" << endl;
    cout << "**********************************" << endl;
}

void FRI::f_chat(int fd, const string &name)
{
    return;
}
void FRI::f_block(int fd, const string &name)
{
    return;
}
void FRI::f_add(int fd, const string &name)
{
    cout << "---------------欢迎 " << name << " 添加好友---------------" << endl;
    string fri_name;
    cout << "请输入您的好友名称:" << endl;
    cin.ignore();
    getline(cin, fri_name);

    json u_i =
        {
            {"-----firends_exit-----", "----------"},
            {"name", fri_name},
        };
    string str = u_i.dump();
    if (write(fd, str.c_str(), str.size()) == -1)
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
        if (buf == "OK")
        {
            cout << "该用户已经注册！可以发送好友申请" << endl;
            getchar();
        }
        else if (buf == "NO")
        {
            cout << "该用户还未注册，请输入正确用户名" << endl;
        }
    }
}

void FRI::f_remove(int fd, const string &name)
{
    return;
}
void FRI::f_showlist(int fd, const string &name)
{
    return;
}
void FRI::read_response(int fd, const string &name)
{
    return;
}

typedef void (FRI::*Pointer)(int fd, const string &name);
typedef struct _table
{
    int choice;
    Pointer p;
} tab;

tab gtab[] = {
    {1, &FRI::f_chat},
    {2, &FRI::f_block},
    {3, &FRI::f_add},
    {4, &FRI::f_remove},
    {5, &FRI::f_showlist},
};

int fri_len = sizeof(gtab) / sizeof(gtab[0]);

void FRI::run(int fd, const string &name)
{
    int choice;
    while (_running)
    {
        menu();
        cout << "请选择：" << endl;
        if (cin >> choice)
        {
            bool flag = false;
            for (int i = 0; i < fri_len; i++)
            {
                if (choice == gtab[i].choice)
                {
                    (this->*gtab[i].p)(fd, name); // 解引用成员函数指针,调用该成员函数
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
void _friend(int fd, const string &name)
{
    FRI fri;
    fri.run(fd, name);
}