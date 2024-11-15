#include "thread/pipe_thread.h"



using namespace duck::thread;


int main(int argc, char* argv[])
{
    google::InstallFailureSignalHandler();
    google::InitGoogleLogging(argv[0]);

    FLAGS_stderrthreshold = 0;
    FLAGS_minloglevel = 0;

    ChainNode node_vi_rate("node_vi_rate");
    ChainNode node_cap("node_cap");
    ChainNode node_preproc("node_preproc");
    ChainNode node_detect("node_detect");

    BroadcastNode node_vi_broad("node_vi_broad");

    ChainNode node_vo_rate("node_vo_rate");
    ChainNode node_vo_pre("node_vo_pre");
    ChainNode node_vo("node_vo");

    ChainNode node_venc_rate("node_venc_rate");
    ChainNode node_venc("node_venc");

    BroadcastNode node_venc_broad("node_venc_broad");

    ChainNode node_record_rate("node_record_rate");
    ChainNode node_record("node_record");

    ChainNode node_rtsp_rate("node_rtsp_rate");
    ChainNode node_rtsp("node_rtsp");

/*
node_vi_rate
node_cap
node_preproc
node_detect
node_vi_broad
    node_vo_rate
    node_vo_pre
    -------------
    node_venc_rate
    node_venc
    node_venc_broad
        node_record_rate
        node_record
        ----------------
        node_rtsp_rate
        node_rtsp
*/

    node_vi_rate.append(&node_cap);
    node_cap.append(&node_preproc);
    node_preproc.append(&node_detect);
    node_detect.append(&node_vi_broad);

    node_vi_broad.add_next_node(&node_vo_rate);

    node_vo_rate.append(&node_vo_pre);
    node_vo_pre.append(&node_vo);

    node_vi_broad.add_next_node(&node_venc_rate);
    node_venc_rate.append(&node_venc);
    node_venc.append(&node_venc_broad);

    node_venc_broad.add_next_node(&node_record_rate);
    node_record_rate.append(&node_record);
  
    node_venc_broad.add_next_node(&node_rtsp_rate);
    node_rtsp_rate.append(&node_rtsp);

    PipeManager xmanager(&node_vi_rate);

    node_vi_rate.show();


    std::cout << "wait key..." << std::endl;
    std::getchar(); 
 

    std::cout << "bye!" << std::endl;

    return 0;
}












