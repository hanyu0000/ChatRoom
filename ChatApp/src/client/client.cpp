#include "head.hpp"
#include "TUI.hpp"
#include "ThreadPool.hpp"

void signal_handler(int signal) {}
// 用于处理用户请求
void _user_requests(int fd)
{
    signal(SIGINT, signal_handler);
    TUI tui;
    tui.run(fd);

    cout << " -------------------------------------------------" << endl;
    cout << "" << endl;
    cout << " --------------欢迎来到聊天室！！！---------------" << endl;
    cout << " " << endl;
    cout << " -------------------------------------------------" << endl;
    cout << " " << endl;
    cout << " " << endl;
    cout << " " << endl;

    HHH jjj;
    jjj.run(fd);

    getchar();
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " <server-ip>" << endl;
        exit(1);
    }

    ThreadPool pool(4);
    // 创建socket设备，返回设备文件描述符
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
        err_("socket");

    struct sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_port = htons(8080);
    inet_pton(AF_INET, argv[1], &serv.sin_addr);

    // 使用fd向服务器发起连接
    if (connect(fd, (struct sockaddr *)&serv, sizeof(serv)) == -1)
        err_("connect");

    // 三次握手成功
    auto task = bind(_user_requests, fd);
    pool.addTask(task);

    // 保持主线程的运行
    this_thread::sleep_for(chrono::minutes(10));

    close(fd);
    return 0;
}