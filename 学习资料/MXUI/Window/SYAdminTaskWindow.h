#pragma once
#include <QWidget>
#include "ui_SYAdminTaskWindow.h"
#include "ui_SYTrainTaskDescWindow.h"
#include <vector>
#include <QSqlRecord>
#include <QTableWidget>
#include "SYTrainTaskStruct.h"
#include "RbShieldLayer.h"
class SYMessageBox;
class SYAddUserWindow;
class SYImportUserWindow;
class SYMessageSendWindow;

class SYAdminGroudpMgrWindow;

class SYAdminTaskWindow : public QWidget
{
	Q_OBJECT
public:
	SYAdminTaskWindow(QWidget* parent = nullptr);

	~SYAdminTaskWindow();

signals:
	void showNextWindow(WindowType windowType);

private:

	void InitTables();

	void LoadData();

	void SetTableContent();

	void RefreshTable();

	bool FilterRecord(const SYTaskWork&	task);

private slots:
    
    void on_taskMgrBtn_clicked();

	void onDeleteTaskTemplate();

	void onDescTaskTemplate();

	void on_newTaskBtn_clicked();

	void on_sendTaskBtn_clicked();

	void on_searchBtn_clicked();

protected:
	void showEvent(QShowEvent *event);

	void keyReleaseEvent(QKeyEvent* event);

private:
	Ui::SYAdminTaskWindow ui;

	QPushButton* m_curSelectedBtn;

	std::vector<SYTaskWork> m_TasksTemplate;

	QTableWidget * m_TaskTable;

};

class  SYTrainTaskDescWindow : public RbShieldLayer
{
	Q_OBJECT

public:

	SYTrainTaskDescWindow(QWidget *parent, const SYTaskWork & taskWork);

	~SYTrainTaskDescWindow();

	void RefreshUI(const SYTaskWork & taskWork);

private slots:

   void on_bt_conform_clicked();

private:
	Ui::SYTrainTaskDescWindow  ui;

	
};
