#pragma once
#include "ui_SYPersonCenterWindow.h"
#include "SYTrainTaskStruct.h"
class SYEditPersonPasswordWindow;

enum WindowType;

class SYPersonCenterWindow : public QWidget
{
	Q_OBJECT
public:
	SYPersonCenterWindow(QWidget *parent = nullptr);

	~SYPersonCenterWindow(void);

signals:
	void showNextWindow(WindowType type);

	

private:
	void setTheoryTestInfo();

	void setSkillingTrainInfo();

	void setTrainTimesInfo();

	void setRankInfo();

	void setCourseAndTaskInfo();

	void refreshMyTasks();

private slots:

	/** 点击按钮，编辑个人信息 */
	void on_editBtn_clicked();

	void on_messageBtn_clicked();

	void on_addTaskBtn_clicked();

	void onCourseListTableItemClicked(int row, int col);

private:
	std::vector<SYTaskWork> m_allTasks;

	Ui::SYPersonCenterFrame ui;
	int m_userId;

	SYEditPersonPasswordWindow* m_editPasswordWindow;
};
