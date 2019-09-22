#pragma once
#include "ui_SYQuestionDoExamWindow.h"
#include <QTimer>
#include <QToolButton>
#include "MxProcessCommunicator.h"

class ExamPaperQuestion;
class SYQuestionDoExamWindow : public QWidget
{
	Q_OBJECT
public:
	SYQuestionDoExamWindow(QWidget *parent = nullptr);
	~SYQuestionDoExamWindow(void);

	
	void refreshQuestionUI();

	void GetExamQuestionFromExistMission(int missionID);

	void userAnswerUpdateOrder(QString &answer);

	void  setMissionInfo();

signals:
	void showNextWindow(WindowType type);
	void ReplaceCurrentWindow(WindowType);
	void ExitCurrentWindow();

public slots:

    void GetExamQuestionFromPaper(int paperid,int number);
	
	void onClickeQuestionItem(QListWidgetItem *item);

	void on_bt_finish_clicked();

	void on_bt_next_clicked();

	void on_bt_prev_clicked();

	void on_examIsOver();

protected:
	void showEvent(QShowEvent* event);

	void mousePressEvent(QMouseEvent* event);

private:
	bool eventFilter(QObject *obj, QEvent *event);

	int  m_ExamPaperID;

	Ui::SYQuestionDoExamWindow ui;
	QString m_userName;
	QString m_realName;
	UserPermission m_permission;

	QTimer* m_timer;
	QTimer* m_updateUITimer;

	std::vector<ExamPaperQuestion> m_QuestsList;

	QPushButton* m_preButton;

	int m_QuestIndex;

	int m_StartMilliseconds;
	bool m_isMissionReDone;
};
