#pragma once
//收件箱消息查看窗口
#include "RbShieldLayer.h"
#include "ui_RbMessageViewWindow.h"
#include <QDateTime>
#include <QWidget>

class RbMessageViewWindow : public QWidget
{
	Q_OBJECT
public:
	RbMessageViewWindow(QWidget *parent,QString subject,QString content);
	~RbMessageViewWindow();

protected:
	void closeEvent(QCloseEvent * event);
signals:
	void messageClose();

public slots:
	void onClickedcloseBtn();

private:
	Ui::RbMessageViewWindow ui;
	QString m_subject;
	QString m_content;
	int m_initHeight;
};