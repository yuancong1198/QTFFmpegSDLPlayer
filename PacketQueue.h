
#pragma once

#include <queue>

#include <SDL.h>
#include <QWaitCondition>
#include <QMutex>
extern "C"{

#include <libavcodec\avcodec.h>

}

/*
    ���뻺������֡�����������Ӷ��̺߳ͽ����߳�֮�����Ŧ����Ҫ���ڷ�ֹ���綶���ͽ��붶�����������𿨶ٵ�����
*/

class PacketQueue
{
public:
	PacketQueue();
	bool enQueue(const AVPacket packet);//��Ϣ����-���
	AVPacket deQueue(); //��Ϣ���г���
	Uint32 getPacketSize();//��ǰ��Ϣ���еĳ���
	void queueFlush();//�����Ϣ����(����ʱ�������������á����Ž��������϶���)
	~PacketQueue();
private:
	std::queue<AVPacket> queue;
	Uint32    size;
	QMutex mutex;
	QWaitCondition cond;
};

 