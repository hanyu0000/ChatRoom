#include "server.h"
#include "t_main.h"
int main()
{
    signal(SIGCHLD, handler);
    struct sockaddr_in serv;

    int fd = socket(AF_INET, SOCK_STREAM, 0); // 创建一个通讯端点，返回端点文件描述符
    if (fd == -1)
        err_("socket");

    // 对serv变量成员进行初始化
    const char *server_ip = "127.0.0.1";
    serv.sin_family = AF_INET;
    serv.sin_port = htons(8080);
    serv.sin_addr.s_addr = inet_addr(server_ip);

    int b = bind(fd, (struct sockaddr *)&serv, sizeof(serv)); // 将fd和本地地址端口号进行绑定
    if (b == -1)
        err_("bind");
    // 将fd设置为被动连接,监听客户端连接
    // 将客户端的连接放入未决连接队列
    // 指定未决队列长度
    listen(fd, 5);

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
        int c_fd = accept(fd, nullptr, nullptr); // 提取一个进行处理，返回连接描述符，与客户端进行通讯
        if (c_fd == -1)
            err_("accept");
        pid_t pid = fork(); // 创建子进程，子进程继承父进程的文件描述符
        if (pid == -1)
            err_("fork");
        if (pid == 0) // 子进程
        {
            close(fd);

            char b1[BUFFER_SIZE] = {0};
            char b2[BUFFER_SIZE] = {0};
            char name[BUFFER_SIZE] = {0};
            char pwd[BUFFER_SIZE] = {0};
            ssize_t _r = read(fd, b1, BUFFER_SIZE - 1);
            if (_r < 0)
                err_("read");
            b1[_r] = '\0';
            strncpy(name, b1, BUFFER_SIZE);

            _r = read(fd, b2, BUFFER_SIZE - 1);
            if (_r < 0)
                err_("read");
            b2[_r] = '\0';
            strncpy(pwd, b2, BUFFER_SIZE);
    
            // 存储用户名和密码到Redis
            if(name && pwd){
                redisReply *reply;
                reply = (redisReply*)redisCommand(redis_Context,"用户名称 : %s用户密码 : %s",name,pwd);
                if(reply == nullptr){
                    cerr << "Redis传输数据失败" << endl;
                    redisFree(redis_Context);
                    exit(1);
                }
                freeReplyObject(reply);
                cout << "用户 " << name << "登陆到聊天室" << endl;
                t_main(c_fd); // 业务处理
            }

            close(c_fd); // 关闭本次连接
            exit(0);
        }
        else // 父进程
            close(c_fd);
    }
    redisFree(redis_Context);
    return 0;
}