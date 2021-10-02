#pragma once

#include <sys/epoll.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <functional>
#include <vector>
#include <iostream>
#include <algorithm>
#include <thread>

// https://cloud.tencent.com/developer/article/1763594
// timer解析

class Timer
{
public:
    Timer(int rotations, int slot, std::function<void(void)> func)
        : rotations_(rotations), slot_(slot), func_(func) { }
    inline int GetRotations() { return rotations_; }
    inline void DecreaseRotations() { --rotations_; }
    inline void Active() {
        // std::thread worker(func_);
        // worker.join();
        // 如果要额外线程，还是要用户自己来写，定时器不管
        func_();
    }
    inline int GetSlot() { return slot_; }
private:
    int rotations_;
    int slot_;
    std::function<void(void)> func_;
    void* args_;
};

// 时间轮定时器的实现
class TimeWheel
{
public:
    TimeWheel(int nslots)
        : nslosts_(nslots), curslot_(0),
          slots_(nslosts_, std::vector<Timer *>()), starttime_(GetCurrentMillisecs())
    {}

    ~TimeWheel()
    {
        for (std::vector<Timer *> vect : slots_)
            { for (Timer *timer : vect) {delete timer;}}
    }

    unsigned long long GetCurrentMillisecs()
    {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
        return ts.tv_sec * 1000 + ts.tv_nsec / (1000 * 1000);
    }

    // 根据 timeout 来判断timer应该存放在哪个轮的哪个slot上
    Timer *AddTimer(unsigned long long timeout, std::function<void(void)>fun)
    {
        int slot = 0;
        Timer *timer = NULL;

        if (timeout < 0) {return NULL;}

        slot = (curslot_ + (timeout % nslosts_)) % nslosts_;

        timer = new Timer(timeout / nslosts_, slot, fun);
        slots_[slot].push_back(timer);
        return timer;
    }

    void DelTimer(Timer *timer)
    {
        if (!timer) {return;}

        //delete timer;
        std::vector<Timer *>::iterator it = std::find(slots_[timer->GetSlot()].begin(), slots_[timer->GetSlot()].end(), timer);
        if (it != slots_[timer->GetSlot()].end())
        { slots_[timer->GetSlot()].erase(it);}
    }

    void Tick()
    {
        for (auto it = slots_[curslot_].begin(); it != slots_[curslot_].end();)
        {
            if ((*it)->GetRotations() > 0)
            {
                (*it)->DecreaseRotations();
                ++it;
                continue;
            }
            else
            {
                Timer *timer = *it;
                timer->Active();  // 触发timer，执行注册的函数
                it = slots_[curslot_].erase(it);
                delete timer;
            }
        }

        int tmp = ++curslot_;
        curslot_ = tmp % nslosts_;
    }

    // 认为epoll存在误差，自己测试了一下也确实存在误差
    void TakeAllTimeout()
    {
        int now = GetCurrentMillisecs();
        int cnt = now - starttime_;
        for (int i = 0; i < cnt; ++i) {Tick();}
        starttime_ = now;
    }
/**
 * @brief 只有一个时间轮，轮子上有若干的slots
 */
private:
    int nslosts_;  // slot总数
    int curslot_;  // 当前slot，这个会根据时钟的tick一直移动
    unsigned long long starttime_;
    std::vector<std::vector<Timer *>> slots_;
};