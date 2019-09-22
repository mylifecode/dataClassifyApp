#include "SYAdminSendTaskWindow.h"
#include "MxDefine.h"
#include <SYDBMgr.h>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <initializer_list>
#include <SYUserInfo.h>
#include <SYMessageBox.h>
#include "SYStringTable.h"
#include "SYAdminSendTaskConfirmDlg.h"



SYAdminSendTaskWindow::SYAdminSendTaskWindow(QWidget* parent) :QWidget(parent),
	m_curSelectedBtn(nullptr),
	m_TaskTable(0)
{
	ui.setupUi(this);

	//m_curSelectedBtn = ui.taskMgrBtn;
	//ui.sendMessageBtn->setVisible(false);

	//connect(ui.leftMenuWindow, &SYMenuWindow::showNextWindow, this, &SYAdminTaskWindow::showNextWindow);

	//InitTables();

	//RefreshTable();
	SetTableContent();

	Mx::setWidgetStyle(this, "qss:SYAdminSendTaskWindow.qss");

	connect(ui.checkBox, &QCheckBox::stateChanged, this, &SYAdminSendTaskWindow::on_checkbox_statechanged);
	connect(ui.checkBox_2, &QCheckBox::stateChanged, this, &SYAdminSendTaskWindow::on_checkbox_statechanged);

}

SYAdminSendTaskWindow::~SYAdminSendTaskWindow()
{

}

void SYAdminSendTaskWindow::InitTables()
{
	/*
	auto InitImpl = [](QTableWidget* table,int columnCount,std::initializer_list<QString> horizontalLabels,std::vector<int> sectionWidths){
		table->setColumnCount(columnCount);

		QStringList labels;
		for(const auto& lb : horizontalLabels)
			labels.append(lb);
		table->setHorizontalHeaderLabels(labels);

		table->setFrameShape(QFrame::NoFrame);
		table->setShowGrid(false);
		table->setSelectionMode(QAbstractItemView::SingleSelection);
		table->setSelectionBehavior(QAbstractItemView::SelectRows);
		table->setAlternatingRowColors(true);
		table->setFocusPolicy(Qt::NoFocus);

		QHeaderView* headerView = table->horizontalHeader();
		headerView->setStretchLastSection(true);
		headerView->setHighlightSections(false);
		headerView->setObjectName("horizontalHeader");
		//headerView->setSectionsMovable(false);
		headerView->setEnabled(false);

		//set section width
		for(std::size_t i = 0; i < sectionWidths.size(); ++i){
			table->setColumnWidth(i, sectionWidths[i]);
		}

		headerView = table->verticalHeader();
		headerView->hide();
	};
	QStringList headerLabels;

	//教师表
	m_TaskTable = new QTableWidget;

	InitImpl(m_TaskTable,
			 6,
			 { SYStringTable::GetString(SYStringTable::STR_TASKNAME), 
			   SYStringTable::GetString(SYStringTable::STR_TASKTAG), 
			   SYStringTable::GetString(SYStringTable::STR_CREATEDATE), 
			   SYStringTable::GetString(SYStringTable::STR_USEDNUM),
			   SYStringTable::GetString(SYStringTable::STR_CREATROR), 
			   SYStringTable::GetString(SYStringTable::STR_OPERATION) },
			 {330, 170, 250, 180, 170, 400});
	ui.stackedWidget->addWidget(m_TaskTable);
	*/
}

void SYAdminSendTaskWindow::SetTableContent()
{
	
	SYTaskTrainDataMgr::GetInstance().PullAllTaskTemplatesBelongToAssignor(-1, m_TasksTemplate);

	ui.list_task->clear();

	int ItemFixHeight = 70;
	int ItemHintWidth = 700;
	for (int c = 0; c < m_TasksTemplate.size(); c++)
	{
		SYSendTaskListItem * itemFrame = new SYSendTaskListItem(ui.list_task, c);
		
		itemFrame->Create(ItemFixHeight, m_TasksTemplate[c].m_TaskName, m_TasksTemplate[c].m_TaskTag, m_TasksTemplate[c].m_Assignor);  //
		
		itemFrame->m_ID = m_TasksTemplate[c].m_ID;  // 

		QListWidgetItem  * listWidgetItem = new QListWidgetItem(ui.list_task);
		
		listWidgetItem->setSizeHint(QSize(ItemHintWidth, ItemFixHeight));

		ui.list_task->setItemWidget(listWidgetItem, itemFrame);
	}


	//
	ui.list_student->clear();
	
	std::vector<QSqlRecord> userRecords;
	
	SYDBMgr::Instance()->QueryAllUserInfo(userRecords);
	
	for (int c = 0; c < userRecords.size(); c++)
	{
		QString realName = userRecords[c].value("realName").toString();

		int Permission = userRecords[c].value("permission").toInt();

		int ID = userRecords[c].value("id").toInt();

		if (Permission == UP_Student)
		{
			SYSendTaskListItem * itemFrame = new SYSendTaskListItem(ui.list_student, c);

			itemFrame->Create(ItemFixHeight, realName, "-", "-");

			itemFrame->m_ID = ID;

			itemFrame->m_UserName = realName;

			QListWidgetItem  * listWidgetItem = new QListWidgetItem(ui.list_student);

			listWidgetItem->setSizeHint(QSize(ItemHintWidth, ItemFixHeight));

			ui.list_student->setItemWidget(listWidgetItem, itemFrame);
		}
	}
}

void SYAdminSendTaskWindow::on_checkbox_statechanged(int state)
{
	QListWidget* listWidget;
	QCheckBox* box = (QCheckBox*)sender();
	if (box == ui.checkBox)
		listWidget = ui.list_task;
	else
		listWidget = ui.list_student;

	for (int c = 0; c < listWidget->count(); c++)
	{
		QListWidgetItem *item = listWidget->item(c);
		SYSendTaskListItem * itemFrame = dynamic_cast<SYSendTaskListItem *>(listWidget->itemWidget(item));
		if (itemFrame)
		{
			itemFrame->SetChecked(state);
		}
	}

}

void SYAdminSendTaskWindow::on_bt_conform_clicked()
{
	QStringList taskstrinList;
	std::vector<SYTaskWork> taskSelected;

	for (int c = 0; c < ui.list_task->count(); c++)
	{
		QListWidgetItem *item = ui.list_task->item(c);

		SYSendTaskListItem * itemFrame = dynamic_cast<SYSendTaskListItem *>(ui.list_task->itemWidget(item));

		if (itemFrame && itemFrame->IsChecked())
		{
			taskstrinList.append(itemFrame->m_Label0->text());
			taskSelected.push_back(m_TasksTemplate[c]);
		}
	}

	QStringList studentStrList;
	std::vector<std::pair<int, QString>> studentSelected;

	for (int c = 0; c < ui.list_student->count(); c++)
	{
		QListWidgetItem *item = ui.list_student->item(c);

		SYSendTaskListItem * itemFrame = dynamic_cast<SYSendTaskListItem *>(ui.list_student->itemWidget(item));

		if (itemFrame && itemFrame->IsChecked())
		{
			studentStrList.append(itemFrame->m_Label0->text());
			studentSelected.push_back(std::make_pair(itemFrame->m_ID, itemFrame->m_UserName));
		}
	}

	if (taskSelected.size() == 0)
	{
		SYMessageBox * messageBox = new SYMessageBox(this, CHS(""), CHS("您没有选择任务"), 1);
		messageBox->showFullScreen();
		messageBox->exec();
		return;
	}

	else if (studentSelected.size() == 0)
	{
		SYMessageBox * messageBox = new SYMessageBox(this, CHS(""), CHS("您没有选择学生"), 1);
		messageBox->showFullScreen();
		messageBox->exec();
		return;
	}

	SYProcessingDlg * process = new SYProcessingDlg(this);

	TaskSendingThread * sendthread = new TaskSendingThread(process);
	sendthread->userSelected = studentSelected;
	sendthread->taskSelected = taskSelected;

	SYAdminSendTaskConfirmDlg * dlg = new SYAdminSendTaskConfirmDlg(this, taskstrinList, studentStrList);
	dlg->showFullScreen();
	int button = dlg->exec();
	if (button == 2)
	{
		sendthread->start();
		process->showFullScreen();
		process->exec();
		sendthread->stop();
	}
}
