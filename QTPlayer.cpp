#include "QTPlayer.h"
#include "DisplayMediaTimer.h"
#include <QVBoxLayout>
#include<QFileDialog>
#include<QMessageBox>
#include "ImageFilter.h"
#include "Media.h"
#include "ReadPacketsThread.h"
#include <QContextMenuEvent>
#include <QInputDialog>
 
static bool isPressSlider = false;
static bool isPlay = false;
QTPlayer::QTPlayer(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	QObject::connect(DisplayMediaTimer::getInstance(),//�źŷ����Ķ���
		SIGNAL(updateFrame(QImage*)),//�ź�
		ui.openGLWidget,//�۽��յĶ���
		SLOT(setVideoImage(QImage*))//��
	);
	QObject::connect(this,//�źŷ����Ķ���
		SIGNAL(sendPos(float)),//�ź�
		ReadPacketsThread::getInstance(),//�۽��յĶ���
		SLOT(receivePos(float))//��
	);
	setMinimumWidth(400);
	setMinimumHeight(300);
	bottomAnimation = new QPropertyAnimation(ui.bottemWidget, "geometry");
	setMouseTracking(true);
	ui.openGLWidget->setMouseTracking(true);
	popMenu = new QMenu(this);
	leftRightMirrorAction = new QAction(this);
	upDownMirrorAction = new QAction(this);
	rgbAction = new QAction(this);
	grayAction = new QAction(this);
	netAddressAction = new QAction(this);
	leftRightMirrorAction->setText(QString::fromLocal8Bit("���Ҿ���"));
	upDownMirrorAction->setText(QString::fromLocal8Bit("���¾���"));
	rgbAction->setText(QString::fromLocal8Bit("��ɫ"));
	grayAction->setText(QString::fromLocal8Bit("�ڰ�"));
	netAddressAction->setText(QString::fromLocal8Bit("���紮��"));
	connect(leftRightMirrorAction, &QAction::triggered, this, &QTPlayer::mirrorLeftAndRight);
	connect(upDownMirrorAction, &QAction::triggered, this, &QTPlayer::mirrorUpAndDown);
	connect(rgbAction, &QAction::triggered, this, &QTPlayer::gray2Rgb);
	connect(grayAction, &QAction::triggered, this, &QTPlayer::rgb2Gray);
	connect(netAddressAction, &QAction::triggered, this, &QTPlayer::netAddressInput);
    connect(ui.playslider, &QSlider::valueChanged, this, &QTPlayer::sliderMoving);
	startTimer(40);
    //�������ʱ������������ģ����ֳ���
	ReadPacketsThread *readPacketsThread = ReadPacketsThread::getInstance();
	readPacketsThread->start();

	Media::getInstance()->video->start();
	play();
}

void QTPlayer::netAddressInput() {
	bool ok = FALSE;
 
	QString text = QInputDialog::getText( this,
		QString::fromLocal8Bit("������"),
		QString::fromLocal8Bit("�������ַ"),
		QLineEdit::Normal, QString::null, &ok);
	if (ok&&!text.isEmpty()) {
		openNetAddressVideo(text);
	}
		 
	else
		;// �û�����Cancel

}

void QTPlayer::openNetAddressVideo(QString address)
{
	std::string file = address.toLocal8Bit().data();//��ֹ������
	Media *media = Media::getInstance()
		->setMediaFile(file.c_str())
		->config(); 
	setWindowTitle(address);
	int total = Media::getInstance()->totalMs;
	ui.totalHour->setText(QString::number((int)(Media::getInstance()->totalMs / 1000 / 60 / 60)) + ":");//������Ƶ�ܵ�ʱ����
	ui.totalMinute->setText(QString::number((int)(Media::getInstance()->totalMs / 1000 / 60) % 60) + ":");
	ui.totalSecond->setText(QString::number((int)(Media::getInstance()->totalMs / 1000) % 60 % 60));
	isPlay = false;
	play();
	float pos = 0;
	pos = (float)ui.volumeSlider->value() / (float)(ui.volumeSlider->maximum() + 1);
	if (Media::getInstance()->getAVFormatContext())
		Media::getInstance()->audio->setVolume(pos*SDL_MIX_MAXVOLUME);//��ʼ������Ϊ���������һ��
}

//************************************
// Method:    contextMenuEvent
// FullName:  QTPlayer::contextMenuEvent
// Access:    public 
// Returns:   void
// Qualifier:�һ������˵�
// Parameter: QContextMenuEvent * event
//************************************
void QTPlayer::contextMenuEvent(QContextMenuEvent *event) {
	popMenu->clear();
	popMenu->addAction(leftRightMirrorAction);
	popMenu->addAction(upDownMirrorAction);
	popMenu->addAction(rgbAction);
	popMenu->addAction(grayAction);
	popMenu->addAction(netAddressAction);

	//�˵����ֵ�λ��Ϊ��ǰ����λ��
	popMenu->exec(QCursor::pos());
	event->accept();
}

//************************************
// Method:    mouseDoubleClickEvent
// FullName:  QTPlayer::mouseDoubleClickEvent
// Access:    public 
// Returns:   void
// Qualifier:���˫�������С��
// Parameter: QMouseEvent * event
//************************************
void QTPlayer::mouseDoubleClickEvent(QMouseEvent * event) {
	if (windowState() &  Qt::WindowFullScreen)
		showNormal();
	else
		showFullScreen();
}

//************************************
// Method:    mouseMoveEvent
// FullName:  QTPlayer::mouseMoveEvent
// Access:    public 
// Returns:   void
// Qualifier:���أ���ʾ�ײ�
// Parameter: QMouseEvent * event
//************************************
void QTPlayer::mouseMoveEvent(QMouseEvent *event) {
	int y = event->globalY();
	if (y >= height() - 100 || y <= 5) {
		showBottomInAnimation();
	}
	else {
		hideBottomInAnimation();
	}
}

//************************************
// Method:    openVideoFile
// FullName:  QTPlayer::openVideoFile
// Access:    public 
// Returns:   void
// Qualifier:ѡ�����Ƶ�ļ�
//************************************
void QTPlayer::openVideoFile() {
	QString fileName = QFileDialog::getOpenFileName(NULL, "please select your video file!");
	if (fileName.isEmpty()) {
		return;
	}
	QStringList titles = fileName.split("/");
	QString postfix = titles.constLast().split(".").last();
	if (postfix != QString::fromLocal8Bit("mp4") 
		&& postfix != QString::fromLocal8Bit("rmvb")
		&& postfix != QString::fromLocal8Bit("flv")
		&& postfix != QString::fromLocal8Bit("avi")
		&& postfix != QString::fromLocal8Bit("mkv")
        && postfix != QString::fromLocal8Bit("ts")
        && postfix != QString::fromLocal8Bit("mov")
		) {
		return;
	}
	std::string file = fileName.toLocal8Bit().data();//��ֹ������
	Media *media = Media::getInstance()
		->setMediaFile(file.c_str())
		->config();

	setWindowTitle(titles.constLast());
	int total =Media::getInstance()->totalMs;
	ui.totalHour->setText(QString::number((int)(Media::getInstance()->totalMs / 1000 / 60 / 60)) + ":");//������Ƶ�ܵ�ʱ����
	ui.totalMinute->setText(QString::number((int)(Media::getInstance()->totalMs / 1000 / 60) % 60) + ":");
	ui.totalSecond->setText(QString::number((int)(Media::getInstance()->totalMs / 1000) % 60 % 60));
	isPlay = false;
	play();
	float pos = 0;
	pos = (float)ui.volumeSlider->value() / (float)(ui.volumeSlider->maximum() + 1);
	if (Media::getInstance()->getAVFormatContext())
		Media::getInstance()->audio->setVolume(pos*SDL_MIX_MAXVOLUME);//��ʼ������Ϊ���������һ��
}

//************************************
// Method:    setVolume
// FullName:  QTPlayer::setVolume
// Access:    public 
// Returns:   void
// Qualifier: ����������С
// Parameter: int volume ������С0-128
//************************************
void QTPlayer::setVolume(int volume) {
	float pos = 0;
	pos = (float)ui.volumeSlider->value() / (float)(ui.volumeSlider->maximum() + 1);
	if (Media::getInstance()->getAVFormatContext())
	    Media::getInstance()->audio->setVolume(pos*SDL_MIX_MAXVOLUME);
}


void QTPlayer::sliderMoving(int pos)
{
/*    if(pos = )*/
}

//************************************
// Method:    showBottomInAnimation
// FullName:  QTPlayer::showBottomInAnimation
// Access:    private 
// Returns:   void
// Qualifier:�ײ���ʾ����Ч��
//************************************
void QTPlayer::showBottomInAnimation()
{
	if (ui.bottemWidget->y() == height()) {
		bottomAnimation->setDuration(500);
		bottomAnimation->setStartValue(QRect(ui.bottemWidget->x(), height(), ui.bottemWidget->width(), ui.bottemWidget->height()));

		bottomAnimation->setEndValue(QRect(ui.bottemWidget->x(), this->height() - ui.bottemWidget->height(), ui.bottemWidget->width(), ui.bottemWidget->height()));
		bottomAnimation->start();
	}
}

//************************************
// Method:    hideBottomInAnimation
// FullName:  QTPlayer::hideBottomInAnimation
// Access:    private 
// Returns:   void
// Qualifier:�ײ����ض���Ч��
//************************************
void QTPlayer::hideBottomInAnimation()
{
	if (ui.bottemWidget->y() == this->height() - ui.bottemWidget->height()) {
		bottomAnimation->setDuration(500);
 		bottomAnimation->setStartValue(QRect(ui.bottemWidget->x(), this->height() - ui.bottemWidget->height(), ui.bottemWidget->width(), ui.bottemWidget->height()));

		bottomAnimation->setEndValue(QRect(ui.bottemWidget->x(), this->height(), ui.bottemWidget->width(), ui.bottemWidget->height()));
		bottomAnimation->start();
	}
}

//************************************
// Method:    timerEvent
// FullName:  QTPlayer::timerEvent
// Access:    public 
// Returns:   void
// Qualifier:���ý������Ͳ���ʱ��
// Parameter: QTimerEvent * e
//************************************
void QTPlayer::timerEvent(QTimerEvent * e)
{
    double pts = (double)Media::getInstance()->pts * 1000;
    double total = (double)Media::getInstance()->totalMs;
    if (pts / 1000 >= total)
    {
        if (Media::getInstance()->getAVFormatContext())
            Media::getInstance()->close();
        return;
    }
	if (Media::getInstance()->totalMs > 0)
	{
		double rate = pts / total;
		if (!isPressSlider&&isPlay) {
            ui.playslider->setValue(rate);
		    ui.currentHour->setText(QString::number((int)(pts / 1000000 / 60 / 60))+":");
		    ui.currentMinute->setText(QString::number((int)(pts / 1000000 / 60) % 60) + ":");
	     	ui.currentSecond->setText(QString::number((int)(pts / 1000000) % 60 % 60));
		}
			
	}
}

//************************************
// Method:    sliderPress
// FullName:  QTPlayer::sliderPress
// Access:    public 
// Returns:   void
// Qualifier: ���½�����
//************************************
void QTPlayer::sliderPress()
{
	isPressSlider = true;
}

//************************************
// Method:    sliderRelease
// FullName:  QTPlayer::sliderRelease
// Access:    public 
// Returns:   void
// Qualifier: �������ͷ�
//************************************
void QTPlayer::sliderRelease()
{
	isPressSlider = false;
	float pos = 0;
	pos = (float)ui.playslider->value() / (float)(ui.playslider->maximum() + 1);
	sendPos(pos);
}

//************************************
// Method:    play
// FullName:  QTPlayer::play
// Access:    public 
// Returns:   void
// Qualifier:��Ƶ����
//************************************
void QTPlayer::play() {
	const QString PAUSE = "QPushButton#playButton{border-image:url(:/QTPlayer/Resources/PauseP.png);}";

	const QString PLAY = "QPushButton#playButton{border-image:url(:/QTPlayer/Resources/PlayP.png);}";
	isPlay = !isPlay;
	DisplayMediaTimer::getInstance()->setPlay(isPlay);
	if (isPlay)
	{
		ui.bottemWidget->setStyleSheet(PAUSE);
		if(Media::getInstance()->getAVFormatContext())
		    Media::getInstance()->audio->audioPlay();
	}
	else
	{
		ui.bottemWidget->setStyleSheet(PLAY);
		if (Media::getInstance()->getAVFormatContext())
		    SDL_CloseAudio();
	}
}

//************************************
// Method:    mirrorUpAndDown
// FullName:  QTPlayer::mirrorUpAndDown
// Access:    public 
// Returns:   void
// Qualifier:���¾������
//************************************
void QTPlayer::mirrorUpAndDown() {
	if (Media::getInstance()->getAVFormatContext() && isPlay)
		ImageFilter::getInstance()->addTask(XTask{ XTASK_MIRRORUPANDDOWN });
}

//************************************
// Method:    mirrorLeftAndRight
// FullName:  QTPlayer::mirrorLeftAndRight
// Access:    public 
// Returns:   void
// Qualifier:���Ҿ������
//************************************
void QTPlayer::mirrorLeftAndRight() {
	if (Media::getInstance()->getAVFormatContext() && isPlay)
	    ImageFilter::getInstance()->addTask(XTask{ XTASK_MIRRORLEFTANDRIGHT });
}

//************************************
// Method:    gray2Rgb
// FullName:  QTPlayer::gray2Rgb
// Access:    public 
// Returns:   void
// Qualifier: �Ҷ�ͼתrgb
//************************************
void QTPlayer::gray2Rgb()
{
	if (Media::getInstance()->getAVFormatContext() && isPlay)
	    ImageFilter::getInstance()->addColorTask(ColorTask{ COLRTASK_GRAY_TO_RGBA });
}

//************************************
// Method:    rgb2Gray
// FullName:  QTPlayer::rgb2Gray
// Access:    public 
// Returns:   void
// Qualifier:rgbͼת�Ҷ�ͼ
//************************************
void QTPlayer::rgb2Gray()
{
	if (Media::getInstance()->getAVFormatContext() && isPlay) {
		for(int i=0;i<5;i++)
        ImageFilter::getInstance()->addColorTask(ColorTask{ COLRTASK_RGBA_TO_GRAY });
	}
	   
}

//************************************
// Method:    resizeEvent
// FullName:  QTPlayer::resizeEvent
// Access:    public 
// Returns:   void
// Qualifier: ����ı��Сʱ���������пؼ����Բ��Ű�ťΪ��׼
// Parameter: QResizeEvent * e
//************************************
void QTPlayer::resizeEvent(QResizeEvent *e) {
	ui.openGLWidget->resize(size());
	ui.bottemWidget->resize(this->width(), ui.bottemWidget->height());
	ui.bottemWidget->move(0,this->height() - ui.bottemWidget->height());

	ui.playButton->move((ui.bottemWidget->width()- ui.playButton->width())/2, (ui.bottemWidget->height() - ui.playButton->height()) / 2);
	ui.openButton->move(ui.playButton->x()+ ui.playButton->width()+30, ui.playButton->y());

	ui.volumeSlider->move(this->width()-ui.volumeSlider->width()-25, ui.bottemWidget->height() - ui.volumeSlider->height()-20);
	ui.playslider->move(25, ui.playButton->y()-20);
	ui.playslider->resize(ui.bottemWidget->width() - 50, ui.playslider->height());

	ui.totalLayout->setSpacing(0);
	ui.totalContainer->setLayout(ui.totalLayout);
	ui.totalContainer->move(ui.playslider->x() + ui.playslider->width() - ui.totalContainer->width()-10, ui.playslider->y()+10);

	ui.currentLayout->setSpacing(0);
	ui.currentContainer->setLayout(ui.currentLayout);
	ui.currentContainer->resize(ui.totalContainer->size());
	ui.currentContainer->move(ui.playslider->x()+10, ui.playslider->y() + 10);
	DisplayMediaTimer::getInstance()->resetImage(width(), height());
}