#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "thread/thread.h" 
#include "thread/queue.h"
#include "thread/ringbuffer.h"

namespace duck {
namespace thread {


class PipeStamp
{
public:
    PipeStamp(const std::string& name, size_t pipe_data_id) : thread_name_(name), pipe_data_id_(pipe_data_id) {}

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

    std::string name() {
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

 

//流水线节点，interface
class PipeNode : public Thread
{
public:
    PipeNode(const std::string& node_name) : Thread(node_name), pre_node_(this), level_(0), running_(false) {}
    virtual bool is_broadcast() {return false;}  
    virtual bool is_slave() {return false;}
    virtual PipeNode* append(PipeNode* node) = 0;
    virtual void put(std::shared_ptr<PipeData> pipe_data) = 0;
    virtual std::shared_ptr<PipeData> get() = 0;
    virtual std::shared_ptr<PipeData> get_async() {return get();}

    virtual void show() = 0;
    virtual void collect(std::vector<PipeNode*>& node_vec) = 0;

    virtual void set_pre_node(PipeNode* node) {
        pre_node_ = node;
    }
    virtual void set_level(int value) {level_ = value;}
    virtual void inc_level() = 0;

    PipeNode* pre_node() {
        return pre_node_;
    }

    int level() {
        return level_;
    }

    bool running() {
        return running_;
    }
 
protected:
    PipeNode* pre_node_;
    int level_; 
    bool running_;
};

//链条的一环，点对点
class ChainNode : public PipeNode
{
public:
    ChainNode(const std::string& node_name) : PipeNode(node_name), next_node_(nullptr) {}

    virtual PipeNode* append(PipeNode* node) {
        next_node_ = node;
        node->set_level(level()); 
        node->set_pre_node(this);
        return node;
    }

    virtual void inc_level() {
        level_++;
        if (next_node_) {
            next_node_->set_level(level());
        }
    }

    virtual void collect(std::vector<PipeNode*>& node_vec) {
        node_vec.push_back(this);
        if (next_node_) {
            next_node_->collect(node_vec);
        }
    }

    virtual void show() { 
        std::stringstream ss;
        for (int i = 0; i < level(); i++) {
            ss << "\t";
        }
        std::cout << ss.str();
        std::cout << "|── " << name() << ", pre=" << pre_node()->name() << std::endl; 
        if (next_node_) {
            next_node_->show();
        } else {
            std::cout << ss.str();
            std::cout << "------------------------------" << std::endl;
        }
    }

    PipeNode* next_node() {
        return next_node_;
    }


protected: 
    PipeNode* next_node_;
};

//广播，1对多
class BroadcastNode : public PipeNode
{
public:
    BroadcastNode(const std::string& node_name, int buff_num = 4) : PipeNode(node_name), buff_(buff_num) {}

    virtual bool is_broadcast() {return true;}

    virtual void process() {
        running_ = true;
        while(true)
        {
            
            std::shared_ptr<PipeData> pipe_data = get();

            PipeStamp pipe_stamp(name(), pipe_data->pipe_data_id());
            pipe_stamp.record_now();
 


            pipe_stamp.record_now();
            pipe_data->push_stamp(pipe_stamp);

            for (const auto node : next_node_list_) {
                if (!node->is_slave()) {
                    node->put(pipe_data);
                } 
            }

            if (pipe_data->quit()) {
                break;
            }


        }
        running_ = false;
    }

    virtual void put(std::shared_ptr<PipeData> pipe_data) {
        buff_.put(pipe_data);
    }

    virtual std::shared_ptr<PipeData> get() {
        return buff_.get_sync();
    }

    virtual std::shared_ptr<PipeData> get_async() {
        return buff_.get_async();
    }

    virtual PipeNode* append(PipeNode* node) {
        node->set_pre_node(this);
        node->set_level(level()); 
        node->inc_level();
        next_node_list_.push_back(node); 
        return node;
    }


    virtual void inc_level() {
        level_++;
        for (const auto node : next_node_list_) {
            node->inc_level();
        }
    }

    virtual void collect(std::vector<PipeNode*>& node_vec) {
        node_vec.push_back(this);
        for (const auto node : next_node_list_) { 
            node->collect(node_vec);
        }
    }


    void show() {  
        std::stringstream ss;
        for (int i = 0; i < level(); i++) {
            ss << "\t";
        }
        std::cout << ss.str();
        std::cout << "#── " << name() << ", pre=" << pre_node()->name() << std::endl;  
        if (!next_node_list_.empty()) {
            for (const auto node : next_node_list_) { 
                node->show();
            }
        } else {
            std::cout << ss.str();
            std::cout << "------------------------------" << std::endl;
        }
        
    }

protected:
    std::list<PipeNode*> next_node_list_;
    RingBuffer<std::shared_ptr<PipeData> > buff_;
};



//从自己的fifo中取出一个PipeData，处理后，交给下一个节点。
class FilterNode : public ChainNode
{
public:
    FilterNode(const std::string& node_name, int queue_num) : ChainNode(node_name), fifo_(queue_num) {}

    virtual void process() {

        running_ = true;
        while(true)
        {
            std::shared_ptr<PipeData> pipe_data = get();

            PipeStamp pipe_stamp(name(), pipe_data->pipe_data_id());
            pipe_stamp.record_now(); 
 

            LOG(INFO) << name() << " thread process: " << pipe_data->pipe_data_id() << " data, is_quit: " << (pipe_data->quit() ? "true" : "false");
            
            if (!pipe_data->quit()) {
                compute(pipe_data); 
            }
            
            pipe_stamp.record_now();
            pipe_data->push_stamp(pipe_stamp);
            if (next_node()) {
                next_node()->put(pipe_data);
            }

            if (pipe_data->quit()) {
                break;
            } 
        }
        running_ = false;
    }

    virtual void compute(std::shared_ptr<PipeData> pipe_data) = 0;

    virtual void put(std::shared_ptr<PipeData> pipe_data) {
        fifo_.push(pipe_data);
    }

    std::shared_ptr<PipeData> get() {
        return fifo_.pop();
    }

protected:
    SafeQueue<std::shared_ptr<PipeData> > fifo_;
};


//从节点，异步的从上个节点获取PipeData，交给下一个节点。
class SlaveRateNode : public ChainNode
{
public:
    SlaveRateNode(const std::string& node_name, float fps) 
        : ChainNode(node_name), fps_(fps) {
            
        period_us_ = (fps > 0) ? 1000000.0 / fps : 1.0;
    }

    virtual bool is_slave() {return true;}

    virtual void process() {

        running_ = true;
        while(true) { 
            auto t0 = std::chrono::high_resolution_clock::now();

            std::shared_ptr<PipeData> pipe_data = pre_node()->get_async();

            PipeStamp pipe_stamp(name(), pipe_data->pipe_data_id());
            pipe_stamp.record_now(); 
 

            pipe_stamp.record_now();
            pipe_data->push_stamp(pipe_stamp);

            if (next_node()) {
                next_node()->put(pipe_data);
            }

            if (pipe_data->quit()) {
                break;
            }
    
            auto t1 = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
            long diff_us = period_us_ - duration.count();
            if ((fps_ > 0) && ( diff_us > 0)) {
                std::chrono::microseconds us(diff_us);
                std::this_thread::sleep_for(us);
            }
            
        }
        running_ = false;
    }

    virtual void put(std::shared_ptr<PipeData> pipe_data) {
        LOG(FATAL) << "RootNode have no put method!";
    }

    virtual std::shared_ptr<PipeData> get() {
        LOG(FATAL) << "RootNode have no get method!";
    }



protected:
    float fps_;
    float period_us_;
};



//产生PipeData，并传递给下一个节点
class RootNode : public ChainNode
{
public:
    RootNode(const std::string& node_name) 
        : ChainNode(node_name), frame_count_(0), quit_(false) {
             
    }

    virtual void init() {
        collect(node_vec_);
        for (size_t i = 0; i < node_vec_.size(); i++) {
            std::cout << node_vec_[i]->name() << std::endl;
        }
    }

    virtual void process() {

        running_ = true;
        while(true)
        { 
            std::shared_ptr<PipeData> pipe_data = std::make_shared<PipeData>(frame_count_, quit_); 

            PipeStamp pipe_stamp(name(), pipe_data->pipe_data_id());
            pipe_stamp.record_now(); 
 

            pipe_stamp.record_now();
            pipe_data->push_stamp(pipe_stamp);

            if (next_node()) {
                next_node()->put(pipe_data);
            }
            
            if (pipe_data->quit()) {
                break;
            }

            frame_count_++;
 

        }
        running_ = false;
    }

    void run() {
        for (size_t i = 1; i < node_vec_.size(); i++) {
            node_vec_[i]->start();
        }
        for (size_t i = 1; i < node_vec_.size(); i++) {
            while(!node_vec_[i]->running());
        }
        start();
    }

    void stop() {
        quit_ = true;
        for (size_t i = 0; i < node_vec_.size(); i++) {
            node_vec_[i]->join();
        } 
    }

    virtual void put(std::shared_ptr<PipeData> pipe_data) {
        LOG(FATAL) << "RootNode have no put method!";
    }

    virtual std::shared_ptr<PipeData> get() {
        LOG(FATAL) << "RootNode have no get method!";
    }

protected: 
    size_t frame_count_;
    bool quit_; 

    std::vector<PipeNode*> node_vec_;
};


}//namespace thread
}//namespace duck


