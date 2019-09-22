#include "RbMoviePlayer.h"
#include "SYTabPageLibVideo.h"
#include "MxDefine.h"
#include "SYMainWindow.h"


SYTabPageLibVideo::SYTabPageLibVideo(QWidget * parent)
	: QWidget(parent),
	m_videoPlayer(nullptr)
{
	ui.setupUi(this);
	
	Mx::setWidgetStyle(this, "qss:SYTabPageLibVideo.qss");
}

SYTabPageLibVideo::~SYTabPageLibVideo(void)
{
	
}

void SYTabPageLibVideo::setVideos(const QVector<QString>& videoFiles)
{
	//create new controls
	QPoint offset(70, 40);
	QSize buttonSize(232, 142);
	int spacingX = 70;
	int spacingY = 86;
	int numOfVideo = videoFiles.size();

	if(numOfVideo > 8)
		numOfVideo = 8;

	if(m_videoButtons.size() == 0){
		for(int i = 0; i < 8; ++i){
			auto button = new QPushButton(this);
			button->resize(buttonSize);
			button->setObjectName("videoBtn");
			connect(button, &QPushButton::clicked, this, &SYTabPageLibVideo::onClieckedVideoBtn);
			m_videoButtons.push_back(button);
		}
	}

	for(auto button : m_videoButtons){
		button->hide();
	}

	bool finish = false;
	for(int row = 0; row < 2; ++row){
		for(int col = 0; col < 4; ++col){
			int index = row * 4 + col;
			if(index >= numOfVideo){
				finish = true;
				break;
			}

			const QString& fileName = videoFiles.at(index);
			auto* button = m_videoButtons[index];
			button->setProperty("videoFileName", fileName);
			//button->setText(content);
			button->move(col * (buttonSize.width() + spacingX) + offset.x(), row * (buttonSize.height() + spacingY) + offset.y());
			button->setVisible(true);
		}

		if(finish)
			break;
	}
}

void SYTabPageLibVideo::onClieckedVideoBtn()
{
	QPushButton* button = static_cast<QPushButton*>(sender());
	QString fileName = button->property("videoFileName").toString();

	if(m_videoPlayer == nullptr)
		m_videoPlayer = new RbMoviePlayer(SYMainWindow::GetInstance());

	m_videoPlayer->loadMoviePath(fileName);
	m_videoPlayer->Play();
	m_videoPlayer->show();
}