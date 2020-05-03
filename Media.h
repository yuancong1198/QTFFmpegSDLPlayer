#pragma once
#include "Audio.h"
#include "Video.h"
#include <QMutex>
extern "C" {

#include <libavformat\avformat.h>

}
/**
    	解析模块的主要作用：负责解析各种封装格式。
        解析模式的输入：由IO模块读取的二进制数据。
        解析模式的输出：音视频的媒体信息、未解码的音频包、未解码的视频包。
 */
class Media
{
public:
	static Media *getInstance() {
		static Media media;
		return &media;
	}
	~Media();
	Media * config();
	Media * setMediaFile(const char*filename);
	bool checkMediaSizeValid();
	int getVideoStreamIndex();
	int getAudioStreamIndex();
	void enqueueVideoPacket(const AVPacket &packet);
	void enqueueAudioPacket(const AVPacket &packet);
	void startAudioPlay();
	AVFormatContext *getAVFormatContext();
	void close();
    Video *video;
    Audio *audio;
	int totalMs = 0;//当前视频的总播放时长
	int pts = 0; //播放器的主时钟
private:
    Media();
	AVFormatContext *pFormatCtx;
	const char *filename;
	QMutex mutex;
};

