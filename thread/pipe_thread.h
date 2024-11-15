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

//流水线节点，interface
class PipeNode : public Thread
{
public:
    PipeNode(const std::string& node_name) : Thread(node_name), pre_node_(this), level_(0) {}
    virtual bool is_broadcast() {return false;}  
    virtual bool is_slave() {return false;}
    virtual void put(std::shared_ptr<PipeData> pipe_data) = 0;
    virtual std::shared_ptr<PipeData> get() = 0;
    virtual std::std::shared_ptr<PipeData> get_async() {return get();}

    virtual void show() = 0;

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

protected:
    PipeNode* pre_node_;
    int level_;

};

//链条的一环，点对点
class ChainNode : public PipeNode
{
public:
    ChainNode(const std::string& node_name) : PipeNode(node_name), next_node_(nullptr) {}

    void append(PipeNode* node) {
        next_node_ = node;
        node->set_level(level()); 
        node->set_pre_node(this);
    }

    virtual void inc_level() {
        level_++;
        if (next_node_) {
            next_node_->set_level(level());
        }
    }

    void show() { 
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

        while(true)
        {
            PipeStamp pipe_stamp(thread_name(), pipe_data->pipe_data_id());
            pipe_stamp.record_now();

            std::shared_ptr<PipeData> pipe_data = get();

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

    void add_next_node(PipeNode* node) {
        node->set_pre_node(this);
        node->set_level(level()); 
        node->inc_level();
        next_node_list_.push_back(node); 
    }

    virtual void inc_level() {
        level_++;
        for (const auto node : next_node_list_) {
            node->inc_level();
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

//产生PipeData，并传递给下一个节点
class MasterRateNode : public ChainNode
{
public:
    MasterRateNode(const std::string& node_name, float fps = -1) 
        : ChainNode(node_name), fps_(fps), frame_count_(0), quit_(false) {
            
            period_us_ = (fps > 0) ? 1000000.0 / fps : 1.0;
        }

    virtual void process() {

        timestamp_ = std::chrono::high_resolution_clock::now(); 
        while(true)
        {
            auto t0 = std::chrono::high_resolution_clock::now();

            PipeStamp pipe_stamp(thread_name(), pipe_data->pipe_data_id());
            pipe_stamp.record_now(); 

            std::shared_ptr<PipeData> pipe_data = std::make_shared<PipeData>(frame_count_, quit_); 

            pipe_stamp.record_now();
            pipe_data->push_stamp(pipe_stamp);

            if (next_node()) {
                next_node()->put(pipe_data);
            }
            
            if (pipe_data->quit()) {
                break;
            }

            frame_count_++;
            auto t1 = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
            float diff_us = period_us_ - duration.count();
            if ((fps_ > 0) && ( diff_us > 0)) {
                std::chrono::microseconds us(diff_us);
                std::this_thread::sleep_for(us);
            }

        }

    }

    virtual void put(std::shared_ptr<PipeData> pipe_data) {
        LOG(FATAL) << "MasterRateNode have no put method!";
    }

    virtual std::shared_ptr<PipeData> get() {
        LOG(FATAL) << "MasterRateNode have no get method!";
    }

protected:
    float fps_;
    float period_us_;
    size_t frame_count_;
    bool quit_; 
};

//从自己的fifo中取出一个PipeData，处理后，交给下一个节点。
class FilterNode : public ChainNode
{
public:
    FilterNode(const std::string& node_name, int queue_num) : ChainNode(node_name), fifo_(queue_num) {}

    virtual void process() {
        while(true)
        {
            std::shared_ptr<PipeData> pipe_data = get();

            PipeStamp pipe_stamp(thread_name(), pipe_data->pipe_data_id());
            pipe_stamp.record_now(); 

            LOG(INFO) << thread_name() << " thread process: " << pipe_data->pipe_data_id() << " data, is_quit: " << (pipe_data->quit() ? "true" : "false");
            
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
    }

    virtual void compute(std::shared_ptr<PipeData> pipe_data) = 0;

    virtual void put(std::shared_ptr<PipeData> pipe_data) {
        fifo_.push(pipe_data);
    }

    std::shared_ptr<PipeData> get() {
        return fifo_.get();
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

        while(true) { 
            auto t0 = std::chrono::high_resolution_clock::now();

            PipeStamp pipe_stamp(thread_name(), pipe_data->pipe_data_id());
            pipe_stamp.record_now(); 

            std::shared_ptr<PipeData> pipe_data = pre_node()->get_async();

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
            float diff_us = period_us_ - duration.count();
            if ((fps_ > 0) && ( diff_us > 0)) {
                std::chrono::microseconds us(diff_us);
                std::this_thread::sleep_for(us);
            }
            
        }
    }

    virtual void put(std::shared_ptr<PipeData> pipe_data) {
        LOG(FATAL) << "MasterRateNode have no put method!";
    }

    virtual std::shared_ptr<PipeData> get() {
        LOG(FATAL) << "MasterRateNode have no get method!";
    }



protected:
    float fps_;
    float period_us_;
};





class PipeManager
{
public:
    PipeManager(PipeNode* root) : root_(root) {}
protected:
    PipeNode* root_;
};


}//namespace thread
}//namespace duck


