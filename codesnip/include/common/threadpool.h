// 线程池造轮子
#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

class ThreadPool
{
public:
    // the constructor just launches some amount of workers
    ThreadPool(size_t threads) : stop(false)
    {
        for(size_t i = 0;i<threads;++i) {
            // std::vector<std::thread> workers;
            workers.emplace_back(
                // lambda函数前面的方括号指定哪些变量被lambda捕获
                // 捕获意味着可以在lambda内部处理lambda外部的变量
                // 在类内部可以捕获this, 然后调用类方法
                [this]
                {
                    for(;;)
                    {
                        // std::function的实例可以对任何 可以调用的目标实体 进行存储、复制、和调用操作
                        // void() 函数返回类型void，无参数
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(this->queue_mutex);
                            // 这一句wait相当于
                            // while(!(this->stop || !this->tasks.empty())){
                            //     this->condition.wait(lock);
                            // }
                            // 梳理一下继续执行的条件，除非返回true，否则阻塞在这里
                            this->condition.wait(lock, [this]{ return this->stop || !this->tasks.empty(); });
                            // 重新获得所并执行到这里有两种情况，要么stop为true，要么tasks非空
                            if(this->stop && this->tasks.empty()) { return; }  // 保底
                            // 右值的转换，避免临时变量的多次无意义的拷贝
                            task = std::move(this->tasks.front());
                            this->tasks.pop();
                        }
                        task();
                    }
                }
            );
        }
    }

    // add new work item to the pool
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using return_type = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
            
        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            // don't allow enqueueing after stopping the pool
            if(stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");

            tasks.emplace([task](){ (*task)(); });
        }
        condition.notify_one();
        return res;
    }

    ThreadPool(const ThreadPool &) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;

    // the destructor joins all threads
    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for(std::thread &worker: workers)
            worker.join();
    }
private:
    // need to keep track of threads so we can join them
    std::vector<std::thread> workers;
    // the task queue
    std::queue<std::function<void()>> tasks;
    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};
#endif