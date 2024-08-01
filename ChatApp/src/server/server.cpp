#include "head.hpp"

// 维护活跃客户端的文件描述符和用户名
map<int, string> client_map;
void fd_user(int fd, string &name)
{
    client_map[fd] = name;
    cout << "用户:" << name << "  fd:" << fd << endl;
}
string get_name(int fd, const map<int, string> &client_map);
int get_fd(const string &username, const map<int, string> &client_map);
void serv_main(int my_fd, const json &request, map<int, string> &client_map);

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
    // 判断用户名是否存在
    bool friends_exit(const string &username)
    {
        redisReply *reply = (redisReply *)redisCommand(context, "EXISTS %s", username.c_str());
        if (reply == nullptr)
        {
            cerr << "Redis EXISTS 命令失败" << endl;
            return false;
        }

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
    // 创建一个通讯端点，返回端点文件描述符
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
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
        cout << "等待事件发生......" << endl;
        // 检测事件发生
        int n = epoll_wait(epfd, evs, size, -1);
        for (int i = 0; i < n; ++i)
        {
            // 取出当前的文件描述符
            int fd = evs[i].data.fd;
            // 判断文件描述符是否用于监听
            if (fd == lfd)
            {
                // 建立新的连接
                int c_fd = accept(fd, nullptr, nullptr);
                if (c_fd == -1)
                    err_("accept");

                cout << "新连接建立,文件描述符为: " << c_fd << endl;

                // 文件描述符修改为非阻塞
                int flag = fcntl(c_fd, F_GETFL);
                flag |= O_NONBLOCK;
                fcntl(c_fd, F_SETFL, flag);

                // 新得到的文件描述符添加到epoll模型中
                struct epoll_event ev;
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = c_fd;

                if (epoll_ctl(epfd, EPOLL_CTL_ADD, c_fd, &ev) == -1) // 拷贝
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
                        client_map.erase(fd);                        // 从客户端映射中删除
                        close(fd);
                        break;
                    }
                    else
                    {
                        buf.resize(len);
                        try
                        {
                            json request = json::parse(buf);
                            if (request.contains("----------manage----------")) // 好友管理，群管理
                                serv_main(fd, request, client_map);             // 业务处理
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
                               
                                fd_user(fd, name);

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
    }
    close(lfd);
    return 0;
}
void serv_main(int my_fd, const json &request, map<int, string> &client_map)
{
    cout << "处理来自 " << my_fd << " 的请求: " << request.dump(4) << endl;
    for (const auto &pair : client_map)
        cout << "Username: " << pair.second << ", FD: " << pair.first << endl;
    try
    {
        if (request.contains("----------charge----------") && request["----------charge----------"].get<string>() == "friends")
        {
            if (request.contains("------choice------") && request["------choice------"].get<string>() == "add")
            {
                string f_name = request["name"].get<string>();
                string my_name = get_name(my_fd, client_map);

                if (my_name == "NO")
                    cout << "获取用户名错误" << endl;

                int f_fd = get_fd(f_name, client_map);

                json response;
                if (f_fd == -1)
                    response["message"] = "该用户未上线，已发送好友申请给用户";
                else
                    response["message"] = "该用户在线，已发送好友申请给用户";
                string response_str = response.dump();

                json message; // 发送消息到好友客户端
                message["message"] = f_name + ":你有一个来自新朋友的好友申请： " + my_name;
                string message_str = message.dump();
                if (send(f_fd, message_str.c_str(), message_str.size(), 0) == -1)
                    err_("send");
                else if (send(my_fd, response_str.c_str(), response_str.size(), 0) == -1)
                    err_("send");
            }
            else if (request.contains("------choice------") && request["------choice------"].get<string>() == "group")
            {
                // 群管理的逻辑
            }
        }
    }
    catch (const json::exception &e)
    {
        cerr << "JSON 处理错误: " << e.what() << endl;
    }
}

string get_name(int fd, const map<int, string> &client_map)
{
    auto it = client_map.find(fd);
    if (it != client_map.end())
        return it->second;
    else
        return "NO";
}

int get_fd(const string &name, const map<int, string> &client_map)
{
    for (const auto &pair : client_map)
    {
        if (pair.second == name)
            return pair.first;
    }
    return -1;
}