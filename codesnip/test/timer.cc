#include <condition_variable>
#include <mutex>
#include <chrono>
#include <future>
#include "common/timer.h"
// https://suchprogramming.com/epoll-in-3-easy-steps/

#define LOCAL_MAX_EVENTS 5
#define TIME_OUT 1

TimeWheel* gtw;
std::mutex mu_;
std::condition_variable cond_;

unsigned long long __GetCurrentMillisecs()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / (1000 * 1000);
}

void 
Worker(TimeWheel* tw)
{
    // int event_count;
    struct epoll_event event, events[LOCAL_MAX_EVENTS];

    int epoll_fd = epoll_create1(0);    
    if(epoll_fd == -1){
        perror("Failed to create epoll file descriptor\n");
        exit(EXIT_FAILURE);
    }
    event.events = EPOLLIN;
    event.data.fd = 0;

    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, 0, &event))
    {
        perror("Failed to add file descriptor to epoll\n");
        close(epoll_fd);
        exit(EXIT_FAILURE);
    }
 
    for(;;)
    {
        // // epoll的超时时间是ms
        // event_count = epoll_wait(
        //     epoll_fd, events, LOCAL_MAX_EVENTS, TIME_OUT);
        // // std::printf("time is %lld\n", __GetCurrentMillisecs());
        epoll_wait(
            epoll_fd, events, LOCAL_MAX_EVENTS, TIME_OUT);
        tw->TakeAllTimeout();
    }

    if(close(epoll_fd))
    {
        perror("Failed to close epoll file descriptor\n");
        exit(EXIT_FAILURE);
    }

    return;
}

int main()
{
    TimeWheel tw(60*1000);
    gtw = &tw;
    std::thread worker(Worker, gtw);

    // 单位是ms
    gtw->AddTimer(
        2000, []() { std::cout << "hello world" << std::endl; });
    gtw->AddTimer(
        5000, []() { std::cout << "hello double_D" << std::endl; });
    std::printf("main thread go on...\n");

    std::unique_lock<std::mutex> lock_(mu_);
    while(true){
        if(cond_.wait_for(lock_, std::chrono::milliseconds(5000))==std::cv_status::timeout){
            // 是否会被自己唤醒呢
            std::printf("wake again because of timeout\n");
        }   
    }
    while(true);
    worker.join();
}