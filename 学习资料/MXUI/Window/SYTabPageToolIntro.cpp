#include "SYTabPageToolIntro.h"
#include <QMessageBox>
#include <QString>
#include "MxDefine.h"
#include "RbMoviePlayer.h"
#include "SYMainWindow.h"

SYTabPageToolIntro::SYTabPageToolIntro(QWidget * parent) 
	:QWidget(parent),
	m_moviePlayer(nullptr)
{
	ui.setupUi(this);
	
	setAttribute(Qt::WA_TranslucentBackground);

	Mx::setWidgetStyle(this, "qss:SYTabPageToolIntro.qss");
}

SYTabPageToolIntro::~SYTabPageToolIntro(void)
{
	
}

void SYTabPageToolIntro::setToolInfo(const QString& toolName, const QString& videoFileName, const QString& description)
{
	m_toolName = toolName;
	m_videoFileName = videoFileName;
	m_description;

	ui.toolNameLabel->setText(toolName);
	ui.textBrowser->setText(description);
}

void SYTabPageToolIntro::on_playBtn_clicked()
{
	if(m_videoFileName.size() == 0){
		QMessageBox::information(this, "", QString::fromLocal8Bit("ÔÝÎÞÊÓÆµ"));
		return;
	}
	
	if(m_moviePlayer == nullptr)
		m_moviePlayer = new RbMoviePlayer(SYMainWindow::GetInstance());

	//m_moviePlayer->showFullScreen();
	m_moviePlayer->SetTitle(m_toolName);
	m_moviePlayer->loadMoviePath(m_videoFileName);
	m_moviePlayer->Play();
	m_moviePlayer->show();
}