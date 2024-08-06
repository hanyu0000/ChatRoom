#include "head.hpp"
#include "TUI.hpp"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " <server-ip>" << endl;
        exit(1);
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
        err_("socket");

    struct sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_port = htons(8080);
    inet_pton(AF_INET, argv[1], &serv.sin_addr);

    if (connect(fd, (struct sockaddr *)&serv, sizeof(serv)) == -1)
        err_("connect");
        
    while (1)
    {
        TUI tui;
        string name = tui.run(fd);

        cout << "" << endl;
        cout << "" << endl;
        cout << " ---------------------------------------------------" << endl;
        cout << "" << endl;
        cout << " ---------------欢迎来到聊天室！！！----------------" << endl;
        cout << "" << endl;
        cout << " ---------------------------------------------------" << endl;
        cout << "" << endl;
        cout << "" << endl;

        main_menu(fd, name);
    }
    close(fd);
    return 0;
}