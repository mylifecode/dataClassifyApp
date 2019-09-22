#include "AsWindow.h"
#include <QProcess>
#include <QResizeEvent>
#include <QLayout>
#include <QDesktopWidget>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>
#include "RbAbout.h"
#include "MxDefine.h"

#define QUIT_ON_SHUTDOWN   1
#define HIDE_WINDOW_TIME   100


AsWindow::AsWindow(QWidget *parent, Qt::WindowFlags flags) :
QDialog(parent, flags | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::CustomizeWindowHint)
,m_parent(NULL)
{ 
	ui.setupUi(this);
	setWindowTitle(QString::fromLocal8Bit("Sim-Laparo 腹腔镜手术模拟系统"));

	m_bg = ui.abBackgroundFrame;
	m_bg->setWindowFlags(Qt::Widget | Qt::WindowStaysOnBottomHint);	
    ui.topFrame->setWindowFlags(Qt::WindowStaysOnTopHint);

	setAttribute(Qt::WA_AlwaysShowToolTips, true);

	setContextMenuPolicy (Qt::NoContextMenu);

	connect( ui.exitBtn , SIGNAL( clicked() ) , this, SLOT( on_exitBtnClicked() ) );
	connect( ui.aboutBtn , SIGNAL( clicked() ) , this, SLOT( on_aboutBtnClicked() ) );
	connect( ui.retBackBtn , SIGNAL( clicked() ) , this, SLOT( on_retBtnClicked() ) );
	connect(ui.msgBtn,SIGNAL(clicked()),this,SLOT(on_msgBtnClicked()));
	connect(ui.perCenterBtn,SIGNAL(clicked()),this,SLOT(on_perCenterClicked()));


    ui.msgBtn->hide();

	installEventFilter(this);

	Mx::setWidgetStyle(this,"qss:AsWindow.qss");
} 

AsWindow::~AsWindow()
{
}
void AsWindow::CloseMe(int delay)
{
	if (m_parent)
	{
		m_parent->show();
	}
	QTimer::singleShot(/*10*/delay,this,SLOT(close()));
}
void AsWindow::on_retBtnClicked()
{
	CloseMe(100);
}

void AsWindow::on_exitBtnClicked()
{

//     SYMessageBox* pQuitMissionMsgBox = new SYMessageBox( this , QString::fromLocal8Bit("退出系统并关闭电源？") , QDialogButtonBox::Yes | QDialogButtonBox::No , QDialogButtonBox::Yes );
// 
//     pQuitMissionMsgBox->setStandardButtonText(QDialogButtonBox::Yes , QString::fromLocal8Bit("确定"));
//     pQuitMissionMsgBox->setStandardButtonText(QDialogButtonBox::No , QString::fromLocal8Bit("取消"));
//     pQuitMissionMsgBox->setWindowTitle( QString("") );
//     int returntag = pQuitMissionMsgBox->exec();
// 	
// 	if (returntag == QDialogButtonBox::Yes)
// 	{
// 		qApp->quit();
// 		if ( QUIT_ON_SHUTDOWN )
// 		{
// 			STARTUPINFOA si = {0};
// 			si.cb = sizeof(si);
// 			si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
// 			si.hStdError = (HANDLE)2;
// 			si.hStdInput = (HANDLE)0;
// 			si.hStdOutput = (HANDLE)1;
// 			//	si.wShowWindow = SW_HIDE;
// 			PROCESS_INFORMATION pi;
// 			CreateProcessA(
// 				0,
// 				(LPSTR)"C:/Windows/System32/shutdown.exe -s -t 0",// -t 0
// 				0,
// 				0,
// 				TRUE,
// 				0,
// 				0,
// 				0,
// 				&si, &pi);
// 		}
// 	}
}

void AsWindow::on_aboutBtnClicked()
{
//     BlackWidget* bb = new BlackWidget(this);
//     bb->show();
	RbAbout * pAbout = new RbAbout( this );
    pAbout->show();
}  

void AsWindow::on_msgBtnClicked()
{

}

void AsWindow::on_perCenterClicked()
{

}
bool AsWindow::event( QEvent *e )
{
	switch (e->type())
	{
	case QEvent::Resize:
		{
			QResizeEvent *_e = static_cast<QResizeEvent*>(e);
			m_bg->setGeometry(0, 0, _e->size().width(), _e->size().height());
			m_bg->lower();
		}
		break;
	default:
		break;
	}
	return QDialog::event(e);
}

bool AsWindow::eventFilter(QObject *watched, QEvent *event)
{
	if (event->type() == QEvent::KeyPress) 
	{  
	     QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);  
	     if(keyEvent->key() == Qt::Key_Escape)  
		 {
			 return true;  
		 }
	}
	return __super::eventFilter(watched,event);
}

QToolButton *AsWindow::getRetBtn()
{
	return ui.retBackBtn;
}

QToolButton *AsWindow::getAboutBtn()
{
	return ui.aboutBtn;
}
QToolButton *AsWindow::getExitBtn()
{
	return ui.exitBtn;
}

QToolButton *AsWindow::getMsgBtn()
{

	return  ui.msgBtn;
}
QToolButton *AsWindow::getPerCenterBtn()
{

	return ui.perCenterBtn;
}

void AsWindow::closeEvent( QCloseEvent * )
{
    deleteLater();
}

void AsWindow::showOnHideParent( QWidget * pWidget )
{
	showFullScreen();
	if (pWidget)
	{
		QTimer::singleShot( HIDE_WINDOW_TIME , pWidget , SLOT( hide() ) );
	}
}

void AsWindow::onClickedtipBtn()
{
}