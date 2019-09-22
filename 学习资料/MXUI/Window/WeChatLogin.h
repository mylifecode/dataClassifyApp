#ifndef WECHATLOGIN_H
#define WECHATLOGIN_H

#include "SYMainFrameWindow.h"
#include "ui_WeChatLogin.h"
#include <QNetworkReply>

class WeChatLogin : public SYMainFrameWindow
{
	Q_OBJECT

public:
	WeChatLogin(QWidget *parent = 0);
	~WeChatLogin();

	void SetUrl(QString strUrl);

protected:
	virtual void hideEvent(QHideEvent * event);

private:
	Ui::WeChatLogin ui;
	QString m_strUrl;

};

#endif 
