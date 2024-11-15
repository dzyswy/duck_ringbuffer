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
    virtual void process() {}
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
    BroadcastNode(const std::string& node_name) : PipeNode(node_name) {}

    virtual bool is_broadcast() {return true;}

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


