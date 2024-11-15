#pragma once

#include "thread/pipe_thread.h"

using namespace duck::thread;

namespace duck {
namespace pipe {

class CaptureNode : public FilterNode
{
public:
    CaptureNode(const std::string& node_name, int queue_num = 4) : FilterNode(node_name, queue_num) {}
    void compute(std::shared_ptr<PipeData> pipe_data) {

        std::this_thread::sleep_for(std::chrono::milliseconds(25)); 
    }
};

class PreProcNode : public FilterNode
{
public:
    PreProcNode(const std::string& node_name, int queue_num = 4) : FilterNode(node_name, queue_num) {}
    void compute(std::shared_ptr<PipeData> pipe_data) {

        std::this_thread::sleep_for(std::chrono::milliseconds(25)); 
    }
};

class DetectNode : public FilterNode
{
public:
    DetectNode(const std::string& node_name, int queue_num = 4) : FilterNode(node_name, queue_num) {}
    void compute(std::shared_ptr<PipeData> pipe_data) {

        std::this_thread::sleep_for(std::chrono::milliseconds(25)); 
    }
};


class VoPreNode : public FilterNode
{
public:
    VoPreNode(const std::string& node_name, int queue_num = 4) : FilterNode(node_name, queue_num) {}
    void compute(std::shared_ptr<PipeData> pipe_data) {

        std::this_thread::sleep_for(std::chrono::milliseconds(25)); 
    }
};


class VoNode : public FilterNode
{
public:
    VoNode(const std::string& node_name, int queue_num = 4) : FilterNode(node_name, queue_num) {}
    void compute(std::shared_ptr<PipeData> pipe_data) {

        std::this_thread::sleep_for(std::chrono::milliseconds(25)); 
    }
};

class VencNode : public FilterNode
{
public:
    VencNode(const std::string& node_name, int queue_num = 4) : FilterNode(node_name, queue_num) {}
    void compute(std::shared_ptr<PipeData> pipe_data) {

        std::this_thread::sleep_for(std::chrono::milliseconds(25)); 
    }
};

class RecordNode : public FilterNode
{
public:
    RecordNode(const std::string& node_name, int queue_num = 4) : FilterNode(node_name, queue_num) {}
    void compute(std::shared_ptr<PipeData> pipe_data) {

        std::this_thread::sleep_for(std::chrono::milliseconds(25)); 
    }
};

class RtspNode : public FilterNode
{
public:
    RtspNode(const std::string& node_name, int queue_num = 4) : FilterNode(node_name, queue_num) {}
    void compute(std::shared_ptr<PipeData> pipe_data) {

        std::this_thread::sleep_for(std::chrono::milliseconds(25)); 
    }
};


}//namespace pipe
}//namespace duck


