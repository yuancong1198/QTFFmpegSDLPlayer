
#pragma once

#include <queue>

#include <SDL.h>
#include <QWaitCondition>
#include <QMutex>
extern "C"{

#include <libavcodec\avcodec.h>

}

/*
    解码缓冲区：帧缓冲区是连接读线程和解码线程之间的枢纽，主要用于防止网络抖动和解码抖动，避免引起卡顿的现象。
*/

class PacketQueue
{
public:
	PacketQueue();
	bool enQueue(const AVPacket packet);//消息队列-入队
	AVPacket deQueue(); //消息队列出队
	Uint32 getPacketSize();//当前消息队列的长度
	void queueFlush();//清空消息队列(调用时机：播放器重置、播放进度条被拖动等)
	~PacketQueue();
private:
	std::queue<AVPacket> queue;
	Uint32    size;
	QMutex mutex;
	QWaitCondition cond;
};

 