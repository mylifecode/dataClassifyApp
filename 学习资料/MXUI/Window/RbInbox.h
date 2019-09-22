#pragma once
//用户收件箱窗口界面管理
#include "RbShieldLayer.h"
#include "ui_RbInbox.h"
#include "RbMessageViewWindow.h"

class RbInbox : public RbShieldLayer
{
	Q_OBJECT
public:
	RbInbox(QWidget *parent);
	~RbInbox();
	void BuildInboxWindow();
	void mouseMoveEvent(QMouseEvent *event);
	void Scroll_up();
	void Scroll_down();

protected:
	void closeEvent(QCloseEvent * event);
signals:
	void indexClose();

public slots:
	void onClickedcloseBtn();
	void onOpenaContent(int row,int column);
	void onOpenpContent(int row,int column);
	void onMessageClose();
	void onButtonClicked();

private:
	Ui::RbInbox ui;
	QString m_receiverName;
	QString m_groupName;
	QString m_sendName;
	QString m_sendTime;
	QString m_subject;
	QString m_content;
	RbMessageViewWindow* m_pMessageView;
};

