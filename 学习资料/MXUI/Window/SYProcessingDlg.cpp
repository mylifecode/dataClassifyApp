#include "SYProcessingDlg.h"
#include <QPushButton>
#include <QSound>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QMessageBox>
#include "MxDefine.h"
#include "SYStringTable.h"
SYProcessingDlg::SYProcessingDlg(QWidget *parent) 
	:RbShieldLayer(parent),
	m_closeTimer(nullptr),
	m_autoProcessTimer(nullptr),
	m_processValue(0),
	m_maxProcessValue(1000)
{
	ui.setupUi(this);
	hideCloseButton();
	hideOkButton();

	setAttribute(Qt::WA_DeleteOnClose);
	Mx::setWidgetStyle(this,"qss:SYProcessingDlg.qss");

	connect(this, SIGNAL(SendRefreshBar(int)), this, SLOT(InternalRefreshBar(int)));
	connect(this, SIGNAL(SendComplete()), this, SLOT(InternalComplete()));

	m_MaxBarLength = ui.lb_processbar->maximumWidth();
	m_HasShowed = false;
	ui.bt_conform->setVisible(false);
	ui.lb_content->setVisible(false);

	m_closeTimer = new QTimer(this);
	connect(m_closeTimer, &QTimer::timeout, this, &SYProcessingDlg::onCloseTimer);

	
}

SYProcessingDlg::~SYProcessingDlg()
{

}

void SYProcessingDlg::SetProcess(int percent)
{
	if (m_HasShowed)
		emit SendRefreshBar(percent);
}

void SYProcessingDlg::SetCompleted()
{
	emit SendComplete();
}

void SYProcessingDlg::InternalComplete()
{
	ui.lb_processbar->setFixedWidth(m_MaxBarLength);
	ui.bt_conform->setVisible(true);
	ui.lb_content->setVisible(true);
	ui.lb_title->setText(SYStringTable::GetString(SYStringTable::STR_SENDFINISH));
	m_closeTimer->setSingleShot(true);
	m_closeTimer->start(3000);
}

void SYProcessingDlg::InternalRefreshBar(int percent)
{
	float t = (float)percent / (float)100;

	if(t < 0.f)
		t = 0.f;
	else if(t > 1.0f)
		t = 1.0f;

	int nowWidth = m_MaxBarLength * t;

	ui.lb_processbar->setFixedWidth(nowWidth);
}

void SYProcessingDlg::showEvent(QShowEvent* event)
{
	 ui.lb_processbar->setFixedWidth(0);
	 m_HasShowed = true;
}

void SYProcessingDlg::on_bt_conform_clicked()
{
	m_closeTimer->stop();
	done(1);
}

void SYProcessingDlg::onCloseTimer()
{
	done(1);
}

void SYProcessingDlg::SetAutoProcess(int waitTime)
{
	if(m_autoProcessTimer == nullptr){
		m_autoProcessTimer = new QTimer(this);
		m_autoProcessTimer->setInterval(100);
		connect(m_autoProcessTimer, &QTimer::timeout, this, &SYProcessingDlg::onAutoProcessTimer);
	}
	
	m_autoProcessTimer->start();
	m_processValue = 0;
	m_maxProcessValue = waitTime;
}

void SYProcessingDlg::onAutoProcessTimer()
{
	int step = m_autoProcessTimer->interval();
	m_processValue += step;
	SetProcess(100.f * m_processValue / m_maxProcessValue);
	if(m_processValue >= m_maxProcessValue){
		m_autoProcessTimer->stop();
		SetCompleted();
	}
}