#include "head.hpp"
#include "TUI.hpp"
#include "HHH.hpp"
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

    close(fd);
    return 0;
}