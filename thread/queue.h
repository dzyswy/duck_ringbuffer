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
class SafeQueue
{
public:
    SafeQueue() : deep_(-1) {}
    SafeQueue(int deep) : deep_(deep) {

    }
 

    void push(T data) {

        std::unique_lock<std::mutex> lock(mutex_);

        while(((deep_ > 0) && (list_.size() >= deep_))) {
            LOG(INFO) << "queue is full, wait an available position...";
            full_cond_.wait(lock);
        }
        list_.push_back(data);
        empty_cond_.notify_one(); 

    }

    T pop() {

        std::unique_lock<std::mutex> lock(mutex_);

        while(list_.empty()) {
            LOG(INFO) << "queue is empty, wait an available data...";
            empty_cond_.wait(lock);
        }
        T data = list_.front();
        list_.pop_front();
        full_cond_.notify_one(); 

        return data;
    }

    bool empty() {
        std::unique_lock<std::mutex> lock(mutex_);
        return list_.empty();
    }

    bool full() {
        std::unique_lock<std::mutex> lock(mutex_);
        bool is_full = ((deep_ > 0) && (list_.size() >= deep_)) ? true : false;
        return is_full;
    }

    size_t count() {
        std::unique_lock<std::mutex> lock(mutex_);
        return list_.size();
    }

    int deep() {
        std::unique_lock<std::mutex> lock(mutex_);
        return deep_;
    }


protected:
    int deep_;
    std::list<T > list_;
    std::mutex mutex_;
    std::condition_variable full_cond_;
    std::condition_variable empty_cond_;
};


}//namespace thread
}//namespace duck









