#pragma once
#include "ui_SYQuestionExamResultWindow.h"
#include <QTimer>
#include <QToolButton>
#include "MxProcessCommunicator.h"

class ExamPaperQuestion;
class SYQuestionExamResultWindow : public QWidget
{
	Q_OBJECT
public:
	SYQuestionExamResultWindow(QWidget *parent = nullptr);
	
	~SYQuestionExamResultWindow(void);
	
	void refreshQuestionUI();

	void setMissionInfo(QString paperName, int questNum, int userUsedTime, int examTotalScore, int userRightItemNum, int userErrorItemNum, int userScore);

signals:
	void showNextWindow(WindowType type);

public slots:

   
	void onClickeQuestionItem(QListWidgetItem *item);

	void on_knowledgeLibBtn_clicked();

	void on_dataCenterBtn_clicked();

	void on_answerBtn_clicked();

	void on_personCenterBtn_clicked();

	void on_bt_next_clicked();

	void on_bt_prev_clicked();

protected:


private:
	
	Ui::SYQuestionExamResultWindow ui;
	QString m_userName;
	QString m_realName;
	UserPermission m_permission;

	QTimer m_timer;

	std::vector<ExamPaperQuestion> m_QuestsList;

	QPushButton* m_preButton;

	int m_QuestIndex;
};
