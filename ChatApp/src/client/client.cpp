#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "gui_main.h"
using namespace std;
void err_(const char *n)
{
    if (n)
        perror(n);
    else
        perror("Error\n");
    exit(1);
}
int main(int argc, char *argv[])
{
    struct sockaddr_in serv;
    int fd = socket(AF_INET, SOCK_STREAM, 0); // 创建socket设备，返回设备文件描述符
    if (fd == -1)
        err_("socket");
    // 服务器信息初始化
    serv.sin_family = AF_INET;
    serv.sin_port = htons(8080);
    inet_pton(AF_INET, argv[1], &serv.sin_addr);

    int c = connect(fd, (struct sockaddr *)&serv, sizeof(serv)); // 使用fd向服务器发起连接
    if (c == -1)
        err_("connect");
    // 三次握手成功
    while (1)
    {
        UI user_info = gui_main();
        cout << " -------------------欢迎来到聊天室！！！ -------------------" << endl;
        if (strlen(user_info.name) > 0 && strlen(user_info.pwd) > 0)
        {
            cout << "用户名称: " << user_info.name << endl;
            cout << "用户密码: " << user_info.pwd << endl;

            // 在这里可以添加跳转到其他处理逻辑的代码

            if (write(fd, user_info.name, strlen(user_info.name)) == -1)
                err_("name");
            if (write(fd, user_info.pwd, strlen(user_info.pwd)) == -1)
                err_("pwd");
        }
    }
    close(fd);
    return 0;
}