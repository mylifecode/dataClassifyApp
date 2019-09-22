#pragma once
#include "ui_SYPersonTaskListsWindow.h"
#include "ui_SYPersonTaskListItem.h"
#include "SYTrainTaskStruct.h"
enum WindowType;

class SYPersonTaskListItem : public QFrame
{
public:
	SYPersonTaskListItem(QWidget * parent, int itemindex);
	
	~SYPersonTaskListItem();

	void Create(const QString & userName, const QString & groupName, int ItemFixHeight, const QString & skinDir);

	int m_itemIndex;
	
	QLabel * m_lb_name;
	
	QLabel * m_lb_class;
};

class SYPersonTrainListItem : public QWidget
{
	Q_OBJECT

public:
	explicit SYPersonTrainListItem(QWidget *parent = 0);

	~SYPersonTrainListItem();

	void SetContent(const QString & name, const QString &  totalTrainTime, const QString & trainPeriod, int NeedPassTime, int CurrPassTime, int CompleteState);

	void SetTrainLaunchData(const QString & runName , const QString & showName , int TaskID , int TaskTrainID);
	
	int m_TaskID;
	int m_TaskTrainID;
	QString m_TrainShowName;
	QString m_TrainRunName;


public slots:
	
    void on_bt_starttrain_clicked();


private:
	Ui::SYPersonTaskListItem ui;
};


class SYPersonTaskListsWindow : public QWidget
{
	Q_OBJECT
public:
	SYPersonTaskListsWindow(QWidget *parent = nullptr);
	~SYPersonTaskListsWindow(void);

	void RefreshTaskDetail(SYTaskWork & taskWork);

	int RefrehTasksList(int selectedID);

	int  m_CurrSelectedItem;
signals:
	void showNextWindow(WindowType type);

	//void DoExamMission(int missionid);

public slots:

    void onClickeTaskItem(QListWidgetItem *item);

	

protected:
	void showEvent(QShowEvent* event);

	void mousePressEvent(QMouseEvent* event);

private:
	std::vector<SYTaskWork> m_allTasks;

	Ui::SYPersonTaskListsWindow ui;
	QString m_userName;
	QString m_realName;
	//UserPermission m_permission;
};
