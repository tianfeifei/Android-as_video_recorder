//
// Created by tianfei on 2023/11/14.
//
#include "h264_reader_thread.h"
#include "live_common_packet_pool.h"

#define LOG_TAG "H264ReaderThread"

H264ReaderThread::H264ReaderThread(char *h264Path) : LiveThread() {

    path = h264Path;
    LOGI("H264ReaderThread-->%s", path);
    packetPool = LiveCommonPacketPool::GetInstance();
    LOGI("LivePacketPool1-->%p", packetPool);
    isSPSUnWriteFlag= true;
    packetPool->initRecordingVideoPacketQueue();
}


H264ReaderThread::~H264ReaderThread() {
    delete[]path;
    path = nullptr;
}

void H264ReaderThread::handleRun(void *ptr) {
    LOGI("handleRun-->%s", path);
    av_register_all();
    if (avformat_open_input(&formatContext, path, NULL, NULL) != 0) {
        LOGI("avformat_open_input failed");
        return;
    }

    // 查找视频流信息
    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        LOGI("Failed to find stream information");
        avformat_close_input(&formatContext);
        return;
    }


    // 查找音频流索引
    int videoStreamIndex = -1;
    for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }


    codecContext = formatContext->streams[videoStreamIndex]->codec;
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
        if (packet.stream_index == videoStreamIndex) {
            // 处理音频帧数据（例如：保存到内存、处理等）

            //todo pts 赋值，直接取当前时间
            packet.pts = -1;
            packet.dts = -1;

            pushToQueue(&packet);
            av_packet_unref(&packet);
            sleep(40);

        }
    }
    LOGI("读取完成%d", packetPool->getRecordingVideoPacketQueueSize());
}

void H264ReaderThread::stop() {}


void H264ReaderThread::pushToQueue(AVPacket *pkt) {
    AVRational time_base = {1, 1000};
    int64_t presentationTimeMills = pkt->pts * av_q2d(time_base) * 1000.0f;
//    LOGI("pkt : {%llu, %llu}", pkt->pts, pkt->dts);
    int nalu_type = (pkt->data[4] & 0x1F);
    bool isKeyFrame = false;
    byte *frameBuffer;
    int frameBufferSize = 0;
    const char bytesHeader[] = "\x00\x00\x00\x01";
    size_t headerLength = 4; //string literals have implicit trailing '\0'
    if (H264_NALU_TYPE_SEQUENCE_PARAMETER_SET == nalu_type) {
        //说明是关键帧
        isKeyFrame = true;
        //分离出sps pps
        vector<NALUnit *> *units = new vector<NALUnit *>();
        parseH264SpsPps(pkt->data, pkt->size, units);
        if (isSPSUnWriteFlag) {

            NALUnit *spsUnit = units->at(0);
            uint8_t *spsFrame = spsUnit->naluBody;
            int spsFrameLen = spsUnit->naluSize;
            NALUnit *ppsUnit = units->at(1);
            uint8_t *ppsFrame = ppsUnit->naluBody;
            int ppsFrameLen = ppsUnit->naluSize;
            //将sps、pps写出去
            int metaBuffertSize = headerLength + spsFrameLen + headerLength + ppsFrameLen;
            byte *metaBuffer = new byte[metaBuffertSize];
            memcpy(metaBuffer, bytesHeader, headerLength);
            memcpy(metaBuffer + headerLength, spsFrame, spsFrameLen);
            memcpy(metaBuffer + headerLength + spsFrameLen, bytesHeader, headerLength);
            memcpy(metaBuffer + headerLength + spsFrameLen + headerLength, ppsFrame, ppsFrameLen);
            packetPool->pushRecordingVideoPacketToQueue(
                    this->newLiveVideoPacket(metaBuffer, metaBuffertSize, presentationTimeMills));
            isSPSUnWriteFlag = false;
        }
        vector<NALUnit *>::iterator i;
        bool isFirstIDRFrame = true;
        for (i = units->begin(); i != units->end(); ++i) {
            NALUnit *unit = *i;
            int naluType = unit->naluType;
            if (H264_NALU_TYPE_SEQUENCE_PARAMETER_SET != naluType &&
                H264_NALU_TYPE_PICTURE_PARAMETER_SET != naluType) {
                int idrFrameLen = unit->naluSize;
                frameBufferSize += headerLength;
                frameBufferSize += idrFrameLen;
            }
        }
        frameBuffer = new byte[frameBufferSize];
        int frameBufferCursor = 0;
        for (i = units->begin(); i != units->end(); ++i) {
            NALUnit *unit = *i;
            int naluType = unit->naluType;
            if (H264_NALU_TYPE_SEQUENCE_PARAMETER_SET != naluType &&
                H264_NALU_TYPE_PICTURE_PARAMETER_SET != naluType) {
                uint8_t *idrFrame = unit->naluBody;
                int idrFrameLen = unit->naluSize;
                //将关键帧分离出来
                memcpy(frameBuffer + frameBufferCursor, bytesHeader, headerLength);
                frameBufferCursor += headerLength;
                memcpy(frameBuffer + frameBufferCursor, idrFrame, idrFrameLen);
                frameBufferCursor += idrFrameLen;
                frameBuffer[frameBufferCursor - idrFrameLen - headerLength] = ((idrFrameLen) >> 24) & 0x00ff;
                frameBuffer[frameBufferCursor - idrFrameLen - headerLength + 1] = ((idrFrameLen) >> 16) & 0x00ff;
                frameBuffer[frameBufferCursor - idrFrameLen - headerLength + 2] = ((idrFrameLen) >> 8) & 0x00ff;
                frameBuffer[frameBufferCursor - idrFrameLen - headerLength + 3] = ((idrFrameLen)) & 0x00ff;
            }
            delete unit;
        }
        delete units;
    } else {
        //说明是非关键帧, 从Packet里面分离出来
        isKeyFrame = false;
        auto *units = new vector<NALUnit *>();
        parseH264SpsPps(pkt->data, pkt->size, units);
        vector<NALUnit *>::iterator i;
        for (i = units->begin(); i != units->end(); ++i) {
            NALUnit *unit = *i;
            int nonIDRFrameLen = unit->naluSize;
            frameBufferSize += headerLength;
            frameBufferSize += nonIDRFrameLen;
        }
        frameBuffer = new byte[frameBufferSize];
        int frameBufferCursor = 0;
        for (i = units->begin(); i != units->end(); ++i) {
            NALUnit *unit = *i;
            uint8_t *nonIDRFrame = unit->naluBody;
            int nonIDRFrameLen = unit->naluSize;
            memcpy(frameBuffer + frameBufferCursor, bytesHeader, headerLength);
            frameBufferCursor += headerLength;
            memcpy(frameBuffer + frameBufferCursor, nonIDRFrame, nonIDRFrameLen);
            frameBufferCursor += nonIDRFrameLen;
            frameBuffer[frameBufferCursor - nonIDRFrameLen - headerLength] = ((nonIDRFrameLen) >> 24) & 0x00ff;
            frameBuffer[frameBufferCursor - nonIDRFrameLen - headerLength + 1] = ((nonIDRFrameLen) >> 16) & 0x00ff;
            frameBuffer[frameBufferCursor - nonIDRFrameLen - headerLength + 2] = ((nonIDRFrameLen) >> 8) & 0x00ff;
            frameBuffer[frameBufferCursor - nonIDRFrameLen - headerLength + 3] = ((nonIDRFrameLen)) & 0x00ff;
            delete unit;
        }
        delete units;
    }
    bool result = packetPool->pushRecordingVideoPacketToQueue(
            this->newLiveVideoPacket(frameBuffer, frameBufferSize, presentationTimeMills));
}

LiveVideoPacket *H264ReaderThread::newLiveVideoPacket(byte *buffer, int size, int64_t timeMills) {
    if (startTime == -1)
        startTime = getCurrentTime();
    int recordingDuration = getCurrentTime() - startTime;

    LiveVideoPacket *h264Packet = new LiveVideoPacket();
    h264Packet->buffer = buffer;
    h264Packet->size = size;
    h264Packet->timeMills = recordingDuration;
    h264Packet->pts = -1;
    h264Packet->dts = -1;
    return h264Packet;
}