#ifndef USER_TO_FD_HPP
#define USER_TO_FD_HPP
#include <iostream>
#include <map>
#include <set>
#include <mutex>
#include <queue>
#include <atomic>
#include <memory>
#include <cstdio>
#include <limits>
#include <vector>
#include <thread>
#include <future>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <ncurses.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <functional>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/sendfile.h>
#include <nlohmann/json.hpp>
#include <hiredis/hiredis.h>
#include <condition_variable>
using namespace nlohmann;
using namespace std;
class IO
{
public:
    static int readn(int fd, int size, char *buf)
    {
        char *p = buf;
        int count = size;
        while (count > 0)
        {
            int len = recv(fd, p, count, 0);
            if (len == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    cout << " 继续尝试读取" << endl;
                return -1;
            }
            else if (len == 0)
                return size - count; // 连接关闭
            p += len;
            count -= len;
        }
        return size; // 返回读取的字节数
    }

    static int writen(int fd, const string &msg)
    {
        int count = msg.size();
        const char *buf = msg.c_str();
        while (count > 0)
        {
            int len = send(fd, buf, count, 0);
            if (len < 0)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    cout << " 继续尝试写入" << endl;
                return -1;
            }
            else if (len == 0)
                continue;
            count -= len;
            buf += len;
        }
        return msg.size();
    }

    static int send_msg(int fd, const string msg)
    {
        if (msg.empty() || fd < 0)
            return -1;
        uint32_t biglen = htonl(msg.size());
        string data(4 + msg.size(), '\0'); // 4个字节+字符串长度
        memcpy(data.data(), &biglen, 4);
        memcpy(data.data() + 4, msg.data(), msg.size());
        return writen(fd, data);
    }

    static int recv_msg(int cfd, string &msg)
    {
        // 读消息头，获取消息长度
        uint32_t len = 0;
        if (readn(cfd, 4, (char *)&len) != 4)
            return -1;
        len = ntohl(len);
        // 根据消息长度分配缓冲区并读取消息体
        vector<char> buf(len);
        int ret = readn(cfd, len, buf.data());
        if (ret != len)
            return -1;
        if (ret == 0)
            cout << "客户端关闭连接" << endl;
        msg.assign(buf.begin(), buf.end());
        return ret;
    }
};

#endif
static void err_(const char *n)
{
    perror(n);
    exit(1);
}