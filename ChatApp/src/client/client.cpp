#include "head.hpp"
#include "TUI.hpp"

void signal_handler(int signal) {}
atomic<bool> stop_flag(false);
mutex mtx;
condition_variable cv;
queue<string> message_queue;

void receive_messages(int fd)
{
    string buf(128, '\0');
    while (!stop_flag)
    {
        int n = read(fd, &buf[0], buf.size());
        if (n > 0)
        {
            buf.resize(n);
            lock_guard<mutex> lock(mtx);
            message_queue.push(buf);
            cv.notify_one();
        }
        else if (n == 0)
        {
            cout << "服务器关闭连接......" << endl;
            stop_flag = true;
            cv.notify_one();
        }
        else
        {
            err_("read");
            stop_flag = true;
            cv.notify_one();
        }
    }
}

void process_messages(int fd) // 处理
{
    while (!stop_flag || !message_queue.empty())
    {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, []
                { return !message_queue.empty() || stop_flag; });

        while (!message_queue.empty())
        {
            string buf = message_queue.front();
            message_queue.pop();
            lock.unlock();
            try
            {
                json message = json::parse(buf);
                if (message.contains("---------new.friend-------"))
                {
                    string f_name = message["my_name"];
                    cout << "用户< " << f_name << " >, 向您发送了一条好友申请！" << endl;
                    cout << "您是否同意? T or F" << endl;
                    char a;
                    string reply;
                    while (1)
                    {
                        cin >> a;
                        if (a == 'T' || a == 't')
                        {
                            reply = "YES";
                            break;
                        }
                        else if (a == 'F' || a == 'f')
                        {
                            reply = "NO";
                            break;
                        }
                        else
                            cout << "输入错误！请重新输入：" << endl;
                    }
                    json ui = {
                        {"----------manage----------", "manage"},
                        {"----------choice----------", "add-friend"},
                        {"reply", reply},
                    };
                    string str = ui.dump();
                    lock_guard<mutex> lock(mtx);
                    if (write(fd, str.c_str(), str.size()) == -1)
                        err_("write");
                }
            }
            catch (json::parse_error &e)
            {
                cerr << "JSON解析错误: " << e.what() << endl;
            }

            lock.lock();
        }
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

    cout << " " << endl;
    cout << " " << endl;
    cout << " " << endl;
    cout << " -------------------------------------------------" << endl;
    cout << "" << endl;
    cout << " --------------欢迎来到聊天室！！！---------------" << endl;
    cout << " " << endl;
    cout << " -------------------------------------------------" << endl;
    cout << " " << endl;
    cout << " " << endl;
    cout << " " << endl;

    // 线程安全的队列：message_queue 存储从服务端接收到的消息。使用 mutex 和 condition_variable 确保线程安全和同步
    // receive_messages 线程：读取数据并将其放入队列中，同时通知 process_messages 线程有新消息到达
    // process_messages 线程：等待消息队列中有数据到达，然后处理这些消息。使用 condition_variable 来同步和等待新消息
    // 主线程：处理用户输入并管理程序的运行状态。用户输入 q 可以退出程序并通知其他线程停止工作

    thread receiver_thread(receive_messages, fd);
    thread processor_thread(process_messages, fd);

    char a;
    while (1)
    {
        cout << "*       1.查看消息页面           2.进入管理页面    *" << endl;
        cout << "" << endl;
        cout << "" << endl;
        cin >> a;
        if (a == '1')
        { // 主线程处理查看消息的逻辑
            unique_lock<mutex> lock(mtx);
            while (!message_queue.empty())
            {
                string buffer = message_queue.front();
                message_queue.pop();
            }
        }
        else if (a == '2')
        {
            HHH jjj;
            jjj.run(fd);
        }
        else
            cout << "请输入正确选项！" << endl;
    }

    // 等待接收消息的线程结束
    if (receiver_thread.joinable())
        receiver_thread.join();

    if (processor_thread.joinable())
        processor_thread.join();

    close(fd);
    return 0;
}