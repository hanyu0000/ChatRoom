#ifndef HHH_H
#define HHH_H
#include <iostream>
class HHH
{
public:
    HHH() : running(true), f_stop(false), g_stop(false) {}

    void run(int fd);
    void menu();
    void g_chat(int fd);
    void f_reply(int fd);
    void f_chat(int fd);
    void f_block(int fd);
    void f_unblock(int fd);
    void f_add(int fd);
    void f_delete(int fd);
    void g_reply(int fd);

    void g_create(int fd);
    void g_showlist(int fd);
    void g_join(int fd);
    void g_showuser(int fd);
    void g_leave(int fd);
    void g_disband(int fd);
    void g_add_manager(int fd);// 设置管理员(群主)
    void g_delete_manager(int fd);// 删除管理员(群主)
    void g_delete_people(int fd);// 移除群用户(群主/管理员)
    void file_pass(int fd);

private:
    atomic<bool> running;
    atomic<bool> f_stop; // 用于停止接收线程
    atomic<bool> g_stop;
};

#endif