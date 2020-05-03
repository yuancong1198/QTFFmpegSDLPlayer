#include "DisplayMediaTimer.h"
#include "Media.h"
#include <QMutexLocker>
#include "ImageFilter.h"
static const double SYNC_THRESHOLD = 0.01;//ͬ���ٽ�ֵ
static const double NOSYNC_THRESHOLD = 10.0;//��ͬ���ٽ�ֵ

//************************************
// Method:    synchronizeAudioAndVideo
// FullName:  DisplayMediaTimer::synchronizeAudioAndVideo
// Access:    public 
// Returns:   void
// Qualifier:����Ƶͬ������
//************************************
void DisplayMediaTimer::synchronizeAudioAndVideo()
{		 
 	if (!isPlay) {
		timer->setInterval(20);//��û���ž������´μ��
		return;
	}
	if (Media::getInstance()->video->getVideoFrameSiez() == 0) {
		timer->setInterval(20);//��Ƶ����û֡�������´μ��
		return;
	}
	static int lastWidth = imageWidth;
	static int lastHeight = imageHeight;
	static QImage::Format format = img->format();
	if (lastHeight != imageHeight || lastWidth != imageWidth) {//��Ƶ��߷����仯Ҫ����ͼƬ
		if (img) {
			delete img;
			img = NULL;
		}
		lastHeight = imageHeight;
		lastWidth = imageWidth;
	}
	
	if (img == NULL)
	{
		if (buf != nullptr) {
			delete buf;
			buf = nullptr;
		}
		if (format == QImage::Format_ARGB32) {			
            buf = new uchar[imageWidth*imageHeight * 4];//��ɫ��Ƶ����
		    img = new QImage(buf, imageWidth , imageHeight, QImage::Format_ARGB32);
		}
		else {
			buf = new uchar[imageWidth*imageHeight];
			img = new QImage(buf, imageWidth, imageHeight, QImage::Format_Grayscale8);//�ڰ���Ƶ����
		}
		
	}
	AVFrame * frame;
	frame = Media::getInstance()->video->dequeueFrame(); //��ȡ�ѽ������Ƶ֡
	
	// ����Ƶͬ������Ƶ�ϣ�������һ֡���ӳ�ʱ��
	double current_pts = *(double*)frame->opaque;//��ȡ��ǰ֡��ʱ���
	double delay = current_pts - Media::getInstance()->video->getFrameLastPts();//��ǰ��Ƶ֡����һ֡��ʱ����
	if (delay <= 0 || delay >= 1.0)//������һ֡�쳣��Ƶ֡
		delay = Media::getInstance()->video->getFrameLastDelay();
	Media::getInstance()->video->setFrameLastDelay(delay);
	Media::getInstance()->video->setFrameLastPts(current_pts);
	// ��ǰ��ʾ֡��PTS��������ʾ��һ֡���ӳ�
	double master_clock = Media::getInstance()->audio->getAudioClock();//��ʱ���
	double diff = current_pts - master_clock;// diff < 0 => video slow,diff > 0 => video quick
	double threshold = (delay > SYNC_THRESHOLD) ? delay : SYNC_THRESHOLD;
	if (fabs(diff) < NOSYNC_THRESHOLD) // ��ͬ��
	{
		if (diff <= -threshold) // ���ˣ�delay��Ϊ0
			delay = 0;
		else if (diff >= threshold) // ���ˣ��ӱ�delay
			delay *= 2;
	}	 
	Media::getInstance()->video->setFrameTimer(Media::getInstance()->video->getFrameTimer() + delay) ;
 
	double actual_delay = Media::getInstance()->video->getFrameTimer() - static_cast<double>(av_gettime()) / 1000000.0;
	if (actual_delay <= 0.010)
		actual_delay = 0.010;
	timer->setInterval(static_cast<int>(actual_delay * 1000 + 0.5));
	AVCodecContext *videoCtx = Media::getInstance()->video->getVideoStream()->codec;
	img = ImageFilter::getInstance()->filterColor(img);//ɫ�ʹ���
	uint8_t *data[AV_NUM_DATA_POINTERS] = { 0 };
	data[0] = (uint8_t *)img->bits();
	int linesize[AV_NUM_DATA_POINTERS] = { 0 };
	
	if (img->format() == QImage::Format_Grayscale8) {//�Ҷ���Ƶ
		Media::getInstance()->video->swsContext = sws_getCachedContext(Media::getInstance()->video->swsContext, videoCtx->width,
			videoCtx->height,
			videoCtx->pix_fmt,
			imageWidth, imageHeight,
			AV_PIX_FMT_GRAY8,
			SWS_BICUBIC,
			NULL, NULL, NULL
		);
		
		linesize[0] = imageWidth;
	}
	else {//��ɫ��Ƶ
		Media::getInstance()->video->swsContext = sws_getCachedContext(Media::getInstance()->video->swsContext, videoCtx->width,
			videoCtx->height,
			videoCtx->pix_fmt,
			imageWidth, imageHeight,
			AV_PIX_FMT_BGRA,
			SWS_BICUBIC,
			NULL, NULL, NULL
		);

		linesize[0] = imageWidth * 4;
	}
	
    auto a = Media::getInstance()->video->swsContext;
    auto b = frame;
	int h = sws_scale(Media::getInstance()->video->swsContext, frame->data, frame->linesize, 0, videoCtx->height,
		data,
		linesize
	);//����ͼƬ
	img->setColorCount(1);
	img = ImageFilter::getInstance()->filter(img);//��Ƶ����������
	emit updateFrame(img);//�����źŵ�ui�̲߳�����Ƶ
	av_frame_unref(frame);
	av_frame_free(&frame);
	//mutex.unlock();
}

//************************************
// Method:    DisplayMediaTimer
// FullName:  DisplayMediaTimer::DisplayMediaTimer
// Access:    private 
// Returns:   
// Qualifier:������Ƶ��ʱ��
//************************************
DisplayMediaTimer::DisplayMediaTimer()
{
	buf = new uchar[imageWidth*imageHeight * 4];
	img = new QImage(buf, imageWidth, imageHeight, QImage::Format_ARGB32);
	timer = new QTimer;
	connect(timer, SIGNAL(timeout()), this, SLOT(synchronizeAudioAndVideo()));
	timer->start(20);
}

//************************************
// Method:    resetImage
// FullName:  DisplayMediaTimer::resetImage
// Access:    public 
// Returns:   void
// Qualifier:������Ƶ��С�����������С��ʱʹ��
// Parameter: int width ��
// Parameter: int height ��
//************************************
void DisplayMediaTimer::resetImage(int width, int height) {	 
	imageWidth = width;
	imageHeight = height;
}


DisplayMediaTimer::~DisplayMediaTimer()
{
	if(img)
	    delete img;
	if(buf)
	    delete buf;
	if(timer)
	    delete timer;
}

//************************************
// Method:    setPlay
// FullName:  DisplayMediaTimer::setPlay
// Access:    public 
// Returns:   void
// Qualifier: ���ò���״̬
// Parameter: bool isPlaying
//************************************
void DisplayMediaTimer::setPlay(bool isPlaying)
{
	this->isPlay = isPlaying;
}

//************************************
// Method:    isPlaying
// FullName:  DisplayMediaTimer::isPlaying
// Access:    public 
// Returns:   bool
// Qualifier: ��ȡ����״̬
//************************************
bool DisplayMediaTimer::isPlaying()
{
	return isPlay;
}
