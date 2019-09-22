#include "MxDefine.h"
#include "SYMessageBox.h"
#include "MxGlobalConfig.h"
#include "SYUserInfo.h"
#include "SYPersonTaskListsWindow.h"
#include "SYTrainTaskStruct.h"
#include "SYStringTable.h"
#include "MXApplication.h"
SYPersonTaskListItem::SYPersonTaskListItem(QWidget * parent, int itemindex) : QFrame(parent)
{
	m_itemIndex = itemindex;
	m_lb_name = m_lb_class = 0;
}

SYPersonTaskListItem::~SYPersonTaskListItem()
{

}

void SYPersonTaskListItem::Create(const QString & trainName, const QString & trainsState, int ItemFixHeight, const QString & skinDir)
{
	setFixedHeight(ItemFixHeight);
	
	m_lb_name  = new QLabel(this);
	
	m_lb_class = new QLabel(this);

	//user name
	m_lb_name->move(15 + 16, 0);
	m_lb_name->setFixedHeight(ItemFixHeight);
	m_lb_name->setFixedWidth(250);
	m_lb_name->setText(trainName);
	m_lb_name->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	m_lb_name->setStyleSheet(QString("font-size:16px; color:#b7c5d8;").arg(skinDir));

	//user class
	m_lb_class->move(15 + 16 + 250, 0);
	m_lb_class->setFixedHeight(ItemFixHeight);
	m_lb_class->setFixedWidth(120);
	m_lb_class->setText(trainsState);
	m_lb_class->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	m_lb_class->setStyleSheet(QString("font-size:16px; color:#b7c5d8;").arg(skinDir));

}

SYPersonTrainListItem::SYPersonTrainListItem(QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this);
	Mx::setWidgetStyle(this, "qss:SYPersonTaskListItem.qss");

	m_TaskID = m_TaskTrainID = -1;
	m_TrainRunName = m_TrainShowName = "";
}

SYPersonTrainListItem::~SYPersonTrainListItem()
{

}

void SYPersonTrainListItem::SetContent(const QString & name, const QString &  totalTrainTime, const QString & trainPeriod, int NeedPassTime, int CurrPassTime, int CompleteState)
{
	ui.lb_trainname->setText(name);
	ui.lb_trainminutes->setText(totalTrainTime);
	ui.lb_traingiventime->setText(trainPeriod);
	ui.lb_trainpercent->setText(QString::number(CurrPassTime) + QString("/") + QString::number(NeedPassTime));

	QString stateStr;

	if (NeedPassTime <= CurrPassTime)
	{
		stateStr = SYStringTable::GetString(SYStringTable::STR_FINISH);
		ui.lb_trainstate->setStyleSheet(QString("color:#22c874;"));
		ui.bt_starttrain->setStyleSheet(QString("color:#ffffff;"));
	}
	else
	{
		stateStr = SYStringTable::GetString(SYStringTable::STR_NOTFINISH);
		ui.lb_trainstate->setStyleSheet(QString("color:#ff4242;"));
	}
	
	ui.lb_trainstate->setText(stateStr);

}

void SYPersonTrainListItem::SetTrainLaunchData(const QString & runName, const QString & showName, int TaskID, int TaskTrainID)
{
	m_TrainRunName = runName;
	m_TrainShowName = showName;
	m_TaskID = TaskID;
	m_TaskTrainID = TaskTrainID;
}
void SYPersonTrainListItem::on_bt_starttrain_clicked()
{
	std::string modulefile = MxGlobalConfig::Instance()->GetVirtualTrainModule();

	int errorCode = (static_cast<MXApplication*>(qApp))->LaunchModule(modulefile,
		                                                              m_TrainRunName,
																	  m_TrainShowName,
																	  m_TrainRunName,
																	  "",
		                                                              m_TaskID,
		                                                              m_TaskTrainID,
																	  true);

	if (errorCode != 0)
	{
		SYMessageBox * messageBox = new SYMessageBox(this, CHS("技能训练模块启动失败"), CHS("error code : %1").arg(errorCode));
		messageBox->showFullScreen();
		messageBox->exec();
	}
}
SYPersonTaskListsWindow::SYPersonTaskListsWindow(QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this);

	m_userName = SYUserInfo::Instance()->GetUserName();
	m_realName = SYUserInfo::Instance()->GetRealName();

	QIcon headIcon = SYUserInfo::Instance()->GetHeadIcon();

	Mx::setWidgetStyle(this, "qss:SYPersonTaskListsWindow.qss");

	connect(ui.list_tasks, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onClickeTaskItem(QListWidgetItem*)));

	int taksid = SYTaskTrainDataMgr::GetInstance().GetSelectedTaskID();

	m_CurrSelectedItem = RefrehTasksList(taksid);
	
}

SYPersonTaskListsWindow::~SYPersonTaskListsWindow(void)
{
	
}

void SYPersonTaskListsWindow::onClickeTaskItem(QListWidgetItem *item)
{
	int index = ui.list_tasks->row(item);
	if (index >= 0 && index < m_allTasks.size())
	{
		RefreshTaskDetail(m_allTasks[index]);
		m_CurrSelectedItem = index;
	}
}
int SYPersonTaskListsWindow::RefrehTasksList(int selectedID)
{
	int SelectedItem = -1;

	QString skinDir = MxGlobalConfig::Instance()->GetSkinDir();
	
	int ItemFixHeight = 60;

	int userId = SYUserInfo::Instance()->GetUserId();

	
	SYTaskTrainDataMgr::GetInstance().PullAllTasksBelongsToReceiver(userId, m_allTasks);

	ui.list_tasks->clear();
	
	for (int c = 0; c < m_allTasks.size(); c++)
	{
		SYPersonTaskListItem * itemFrame = new SYPersonTaskListItem(ui.list_tasks, c);
		itemFrame->Create(m_allTasks[c].m_TaskName, m_allTasks[c].m_StateStr, ItemFixHeight, skinDir);

		QListWidgetItem  * listWidgetItem = new QListWidgetItem(ui.list_tasks);
		listWidgetItem->setSizeHint(QSize(375, ItemFixHeight));

		ui.list_tasks->setItemWidget(listWidgetItem, itemFrame);
		
		if (SelectedItem < 0 && m_allTasks[c].m_ID == selectedID)
		{
			SelectedItem = c;
			listWidgetItem->setSelected(true);
		}
	}
	return SelectedItem;
}
void SYPersonTaskListsWindow::RefreshTaskDetail(SYTaskWork & taskWork)
{
	QString datestartStr = taskWork.m_SendDate.toString(SYStringTable::GetString(SYStringTable::STR_DATEFORMAT));
	QString dateendStr = taskWork.m_SendDate.addDays(taskWork.m_TaskTimeDay).toString(SYStringTable::GetString(SYStringTable::STR_DATEFORMAT));
	QString trainPeriod = SYStringTable::GetString(SYStringTable::STR_NUMDAY).arg(taskWork.m_TaskTimeDay);
	//
	ui.lb_taskname->setText(taskWork.m_TaskName);
	ui.lb_tasktag->setText(taskWork.m_TaskTag);
	ui.lb_assignor->setText(taskWork.m_Assignor);
	ui.lb_sendata->setText(taskWork.m_SendDate.toString(SYStringTable::GetString(SYStringTable::STR_DATEFORMAT)));


	ui.lb_statrdate->setText(datestartStr);
	ui.lb_enddate->setText(dateendStr);
	ui.lb_period->setText(trainPeriod);

	if (taskWork.m_TrainUnits.size() == 0)
	    SYTaskTrainDataMgr::GetInstance().PullAllTrainsBelongsToTask(taskWork);

	ui.list_trains->clear();
	
	for (int c = 0; c < taskWork.m_TrainUnits.size(); c++)
	{
		TaskTrainUnit & trainUnit = taskWork.m_TrainUnits[c];

		QString trainedMinutes = SYStringTable::GetString(SYStringTable::STR_NUMMINUTE).arg(trainUnit.m_TotalTrainedTime);

		TrainCanBeAssign trainInfo;
		
		if (SYTaskTrainDataMgr::GetInstance().GetTrainInforBySheetCode(trainUnit.m_ScoreCode, trainInfo))
		{
			QListWidgetItem  * listWidgetItem = new QListWidgetItem(ui.list_trains);

			SYPersonTrainListItem * theWidgetItem = new SYPersonTrainListItem;
			
			theWidgetItem->SetContent(trainInfo.m_ShowName, trainedMinutes, trainPeriod, trainUnit.m_NeedPassTimes, trainUnit.m_CurrPassedTimes, 0);
			
			theWidgetItem->SetTrainLaunchData(trainInfo.m_RunName, trainInfo.m_ShowName, taskWork.m_ID, trainUnit.m_DataBaseID);
			//Making sure that the listWidgetItem has the same size as the TheWidgetItem
			listWidgetItem->setSizeHint(QSize(370, 184));

			//Finally adding the itemWidget to the list
			ui.list_trains->setItemWidget(listWidgetItem, theWidgetItem);
		}
	}
}



void SYPersonTaskListsWindow::showEvent(QShowEvent* event)
{
	if (m_CurrSelectedItem >= 0 && m_CurrSelectedItem < m_allTasks.size())
	{
		m_allTasks[m_CurrSelectedItem].m_TrainUnits.clear();//in show event force re-pull from data space
		RefreshTaskDetail(m_allTasks[m_CurrSelectedItem]);
	}
}

void SYPersonTaskListsWindow::mousePressEvent(QMouseEvent* mouseEvent)
{
	
}
