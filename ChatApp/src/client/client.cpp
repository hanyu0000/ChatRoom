#include "head.hpp"
#include "TUI.hpp"
#include "HHH.hpp"
atomic<bool> Heartbeat(false);
void sendHeartbeat(int fd, int time)
{
    while (!Heartbeat)
    {
        json heartbeat;
        heartbeat["type"] = "heartbeat";
        string heartbeat_message = heartbeat.dump();
        if (IO::send_msg(fd, heartbeat_message) == -1)
        {
            cerr << "发送心跳包时出错" << endl;
            Heartbeat = true;
        }
        this_thread::sleep_for(chrono::seconds(time));
    }
}

int main(int argc, char *argv[])
{
    signal(SIGTSTP, SIG_IGN); // 忽略Ctrl+Z

    if (argc != 2)
        exit(1);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
        err_("socket");

    struct sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_port = htons(8080);
    inet_pton(AF_INET, argv[1], &serv.sin_addr);

    if (connect(fd, (struct sockaddr *)&serv, sizeof(serv)) == -1)
        err_("connect");

    // 启动一个线程用于发送心跳包
    int time = 5; 
    thread heartbeatSender(sendHeartbeat, fd, time);

    TUI tui;
    tui.run(fd);
    cout << "" << endl;
    cout << " ---------------------------------------------------" << endl;
    cout << "" << endl;
    cout << " ---------------欢迎来到聊天室！！！----------------" << endl;
    cout << "" << endl;
    cout << " ---------------------------------------------------" << endl;
    cout << "" << endl;
    HHH mnue;
    mnue.run(fd);

    Heartbeat = true;
    heartbeatSender.join(); // 等待心跳包线程结束

    close(fd);
    return 0;
}