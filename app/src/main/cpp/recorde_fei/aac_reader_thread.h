//
// Created by tianfei on 2023/11/14.
//


#ifndef ANDROID_AS_VIDEO_RECORDER_AAC_READER_THREAD_H
#define ANDROID_AS_VIDEO_RECORDER_AAC_READER_THREAD_H


#include "../common/CommonTools.h"
#include "../livecore/common/live_thread.h"
#include "../livecore/platform_dependent/platform_4_live_common.h"
#include "../livecore/common/live_packet_pool.h"
#include "../livecore/platform_dependent/platform_4_live_ffmpeg.h"
#include "./h264_util.h"
#include "../livecore/common/live_audio_packet_pool.h"


class AACReaderThread : public LiveThread {

public:
    AVFormatContext *formatContext = NULL;
    AVCodecContext *codecContext = NULL;
    AVPacket packet;
    AVCodec *codec = NULL;
    LiveAudioPacketPool *aacPacketPool; //音频包队列

    char * path1;//文件路径
    AVRational time_base = {1, 16000};

    AACReaderThread(char *aacPath);

    ~AACReaderThread();


    void handleRun(void *ptr);
private:
    int64_t startTime=-1;

};

#endif //ANDROID_AS_VIDEO_RECORDER_AAC_READER_THREAD_H