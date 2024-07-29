#include <iostream>
#include <map>
#include <future>
#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <atomic>
#include <memory>
#include <sys/socket.h>
#include <functional>
#include <condition_variable>
using namespace std;

// 线程池类
class ThreadPool
{
public:
    // 构造函数：初始化线程池，默认最小线程数为4，最大线程数为系统硬件并发数
    ThreadPool(int min = 4, int max = thread::hardware_concurrency());
    ~ThreadPool(); // 析构函数：清理资源
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

ThreadPool::ThreadPool(int min, int max) : _maxThreads(max), _minThreads(min), _stop(false), _exitNumber(0)
{
    _idleThreads = _curThreads = min;
    cout << "线程数量" << _curThreads << endl;
    _manager = make_unique<thread>(&ThreadPool::manager, this);
    for (int i = 0; i < _curThreads; ++i)
    {
        thread t(&ThreadPool::worker, this);
        _workers.insert(make_pair(t.get_id(), move(t)));
    }
}

ThreadPool::~ThreadPool()
{
    _stop = true;
    _condition.notify_all();
    for (auto &i : _workers)
    {
        thread &t = i.second;
        if (t.joinable())
        {
            cout << "线程---" << t.get_id() << "将要退出了..." << endl;
            t.join();
        }
    }
    if (_manager->joinable())
        _manager->join();
}
// 将任务添加到线程池中，并通知一个等待线程执行该任务
void ThreadPool::addTask(function<void()> f)
{
    {
        lock_guard<mutex> locker(_queueMutex);
        // 在队列中原地构造任务对象
        _tasks.emplace(f);
    }
    // 通知一个等待该条件变量的线程，表明队列中有新的任务可以处理
    _condition.notify_one();
}

void ThreadPool::manager()
{
    while (!_stop.load())
    {
        // sleep_for 是 std::this_thread 命名空间中的一个静态成员函数，用于使当前线程休眠指定的时间段
        // chrono 是 C++11 标准库中的一个命名空间，提供了时间处理的功能。
        // chrono::seconds 是一个时间单位类，用于表示秒数。
        // chrono::seconds(2) 创建了一个表示 2 秒钟的时间段对象。
        this_thread::sleep_for(chrono::seconds(2));
        // load() 是 std::atomic 类模板提供的方法，用于安全地读取原子变量的当前值
        // 这个方法会返回变量的值，并且在读取时保证线程安全，避免数据竞争
        int idle = _idleThreads.load();
        int current = _curThreads.load();
        if (idle > current / 2 && current > _minThreads)
        {
            // store(2) 将值 2 存储到 _exitNumber 中
            _exitNumber.store(2);
            _condition.notify_all();
            // unique_lock<mutex>:unique_lock 是 C++ 标准库中提供的一种锁类，它可以管理一个互斥量的锁状态
            // unique_lock 是一个更灵活的锁封装，它支持更多的功能，例如延迟锁定、条件变量的使用、锁的移动等
            // mutex 是一个互斥量类，unique_lock 可以用于管理这种类型的互斥量
            // lck(_idsMutex):lck 是一个 unique_lock<mutex> 类型的对象
            //_idsMutex 是一个 std::mutex 类型的互斥量，unique_lock 对象 lck 会在创建时锁定这个互斥量

            // 加锁: unique_lock 对象 lck 在构造时会锁定 _idsMutex，
            // 确保在 lck 的作用域内，只有当前线程能够访问被 _idsMutex 保护的共享资源
            // 自动解锁: 当 lck 对象的作用域结束时（即 lck 被销毁），unique_lock 会自动解锁 _idsMutex，防止死锁情况发生
            unique_lock<mutex> lck(_idsMutex);
            for (const auto &id : _ids)
            {
                auto i = _workers.find(id);
                if (i != _workers.end())
                {
                    cout << "线程" << (*i).first << "被销毁了......" << endl;
                    (*i).second.join();
                    _workers.erase(i);
                }
            }
            _ids.clear();
        }
        else if (idle == 0 && current < _maxThreads)
        {
            thread t(&ThreadPool::worker, this);
            cout << "添加了一个线程,id:" << t.get_id() << endl;
            _workers.insert(make_pair(t.get_id(), move(t)));
            _curThreads++;
            _idleThreads++;
        }
    }
}

void ThreadPool::worker()
{
    unique_lock<mutex> locker(_queueMutex);
    while (!_stop.load())
    {
        while (!_stop && _tasks.empty())
        {
            _condition.wait(locker);
            if (_exitNumber.load() > 0)
            {
                cout << "线程任务结束,id:" << this_thread::get_id() << endl;
                _exitNumber--;
                _curThreads--;
                unique_lock<mutex> lck(_idsMutex);
                _ids.emplace_back(this_thread::get_id());
                return;
            }
        }
        if (!_tasks.empty())
        {
            cout << "取出一个任务..." << endl;
            auto task = move(_tasks.front());
            _tasks.pop();
            _idleThreads--;
            locker.unlock();
            task();
            locker.lock();
            _idleThreads++;
        }
    }
}

void calc(int x, int y)
{
    this_thread::sleep_for(chrono::seconds(2));
}

int thread_pool(char *buf)
{
    ThreadPool pool(4);
    for (int i = 0; i < 10; ++i)
    {
        auto func = bind(calc, i, i * 2);
        pool.addTask(func);
    }
    getchar();
    return 0;
}