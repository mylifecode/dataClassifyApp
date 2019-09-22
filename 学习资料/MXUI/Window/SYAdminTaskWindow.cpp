#include "SYAdminTaskWindow.h"
#include "MxDefine.h"
#include <SYDBMgr.h>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <initializer_list>
#include <SYUserInfo.h>
#include <SYMessageBox.h>
#include "SYStringTable.h"
#include "SYModifyTrainTaskWindow.h"
SYAdminTaskWindow::SYAdminTaskWindow(QWidget* parent)
	:QWidget(parent),
	m_curSelectedBtn(nullptr),
	m_TaskTable(0)
{
	ui.setupUi(this);

	m_curSelectedBtn = ui.taskMgrBtn;  //人员管理
	//ui.sendMessageBtn->setVisible(false);

	connect(ui.leftMenuWindow, &SYMenuWindow::showNextWindow, this, &SYAdminTaskWindow::showNextWindow);

	InitTables();

	RefreshTable();

	Mx::setWidgetStyle(this, "qss:SYAdminTaskWindow.qss");
}

SYAdminTaskWindow::~SYAdminTaskWindow()
{

}

void SYAdminTaskWindow::InitTables()
{
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

}

void SYAdminTaskWindow::LoadData()
{
	m_TasksTemplate.clear();

	//load user info
	SYTaskTrainDataMgr::GetInstance().PullAllTaskTemplatesBelongToAssignor(-1, m_TasksTemplate);
}

void SYAdminTaskWindow::SetTableContent()
{
	m_TaskTable->clearContents();
	const int rowHeight = 70;
	const QString dateTimeFormat = SYStringTable::GetString(SYStringTable::STR_DATETIMEFORMAT);
	auto SetItemAttribute = [](QTableWidgetItem* item,const QString& text,Qt::Alignment align){
		item->setText(text);
		item->setTextAlignment(align);
		item->setTextColor(QColor(0xb7, 0xc5, 0xd8));

		QFont font = item->font();
		font.setPixelSize(16);
		item->setFont(font);

		Qt::ItemFlags flags = item->flags();
		flags = flags & (~Qt::ItemIsEditable);
		item->setFlags(flags);
	};

	//bool canOperate = true;// SYUserInfo::Instance()->IsSuperManager();

	//任务表
	if (m_TaskTable)
	{
		m_TaskTable->setRowCount(m_TasksTemplate.size());

		int row = 0;
		for (int i = 0; i < m_TasksTemplate.size(); ++i)
		{
			SYTaskWork & task = m_TasksTemplate[i];

			if(FilterRecord(task) == false)
				continue;

			//col 0
			QTableWidgetItem * item = new QTableWidgetItem;
			item->setData(Qt::UserRole, task.m_ID);		//删除时，便于删除这一行的数据
			SetItemAttribute(item, task.m_TaskName, Qt::AlignHCenter | Qt::AlignVCenter);
			m_TaskTable->setItem(row, 0, item);

			//col 1
			item = new QTableWidgetItem;
			SetItemAttribute(item, task.m_TaskTag, Qt::AlignHCenter | Qt::AlignVCenter);
			m_TaskTable->setItem(row, 1, item);

			//col 2
			item = new QTableWidgetItem;
			SetItemAttribute(item, task.m_SendDate.toString(SYStringTable::GetString(SYStringTable::STR_DATEFORMAT)), Qt::AlignHCenter | Qt::AlignVCenter);
			m_TaskTable->setItem(row, 2, item);

			//col 3
			item = new QTableWidgetItem;
			SetItemAttribute(item, "0", Qt::AlignHCenter | Qt::AlignVCenter);
			m_TaskTable->setItem(row, 3, item);

			//col 4
			item = new QTableWidgetItem;
			SetItemAttribute(item, task.m_Assignor , Qt::AlignHCenter | Qt::AlignVCenter);
			m_TaskTable->setItem(row, 4, item);

			//col 5
			QWidget* widget = new QWidget;
			QHBoxLayout* hLayout = new QHBoxLayout;

			QPushButton* descBtn = new QPushButton();
			descBtn->setText(SYStringTable::GetString(SYStringTable::STR_Check));
			descBtn->setObjectName("descBtn");
			descBtn->setEnabled(true);
			descBtn->setProperty("index", row);
			connect(descBtn, &QPushButton::clicked, this, &SYAdminTaskWindow::onDescTaskTemplate);


			QPushButton* deleteBtn = new QPushButton();
			deleteBtn->setText(SYStringTable::GetString(SYStringTable::STR_Delete));
			deleteBtn->setObjectName("deleteBtn");
			bool permitdel = task.m_Assignorid == SYUserInfo::Instance()->m_userId ? true : false;
			deleteBtn->setEnabled(permitdel);
			if (permitdel == false)
			{
				deleteBtn->setStyleSheet("color:#AAAAAA;");
			}
			deleteBtn->setProperty("index", row);
			connect(deleteBtn, &QPushButton::clicked, this, &SYAdminTaskWindow::onDeleteTaskTemplate);


			hLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
			hLayout->addWidget(descBtn);
			hLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
			hLayout->addWidget(deleteBtn);
			hLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
			widget->setLayout(hLayout);

			m_TaskTable->setCellWidget(row, 5, widget);
			m_TaskTable->setRowHeight(row, rowHeight);

			++row;
		}

		m_TaskTable->setRowCount(row);
	}
}

void SYAdminTaskWindow::RefreshTable()
{
	//reload
	LoadData();
	SetTableContent();
}

bool SYAdminTaskWindow::FilterRecord(const SYTaskWork& task)
{
	QString text = ui.searchLineEdit->text();
	if(text.size() == 0)
		return true;

	if(task.m_TaskName.contains(text, Qt::CaseInsensitive))
		return true;

	if(task.m_TaskTag.contains(text, Qt::CaseInsensitive))
		return true;

	if(task.m_SendDate.toString(SYStringTable::GetString(SYStringTable::STR_DATEFORMAT)).contains(text, Qt::CaseInsensitive))
		return true;

	if(text == "0")
		return true;

	if(task.m_Assignor.contains(text, Qt::CaseInsensitive))
		return true;

	return false;
}

void SYAdminTaskWindow::on_taskMgrBtn_clicked()
{
	ui.taskMgrBtn->setChecked(true);
}

void SYAdminTaskWindow::on_newTaskBtn_clicked()
{
	SYModifyTrainTaskWindow * addTaskWindow = new SYModifyTrainTaskWindow(this);
	addTaskWindow->showFullScreen();
	int button = addTaskWindow->exec();
	if (button ==    2)
	{
		RefreshTable();
	}
}

void SYAdminTaskWindow::on_sendTaskBtn_clicked()
{
	showNextWindow(WT_AdminSendTaskWindow);
}

void SYAdminTaskWindow::on_searchBtn_clicked()
{
	SetTableContent();
}

void SYAdminTaskWindow::showEvent(QShowEvent* event)
{
	if(m_curSelectedBtn)
		m_curSelectedBtn->setChecked(true);

	ui.leftMenuWindow->setCurSelectedItem(WT_AdminTaskWindow);
}

void SYAdminTaskWindow::keyReleaseEvent(QKeyEvent* event)
{
	int key = event->key();
	if(key == Qt::Key_Enter || key == Qt::Key_Return)
		on_searchBtn_clicked();
}

void SYAdminTaskWindow::onDeleteTaskTemplate()
{
	QPushButton* button = static_cast<QPushButton*>(sender());
}

void SYAdminTaskWindow::onDescTaskTemplate()
{
	QPushButton* button = static_cast<QPushButton*>(sender());

	bool ok;
	
	int index = button->property("index").toInt(&ok);

	if (ok)
	{
		if (m_TasksTemplate[index].m_TrainUnits.size() == 0)
		   SYTaskTrainDataMgr::GetInstance().PullAllTrainsBelongsToTask(m_TasksTemplate[index]);

		SYTrainTaskDescWindow * descwindow = new SYTrainTaskDescWindow(this, m_TasksTemplate[index]);
		descwindow->showFullScreen();
		descwindow->exec();
	}
}



SYTrainTaskDescWindow::SYTrainTaskDescWindow(QWidget *parent, const SYTaskWork & taskWork) : RbShieldLayer(parent)
{
	ui.setupUi(this);
	hideCloseButton();
	hideOkButton();

	setAttribute(Qt::WA_DeleteOnClose);
		
	RefreshUI(taskWork);
	
	Mx::setWidgetStyle(this, "qss:SYAdminTaskWindow.qss");
}

SYTrainTaskDescWindow::~SYTrainTaskDescWindow()
{

}

void SYTrainTaskDescWindow::RefreshUI(const SYTaskWork & taskWork)
{
	QString numDay   = SYStringTable::GetString(SYStringTable::STR_NUMDAY).arg(taskWork.m_TaskTimeDay);
	QString numTrain = SYStringTable::GetString(SYStringTable::STR_AMOUNT).arg(taskWork.m_TrainUnits.size());

	ui.lb_day->setText(numDay);
	ui.lb_num->setText(numTrain);
	ui.lb_tag->setText(taskWork.m_TaskTag);
	ui.lb_name->setText(taskWork.m_TaskName);
	
	QStringList labels;
	labels.append(SYStringTable::GetString(SYStringTable::STR_TRAINNAME));
	labels.append(SYStringTable::GetString(SYStringTable::STR_PASSNEEDSCORE));
	labels.append(SYStringTable::GetString(SYStringTable::STR_NEEDPASSTIMES));
	
	ui.tblist_trains->setColumnCount(3);
	ui.tblist_trains->setHorizontalHeaderLabels(labels);
	ui.tblist_trains->setFrameShape(QFrame::NoFrame);

	QHeaderView* headerView = ui.tblist_trains->horizontalHeader();
	headerView->setObjectName("horizontalHeader");
	headerView->setEnabled(false);

	//set section width
	for (std::size_t i = 0; i < 3; ++i)
	{
		ui.tblist_trains->setColumnWidth(i, 250);
	}

	headerView = ui.tblist_trains->verticalHeader();
	headerView->hide();

	ui.tblist_trains->setRowCount(taskWork.m_TrainUnits.size());
	
	auto SetItemAttribute = [](QTableWidgetItem* item, const QString& text, Qt::Alignment align){
		item->setText(text);
		item->setTextAlignment(align);
		item->setTextColor(QColor(0xb7, 0xc5, 0xd8));

		QFont font = item->font();
		font.setPixelSize(16);
		item->setFont(font);

		Qt::ItemFlags flags = item->flags();
		flags = flags & (~Qt::ItemIsEditable);
		item->setFlags(flags);
	};
	
	TrainCanBeAssign trainInfo;

	const int rowHeight = 70;

	for (int row = 0; row < taskWork.m_TrainUnits.size(); ++row)
	{
		const TaskTrainUnit & train = taskWork.m_TrainUnits[row];

		if (SYTaskTrainDataMgr::GetInstance().GetTrainInforBySheetCode(train.m_ScoreCode, trainInfo))
		{
			//col 0
			QTableWidgetItem * item = new QTableWidgetItem;
			SetItemAttribute(item, trainInfo.m_ShowName, Qt::AlignHCenter | Qt::AlignVCenter);
			ui.tblist_trains->setItem(row, 0, item);

			//col 1
			item = new QTableWidgetItem;
			SetItemAttribute(item, QString::number(train.m_PassNeedScore), Qt::AlignHCenter | Qt::AlignVCenter);
			ui.tblist_trains->setItem(row, 1, item);

			//col 2
			item = new QTableWidgetItem;
			SetItemAttribute(item, QString::number(train.m_NeedPassTimes), Qt::AlignHCenter | Qt::AlignVCenter);
			ui.tblist_trains->setItem(row, 2, item);

			ui.tblist_trains->setRowHeight(row, rowHeight);
		}
	}

}
void SYTrainTaskDescWindow::on_bt_conform_clicked()
{
	done(2);
}