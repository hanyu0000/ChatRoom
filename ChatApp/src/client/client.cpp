#include "head.hpp"
#include "tui_main.hpp"

int main(int argc, char *argv[])
{
    struct sockaddr_in serv;
    int fd = socket(AF_INET, SOCK_STREAM, 0); // 创建socket设备，返回设备文件描述符
    if (fd == -1)
        err_("socket");

    serv.sin_family = AF_INET;
    serv.sin_port = htons(8080);
    inet_pton(AF_INET, argv[1], &serv.sin_addr);

    int c = connect(fd, (struct sockaddr *)&serv, sizeof(serv)); // 使用fd向服务器发起连接
    if (c == -1)
        err_("connect");
    // 三次握手成功
    while (1)
    {
        UI user_info = tui_main();
        cout << " -------------------欢迎来到聊天室！！！ -------------------" << endl;
        if (strlen(user_info.name) > 0 && strlen(user_info.pwd) > 0)
        {
            cout << "用户名称: " << user_info.name << endl;
            cout << "用户密码: " << user_info.pwd << endl;

            json u_i =
                {
                    {"name", user_info.name},
                    {"pwd", user_info.pwd},
                };
            // 将 JSON 对象转换为字符串
            string str = u_i.dump();
            ssize_t w = write(fd, str.c_str(), str.size());
            if (w == -1)
                err_("write");

            char buf[128] = {0};
            int r = read(fd, buf, sizeof(buf) - 1);
            if (r == -1)
                err_("read");
            // thread_pool();
            if (r > 0)
                cout << "服务器响应" << endl;
            buf[r] = '\0'; // 确保字符串以 null 结尾
            cout << "从服务端接收到: " << buf << endl;

            // int content();
        }
    }
    close(fd);
    return 0;
}