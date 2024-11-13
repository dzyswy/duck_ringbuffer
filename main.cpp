#include "thread/ringbuffer.h"






int main(int argc, char* argv[])
{
    google::InstallFailureSignalHandler();
    google::InitGoogleLogging(argv[0]);

    FLAGS_stderrthreshold = 0;
    FLAGS_minloglevel = 0;

    std::vector<Frame> frame_buff;

    int frame_num = 4;
    for (int i = 0; i < frame_num; i++) {
        std::shared_ptr<Frame> frame = std::make_shared<Frame>();
    }

    RingBuffer<std::shared_ptr<Frame> > ring_buff(frame_num);

    ProducterThread producter(&ring_buff);
    CustomerThread customer0(&ring_buff);

    std::cout << "wait key..." << std::endl;
    std::getchar(); 

    xpipe.stop();

    std::cout << "bye!" << std::endl;

    return 0;
}












