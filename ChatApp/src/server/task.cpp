#include "head.hpp"
#include "task.hpp"
// 实例化 RedisServer 对象
RedisServer redis;
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
    if (Util::send_msg(fd, str) == -1)
        err_("send_msg");
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
    if (Util::send_msg(fd, str) == -1)
        err_("send_msg");
}
void login(int fd, json j)
{
    string name = j["name"].get<string>();
    fd_user(fd, name);
}
// 是否注册
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
    if (Util::send_msg(fd, str) == -1)
        err_("send_msg");
}
// 文件//file
void file(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    string f_name = j["name"];
    int f_fd = get_fd(f_name, client_map);
    string filename = j["filename"];
    off_t filesize = j["filesize"];

    // 创建临时文件以接收数据
    string temp_filename = "/tmp/" + filename;
    int file_fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (file_fd == -1)
        err_("open file fail");

    off_t sum = 0;
    ssize_t len;
    char buffer[4096];
    while (sum < filesize)
    {
        len = recv(fd, buffer, sizeof(buffer), 0);
        if (len <= 0)
        {
            perror("recv");
            break;
        }
        if (write(file_fd, buffer, len) != len)
        {
            perror("write");
            break;
        }
        sum += len;
    }
    close(file_fd);
    // 将文件存储到 Redis 中
    redis.store_file(my_name, f_name, temp_filename);
    // 删除临时文件
    remove(temp_filename.c_str());

    // 通知
    json to_my = {
        {"have_file", my_name}};
    string message = to_my.dump();
    if (Util::send_msg(f_fd, message) == -1)
        err_("send_msg");
}
void send_file(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    string f_name = j["name"];
    int f_fd = get_fd(f_name, client_map);
    string filename = j["filename"];

    string file_content = redis.retrieve_file(f_name, my_name, filename);

    json u_i = {
        {"type", "file"},
        {"filename", filename},
        {"filesize", file_content.size()}};
    string message = u_i.dump();
    if (Util::send_msg(fd, message) == -1)
        err_("send_msg");

    const char *data = file_content.data();
    size_t total_size = file_content.size();
    size_t sent_size = 0;
    ssize_t len;

    while (sent_size < total_size)
    {
        len = send(fd, data + sent_size, total_size - sent_size, 0);
        if (len <= 0)
        {
            perror("send");
            break;
        }
        sent_size += len;
    }
    cout << "文件发送完成！" << endl;
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
    if (Util::send_msg(fd, message) == -1)
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
    if (Util::send_msg(fd, message) == -1)
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
    {
        cout << "添加管理员: " << member << endl;
        redis.addAdminToGroup(group, member); // 群主加管理员
    }
}
// 删除管理员
void delete_manager(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;

    string group = j["group"].get<string>();
    vector<string> members = j["members"];
    for (const auto &member : members)
    {
        cout << "添加管理员: " << member << endl;
        redis.removeAdminFromGroup(group, member); // 群主加管理员
    }
}
// 管理员列表
void managelist(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;

    vector<string> managelist = redis.getManagers(my_name);
    for (const auto &name : managelist)
        cout << my_name << "的管理员名单:" << name << endl;
    json to_my = {
        {"managelist", managelist}};
    string message = to_my.dump();
    if (Util::send_msg(fd, message) == -1)
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
    if (Util::send_msg(fd, message) == -1)
        err_("send_msg");
}
// 好友聊天
void f_chat(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;
    string f_name = j["name"].get<string>();
    int f_fd = get_fd(f_name, client_map);

    string reply = j["message"].get<string>();
    json to = {
        {"chat", reply},
        {"f_name", my_name}};
    string message = to.dump();
    if (Util::send_msg(f_fd, message) == -1)
        err_("send_msg");
    cout << " 转发消息成功！ " << endl;
    // 存离线消息
    redis.storeChatRecord(f_name, my_name, reply);
}
//  请求聊天记录
void f_chatHistry(int fd, json j)
{
    return;
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
// 好友添加 addfriend
void f_add(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;
    string f_name = j["name"].get<string>();
    int f_fd = get_fd(f_name, client_map);

    // 判断用户是否存在
    bool userexit = redis.friends_exit(f_name);
    cout << "userexit:" << userexit << endl;
    if (!userexit) // 用户不存在
    {
        json a =
            {
                {"nopeople", f_name}};
        string str = a.dump();
        if (Util::send_msg(fd, str) == -1)
            err_("send_msg");
        return;
    }

    json b = {
        {"newfriend", my_name}};
    string message = b.dump();
    if (Util::send_msg(f_fd, message) == -1)
        err_("send_msg");
    cout << "发送好友申请成功!" << endl;
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
        if (Util::send_msg(fd, message) == -1)
            err_("send_msg");
    }
    else if (reply == "NO")
    {
        json to_my = {
            {"newfriendreply", reply},
            {"f_name", f_name}};
        string message = to_my.dump();
        cout << "不同意加好友" << endl;
        if (Util::send_msg(fd, message) == -1)
            err_("send_msg");
    }
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
        if (Util::send_msg(fd, str) == -1)
            err_("send_msg");
        return;
    }
    redis.removeFriend(my_name, f_name);
    redis.removeFriend(f_name, my_name);
    json a = {
        {"deletefriend", "YES"},
        {"f_name", f_name}};
    string message = a.dump();
    if (Util::send_msg(fd, message) == -1)
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
    if (Util::send_msg(fd, message) == -1)
        err_("send_msg");
}
// 删除群成员
void delete_people(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;
    string group = j["group"].get<string>();
    string name = j["name"].get<string>();

    // 我是群主，可以删任何人
    if (redis.isGroupOwner(group, my_name))
        redis.removeMemberFromGroup(group, name);
    // 我是管理员，可以删普通用户
    else if (redis.isGroupManager(group, my_name))
    {
        if (redis.isGroupOwner(group, name) || redis.isGroupManager(group, name))
        {
            json response = {
                {"type", "delete_people"}};
            string message = response.dump();
            if (Util::send_msg(fd, message) == -1)
                err_("send_msg");
        }
        else
            redis.removeMemberFromGroup(group, name);
    }
    else
    {
        json response = {
            {"type", "delete_people"}};
        string message = response.dump();
        if (Util::send_msg(fd, message) == -1)
            err_("send_msg");
    }
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
    if (Util::send_msg(fd, message) == -1)
        err_("send_msg");
}
// 创建群聊
void g_create(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;

    string group = j["group"].get<string>();
    cout << "创建群聊: " << group << endl;

    redis.createGroup(group);            // 创建群聊
    redis.mycreateGroup(group, my_name); // 加入我的群聊列表

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

    redis.deleteGroup(group, my_name); // 删除群聊
    redis.removeUserFromGroupList(my_name, group);
    vector<string> members = redis.getGroupMembers(group);
    for (const auto &member : members)
    {
        redis.removeUserFromGroupList(member, group);
        redis.removeUserFromGroupList(member, group);
    }
}
// 加入群聊
void g_join(int fd, json j)
{
    string my_name = get_name(fd, client_map);
    cout << "处理来自 <" << my_name << ">: " << fd << " 的请求: " << j.dump(3) << endl;

    string group = j["group"].get<string>();
    cout << group << endl;
    // 获取群成员
    vector<string> members = redis.getGroupMembers(group);
    for (const auto &member : members)
    {
        // 是群主或管理员
        if (redis.isGroupOwner(group, member) || redis.isGroupManager(group, member))
        {
            string request = "User " + my_name + " 请求加入群组 " + group;
            int manager_fd = get_fd(member, client_map);
            // 获取fd
        }
    }
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