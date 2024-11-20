#include "thread/pipe_thread.h"
#include "pipe/user_node.h"
#include "timer/timer_manager.h"

using namespace duck::pipe;
using namespace duck::thread;
using namespace duck::timer;

void stats_fps(BenchMarkNode* node)
{ 
    int fps = node->calc_fps();
    LOG(WARNING) << node->name() << " fps=" << fps; 
}

int main(int argc, char* argv[])
{
    google::InstallFailureSignalHandler();
    google::InitGoogleLogging(argv[0]);

    if (argc != 2) {
        printf("usage: %s (0:INFO, 1:WARNING, 2:ERROR, 3:FATAL)\n", argv[0]);
        return -1;
    }

    FLAGS_stderrthreshold = atoi(argv[1]);
    FLAGS_minloglevel = 0;

    TimerManager manager(1);
 
    CaptureNode node_cap("node_cap");
    PreProcNode node_pre_proc("node_pre_proc");
    DetectNode node_detect("node_detect"); 
 
    VoPreNode node_vo_pre("node_vo_pre", 4, 33333);
    VoNode node_vo("node_vo");
    BenchMarkNode node_bench_vo("node_bench_vo");


    VencNode node_venc("node_venc"); 
 
    RecordNode node_record("node_record", 4, 50000);
    BenchMarkNode node_bench_record("node_bench_record");
 
    RtspNode node_rtsp("node_rtsp", 4, 40000);
    BenchMarkNode node_bench_rtsp("node_bench_rtsp");

    node_cap.append(&node_pre_proc)->append(&node_detect)->append(&node_vo_pre)->append(&node_vo)->append(&node_bench_vo);
 
    node_detect.append(&node_venc)->append(&node_record);//->append(&node_bench_record);

    node_venc.append(&node_rtsp);//->append(&node_bench_rtsp);

    manager.submit(1000, stats_fps, &node_bench_vo);
    manager.submit(1000, stats_fps, &node_bench_record);
    manager.submit(1000, stats_fps, &node_bench_rtsp);
    manager.start(); 

    node_cap.show();

    node_cap.start();
    std::this_thread::sleep_for(std::chrono::seconds(5)); 
    node_cap.stop();

    std::cout << "wait key..." << std::endl;
    std::getchar(); 
    manager.stop(); 

    std::cout << "bye!" << std::endl;

    return 0;
}












