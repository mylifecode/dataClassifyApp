#include "StdAfx.h"
#include "ScoreBoardWeb.h"
#include "MxDefine.h"
#include <QDebug>
#include "RbShutdownBox.h"
#include "RbAbout.h"

ScoreBoardWeb* ScoreBoardWeb::s_ScoreboardWeb = NULL;
QMutex* ScoreBoardWeb::s_pMutexClose = NULL;

ScoreBoardWeb::ScoreBoardWeb(QWidget *parent)
	: QDialog(parent)
#if !USE_IE
	, m_pWebView(NULL)
#endif
{
	ui.setupUi(this);
	setAttribute(Qt::WA_TranslucentBackground, true); setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);//设置背景透明
	connect(ui.closeBtn, SIGNAL(clicked()), this, SLOT(onClickedClose()));
	connect(ui.exitBtn, SIGNAL(clicked()), this, SLOT(onClickedShutdownBtn()));
	connect(ui.aboutBtn, SIGNAL(clicked()), this, SLOT(onClickedAboutBtn()));
#if USE_IE
	ui.ieBrowse->setControl(QString::fromUtf8("{8856F961-340A-11D0-A96B-00C04FD705A2}")); //设置插件为IE
	ui.ieBrowse->dynamicCall("Navigate(const QString&)", "about:blank"); //调用参数

	connect(ui.ieBrowse, SIGNAL(NavigateComplete2(IDispatch*, QVariant&)), this, SLOT(NavigateComplete(IDispatch*, QVariant&)));
#else
	
	m_pWebView = new QWebEngineView(this);
	QVBoxLayout* pLayout = new QVBoxLayout(this);
	pLayout->addWidget(m_pWebView);
	pLayout->setMargin(0);
	ui.webFrame->setLayout(pLayout);
	connect(m_pWebView, SIGNAL(loadFinished(bool)), this, SLOT(FinishLoading(bool)));
	connect(m_pWebView, SIGNAL(loadProgress(int)), this, SLOT(SetProgress(int)));

	if (ui.label)
	{
		ui.label->setStyleSheet(QString("color:red; font:20pt \"微软雅黑\";"));
		ui.label->setAlignment(Qt::AlignCenter);

		SetWaitingText(QString::fromLocal8Bit("服务器数据加载中..."));

		ui.label->setVisible(false);
	}

	m_strUrl.clear();
	m_strPreUrl.clear();

	QString url("");
	m_pWebView->load(QUrl(url));
	m_pWebView->show();
#endif
	setAttribute(Qt::WA_DeleteOnClose, true);
	Mx::setWidgetStyle(this, "qss:ScoreBoardWeb.qss");

	if (NULL == s_pMutexClose)
		s_pMutexClose = new QMutex;

	if (NULL == ScoreBoardWeb::s_ScoreboardWeb)
		ScoreBoardWeb::s_ScoreboardWeb = this;
}

ScoreBoardWeb::~ScoreBoardWeb()
{
	if (this == ScoreBoardWeb::s_ScoreboardWeb)
		ScoreBoardWeb::s_ScoreboardWeb = NULL;
}

void ScoreBoardWeb::onClickedClose()
{
	s_pMutexClose->lock();
	close();
	s_pMutexClose->unlock();
}

ScoreBoardWeb* ScoreBoardWeb::GetActiveInstance(void)
{
	return ScoreBoardWeb::s_ScoreboardWeb;
}

void ScoreBoardWeb::SetInstanceURL(QString strUrl)
{
	ScoreBoardWeb::s_pMutexClose->lock();

	ScoreBoardWeb* scoreBoareWeb = ScoreBoardWeb::GetActiveInstance();

	if (scoreBoareWeb)
		scoreBoareWeb->SetUrl(strUrl);

	ScoreBoardWeb::s_pMutexClose->unlock();
}

void ScoreBoardWeb::SetUrl(QString strUrl)
{
	m_strUrl = strUrl;

#if USE_IE
	ui.ieBrowse->dynamicCall("Navigate(const QString&)", m_strUrl); //调用参数
#else
	QString strText = "";


	if (m_strPreUrl.isEmpty())
	{
		SetWaitingText(QString::fromLocal8Bit("服务器数据加载中..."));
		strText = m_strUrl;
	}

	LoadWebUrl(strText);
#endif
}

void ScoreBoardWeb::showEvent(QShowEvent * event)
{
#if !USE_IE
	SetWaitingText(QString::fromLocal8Bit("服务器数据等待中..."));
	
	if (!m_strPreUrl.isEmpty())
	{
		LoadWebUrl("");
	}
#endif
}

void ScoreBoardWeb::FinishLoading(bool bFinished)
{
	if (m_nProgress == 100)
	{
		if (ui.label && !m_strPreUrl.isEmpty())
		{
			ui.label->setVisible(false);
		}

		if (m_strPreUrl.isEmpty())
		{
			if (!m_strUrl.isEmpty())
			{
				SetWaitingText(QString::fromLocal8Bit("服务器数据加载中..."));
				LoadWebUrl(m_strUrl);
				m_strUrl.clear();
			}
		}
		m_nProgress = 0;
	}
	qDebug() << "finished" << bFinished;
}

void ScoreBoardWeb::SetWaitingText(QString strText)
{
	if (ui.label)
	{
		ui.label->setText(strText);
		ui.label->adjustSize();
		ui.label->setVisible(true);
	}
}

void ScoreBoardWeb::LoadWebUrl(QString strUrl)
{
	m_nProgress = 0;
	m_strPreUrl = strUrl;
#if !USE_IE
	m_pWebView->load(QUrl::fromUserInput(m_strPreUrl));
#endif
}

void ScoreBoardWeb::SetProgress(int nProgress)
{
	m_nProgress = nProgress;
	qDebug() << "Loading:"  << m_nProgress;
}

void ScoreBoardWeb::onClickedShutdownBtn()
{
	RbShutdownBox * pShutdownBox = new RbShutdownBox(this);
	pShutdownBox->show();
}

void ScoreBoardWeb::onClickedAboutBtn()
{
	RbAbout * pAbout = new RbAbout(this);
	pAbout->showFullScreen();
	
}

void ScoreBoardWeb::hideEvent(QHideEvent * event)
{
#if USE_IE
	ui.ieBrowse->dynamicCall("Navigate(const QString&)", "about:blank"); //调用参数
#endif
}

#if USE_IE
void ScoreBoardWeb::NavigateComplete(IDispatch *pDisp, QVariant &URL)
{
//	qDebug() << URL.toString();
}
#endif

