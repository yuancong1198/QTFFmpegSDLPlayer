#pragma once
#include "PacketQueue.h"
#include "FrameQueue.h"
#include <QThread>
extern "C" {

#include <libavformat/avformat.h>
#include<libswscale/swscale.h>
#include <libswresample/swresample.h>
}

/*
    视频解码线程：负责将未被解码的码流进行解码操作。处于帧缓冲区和显示缓冲区之间，输入码流，输出PCM帧数据。
*/

class Video:public QThread
{
public:
	Video();
	~Video();		
	void run();
	double synchronizeVideo(AVFrame *&srcFrame, double &pts);
	int getStreamIndex();
	AVCodecContext * getAVCodecCotext();
	void enqueuePacket(const AVPacket &pkt);

	AVFrame * dequeueFrame();   //显示帧出列
	void setStreamIndex(const int &streamIndex);
	int getVideoQueueSize();//获取帧缓冲区的当前大小
	void setVideoStream(AVStream *& stream);
	AVStream * getVideoStream();	
	void setAVCodecCotext(AVCodecContext *avCodecContext);
	void setFrameTimer(const double &frameTimer);
	double getFrameTimer();
	void setFrameLastPts(const double &frameLastPts);
	double getFrameLastPts();
	void setFrameLastDelay(const double &frameLastDelay);
	double getFrameLastDelay();
	int getVideoFrameSiez();
	SwsContext *swsContext = NULL;
	void clearFrames();
	void clearPackets();
private:
	double frameTimer;         // Sync fields 音视同步时，解析视频帧的定时器参数
	double frameLastPts;        //上一视频帧的显示时间戳，默认为0
	double frameLastDelay; //上一帧与上上帧之间的时间间隔
	double videoClock;          //视频的时间时钟
	PacketQueue *videoPackets;	            //未解码码流的消息队列
	FrameQueue frameQueue;                //已解码PCM的消息队列
	AVStream *stream;
	int streamIndex = -1;
	QMutex mutex;
	AVCodecContext *videoContext;       //解码器上下文
};

