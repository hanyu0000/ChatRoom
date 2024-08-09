#pragma once
#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <atomic>
#include <functional>
#include <condition_variable>
#include <map>
#include <future>
using namespace std;

// 线程池类
class ThreadPool
{
public:
    ThreadPool(int min = 4, int max = thread::hardware_concurrency());
    ~ThreadPool();
    void addTask(function<void()> f); // 传入一个函数类型的任务

private:
    void my_manager();
    void worker();

private:
    int maxThreads;                  // 最大线程数
    int minThreads;                  // 最小线程数
    atomic<bool> stop;               // 线程池是否停止的标志
    atomic<int> exitNumber;          // 需要退出的线程数
    atomic<int> idleThreads;         // 空闲线程数
    atomic<int> curThreads;          // 当前线程数
    thread *manager;                 // 管理线程
    map<thread::id, thread> workers; // 工作线程
    queue<function<void()>> tasks;   // 任务队列
    mutex queueMutex;                // 任务队列的互斥锁
    condition_variable condition;    // 用于任务调度的条件变量
    mutex idsMutex;                  // 保护ids的互斥锁
    vector<thread::id> ids;          // 待退出的线程ID
};