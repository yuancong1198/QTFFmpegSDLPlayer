#pragma once
#include <QThread>
#include <QMutex>

/*
    解码线程
    解析模块的主要作用：负责解析各种封装格式。
    解析模式的输入：由IO模块读取的二进制数据。
    解析模式的输出：音视频的媒体信息、未解码的音频包、未解码的视频包。
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

