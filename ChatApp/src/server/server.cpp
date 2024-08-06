#include "head.hpp"
#include "ThreadPool.hpp"
#include "serv_main.hpp"
void handleClientMessage(int fd, const json &j);

void runServer(int port)
{
    // 创建一个通讯端点，返回端点文件描述符
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1)
        perror("socket");

    // 设置端口复用
    int opt = 1;
    if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        perror("setsockopt");

    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    serv.sin_addr.s_addr = htonl(INADDR_ANY);

    // 将fd和本地地址端口号进行绑定
    if (bind(lfd, (struct sockaddr *)&serv, sizeof(serv)) == -1)
        perror("bind");

    // 将fd设置为被动连接,监听客户端连接
    if (listen(lfd, 64) == -1)
        perror("listen");

    // 创建一个新的epoll实例，并返回一个文件描述符，这个文件描述符用于以后对这个epoll实例进行操作
    int epfd = epoll_create(100);
    if (epfd == -1)
        perror("epoll_create");

    // 往epoll实例中添加需要检测的节点，监听文件的描述符
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET; // 写事件
    ev.data.fd = lfd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev) == -1)
        perror("epoll_ctl");

    struct epoll_event evs[1024];
    int size = sizeof(evs) / sizeof(struct epoll_event);

    ThreadPool pool(4, 10);

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
                while (1)
                {
                    // 建立新的连接
                    int c_fd = accept(fd, nullptr, nullptr);
                    if (c_fd == -1)
                    {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                            break;
                        else
                            perror("accept");
                    }
                    cout << "新连接建立,文件描述符为: " << c_fd << endl;
                    // 文件描述符修改为非阻塞
                    int flags = fcntl(fd, F_GETFL, 0);
                    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

                    // 将客户端的socket加入epoll模型中
                    struct epoll_event ev;
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = c_fd;

                    if (epoll_ctl(epfd, EPOLL_CTL_ADD, c_fd, &ev) == -1)
                        perror("epoll_ctl");
                }
            }
            else
            {
                // 从树上去除该套接字
                struct epoll_event temp;
                temp.data.fd = fd;
                temp.events = EPOLLIN || EPOLLET;
                epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &temp);

                char buffer[1024];
                memset(buffer, 0, 1024);
                int r = read(fd, buffer, sizeof(buffer));
                if (r == -1)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                    {
                        cout << "服务端数据接受完毕......" << endl;
                        break;
                    }
                    else
                        err_("recv");
                }
                else if (r == 0)
                {
                    cout << "----------用户fd: " << fd << "下线----------" << endl;
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr); // 将这个文件描述符从epoll模型中删除
                    client_map.erase(fd);                        // 从客户端映射中删除
                    close(fd);
                    break;
                }
                if (r > 0)
                {
                    string message(buffer, r);
                    try
                    {
                        auto j = json::parse(message);
                        pool.addTask([fd, j]
                                     { handleClientMessage(fd, j); });
                    }
                    catch (const json::parse_error &e)
                    {
                        cerr << "JSON 解析失败: " << e.what() << endl;
                    }
                }
            }
        }
    }
    close(lfd);
    close(epfd);
}
int main()
{
    runServer(8080);
    return 0;
}

void handleClientMessage(int fd, const json &j)
{
    string msg_type = j["type"];
    cout << msg_type << endl;
    if (msg_type == "register")
    {
        doregister(fd, j); // 注册
    }
    else if (msg_type == "logout")
    {
        logout(fd, j); // 注销
    }
    else if (msg_type == "isUser")
    {
        isUser(fd, j); // 是否注册
    }
    else if (msg_type == "chatlist")
    {
        f_chatlist(fd, j); // 好友列表
    }
    else if (msg_type == "chat")
    {
        f_chat(fd, j); // 好友聊天
    }
    else if (msg_type == "blockfriend")
    {
        f_block(fd, j); // 好友屏蔽
    }
    else if (msg_type == "unblockfriend")
    {
        f_unblock(fd, j); // 好友取消屏蔽
    }
    else if (msg_type == "addfriend")
    {
        f_add(fd, j); // 好友添加
    }
    else if (msg_type == "deletefriend")
    {
        f_delete(fd, j); // 好友删除
    }
    else if (msg_type == "userlist")
    {
        g_showuser(fd, j); // 查看群用户
    }
    else if (msg_type == "grouplist")
    {
        g_showlist(fd, j); // 查询群聊
    }
    else if (msg_type == "create_group")
    {
        g_create(fd, j); // 创建群聊
    }
    else if (msg_type == "disband_group")
    {
        g_disband(fd, j); // 删除群聊
    }
    else if (msg_type == "leave_group")
    {
        g_leave(fd, j); // 退出群聊
    }
    else if (msg_type == "join_group")
    {
        g_join(fd, j); // 加入群聊
    }
}

void fd_user(int fd, string &name)
{
    client_map[fd] = name;
    cout << "用户:" << name << "  fd:" << fd << endl;
}
bool is_name_present(const string &name)
{
    for (const auto &pair : client_map)
        if (pair.second == name)
            return true;
    return false;
}