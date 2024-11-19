#pragma once

#include "thread/pipe_thread.h"

using namespace duck::thread;

namespace duck {
namespace pipe {

class CaptureNode : public FilterNode
{
public:
    CaptureNode(const std::string& node_name, int queue_num = 4) : FilterNode(node_name, queue_num) {}
    void compute(PipeData pipe_data) {

        std::this_thread::sleep_for(std::chrono::milliseconds(19)); 
    }
};

class PreProcNode : public FilterNode
{
public:
    PreProcNode(const std::string& node_name, int queue_num = 4) : FilterNode(node_name, queue_num) {}
    void compute(PipeData pipe_data) {

        std::this_thread::sleep_for(std::chrono::milliseconds(19)); 
    }
};

class DetectNode : public FilterNode
{
public:
    DetectNode(const std::string& node_name, int queue_num = 4) : FilterNode(node_name, queue_num) {}
    void compute(PipeData pipe_data) {

        std::this_thread::sleep_for(std::chrono::milliseconds(19)); 
    }
};


class VoPreNode : public FilterNode
{
public:
    VoPreNode(const std::string& node_name, int queue_num = 4) : FilterNode(node_name, queue_num) {}
    void compute(PipeData pipe_data) {

        std::this_thread::sleep_for(std::chrono::milliseconds(19)); 
    }
};


class VoNode : public FilterNode
{
public:
    VoNode(const std::string& node_name, int queue_num = 4) : FilterNode(node_name, queue_num) {}
    void compute(PipeData pipe_data) {

        std::this_thread::sleep_for(std::chrono::milliseconds(19)); 
    }
};

class VencNode : public FilterNode
{
public:
    VencNode(const std::string& node_name, int queue_num = 4) : FilterNode(node_name, queue_num) {}
    void compute(PipeData pipe_data) {

        std::this_thread::sleep_for(std::chrono::milliseconds(25)); 
    }
};

class RecordNode : public FilterNode
{
public:
    RecordNode(const std::string& node_name, int queue_num = 4) : FilterNode(node_name, queue_num) {}
    void compute(PipeData pipe_data) {

        std::this_thread::sleep_for(std::chrono::milliseconds(25)); 
    }
};

class RtspNode : public FilterNode
{
public:
    RtspNode(const std::string& node_name, int queue_num = 4) : FilterNode(node_name, queue_num) {}
    void compute(PipeData pipe_data) {

        std::this_thread::sleep_for(std::chrono::milliseconds(25)); 
    }
};

class BenchMarkNode : public FilterNode
{
public:
    BenchMarkNode(const std::string& node_name, int queue_num = 4) : FilterNode(node_name, queue_num), frame_count_(0), pre_frame_count_(0) {}

    void compute(PipeData pipe_data) {
        frame_count_++;
        pipe_data.show();
        LOG(INFO) << "latency: " << pipe_data.latency_ms();
    }

    int calc_fps() { 
        fps_ = (frame_count_ > pre_frame_count_) ? (frame_count_ - pre_frame_count_) : (pre_frame_count_ - frame_count_);
        pre_frame_count_ = frame_count_;
        return fps_;
    }



protected:
    int frame_count_;
    int pre_frame_count_;
    int fps_;
};


}//namespace pipe
}//namespace duck


