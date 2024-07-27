#include "head.hpp"
#include "t_main.hpp"
int main()
{
    const char *server_ip = "127.0.0.1";
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
    listen(lfd, 64);

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
        int n = epoll_wait(epfd, evs, size, -1);
        for (int i = 0; i < n; ++i)
        {
            int curfd = evs[i].data.fd; // 取出当前的文件描述符

            if (curfd == lfd) // 判断文件描述符是否用于监听
            {
                // 建立新的连接
                int c_fd = accept(curfd, nullptr, nullptr);
                if (c_fd == -1)
                    err_("accept");
                // 新得到的文件描述符添加到epoll模型中, 下一轮循环的时候就可以被检测了
                ev.events = EPOLLIN; // 读缓冲区是否有数据
                ev.data.fd = c_fd;

                if (epoll_ctl(epfd, EPOLL_CTL_ADD, c_fd, &ev) == -1)
                    err_("epoll_ctl");
            }
            else
            {
                // 接收数据
                char buf[128];
                ssize_t r = read(curfd, buf, sizeof(buf));

                if (r == 0)
                {
                    cout << "客户端断开连接" << endl;
                    epoll_ctl(epfd, EPOLL_CTL_DEL, curfd, nullptr);
                    close(curfd);
                }

                string data(buf, r);
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
                
                t_main(curfd); // 业务处理
            }
        }
    }
    redisFree(redis_Context);
    return 0;
}