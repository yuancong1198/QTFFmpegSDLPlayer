
#pragma once
#include <queue>
#include <QMutex>
#include <QWaitCondition>
extern "C"{

#include <libavcodec\avcodec.h>
#include <libavformat/avformat.h>
}
 
/*
    ��ʾ�������������ӽ���ģ�����ʾģ��֮�����Ŧ����Ҫ�����Ƿ�ֹ����/��Ⱦ�������µĿ��ٺ�ʵ������ͬ����
    ʵ��ԭ����Ϣ����
*/

class FrameQueue
{
public:
	static const int capacity = 30;
	FrameQueue();
	bool enQueue(const AVFrame* frame);
	AVFrame * deQueue();	
	int getQueueSize();
	void queueFlush();
private:
    std::queue<AVFrame*> queue;//�ѽ��������Ƶ֡����
	QMutex mutex;
    /*QWaitCondition ���ڶ��̵߳�ͬ����һ���̵߳���QWaitCondition::wait() �����ȴ���
    ֱ����һ���̵߳���QWaitCondition::wake() ���Ѳż�������ִ�С�*/
	QWaitCondition cond;//��������


};

