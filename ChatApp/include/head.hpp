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
#include <sys/uio.h>
#include <functional>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <json.hpp>
#include <hiredis/hiredis.h>
#include <condition_variable>
using json = nlohmann::json;
using namespace std;

static void err_(const char *n)
{
    perror(n);
    exit(1);
}

#endif 