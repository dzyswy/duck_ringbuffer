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
    BenchMarkNode(const std::string& node_name, int queue_num = 4) : FilterNode(node_name, queue_num) {}

    void compute(PipeData pipe_data) {
 
        pipe_data.show();
        LOG(INFO) << "latency: " << pipe_data.latency_ms();
    }

};


}//namespace pipe
}//namespace duck


