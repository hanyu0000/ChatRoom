#include <iostream>
#include <cstring> 
#include <fstream>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <hiredis/hiredis.h>
#include <json.hpp>
using json = nlohmann::json;
#define BUFFER_SIZE 50
using namespace std;

struct UserInfo {
    string name;
    string pwd;
};
static void err_(const char *n)
{
    if (n)
        perror(n);
    else
        perror("Error\n");
    exit(1);
}