//
// Created by tianfei on 2023/11/17.
//
#include "aac_reader_thread.h"

#define LOG_TAG "AACReaderThread"

AACReaderThread::AACReaderThread(char *aacPath) : LiveThread() {
    LOGI("AACReaderThread-->%s", aacPath);
    path1 = aacPath;
    aacPacketPool = LiveAudioPacketPool::GetInstance();
    aacPacketPool->initAudioPacketQueue();
}


void AACReaderThread::handleRun(void *ptr) {
    LOGI("handleRun-->%s", path1);

    av_register_all();
    if (avformat_open_input(&formatContext, path1, NULL, NULL) != 0) {
        LOGI("avformat_open_input failed");
        return;
    }

    // 查找音频流信息
    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        LOGI("Failed to find stream information");
        avformat_close_input(&formatContext);
        return;
    }


    // 查找音频流索引
    int audioStreamIndex = -1;
    for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndex = i;
            break;
        }
    }


    codecContext = formatContext->streams[audioStreamIndex]->codec;
    codec = avcodec_find_decoder(codecContext->codec_id);
    if (!codec) {
        printf("Failed to find decoder\n");
        return;
    }

    // 打开解码器
    if (avcodec_open2(codecContext, codec, NULL) < 0) {
        printf("Failed to open decoder\n");
        return;
    }


    // 读取一帧数据
    while (av_read_frame(formatContext, &packet) >= 0) {
        if (packet.stream_index == audioStreamIndex) {
            // 处理音频帧数据（例如：保存到内存、处理等）
            packet.pts = currentTimeMills();
            packet.dts = packet.pts;
            LOGI("av_read_frame:size=%d, pts=%lld", packet.size, packet.pts);
            LiveAudioPacket *audioPacket = new LiveAudioPacket();
            audioPacket->data = new byte[packet.size];
            memcpy(audioPacket->data, packet.data, packet.size);
            audioPacket->size = packet.size;
            audioPacket->position = (float) (packet.pts * av_q2d(time_base) * 1000.0f);
            aacPacketPool->pushAudioPacketToQueue(audioPacket);
            av_packet_unref(&packet);
            sleep(30);
        }
    }
    LOGI("成功放入音频队列%d", aacPacketPool->getAudioPacketQueueSize());

}

AACReaderThread::~AACReaderThread() {
    LOGI("AACReaderThread析构");
    delete []path1;
    path1 = nullptr;
}
