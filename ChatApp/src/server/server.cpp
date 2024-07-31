#include "head.hpp"
#include "serv_main.hpp"

class RedisServer
{
public:
    RedisServer(const string &hostname, int port)
    {
        context = redisConnect(hostname.c_str(), port);
        if (context == nullptr || context->err)
        {
            cerr << "Redis 连接失败" << endl;
            if (context)
                redisFree(context);
            throw runtime_error("Redis 连接失败");
        }
        cout << "redis连接成功" << endl;
    }

    ~RedisServer()
    {
        if (context)
            redisFree(context);
    }

    // 存储用户信息
    bool setPassword(const string &username, const string &password)
    {
        redisReply *reply = (redisReply *)redisCommand(context, "SET %s %s", username.c_str(), password.c_str());
        if (reply == nullptr)
        {
            cerr << "Redis SET 命令失败" << endl;
            return false;
        }

        bool success = (reply->type == REDIS_REPLY_STATUS && strcmp(reply->str, "OK") == 0);
        freeReplyObject(reply);
        return success;
    }
    // 判断用户是否注册
    bool isUser(const string &username, const string &password)
    {
        redisReply *reply = (redisReply *)redisCommand(context, "GET %s", username.c_str());
        if (reply == nullptr)
        {
            cerr << "Redis GET 命令失败" << endl;
            return false;
        }

        bool registered = false;
        if (reply->type == REDIS_REPLY_STRING && password == reply->str)
            registered = true;

        freeReplyObject(reply);
        return registered;
    }
    // 判断好友是否存在
    bool friends_exit(const string &username)
    {
        redisReply *reply = (redisReply *)redisCommand(context, "EXISTS %s", username.c_str());
        if (reply == nullptr)
        {
            cerr << "Redis EXISTS 命令失败" << endl;
            return false;
        }
        // 检查返回的回复类型和内容
        bool registered = false;
        if (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1)
            registered = true;

        freeReplyObject(reply);
        return registered;
    }
    // 删除用户信息
    bool deleteUser(const string &username)
    {
        redisReply *reply = (redisReply *)redisCommand(context, "DEL %s", username.c_str());
        if (reply == nullptr)
        {
            cerr << "Redis DEL 命令失败" << endl;
            return false;
        }
        bool success = (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1);
        freeReplyObject(reply);
        return success;
    }

private:
    redisContext *context;
};

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
    ev.events = EPOLLIN | EPOLLET; // 写事件
    ev.data.fd = lfd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev) == -1)
        err_("epoll_ctl");

    struct epoll_event evs[1024];
    int size = sizeof(evs) / sizeof(struct epoll_event);

    // 连接到redis服务器
    RedisServer redisServer("127.0.0.1", 6379);

    // 握手成功
    while (1)
    {
        cout << "等待事件......" << endl;
        int n = epoll_wait(epfd, evs, size, -1); // 检测事件发生
        for (int i = 0; i < n; ++i)
        {
            int fd = evs[i].data.fd; // 取出当前的文件描述符
            if (fd == lfd)           // 判断文件描述符是否用于监听
            {
                int cfd = accept(fd, nullptr, nullptr); // 建立新的连接
                if (cfd == -1)
                    err_("accept");

                cout << "新连接建立,fd: " << cfd << endl;

                // 文件描述符修改为非阻塞
                int flag = fcntl(cfd, F_GETFL);
                flag |= O_NONBLOCK;
                fcntl(cfd, F_SETFL, flag);

                // 新得到的文件描述符添加到epoll模型中
                struct epoll_event ev;
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = cfd;

                if (epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev) == -1) // 拷贝
                    err_("epoll_ctl");
            }
            else // 通信
            {
                string buf(128, '\0');
                // 循环读数据
                while (1)
                {
                    int len = recv(fd, &buf[0], buf.size(), 0);
                    if (len == -1)
                    {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                        {
                            cout << "数据接受完毕......" << endl;
                            break;
                        }
                        else
                            err_("recv");
                    }
                    else if (len == 0)
                    {
                        cout << "----------用户fd: " << fd << "下线----------" << endl;
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr); // 将这个文件描述符从epoll模型中删除
                        close(fd);
                        break;
                    }

                    buf.resize(len);
                    cout << "服务端接收到数据: " << buf << endl;
                    try
                    {
                        json request = json::parse(buf);
                        // 好友管理，群管理
                        if (request.contains("manage-manage"))
                        {
                            // 业务处理
                            serv_main(fd, request);
                        }
                        // 用户信息删除
                        else if (request.contains("xxxxx")) // 检查是否包含 "xxxxx" 键
                        {
                            string name = request["name"].get<string>();
                            bool success = redisServer.deleteUser(name);

                            string response = success ? "deleteOK" : "deleteNO";
                            if (send(fd, response.c_str(), response.size(), 0) == -1)
                                err_("send");
                            cout << response << endl;
                        }
                        // 用户信息注册
                        else if (request.contains("------sssss------"))
                        {
                            string name = request["name"].get<string>();
                            string pwd = request["pwd"].get<string>();
                            bool success = redisServer.setPassword(name, pwd);

                            string response = success ? "setOK" : "setNO";
                            if (send(fd, response.c_str(), response.size(), 0) == -1)
                                err_("send");

                            cout << response << endl;
                        }
                        // 判断好友是否存在
                        else if (request.contains("-----firends_exit-----"))
                        {
                            string name = request["name"].get<string>();
                            bool success = redisServer.friends_exit(name);

                            string response = success ? "OK" : "NO";
                            if (send(fd, response.c_str(), response.size(), 0) == -1)
                                err_("send");

                            cout << response << endl;
                        }
                        // 判断用户是否注册
                        else if (request.contains("---charge_user---"))
                        {
                            string name = request["name"].get<string>();
                            string pwd = request["pwd"].get<string>();
                            bool registered = redisServer.isUser(name, pwd);

                            string response = registered ? "IS" : "NO";
                            if (send(fd, response.c_str(), response.size(), 0) == -1)
                                err_("send");

                            cout << response << endl;
                        }
                    }
                    catch (const json::parse_error &e)
                    {
                        cerr << "服务端JSON解析错误: " << e.what() << endl;
                        continue;
                    }
                }
            }
        }
    }
    close(lfd);
    return 0;
}