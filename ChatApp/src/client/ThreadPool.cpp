#include "head.hpp"
#include "ThreadPool.hpp"

ThreadPool::ThreadPool(int min, int max) : _maxThreads(max), _minThreads(min), _stop(false), _exitNumber(0)
{
    _idleThreads = _curThreads = min;
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
            t.join();
    }
    if (_manager->joinable())
        _manager->join();
}
// 将任务添加到线程池中，并通知一个等待线程执行该任务
void ThreadPool::addTask(function<void()> f)
{
    {
        lock_guard<mutex> locker(_queueMutex);
        _tasks.emplace(f); // 在队列中原地构造任务对象
    }
    // 通知一个等待该条件变量的线程，表明队列中有新的任务可以处理
    _condition.notify_one();
}

void ThreadPool::manager()
{
    while (!_stop.load())
    {
        // sleep_for 是 std::this_thread 命名空间中的一个静态成员函数，用于使当前线程休眠指定的时间段
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
                    (*i).second.join();
                    _workers.erase(i);
                }
            }
            _ids.clear();
        }
        else if (idle == 0 && current < _maxThreads)
        {
            thread t(&ThreadPool::worker, this);
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
                _exitNumber--;
                _curThreads--;
                unique_lock<mutex> lck(_idsMutex);
                _ids.emplace_back(this_thread::get_id());
                return;
            }
        }
        if (!_tasks.empty())
        {
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