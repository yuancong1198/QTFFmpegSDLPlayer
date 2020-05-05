
#pragma once
#include <queue>
#include <QMutex>
#include <QWaitCondition>
extern "C"{

#include <libavcodec\avcodec.h>
#include <libavformat/avformat.h>
}
 
/*
    显示缓冲区：是连接解码模块和显示模块之间的枢纽。主要作用是防止解码/渲染抖动导致的卡顿和实现音视同步。
    实现原理：消息队列
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
    std::queue<AVFrame*> queue;//已解码的音视频帧队列
	QMutex mutex;
    /*QWaitCondition 用于多线程的同步，一个线程调用QWaitCondition::wait() 阻塞等待，
    直到另一个线程调用QWaitCondition::wake() 唤醒才继续往下执行。*/
	QWaitCondition cond;//条件变量


};

