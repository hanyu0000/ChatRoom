#include "head.hpp"
#include "task.hpp"
RedisServer redis;
namespace fs = filesystem;
map<string, string> current_chat_map;
string getCurrentChatObject(const string &user);
// 收文件
void recv_file(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    string f_name = j["name"];
    off_t filesize = j["filesize"];

    string filename = j["filename"];
    string directory = "/home/zxc/files";

    struct stat st;
    if (stat(directory.c_str(), &st) == -1)
        if (mkdir(directory.c_str(), 0755) == -1)
            err_("mkdir failed");

    string filepath = directory + "/" + filename; //
    cout << "文件路径为:" << filepath << endl;
    redis.storeFilePath(my_name, f_name, filepath); // 存文件路径

    int file_fd = open(filepath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (file_fd == -1)
        err_("打开文件失败");

    char buffer[4 << 20];
    off_t sum = 0;
    ssize_t len;
    while (sum < filesize)
    {
        len = recv(fd, buffer, sizeof(buffer), 0);
        if (len > 0)
        {
            if (write(file_fd, buffer, len) != len)
            {
                close(file_fd);
                err_("write file");
            }
            sum += len;
        }
        else if (len == 0)
        {
            cout << "连接关闭!!!" << endl;
            break;
        }
        else if (errno == EAGAIN)
            this_thread::sleep_for(chrono::milliseconds(50));
        else
        {
            close(file_fd);
            err_("recv_file");
        }
    }
    close(file_fd);
}
void charge_file(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    pair<string, string> file_info = redis.getFilePath(my_name);
    string filepath = file_info.first;
    string sender = file_info.second;
    if (filepath.empty())
    {
        cout << "文件路径为空，没有文件消息！" << endl;
        json to_my = {
            {"have_file", "NO"}};
        string message = to_my.dump();
        if (IO::send_msg(fd, message) == -1)
            err_("send_msg");
        return;
    }
    else
    {
        cout << "有文件消息！" << endl;
        json to_my = {
            {"have_file", sender}};
        string message = to_my.dump();
        if (IO::send_msg(fd, message) == -1)
            err_("send_msg");
    }
}
// 发文件
void send_file(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    if (j.contains("NNN"))
    {
        cout << "用户不接收该文件消息!" << endl;
        return;
    }
    else if (j.contains("YYY"))
    {
        pair<string, string> file_info = redis.getFilePath(my_name);
        string filepath = file_info.first;
        cout << "文件路径:" << filepath << endl;
        int file_fd = open(filepath.c_str(), O_RDONLY);
        if (file_fd == -1)
            err_("打开文件失败");

        struct stat info;
        if (fstat(file_fd, &info) == -1)
            err_("fstat");

        size_t filesize = info.st_size;
        cout << "文件大小:" << filesize << endl;
        // 发送文件大小给接收者
        json json = {
            {"type", "sendfile"},
            {"filename", fs::path(filepath).filename().string()},
            {"filesize", filesize}};
        string message = json.dump();
        if (IO::send_msg(fd, message) == -1)
            err_("send_msg");

        // 发送文件内容
        off_t sum = 0;
        while (sum < filesize)
        {
            ssize_t len = sendfile(fd, file_fd, &sum, filesize - sum);
            if (len == -1)
            {
                if (errno == EAGAIN)
                {
                    usleep(1000);
                    continue;
                }
                else
                {
                    cerr << "Error during sendfile: " << strerror(errno) << endl;
                    cerr << "Details:" << endl;
                    cerr << "  Current offset: " << sum << endl;
                    cerr << "  Remaining bytes: " << filesize - sum << endl;
                    cerr << "  File descriptor: " << file_fd << endl;
                    cerr << "  Destination descriptor: " << fd << endl;
                    err_("sendfile");
                }
            }
        }
        close(file_fd);
        cout << "文件发送完成！" << endl;
    }
}
// 注册
void doregister(int fd, json j)
{
    string name = j["name"].get<string>();
    string pwd = j["pwd"].get<string>();

    string reply;
    bool flag = redis.friends_exit(name);
    string buf = flag ? "exitOK" : "exitNO";
    if (buf == "exitNO") // 用户未注册
    {
        bool success = redis.setPassword(name, pwd);
        reply = success ? "setOK" : "setNO";
    }
    else
        reply = buf; // 用户已注册

    cout << reply << endl;
    json u_i =
        {
            {"register", reply}};
    string str = u_i.dump();
    if (IO::send_msg(fd, str) == -1)
        err_("send_msg");
}
// 注册
void login(int fd, json j)
{
    string name = j["name"].get<string>();
    fd_user(fd, name);
}
// 注销
void logout(int fd, json j)
{
    string name = j["name"].get<string>();
    string pwd = j["pwd"].get<string>();

    bool success = redis.deleteUser(name); // 删除用户名
    redis.removeAllFriends(name);          // 删除好友列表
    redis.removeAllBlockedUsers(name);     // 删除屏蔽列表
    redis.removeUserGroupList(name);       // 删除群组列表
    redis.removeUserFromAllGroups(name);   // 退群
    redis.removeGroupsByMaster(name);      // 删除用户创建的所有群聊
    redis.removeChatRecordsByUser(name);   // 删除所有聊天记录
    string reply = success ? "deleteOK" : "deleteNO";
    cout << reply << endl;
    json u_i =
        {
            {"logout", reply}};
    string str = u_i.dump();
    if (IO::send_msg(fd, str) == -1)
        err_("send_msg");
}
// 登录--是否注册
void isUser(int fd, json j)
{
    string name = j["name"].get<string>();
    string pwd = j["pwd"].get<string>();

    string reply;
    if (is_name_present(name))
    {
        cout << "该用户已经登录" << endl;
        reply = "loading";
    }
    else
    {
        bool registered = redis.isUser(name, pwd);
        reply = registered ? "IS USER" : "NO USER";
    }
    cout << reply << endl;
    json u_i =
        {
            {"isUser", reply}};
    string str = u_i.dump();
    if (IO::send_msg(fd, str) == -1)
        err_("send_msg");
}
// 群聊
void g_chat(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;
    string group = j["group"].get<string>();

    string hhh = j["message"].get<string>();
    if (hhh == "exit")
    {
        cout << "客户端退出聊天" << endl;
        json to = {
            {"g_chat", hhh}};
        string age = to.dump();
        if (IO::send_msg(fd, age) == -1)
            err_("send_msg");
        cout << "成功转发退出群聊消息！" << endl;
    }
    string reply = my_name + ':' + j["message"].get<string>(); // 消息
    cout << reply << endl;
    redis.storeGroupMessage(group, reply); // 存聊天记录

    vector<string> userlist = redis.getGroupMembers(group);
    cout << "群聊 < " << group << " > 的用户列表: " << endl;
    for (const auto &name : userlist)
    {
        int f_fd = get_fd(name, client_map);
        if (f_fd == -1 || f_fd == fd)
        {
            cout << name << "不在线" << endl;
            continue;
        }
        cout << f_fd << ":" << name << "在线" << endl;
        json to = {
            {"g_chat", reply}};
        string message = to.dump();
        if (IO::send_msg(f_fd, message) == -1)
            err_("send_msg");
        cout << "成功转发群聊消息！" << endl;
    }
}
// 请求群聊天记录
void g_chatHistry(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;
    string group = j["group"].get<string>();

    vector<string> g_chat = redis.getGroupMessages(group);
    for (const auto &message : g_chat)
        cout << message << endl;
    if (g_chat.empty())
        cout << "没有聊天记录！" << endl;

    json to_my = {
        {"g_chatHistry", g_chat}};
    string message = to_my.dump();
    if (IO::send_msg(fd, message) == -1)
        err_("send_msg");
}
// 好友聊天
void f_chat(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;
    string f_name = j["name"].get<string>();
    current_chat_map[my_name] = f_name;
    cout << my_name << ":" << f_name << endl;

    string reply = j["message"].get<string>(); // 消息
    if (reply == "12345zxcvb")
        return;
    // 是否被屏蔽
    if (redis.isUserBlocked(my_name, f_name))
    {
        json to = {
            {"chat", "blocked"},
            {"f_name", my_name}};
        string message = to.dump();
        if (IO::send_msg(fd, message) == -1)
            err_("send_msg");
        return;
    }

    json to = {
        {"chat", reply},
        {"f_name", my_name}};
    string message = to.dump();
    if (reply == "exit")
    {
        if (IO::send_msg(fd, message) == -1)
            err_("send_msg");
        current_chat_map.erase(my_name);
        cout << "客户端退出聊天" << endl;
        return;
    }

    int f_fd = get_fd(f_name, client_map);
    string name = getCurrentChatObject(f_name);
    cout << f_name << "当前的聊天对象" << name << endl;
    if (f_fd == -1 || name != my_name)
    {
        redis.storeOfflineMessage(my_name, f_name, reply); // 离线
        return;
    }

    redis.storeChatRecord(f_name, my_name, reply); // 存聊天记录
    if (IO::send_msg(f_fd, message) == -1)
        err_("send_msg");
    cout << "聊天消息转发成功！ " << endl;
}
//  请求聊天记录
void f_chatHistry(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;
    string f_name = j["name"].get<string>();

    vector<string> Chat = redis.getChatRecord(my_name, f_name);
    for (const auto &message : Chat)
        cout << message << endl;
    json to_my = {
        {"f_chatHistry", Chat}};
    string message = to_my.dump();
    if (IO::send_msg(fd, message) == -1)
        err_("send_msg");
}
// 好友添加 addfriend
void f_add(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;
    string f_name = j["name"].get<string>();

    // 判断用户是否存在
    bool userexit = redis.friends_exit(f_name);
    cout << "userexit:" << userexit << endl;
    if (!userexit) // 用户不存在
    {
        json a =
            {
                {"nopeople", f_name}};
        string str = a.dump();
        if (IO::send_msg(fd, str) == -1)
            err_("send_msg");
        return;
    }
    redis.storeFriendRequest(f_name, my_name); // 存储好友申请
    cout << "好友申请已存储至 Redis" << endl;
}
//  请求好友申请消息
void newfriend_leave(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;

    while (1)
    {
        json b;
        // 离线消息
        string sender = redis.getAndRemoveFriendRequest(my_name);
        if (sender == "NO")
        {
            b = {
                {"newfriend_leave", "NO"}};
            string message = b.dump();
            if (IO::send_msg(fd, message) == -1)
                err_("send");
            break;
        }
        else
        {
            b = {
                {"newfriend_leave", sender}};
            string message = b.dump();
            if (IO::send_msg(fd, message) == -1)
                err_("send");
        }
    }
    cout << "发送好友申请消息成功!" << endl;
}
// 好友添加回复
void f_addreply(int fd, json j)
{
    string f_name = get_name(fd, client_map);
    cout << "处理来自 <" << f_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;
    string my_name = j["name"].get<string>();
    int my_fd = get_fd(my_name, client_map); // 发出申请

    string reply = j["reply"].get<string>();
    cout << reply << endl;
    if (reply == "YES")
    {
        json to_my = {
            {"newfriendreply", reply},
            {"f_name", f_name}};
        string message = to_my.dump();
        cout << "同意加好友" << endl;
        redis.addFriend(my_name, f_name);
        redis.addFriend(f_name, my_name);
        if (IO::send_msg(fd, message) == -1)
            err_("send_msg");
    }
    else if (reply == "NO")
    {
        json to_my = {
            {"newfriendreply", reply},
            {"f_name", f_name}};
        string message = to_my.dump();
        cout << "不同意加好友" << endl;
        if (IO::send_msg(fd, message) == -1)
            err_("send_msg");
    }
}
//  请求离线消息f_chat_leave
void f_chat_leave(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;
    string f_name = j["name"].get<string>();

    json to;
    // 有离线消息
    if (redis.hasOfflineMessageFromSender(my_name, f_name))
    {
        vector<string> reply = redis.getOfflineMessages(my_name, f_name);
        to = {
            {"f_chat_leave", reply}};
        for (const auto &message : reply)
            cout << message << endl;
    }
    else
    {
        to = {
            {"f_chat_leave", "NO"}};
        cout << "没有离线消息转发成功！" << endl;
    }
    string message = to.dump();
    if (IO::send_msg(fd, message) == -1)
        err_("send");
}
// 屏蔽列表
void block_list(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;

    vector<string> block_list = redis.getBlockedUsers(my_name);
    for (const auto &name : block_list)
        cout << my_name << "的屏蔽名单:" << name << endl;
    json to_my = {
        {"block_list", block_list}};
    string message = to_my.dump();
    if (IO::send_msg(fd, message) == -1)
        err_("send_msg");
}
// 我创建的群聊列表
void my_group_list(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;

    vector<string> list = redis.getGroupsByUser(my_name);
    for (const auto &name : list)
        cout << my_name << "创建的群聊名单:" << name << endl;
    json to_my = {
        {"mygrouplist", list}};
    string message = to_my.dump();
    if (IO::send_msg(fd, message) == -1)
        err_("send_msg");
}
// 设置管理员
void add_manager(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;

    string group = j["group"].get<string>();
    vector<string> members = j["members"];
    for (const auto &member : members)
        redis.addAdminToGroup(group, member); // 群主加管理员
}
// 删除管理员
void delete_manager(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;

    string group = j["group"].get<string>();
    vector<string> members = j["members"];
    for (const auto &member : members)
        redis.removeAdminFromGroup(group, member); // 群主加管理员
}
// 管理员列表
void managelist(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    string group = j["group"].get<string>();
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;

    vector<string> managelist = redis.getManagers(group);
    for (const auto &name : managelist)
        cout << my_name << "的管理员名单:" << name << endl;
    json to_my = {
        {"managelist", managelist}};
    string message = to_my.dump();
    if (IO::send_msg(fd, message) == -1)
        err_("send_msg");
}
// 好友列表
void f_chatlist(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;

    vector<string> chatlist = redis.getFriends(my_name);
    for (const auto &name : chatlist)
        cout << my_name << "的好友名单:" << name << endl;
    json to_my = {
        {"chatlist", chatlist}};
    string message = to_my.dump();
    if (IO::send_msg(fd, message) == -1)
        err_("send_msg");
}
// 好友屏蔽
void f_block(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;
    string f_name = j["name"].get<string>();
    int f_fd = get_fd(f_name, client_map);
    redis.blockUser(my_name, f_name);
}
// 取消屏蔽
void f_unblock(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;
    string f_name = j["name"].get<string>();
    int f_fd = get_fd(f_name, client_map);
    redis.unblockUser(my_name, f_name); // 从屏蔽列表取出
}
// 好友删除
void f_delete(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;
    string f_name = j["name"].get<string>();
    int f_fd = get_fd(f_name, client_map);

    bool userexit = redis.friends_exit(f_name);
    cout << "userexit:" << userexit << endl;
    if (!userexit)
    {
        json a =
            {
                {"nopeople", f_name}};
        string str = a.dump();
        if (IO::send_msg(fd, str) == -1)
            err_("send_msg");
        return;
    }
    redis.removeFriend(my_name, f_name);
    redis.removeFriend(f_name, my_name);
    json a = {
        {"deletefriend", "YES"},
        {"f_name", f_name}};
    string message = a.dump();
    if (IO::send_msg(fd, message) == -1)
        err_("send_msg");
}
// 查看群用户
void g_showuser(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;
    string group = j["group"].get<string>();
    cout << group << endl;

    vector<string> userlist = redis.getGroupMembers(group);
    cout << "群聊 < " << group << " > 的用户列表: " << endl;
    for (const auto &grp : userlist)
        cout << grp << endl;

    json response = {
        {"type", "userlist"},
        {"userlist", userlist}};
    string message = response.dump();
    if (IO::send_msg(fd, message) == -1)
        err_("send_msg");
}
// 删除群成员
void delete_people(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;
    string group = j["group"].get<string>();
    string name = j["name"].get<string>();

    json response;
    // 我是群主，可以删任何人
    if (redis.isGroupMaster(group, my_name))
    {
        redis.removeMemberFromGroup(group, name);
        response = {
            {"type", "OK"}};
    }
    // 我是管理员，可以删普通用户
    else if (redis.isGroupManager(group, my_name))
    {
        if (redis.isGroupMaster(group, my_name))
        {
            response = {
                {"type", "delete_people"}};
        }
        else
        {
            redis.removeMemberFromGroup(group, name);
            response = {
                {"type", "OK"}};
        }
    }
    else
    {
        response = {
            {"type", "delete_people"}};
    }
    string message = response.dump();
    if (IO::send_msg(fd, message) == -1)
        err_("send_msg");
}
// 查询群聊
void g_showlist(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;

    vector<string> grouplist = redis.getUserGroupList(my_name);
    cout << "用户 < " << my_name << " > 的群聊列表: " << endl;

    for (const auto &grp : grouplist)
        cout << grp << endl;

    json response = {
        {"type", "grouplist"},
        {"grouplist", grouplist}};
    string message = response.dump();
    if (IO::send_msg(fd, message) == -1)
        err_("send_msg");
}
// 创建群聊
void g_create(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;

    string group = j["group"].get<string>();
    cout << "创建群聊: " << group << endl;
    // 群聊-群主
    redis.setGroupMaster(group, my_name);

    redis.createGroup(group);            // 创建群聊
    redis.mycreateGroup(group, my_name); // 加入群聊列表

    vector<string> members = j["members"];
    redis.addMemberToGroup(group, my_name);
    redis.addUserToGroupList(my_name, group);
    for (const auto &member : members)
    {
        cout << "添加成员: " << member << endl;
        redis.addMemberToGroup(group, member);   // 向群聊添加成员
        redis.addUserToGroupList(member, group); // 群聊添加至成员列表
    }
}
// 删除群聊
void g_disband(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;

    string group = j["group"].get<string>();
    cout << group << endl;

    redis.deleteGroup(group, my_name);             // 删除群聊
    redis.deleteAllGroupMessages(group);           // 删除聊天记录
    redis.removeUserFromGroupList(my_name, group); // 从群列表里面删除
    vector<string> members = redis.getGroupMembers(group);
    for (const auto &member : members)
        redis.removeUserFromGroupList(member, group);
}
// 加入群聊
void g_join(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;
    string group = j["group"].get<string>();
    cout << group << endl;

    if (!redis.groupExists(group)) // 群聊不存在
    {
        json Json = {
            {"type", "join_group"},
            {"group", "NO"}};
        string str = Json.dump();
        if (IO::send_msg(fd, str) == -1)
            cerr << "发送消息失败" << endl;
        return;
    }

    string mess = my_name + ":" + group;
    // 获取群成员
    vector<string> members = redis.getGroupMembers(group);
    for (const auto &member : members)
    {
        // 是群主或管理员
        if (redis.isGroupManager(group, member) || redis.isGroupMaster(group, member))
        {
            cout << member << endl;
            redis.storeGroupRequest(member, mess);
        }
    }
}
// 处理入群申请
void g_reply(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;

    while (1)
    {
        auto result = redis.getAndRemoveGroupRequest(my_name);
        string sender = result.first;
        if (sender == "NO")
        {
            json b = {
                {"g_reply", "NO"}};
            string message = b.dump();
            if (IO::send_msg(fd, message) == -1)
                err_("send");
            break;
        }
        else
        {
            // 同意申请
            string group = result.second;
            string hhh = sender + ":" + group;
            redis.removeGroupRequest(my_name, hhh);
            json b = {
                {"g_reply", sender},
                {"group", group}};
            string message = b.dump();
            if (IO::send_msg(fd, message) == -1)
                err_("send");
        }
    }
    cout << "发送群聊申请消息成功!" << endl;
}
// 入群回复
void g_addreply(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;
    string f_name = j["name"].get<string>();

    string reply = j["reply"].get<string>();
    cout << reply << endl;
    if (reply == "YES")
    {
        cout << "同意入群" << endl;
        string group = j["group"].get<string>();
        redis.addMemberToGroup(group, f_name);   // 向群聊添加成员
        redis.addUserToGroupList(f_name, group); // 群聊添加至成员列表
    }
    else if (reply == "NO")
        cout << "不同意入群" << endl;
}
// 退出群聊
void g_leave(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;
    string group = j["group"].get<string>();
    cout << "用户 <" << my_name << "> 退出群聊: " << group << endl;
    redis.removeUserFromGroupList(my_name, group);
    redis.removeMemberFromGroup(group, my_name); // 退出群聊
}
string get_name(int fd, const map<int, string> &client_map)
{
    auto it = client_map.find(fd);
    if (it != client_map.end())
        return it->second;
    return "";
}
int get_fd(const string &username, const map<int, string> &client_map)
{
    for (const auto &pair : client_map)
        if (pair.second == username)
            return pair.first;
    return -1;
}
string getCurrentChatObject(const string &user)
{
    auto it = current_chat_map.find(user);
    if (it != current_chat_map.end())
    {
        return it->second; // 返回当前聊天对象
    }
    return "NO";
}