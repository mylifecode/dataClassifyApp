#pragma once
#include <QWidget>
#include "ui_SYAdminPersonWindow.h"
#include <vector>
#include <QSqlRecord>
#include <QTableWidget>

class SYMessageBox;
class SYAddUserWindow;
class SYImportUserWindow;
class SYMessageSendWindow;
class SYProcessingDlg;

class SYAdminGroudpMgrWindow;

class SYAdminPersonWindow : public QWidget
{
	Q_OBJECT
public:
	SYAdminPersonWindow(QWidget* parent = nullptr);

	~SYAdminPersonWindow();

signals:
	void showNextWindow(WindowType windowType);

private:

	void InitTables();

	void LoadData();

	void SetTableContent();

	void SetTeacherTableContent();
	
	void SetStudentTableContent();

	void ResetTableRecordShowStatus();

	void RefreshTable();


private slots:
	void on_teacherMgrBtn_clicked();

	void on_studentMgrBtn_clicked();

	void on_groupMgrBtn_clicked();

	void on_searchBtn_clicked();

	void on_newBtn_clicked();

	void on_importBtn_clicked();

	void on_sendMessageBtn_clicked();

	void onDeleteRecord();

	void onResetPassword();

protected:
	void showEvent(QShowEvent *event);

	void keyReleaseEvent(QKeyEvent* event);

private:
	Ui::SYAdminPersonWindow ui;

	QPushButton* m_curSelectedBtn;

	std::vector<QSqlRecord> m_allUserInfos;
	/// pair中第一个成员为索引，第二个成员决定是否显示,默认true
	std::vector<std::pair<int,bool>> m_teacherInfoIndexPairs;
	std::vector<std::pair<int, bool>> m_studentInfoIndexPairs;

	QTableWidget* m_teacherTable;
	QTableWidget* m_studentTable;
	SYAdminGroudpMgrWindow* m_groupWindow;

	SYAddUserWindow* m_addUserWindow;
	SYImportUserWindow* m_importUserWindow;
	SYMessageSendWindow* m_messageSendWindow;

	SYProcessingDlg* m_processingDlg;
};

