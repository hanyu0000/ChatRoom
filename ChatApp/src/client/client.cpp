#include "head.hpp"
#include "TUI.hpp"

void signal_handler(int signal) {}

void process_response(string &response, int fd)
{
    try
    {
        json j = json::parse(response);
        if (j.contains("show.friend.list"))
        {
            string value = j["show.friend.list"];
            if (value == "YES")
                cout << "服务器确认显示好友列表" << endl;
            if (value == "NO")
                cout << "您没有任何好友记录" << endl;
            return;
        }
        else if (j.contains("new.friend"))
        {
            string value = j["new.friend"];
            cout << "你有一条来自：" << value << "的好友申请" << endl;
            cout << "是否同意该用户的好友申请? T or F" << endl;
            char a;
            while (1)
            {
                cin >> a;
                json mas;
                if (a == 'T' || a == 't')
                {
                    mas =
                        {
                            {"<manage>", "<manage>"},
                            {"add_friendreply", "YES"},
                        };
                    string message = mas.dump();
                    if (send(fd, message.c_str(), message.size(), 0) == -1)
                        err_("send");
                    cout << "同意好友申请发送成功" << endl;
                    break;
                }
                else if (a == 'F' || a == 'f')
                {
                    mas =
                        {
                            {"<manage>", "<manage>"},
                            {"add_friendreply", "NO"},
                        };
                    string message = mas.dump();
                    if (send(fd, message.c_str(), message.size(), 0) == -1)
                        err_("send");
                    cout << "不同意好友申请发送成功" << endl;
                    break;
                }
                else
                    cout << "请按要求输入正确选项:" << endl;
            }
            return;
        }
        else if (j.contains("new.friend.reply"))
        {
            string value = j["new.friend.reply"];
            if (value == "YES")
            {
                string f_name = j["f_name"];
                cout << "您成功与用户: " << f_name << " 建立好友关系" << endl;
            }
            else if (value == "NO")
            {
                string f_name = j["f_name"];
                cout << "用户:" << f_name << "不同意你的好友申请！" << endl;
            }
            return;
        }
        else
            cout << "没有接收到好友列表" << endl;
    }
    catch (const json::parse_error &e)
    {
        cerr << "JSON 解析错误: " << e.what() << endl;
    }
    catch (const json::type_error &e)
    {
        cerr << "JSON 类型错误: " << e.what() << endl;
    }
    catch (const exception &e)
    {
        cerr << "错误: " << e.what() << endl;
    }
}

int main(int argc, char *argv[])
{
    // signal(SIGINT, signal_handler);
    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " <server-ip>" << endl;
        exit(1);
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
        err_("socket");

    struct sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_port = htons(8080);
    inet_pton(AF_INET, argv[1], &serv.sin_addr);

    if (connect(fd, (struct sockaddr *)&serv, sizeof(serv)) == -1)
        err_("connect");

    // 三次握手成功

    TUI tui;
    tui.run(fd);

    cout << "" << endl;
    cout << "" << endl;
    cout << "" << endl;
    cout << " ---------------------------------------------------" << endl;
    cout << "" << endl;
    cout << " ---------------欢迎来到聊天室！！！----------------" << endl;
    cout << "" << endl;
    cout << " ---------------------------------------------------" << endl;
    cout << "" << endl;
    cout << "" << endl;

    char a;
    while (1)
    {
        cout << "" << endl;
        cout << "" << endl;
        cout << "按 < 1 > 进入管理页面             < 2 > 进入消息页面" << endl;
        cout << "" << endl;
        cout << "" << endl;
        cin >> a;
        if (a == '1')
        {
            HHH jjj;
            jjj.run(fd);
        }
        else if (a == '2')
        {
            string buffer(128, '\0');
            int valread = read(fd, &buffer[0], 1024);
            if (valread > 0)
            {
                buffer.resize(valread);
                cout << buffer << endl;
                process_response(buffer, fd);
            }
            else
                cerr << "读取数据失败" << endl;
        }
        else
            cout << "请输入正确数字:" << endl;
    }
    getchar();
    close(fd);
    return 0;
}