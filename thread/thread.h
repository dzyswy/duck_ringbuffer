#pragma once

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <glog/logging.h>

namespace duck {
namespace thread {




class Thread
{
public:
    Thread(const std::string& thread_name) : thread_name_(thread_name) {}

    virtual void process() = 0;

    void start() {
        thread_ = std::thread(Thread::thread_handle, this); 
        //thread_.detach();
    }

    void join() {
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    std::string name() {
        return thread_name_;
    }

protected:
    static void thread_handle(Thread* thread) {
        LOG(INFO) << "thread is running!";
        thread->process();
        LOG(INFO) << "thread is quit!";
    }

protected:
    std::string thread_name_;
    std::thread thread_;
};




}//namespace thread
}//namespace duck


