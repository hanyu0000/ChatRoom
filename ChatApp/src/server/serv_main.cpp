#include "head.hpp"
#include "serv_main.hpp"

void serv_main(int my_fd, const json &request)
{
    cout << "JSON: " << request.dump(4) << endl;
    string buf(128, '\0');
    int friend_fd;
    // 好友
    try
    {
        if (request.contains("user_choice") && request["user_choice"].get<string>() == "add-add")
        {
            string friend_name = request["fri_name"].get<string>();
            string user_name = request["name"].get<string>();
            cout << "好友名称: " << "fri_name" << endl;
            cout << "用户名称: " << "name" << endl;

            // 发送消息到好友客户端
            json message;
            message["from"] = user_name;
            message["message"] = "你有一个来自新朋友的好友申请： " + user_name;
            string message_str = message.dump();
            if (send(friend_fd, message_str.c_str(), message_str.size(), 0) == -1)
                err_("send");
            else
            {
                cout << "成功发送好友申请给用户: " << friend_name << endl;
                json response;
                response["message"] = "yyy";
                string response_str = response.dump();
                if (send(my_fd, response_str.c_str(), response_str.size(), 0) == -1)
                    err_("send");
            }
        }
    }
    catch (const json::exception &e)
    {
        cerr << "JSON 处理错误: " << e.what() << endl;
    }
}