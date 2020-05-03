#pragma once
#include <QThread>
#include <QMutex>

/*
    �����߳�
    ����ģ�����Ҫ���ã�����������ַ�װ��ʽ��
    ����ģʽ�����룺��IOģ���ȡ�Ķ��������ݡ�
    ����ģʽ�����������Ƶ��ý����Ϣ��δ�������Ƶ����δ�������Ƶ����
*/
class ReadPacketsThread :public QThread
{
	Q_OBJECT
public:
	static ReadPacketsThread * getInstance() {
		static ReadPacketsThread rpt;
		return &rpt;
	}
	void run();
	~ReadPacketsThread();
	bool getIsPlaying();
	float currentPos = 0;
	bool isSeek = false;
	void setPlaying(bool isPlaying);
public slots:
	void receivePos(float pos);
private:
	QMutex mutex;
	ReadPacketsThread();
	bool isPlay = false;
};

