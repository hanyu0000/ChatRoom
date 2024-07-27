#include "gui_main.hpp"
#include "head.hpp"

class TUI
{
public:
    void run();
    void meun()
    {
        cout << "    1.好友管理        2.群管理    " << endl;
        char a;
        cin >> a;
        if (a == '1')
        {
            cout << "**********************************" << endl;
            cout << "*   1.查询好友列表     2.屏蔽好友    *" << endl;
            cout << "*   3.添加好友        4.删除好友    *" << endl;
            cout << "*   5.好友聊天                     *" << endl;
            cout << "********************************" << endl;
        } 
/*
 群管理
     实现群组的创建、解散
     实现用户申请加入群组
     实现用户查看已加入的群组
     实现群组成员退出已加入的群组
     实现群组成员查看群组成员列表
     实现群主对群组管理员的添加和删除
     实现群组管理员批准用户加入群组
     实现群组管理员/群主从群组中移除用户
     实现群组内聊天功能
*/
        if (a == '2')
        {
            cout << "**********************************" << endl;
            cout << "*   1.查询好友列表     2.屏蔽好友    *" << endl;
            cout << "*   3.添加好友        4.删除好友    *" << endl;
            cout << "*   5.好友聊天                     *" << endl;
            cout << "*                         *" << endl;
            cout << "*                         *" << endl;
            cout << "*                         *" << endl;
            cout << "********************************" << endl;
        }
    }

private:
};
void TUI::run()
{
    meun();
}
int content()
{
    TUI tui;
    tui.run();
    return 0;
}