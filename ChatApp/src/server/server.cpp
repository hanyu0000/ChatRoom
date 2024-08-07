#include "head.hpp"
#include "ThreadPool.hpp"
#include "task.hpp"
map<int, string> client_map;
void handleClientMessage(int fd, const json &j);
void process_client_messages(int fd);

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
            if (evs[i].events & EPOLLIN)
            {
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
                else
                    process_client_messages(fd);
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

void process_client_messages(int fd)
{
    ThreadPool pool(4, 10);

    // 读取消息头，获取消息长度
    uint32_t msg_len = 0;
    int ret = Util::readn(fd, sizeof(uint32_t), (char *)&msg_len);
    if (ret != sizeof(uint32_t))
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            cout << "服务端数据接受完毕......" << endl;
        else
            cerr << "recv 错误: " << strerror(errno) << endl;
        return;
    }

    msg_len = ntohl(msg_len); // 将消息长度从网络字节序转换为主机字节序

    // 根据消息长度分配缓冲区并读取消息体
    char *buffer = new char[msg_len + 1];
    memset(buffer, 0, msg_len + 1);
    ret = Util::readn(fd, msg_len, buffer);
    if (ret != msg_len)
    {
        if (ret == 0)
        {
            cout << "----------用户fd: " << fd << "下线----------" << endl;
            epoll_ctl(fd, EPOLL_CTL_DEL, fd, nullptr); // 从epoll模型中删除文件描述符
            client_map.erase(fd);                      // 从客户端映射中删除
            close(fd);
        }
        else
            cerr << "读取消息体时出错: " << strerror(errno) << endl;
        delete[] buffer;
        return;
    }

    // 处理消息
    string message(buffer, ret);
    delete[] buffer;
    try
    {
        auto j = json::parse(message);
        // bind()方法会创建一个新函数，称为绑定函数，当调用这个绑定函数时，绑定函数会以创建它时传入
        // bind()方法的第一个参数作为 this，传入 bind() 方法的第二个
        // 以及以后的参数加上绑定函数运行时本身的参数按照顺序作为原函数的参数来调用原函数。
        auto task = bind(handleClientMessage, fd, j);
        pool.addTask(task);
    }
    catch (const json::parse_error &e)
    {
        cerr << "JSON 解析失败: " << e.what() << endl;
    }
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
    else if (msg_type == "addfriendreply")
    {
        f_addreply(fd, j);
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