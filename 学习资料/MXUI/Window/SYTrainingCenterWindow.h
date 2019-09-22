#pragma once
#include "ui_SYTrainingCenterWindow.h"
#include <QTimer>
#include <QToolButton>
#include "MxProcessCommunicator.h"
class MxScreenDataSender;
class MxDemonstrationWindow;

class SYTrainingCenterWindow : public QWidget
{
	Q_OBJECT
public:
	SYTrainingCenterWindow(QWidget *parent = nullptr);
	~SYTrainingCenterWindow(void);

signals:
	void showNextWindow(WindowType type);

public slots:

	/** 发送会话命令给服务端 */
	void onSendSessionCommand();

	void onButtonClicked();

	void on_knowledgeLibBtn_clicked();

	void on_dataCenterBtn_clicked();

	void on_answerBtn_clicked();

	void on_personCenterBtn_clicked();

protected:
	void showEvent(QShowEvent* event);

	void mousePressEvent(QMouseEvent* event);

	void mouseReleaseEvent(QMouseEvent* event);

	void LaunchRealTrainModule();
private:
	Ui::SYTrainingCenterWindow ui;
	QString m_userName;
	QString m_realName;
	UserPermission m_permission;
	MxScreenDataSender* m_screenDataSender;
	QTimer m_timer;
	QPushButton* m_ChengJiBtn;
	MxDemonstrationWindow* m_demonstrationWindow;
	std::multimap<UserPermission, QString> m_mapLoginModule;

	QPushButton* m_preButton;

	int m_clickedFrameIndex;
};
