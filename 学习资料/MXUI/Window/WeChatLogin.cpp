#include "WeChatLogin.h"

WeChatLogin::WeChatLogin(QWidget *parent)
	: SYMainFrameWindow(parent)
{
	ui.setupUi(this);
	setWindowState(Qt::WindowFullScreen);

	ui.ieBrowse->setControl(QString::fromUtf8("{8856F961-340A-11D0-A96B-00C04FD705A2}")); //设置插件为IE
	ui.ieBrowse->dynamicCall("Navigate(const QString&)", "about:blank"); //调用参数

}

WeChatLogin::~WeChatLogin()
{
}

void WeChatLogin::SetUrl(QString strUrl)
{
	m_strUrl = strUrl;
	ui.ieBrowse->dynamicCall("Navigate(const QString&)", m_strUrl); //调用参数
}

void WeChatLogin::hideEvent(QHideEvent * event)
{
	ui.ieBrowse->dynamicCall("Navigate(const QString&)", "about:blank"); //调用参数
}

