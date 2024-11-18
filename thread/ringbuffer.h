#pragma once

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <list>
#include <vector>
#include <memory>
#include <glog/logging.h>


namespace duck {
namespace thread {


template<typename T>
class RingBuffer
{
public:
    RingBuffer(size_t deep, const std::string& buff_name = std::string()) : deep_(deep), wptr_(0), buff_name_(buff_name) {}

    void put(T value) {
        std::unique_lock<std::mutex> lock(mutex_);

        if (buff_.size() < deep_) {
            buff_.push_back(value);
        } else {
            buff_[wptr_ % deep_] = value;
        }
        
        cond_.notify_all();
        wptr_++;
    }

    T get_async() {
        std::unique_lock<std::mutex> lock(mutex_);

        while(buff_.empty())
        {
            LOG(INFO) << name() << " queue is empty, wait an available data...";
            cond_.wait(lock);
        }

        return buff_[(wptr_ - 1) % deep_];
    }

    T get_sync() {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock);
        return buff_[(wptr_ - 1) % deep_];
    }

    std::string name() {
        return buff_name_;
    }


protected:
    size_t deep_;
    size_t wptr_;
    std::string buff_name_;
    std::vector<T> buff_;
    std::condition_variable cond_;
    std::mutex mutex_;

};

 



}//namespace thread
}//namespace duck











