#include "videoPlayApp.h"
#include<qlayout.h>
#include<QBoxLayout>
#include<QVBoxLayout>
#include<qmessagebox.h>
#include<qfiledialog.h>



#define CHS(text) QString::fromLocal8Bit(text)

videoPlayApp::videoPlayApp(QWidget *parent)
	: QWidget(parent),
	filePath(""),
	playState(STOP),
	player(NULL),
	playerlist(NULL),
	videoWidget(NULL),
	videoName(NULL)
{
	//ui.setupUi(this);
	initialize();
}

//ҳ�沼�ֳ�ʼ��
void videoPlayApp::initialize()
{
	
	QVBoxLayout* vLayout = new QVBoxLayout();
	QHBoxLayout* h1Layout = new QHBoxLayout();
	videoName = new QLabel(CHS("���ڲ���-")+"XX.mp4");
	videoName->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
	QFont font;
	font.setPointSize(20);
	//font.
	videoName->setFont(font);
	h1Layout->addItem(new QSpacerItem(1, QSizePolicy::Expanding));
	h1Layout->addWidget(videoName);
	h1Layout->addItem(new QSpacerItem(1, QSizePolicy::Expanding));

	videoWidget = new QVideoWidget;
	videoWidget->setSizePolicy(QSizePolicy::Policy::Expanding,QSizePolicy::Policy::Expanding);

	QPushButton* openFile = new QPushButton("open");
	openFile->setFixedSize(QSize(150, 50));
	connect(openFile, &QPushButton::clicked, this, &videoPlayApp::openFileSlots);

	QPushButton* play = new QPushButton("play/stop");
	play->setFixedSize(QSize(150, 50));
	connect(play, &QPushButton::clicked, this, &videoPlayApp::playSlots);

	QPushButton* nextVideo = new QPushButton("next");
	nextVideo->setFixedSize(QSize(150, 50));
	connect(nextVideo, &QPushButton::clicked, this, &videoPlayApp::nextSlots);

	QPushButton* exit = new QPushButton("exit");
	exit->setFixedSize(QSize(150, 50));
	connect(exit, &QPushButton::clicked, this, &videoPlayApp::exitSlots);
	
	QHBoxLayout* hLayout = new QHBoxLayout();
	hLayout->addWidget(openFile);
	hLayout->addWidget(play);
	hLayout->addWidget(nextVideo);
	hLayout->addWidget(exit);
	//hLayout->setContentsMargins(30, 10, 10, 30);

	vLayout->addItem(new QSpacerItem(10,QSizePolicy::Fixed));
	vLayout->addLayout(h1Layout);
	vLayout->addWidget(videoWidget);
	vLayout->addItem(new QSpacerItem(10, QSizePolicy::Expanding));
	//vLayout->addSpacerItem(new QSpacerItem(1, QSizePolicy::Preferred));
	vLayout->addLayout(hLayout);
	//vLayout->setContentsMargins(0,0,0,0);
	//vLayout->setSpacing(0);
	this->setLayout(vLayout);

	player = new QMediaPlayer(this);
	playerlist = new QMediaPlaylist(this);
}


bool videoPlayApp::openFileSlots()
{
	//���ڲ���ʱ�ȹرղ�����
	if (playState == PlayState::PLAY)
	{
		player->pause();
		playState = PAUSE;
	}

	//��ò����ļ�����·��
	QStringList filePaths = QFileDialog::getOpenFileNames(NULL,CHS("��ѡ������Ƶ�ļ�"),"./","video/mp3 (*.*)");
	for (auto &path : filePaths)
	{
		QString videoName = "";
		int loc = path.lastIndexOf("/");
		videoName = path.mid(loc+1);
		//�б��ļ��Ƿ��Ѿ�����
		if (videoNameAndPath.find(videoName) == videoNameAndPath.end())
		{
			videoNameAndPath[videoName] = path;
			videoPathVector.push_back(path);
		}
	}
	//·���շ���
	if (filePaths.isEmpty())
	{
		return false;
	}
	QMessageBox::information(this, "", CHS("�ļ�����ɹ�!"), QMessageBox::Yes);
	return true;
}
//������Ƶ
void videoPlayApp::playSlots()
{
	//��һ�ε��
	if (videoPathVector.isEmpty())
	{
		QMessageBox::information(this, "", CHS("���ȵ��벥���ļ�!"), QMessageBox::Yes);
		return;
	}
	////���ڲ���ʱ����л�Ϊ��ͣ
	//if (playState == PlayState::PLAY)
	//{
	//	player->pause();
	//	playState = PAUSE;
	//	return;
	//}
	////�л�Ϊ����
	//else if (playState == PlayState::PAUSE)
	//{
	//	player->play();
	//	playState = PLAY;
	//	return;
	//}
	//else
	//{// stop״̬
	//	//����ղ����б����µ���
	//	playerlist->clear();
	//	for (auto& filePath : videoPathVector)
	//	{
	//		//��Ӳ�����Դ
	//		playerlist->addMedia(QUrl::fromLocalFile(filePath));
	//	}
	//	//���õ�ǰ����˳��
	//	playerlist->setCurrentIndex(currentIndex);
	//	//��ʾ��ǰ������Ϣ
	//	//int currentIndex = playerlist->currentIndex();
	//	if (currentIndex < videoPathVector.size())
	//	{
	//		QString currentFilePath = videoPathVector[currentIndex];
	//		int loc = currentFilePath.lastIndexOf("/");
	//		QString tmpVideoName = currentFilePath.mid(loc + 1);
	//		videoName->setText(CHS("���ڲ���-") + tmpVideoName);
	//	}
	//	//���ò���ģʽ
	//	playerlist->setPlaybackMode(QMediaPlaylist::PlaybackMode::CurrentItemInLoop);
	//	//���ò����б�
	//	player->setPlaylist(playerlist);
	//	//������Ƶ��ʾ����
	//	player->setVideoOutput(videoWidget);
	//	//��ʾ��Ƶ
	//	videoWidget->show();
	//	//������Ƶ
	//	player->play();
	//	playState = PLAY;
	//}

	switch (playState)
	{
		case(PAUSE):
		{
			playState = PLAY;
			player->play();
			break;
		}
		case(PLAY):
		{
			playState = PAUSE;
			player->pause();
			break;
		}
		case(STOP):
		{
			initVideoPlayInfo();
			break;
		}
		default:
			break;
	}
}


void videoPlayApp::initVideoPlayInfo()
{
	//����ղ����б����µ���
	//playerlist->clear();
	for (auto& filePath : videoPathVector)
	{
		//��Ӳ�����Դ
		playerlist->addMedia(QUrl::fromLocalFile(filePath));
	}
	//��ʾ��ǰ������Ϣ
	playerlist->setCurrentIndex(0);

	int currentIndex = playerlist->currentIndex();
	if (currentIndex < videoPathVector.size())
	{
		QString currentFilePath = videoPathVector[currentIndex];
		int loc = currentFilePath.lastIndexOf("/");
		QString tmpVideoName = currentFilePath.mid(loc + 1);
		videoName->setText(CHS("���ڲ���-") + tmpVideoName);
	}
	//���ò���ģʽ
	playerlist->setPlaybackMode(QMediaPlaylist::PlaybackMode::CurrentItemInLoop);
	//���ò����б�
	player->setPlaylist(playerlist);
	//������Ƶ��ʾ����
	player->setVideoOutput(videoWidget);
	//��ʾ��Ƶ
	videoWidget->show();
	//������Ƶ
	player->play();
	playState = PLAY;
}
//�˳�������
bool videoPlayApp::exitSlots()
{
	QMessageBox::StandardButton btn=QMessageBox::information(this, "", CHS("�Ƿ�ȷ���˳���"), QMessageBox::Yes | QMessageBox::No);
	if (btn == QMessageBox::Yes)
	{	
		//ֹͣ����
		player->stop();
		playState = STOP;
		this->close();
		return true;
	}
	return false;
}

//������һ����Դ
void videoPlayApp::nextSlots()
{
	if (playerlist->isEmpty())
	{
		QMessageBox::information(this, "", CHS("�뵼�벥���ļ�!"), QMessageBox::Yes);
		return;
	}
	player->stop();
	playState = STOP;
	//ȷ����ǰ�Ѿ�������Դ�ļ�
	int totalMediaNumber = playerlist->mediaCount();
	int currentIndex = playerlist->currentIndex();
	int nextIndex;
	//�б��Ƿ����һ����Դ
	if (currentIndex >= totalMediaNumber - 1)
	{
		nextIndex = 0;
		playerlist->setCurrentIndex(nextIndex);
	}
	else
	{
		nextIndex = currentIndex + 1;
		playerlist->setCurrentIndex(nextIndex);
	}
	//����video����
	if (nextIndex < videoPathVector.size())
	{
		QString currentFilePath = videoPathVector[nextIndex];
		int loc = currentFilePath.lastIndexOf("/");
		QString tmpVideoName = currentFilePath.mid(loc + 1);
		videoName->setText(CHS("���ڲ���-") + tmpVideoName);
	}
	player->play();
	playState = PLAY;
}