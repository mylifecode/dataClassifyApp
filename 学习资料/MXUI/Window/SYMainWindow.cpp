#include "SYMainWindow.h"
#include "MxGlobalConfig.h"
#include "MxDefine.h"
#include "RbAbout.h"
#include <QSound>
#include "SYMessageBox.h"
#include "DataStoreService.h"
#include "SYAdminPortalWindow.h"
#include "SYAdminPersonWindow.h"
#include "SYAdminSendTaskWindow.h"
#include "SYAdminTaskWindow.h"

#include "SYMonitorWindow.h"
#include "MxDemonstrationWindow.h"
#include "SYPersonCenterWindow.h"
#include "SYPersonTaskListsWindow.h"
#include "SYMessageCenterWindow.h"
#include "SYLoginWindow.h"
#include "SYTrainingCenterWindow.h"
#include "SYAdminTrainingCenterWindow.h"
#include "SYSurgeryTrainWindow.h"
#include "SYKnowLibWindow.h"
#include "SYSkillTrainingWindow.h"
#include "SYDataCenterWindow.h"
#include "SYAdminDataCenterWindow.h"
#include"SYAdminTheoryTestWindow.h"
#include "SYDBMgr.h"
#include "RbShutdownBox.h"
#include "SYStringTable.h"


SYMainWindow* SYMainWindow::m_instance = nullptr;

SYMainWindow::SYMainWindow(QWidget* parent)
	:QWidget(parent),
	m_shutdownWindow(nullptr),
	m_aboutWindow(nullptr),
	m_curWindowIndex(-1),
	m_demonstrationWindow(nullptr),
	m_timer(nullptr)
	
{
	m_ui.setupUi(this);

	m_ui.lb_maintitle->setText(SYStringTable::GetString(SYStringTable::STR_SYSTEMNAME));

	Initialize();

	qRegisterMetaType<WindowType>("WindowType");

	//timer
	m_timer = new QTimer(this);
	m_timer->setInterval(30 * 1000);
	connect(m_timer, &QTimer::timeout, this, &SYMainWindow::onTimer);

	Mx::setWidgetStyle(this, "qss:SYMainWindow.qss");
	
}


SYMainWindow::~SYMainWindow()
{
	if(m_demonstrationWindow)
		delete m_demonstrationWindow;
}

void SYMainWindow::CreateInstance()
{
	if(m_instance == nullptr)
		m_instance = new SYMainWindow;
}

void SYMainWindow::DestoryInstance()
{
	if(m_instance)
	{
		delete m_instance;
		m_instance = nullptr;
	}
}

void SYMainWindow::on_exitBtn_clicked()
{
	SYMessageBox * messageBox = new SYMessageBox(this, "", CHS("确定退出登录？"), 2);
	messageBox->showFullScreen();
	if(messageBox->exec() == 2)
	{
		ReturnToLoginWindow();
	}
}

void SYMainWindow::on_backBtn_clicked()
{
	bool canRemove = false;

	if(m_curWindowIndex == 1)
	{
		SYMessageBox * messageBox = new SYMessageBox(this, "", CHS("确定退出登录？"), 2);
		messageBox->showFullScreen();
		if(messageBox->exec() == 2)
		{
			SYUserInfo::Instance()->Logout();
			SYDBMgr::Instance()->Close();
			m_timer->stop();
			canRemove = true;
		}
	}
	else if(m_curWindowIndex > 1)
		canRemove = true;

	if(canRemove)
	{
		QWidget* removedWidget = m_ui.stackedWidget->widget(m_curWindowIndex);
		m_ui.stackedWidget->removeWidget(removedWidget);
	
		WindowType type = GetWindowType(removedWidget);
		if(type != WT_DemonstrationWindow)		//对于学生的示教窗口需要单独处理
			delete removedWidget;

		if(m_curWindowIndex == 1 && m_demonstrationWindow)	//返回到登录界面时才删除示教窗口，因为之前一直要监听教师机的消息
		{
			delete m_demonstrationWindow;
			m_demonstrationWindow = nullptr;
		}
		
		if(m_curWindowIndex == 1)
		{
			m_ui.titleBar->hide();
		}

		if(m_aboutWindow)
			m_aboutWindow->hide();

		--m_curWindowIndex;

		QWidget * backedWidget = m_ui.stackedWidget->widget(m_curWindowIndex);

		if (backedWidget)
		{
			connect(this, SIGNAL(BackToWidget(QWidget*)), backedWidget, SLOT(onBackToWidget(QWidget*)));

			emit BackToWidget(backedWidget);
			m_ui.stackedWidget->setCurrentIndex(m_curWindowIndex);
		}
	}
}


void SYMainWindow::on_shutdownBtn_clicked()
{
	//if(m_shutdownWindow == nullptr)
		//m_shutdownWindow = new RbShutdownBox(this);

	//m_shutdownWindow->show();

	SYMessageBox * messageBox = new SYMessageBox(this, "", CHS("退出系统并关闭电源？"), 2);
	
	messageBox->showFullScreen();
	
	if (messageBox->exec() == 2)
	{
		qApp->quit();
		STARTUPINFOA si = { 0 };
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		si.hStdError = (HANDLE)2;
		si.hStdInput = (HANDLE)0;
		si.hStdOutput = (HANDLE)1;
		//	si.wShowWindow = SW_HIDE;
		PROCESS_INFORMATION pi;
		CreateProcessA(
			0,
			(LPSTR)"C:/Windows/System32/shutdown.exe -s -t 0",// -t 0
			0,
			0,
			TRUE,
			0,
			0,
			0,
			&si, &pi);
	}
}

void SYMainWindow::on_aboutBtn_clicked()
{
	if(m_aboutWindow == nullptr)
	{
		m_aboutWindow = new RbAbout(this);
		m_aboutWindow->move((width() - m_aboutWindow->width()) / 2,
							(height() - m_aboutWindow->height()) / 2);
	}

	QSound::play(MxGlobalConfig::Instance()->GetAudioDir() + "/Button54.wav");
	
	m_aboutWindow->show();
}

void SYMainWindow::Initialize()
{
	//1 remove old widget from stacked widget
	std::vector<QWidget*> removedWidgets;
	int count = m_ui.stackedWidget->count();
	for(int i = 0; i < count;++i)
	{
		QWidget* widget = m_ui.stackedWidget->widget(i);
		removedWidgets.push_back(widget);
	}

	for(auto widget : removedWidgets)
	{
		m_ui.stackedWidget->removeWidget(widget);
		delete widget;
	}

	//2 set login window
	SYLoginWindow* widget = new SYLoginWindow;

	connect(widget, SIGNAL(showNextWindow(WindowType)), SLOT(onShowNextWindow(WindowType)));

	
	m_ui.stackedWidget->addWidget(widget);
	m_ui.stackedWidget->setCurrentIndex(0);
	m_curWindowIndex = 0;
	m_ui.titleBar->hide();
}

void SYMainWindow::mousePressEvent(QMouseEvent* event)
{
	if(m_aboutWindow && m_aboutWindow->isVisible())
	{
		QPoint globalPoint = event->globalPos();
		if(m_aboutWindow->contentContain(globalPoint) == false)
			m_aboutWindow->hide();
	}
}

int SYMainWindow::GetWindowDepthLevel(WindowType type)
{
	UserPermission permission = SYUserInfo::Instance()->GetUserPermission();

	switch (type)
	{
	case WT_LoginWindow:
		 return 0;
		 break;
	
	case WT_AdminPortalWindow:
		 return 1;
		 break;

	case WT_TrainingCenterWindow:
	case WT_AdminTrainingCenterWindow:
	case WT_AdminPersonWindow:
	case WT_AdminTaskWindow:
	case WT_AdminTheoryTestWindow:
		 return 2;
		 break;

	case WT_PersonCenterWindow:
	case WT_KnowLibWindow:
	case WT_SkillTrainingWindow:
	case WT_DataCenterWindow:
		 if (permission >= UP_Teacher)
			 return 2;
		 else
		     return 3;
		 break;

	case WT_AdminDataCenterWindow:
		return 2;

	case WT_PersonTasksWindow:
		 return 4;
	}

	return -1;//means unknow
}

void SYMainWindow::onShowNextWindow(WindowType type)
{
	QWidget* widget = nullptr;

	//create window
	switch(type)
	{
	//case WT_UserWindow:
		//widget = new RbUserWindow;
		//if(m_demonstrationWindow == nullptr)
		//{
		///	m_demonstrationWindow = new MxDemonstrationWindow;
		//	connect(m_demonstrationWindow, &MxDemonstrationWindow::ExitCurrentWindow, this, &SYMainWindow::onExitCurrentWindow);
		//}
		//break;
	case WT_TrainingCenterWindow:
		widget = new SYTrainingCenterWindow;
		break;
	case WT_AdminTrainingCenterWindow:
		widget = new SYAdminTrainingCenterWindow;
		break;
	case WT_SurgeyTrainWindow:
		widget = new SYSurgeryTrainWindow;
		break;
// 	case WT_WeChatLoginWindow:
// 	{
// 		WeChatLogin* wechatWindow = new WeChatLogin;
// 		QString scanUrl = MxGlobalConfig::Instance()->GetScanUrl();
// 		wechatWindow->SetUrl(scanUrl);
// 		widget = wechatWindow;
// 		break;
// 	}
	case WT_AdminPortalWindow:
	{
		SYAdminPortalWindow * adminWindow = new SYAdminPortalWindow;
		widget = adminWindow;
		break;
	}
	case WT_AdminPersonWindow:
		widget = new SYAdminPersonWindow;
		break;

	case WT_AdminTaskWindow:
		widget = new SYAdminTaskWindow;
		break;
	case WT_AdminSendTaskWindow:
		widget = new SYAdminSendTaskWindow;
		break;
	//case WT_AdminGroupMgrWindow:
	//{
	//	SYAdminGroudpMgrWindow * adminWindow = new SYAdminGroudpMgrWindow;
		//widget = adminWindow;
	//	break;
//	}

	case WT_AdminScoreWindow:
		//widget = new RbScoreWindow;
		break;
	case WT_AdminDataCenterWindow:
		widget = new SYAdminDataCenterWindow;
		break;
	case WT_AdminTheoryTestWindow:
		widget = new SYAdminTheoryTestWindow;		
		break;
	/*case WT_AdminQuestionsMgrWindow:
		widget = new SYAdminQuestionsMgrWindow;
		break;*/
	/*case WT_AdminKnowledgeSetManageWindow:
		widget = new SYAdminKnowledgeSetManageWindow;
		break;*/
	//case WT_ScoreBoardWebWindow:
	//	widget = new ScoreBoardWeb;
	//	break;
	//case WT_UserScoreWindow:
	//	widget = new RbUserScoreWindow;
	//	break;
	//case WT_NewDocWindow:
	//	//widget = new RbNewDocWindow;
		//break;
	//case WT_UserManageWindow:
		//widget = new RbAdminUserManage;
	//	break;
	case WT_MonitorWindow:
		widget = new SYMonitorWindow;
		break;
	case WT_DemonstrationWindow:
		//widget = new MxDemonstrationWindow;
		if(m_demonstrationWindow == nullptr)
			throw "no demonstration window.";
		widget = m_demonstrationWindow;
		break;
	case WT_PersonCenterWindow:
		widget = new SYPersonCenterWindow;
		break;
	case WT_MessageCenterWindow:
		widget = new SYMessageCenterWindow;
		break;

	case WT_KnowLibWindow:
		widget = new SYKnowLibWindow;
		break;
	case WT_SkillTrainingWindow:
		widget = new SYSkillTrainingWindow;
		break;
	case WT_DataCenterWindow:
		widget = new SYDataCenterWindow;
		break;
	case WT_PersonTasksWindow:
		widget = new SYPersonTaskListsWindow;
		break;
	case WT_None:
		break;
	}

	QWidget * oldWidget = m_ui.stackedWidget->currentWidget();
	WindowType oldType  = GetWindowType(oldWidget);

	int currDepLevel = GetWindowDepthLevel(type);
	int oldDepLevel  = GetWindowDepthLevel(oldType);

	//if current level <= remove the old one first
	if (currDepLevel >= 0 && oldDepLevel >= 0 && currDepLevel <= oldDepLevel)
	{
		bool canRemove = false;

		if (m_curWindowIndex > 0)
			canRemove = true;

		if (canRemove)
		{
			QWidget* removedWidget = m_ui.stackedWidget->widget(m_curWindowIndex);
			m_ui.stackedWidget->removeWidget(removedWidget);//移除当前窗口

			delete removedWidget;

			if (m_aboutWindow)
				m_aboutWindow->hide();

			--m_curWindowIndex;

			QWidget* backedWidget = m_ui.stackedWidget->widget(m_curWindowIndex);

			if (backedWidget)
			{
				m_ui.stackedWidget->setCurrentIndex(m_curWindowIndex); //设置当前显示窗口
			}
		}
	}
	//add new window
	if(widget)
	{
		m_curWindowIndex++;
		m_ui.stackedWidget->addWidget(widget);
		m_ui.stackedWidget->setCurrentWidget(widget);  //设置当前显示窗口
		if(m_curWindowIndex > 0)  //利用定时器记录登陆时间
		{
			m_ui.titleBar->show();
			if(m_curWindowIndex == 1)
				m_timer->start();
		}		
		connect(widget, SIGNAL(showNextWindow(WindowType)), SLOT(onShowNextWindow(WindowType)));
		SetWindowType(widget, type);
	}
}

void SYMainWindow::onExitCurrentWindow(WindowType type)
{
	on_backBtn_clicked();
}


void SYMainWindow::onTimer()
{
	SYUserInfo::Instance()->UpdateTotalOnlineTimeToDB();  //更新在线时间
}

void SYMainWindow::ReturnToLoginWindow()  //退出登录
{
	if(m_curWindowIndex < 0)
		return;

	while(m_curWindowIndex > 0)
	{
		QWidget* widget = m_ui.stackedWidget->widget(m_curWindowIndex);
		m_ui.stackedWidget->removeWidget(widget);
		delete widget;
		--m_curWindowIndex;
	}

	if(m_demonstrationWindow)
	{
		delete m_demonstrationWindow;
		m_demonstrationWindow = nullptr;
	}

	m_ui.titleBar->hide();
	SYUserInfo::Instance()->Logout();
	SYDBMgr::Instance()->Close();
	m_ui.stackedWidget->setCurrentIndex(0);
	m_timer->stop();
}

void SYMainWindow::SetWindowType(QWidget* widget,WindowType type)
{
	if(widget)
		widget->setProperty("WindowType", static_cast<int>(type));
}

WindowType SYMainWindow::GetWindowType(QWidget* widget)
{
	if(widget)
		return static_cast<WindowType>(widget->property("WindowType").toInt());
	else
		return WT_None;
}

void SYMainWindow::showEvent(QShowEvent *e)
{
	setAttribute(Qt::WA_Mapped);
	QWidget::showEvent(e);
}

