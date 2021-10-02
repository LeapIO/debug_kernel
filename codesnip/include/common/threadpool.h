// // 线程池造轮子
// #pragma once

// #include <vector>
// #include <queue>
// #include <memory>
// #include <thread>
// #include <mutex>
// #include <condition_variable>
// #include <future>
// #include <functional>
// #include <stdexcept>

// /**
//  * @brief 
//  * emplace_back()与push_back()类似，但是前者更适合用来传递对象，因为它可以避免对象作为参数被传递时在拷贝成员上的开销
//  *
//  * 这里emplace_back()了一个lambda表达式[this]{…}
//  * lambda表达式本身代表一个匿名函数(即没有函数名的函数)，通常格式为
//  *      [捕获列表](参数列表)->return 返回类型{函数体}。而在本代码中的lambda表达式是作为一个线程放入workers[]中
//  * 这个线程体是一个for循环
//  */
// class ThreadPool 
// {
// public:
//     // the constructor just launches some amount of workers
//     explicit ThreadPool(size_t threads) : stop(false)
//     {
//         for(size_t i=0; i<threads; ++i) 
//         {
//             // 下面的内容已经是work线程中的内容了
//             workers.emplace_back([this](){
//                 // 反正大括号里的是函数体，已经是work线程的内容了
//                 while(true)
//                 {
//                     std::function<void()> task;  // task是封装过的函数对象
//                     // 创建了一个新的作用域
//                     {
//                         std::unique_lock<std::mutex> lock(queue_mutex);
//                         // 函数体返回true时，可以继续向下执行
//                         // 当线程池运行或者任务列表为空时，线程进入阻塞态并暂时释放锁
//                         // 条件跳转中有未初始化的变量
//                         // while(true){
//                         //     // 循环体条件true，则阻塞
//                         //     this->condition.wait(lock);
//                         // }
//                         // 满足条件之后继续获得锁
//                         // 后面的条件是false,则阻塞
//                         condition.wait(lock,
//                             [this](){ return stop || !tasks.empty();});
//                         if(stop && tasks.empty()) {return;}
//                         task = std::move(tasks.front());
//                         tasks.pop();
//                     }

//                     // 真正的执行任务，如果当前线程的任务队列有空，就可以执行
//                     // task(); 
//                 }
//             });
//         }
//     }

//     // add new work item to the pool
//     // 这是一个函数模板，标识可以接受多个参数
//     // F&& 表示的是右值引用，延长声明周期的
//     // 所以最后这个模板函数enqueue()的返回值类型就是F(Args…)的异步执行结果类型
//     template<class F, class... Args>
//     auto enqueue(F&& f, Args&&... args)
//         -> std::future<typename std::result_of<F(Args...)>::type>
//     {
//         using return_type = typename std::result_of<F(Args...)>::type;
//         // 创建 std::packaged_task<return_type()>, 其中task是指向它的指针
//         auto task = std::make_shared<std::packaged_task<return_type()>>(
//             std::bind(std::forward<F>(f), std::forward<Args>(args)...)
//         );

//         std::future<return_type> res = task->get_future();
//         {
//             std::unique_lock<std::mutex> lock(queue_mutex);
//             // don't allow enqueueing after stopping the pool
//             if(stop) {throw std::runtime_error("enqueue on stopped ThreadPool");}
//             tasks.emplace([task](){ (*task)(); });
//         }
//         // 唤醒线程池中的资源
//         condition.notify_one();
//         return res;
//     }

//     // the destructor joins all threads
//     ~ThreadPool()
//     {
//         {
//             std::unique_lock<std::mutex> lock(queue_mutex);
//             stop = true;
//         }
//         condition.notify_all();
//         for(std::thread &worker: workers) {worker.join();}
//     }

//     ThreadPool(const ThreadPool &) = delete;
//     ThreadPool &operator=(const ThreadPool &) = delete;
// private:
//     // need to keep track of threads so we can join them
//     std::vector<std::thread> workers;
//     // the task queue
//     std::queue<std::function<void()>> tasks;
//     // synchronization
//     std::mutex queue_mutex;
//     std::condition_variable condition;
//     bool stop;
// };

// 上面那个不行确实是自己环境的问题
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
//
#pragma once

#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>
#include <random>

// Thread safe implementation of a Queue using an std::queue
// 线程安全的队列，其实就是加了一个mutex
template <typename T>
class SafeQueue {
private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
public:
    SafeQueue() {}

    SafeQueue(SafeQueue& other) {
        //TODO:
    }

    ~SafeQueue() {}

    bool empty() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    int size() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    void enqueue(T& t) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(t);
    }
  
    bool dequeue(T& t) {
        std::unique_lock<std::mutex> lock(m_mutex);

        if (m_queue.empty()) { return false; }
        t = std::move(m_queue.front());
        
        m_queue.pop();
        return true;
    }
};

class ThreadPool {
private:
    class ThreadWorker {
    private:
        int m_id;
        ThreadPool *m_pool;
    public:
        ThreadWorker(ThreadPool * pool, const int id)
        : m_pool(pool), m_id(id) {}

        void operator()() {
            std::function<void()> func;
            bool dequeued;
            while (!m_pool->m_shutdown) {
                {
                    std::unique_lock<std::mutex> lock(m_pool->m_conditional_mutex);
                    if (m_pool->m_queue.empty()) {
                        m_pool->m_conditional_lock.wait(lock);
                    }
                    dequeued = m_pool->m_queue.dequeue(func);
                }
                if (dequeued) { func(); }
            }
        }
    };

    bool m_shutdown;
    SafeQueue<std::function<void()>> m_queue;
    std::vector<std::thread> m_threads;
    std::mutex m_conditional_mutex;
    std::condition_variable m_conditional_lock;
public:
    ThreadPool(const int n_threads)
        : m_threads(std::vector<std::thread>(n_threads)), m_shutdown(false) {
    }

    ThreadPool(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;

    ThreadPool & operator=(const ThreadPool &) = delete;
    ThreadPool & operator=(ThreadPool &&) = delete;

    // Inits thread pool
    void Init() {
        for (int i=0; i<static_cast<int>(m_threads.size()); ++i) {
            // 线程的构造函数啥也不干，线程创建后没有任务就等待
            m_threads[i] = std::thread(ThreadWorker(this, i));
        }
    }

    // Waits until threads finish their current task and shutdowns the pool
    void Shutdown() {
        m_shutdown = true;
        m_conditional_lock.notify_all();
        
        for (int i=0; i<static_cast<int>(m_threads.size()); ++i) {
            if(m_threads[i].joinable()) {m_threads[i].join();}
        }
    }

    // Submit a function to be executed asynchronously by the pool
    template<typename F, typename...Args>
    auto Submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> 
    {
        // Create a function with bounded parameters ready to execute
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        // Encapsulate it into a shared ptr in order to be able to copy construct / assign 
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

        // Wrap packaged task into void function
        std::function<void()> wrapper_func = [task_ptr]() { (*task_ptr)(); };

        // Enqueue generic wrapper function
        m_queue.enqueue(wrapper_func);

        // Wake up one thread if its waiting
        m_conditional_lock.notify_one();

        // Return future from promise
        return task_ptr->get_future();
    }
};