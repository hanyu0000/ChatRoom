#include <iostream>
#include <cstring> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <hiredis/hiredis.h>
#include <fstream>
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
static void handler(int n)
{
    wait(nullptr);
}