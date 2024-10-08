#include "ThreadPool.hpp"
#include <iostream>

ThreadPool::ThreadPool(int min, int max) : maxThreads(max),
                                           minThreads(min), stop(false), exitNumber(0)
{
    // idleThreads = curThreads = max / 2;
    idleThreads = curThreads = min;
    manager = new thread(&ThreadPool::manager, this); // 创建管理线程
    for (int i = 0; i < curThreads; ++i)
    {
        thread t(&ThreadPool::worker, this);// 创建工作线程
        t.detach();
        workers.insert(make_pair(t.get_id(), move(t))); // 将工作线程放进线程池
    }
}

ThreadPool::~ThreadPool()
{
    stop = true; // 释放资源
    condition.notify_all();
    for (auto &it : workers)
    {
        thread &t = it.second;
        if (t.joinable())
            t.join();
    }
    if (manager->joinable())
        manager->join();
    delete manager;
}

void ThreadPool::addTask(function<void()> f)
{
    {
        lock_guard<mutex> locker(queueMutex);
        tasks.emplace(f); // 添加任务到任务队列push
        // 调用 emplace 时，它会在容器的内存空间中直接构造对象
        // 你传递给 emplace 的参数会被直接传递给对象的构造函数
    }
    condition.notify_one(); // 通知线程有新任务
}

void ThreadPool::my_manager()
{
    while (!stop.load()) // 以原子方式读取其当前值
    {
        this_thread::sleep_for(chrono::seconds(2)); // 每2秒检查
        int idle = idleThreads.load();
        int current = curThreads.load();
        if (idle > current / 2 && current > minThreads) 
        {
            exitNumber.store(2);
            condition.notify_all();
            unique_lock<mutex> lck(idsMutex);
            for (const auto &id : ids)
            {
                auto it = workers.find(id);
                if (it != workers.end())
                {
                    (*it).second.join();
                    workers.erase(it);
                }
            }
            ids.clear();
        }
        else if (idle == 0 && current < maxThreads) //+
        {
            thread t(&ThreadPool::worker, this);
            t.detach();
            workers.insert(make_pair(t.get_id(), move(t)));
            curThreads++;
            idleThreads++;
        }
    }
}

void ThreadPool::worker()
{
    while (!stop.load())
    {
        function<void()> task = nullptr;
        {
            unique_lock<mutex> locker(queueMutex);
            while (!stop && tasks.empty())
            {
                condition.wait(locker); // 线程进入等待状态，由condition.notify_one()或condition.notify_all()触发
                if (exitNumber.load() > 0)
                {
                    exitNumber--;
                    curThreads--;
                    unique_lock<mutex> lck(idsMutex);
                    ids.emplace_back(this_thread::get_id());
                    return;
                }
            }

            if (!tasks.empty()) // 检查任务是否为空，不为空从队列取出一个任务执行
            {
                task = move(tasks.front());
                tasks.pop();
            }
        }

        if (task)
        {
            idleThreads--;
            task(); // 执行任务
            idleThreads++;
        }
    }
}