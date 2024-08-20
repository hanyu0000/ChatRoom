#include "head.hpp"
#include "HHH.hpp"
typedef void (HHH::*Pointer)(int fd);
typedef struct HHH_table
{
    int choice;
    Pointer p;
} tab;
//全局数组
tab gtab[] = {
    {1, &HHH::f_add},
    {2, &HHH::f_reply},
    {3, &HHH::f_delete},
    {4, &HHH::f_chat},
    {5, &HHH::f_block},
    {6, &HHH::f_unblock},
    {7, &HHH::g_showlist},
    {8, &HHH::g_showuser},
    {9, &HHH::g_join},
    {10, &HHH::g_leave},
    {11, &HHH::g_create},
    {12, &HHH::g_disband},
    {13, &HHH::g_add_manager},
    {14, &HHH::g_delete_manager},
    {15, &HHH::g_delete_people},
    {16, &HHH::file_pass},
    {17, &HHH::g_chat},
    {18, &HHH::g_reply},
    {19, &HHH::last},
};
void HHH::menu()
{
    cout << "" << endl;
    cout << "----------------------------------------------------" << endl;
    cout << "           1.好友申请          2.申请消息                 " << endl;
    cout << "           3.删除好友          4.好友聊天                 " << endl;
    cout << "           5.屏蔽好友          6.取消屏蔽                 " << endl;
    cout << "           7.查询群聊          8.查群用户                 " << endl;
    cout << "           9.加入群聊          10.退出群聊               " << endl;
    cout << "           11.创建群聊         12.解散群聊               " << endl;
    cout << "           13.添加管理员       14.删除管理员              " << endl;
    cout << "           15.移除群用户       16.文件传输               " << endl;
    cout << "           17.群聊            18.群聊申请                " << endl;
    cout << "                  19.回到登录页面                      " << endl;
    cout << "----------------------------------------------------" << endl;
    cout << "" << endl;
}
int HHH_len = sizeof(gtab) / sizeof(gtab[0]);
void HHH::run(int fd)
{
    int choice;
    while (running)
    {
        menu();
        cout << "" << endl;
        cout << "请选择：" << endl;
        if (cin >> choice)
        {
            bool flag = false;
            for (int i = 0; i < HHH_len; i++)
            {
                if (choice == gtab[i].choice)
                {
                    (this->*gtab[i].p)(fd);
                    flag = true;
                    if (choice == 19)
                        return;
                    break;
                }
            }
            if (!flag)
                cout << "输入无效，请重新输入:" << endl;
        }
        else
        {
            cout << "输入无效，请重新输入:" << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
}