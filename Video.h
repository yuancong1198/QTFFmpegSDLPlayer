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
    ��Ƶ�����̣߳�����δ��������������н������������֡����������ʾ������֮�䣬�������������PCM֡���ݡ�
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

	AVFrame * dequeueFrame();   //��ʾ֡����
	void setStreamIndex(const int &streamIndex);
	int getVideoQueueSize();//��ȡ֡�������ĵ�ǰ��С
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
	double frameTimer;         // Sync fields ����ͬ��ʱ��������Ƶ֡�Ķ�ʱ������
	double frameLastPts;        //��һ��Ƶ֡����ʾʱ�����Ĭ��Ϊ0
	double frameLastDelay; //��һ֡������֮֡���ʱ����
	double videoClock;          //��Ƶ��ʱ��ʱ��
	PacketQueue *videoPackets;	            //δ������������Ϣ����
	FrameQueue frameQueue;                //�ѽ���PCM����Ϣ����
	AVStream *stream;
	int streamIndex = -1;
	QMutex mutex;
	AVCodecContext *videoContext;       //������������
};

