#include "ToolsMgr.h"
#include "ScreenEffect.h"
#include "BasicTraining.h"
#include "KeyboardInput.h"
#include "InputSystem.h"
#include "MXApplication.h"
#include "RbMoviePlayer.h"
#include "MXOgreWrapper.h"
#include "SYTrainWindow.h"
#include "RbPhantomBoxTest.h"
#include<QPair>
#include "Rbadjustgyrotool.h"
#include "XMLWrapperTraining.h"
#include "EffectManager.h"
#include "SYLoading.h"
#include "Inception.h"
#include "../MXRobot/Include/stdafx.h"

#include "NewTrain/PhysicsWrapper.h"
#include "NewTrain/MisNewTraining.h"


#include "SYMessageDialog.h"
#include "SYDBMgr.h"
#include <QBitmap>
#include <QPainter>
#include "RbScreenRecorder.h" //bacon add
#include "SYUserInfo.h"
#include "MXGlobalConfig.h"
#include "CommonDataStruct.h"
#include "RbTrainingEvaluateWindow.h"
#include "SYTrainingReport.h"
#include "MxTrainingDebugWindow.h"
#include "RbFixedToolsMenu.h"
#include "MxScreenDataSender.h"
#include "RbConnectStatusWindow.h"
#include "DataStoreService.h"
#include "SYScoreTable.h"
#include "SYTrainRecordDetailWindow.h"
#include "SYStringTable.h"
#include"SYMessageBox.h"
#include"TrainModuleConfig.h"
#include"TrainSkillImproveTipWindow.h"

#define USE_PIX_GRAPHICTOOL     0	//modify this to 1 if you want graphic tools like pix to be use,because Phonon::VideoPlayer also use DX9 for play moive thus make error in dx when pix is attached
#define AUTO_SCREEN_RECORDER	1
#define USE_NEWRECORDER         0
QEvent::Type loadfinishEventType;// = (QEvent::Type)QEvent::registerEventType(QEvent::User + 100);

QWidget* SYTrainWindow::s_widgetSelf = NULL;

SYTrainWindow::SYTrainWindow(CaseInfoParam& params, QWidget *parent) : QWidget(parent, Qt::FramelessWindowHint)
,m_bFirstShow(false)
,m_bTerminated(false)
//,m_remainTime(3600)
,mDeviceUpdateThread(NULL)
,m_pScreenRecorder(NULL)
,m_caseParam(params)
//,m_nCameraTotalSec(0)
,m_bMissionTerminate( false )
,m_bFlagForLock(false)
,m_bFinished( false )
,m_TimeFromTrainActive(-1)
,m_TimeVideoRecord(-1.0f)
,m_trainingDebugWindow(NULL)
,m_pDlgMoviePlayer(NULL)
,m_toolMenu_l(NULL)
,m_toolMenu_r(NULL)
,m_pFixedToolsMenu(NULL)
,m_phantomboxdebug(NULL)
,m_LoadingUI(0)
,m_pDemonstrationBeginBtn(NULL)
,m_pDemonstrationStopBtn(NULL)
,m_pConnectStatusWindow(NULL)
,m_TimerTrain(NULL)
,m_adjustGyro(NULL)
,m_CameraPullOutImg(0)
,m_screenDataSender(NULL)
,m_isAdministrator(false)
{
	Sleep(5000);
	setAttribute(Qt::WA_DeleteOnClose);

	loadfinishEventType = (QEvent::Type)QEvent::registerEventType(QEvent::User + 100);

	Inception::Instance()->SetLeftPedalState(true);
	Inception::Instance()->SetRightPedalState(true);

	Inception::Instance()->m_bCenterWindow=false;
	Inception::Instance()->m_strTrainingName = params.trainEnName.toStdString();
	Inception::Instance()->m_fPercent=0.0;

	//set window
	ui.setupUi( this );

	HideProgressRegion();

	connect(Inception::Instance(), SIGNAL(ShowTrainProgressBar()), this, SLOT(ShowProgressRegion()));

	connect(Inception::Instance(), SIGNAL(ShowTrainCompletness(int , int)), this, SLOT(ShowTrainCompletness(int, int)));

	//m_parent = parent;
	
	//getPerCenterBtn()->hide();
	//getMsgBtn()->hide();
	//getAboutBtn()->hide();
	//getExitBtn()->hide();

	//m_bg->setObjectName(QString::fromUtf8("caseBackgroundFrame"));

	//QDesktopWidget *desktop = QApplication::desktop();
	//int screenIndex = desktop->primaryScreen();
	//QRect dw = desktop->screenGeometry(screenIndex);
	//ui.bigGLWidget->resize(1024,768);
	//resize(dw.width(), dw.height());
	//Initialize Scene
	InitializeScene();

	
	//获取当前用户权限
	if(SYUserInfo::Instance()->IsStudentPermission() == false && SYUserInfo::Instance()->IsVisitor() == false)
	{
		m_isAdministrator = true;

		int serverPort = MxGlobalConfig::Instance()->GetServerPort();
		int clientPort = MxGlobalConfig::Instance()->GetClientPort();
		m_screenDataSender = new MxScreenDataSender(GetDesktopWindow(), 1024, 768, this);
		//befor initialize
		m_screenDataSender->SetSessionPort(serverPort);
		//initialze
		m_screenDataSender->Initialize();
		//after initialize
		if(MxGlobalConfig::Instance()->MulticastMode())
		{
			m_screenDataSender->JoinMulticastGroup(MxGlobalConfig::Instance()->GetMulticastGroupAddress(), clientPort);
			m_screenDataSender->AddDestination(MxGlobalConfig::Instance()->GetMulticastGroupAddress(), clientPort);
		}

		connect(m_screenDataSender, &MxScreenDataSender::ReceiveCommand, this, &SYTrainWindow::onReceiveSessionCommand);
	}
	else
		m_isAdministrator = false;
	
	//
	Mx::setWidgetStyle(this,"qss:SYTrainWindow.qss");
	

	//设置背景色
	QPalette pal(this->palette());
	QColor bgcolor(0x1B, 0x20, 0x2A);
	pal.setColor(QPalette::Background, bgcolor);
	this->setAutoFillBackground(true);
	this->setPalette(pal);
	
	s_widgetSelf = this;

	m_ModuleExisted = false;

	m_InRecordNew = false;
}


SYTrainWindow::~SYTrainWindow()
{
	if (m_LoadingUI)
	{
		delete m_LoadingUI;
		m_LoadingUI = 0;
	}
	if(m_bMissionTerminate == false)//not terminated
	{
		//MessageBoxA(0 , "RBCaseOperation Widget Not Terminate clearly","Warning",0);
	}

	if(m_screenDataSender)
	{
		m_screenDataSender->SendCommand(MxSessionCommand::CT_StopDemonstration);
		delete m_screenDataSender;
	}

	s_widgetSelf = NULL;

}

void SYTrainWindow::HideProgressRegion()   //hide train progress
{
	ui.label_prog->hide();
	ui.label_2->hide();
	ui.lb_prognum->hide();
	ui.lb_p0->hide();
	ui.lb_p1->hide();
	ui.lb_p2->hide();
	ui.lb_p3->hide();
	ui.lb_p4->hide();

}
void SYTrainWindow::ShowProgressRegion()
{
	ui.label_prog->show();
	ui.label_2->show();
	ui.lb_prognum->show();
	ui.lb_p0->show();
	ui.lb_p1->show();
	ui.lb_p2->show();
	ui.lb_p3->show();
	ui.lb_p4->show();

	//ShowProgressNum(3);
}

void SYTrainWindow::ShowTrainCompletness(int curr , int total)  //Training completion schedule setting
{
	int num = curr;

	QString png1 = MxGlobalConfig::Instance()->GetSkinDir() + "/sytrainwindow/progress_bar1.png";
	QString png2 = MxGlobalConfig::Instance()->GetSkinDir() + "/sytrainwindow/progress_bar4.png";
	QString png3 = MxGlobalConfig::Instance()->GetSkinDir() + "/sytrainwindow/progress_bar5.png";
	QString png4 = MxGlobalConfig::Instance()->GetSkinDir() + "/sytrainwindow/progress_bar6.png";
	QString png5 = MxGlobalConfig::Instance()->GetSkinDir() + "/sytrainwindow/progress_bar7.png";

	QString pngn0 = MxGlobalConfig::Instance()->GetSkinDir() + "/sytrainwindow/progress_bar_nothing_left.png";
	QString pngn1 = MxGlobalConfig::Instance()->GetSkinDir() + "/sytrainwindow/progress_bar_nothing_centre.png";
	QString pngn2 = MxGlobalConfig::Instance()->GetSkinDir() + "/sytrainwindow/progress_bar_nothing_right.png";

	if (num >= 1)
		ui.lb_p0->setStyleSheet(QString("border-image:url(%1); background-color:none;").arg(png1));
	else
		ui.lb_p0->setStyleSheet(QString("border-image:url(%1); background-color:none;").arg(pngn0));
	
	if (num >= 2)
		ui.lb_p1->setStyleSheet(QString("border-image:url(%1); background-color:none;").arg(png2));
	else
		ui.lb_p1->setStyleSheet(QString("border-image:url(%1); background-color:none;").arg(pngn1));

	if (num >= 3)
		ui.lb_p2->setStyleSheet(QString("border-image:url(%1); background-color:none;").arg(png3));
	else
		ui.lb_p2->setStyleSheet(QString("border-image:url(%1); background-color:none;").arg(pngn1));

	if (num >= 4)
		ui.lb_p3->setStyleSheet(QString("border-image:url(%1); background-color:none;").arg(png4));
	else
		ui.lb_p3->setStyleSheet(QString("border-image:url(%1); background-color:none;").arg(pngn1));

	if (num >= total)
		ui.lb_p4->setStyleSheet(QString("border-image:url(%1); background-color:none;").arg(png5));
	else
		ui.lb_p4->setStyleSheet(QString("border-image:url(%1); background-color:none;").arg(pngn2));

	QString strtip = QString::number(curr) + QString("/") + QString::number(total);
	ui.lb_prognum->setText(strtip);
}
void SYTrainWindow::ReStartScene()
{
	QuitScene();
	InitializeScene();
}
bool SYTrainWindow::InitializeScene()
{
	//time menu frame
	//ui.timeFrame->setWindowFlags(Qt::Window|Qt::FramelessWindowHint );
	//ui.timeFrame->setAttribute(Qt::WA_TranslucentBackground,true);
	//ui.timeFrame->show();

	bool isFirstLaunch = false;
#if(USE_PIX_GRAPHICTOOL)
	m_LoadingUI = 0;
#else
	if (m_LoadingUI == 0)
	{
		m_LoadingUI = new SYLoading(0);
		QDesktopWidget *desktop = QApplication::desktop();
		int screenIndex = desktop->primaryScreen();
		QRect dw = desktop->screenGeometry(screenIndex);
		QRect rtScreen = dw;
		m_LoadingUI->move((rtScreen.width() - m_LoadingUI->width()) / 2, (rtScreen.height() - m_LoadingUI->height()) / 2);
		isFirstLaunch = true;
	}

	m_LoadingUI->Play();
	m_LoadingUI->show();
#endif

	//new thread for IO
 	mDeviceUpdateThread = new DeviceUpdateThread();
 	mDeviceUpdateThread->start();

	//Initialize Ogre Widget Ogre RenderWindow Ogre Initialize
	MXOgreWrapper::Instance()->InitializeOgreWidget(PART_VIEW, RENDER_WINDOW_LARGE, ui.bigGLWidget);
	
	//after 100 million seconds start loading train scene
	StartLoadingTrainScene();

	grabKeyboard();

	m_bMissionTerminate = false;
	
	setVisible(false);//hide this window when loading finish we show it


	//send parent process ping message
	if (isFirstLaunch)
	    QTimer::singleShot(500, static_cast<MXApplication*>(qApp), SLOT(SendPintMsgToParentModule()));

	m_screenshots.clear();
	m_videoFiles.clear();

	return true;
}

void SYTrainWindow::StartLoadingTrainScene()
{
	(static_cast<MXApplication*>(qApp))->StartLoadingTrain(Inception::Instance()->m_strTrainingName, this);
}

void SYTrainWindow::FinishLoadingTrainScene()
{
	connect(Inception::Instance(), SIGNAL(SelectTool(int, int, int)), this, SLOT(SelectTool(int, int, int)));
	connect(Inception::Instance(), SIGNAL(SelectDegree(int)), this, SLOT(SelectDegree(int)));
	connect(Inception::Instance(), SIGNAL(ShowMovie(QString)), this, SLOT(on_PostShowMovieEvent(QString)));

	connect(Inception::Instance(), SIGNAL(ShowPhantomBoxDebug()), this, SLOT(On_ShowPhantomBoxDebug()));
	connect(Inception::Instance(), SIGNAL(GetPhantomBoxDebugInfo(QString&)), this, SLOT(On_GetPhantomBoxDebugInfo(QString&)));

	connect(Inception::Instance(), SIGNAL(ShowAdjustGyro()), this, SLOT(On_ShowAdjustGyro()));
	connect(Inception::Instance(), SIGNAL(GetAdjustGyroInfo(QString&)), this, SLOT(On_GetAdjustGyroInfo(QString&)));

	//connect(Inception::Instance(), SIGNAL(trainPopupWidget(int, const std::string&, const std::string&, const std::string&, const std::string&)), this, SLOT(on_StashPopupWidget(int, const std::string&, const std::string&, const std::string&, const std::string&)));
	connect(Inception::Instance(), SIGNAL(ShowTip(TipInfo)), this, SLOT(on_ShowTip(TipInfo)), Qt::DirectConnection);
	connect(Inception::Instance(), SIGNAL(ShowFixToolMenue()), this, SLOT(on_ShowFixToolMenu()), Qt::DirectConnection);

	connect(Inception::Instance(), SIGNAL(BeginTiming()), this, SLOT(onTimingStart()));

	ActiveUI();

	(static_cast<MXApplication*>(qApp))->OnTrainLoadingFinished();

	ui.bigGLWidget->setAttribute(Qt::WA_PaintOnScreen);//在创建场景时不启用，避免显示上一画面残影

	Inception::Instance()->SetErrorEndState(false);//正常情况下，非严重错误结束状态。
	
	m_TrainConfigedTime = Inception::Instance()->m_totalTime;// 训练操配置的用时

	m_RecordVideoString = "";
	
	m_trainingStartTime = QDateTime::currentDateTime();
	
	m_TimerTrain->start(1000);
//	m_pTimeWindow->restart(m_remainTime);

	if (!m_bFirstShow)
	{
		m_bFirstShow = true;
		if (m_pDlgMoviePlayer && m_pDlgMoviePlayer->GetNumMovies() > 0)
		{
			Inception::Instance()->EmitShowMovie("Start");
		}
	}

	//hide loading UI and show train window
	setVisible(true);

	if (m_LoadingUI)
	{
		m_LoadingUI->Stop();
		QTimer::singleShot(500, m_LoadingUI, SLOT(hide()));
	}
}

bool SYTrainWindow::TerminateScene()
{
	if (m_bMissionTerminate)
	{
		return false;
	}
	else
	{
		m_bMissionTerminate = true;
	}

	//disconnect slot first
	disconnect(Inception::Instance(), SIGNAL(SelectTool(int, int, int)), this, SLOT(SelectTool(int, int, int)));
	disconnect(Inception::Instance(), SIGNAL(SelectDegree(int)), this, SLOT(SelectDegree(int)));
	disconnect(Inception::Instance(), SIGNAL(ShowMovie(QString)), this, SLOT(on_PostShowMovieEvent(QString)));

	disconnect(Inception::Instance(), SIGNAL(ShowPhantomBoxDebug()), this, SLOT(On_ShowPhantomBoxDebug()));
	disconnect(Inception::Instance(), SIGNAL(GetPhantomBoxDebugInfo(QString&)), this, SLOT(On_GetPhantomBoxDebugInfo(QString&)));

	disconnect(Inception::Instance(), SIGNAL(ShowAdjustGyro()), this, SLOT(On_ShowAdjustGyro()));
	disconnect(Inception::Instance(), SIGNAL(GetAdjustGyroInfo(QString&)), this, SLOT(On_GetAdjustGyroInfo(QString&)));

	//disconnect(Inception::Instance(), SIGNAL(trainPopupWidget(int, const std::string&, const std::string&, const std::string&, const std::string&)), this, SLOT(on_StashPopupWidget(int, const std::string&, const std::string&, const std::string&, const std::string&)));

	disconnect(Inception::Instance(), SIGNAL(ShowTip(TipInfo)), this, SLOT(on_ShowTip(TipInfo)));

	disconnect(Inception::Instance(), SIGNAL(ShowFixToolMenue()), this, SLOT(on_ShowFixToolMenu()));

	disconnect(Inception::Instance(), SIGNAL(BeginTiming()), this, SLOT(onTimingStart()));

	//deactive ui
	DeactiveUI();

	//release key board
	releaseKeyboard();

	//stop device thread
	if (mDeviceUpdateThread != 0)
	{
		if (mDeviceUpdateThread->isRunning())
		{
			mDeviceUpdateThread->stop();
			if (QThread::currentThread() != mDeviceUpdateThread)
				mDeviceUpdateThread->wait();
		}
	}

	//now delete object
	SAFE_DELETE(mDeviceUpdateThread);

	m_strTipGuide = "";

	return true;
}

void SYTrainWindow::ActiveUI()
{
	CBasicTraining * pTraining = GetCurrentTraining();

	CXMLWrapperTraining * pTrainconfig = pTraining->m_pTrainingConfig;

	bool isSkillTrain = (pTrainconfig->m_Type == "NewBasicTraining");

	QSize wSize = ui.bigGLWidget->size();

	QPoint p(ui.bigGLWidget->mapToGlobal(QPoint(0, 0)));

	//m_pDlgMoviePlayer = new RbMoviePlayer(this, m_caseParam.trainEnName);
	m_pDlgMoviePlayer = new RbMoviePlayer(this);   //
	m_pDlgMoviePlayer->setVisible(false);
	
	//create tools for left and right from config and position them in right place!
    m_toolMenu_l = new  SYToolMenu(m_caseParam.trainEnName, pTrainconfig->m_ToolForTasks, true, ui.bigGLWidget);
	m_toolMenu_l->AddMenuEventListener(this);

	m_toolMenu_r = new  SYToolMenu(m_caseParam.trainEnName, pTrainconfig->m_ToolForTasks, false, ui.bigGLWidget);
	m_toolMenu_r->AddMenuEventListener(this);

	//int lheight = m_toolMenu_l->height();
	///m_toolMenu_l->setPositon(QPoint(p.x(), p.y() + (wSize.height() - lheight) / 2));
	///m_toolMenu_r->setPositon(QPoint(p.x() + wSize.width() - m_toolMenu_r->width(), p.y() + (wSize.height() - m_toolMenu_l->height()) / 2));

	//
	m_pFixedToolsMenu = new RbFixedToolsMenu(ui.bigGLWidget);
	m_pFixedToolsMenu->setCurCaseOperationWidget(this);
	m_pFixedToolsMenu->move((wSize.width() - m_pFixedToolsMenu->width()) / 2, 125);
	m_pFixedToolsMenu->hide();

	m_phantomboxdebug = new RbPhantomBoxTest(this);
	m_phantomboxdebug->hide();
	
#if(0)
	m_pDemonstrationBeginBtn = new QPushButton(ui.timeFrame);
	m_pDemonstrationBeginBtn->setObjectName("demonstrationBeginBtn");
	m_pDemonstrationBeginBtn->move(wSize.width() - 40, 5);
	m_pDemonstrationBeginBtn->hide();
	
	m_pDemonstrationStopBtn = new QPushButton(ui.timeFrame);
	m_pDemonstrationStopBtn->setObjectName("demonstrationStopBtn");
	m_pDemonstrationStopBtn->setEnabled(false);
	m_pDemonstrationStopBtn->move(m_pDemonstrationBeginBtn->x(), m_pDemonstrationBeginBtn->y() + m_pDemonstrationBeginBtn->height() + 30);
	m_pDemonstrationStopBtn->hide();

	if (m_isAdministrator)
	{
		m_pConnectStatusWindow = new RbConnectStatusWindow(ui.bigGLWidget);
		m_pConnectStatusWindow->move(0, wSize.height() - m_pConnectStatusWindow->height() - 48);
		//m_pConnectStatusWindow->show();

		//m_pDemonstrationBeginBtn->show();
		//m_pDemonstrationStopBtn->show();
		ui.bt_recordvid->show();
	}
#endif
	//创建训练的计时器
	m_TimerTrain = new QTimer(this);

	m_adjustGyro = new RbAdjustGyroTool(this);
	m_adjustGyro->hide();

	ui.lb_traintime->setText(QString("00:00"));
	ui.lb_recordtime->setText(QString("00:00"));
	QString strPixmap = MxGlobalConfig::Instance()->GetSkinDir() + "/sytrainwindow/video_icon.png";
	ui.bt_recordvid->setStyleSheet(QString("border-image:url(%1);").arg(strPixmap));
	ui.tb_recordswitch->setEnabled(true);
	ui.bt_capscreen->SetTipStr("");
	ui.lb_trainname->setText(m_caseParam.trainChName);

	if (!isSkillTrain)
	{
		ui.lb_level->hide();
		ui.lb_difficult->hide();
		ui.icon_level->hide();
	}
	else
	{
		QString levl1Icon = MxGlobalConfig::Instance()->GetSkinDir() + "/sytrainwindow/level1.png";
		QString levl2Icon = MxGlobalConfig::Instance()->GetSkinDir() + "/sytrainwindow/level2.png";
		QString levl3Icon = MxGlobalConfig::Instance()->GetSkinDir() + "/sytrainwindow/level3.png";

		ui.tb_zerodegree->hide();
		ui.tb_30degree->hide();
		ui.lb_zerodeg->hide();
		ui.lb_thirtydeg->hide();

		int diffcultLevel = pTrainconfig->m_DifficultLevel;
		if (diffcultLevel >= 3)
		{
			ui.lb_level->setText(SYStringTable::GetString(SYStringTable::STR_SKILLTRAINLEVEL3));
			ui.icon_level->setStyleSheet(QString("border-image:url(%1); background-color:none;").arg(levl3Icon));
		}
		else if (diffcultLevel >= 2)
		{
			ui.lb_level->setText(SYStringTable::GetString(SYStringTable::STR_SKILLTRAINLEVEL2));
			ui.icon_level->setStyleSheet(QString("border-image:url(%1); background-color:none;").arg(levl2Icon));
		}
		else
		{
			ui.lb_level->setText(SYStringTable::GetString(SYStringTable::STR_SKILLTRAINLEVEL1));
			ui.icon_level->setStyleSheet(QString("border-image:url(%1); background-color:none;").arg(levl1Icon));
		}
	}

	connect(m_TimerTrain, SIGNAL(timeout()), this, SLOT(OnTrainTimeElapsed()));
	
	connect(m_phantomboxdebug, SIGNAL(closeExit()), this, SLOT(On_DestroyPhantomBoxDebug()));

	connect(ui.tb_recordswitch, SIGNAL(stateChanged(int)), this, SLOT(onSwitchRecordBtn(int)));

	connect(ui.bt_back, SIGNAL(clicked()), this, SLOT(on_completeBtn_clicked()));

	connect(m_pDemonstrationBeginBtn, SIGNAL(clicked()), this, SLOT(onDemonstrationBeginBtn()));
	connect(m_pDemonstrationStopBtn, SIGNAL(clicked()), this, SLOT(onDemonstrationStopBtn()));
}

void SYTrainWindow::DeactiveUI()
{
	m_toolMenu_l->RemoveMenuEventListener(this);
	m_toolMenu_r->RemoveMenuEventListener(this);

	disconnect(ui.bt_back, SIGNAL(clicked()), this, SLOT(on_completeBtn_clicked()));
	//disconnect(ui.bt_recordvid, SIGNAL(clicked()), this, SLOT(onRecordBtn()));
	disconnect(ui.tb_recordswitch, SIGNAL(stateChanged(int)), this, SLOT(onSwitchRecordBtn(int)));

	disconnect(m_pDemonstrationBeginBtn, SIGNAL(clicked()), this, SLOT(onDemonstrationBeginBtn()));
	disconnect(m_pDemonstrationStopBtn, SIGNAL(clicked()), this, SLOT(onDemonstrationStopBtn()));
	disconnect(m_phantomboxdebug, SIGNAL(closeExit()), this, SLOT(On_DestroyPhantomBoxDebug()));
	disconnect(m_TimerTrain, SIGNAL(timeout()), this, SLOT(OnTrainTimeElapsed()));

	ui.rightToolFrame->setStyleSheet(QString("image:url();"));
	ui.leftToolFrame ->setStyleSheet(QString("image:url();"));
	//stop timers
	m_TimerTrain->stop();
	//m_pTipTimer->stop();

	//stop movie
	if (m_pDlgMoviePlayer)//&& (!m_pDlgMoviePlayer->isHidden()))
	{
		m_pDlgMoviePlayer->Stop();
	}
	SAFE_DELETE(m_pDlgMoviePlayer);
	
	SAFE_DELETE(m_toolMenu_l);

	SAFE_DELETE(m_toolMenu_r);

	SAFE_DELETE(m_pFixedToolsMenu);

	SAFE_DELETE(m_phantomboxdebug);

	SAFE_DELETE(m_pDemonstrationBeginBtn);

	SAFE_DELETE(m_pDemonstrationStopBtn);

	SAFE_DELETE(m_pConnectStatusWindow);

	SAFE_DELETE(m_TimerTrain);

	SAFE_DELETE(m_adjustGyro);

	SAFE_DELETE(m_CameraPullOutImg);
//	SAFE_DELETE(m_pDlgLoading);
}

void SYTrainWindow::OnToolMenuSelected(int side, QString & picFile, QString &type, QString &subType)
{
	if (side == 0)//left
	{
		ui.leftToolFrame->setStyleSheet(QString("image:url(%1);").arg(picFile));
	}
	else if (side == 1)//right
	{
		ui.rightToolFrame->setStyleSheet(QString("image:url(%1);").arg(picFile));
	}
}
//////////////////////////////////////////////////////////////////////////
void SYTrainWindow::On_GetPhantomBoxDebugInfo(QString& strinfo)
{
	if (m_phantomboxdebug)
	{
		strinfo= QString::fromStdString(m_phantomboxdebug->getWidgetValueString());
	}
}

void SYTrainWindow::On_ShowPhantomBoxDebug()
{
	if (m_phantomboxdebug)
	{
		m_phantomboxdebug->show();
		m_phantomboxdebug->resetWidgetValue();
	}
}

void SYTrainWindow::On_DestroyPhantomBoxDebug()
{
	if (m_phantomboxdebug)
	{
		SAFE_DELETE(m_phantomboxdebug);
	}
}

//////////////////////////////////////////////////////////////////////////
void SYTrainWindow::On_ShowAdjustGyro()
{
	if (m_adjustGyro)
	{
		m_adjustGyro->show();
		m_adjustGyro->resetWidgetValue();
	}
}

void SYTrainWindow::On_GetAdjustGyroInfo(QString& strinfo)
{
	if (m_adjustGyro)
	{
		strinfo= QString::fromStdString(m_adjustGyro->getWidgetValueString());
	}
}

QString SYTrainWindow::FormatSeconds(float totalSeconds)
{
	int minutePart = totalSeconds / 60;
	int secondPart = (totalSeconds - minutePart * 60);
	
	QChar timeStr[6];

	int  minute0 = minutePart / 10;
	int  minute1 = minutePart - 10 * minute0;
	timeStr[0] = minute0 + ('0');
	timeStr[1] = minute1 + ('0');
	timeStr[2] = QChar(':');
	int  second0 = secondPart / 10;
	int  second1 = secondPart - 10 * second0;
	timeStr[3] = second0 + ('0');
	timeStr[4] = second1 + ('0');
	timeStr[5] = 0;

	QString formattedStr(timeStr);

	return formattedStr;
}
void SYTrainWindow::onTimingStart()
{
	//m_IsTimingStart = true;
	m_TimeFromTrainActive = 0;
}
//////////////////////////////////////////////////////////////////////////
void SYTrainWindow::OnTrainTimeElapsed()
{
	float remainTime = m_TrainConfigedTime - m_TimeFromTrainActive;

	bool  userExitTrain = false;

	MisNewTraining * currTrain = dynamic_cast<MisNewTraining*>(CTrainingMgr::Get()->GetCurTraining());

	if (currTrain)
	{
		TrainPopupWidgetButtonActionType QuitType = currTrain->GetTrainQuitType();
		if (QuitType == TPWBT_TRAIN_COMPLETELY || QuitType == TPWBT_TRAIN_FATALERROR
		 || QuitType == TPWBT_TRAIN_ERRORWITHOUTQUIT || QuitType == TPWBT_TRAIN_TIMESUP)
		{
			userExitTrain = true;
		}
	}
	
	if (userExitTrain || remainTime <= 0)
	{
		disconnect(m_TimerTrain, SIGNAL(timeout()), this, SLOT(OnTrainTimeElapsed()));

		if (userExitTrain == false)//非用户主动退出训练。
		{
			if (currTrain)
			{
				currTrain->TrainTimeOver();
			}
			ShowModleDlg(TPWBT_TRAIN_TIMESUP, "", "训练超时", "退出训练", "重新开始");
		}
		else//更具用户不同的退出种类弹出不同对话框
		{
			switch (currTrain->GetTrainQuitType())
			{
			   case TPWBT_TRAIN_COMPLETELY:
				    ShowModleDlg(TPWBT_TRAIN_COMPLETELY, "", "完成训练", "退出训练", "重新开始");
				    break;
			   case TPWBT_TRAIN_FATALERROR:
				    ShowModleDlg(TPWBT_TRAIN_FATALERROR, "", "严重操作失误", "退出训练", "重新开始");
				    break;
			   case TPWBT_TRAIN_ERRORWITHOUTQUIT:
				    ShowModleDlg(TPWBT_TRAIN_ERRORWITHOUTQUIT, "", "严重操作失误", "退出训练", "继续训练");
				    break;
			}
		}
		return;
	}

	if (m_TimeFromTrainActive >= 0)
	{
        QString strTimeTrain = FormatSeconds(m_TimeFromTrainActive);
		
		ui.lb_traintime->setText(strTimeTrain);

		Inception::Instance()->m_remainTime = remainTime;
		
		m_TimeFromTrainActive++;
	}

	if (m_TimeVideoRecord >= 0)//<0 means not in record
	{
		m_TimeVideoRecord++;
		
		QString strVideoTime = FormatSeconds(m_TimeVideoRecord);
		
		ui.lb_recordtime->setText(strVideoTime);
		
		if (ui.tb_recordswitch->isEnabled() == false && m_TimeVideoRecord > 10)
		{
			ui.tb_recordswitch->setEnabled(true);
		}
	}
}

void SYTrainWindow::HideTimeFrame()
{
	//ui.timeFrame->hide();
}

void SYTrainWindow::ShowTimeFrame()
{
	//ui.timeFrame->show();
}

void SYTrainWindow::showEvent( QShowEvent * event )
{
	QRect rect = ui.leftToolFrame->geometry();
	m_toolMenu_l->setPositon(QPoint(rect.x() + rect.width(), rect.y()));
	m_toolMenu_l->hide();

	rect = ui.rightToolFrame->geometry();
	m_toolMenu_r->setPositon(QPoint(rect.x() - m_toolMenu_r->width(), rect.y()));
	m_toolMenu_r->hide();
	//
	if (m_CameraPullOutImg == 0)
	{
		QString strImageShow = MxGlobalConfig::Instance()->GetSkinDir() + "/sytrainwindow/camerapullout.png";
		int width = ui.bigGLWidget->width();
		int height = ui.bigGLWidget->height();
		m_CameraPullOutImg = new QLabel(ui.bigGLWidget);
		m_CameraPullOutImg->setVisible(false);
		m_CameraPullOutImg->setScaledContents(true);
		m_CameraPullOutImg->setFixedWidth(width);
		m_CameraPullOutImg->setFixedHeight(height);
		
		m_CameraPullOutImg->setStyleSheet(QString("border-image:url(%1); background-color:none;").arg(strImageShow));
	}

	//ShowProgressRegion();
}

void SYTrainWindow::mouseMoveEvent(QMouseEvent *event)
{
	ui.bigGLWidget->mouseMoveEvent(event);
}

void SYTrainWindow::wheelEvent(QWheelEvent *event)
{
    ui.bigGLWidget->wheelEvent(event);
}

void SYTrainWindow::keyReleaseEvent (QKeyEvent * evt)
{
	ui.bigGLWidget->keyReleaseEvent(evt);
	if(m_trainingDebugWindow)
		m_trainingDebugWindow->keyReleaseEvent(evt);
}

void SYTrainWindow::keyPressEvent( QKeyEvent *event )
{
	ui.bigGLWidget->keyPressEvent(event);
	if(m_trainingDebugWindow)
	   m_trainingDebugWindow->keyPressEvent(event);

	switch(event->key())
	{
	case Qt::Key_F1:
		{
			if(m_trainingDebugWindow == nullptr)
			{
				m_trainingDebugWindow = new MxTrainingDebugWindow(this, (MisNewTraining*)GetCurrentTraining());
				m_trainingDebugWindow->show();
				
				connect(Inception::Instance(), SIGNAL(ShowDebugInfo(const std::string&)), m_trainingDebugWindow, SLOT(onAddOneDebugInfo(const std::string&)));
			}
			else
			{
				m_trainingDebugWindow->setVisible(!m_trainingDebugWindow->isVisible());
			}

			GetCurrentTraining()->SetEditMode(true/*m_trainingDebugWindow->isVisible()*/);

			m_trainingDebugWindow->setTraining(dynamic_cast<MisNewTraining*>(GetCurrentTraining()));

			if (m_trainingDebugWindow->isVisible())
			    HideTimeFrame();
			else
			    ShowTimeFrame();
		}
		break;
	case Qt::Key_F2:
	case Qt::Key_F3:
	case Qt::Key_F4:
		if (m_toolMenu_l)
 		    m_toolMenu_l->keyPressEvent(event);
		break;
	case Qt::Key_F5:
	case Qt::Key_F6:
	case Qt::Key_F7:
		if (m_toolMenu_r)
		    m_toolMenu_r->keyPressEvent(event);
		break;
	case Qt::Key_F9:
	case Qt::Key_F10:
	case Qt::Key_F11:
		
		break;
	}

    ITraining * pTraining = (static_cast<MXApplication*>(qApp))->GetEngineCore()->GetTrainingMgr()->GetCurTraining();
    if (pTraining)
    {
        pTraining->KeyPress(event);//move the organ by keyboard
    }
}


void SYTrainWindow::on_completeBtn_clicked()
{
	//new code
	m_TimerTrain->stop();

	if (m_bFinished)
	{
		m_bFinished = false;
		return;
	}

	ShowModleDlg(0,"", "是否退出训练","是","否");
}

void SYTrainWindow::on_tipBtn_clicked()
{
	const QString & caseDir = MxGlobalConfig::Instance()->GetCurTrainCaseDir();
	const QString & caseFileName = MxGlobalConfig::Instance()->GetCurTrainCaseFileName();
}

void SYTrainWindow::on_ShowTip( TipInfo tipInfo )
{
	switch (tipInfo.eIconType)
	{
	default:
	case TipInfo::TIT_GUIDE:
		m_strTipGuide = tipInfo.str;
		if(ui.tipLabel)
		{
			//m_pTipWindow->show();//tip window may be close in initial of the scene resure show
			std::string test = m_strTipGuide.toLocal8Bit().toStdString();
			ui.tipLabel->setText(m_strTipGuide);
		}
		break;
	case TipInfo::TIT_TIP:
	case TipInfo::TIT_TIP_WARNING:
		if (ui.tipLabel)
		{
			//m_pTipWindow->show();
			ui.tipLabel->setText(tipInfo.str);
		}
		//if (m_pTipTimer)
		//{
		//	if (m_pTipTimer->isActive())
			//{
			//	m_pTipTimer->stop();
			//}

			//m_pTipTimer->setInterval(1000 * tipInfo.nDuration);
			//m_pTipTimer->start();
		//}
		break;
	}
}

void SYTrainWindow::on_ShowFixToolMenu()
{
	m_pFixedToolsMenu->show();
}

void SYTrainWindow::on_picBtn_clicked()
{
	/*static int clickitme = 0;
	if (clickitme % 2 == 0)
	{
		ui.tipLabel->setText(QString::fromLocal8Bit("消除成功，请定位字母A"));
	}
	else
	{
		ui.tipLabel->setText(QString::fromLocal8Bit("消除成功，请定位字母B"));
		std::string test = ui.tipLabel->text().toLocal8Bit().toStdString();
	}


	clickitme++;
	return;*/

}

void SYTrainWindow::on_bt_capscreen_clicked()
{
	if (m_screenshots.size() == 12)
	{
		if (ui.tipLabel)
			ui.tipLabel->setText(QString::fromLocal8Bit("无法保存截图：截图数量已达到最大值12"));
		return;
	}

	int width  = ui.bigGLWidget->width();
	int height = ui.bigGLWidget->height();

	QPixmap pixmap = QPixmap::grabWindow(ui.bigGLWidget->winId());
	QString userName = SYUserInfo::Instance()->GetUserName();
	QString picName = QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss") + ".jpg";
	QString path = "./data/" + userName;

	QDir createDir;
	if (!createDir.exists(path))
	{
		bool success = createDir.mkpath(path);
	}
	ITraining * pTraining = (static_cast<MXApplication*>(qApp))->GetEngineCore()->GetTrainingMgr()->GetCurTraining();
	if (pTraining)
	{
		QString trainName = QString::fromStdString(pTraining->m_strName);

		picName = trainName + "-" + picName;
		path += "/" + picName;

		//pixmap = pixmap.scaled(634, 460);
		//pixmap.save(path, QString("jpg").toUtf8());
		m_screenshots.insert(picName, pixmap);

		if (ui.tipLabel)
			ui.tipLabel->setText(QString::fromLocal8Bit("保存快照成功，可进入训练成绩中查看快照！"));
	}

	int numCap = m_screenshots.size();
	if (numCap > 0)
		ui.bt_capscreen->SetTipStr(QString::number(numCap));

	//
	int  borderwid = 6;
	QPen borderPen(QColor(255,255,255), borderwid, Qt::SolidLine);

	QPainter painter(&pixmap);
	painter.setPen(borderPen);
	painter.drawRect(QRect(borderwid*0.5f, borderwid*0.5f, pixmap.width() - borderwid*0.5f, pixmap.height() - borderwid*0.5f));

	ShowCapScreenAnimation(pixmap , 500);
}

void SYTrainWindow::ShowCapScreenAnimation(const QPixmap & pixmap, int mseconds)
{
	QLabel * screenCapShow = 0;

	screenCapShow = new QLabel(ui.bigGLWidget);
	screenCapShow->setVisible(true);
	screenCapShow->setScaledContents(true);
	screenCapShow->setPixmap(pixmap);

	int width  = ui.bigGLWidget->width();
	int height = ui.bigGLWidget->height();

	QPropertyAnimation* animation = new QPropertyAnimation(screenCapShow, "geometry");
	
	animation->setDuration(mseconds);
	animation->setStartValue(QRect(0, 0, width, height));
	animation->setEndValue(QRect(width - width*0.1f, height - height*0.1f, width*0.1f, height*0.1f));
	animation->start(QAbstractAnimation::DeleteWhenStopped);

	connect(animation, SIGNAL(finished()), this, SLOT(on_CapScreenAnime_Finished()));
}

void SYTrainWindow::on_CapScreenAnime_Finished()
{
	QPropertyAnimation * animation = qobject_cast<QPropertyAnimation*>(sender());
	if (animation)
	{
		disconnect(animation, SIGNAL(finished()), this, SLOT(on_CapScreenAnime_Finished()));

		QLabel * screenPicLable = qobject_cast<QLabel*>(animation->targetObject());

		if (screenPicLable)
		{
			delete screenPicLable;
		}
	}
}

void SYTrainWindow::on_tb_zerodegree_toggled(bool check)
{
	if (check == false)
		return;

	MisNewTraining * Train = GetCurrentTraining();
	if (Train)
	{
		Train->SetCameraSpecialAngle(0);
	}
}

void SYTrainWindow::on_tb_30degree_toggled(bool check)
{
	if (check == false)
		return;
	MisNewTraining * Train = GetCurrentTraining();
	if (Train)
	{
		Train->SetCameraSpecialAngle(30.0f);
	}
}

void SYTrainWindow::on_tb_anatomy_clicked()
{

}
void SYTrainWindow::onDemonstrationBeginBtn()
{
	if(m_screenDataSender)
	{
		m_pDemonstrationBeginBtn->setEnabled(false);
		m_pDemonstrationStopBtn->setEnabled(true);
		m_screenDataSender->StartSendScreenData();
		m_screenDataSender->SendCommand(MxSessionCommand::CT_BeginDemonstration);
	}
}

void SYTrainWindow::onDemonstrationStopBtn()
{
	if(m_screenDataSender)
	{
		m_pDemonstrationBeginBtn->setEnabled(true);
		m_pDemonstrationStopBtn->setEnabled(false);
		m_screenDataSender->StopSendScreenData();
		m_screenDataSender->SendCommand(MxSessionCommand::CT_StopDemonstration);
	}
}

void SYTrainWindow::onSwitchRecordBtn(int check)
{
	if (check)
	{
		if (MxGlobalConfig::Instance()->UseHardwareEncode())
		{
#if(USE_NEWRECORDER)
			if (m_InRecordNew == false)
#else
			if (m_pScreenRecorder == nullptr)
#endif
			{
				ui.tb_recordswitch->setEnabled(false);//10s后方可停止
				
				QString strPixmap =  MxGlobalConfig::Instance()->GetSkinDir() + "/sytrainwindow/video_icon2.png";
				ui.bt_recordvid->setStyleSheet(QString("border-image:url(%1);").arg(strPixmap));
		
				StartVideoRecording();
			}
		}
		else
		{
			//software encode
#if(!USE_NEWRECORDER)
			if (!m_screenDataSender->HasVideoFileWrited())
			{
				m_screenDataSender->StartWriteVideoData(MakeVideoFileName());
				m_TimeVideoRecord = 0;
				ui.tb_recordswitch->setEnabled(false);		//10s后方可停止
			}
#endif
		}
	}
	else
	{
		if (MxGlobalConfig::Instance()->UseHardwareEncode())
		{
#if(USE_NEWRECORDER)
			if (m_InRecordNew == true)
#else
			if (m_pScreenRecorder)
#endif
			{
				std::string videoFile = StopVideoRecording();
				if(videoFile.size() > 0)
					m_videoFiles.push_back(QString::fromStdString(videoFile));
				QString strPixmap = MxGlobalConfig::Instance()->GetSkinDir() + "/sytrainwindow/video_icon.png";
				ui.bt_recordvid->setStyleSheet(QString("border-image:url(%1);").arg(strPixmap));
			}
		}
		else
		{
#if(!USE_NEWRECORDER)
			//software encode
			if (m_screenDataSender->HasVideoFileWrited())
			{
				m_screenDataSender->StopWriteVideoData();
				///m_videoRecordingStopTime = m_ElapsedTime;
			}
#endif
		}
		m_TimeVideoRecord = -1;//negative meanse not record
	}
}

void SYTrainWindow::onReceiveSessionCommand(unsigned int ip, const MxSessionCommand& command)
{
	if(m_pConnectStatusWindow)
	{
		switch(command.Type())
		{
		case MxSessionCommand::CT_RequestConnect:
			//已处理重复添加的情况
			if(MxGlobalConfig::Instance()->MulticastMode() == false)
				m_screenDataSender->AddDestination(ip, MxGlobalConfig::Instance()->GetClientPort());
			break;
		case MxSessionCommand::CT_Enter:
		case MxSessionCommand::CT_BeginWatch:
			m_pConnectStatusWindow->AddUser(ip);
			break;
		case MxSessionCommand::CT_Exit:
			if(MxGlobalConfig::Instance()->MulticastMode() == false)
				m_screenDataSender->DeleteDestination(ip, MxGlobalConfig::Instance()->GetClientPort());
			m_pConnectStatusWindow->RemoveUser(ip);
			break;
		case MxSessionCommand::CT_StopWatch:
			m_pConnectStatusWindow->RemoveUser(ip);
			break;
		case MxSessionCommand::CT_Update:
			m_pConnectStatusWindow->UpdateUserInfo(ip, command.GetData("UserName"), command.GetData("RealName"));
			break;
		}
	}
}

void SYTrainWindow::on_ShowMovie()
{
	if (m_pDlgMoviePlayer)
	{
		if (m_pDlgMoviePlayer->isHidden())
		{
			m_pDlgMoviePlayer->show();
			m_pDlgMoviePlayer->Play();
		}
		else
		{
			m_pDlgMoviePlayer->Stop();
		}
	}
}

void SYTrainWindow::on_LoadMovie(QString strMovieName)
{
	if (m_pDlgMoviePlayer)
	{
		m_pDlgMoviePlayer->Load(strMovieName);
	}
}


void SYTrainWindow::on_PostShowMovieEvent(QString strMovieName)
{
	if (m_pDlgMoviePlayer)
	{
		if(m_pDlgMoviePlayer->Load(strMovieName))
		{
			QApplication::postEvent(this, new QEvent(QEvent::Type(SHOW_MOVIE_EVENT)));
		}
	}
}

void SYTrainWindow::on_PostLoadMovieEvent(QString strMovieName)
{
	m_strMovieName = strMovieName;
	QApplication::postEvent(this, new QEvent(QEvent::Type(LOAD_MOVIE_EVENT)));
}

void SYTrainWindow::on_PostShowMovieAndFinishEvent()
{
	m_bFinished = true;
}

void SYTrainWindow::customEvent(QEvent *event)
{
	switch (event->type())
	{
	case SHOW_MOVIE_EVENT:
		on_ShowMovie();
		break;
	case LOAD_MOVIE_EVENT:
		on_LoadMovie(m_strMovieName);
		break;
	//case SHOW_MOVIE_AND_FINISH:
	//	on_ShowMovieAndFinish();
		break;
	case FINISHED_LOADING_EVENT:
		FinishLoadingTrainScene();
		break;
	}
}

void SYTrainWindow::EnableTimer()
{
	if (!m_TimerTrain->isActive())
	{
		m_TimerTrain->start(1000);
	}
}

void SYTrainWindow::DisableTimer()
{
	if (m_TimerTrain->isActive())
	{
		m_TimerTrain->stop();
	}
}

MisNewTraining * SYTrainWindow::GetCurrentTraining()
{
	ITraining * pTraining = (static_cast<MXApplication*>(qApp))->GetEngineCore()->GetTrainingMgr()->GetCurTraining();
	return static_cast<MisNewTraining*>(pTraining);
}

CToolsMgr * SYTrainWindow::GetToolsMgr()
{
	CBasicTraining* pTraining = GetCurrentTraining();

	if ( ! pTraining )
	{
		return NULL;
	}

	CToolsMgr * pToolsMgr = pTraining->m_pToolsMgr;

	if ( ! pToolsMgr )
	{
		return NULL;
	}

	return pToolsMgr;
}

void SYTrainWindow::SelectTool(int handType, int msgType, int submsgType)
{
	switch(handType)
	{
	case 0:
		m_toolMenu_l->handleMsg(msgType , submsgType);
		break;
	case 1:
		m_toolMenu_r->handleMsg(msgType, submsgType);
		break;
	default:
		break;
	}
}

void SYTrainWindow::SelectDegree( int msgType )
{
 		
}

void SYTrainWindow::PushScoreCode(const char code[20], int begintime, int endtime)
{
	//MX::pushScoreCode(code, begintime, endtime);
	MxProcessCommunicator* pCommnunicator = (static_cast<MXApplication*>(qApp))->getCommunicator();

	PushOnlineCodeMsg * msg = new PushOnlineCodeMsg();
	strcpy(msg->m_Code, code);
	msg->m_StartTime = begintime;
	msg->m_EndTime = endtime;
	pCommnunicator->SendMessage(msg);

	delete msg;
}

void SYTrainWindow::StopOnlineGrade()
{
	MxProcessCommunicator* pCommnunicator = (static_cast<MXApplication*>(qApp))->getCommunicator();

	StopOnlineGradeMsg* msg = new StopOnlineGradeMsg();
	pCommnunicator->SendMessage(msg);

	delete msg;
}

void SYTrainWindow::StartOnlineGrade(const char* sheetCode)
{
	MxProcessCommunicator* pCommnunicator = (static_cast<MXApplication*>(qApp))->getCommunicator();

	StartOnlineGradeMsg* msg = new StartOnlineGradeMsg();

	size_t lenUrl = strlen(sheetCode) + 1;
	strcpy_s(msg->m_sheetCode, lenUrl, sheetCode);

	//PingMsg * msg = new PingMsg();
	pCommnunicator->SendMessage(msg);

	delete msg;
}

void SYTrainWindow::StopDataServiceCallBack(int, char *pUrl)
{
	QString  strHome = MxGlobalConfig::Instance()->GetScanUrl();

	QString strUrl = pUrl;

	if (strHome != "")
	{
		int pos1stSlash = strUrl.indexOf(":") + 3;
		int posFind = strUrl.indexOf("/", pos1stSlash);

		strUrl = strHome + strUrl.mid(posFind);
		
	}

	//QString strLog = "*** stop dsService *** ";
	//strLog += strUrl;
	
	ScoreBoardWeb::SetInstanceURL(strUrl);

	//Ogre::LogManager* pLogMgr = Ogre::LogManager::getSingletonPtr();
	//Ogre::LogManager::getSingletonPtr()->logMessage(strLog);

}

void SYTrainWindow::QuitScene()
{
	m_pFixedToolsMenu->ClearAllFixedTool();

	CBasicTraining * pTraining = GetCurrentTraining();
	MisNewTraining * newtraining = dynamic_cast<MisNewTraining *>(pTraining);
	
	//save record
	StopVideoRecording();

	//terminate app then terminate ogre engine
	(static_cast<MXApplication*>(qApp))->Terminate();
	
	MXOgreWrapper::Instance()->RemoveAllOgreWidget();
	
	//clear all ui element
	TerminateScene();

	if(m_trainingDebugWindow)
	{
		m_trainingDebugWindow->setTraining(nullptr);
		m_trainingDebugWindow->hideAllSubWindow();
	}

	SaveResult();

	m_TimeFromTrainActive = -1.0f;
	m_TimeVideoRecord = -1.0f;

	//clear screen capture
	m_screenshots.clear();
}

void SYTrainWindow::SaveResult()
{
	SYTrainingReport* trainingReport = SYTrainingReport::Instance();
	SYScoreTable* scoreTable = trainingReport->GetScoreTable();

	if(scoreTable == nullptr)
		return;

	int totalTime = Inception::Instance()->m_totalTime;
	int costTime  = m_TimeFromTrainActive;

	m_reportRecord.reset();
	//1 get all score item detail
	trainingReport->GetScoreItemDetails(m_reportRecord.m_scoreItemDetails);

	//2 update attributes
	m_reportRecord.m_userId = SYUserInfo::Instance()->GetUserId();
	m_reportRecord.m_trainTypeName = scoreTable->GetTrainTypeName();
	m_reportRecord.m_trainTypeCode = scoreTable->GetTrainTypeCode();
	m_reportRecord.m_trainName = scoreTable->GetTrainName();
	m_reportRecord.m_trainCode = scoreTable->GetTrainCode();
	m_reportRecord.m_beginTime = m_trainingStartTime.toString("yyyy-MM-dd HH:mm:ss.zzz");
	m_reportRecord.m_costedTime = costTime;

#if(1)
	{
		m_reportRecord.m_LeftToolMovDist  = SYTrainingReport::Instance()->GetLeftToolMovedDistance();
		
		m_reportRecord.m_RightToolMovDist = SYTrainingReport::Instance()->GetRightToolMovedDistance();
		
		m_reportRecord.m_LeftToolSpeed    = SYTrainingReport::Instance()->GetLeftToolMovedSpeed();
		
		m_reportRecord.m_RightToolSpeed   = SYTrainingReport::Instance()->GetRightToolMovedSpeed();
	}
#endif
	int score = 0;
	for(const auto& scoreItemDetail : m_reportRecord.m_scoreItemDetails){
                              		score += scoreTable->GetScoreValue(scoreItemDetail.GetScoreCode());
	}
	m_reportRecord.m_score = score;
	//m_reportRecord.updateScore();

	//3 save data
	//		if(!isVisitor)
	int TrainTaskID = static_cast<MXApplication*>(qApp)->m_TrainID;
	if (TrainTaskID >= 0)//this is a task
	{
		SYDBMgr::Instance()->UpdateTaskTrainData(TrainTaskID, m_TimeFromTrainActive / 60 + 1, score);
	}

	int scoreID = SYDBMgr::Instance()->SaveReportRecord(m_reportRecord, m_screenshots,m_videoFiles);
	
	if (scoreID)
	{
		QVector<QPair<QString, QString>> trainInfos;
		QVector<QPair<QString, bool>> trainLockStates;

		bool isDisplay = IsDisplaySkillImproveWindow(trainInfos,trainLockStates);
		//isDisplay = true;
		if (isDisplay)
		{
			TrainSkillImproveTipWindow*  skillImproveWindow = new TrainSkillImproveTipWindow(this);

			skillImproveWindow->setBtnVisble(trainInfos);
			skillImproveWindow->setTrainLevelState(trainLockStates);
			skillImproveWindow->showFullScreen();
			QTimer::singleShot(5000, skillImproveWindow, &TrainSkillImproveTipWindow::timeOut);
			skillImproveWindow->exec();
			
		}
	
		SYMessageBox* skillImproveWin = new SYMessageBox(this, "", CHS("恭喜你,已经完成训练!"), 1);
		skillImproveWin->showFullScreen();
		skillImproveWin->exec();

		SYTrainRecordDetailWindow * pReportWindow = new SYTrainRecordDetailWindow(this);
		
		pReportWindow->setContent(SYUserInfo::Instance()->GetUserId(),
			                scoreID,
							m_reportRecord.m_trainName,
			                m_reportRecord.m_score,
							scoreTable->GetCode());
		pReportWindow->setTrainUsedTime(costTime);
		pReportWindow->showFullScreen();
		pReportWindow->exec();
	}
}


bool SYTrainWindow::IsDisplaySkillImproveWindow(QVector<QPair<QString, QString>> &trainInfos, QVector<QPair<QString, bool>> &trainLockStates)
{
	//获得当前类型训练的所有训练阶段的traincode和skill熟练程度
	
	trainInfos=FromXMLReadTrainModuleInfo();

	if (trainInfos.isEmpty())
		return false;
	bool isDisplay = true;

	int skillPassScore = 0;  //技能训练合格分数,可以设为60

	//test

	if (m_reportRecord.m_score < skillPassScore)
	{
		return false;
	}
	//else
	//{
	//	isUpdateLockState = true;
	//}

	//查询各项技能训练通过的分数

	QString cur_trainCode = m_caseParam.trainCode;
	int userId = SYUserInfo::Instance()->GetUserId();

	//完成训练之前的各个技能训练的熟练程度

	int stage=0;
	for (std::size_t i = 0; i < trainInfos.size(); i++)
	{
		QString trainCode = trainInfos[i].first;
		QString level = trainInfos[i].second;
		if (cur_trainCode == trainCode)
		{
			stage = i;
		}
		if (i == 0)
		{
			QPair<QString, bool> trainLockState(level, true);
			trainLockStates.push_back(trainLockState);
		}
		else
		{
			bool ret = SYDBMgr::Instance()->QueryUserTrainIsLock(userId, trainCode);

			QPair<QString, bool> trainLockState(level, ret);
			trainLockStates.push_back(trainLockState);
		}
	}
	//判断当前训练是否为最后的训练阶段
	if (stage == trainInfos.size() - 1)
	{
		isDisplay = false;
	}
	else
	{
		//判定下个阶段训练解锁情况,return true for next item is unlocked
		QPair<QString, bool> nextTrainStageLockState;
		nextTrainStageLockState = trainLockStates[stage + 1];
		bool isLock = nextTrainStageLockState.second;
		if (isLock)
			isDisplay = false;
	}

	if (isDisplay)
	{
		QString trainCode = trainInfos[stage + 1].first;
		SYDBMgr::Instance()->UpdateUserSkillTrainLockState(userId, trainCode);
		bool ret = SYDBMgr::Instance()->QueryUserTrainIsLock(userId, trainCode);
		trainLockStates[stage + 1].second = ret;
		//技能训练各阶段的熟练状态
	}

	return isDisplay;

}

QVector<QPair<QString,QString>> SYTrainWindow::FromXMLReadTrainModuleInfo()
{

	TrainModuleConfig::Instance()->LoadFromXML(MxGlobalConfig::Instance()->GetCourseTrainXmlConfigFileName());

	QDomElement element = TrainModuleConfig::Instance()->m_document.documentElement();
	QDomNode node = element.firstChild();
	QString strMenuName;
	QString strMenuItemName;
	QString strObjectName;
	//CaseInfoParam m_caseParam;
	QString cur_TrainCode = m_caseParam.trainCode;
	bool isQuit = false;
	QVector<QPair<QString, QString>> trainModuleInfos;

	while (!node.isNull())
	{
		if (node.isElement())
		{
			QDomElement moduleElement = node.toElement();

			if (moduleElement.attribute("ModuleName") == QString("SkillTrain"))
			{
				QDomNode subModuleNode = moduleElement.firstChild();

				while (!subModuleNode.isNull())
				{
					QDomElement subModuleEle = subModuleNode.toElement();
					QString moduleTrainName = subModuleEle.attribute("ModuleName");
					QDomNode trainItemNode = subModuleEle.firstChild();

					while (!trainItemNode.isNull())
					{
						QDomElement trainItemEle = trainItemNode.toElement();
						QString trainCode = trainItemEle.attribute("TrainCode");
						QString skillLevel = trainItemEle.attribute("SkillLevel");
						
						if (cur_TrainCode == trainCode)
						{
							isQuit = true;
						}

						QPair<QString, QString> trainInfo(trainCode, skillLevel);
						trainModuleInfos.push_back(trainInfo);
						
						//添加技能锁，被添加锁的训练不能被使用
						trainItemNode = trainItemNode.nextSibling();
					}

					if (isQuit)
						return trainModuleInfos;
					trainModuleInfos.clear();

					subModuleNode = subModuleNode.nextSibling();
				}
				break;
			}

		}
		node = node.nextSibling();
	}

	return trainModuleInfos;
}

void SYTrainWindow::closeEvent(QCloseEvent * event)
{
	ExitModule();
}

void SYTrainWindow::ExitModule()
{
	if (m_ModuleExisted)
		return;

	m_ModuleExisted = true;

	QuitScene();

	static_cast<MXApplication*>(qApp)->exitModuleAndShowParentWindow(this);

	
}

void SYTrainWindow::ShowModleDlg(int mode, const std::string& title, const std::string& description, const std::string& leftButtonText, const std::string& rightButtongText)
{
	//QString qTitle = QString::fromLocal8Bit(title.c_str());
	//暂时没找到basicTrain或者MisNewTrain里获得当前训练名字的方法，暂时这样处理

	bool isRightButtonRetart = (mode > 0 ? true : false);

	QString qTitle;
	QString strTmep = m_caseParam.trainChName;
	for (int i = 0; i != strTmep.length(); ++i)
	{
		if (strTmep[i] != '\\')
		{
			qTitle.push_back(strTmep[i]);
		}
	}

	QString qDescription = QString::fromLocal8Bit(description.c_str());

	QString qLeftButtonText = QString::fromLocal8Bit(leftButtonText.c_str());

	QString qRightButtonText = QString::fromLocal8Bit(rightButtongText.c_str());

	SYMessageDialog * pPopupWidget = new SYMessageDialog(this, qTitle, mode, qDescription, qLeftButtonText, qRightButtonText);
	pPopupWidget->showFullScreen();
	
	//关闭所有计数器,防止在模态对话框的消息循环里面收到计时消息
	if(m_TimerTrain)
	   m_TimerTrain->stop();

	(static_cast<MXApplication*>(qApp))->PauseUpdateLoop();

	bool confirm = pPopupWidget->exec();//确定退出训练,退出并且显示成绩

	(static_cast<MXApplication*>(qApp))->ResumeUpdateLoop();

	if(confirm)
	{
		QTimer::singleShot(10, this, SLOT(ExitModule()));
	}
	else//右键按下
	{
		if(isRightButtonRetart == false)
		{
			//重新打开所有计数器
			if(m_TimerTrain)
			   m_TimerTrain->start();
		}
		else
		{
			ReStartScene();
		}
	}
}

const std::string& SYTrainWindow::MakeVideoFileName()
{
	ITraining * pTraining = (static_cast<MXApplication*>(qApp))->GetEngineCore()->GetTrainingMgr()->GetCurTraining();
	//MisNewTraining * currTrain = dynamic_cast<MisNewTraining*>(CTrainingMgr::Get()->GetCurTraining());
	if(pTraining)
	{
		std::string trainName = pTraining->m_strName;

		QString videoSavePath = MxGlobalConfig::Instance()->GetVideoSaveDir();
		QDir tempDir;
		if(!tempDir.exists(videoSavePath))
		{
			bool success = tempDir.mkpath(videoSavePath);
		}

		QString videoNamePre = QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss");
		std::string videoFileNamePre = videoNamePre.toStdString();//文件前缀

		std::string userName = SYUserInfo::Instance()->GetUserName().toStdString();

		m_videoFileName = MxGlobalConfig::Instance()->GetVideoSaveDir().toStdString() + "\\" + userName + "_" + trainName + "-" + videoFileNamePre + ".mp4";

		m_RecordVideoString = MxGlobalConfig::Instance()->GetVideoSaveDir().toStdString() + "\\\\" + userName + "_" + trainName + "-" + videoFileNamePre + ".mp4";
	}

	return m_videoFileName;
}

void SYTrainWindow::StartVideoRecording()
{
#if AUTO_SCREEN_RECORDER
		#if USE_NEWRECORDER
	      if (m_InRecordNew == false)
	      {
			  std::string videoFileName = MakeVideoFileName();

		      MxProcessCommunicator* pCommnunicator = (static_cast<MXApplication*>(qApp))->getCommunicator();

		      StartRecordTrainMsg* msg = new StartRecordTrainMsg();

			  size_t lenUrl = videoFileName.length() + 1;
			  strcpy_s(msg->m_RecordFileName, lenUrl, videoFileName.c_str());

		      pCommnunicator->SendMessage(msg);

		      delete msg;
		      
			  m_InRecordNew = true;

			  m_TimeVideoRecord = 0;
	    }
		#else
			if( m_pScreenRecorder == nullptr)
			{
				std::string videoFileName = MakeVideoFileName();

				m_pScreenRecorder = new RbScreenRecorder(GetDesktopWindow(), //(HWND)ui.bigGLWidget->winId(), 
														 videoFileName,
														 MxGlobalConfig::Instance()->GetVideoWidth(),
														 MxGlobalConfig::Instance()->GetVideoHeight());
				m_pScreenRecorder->Start();

				m_TimeVideoRecord = 0;
			}
		#endif
#endif
}

std::string SYTrainWindow::StopVideoRecording()
{
#if AUTO_SCREEN_RECORDER
		#if USE_NEWRECORDER
			if (m_InRecordNew == true)
			{
			    MxProcessCommunicator * pCommnunicator = (static_cast<MXApplication*>(qApp))->getCommunicator();

				StopRecordTrainMsg * msg = new StopRecordTrainMsg();

				size_t lenUrl = m_videoFileName.length() + 1;
				
				strcpy_s(msg->m_RecordFileName, lenUrl, m_videoFileName.c_str());

				pCommnunicator->SendMessage(msg);

				delete msg;

				m_InRecordNew = false;

				return m_videoFileName;
			}
		#else
 			if (m_pScreenRecorder)
 			{
				std::string videoPath = m_pScreenRecorder->GetVideoFileName();

 				m_pScreenRecorder->Stop();
				SAFE_DELETE(m_pScreenRecorder);

				//文件改名
				/*std::string videoPathR = videoPath + "r";
				if (rename(videoPath.c_str(), videoPathR.c_str()) != 0)
					return videoPath;
		
				//使用ffmpeg转码
				char szCmd[1024];
				sprintf_s(szCmd, "ffmpeg.exe -i %s -s 1024x600 -vcodec h264 %s", videoPathR.c_str(), videoPath.c_str());
				UINT r = WinExec(szCmd, SW_HIDE);
				*/

				//m_videoRecordingStopTime = m_ElapsedTime;
				return videoPath;
 			}
		#endif
#endif
	return "";
}