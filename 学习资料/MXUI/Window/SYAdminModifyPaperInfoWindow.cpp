#include "SYAdminModifyPaperInfoWindow.h"
#include "ui_SYAdminModifyPaperInfoWindow.h"
#include"SYAdminPaperContentFrameWindow.h"
#include "MxDefine.h"
#include"SYAdminQuestionsMgrWindow.h"
#include <SYDBMgr.h>
#include<QListWidgetItem>
#include<qlistwidget.h>
#include"SYStringTable.h"
#include"SYMessageBox.h"
#include<qmessagebox.h>

//class ListWidgetItemFrame :public QFrame
//{
//public:
//	ListWidgetItemFrame(QWidget*parent, QSqlRecord record, int index)
//	{
//		QHBoxLayout *hLayout = new QHBoxLayout();
//		hLayout->addItem(new QSpacerItem(78, 22, QSizePolicy::Fixed, QSizePolicy::Expanding));
//		QVBoxLayout *vLayout = new QVBoxLayout();
//		vLayout->addSpacerItem(new QSpacerItem(22, 22, QSizePolicy::Fixed, QSizePolicy::Fixed));
//		box = new QCheckBox(this);
//		vLayout->addWidget(box);
//		hLayout->addLayout(vLayout);
//		hLayout->addItem(new QSpacerItem(22, 22, QSizePolicy::Fixed, QSizePolicy::Expanding));
//		SYAdminPaperContentFrameWindow* frameWin = new SYAdminPaperContentFrameWindow(record, index);
//		hLayout->addWidget(frameWin);
//		setLayout(hLayout);
//	}
//	void SetChecked(bool state)
//	{
//		box->setChecked(state);
//	}
//	bool  IsChecked()
//	{
//		if (box->isChecked())
//			return true;
//		else
//			return false;
//	}
//
//	void SetBackGroundColor(bool flag)
//	{
//		if (flag)
//		{
//			setStyleSheet(QString("background-color:rgb(255,0,0)"));
//
//		}
//		else
//
//			setStyleSheet(QString("background-color:#31394c"));
//
//	}
//	void ItemStateChange(bool flag)
//	{
//		SetChecked(flag);
//		//SetBackGroundColor(flag);
//	}
//
//	QCheckBox* getCheckBox()
//	{
//		return box;
//	}
//
//private:
//
//	QCheckBox *box;
//};


SYAdminModifyPaperInfoWindow::SYAdminModifyPaperInfoWindow(QString paperName, QWidget *parent) :
RbShieldLayer(parent), m_curPaperName(paperName)
{
    ui.setupUi(this);

	hideOkButton();
	hideCloseButton();
	setAttribute(Qt::WA_DeleteOnClose);
	
	Mx::setWidgetStyle(this, "qss:SYAdminModifyPaperInfoWindow.qss");

	Initialize();
	RefreshTable();
	RefreshPage();
}

SYAdminModifyPaperInfoWindow::~SYAdminModifyPaperInfoWindow()
{

}


void SYAdminModifyPaperInfoWindow::Initialize()
{

	ui.listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);  //横向滑动隐藏

	connect(ui.listWidget, &QListWidget::itemDoubleClicked, [=](QListWidgetItem *item)
	{
		ListWidgetItemFrame* widget = dynamic_cast<ListWidgetItemFrame*>(ui.listWidget->itemWidget(item));
		bool flag = widget->IsChecked();
		flag = !flag;
		widget->ItemStateChange(flag);
		SaveModifyPaperInfo();
		RefreshPage();
	});

	connect(ui.all_chosed_checkBox, &QCheckBox::stateChanged, this, [=](int state)
	{
		bool flag = state;
		for (std::size_t i = 0; i < ui.listWidget->count(); i++)
		{
			QListWidgetItem* item = ui.listWidget->item(i);

			ListWidgetItemFrame* widget = dynamic_cast<ListWidgetItemFrame*>(ui.listWidget->itemWidget(item));
			if (widget)
			{
				widget->ItemStateChange(flag);
			}
		}
			//SaveModifyPaperInfo();
		  int itemSelectedNumber = GetItemChosedNumber();
		  ui.chosedQuestion->setText(SYStringTable::GetString(SYStringTable::STR_ChosedQuestionNum) + QString::number(itemSelectedNumber));
	}
	);

	connect(ui.deleteBtn, &QPushButton::clicked, this, &SYAdminModifyPaperInfoWindow::on_delete_Btn_Clicked);

	connect(ui.returnBtn, &QPushButton::clicked, this, &SYAdminModifyPaperInfoWindow::on_return_Btn_Clicked);
}

void SYAdminModifyPaperInfoWindow::RefreshTable()
{
	QVector<QSqlRecord> records;
	struct PaperInfoDetail *paperInfoDetail;
	SYDBMgr::Instance()->QueryPaperInfo(m_curPaperName, &paperInfoDetail, records);   //查找试卷信息

	ui.listWidget->clear();//刷新清空
	int index = 0;
	for (auto it = records.begin(); it != records.end(); it++)
	{
		QSqlRecord record = *it;
		ListWidgetItemFrame* widget = new ListWidgetItemFrame(ui.listWidget, record, index);
		QListWidgetItem* item = new QListWidgetItem(ui.listWidget);
		item->setSizeHint(QSize(1400, 400));
		ui.listWidget->setItemWidget(item, widget);

		QCheckBox* box = widget->getCheckBox();
		connect(box, &QCheckBox::stateChanged, this, [=](int state)
		{
			bool flag = state;
			widget->ItemStateChange(flag);
			SaveModifyPaperInfo();
			RefreshPage();
		});

		index++;
	}
}

void SYAdminModifyPaperInfoWindow::RefreshPage()  //单一职责原则
{
	QVector<QSqlRecord> records;
	PaperInfoDetail *paperInfoDetail;
	SYDBMgr::Instance()->QueryPaperInfo(m_curPaperName, &paperInfoDetail, records);   //查找试卷信息

	if (paperInfoDetail)
	{
		QString examTime = paperInfoDetail->examTime;
		QString examTime1 = examTime[0];
		QString examTime2 = examTime[1];
		QString examTime3 = examTime[3];
		QString examTime4 = examTime[4];
		QString questionNums = paperInfoDetail->questionNums;
		QString totalScore = paperInfoDetail->examScore;
		//设置考试答题信息
		ui.paperName->setText(paperInfoDetail->paperName);
		ui.examAnswerTime_1->setText(QString(paperInfoDetail->examTime[0]));
		ui.examAnswerTime_2->setText(QString(paperInfoDetail->examTime[1]));
		ui.examAnswerTime_3->setText(QString(paperInfoDetail->examTime[3]));
		ui.examAnswerTime_4->setText(QString(paperInfoDetail->examTime[4]));
		ui.questionNum->setText(paperInfoDetail->questionNums);
		ui.examScore->setText(paperInfoDetail->examScore);
	}
	int itemSelectedNumber = GetItemChosedNumber();
	ui.chosedQuestion->setText(SYStringTable::GetString(SYStringTable::STR_ChosedQuestionNum) + QString::number(itemSelectedNumber));
}

void SYAdminModifyPaperInfoWindow::SaveModifyPaperInfo()
{
	QString paperName = ui.paperName->text();
	QString examTime1 = ui.examAnswerTime_1->text();
	QString examTime2 = ui.examAnswerTime_2->text();
	QString examTime3 = ui.examAnswerTime_3->text();
	QString examTime4 = ui.examAnswerTime_4->text();
	QString examTime = examTime1 + examTime2 + CHS(":") + examTime3 + examTime4;
	/*QString questionNums = ui.questionNum->text();
	QString examScore = ui.examScore->text();
	QString paperRank = CHS("_");
	QString createPaperTime = CHS("_");
	QString creator = CHS("_");*/

	PaperInfoDetail *paperInfoDetail=new PaperInfoDetail(paperName,examTime);
	
	if (paperInfoDetail)
	{
		QVector<QSqlRecord> delRecords;
		SYDBMgr::Instance()->ModifyPaperInfo(m_curPaperName, delRecords, paperInfoDetail);
	}

	m_curPaperName = paperName;//数据库信息修改
}

int SYAdminModifyPaperInfoWindow::GetItemChosedNumber()
{
	QListWidget* listWidget = ui.listWidget;
	int itemNumber = 0;
	for (std::size_t i = 0; i < listWidget->count(); i++)
	{
		QListWidgetItem* item = listWidget->item(i);
		ListWidgetItemFrame* widget = dynamic_cast<ListWidgetItemFrame*>(listWidget->itemWidget(item));
		if (widget)
		{
			bool flag = true;
			flag = widget->IsChecked();
			if (flag)
				itemNumber++;	
		}
	}
	return  itemNumber;
}

void SYAdminModifyPaperInfoWindow::on_return_Btn_Clicked()
{
	SaveModifyPaperInfo();
	//关闭页面
	done(RbShieldLayer::RC_Ok);

}

void SYAdminModifyPaperInfoWindow::on_delete_Btn_Clicked()
{
	QVector<QSqlRecord> records, delRecords;
	PaperInfoDetail* paperInfoDetail;
	SYDBMgr::Instance()->QueryPaperInfo(m_curPaperName, &paperInfoDetail, records);   //查找试卷信息

	for (std::size_t i = 0; i < ui.listWidget->count(); i++)
	{
		QListWidgetItem* item = ui.listWidget->item(i);

		ListWidgetItemFrame* widget = dynamic_cast<ListWidgetItemFrame*>(ui.listWidget->itemWidget(item));
		if (widget)
		{
			bool flag = widget->IsChecked();
			if (flag)
			{
				QSqlRecord record = records[i];
				delRecords.push_back(record);
			}
		}

	}

	if (delRecords.size() == 0)
	{
		QString chosedDeleteQuestionsTip = SYStringTable::GetString(SYStringTable::STR_ChosedDeleteQuestionsTip);
		SYMessageBox* messageBox = new SYMessageBox(this, "", chosedDeleteQuestionsTip, 1);
		messageBox->setPicture("Shared/personnel_pic_.png");
		messageBox->showFullScreen();
		messageBox->exec();
		return;
	}

	if (delRecords.size() == 0)
	{
		QString chosedDeleteQuestionsTip = SYStringTable::GetString(SYStringTable::STR_ChosedDeleteQuestionsTip);
		SYMessageBox* messageBox = new SYMessageBox(this, "", chosedDeleteQuestionsTip, 1);
		messageBox->setPicture("Shared/personnel_pic_.png");
		messageBox->showFullScreen();
		messageBox->exec();
		return;
	}

	if (delRecords.size() == ui.listWidget->count())
	{
		QString paperConfirmDelete = SYStringTable::GetString(SYStringTable::STR_PaperConfirmDelete);
		SYMessageBox* messageBox = new SYMessageBox(this, "", paperConfirmDelete, 2);
		messageBox->setPicture("Shared/personnel_pic_.png");
		messageBox->showFullScreen();
		int Ret = messageBox->exec();
		if (Ret == 2)  //flag==1cancel
		{
			//SYDBMgr::Instance()->ModifyPaperInfo(m_curPaperName, delRecords, paperInfoDetail);
			SYDBMgr::Instance()->DeletePaperInfo(m_curPaperName);
			done(RbShieldLayer::RC_Ok);
			return;
		}

	}
	else
	{
		QString questionsConfirmDelete = SYStringTable::GetString(SYStringTable::STR_QuestionsConfirmDelete);
		SYMessageBox* messageBox = new SYMessageBox(this, "", questionsConfirmDelete, 2);
		messageBox->setPicture("Shared/personnel_pic_.png");
		messageBox->showFullScreen();
		int Ret = messageBox->exec();
		if (Ret == 2)
		{
			SYDBMgr::Instance()->ModifyPaperInfo(m_curPaperName, delRecords, paperInfoDetail);
			SaveModifyPaperInfo();
			RefreshTable();  //更新显示
			RefreshPage();
		}

	}

}