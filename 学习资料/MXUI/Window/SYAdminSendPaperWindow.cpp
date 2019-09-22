#include "SYAdminSendPaperWindow.h"
#include "ui_SYAdminSendPaperWindow.h"
#include"SYAdminSendTaskWindow.h"
#include "MxDefine.h"
#include"SYMessageBox.h"
#include <SYDBMgr.h>
#include<QListWidget>
#include "SYStringTable.h"
#include"SYUserInfo.h"

#include "SYAdminSendTaskConfirmDlg.h"
#include "SYProcessingDlg.h"
#include"SYAdminSendTaskWindow.h"
#include"SYConfirmSendPaperWindow.h"


SYAdminSendPaperWindow::SYAdminSendPaperWindow(QWidget *parent) :
RbShieldLayer(parent), m_curBtn(NULL)
{
    ui.setupUi(this);
 
	hideCloseButton();
	hideOkButton();
	setAttribute(Qt::WA_DeleteOnClose);

	Mx::setWidgetStyle(this, QString("qss:SYAdminSendPaperWindow.qss"));

	Initialize();

	connect(ui.bt_confirm, &QPushButton::clicked, this, &SYAdminSendPaperWindow::on_confirm_Btn_Clicked);
	connect(ui.checkBox, &QCheckBox::stateChanged, this, &SYAdminSendPaperWindow::on_checkbox_statechanged);
	connect(ui.checkBox_2, &QCheckBox::stateChanged, this, &SYAdminSendPaperWindow::on_checkbox_statechanged);
	connect(ui.bt_return, &QPushButton::clicked, this, [=]()
	{
		done(RbShieldLayer::RC_Ok);
	});
	connect(ui.bt_student, &QPushButton::clicked, this, &SYAdminSendPaperWindow::on_btn_clicked);
	connect(ui.bt_group, &QPushButton::clicked, this, &SYAdminSendPaperWindow::on_btn_clicked);

}

SYAdminSendPaperWindow::~SYAdminSendPaperWindow()
{

}

void SYAdminSendPaperWindow::Initialize()
{
	ui.bt_group->setCheckable(true);
	ui.bt_student->setCheckable(true);
	m_curBtn = ui.bt_student;
	m_curBtn->setChecked(true);
	ui.bt_group->setChecked(false);

	//隐藏发送小组
	//ui.bt_group->hide();
	QVector<QSqlRecord> records;
	SYDBMgr::Instance()->QueryInfo(SYDBMgr::DatabaseTable::DT_ExamPaperList, records);


	int ItemFixedHeight = 70;
	int ItemFixedWidth = 680;

	for (std::size_t i = 0; i < records.size(); i++)
	{
		//左边listwidget
		QString paperName = records[i].value("paperName").toString();
		QString examRank = records[i].value("paperRank").toString();
		QString creator = records[i].value("creator").toString();

		SYListWidgetFrame* widget = new SYListWidgetFrame(i,ui.list_task);;
		widget->Create(ItemFixedHeight, paperName, examRank, creator);
		QListWidgetItem *item = new QListWidgetItem(ui.list_task);
		item->setSizeHint(QSize(ItemFixedWidth, ItemFixedHeight));
		ui.list_task->setItemWidget(item, widget);

	}

	connect(ui.list_task, &QListWidget::itemClicked, this, [=](QListWidgetItem*item)
	{

		SYListWidgetFrame* widget = dynamic_cast<SYListWidgetFrame*>(ui.list_task->itemWidget(item));
		if (widget)
		{
			bool flag = widget->IsChecked();
			widget->SetChecked(!flag);
		}

	});

	connect(ui.list_taskReceived, &QListWidget::itemClicked, this, [=](QListWidgetItem*item)
	{

		SYListWidgetFrame* widget = dynamic_cast<SYListWidgetFrame*>(ui.list_taskReceived->itemWidget(item));
		if (widget)
		{
			bool flag = widget->IsChecked();
			widget->SetChecked(!flag);
		}
	});

	refreshTaskReceivedList();

}


void SYAdminSendPaperWindow::refreshTaskReceivedList()
{
	if (m_curBtn == ui.bt_student)
	{
		ui.stuNameLabel->setText(CHS("学员姓名"));
		ui.taskAssginLabel->setText(CHS("任务分配"));
	
		ui.courseAssignLabel->setText(CHS("课程分配"));
	}
	if (m_curBtn == ui.bt_group)
		
	{
		ui.stuNameLabel->setText(CHS("小组名称"));
		ui.taskAssginLabel->setText(CHS("任务分配"));
		ui.courseAssignLabel->setText(CHS("课程分配"));
	}

	int ItemFixedHeight = 70;
	int ItemFixedWidth = 680;

	ui.list_taskReceived->clear();

	QVector<QSqlRecord> records;

	if (m_curBtn == ui.bt_student)
	{
		std::vector<QSqlRecord> stdRecords = records.toStdVector();
		SYDBMgr::Instance()->QueryAllUserInfo(stdRecords);
		for (auto& record : stdRecords)
		{
			records.push_back(record);
		}
	}
	else
	{

		SYDBMgr::Instance()->QueryAllGroupInfo(records);
	}
	

	for (int c = 0; c < records.size(); c++)
	{
		QString firstItem, secordItem, thirdItem;

		if (m_curBtn == ui.bt_student)
		{
			firstItem = records[c].value("realName").toString();
			//secordItem = records[c].value("taskAssignNum").toString();
		//	ThirdItem = records[c].value("courseAssignNum").toString();
			secordItem = CHS("_");
			thirdItem = CHS("_");
		}
		else
		{
			firstItem = records[c].value("name").toString();
			secordItem = records[c].value("missionnum").toString();
		    thirdItem = records[c].value("coursenum").toString();
			//secordItem = CHS("_");
			//thirdItem = CHS("_");

		}
		//QString realName = userRecords[c].value("realName").toString();

		//int Permission = userRecords[c].value("permission").toInt();

		//int ID = userRecords[c].value("id").toInt();
		//QString taskAssignNum = userRecords[c].value("").toString();

		//if (Permission == UP_Student)
		//{
			SYListWidgetFrame * itemFrame = new SYListWidgetFrame(c, ui.list_taskReceived);



			itemFrame->Create(ItemFixedHeight, firstItem, secordItem, thirdItem);

			//itemFrame->m_ID = ID;

			//itemFrame->m_UserName = realName;

			QListWidgetItem  * listWidgetItem = new QListWidgetItem(ui.list_taskReceived);

			listWidgetItem->setSizeHint(QSize(ItemFixedWidth, ItemFixedHeight));

			ui.list_taskReceived->setItemWidget(listWidgetItem, itemFrame);
	//	}
	}
}

void SYAdminSendPaperWindow::on_btn_clicked()
{
	QPushButton* btn = static_cast<QPushButton*>(sender());

	if (m_curBtn == btn)
	{
		m_curBtn->setChecked(true);
		return;
	}

	m_curBtn->setChecked(false);
	m_curBtn = btn;
	m_curBtn->setChecked(true);

	ui.checkBox_2->setChecked(false);
	refreshTaskReceivedList();
}

void SYAdminSendPaperWindow::on_confirm_Btn_Clicked()
{
	QVector<QString> senders;
	QVector<QString> receivers;
	for (std::size_t i = 0; i < ui.list_task->count(); i++)
	{
		QListWidgetItem *item = ui.list_task->item(i);
		SYListWidgetFrame* widget = dynamic_cast<SYListWidgetFrame*>(ui.list_task->itemWidget(item));
		if (widget)
		{
			if (widget->IsChecked())
			{
				QString sender = widget->GetFrameImInfo();
				senders.push_back(sender);
			}
		}
	}

	for (std::size_t i = 0; i < ui.list_taskReceived->count(); i++)
	{
		QListWidgetItem *item = ui.list_taskReceived->item(i);
		SYListWidgetFrame* widget = dynamic_cast<SYListWidgetFrame*>(ui.list_taskReceived->itemWidget(item));
		if (widget)
		{
			if (widget->IsChecked())
			{
				QString receiver = widget->GetFrameImInfo();
				receivers.push_back(receiver);
			}
		}
	}

	QString content_tips;
	if (senders.size() == 0 || receivers.size() == 0)
	{
		if (senders.size() == 0)
			content_tips = SYStringTable::GetString(SYStringTable::STR_PaperNotChosed);
		else if (receivers.size() == 0)
			content_tips = SYStringTable::GetString(SYStringTable::STR_AssignorNotChosed);
		SYMessageBox* messageBox = new SYMessageBox(this, "", content_tips, 1);
		messageBox->setPicture("Shared/personnel_pic_.png");  //图片需要修改
		messageBox->showFullScreen();
		bool ok = messageBox->exec();
		return;
	}

	bool flag = true;
	SYConfirmSendPaperWindow* confirmSendWin = new SYConfirmSendPaperWindow(senders, receivers, this);

	confirmSendWin->showFullScreen();
	int ok=confirmSendWin->exec();
	if (ok)
	{
		if (m_curBtn == ui.bt_student)
		{
			//写入数据库
			QString assigner = SYUserInfo::Instance()->GetRealName();
			//页面处理
			flag=SYDBMgr::Instance()->AssignPapersToUsers(senders, receivers, assigner);

		}
		if (m_curBtn == ui.bt_group)
		{

			//写入数据库
			QString assigner = SYUserInfo::Instance()->GetRealName();
			//页面处理
			flag=SYDBMgr::Instance()->AssignPapersToGroups(senders, receivers, assigner);


		}


		for (std::size_t i = 0; i < ui.list_task->count(); i++)
		{
			QListWidgetItem *item = ui.list_task->item(i);
			SYListWidgetFrame* widget = dynamic_cast<SYListWidgetFrame*>(ui.list_task->itemWidget(item));
			if (widget)
				widget->SetChecked(false);
		}

		for (std::size_t i = 0; i < ui.list_taskReceived->count(); i++)
		{
			QListWidgetItem *item = ui.list_taskReceived->item(i);
			SYListWidgetFrame* widget = dynamic_cast<SYListWidgetFrame*>(ui.list_taskReceived->itemWidget(item));
			if (widget)
				widget->SetChecked(false);
		}

		if (flag)
		{
			QString sendSucceedTips = SYStringTable::GetString(SYStringTable::STR_SendSucceedTips);
			SYMessageBox* messageBox = new SYMessageBox(this, "", sendSucceedTips, 1);
			messageBox->setPicture("Shared/personnel_pic_.png");  //图片需要修改
			messageBox->showFullScreen();
			messageBox->exec();
			done(RbShieldLayer::RC_Ok);
		}	
	}
}


void SYAdminSendPaperWindow::on_checkbox_statechanged(int state)
{	
	QListWidget* listWidget;
	QCheckBox* box = (QCheckBox*)sender();
	if (box == ui.checkBox)
		listWidget = ui.list_task;
	else 
		listWidget = ui.list_taskReceived;

	for (std::size_t i = 0; i < listWidget->count(); i++)
	{
		QListWidgetItem *item = listWidget->item(i);
		SYListWidgetFrame* widget = dynamic_cast<SYListWidgetFrame*>(listWidget->itemWidget(item));
		if (widget)
			widget->SetChecked(state);
	}

}
