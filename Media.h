#pragma once
#include "Audio.h"
#include "Video.h"
#include <QMutex>
extern "C" {

#include <libavformat\avformat.h>

}
/**
    	����ģ�����Ҫ���ã�����������ַ�װ��ʽ��
        ����ģʽ�����룺��IOģ���ȡ�Ķ��������ݡ�
        ����ģʽ�����������Ƶ��ý����Ϣ��δ�������Ƶ����δ�������Ƶ����
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
	int totalMs = 0;//��ǰ��Ƶ���ܲ���ʱ��
	int pts = 0; //����������ʱ��
private:
    Media();
	AVFormatContext *pFormatCtx;
	const char *filename;
	QMutex mutex;
};

