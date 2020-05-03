#include "ReadPacketsThread.h"
#include "Media.h"
#include <QMutexLocker>
extern "C" {

#include <libavformat\avformat.h>

}
static bool isExit = false;

//************************************
// Method:    run
// FullName:  ReadPacketsThread::run
// Access:    public 
// Returns:   void
// Qualifier:��ȡ����Ƶ�����̴߳�����
//************************************
void ReadPacketsThread::run()
{
	AVPacket packet;
	while (!isExit) {
		QMutexLocker locker(&mutex);
		if (!isPlay) {//��û��ʼ����
			locker.unlock();
			msleep(100);
			continue;
		}
		Media *media = Media::getInstance();
		if (media->audio == nullptr || media->video == nullptr) {
			break;
		}
		if (isSeek) {//�Ƿ���ת��Ƶ�ı�ʶ���ڽ�ѹ����Ƶ����ʱ������ת			 
			int64_t stamp = 0;
			stamp = currentPos * media->video->getVideoStream()->duration;
			 			 
			int ret = av_seek_frame(media->getAVFormatContext(), media->getVideoStreamIndex(),
				stamp, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
			media->audio->clearPacket();//Ҫ������ж���
			media->video->clearPackets();
			media->video->clearFrames();		 	 
			isSeek = false;
		}
		
		bool isInvalid = media->checkMediaSizeValid();//���Ӷ��г���һ����ʱ�Ȳ��������ȴ�������
		if (isInvalid) {
			locker.unlock();
			msleep(10);
			continue;
		}
		if (!media->getAVFormatContext()) {
			locker.unlock();
			msleep(10);
			continue;
		}
        //δ���������Ƶ����->YUV/PCM 
		int ret = av_read_frame(media->getAVFormatContext(), &packet);//��֡����Ƶ��
		if (ret < 0)
		{
			if (ret == AVERROR_EOF) {//��������
				break;
			}
			if (media->getAVFormatContext()->pb->error == 0) // û�д���͵ȴ��´ζ�
			{
				locker.unlock();
				msleep(100);
				continue;
			}
			else {
				break;
			}	
		}
		if (media->audio!=nullptr&& packet.stream_index == media->getAudioStreamIndex()) // ��Ƶ�����д˴����(δ�������Ƶ���ݷ���֡������)
		{
			locker.unlock();
			media->enqueueAudioPacket(packet);
		}
		else if (media->video != nullptr&& packet.stream_index == media->getVideoStreamIndex()) // ��Ƶ�����д˴����(δ�������Ƶ���ݷ���֡������)
		{
			locker.unlock();
			media->enqueueVideoPacket(packet);
		}
		else {
			av_packet_unref(&packet);
		}
        //δ���������Ƶ����->YUV/PCM^
	}
	if(packet.size>=0)
	    av_packet_unref(&packet);//��û���ݲ����ͷ�
}

ReadPacketsThread::ReadPacketsThread()
{
}

ReadPacketsThread::~ReadPacketsThread()
{
	QMutexLocker locker(&mutex);
	isExit = true;
	locker.unlock();
	wait();
}

//************************************
// Method:    getIsPlaying
// FullName:  ReadPacketsThread::getIsPlaying
// Access:    public 
// Returns:   bool
// Qualifier: ��ȡ��Ƶ����״̬
//************************************
bool ReadPacketsThread::getIsPlaying()
{
	return isPlay;
}

//************************************
// Method:    setPlaying
// FullName:  ReadPacketsThread::setPlaying
// Access:    public 
// Returns:   void
// Qualifier: ���ò���״̬
// Parameter: bool isPlaying
//************************************
void ReadPacketsThread::setPlaying(bool isPlaying)
{
	this->isPlay = isPlaying;
}

//************************************
// Method:    receivePos
// FullName:  ReadPacketsThread::receivePos
// Access:    public 
// Returns:   void
// Qualifier: ������ת��λ�ã���ת��ʶ����true
// Parameter: float pos ��Ƶ��תλ��
//************************************
void ReadPacketsThread::receivePos(float pos) {
	currentPos = pos;
	isSeek = true;
}