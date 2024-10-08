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
#endif