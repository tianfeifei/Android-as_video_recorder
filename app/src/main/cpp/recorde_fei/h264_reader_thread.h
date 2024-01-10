#ifndef H264_READER_THREAD_H
#define H264_READER_THREAD_H

#include "../common/CommonTools.h"
#include "../livecore/common/live_thread.h"
#include "../livecore/platform_dependent/platform_4_live_common.h"
#include "../livecore/common/live_packet_pool.h"
#include "../livecore/platform_dependent/platform_4_live_ffmpeg.h"
#include "./h264_util.h"


class H264ReaderThread : public LiveThread {
public:
    AVFormatContext *formatContext = NULL;
    AVCodecContext *codecContext = NULL;
    AVPacket packet;
    AVCodec *codec = NULL;

    LivePacketPool *packetPool; //视频包的队列
    char *path;//文件路径
    H264ReaderThread(char *h264Path);

    ~H264ReaderThread();

    void stop() override;

    void handleRun(void *ptr);

private:
    bool isSPSUnWriteFlag;
    int64_t startTime=-1;

    void pushToQueue(AVPacket *pkt);
    LiveVideoPacket* newLiveVideoPacket(byte* buffer, int size, int64_t timeMills);
};

#endif //H264_READER_THREAD_H
