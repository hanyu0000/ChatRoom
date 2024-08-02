#ifndef T_MAIN_H_
#define T_MAIN_H_

#include <iostream>
#include <map>
#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <atomic>
#include <memory>
#include <functional>
#include <condition_variable>

// 线程池类
class ThreadPool
{
public:
    // 构造函数：初始化线程池，默认最小线程数为4，最大线程数为系统硬件并发数
    ThreadPool(int min = 4, int max = thread::hardware_concurrency());
    ~ThreadPool(); // 析构函数
    void addTask(function<void()> f);

private:
    void manager();
    void worker();

private:
    unique_ptr<thread> _manager;      // 管理线程
    map<thread::id, thread> _workers; // 工作线程映射，键为线程ID，值为线程对象
    vector<thread::id> _ids;          // 存储线程ID的向量
    int _minThreads;                  // 最小线程数
    int _maxThreads;                  // 最大线程数
    atomic<bool> _stop;               // 标志线程池是否停止
    atomic<int> _curThreads;          // 当前线程数
    atomic<int> _idleThreads;         // 空闲线程数
    atomic<int> _exitNumber;          // 需要退出的线程数
    queue<function<void()>> _tasks;   // 任务队列
    // std::function<void()> 是一个可以存储、复制和调用任何可调用对象（如普通函数、Lambda 表达式、函数对象等）的类模板
    // 它的签名是 void()，表示它可以存储一个不接受任何参数并且没有返回值的函数
    mutex _idsMutex;               // 保护线程ID向量的互斥锁
    mutex _queueMutex;             // 保护任务队列的互斥锁
    condition_variable _condition; // 用于任务调度的条件变量
};

#endif