#include "WeChatLogin.h"

WeChatLogin::WeChatLogin(QWidget *parent)
	: SYMainFrameWindow(parent)
{
	ui.setupUi(this);
	setWindowState(Qt::WindowFullScreen);

	ui.ieBrowse->setControl(QString::fromUtf8("{8856F961-340A-11D0-A96B-00C04FD705A2}")); //���ò��ΪIE
	ui.ieBrowse->dynamicCall("Navigate(const QString&)", "about:blank"); //���ò���

}

WeChatLogin::~WeChatLogin()
{
}

void WeChatLogin::SetUrl(QString strUrl)
{
	m_strUrl = strUrl;
	ui.ieBrowse->dynamicCall("Navigate(const QString&)", m_strUrl); //���ò���
}

void WeChatLogin::hideEvent(QHideEvent * event)
{
	ui.ieBrowse->dynamicCall("Navigate(const QString&)", "about:blank"); //���ò���
}

