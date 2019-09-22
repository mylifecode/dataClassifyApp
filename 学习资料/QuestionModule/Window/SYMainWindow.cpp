#include "SYMainWindow.h"
#include "MxGlobalConfig.h"
#include "MxDefine.h"
#include "SYQuestionExamListWindow.h"
#include "SYExamWindow.h"
#include "SYExamResultWindow.h"
#include "MxQuestionModuleApp.h"
#include <QSound>
#include <RbAbout.h>
#include "SYStringTable.h"
#include "SYMessageBox.h"
SYMainWindow* SYMainWindow::m_instance = nullptr;

SYMainWindow::SYMainWindow(QWidget* parent)
	:QWidget(parent),
	m_shutdownWindow(nullptr),
	m_aboutWindow(nullptr),
	m_curWindowIndex(-1)
{
	m_ui.setupUi(this);
	
	m_ui.lb_maintitle->setText(SYStringTable::GetString(SYStringTable::STR_SYSTEMNAME));

	Initialize();

	qRegisterMetaType<WindowType>("WindowType");

	Mx::setWidgetStyle(this, "qss:SYMainWindow.qss");
}


SYMainWindow::~SYMainWindow()
{
	
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
	on_backBtn_clicked();
}

void SYMainWindow::on_backBtn_clicked()
{
	bool canRemove = false;

	if(m_curWindowIndex > 0)
       canRemove = true;

	if(canRemove)
	{
		QWidget* removedWidget = m_ui.stackedWidget->widget(m_curWindowIndex);
		m_ui.stackedWidget->removeWidget(removedWidget);

		delete removedWidget;

		if(m_aboutWindow)
			m_aboutWindow->hide();

		--m_curWindowIndex;   
		
		QWidget* backedWidget = m_ui.stackedWidget->widget(m_curWindowIndex);

		if (backedWidget)
		{
			connect(this, SIGNAL(BackToWidget(QWidget*)), backedWidget, SLOT(onBackToWidget(QWidget*)));

			emit BackToWidget(backedWidget);
			m_ui.stackedWidget->setCurrentIndex(m_curWindowIndex);
		}
	}
	else
	{
		//QTimer::singleShot(10, this, SLOT(close()));
		static_cast<MxQuestionModuleApp*>(qApp)->exitModuleAndShowParentWindow(-1);  //进程通信
	}
}

void SYMainWindow::on_shutdownBtn_clicked()
{
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
	//if(m_aboutWindow == nullptr)
	//{
		//m_aboutWindow = new RbAbout(this);
		//m_aboutWindow->move((width() - m_aboutWindow->width()) / 2,
	//						(height() - m_aboutWindow->height()) / 2);
	//}//

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
	SYQuestionExamListWindow* widget = new SYQuestionExamListWindow();

	connect(widget, SIGNAL(showNextWindow(WindowType)), SLOT(onShowNextWindow(WindowType)));

	connect(this, SIGNAL(BackToWidget(QWidget*)), widget, SLOT(onBackToWidget(QWidget*)));


	m_ui.stackedWidget->addWidget(widget);
	m_ui.stackedWidget->setCurrentIndex(0);
	m_curWindowIndex = 0;
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

void SYMainWindow::onShowNextWindow(WindowType type)
{
	QWidget* widget = nullptr;

	//create window
	switch(type)
	{
	case WT_ExamResultWindow:
		 widget = new SYQuestionExamResultWindow;
		 break;
	case WT_ExamDoWindow:
		 widget = new SYQuestionDoExamWindow;
		 //((SYQuestionDoExamWindow*)widget)->GetExamQuestionFromPaper(-1);//temp
		 break;
	}

	//add window
	if(widget)
	{
		//
		QWidget * oldWidget = m_ui.stackedWidget->currentWidget();
		disconnect(this, SIGNAL(BackToWidget(QWidget*)), oldWidget, SLOT(onBackToWidget(QWidget*)));
		//
		m_curWindowIndex++;
		m_ui.stackedWidget->addWidget(widget);
		m_ui.stackedWidget->setCurrentWidget(widget);
		if(m_curWindowIndex > 0)
		{
			m_ui.titleBar->show();
		}
			
		connect(widget, SIGNAL(showNextWindow(WindowType)), SLOT(onShowNextWindow(WindowType)));
		connect(widget, SIGNAL(ReplaceCurrentWindow(WindowType)), SLOT(onReplaceCurrentWindow(WindowType)));
		connect(widget, SIGNAL(ExitCurrentWindow()), SLOT(onExitCurrentWindow()));

		SetWindowType(widget, type);
	}
}

void SYMainWindow::onReplaceCurrentWindow(WindowType type)
{
	bool canRemove = false;

	if (m_curWindowIndex > 0)
		canRemove = true;

	if (canRemove)
	{
		QWidget* removedWidget = m_ui.stackedWidget->widget(m_curWindowIndex);
		m_ui.stackedWidget->removeWidget(removedWidget);

		delete removedWidget;

		if (m_aboutWindow)
			m_aboutWindow->hide();

		--m_curWindowIndex;

		QWidget* backedWidget = m_ui.stackedWidget->widget(m_curWindowIndex);

		if (backedWidget)
		{
			m_ui.stackedWidget->setCurrentIndex(m_curWindowIndex);
		}
		onShowNextWindow(type);
	}
	else
	{
		
	}
	
}

void SYMainWindow::onExitCurrentWindow()
{
	on_backBtn_clicked();
}

void SYMainWindow::ReturnToLoginWindow()
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


	m_ui.titleBar->hide();
	//SYUserInfo::Instance()->Logout();
//	SYDBMgr::Instance()->Close();
	m_ui.stackedWidget->setCurrentIndex(0);
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