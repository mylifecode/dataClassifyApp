#pragma once
#include <QWidget>
#include <vector>
#include <QSqlRecord>
#include <QTableWidget>
#include "SYTrainTaskStruct.h"
#include "RbShieldLayer.h"
#include "MxDefine.h"
#include "ui_SYAdminSendTaskWindow.h"
#include"SYDBMgr.h"
#include "SYProcessingDlg.h"

class TaskSendingThread : public QThread
{
public:
	TaskSendingThread(SYProcessingDlg * proceedlg) : QThread()
	{
		m_proceedlg = proceedlg;
	}

	void stop()
	{
		wait();
	}
	std::vector<SYTaskWork> taskSelected;

	std::vector<std::pair<int, QString>> userSelected;
protected:
	virtual void run()
	{
		int percent = 0;

		int deltapercent = 100 / (taskSelected.size() + userSelected.size()) + 1;

		for (int taskid = 0; taskid < (int)taskSelected.size(); taskid++)
		{
			if (taskSelected[taskid].m_TrainUnits.size() == 0)
				SYTaskTrainDataMgr::GetInstance().PullAllTrainsBelongsToTask(taskSelected[taskid]);

			percent += deltapercent;
			m_proceedlg->SetProcess(percent);
			Sleep(50);
		}

		for (int useindex = 0; useindex < userSelected.size(); useindex++)//send to every one
		{
			int id = userSelected[useindex].first;

			QString userName = userSelected[useindex].second;

			//submit train
			for (int c = 0; c < taskSelected.size(); c++)
			{
				SYTaskWork & task = taskSelected[c];

				int AssignorID = task.m_Assignorid;
				QString AssignorName = task.m_Assignor;

				int ReceiverID = id;
				QString RecevierName = userName;

				int numTrains = (int)task.m_TrainUnits.size();

				int taskID = SYDBMgr::Instance()->AddOneTaskEntry(taskSelected[c].m_TaskName, AssignorID, ReceiverID, AssignorName, RecevierName, numTrains, false);

				//submit train in task
				for (int t = 0; t < numTrains; t++)
				{
					SYDBMgr::Instance()->AddOneTrainEntryInTask(taskID,
						"",
						task.m_TrainUnits[t].m_ScoreCode,
						task.m_TrainUnits[t].m_PassNeedScore,
						task.m_TrainUnits[t].m_NeedPassTimes);
				}
			}

			percent += deltapercent;

			m_proceedlg->SetProcess(percent);

			Sleep(50);
		}

		m_proceedlg->SetCompleted();
	}
	SYProcessingDlg * m_proceedlg;


};

class SYSendTaskListItem : public QFrame
{
public:

	SYSendTaskListItem::SYSendTaskListItem(QWidget * parent, int itemindex) : QFrame(parent)
	{
		m_itemIndex = itemindex;
	}

	SYSendTaskListItem::~SYSendTaskListItem()
	{

	}

	void SYSendTaskListItem::Create(int ItemFixHeight, const QString & content0, const QString & content1, const QString & content2)
	{
		setFixedHeight(ItemFixHeight);
		m_Label0 = new QLabel(this);
		m_Label1 = new QLabel(this);
		m_Label2 = new QLabel(this);

		m_CheckBox = new QCheckBox(this);

		int offset = 31;

		int itemWidth = 207;
		//user name
		m_CheckBox->move(offset, 0);
		m_CheckBox->setFixedHeight(ItemFixHeight);
		m_CheckBox->setFixedWidth(itemWidth);
		m_CheckBox->setText("");
		offset += itemWidth;

		//
		m_Label0->move(offset, 0);
		m_Label0->setFixedHeight(ItemFixHeight);
		m_Label0->setFixedWidth(itemWidth);
		m_Label0->setText(content0);
		m_Label0->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		m_Label0->setStyleSheet(QString("font-size:16px; color:#b7c5d8;"));
		offset += itemWidth;

		//
		m_Label1->move(offset, 0);
		m_Label1->setFixedHeight(ItemFixHeight);
		m_Label1->setFixedWidth(itemWidth);
		m_Label1->setText(content1);
		m_Label1->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		m_Label1->setStyleSheet(QString("font-size:16px; color:#b7c5d8;"));
		offset += itemWidth;

		//
		m_Label2->move(offset, 0);
		m_Label2->setFixedHeight(ItemFixHeight);
		m_Label2->setFixedWidth(itemWidth);
		m_Label2->setText(content2);
		m_Label2->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		m_Label2->setStyleSheet(QString("font-size:16px; color:#b7c5d8;"));
		offset += itemWidth;
	}

	bool IsChecked()
	{
		return m_CheckBox->isChecked();
	}
	void SetChecked(bool state)
	{
		m_CheckBox->setChecked(state);
	}
	QLabel * m_Label0;
	QLabel * m_Label1;
	QLabel * m_Label2;
	QCheckBox * m_CheckBox;
	int m_itemIndex;

	int m_ID;
	QString m_UserName;
};





class SYAdminSendTaskWindow : public QWidget
{
	Q_OBJECT
public:
	SYAdminSendTaskWindow(QWidget* parent = nullptr);

	~SYAdminSendTaskWindow();

signals:
	void showNextWindow(WindowType windowType);

private:

	void InitTables();

	void SetTableContent();

private slots:
    
    void on_bt_conform_clicked();
	void on_checkbox_statechanged(int state);

protected:


private:
	Ui::SYAdminSendTaskWindow ui;

	QPushButton* m_curSelectedBtn;

	std::vector<SYTaskWork> m_TasksTemplate;

	QTableWidget * m_TaskTable;

};