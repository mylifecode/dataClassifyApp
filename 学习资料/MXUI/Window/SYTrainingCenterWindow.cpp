#include "MxDefine.h"
#include "SYTrainingCenterWindow.h"
#include "SYKnowLibWindow.h"
#include "SYMessageBox.h"
#include "MxGlobalConfig.h"
#include "SYUserInfo.h"
#include "SYDBMgr.h"
#include "MxScreenDataSender.h"
#include "MxSessionCommand.h"
#include "MxDemonstrationWindow.h"
#include "MXApplication.h"

SYTrainingCenterWindow::SYTrainingCenterWindow(QWidget *parent)
: QWidget(parent),
m_screenDataSender(nullptr),
m_demonstrationWindow(nullptr),
m_preButton(nullptr),
m_clickedFrameIndex(-1)
{
	ui.setupUi(this);

	m_userName = SYUserInfo::Instance()->GetUserName();
	m_realName = SYUserInfo::Instance()->GetRealName();

	m_permission = SYUserInfo::Instance()->GetUserPermission();
// 	m_mapLoginModule = Mx::addModuleItemFromXML();
// 	if (MxGlobalConfig::Instance()->onLineConfig())
// 	{
// 		m_permission = UP_Error;
// 	}
// 
// 	std::vector<std::pair<QPushButton *, QLabel*>> bpPairs;
// 	if (m_mapLoginModule.size())
// 	{
// 		for (std::multimap<UserPermission, QString>::iterator itr = m_mapLoginModule.begin(); itr != m_mapLoginModule.end(); ++itr)
// 		{
// 			if (itr->first == m_permission)
// 			{
// 				QPushButton * pBtn = new QPushButton(this);
// 				QLabel * plabel = new QLabel(this);
// 				plabel->setText(QString::fromLocal8Bit("实物操作训练，可进行夹豆子训练、传递训练、。。训练，训练提升镜下操作技能。初学者可参考上述顺序依次训练。"));
// 
// 
// 				bpPairs.push_back(std::make_pair(pBtn, plabel));
// 		
// 				QVBoxLayout * pLayOut = new QVBoxLayout(this);
// 				pLayOut->setSpacing(6);
// 
// 				pLayOut->insertWidget(0 , pBtn);
// 				pLayOut->insertWidget(1, plabel);
// 
// 				QString str = itr->second;
// 				pBtn->setObjectName(str);
// 				pBtn->setProperty("window", str);
// 				int index = ui.horizontalLayout_5->count() - 1;
// 				if (index > 1)
// 				{
// 					ui.horizontalLayout_5->insertSpacerItem(index, new QSpacerItem(115, 20));
// 				}
// 				index = ui.horizontalLayout_5->count() - 1;
// 				ui.horizontalLayout_5->insertLayout(index, pLayOut);
// 				connect(pBtn, SIGNAL(clicked()), this, SLOT(onButtonClicked()));
// 			}
// 		}
// 	}


 	if(MxGlobalConfig::Instance()->EnabledSendScreenData())
 	{
 		m_screenDataSender = new MxScreenDataSender(GetDesktopWindow(),
													MxGlobalConfig::Instance()->GetVideoWidth(),//1248
													MxGlobalConfig::Instance()->GetVideoHeight(),//700,
 													this);
 		m_screenDataSender->Initialize();
 		m_screenDataSender->AddDestination(MxGlobalConfig::Instance()->GetServerIp(),
 										   MxGlobalConfig::Instance()->GetServerPort());
 		m_screenDataSender->StartSendScreenData();
 
 		connect(&m_timer, &QTimer::timeout, this, &SYTrainingCenterWindow::onSendSessionCommand);
 		m_timer.start(10000);
 
 		QTimer::singleShot(2000, Qt::VeryCoarseTimer, this, &SYTrainingCenterWindow::onSendSessionCommand);
 	}

	//m_demonstrationWindow = new MxDemonstrationWindow(this);

	//MXApplication* app = static_cast<MXApplication*>(qApp);
	//connect(app, &MXApplication::ReceiveMessage, this, &RbUserWindow::OnReceiveMessage);
	//connect(app, &MXApplication::ReceiveStop, this, &RbUserWindow::OnReceiveStop);

	//head
// 	QIcon headIcon;
// 	headIcon.addFile("icons:../SkinConfig/symodulewindow/men.png");
// 	ui.headBtn->setIcon(headIcon);
// 	ui.headBtn->setIconSize(QSize(100, 100));
	if(m_realName.size())
		ui.headBtn->setText(m_realName);
	else
		ui.headBtn->setText(m_userName);

	QIcon headIcon = SYUserInfo::Instance()->GetHeadIcon();

// 	if(SYUserInfo::Instance()->IsMale())
// 		headIcon.addFile("icons:/mainWindow/man2.png");
// 	else
// 		headIcon.addFile("icons:/mainWindow/women2.png");

	ui.headBtn->setIcon(headIcon);

	//knowledge lib
	connect(ui.knowledgeLibBtn, &QToolButton::clicked, this, &SYTrainingCenterWindow::onButtonClicked);

	//data center
	connect(ui.dataCenterBtn, &QToolButton::clicked, this, &SYTrainingCenterWindow::onButtonClicked);

	//answer
	connect(ui.answerBtn, &QToolButton::clicked, this, &SYTrainingCenterWindow::onButtonClicked);

	//person center
	connect(ui.personCenterBtn, &QToolButton::clicked, this, &SYTrainingCenterWindow::onButtonClicked);

	Mx::setWidgetStyle(this, "qss:SYTrainingCenterWindow.qss");
}

SYTrainingCenterWindow::~SYTrainingCenterWindow(void)
{
	m_timer.stop();

	if(m_screenDataSender)
	{
		QByteArray byteArray = MxSessionCommand(MxSessionCommand::CT_Exit).ToByteArray();
		
		m_screenDataSender->SendCommand(byteArray);
		m_screenDataSender->StopSendScreenData();
		delete m_screenDataSender;
	}

	if(m_demonstrationWindow)
		delete m_demonstrationWindow;
}

void SYTrainingCenterWindow::onButtonClicked()
{
	QPushButton* toolBtn = (QPushButton*)sender();

	if(m_preButton){
		if(toolBtn == m_preButton)
			m_preButton->setChecked(true);	
		else
			m_preButton->setChecked(false);
	}
	m_preButton = toolBtn;
}

void SYTrainingCenterWindow::onSendSessionCommand()
{
	if(m_screenDataSender)
	{
		MxSessionCommand cmd;
		QByteArray data;

		cmd.SetCommandType(MxSessionCommand::CT_Update);
		cmd.AddData("UserName", SYUserInfo::Instance()->GetUserName());
		cmd.AddData("RealName", SYUserInfo::Instance()->GetRealName());	//如果无真实姓名，那么此操作无效

		data = cmd.ToByteArray();
		m_screenDataSender->SendCommand((const uint8_t*)data.data(), data.size());
	}
}

void SYTrainingCenterWindow::on_knowledgeLibBtn_clicked()
{
	emit showNextWindow(WT_KnowLibWindow);
}

void SYTrainingCenterWindow::on_dataCenterBtn_clicked()
{
	emit showNextWindow(WT_DataCenterWindow);
}

void SYTrainingCenterWindow::on_answerBtn_clicked()
{
	//理论答题窗口
 	int errorCode = (static_cast<MXApplication*>(qApp))->LaunchQuestionModuleProcess();
 	if(errorCode != 0)
	{
        SYMessageBox * messageBox = new SYMessageBox(this, CHS(""), CHS("QuestionMouduleLaunchFailed"));
 		messageBox->showFullScreen();
		messageBox->exec();
 	}
}

void SYTrainingCenterWindow::on_personCenterBtn_clicked()
{
	emit showNextWindow(WT_PersonCenterWindow);
}

void SYTrainingCenterWindow::showEvent(QShowEvent* event)
{
	QIcon headIcon = SYUserInfo::Instance()->GetHeadIcon();
	ui.headBtn->setIcon(headIcon);

	m_clickedFrameIndex = -1;

	if(m_preButton){
		m_preButton->setChecked(false);
	}

	//update tain info
	int totalTime = 0;
	int trainTimes1 = 0;
	int trainTimes2 = 0;
	int trainTimes3 = 0;
	int userId = SYUserInfo::Instance()->GetUserId();
	bool ok = SYDBMgr::Instance()->QueryBasicTrainInfo(userId, totalTime, trainTimes1, trainTimes2, trainTimes3);
	if(!ok)
		return;

	QString info;
	//1
	int h = totalTime / 3600;
	int m = (totalTime - h * 3600) / 60;
	ui.hourLabel->setText(info.setNum(h));
	ui.minuteLabel->setText(info.setNum(m));
	//2
	ui.trainTimesLabel1->setText(info.setNum(trainTimes1));
	ui.trainTimesLabel2->setText(info.setNum(trainTimes2));
	ui.trainTimesLabel3->setText(info.setNum(trainTimes3));
}

void SYTrainingCenterWindow::LaunchRealTrainModule()
{
	std::string modulefile = MxGlobalConfig::Instance()->GetRealTrainModule();

	int errorCode = (static_cast<MXApplication*>(qApp))->LaunchModule(modulefile, "RealTrainModule", "", "实物训练模块", "",-1, -1,false);

	if (errorCode != 0)
	{
		SYMessageBox * messageBox = new SYMessageBox(this, CHS("实物训练模块启动失败"), CHS("error code : %1").arg(errorCode));
		messageBox->showFullScreen();
		messageBox->exec();
	}
}
void SYTrainingCenterWindow::mousePressEvent(QMouseEvent* mouseEvent)
{
	QFrame* frames[3] = {ui.moduleFrame1, ui.moduleFrame2, ui.moduleFrame3};
	WindowType windowTypes[3] = { WT_CourseTrainWindow, WT_SkillTrainingWindow, WT_SurgeyTrainWindow };
	QPoint screenPos, localPos;
	int index = -1;

	for(int i = 0; i < 3;++i){
		screenPos = mouseEvent->screenPos().toPoint();
		localPos = frames[i]->mapFromGlobal(screenPos);
		if(frames[i]->rect().contains(localPos)){
			index = i;
			break;
		}
	}

// 	if(index != -1){
// 		if (index == 0){//启动实物训练
// 			LaunchRealTrainModule();
// 		}
// 		else
// 		    emit showNextWindow(windowTypes[index]);
// 	}
	m_clickedFrameIndex = index;
}

void SYTrainingCenterWindow::mouseReleaseEvent(QMouseEvent* mouseEvent)
{
	QFrame* frames[3] = {ui.moduleFrame1, ui.moduleFrame2, ui.moduleFrame3};
	WindowType windowTypes[3] = {WT_CourseTrainWindow, WT_SkillTrainingWindow, WT_SurgeyTrainWindow};
	QPoint screenPos, localPos;
	int index = -1;

	for(int i = 0; i < 3; ++i){
		screenPos = mouseEvent->screenPos().toPoint();
		localPos = frames[i]->mapFromGlobal(screenPos);
		if(frames[i]->rect().contains(localPos)){
			index = i;
			break;
		}
	}

	if(index != -1 && index == m_clickedFrameIndex){
		if(index == 0){//启动实物训练
			LaunchRealTrainModule();
		}
		else
			emit showNextWindow(windowTypes[index]);
	}
}