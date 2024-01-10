#include "com_changba_songstudio_Videostudio.h"
#include "h264_reader_thread.h"
#include "aac_reader_thread.h"

#define LOG_TAG "Videostudio"

VideoPacketConsumerThread* videoPacketConsumerThread = NULL;
jobject g_obj = 0;


void your_log_callback(void* avcl, int level, const char* format, va_list args) {
	// 转换FFmpeg日志级别到Android日志级别
	int androidLevel;
	switch (level) {
		case AV_LOG_DEBUG:
			androidLevel = ANDROID_LOG_DEBUG;
			break;
		case AV_LOG_INFO:
			androidLevel = ANDROID_LOG_INFO;
			break;
		case AV_LOG_WARNING:
			androidLevel = ANDROID_LOG_WARN;
			break;
		case AV_LOG_ERROR:
			androidLevel = ANDROID_LOG_ERROR;
			break;
		default:
			androidLevel = ANDROID_LOG_UNKNOWN;
			break;
	}

	// 格式化日志消息
	char logMessage[256];
	vsnprintf(logMessage, sizeof(logMessage), format, args);

	// 打印日志到Android日志系统
	__android_log_write(androidLevel, "FFmpeg", logMessage);
}


JNIEXPORT jint JNICALL Java_com_changba_songstudio_Videostudio_startCommonVideoRecord(JNIEnv * env, jobject obj, jstring outputPath,
		jint videoWidth, jint videoheight, jint videoFrameRate, jint videoBitRate,jint audioSampleRate, jint audioChannels,
		jint audioBitRate,
		jint qualityStrategy,
		jint adaptiveBitrateWindowSizeInSecs,
		jint adaptiveBitrateEncoderReconfigInterval,
		jint adaptiveBitrateWarCntThreshold,
		jint adaptiveMinimumBitrate,
		jint adaptiveMaximumBitrate
		) {
	//设置监听ffmpeg日志
	av_log_set_level(AV_LOG_DEBUG);
    // 设置FFmpeg内部日志回调函数
	av_log_set_callback(your_log_callback); // 替换为你自己的日志回调函数


	int initCode = 0;
	JavaVM *g_jvm = NULL;
	env->GetJavaVM(&g_jvm);
	g_obj = env->NewGlobalRef(obj);
	char* videoPath = (char*) (env->GetStringUTFChars(outputPath, NULL));
	//如果视频临时文件存在则删除掉
	remove(videoPath);
	videoPacketConsumerThread = new VideoPacketConsumerThread();
	/** 预先初始化3个队列, 防止在init过程中把Statistics重新置为空的问题；
	 * 由于先初始化消费者，在初始化生产者，所以队列初始化放到这里 **/
	LiveCommonPacketPool::GetInstance()->initRecordingVideoPacketQueue();
	LiveCommonPacketPool::GetInstance()->initAudioPacketQueue(audioSampleRate);
	LiveAudioPacketPool::GetInstance()->initAudioPacketQueue();

	std::map<std::string, int> configMap;

	//设置码率变化策略参数
	configMap["adaptiveBitrateWindowSizeInSecs"] = adaptiveBitrateWindowSizeInSecs;
	configMap["adaptiveBitrateEncoderReconfigInterval"] = adaptiveBitrateEncoderReconfigInterval;
	configMap["adaptiveBitrateWarCntThreshold"] = adaptiveBitrateWarCntThreshold;
	configMap["adaptiveMinimumBitrate"] = adaptiveMinimumBitrate;
	configMap["adaptiveMaximumBitrate"] = adaptiveMaximumBitrate;

	initCode = videoPacketConsumerThread->init(videoPath, videoWidth, videoheight, videoFrameRate, videoBitRate, audioSampleRate,
			audioChannels, audioBitRate, "libfdk_aac", qualityStrategy, configMap,g_jvm, g_obj);
	LOGI("initCode is %d...qualityStrategy:%d", initCode,qualityStrategy);
	if(initCode >= 0){
		videoPacketConsumerThread->startAsync();
	} else{
		/** 如果初始化失败, 那么就把队列销毁掉 **/
		LiveCommonPacketPool::GetInstance()->destoryRecordingVideoPacketQueue();
		LiveCommonPacketPool::GetInstance()->destoryAudioPacketQueue();
		LiveAudioPacketPool::GetInstance()->destoryAudioPacketQueue();
	}
	env->ReleaseStringUTFChars(outputPath, videoPath);
	return initCode;
}

JNIEXPORT void JNICALL Java_com_changba_songstudio_Videostudio_stopVideoRecord(JNIEnv * env, jobject obj) {
	if (NULL != videoPacketConsumerThread) {
		videoPacketConsumerThread->stop();
		delete videoPacketConsumerThread;
		videoPacketConsumerThread = NULL;

		if (g_obj != 0){
			env->DeleteGlobalRef(g_obj);
			g_obj = 0;
		}
	}
}

H264ReaderThread *h264Thread;

AACReaderThread *aarThread;
extern "C" JNIEXPORT void JNICALL
Java_com_changba_songstudio_Videostudio_startReadFileToMakeMp4(JNIEnv *env, jobject thiz, jstring aac_file_path,
															   jstring h264_file_path) {
	char* h264Path = (char*) (env->GetStringUTFChars(h264_file_path, NULL));
	char* aacPath = (char*) (env->GetStringUTFChars(aac_file_path, NULL));


	if (h264Thread != nullptr) {
		delete h264Thread;
		h264Thread= nullptr;
	}
	char* h264PathRef = new char[strlen(h264Path)+1];
	strcpy(h264PathRef,h264Path);
	h264Thread = new H264ReaderThread(h264PathRef);
	h264Thread->startAsync();


	if (aarThread != nullptr) {
		delete aarThread;
		aarThread= nullptr;
	}
	char* aacPathRef=new char [strlen(aacPath)+1];
	strcpy(aacPathRef,aacPath);
	aarThread = new AACReaderThread(aacPathRef);
	aarThread->startAsync();

	env->ReleaseStringUTFChars(h264_file_path, h264Path);
	env->ReleaseStringUTFChars(aac_file_path, aacPath);
}