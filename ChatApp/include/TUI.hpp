#ifndef TUI_H
#define TUI_H
#include <iostream>
class TUI
{
public:
    TUI() : _running(true) {}

    void run(int fd);
    void menu();
    void dologin(int fd);
    void doregister(int fd);
    void dologout(int fd);

private:
    bool _running;
    int read_response(int fd, const string &name, const string &pwd);
};

class HHH
{
public:
    HHH() : running(true) {}

    void run(int fd);
    void menu();
    void f_chat(int fd);
    void f_block(int fd);
    void f_unblock(int fd);
    void f_add(int fd);
    void f_delete(int fd);
    void f_showlist(int fd);
    void g_create(int fd);
    void g_showlist(int fd);
    void g_join(int fd);
    void g_showuser(int fd);
    void g_leave(int fd);
    void g_disband(int fd);

private:
    bool running;
};

#endif