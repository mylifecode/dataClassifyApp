#pragma once
#include "ui_SYQuestionExamListWindow.h"
#include <QTimer>
#include <QToolButton>
#include "MxProcessCommunicator.h"
#include "SYExamGlobal.h"
#include"SYExamRecordItem.h"

class MxScreenDataSender;
class MxDemonstrationWindow;
class SYExamRecordItem;
class SYMainWindow;
class SYQuestionExamListWindow : public QWidget
{
	Q_OBJECT
public:
	SYQuestionExamListWindow(QWidget *parent = nullptr);
	~SYQuestionExamListWindow(void);

	void RefreshMissionList();

signals:
	void showNextWindow(WindowType type);

	//void DoExamMission(int missionid);

public slots:

    void OnItemDescButtonClicked(SYExamRecordItem*item);

	void OnReDoButtonClicked(SYExamRecordItem*item);

	void  on_item_Btn_clicked(SYExamPaperItem* item);

	void onButtonClicked();

	void on_knowledgeLibBtn_clicked();

	void on_dataCenterBtn_clicked();

	void on_answerBtn_clicked();

	void on_personCenterBtn_clicked();

	void on_randexam_clicked();
	
	void on_bt_importquest_clicked();

	void onBackToWidget(QWidget * widget);

protected:
	void showEvent(QShowEvent* event);

	void mousePressEvent(QMouseEvent* event);

private:
	Ui::SYQuestionExamListWindow ui;
	QString m_userName;
	QString m_realName;
	UserPermission m_permission;

	QTimer m_timer;

	std::multimap<UserPermission, QString> m_mapLoginModule;

	std::vector<ExamMission> m_missionlist;
 
	QPushButton* m_preButton;
};
