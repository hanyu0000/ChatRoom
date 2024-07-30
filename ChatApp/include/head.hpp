#include <iostream>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <cstdio>
#include <limits>
#include <unistd.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <json.hpp>
#include <hiredis/hiredis.h>

using json = nlohmann::json;
using namespace std;

static void err_(const char *n)
{
    perror(n);
    exit(1);
}