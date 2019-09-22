#include "SYAdminTrainingCenterWindow.h"
#include <MxDefine.h>
#include "SYUserInfo.h"
#include <SYDBMgr.h>
#include "MxGlobalConfig.h"
#include "MXApplication.h"
#include <SYMessageBox.h>

SYAdminTrainingCenterWindow::SYAdminTrainingCenterWindow(QWidget* parent)
	:QWidget(parent),
	m_clickedFrameIndex(-1)
{
	ui.setupUi(this);

	connect(ui.leftMenuWindow, &SYMenuWindow::showNextWindow, this, &SYAdminTrainingCenterWindow::showNextWindow);

	Mx::setWidgetStyle(this, "qss:SYAdminTrainingCenterWindow.qss");
}

SYAdminTrainingCenterWindow::~SYAdminTrainingCenterWindow()
{

}

void SYAdminTrainingCenterWindow::showEvent(QShowEvent* event)
{
	ui.leftMenuWindow->setCurSelectedItem(WT_AdminTrainingCenterWindow);

	m_clickedFrameIndex = -1;

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

void SYAdminTrainingCenterWindow::LaunchRealTrainModule()
{
	std::string modulefile = MxGlobalConfig::Instance()->GetRealTrainModule();

	int errorCode = (static_cast<MXApplication*>(qApp))->LaunchModule(modulefile, "RealTrainModule", "", "实物训练模块","", -1, -1,false);

	if(errorCode != 0)
	{
		SYMessageBox * messageBox = new SYMessageBox(this, CHS("实物训练模块启动失败"), CHS("error code : %1").arg(errorCode));
		messageBox->showFullScreen();
		messageBox->exec();
	}
}
void SYAdminTrainingCenterWindow::mousePressEvent(QMouseEvent* mouseEvent)
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

	m_clickedFrameIndex = index;
}

void SYAdminTrainingCenterWindow::mouseReleaseEvent(QMouseEvent* mouseEvent)
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