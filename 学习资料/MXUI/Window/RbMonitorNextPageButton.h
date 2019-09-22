#pragma once
#include <QWidget>
#include "ui_RbMonitorNextPageButton.h"
#include <QPoint>


class RbMonitorNextPageButton : public QWidget
{
	Q_OBJECT
public:
	RbMonitorNextPageButton(QWidget * parent = NULL);
	~RbMonitorNextPageButton(void);
	void setWaitingInfo(const QString& userId, const int nWaitingCount);
	QString getLastWaittingUserID();
signals:
	void nextPage();
protected slots:
	void onClickedNextPage();
private:
	Ui::RbMonitorNextPageButton ui;
	QString m_strUserId;									//���ڼ�¼���һ���������ӽ�ʦ�˵�ID
};