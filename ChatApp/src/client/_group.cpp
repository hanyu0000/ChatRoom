#include "head.hpp"
#include "_main.hpp"

class GROU
{
public:
    void run();
    void meun()
    {
        cout << "**********************************" << endl;
        cout << "*                                *" << endl;
        cout << "*    1.创建群聊       2.解散群聊    *" << endl;
        cout << "*    3.查看群聊       4.退出群聊    *" << endl;
        cout << "*    5.查看成员列表                *" << endl;
        cout << "*                                *" << endl;
        cout << "**********************************" << endl;
    }

private:
};
void GROU::run()
{
    meun();
}
void _group(int fd)
{
    GROU grop;
    grop.run();
}