#pragma once
#include "MxDefine.h"
#include "SYMainFrameWindow.h"
#include <qevent.h>
#include "RbShutdownBox.h"
#include "RbAbout.h"
#include <qtimer.h>
#include <QKeyEvent>
#include <QFile.h>
#include <QSound>
#include "MxGlobalConfig.h"
#include "SYUserInfo.h"
#include "DataStoreService.h"
#include "SYMessageBox.h"

SYMainFrameWindow::SYMainFrameWindow(QWidget * parent, Qt::WindowFlags flag)
: QWidget(NULL,flag  |  Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::CustomizeWindowHint),
m_pParentWidget(parent)
{
	ui.setupUi(this);
	setAttribute(Qt::WA_AlwaysShowToolTips, true);
	
	setAttribute(Qt::WA_DeleteOnClose, true);
	
	setContextMenuPolicy (Qt::NoContextMenu);

	m_checkWindow = 0;
	m_pBgFrame = ui.MainFrame;
	m_pBgFrame->setWindowFlags(Qt::Widget | Qt::WindowStaysOnBottomHint);
	m_pBgFrame->setFrameShape(QFrame::NoFrame);

	connect(ui.exitBtn,SIGNAL(clicked()),this,SLOT(onClickedShutdownBtn()));
	connect(ui.aboutBtn,SIGNAL(clicked()),this,SLOT(onClickedAboutBtn()));
	connect(ui.backBtn,SIGNAL(clicked()),this,SLOT(onClickedBackBtn()));
	connect(ui.personBtn,SIGNAL(clicked()),this,SLOT(onClickedPerson()));

	ui.personBtn->hide();			//默认隐藏个人中心按钮
	m_pAbout = new RbAbout( this );
	m_pAbout->hide();

	Mx::setWidgetStyle(this,"qss:SYMainFrameWindow.qss");
}

SYMainFrameWindow::~SYMainFrameWindow(void)
{
	
}

bool SYMainFrameWindow::event(QEvent * e)
{
	switch (e->type())
	{
	case QEvent::Resize:
		{
			QResizeEvent *_e = static_cast<QResizeEvent*>(e);
			m_pBgFrame->setGeometry(0, 0, _e->size().width(), _e->size().height());
			m_pBgFrame->lower();

			m_pAbout->move((_e->size().width() - m_pAbout->size().width()) / 2,(_e->size().height() - m_pAbout->size().height()) / 2);
		}
		break;
	default:
		break;
	}
	return QWidget::event(e);
}

void SYMainFrameWindow::closeEvent(QCloseEvent *e)
{
	if(m_pParentWidget )
	{
		delete m_pParentWidget;
		m_pParentWidget = NULL;
	}
}

void SYMainFrameWindow::mousePressEvent(QMouseEvent *)
{
	hideabout();
}

void SYMainFrameWindow::showEvent(QShowEvent* e)
{
	setWindowTitle(QString::fromLocal8Bit("SimLap"));
	QWidget::showEvent(e);
 }

void SYMainFrameWindow::hideabout()
{
	m_pAbout->hide();
}

void SYMainFrameWindow::onClickedShutdownBtn()
{
	RbShutdownBox * pShutdownBox = new RbShutdownBox(this);
	pShutdownBox->show();
}

void SYMainFrameWindow::onClickedBackBtn()
{
	if(retCheckWindow() == 1 //判断是否是在学生或管理员的操作界面并按下返回按钮，弹出退出登录的确认弹窗
		&& !MxGlobalConfig::Instance()->IsVR())  //如果是虚实结合版本就不用弹框，因为还有选择虚实训练界面
	{
		SYMessageBox * messageBox = new SYMessageBox(this,"",CHS("确定退出登录？"),2);
		messageBox->showFullScreen();
		if(messageBox->exec() == 2)
		{
			const QString & userName = SYUserInfo::Instance()->GetUserName();

			QString strName = "13765815494";
			MX::logout(strName.toLocal8Bit().data());

			if(m_pParentWidget)
			{
				m_pParentWidget->show();
				//if(objectName() != QString("RbCameraWindow"))
					QTimer::singleShot( 100 , this , SLOT( deleteLater() ) );
				/*else
					QTimer::singleShot( 100 , this , SLOT( hide() ) );*/
			}
		}
	}
	else
	{
		if(m_pParentWidget)
		{
			m_pParentWidget->show();
			//if(objectName() != QString("RbCameraWindow"))
				QTimer::singleShot( 100 , this , SLOT( deleteLater() ) );
			/*else
				QTimer::singleShot( 100 , this , SLOT( hide() ) );*/
		}
	}
}

void SYMainFrameWindow::onClickedAboutBtn()
{
	if (m_pAbout == NULL)
	{
		m_pAbout = new RbAbout(this);
	}
	m_pAbout->show();
	QSound::play(MxGlobalConfig::Instance()->GetAudioDir() + "/Button54.wav" ); 
}

void SYMainFrameWindow::onClickedPerson()
{
	
}

QToolButton * SYMainFrameWindow::getBackToolBtn()
{
	return ui.backBtn;
}

void SYMainFrameWindow::fullShowAndHideParent()
{
	showFullScreen();
	if(m_pParentWidget)
	{
		QTimer::singleShot( 100 , m_pParentWidget , SLOT( hide() ) );
	}
}

QWidget * SYMainFrameWindow::getParent()
{
	return m_pParentWidget;
}

void SYMainFrameWindow::hidePersonBtn()
{
	ui.personBtn->hide();
}

void SYMainFrameWindow::showPersonBtn()
{
	ui.personBtn->show();
}

void SYMainFrameWindow::hidebackBtn()
{
	ui.backBtn->hide();
}
void SYMainFrameWindow::hidepowerBtn()
{
	ui.exitBtn->hide();
}
void SYMainFrameWindow::hideaboutBtn()
{
	ui.aboutBtn->hide();
}

