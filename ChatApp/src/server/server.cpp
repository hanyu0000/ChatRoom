#include "head.hpp"
#include "t_main.hpp"

int main()
{
    int lfd = socket(AF_INET, SOCK_STREAM, 0); // 创建一个通讯端点，返回端点文件描述符
    if (lfd == -1)
        err_("socket");

    // 设置端口复用
    int opt = 1;
    if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        err_("setsockopt");

    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(8080);
    serv.sin_addr.s_addr = htonl(INADDR_ANY);

    // 将fd和本地地址端口号进行绑定
    if (bind(lfd, (struct sockaddr *)&serv, sizeof(serv)) == -1)
        err_("bind");

    // 将fd设置为被动连接,监听客户端连接
    if (listen(lfd, 64) == -1)
        err_("listen");

    int epfd = epoll_create(100);
    if (epfd == -1)
        err_("epol_create");

    // 往epoll实例中添加需要检测的节点，监听文件的描述符
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = lfd;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev) == -1)
        err_("epoll_ctl");

    struct epoll_event evs[1024];
    int size = sizeof(evs) / sizeof(struct epoll_event);

    // 连接到redis服务器
    const char *server_ip = "127.0.0.1";
    redisContext *redis_Context = redisConnect(server_ip, 6379);
    if (redis_Context == nullptr || redis_Context->err)
    {
        if (redis_Context)
        {
            cerr << "redis连接错误" << redis_Context->errstr << endl;
            redisFree(redis_Context);
        }
        else
            cerr << "redis连接错误" << endl;
        exit(1);
    }

    while (1)
    {
        int n = epoll_wait(epfd, evs, size, -1); // 等待事件发生
        for (int i = 0; i < n; ++i)
        {
            int curfd = evs[i].data.fd; // 取出当前的文件描述符

            if (curfd == lfd) // 判断文件描述符是否用于监听
            {
                // 建立新的连接
                int c_fd = accept(curfd, nullptr, nullptr);
                if (c_fd == -1)
                    err_("accept");

                int flag = fcntl(c_fd, F_GETFL);
                flag |= O_NONBLOCK;
                fcntl(c_fd, F_SETFL, flag);

                // 新得到的文件描述符添加到epoll模型中, 下一轮循环被检测
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = c_fd;

                if (epoll_ctl(epfd, EPOLL_CTL_ADD, c_fd, &ev) == -1)
                    err_("epoll_ctl");
            }
            else
            {
                char buf[128];
                // 循环读数据
                while (1)
                {
                    int len = recv(curfd, buf, sizeof(buf), 0);
                    if (len == 0)
                    {
                        cout << "----------------用户下线 ----------------" << endl;
                        epoll_ctl(epfd, EPOLL_CTL_DEL, curfd, nullptr); // 将这个文件描述符从epoll模型中删除
                        close(curfd);
                        break;
                    }
                    else if (len > 0)
                    {
                        string data(buf, len);
                        json user_info = json::parse(data);
                        cout << "用户名称: " << user_info["name"] << endl;
                        cout << "用户密码: " << user_info["pwd"] << endl;
                        // 存储用户名和密码到Redis
                        redisReply *reply;
                        string redis_command = "用户名称" + user_info["name"].get<string>() +
                                               "用户密码" + user_info["pwd"].get<string>();
                        reply = (redisReply *)redisCommand(redis_Context, redis_command.c_str());
                        if (reply == nullptr)
                        {
                            cerr << "Redis 传输数据失败" << endl;
                            redisFree(redis_Context);
                            exit(1);
                        }
                        freeReplyObject(reply);
                        cout << "数据传输成功" << endl;

                        // 处理数据或响应
                        string response = "Hello, " + user_info["name"].get<string>();
                        write(curfd, response.c_str(), response.size());
                        write(STDOUT_FILENO, buf, len); // 接收的数据打印到终端
                        send(curfd, buf, len, 0);       // 发送数据

                        t_main(curfd); // 业务处理
                    }
                    else
                    {
                        if (errno == EAGAIN)
                        {
                            cout << "数据读完了..." << endl;
                            break;
                        }
                        else
                            err_("recv");
                    }
                }
            }
        }
    }
    redisFree(redis_Context);
    return 0;
}