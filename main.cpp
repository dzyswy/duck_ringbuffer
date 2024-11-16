#include "thread/pipe_thread.h"
#include "pipe/user_node.h"


using namespace duck::pipe;
using namespace duck::thread;


int main(int argc, char* argv[])
{
    google::InstallFailureSignalHandler();
    google::InitGoogleLogging(argv[0]);

    FLAGS_stderrthreshold = 0;
    FLAGS_minloglevel = 0;

    MasterRateNode node_root("node_root", -1);
    CaptureNode node_cap("node_cap");
    PreProcNode node_pre_proc("node_pre_proc");
    DetectNode node_detect("node_detect");
    BroadcastNode broad_vi("broad_vi", 4);

    SlaveRateNode node_vo_rate("node_vo_rate", 50.0);
    VoPreNode node_vo_pre("node_vo_pre");
    VoNode node_vo("node_vo");


    VencNode node_venc("node_venc");
    BroadcastNode broad_venc("broad_venc", 4);

    SlaveRateNode node_record_rate("node_record_rate", 25.0);
    RecordNode node_record("node_record");

    SlaveRateNode node_rtsp_rate("node_rtsp_rate", 30.0);
    RtspNode node_rtsp("node_rtsp");

    node_root.append(&node_cap);
    node_cap.append(&node_pre_proc);
    node_pre_proc.append(&node_detect);
    node_detect.append(&broad_vi);

    broad_vi.append(&node_vo_rate);
    node_vo_rate.append(&node_vo_pre);
    node_vo_pre.append(&node_vo);


    broad_vi.append(&node_venc);
    node_venc.append(&broad_venc);

    broad_venc.append(&node_record_rate);
    node_record_rate.append(&node_record);

    broad_venc.append(&node_rtsp_rate);
    node_rtsp_rate.append(&node_rtsp);

    
    node_root.show();

    std::cout << "wait key..." << std::endl;
    std::getchar(); 
 

    std::cout << "bye!" << std::endl;

    return 0;
}












