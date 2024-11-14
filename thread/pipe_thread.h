#pragma once

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "thread/thread.h" 
#include "thread/queue.h"

namespace duck {
namespace thread {


class PipeStamp
{
public:
    PipeStamp(const std::string& thread_name, size_t pipe_data_id) : thread_name_(thread_name), pipe_data_id_(pipe_data_id) {}

    void record_now() {
        auto now = std::chrono::high_resolution_clock::now(); 
        time_vec_.push_back(now);
    }


    double time_ms(size_t id) {
        CHECK(id < time_vec_.size()) << "PipeStamp don't record this id time!";
        auto now = time_vec_[id].time_since_epoch();
        auto time = std::chrono::duration_cast<std::chrono::microseconds>(now);
        double ms = (double)time.count() / 1000.0;
        return ms;
    }

    double start_ms() {
        return time_ms(0);
    }
    
    double end_ms() {
        return time_ms(1);
    }

    double duration_ms(size_t start_id = 0, size_t end_id = 1) {
 
        double start = time_ms(start_id);
        double end = time_ms(end_id);
        return (end - start);
    }

    std::string thread_name() {
        return thread_name_;
    }

    size_t pipe_data_id() {
        return pipe_data_id_;
    }

protected:
    std::string thread_name_;
    size_t pipe_data_id_;
    std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>> time_vec_;
};

class PipeData
{
public:
    PipeData(size_t pipe_data_id = 0, bool quit = false) :  pipe_data_id_(pipe_data_id), quit_(quit) {}

    size_t pipe_data_id() {
        return pipe_data_id_;
    }

    void push_stamp(PipeStamp stamp) {
        pipe_stamp_vec_.push_back(stamp);
    }

    std::vector<PipeStamp >& pipe_stamp_vec() {
        return pipe_stamp_vec_;
    }

    bool quit() {
        return quit_;
    }

    double latency_ms() {
        if (pipe_stamp_vec_.empty()) {
            return 0;
        }

        double start = pipe_stamp_vec_.front().start_ms();
        double end = pipe_stamp_vec_.back().end_ms();
        return (end - start);
    }

protected:
    size_t pipe_data_id_;
    std::vector<PipeStamp > pipe_stamp_vec_;
    bool quit_;
};



class PipeDemuxNode : public PipeNode
{
public:

protected:
    std::list<PipeNode*> child_list_;
};


// class DemuxThread : public Thread
// {
// public:

// protected:
//     RingBuffer<std::shared_ptr<PipeData>>
// };

class PipeThread : public Thread
{
public:
    PipeThread(const std::string& thread_name, int queue_num = 4) 
        : Thread(thread_name), fifo_(4), next_thread_(nullptr) {}

    void set_next_thread(PipeThread* thread) {
        next_thread_ = thread;
    }
 
    

    PipeThread* next_thread() {
        return next_thread_;
    }

    void push_data(std::shared_ptr<PipeData> ) {
        fifo_.push();
    }

    SafeQueue<std::shared_ptr<PipeData> >& fifo() {
        return fifo_;
    }

protected: 
    PipeThread* next_thread_; 
    SafeQueue<std::shared_ptr<PipeData> > fifo_;

};


class FilterThread : public PipeThread
{
public:
    FilterThread(const std::string& thread_name, int queue_num = 4) : PipeThread(thread_name, queue_num) {

    }

    void process() {

        while(true)
        {
            std::shared_ptr<PipeData> pipe_data = fifo().pop();
            PipeStamp pipe_stamp(thread_name(), pipe_data->pipe_data_id());
            pipe_stamp.record_now(); 
            LOG(INFO) << thread_name() << " thread process: " << pipe_data->pipe_data_id() << " data, is_quit: " << (pipe_data->quit() ? "true" : "false");
            if (!pipe_data->quit()) {
                compute(pipe_data); 
            }
            
            pipe_stamp.record_now();
            pipe_data->push_stamp(pipe_stamp);
            if (next_thread()) {
                next_thread()->fifo().push(pipe_data);
            }

            if (pipe_data->quit()) {
                break;
            } 
        }

    }

    virtual void compute(std::shared_ptr<PipeData> pipe_data) = 0;
};





}//namespace thread
}//namespace duck


