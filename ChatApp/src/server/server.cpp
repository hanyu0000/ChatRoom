#include "head.hpp"
#include "ThreadPool.hpp"
#include "task.hpp"
map<int, string> client_map;
void handleClientMessage(int fd, const json &j);
void process_client_messages(int fd, int epfd);
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
        int n = epoll_wait(epfd, evs, size, 1000); // 检测事件发生
        for (int i = 0; i < n; ++i)
        {
            int fd = evs[i].data.fd;     // 取出当前的文件描述符
            if (evs[i].events & EPOLLIN) // 判断文件描述符是否用于监听
            {
                if (fd == lfd)
                {
                    int c_fd = accept(fd, nullptr, nullptr); // 建立新的连接
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
                    process_client_messages(fd, epfd);
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
void process_client_messages(int fd, int epfd)
{
    // 读取消息头，获取消息长度
    uint32_t _len = 0;
    int ret = IO::readn(fd, sizeof(uint32_t), (char *)&_len);
    if (ret != sizeof(uint32_t))
    {
        if (errno == EAGAIN)
            cout << "服务端数据接受完毕......" << endl;
        else
        {
            cout << "----------用户fd: " << fd << "下线----------" << endl;
            client_map.erase(fd);
            epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
            close(fd);
        }
        return;
    }
    _len = ntohl(_len); // 将消息长度从网络字节序转换为主机字节序
    char *buffer = new char[_len + 1];
    memset(buffer, 0, _len + 1);
    ret = IO::readn(fd, _len, buffer);
    if (ret != _len)
    {
        if (ret == 0)
        {
            cout << "----------用户fd: " << fd << "下线----------" << endl;
            epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr); // 从epoll模型中删除文件描述符
            client_map.erase(fd);                        // 从客户端映射中删除
            close(fd);
        }
        else
            cerr << "读取消息体时出错: " << strerror(errno) << endl;
        delete[] buffer;
        return;
    }
    string message(buffer, ret);
    delete[] buffer;
    try
    {
        ThreadPool pool(4, 10);
        auto j = json::parse(message);
        string type = j["type"];
        if (type == "recv_file")
        {
            auto task = bind(recv_file, fd, j);
            pool.addTask(task);
        }
        else
        {
            auto task = bind(handleClientMessage, fd, j);
            pool.addTask(task);
        }
    }
    catch (const json::parse_error &e)
    {
        cerr << "JSON 解析失败: " << e.what() << endl;
    }
    catch (const json::type_error &e)
    {
        cerr << "JSON 解析错误: " << e.what() << endl;
    }
}
void handleClientMessage(int fd, const json &j)
{
    string type = j["type"];
    cout << type << endl;
    if (type == "login")
    {
        login(fd, j); // 登录
    }
    else if (type == "send_file")
    {
        send_file(fd, j);
    }
    else if (type == "charge_file")
    {
        charge_file(fd, j);
    }
    else if (type == "register")
    {
        doregister(fd, j); // 注册
    }
    else if (type == "logout")
    {
        logout(fd, j); // 注销
    }
    else if (type == "isUser")
    {
        isUser(fd, j); // 是否注册
    }
    else if (type == "chatlist")
    {
        f_chatlist(fd, j); // 好友列表
    }
    else if (type == "mygrouplist")
    {
        my_group_list(fd, j); // 我创建的群聊列表
    }
    else if (type == "addmanage")
    {
        add_manager(fd, j); // 设置管理员
    }
    else if (type == "deletemanage")
    {
        delete_manager(fd, j); // 删除管理员
    }
    else if (type == "newfriend_leave")
    {
        newfriend_leave(fd, j);
    }
    else if (type == "managelist")
    {
        managelist(fd, j); // 管理员列表
    }
    else if (type == "delete_people")
    {
        delete_people(fd, j);
    }
    else if (type == "chat")
    {
        f_chat(fd, j); // 好友聊天
    }
    else if (type == "g_chat")
    {
        g_chat(fd, j); // 群聊
    }
    else if (type == "f_chat_leave")
    {
        f_chat_leave(fd, j); // 离线消息
    }
    else if (type == "f_chatHistry")
    {
        f_chatHistry(fd, j); // 聊天记录
    }
    else if (type == "g_chatHistry")
    {
        g_chatHistry(fd, j); // 聊天记录
    }
    else if (type == "blockfriend")
    {
        f_block(fd, j); // 好友屏蔽
    }
    else if (type == "unblockfriend")
    {
        f_unblock(fd, j); // 好友取消屏蔽
    }
    else if (type == "block_list")
    {
        block_list(fd, j); // 屏蔽列表
    }
    else if (type == "addfriend")
    {
        f_add(fd, j); // 好友添加
    }
    else if (type == "addfriendreply")
    {
        f_addreply(fd, j); // 好友申请回复
    }
    else if (type == "deletefriend")
    {
        f_delete(fd, j); // 好友删除
    }
    else if (type == "userlist")
    {
        g_showuser(fd, j); // 查看群用户
    }
    else if (type == "grouplist")
    {
        g_showlist(fd, j); // 查询群聊
    }
    else if (type == "create_group")
    {
        g_create(fd, j); // 创建群聊
    }
    else if (type == "disband_group")
    {
        g_disband(fd, j); // 删除群聊
    }
    else if (type == "leave_group")
    {
        g_leave(fd, j); // 退出群聊
    }
    else if (type == "join_group")
    {
        g_join(fd, j); // 加入群聊
    }
    else if (type == "g_reply")
    {
        g_reply(fd, j); // 群聊申请
    }
    else if (type == "apply_g_reply")
    {
        g_addreply(fd, j);
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