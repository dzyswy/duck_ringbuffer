#pragma once

#include <string>
#include <iostream>
#include <iomanip> 
#include <map>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <functional>
#include <glog/logging.h>


namespace duck {
namespace timer {


class Timer
{
public:
    Timer() : period_ms_(0), repeat_(0) {}
    Timer(int64_t period_ms, int repeat, std::function<void()> func) : period_ms_(period_ms), repeat_(repeat), func_(func) {}

    int64_t now_ms() {
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
        int64_t ms = duration.count();
        return ms;
    }

    int64_t process() {
 
        int64_t now = now_ms();
        if (repeat_ != 0) {
            func_();
        }
        if (repeat_ > 0) {
            repeat_--;
        } 

        if (repeat_ == 0) {
            return -1;
        }

        now += period_ms_;
        return now;
    }

   

protected:
    int64_t period_ms_;
    int repeat_;
    std::function<void()> func_;
};




class TimerManager
{
public:
    TimerManager(int tick_ms = 5) : tick_ms_(tick_ms), quit_(true) {}

    int64_t now_ms() {
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
        int64_t ms = duration.count();
        return ms;
    }

    
    template<typename F, typename... Args>
    void submit(int64_t period_ms, int repeat, F && func, Args&&... args) {

        std::unique_lock<std::mutex> lock(mutex_);

        auto ff = std::bind(std::forward<F>(func), std::forward<Args>(args)...);
        
        Timer timer(period_ms, repeat, ff);
        int64_t now = now_ms(); 
        timer_map_.insert(std::make_pair(now, timer));
    }

    template<typename F, typename... Args>
    void submit(int64_t period_ms, F && func, Args&&... args) {
        
        submit(period_ms, -1, std::forward<F>(func), std::forward<Args>(args)...); 
    }

    void process() {
        
        while(!quit_)
        {
            std::chrono::milliseconds ms(tick_ms_);
            std::this_thread::sleep_for(ms);

            update();
        }
    }

    void update()
    {
        while(!quit_)
        {
            Timer timer;
            int ret = pop(&timer);
            if (ret < 0) {
                return;
            }

            int64_t next_time = timer.process();
            if (next_time > 0) {
                push(next_time, timer);
            }
        }
    }


    int pop(Timer* timer) {
        std::unique_lock<std::mutex> lock(mutex_);

        if (timer_map_.empty()) {
            return -1;
        }

        int64_t now = now_ms();

        auto it = timer_map_.begin();
        if (it->first > now) {
            return -1;
        }
  
        *timer = it->second;
        timer_map_.erase(it);

        return 0;
    }

    void push(int64_t next_time, Timer timer) {

        std::unique_lock<std::mutex> lock(mutex_);

        timer_map_.insert(std::make_pair(next_time, timer));

    }


    

    void start() {
        quit_ = false;
        thread_ = std::thread(TimerManager::thread_handle, this);
    }

    void stop() {
        quit_ = true;
        if (thread_.joinable()) {
            thread_.join();
        }
    }

public:
    static void thread_handle(TimerManager* manager) {
        manager->process();
    }

protected:
    int tick_ms_;
    std::multimap<int64_t, Timer> timer_map_;

    
    std::thread thread_;
    bool quit_;
    std::mutex mutex_;
};


}//namespace timer
}//namespace duck















