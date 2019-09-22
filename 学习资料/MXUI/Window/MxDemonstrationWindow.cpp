#include "MxDemonstrationWindow.h"
#include <QTimer>
#include "MxNetworkVideoDecoder.h"
#include <QPainter>
#include "MxDefine.h"
#include "MxSessionCommand.h"
#include "SYUserInfo.h"
#include "SYMessageBox.h"
MxDemonstrationWindow::MxDemonstrationWindow(QWidget* parent)
	:QWidget(parent),
	m_decoder(nullptr),
	m_hasDemonstration(false)
{
	m_ui.setupUi(this);
	m_ui.exitBtn->hide();
	m_ui.demonstrationWindow->installEventFilter(this);

	connect(m_ui.exitBtn, SIGNAL(clicked()), this, SLOT(onClickedExitBtn()));

	m_decoder = new MxNetworkVideoDecoder(this);

	connect(m_decoder, SIGNAL(DecodeFinish(const QPixmap&)), this, SLOT(onDecodeFinish(const QPixmap&)));
	connect(m_decoder, SIGNAL(BeginDemonstration()), this, SLOT(onDemonstrationBegin()));
	connect(m_decoder, SIGNAL(StopDemonstration()), this, SLOT(onDemonstrationStop()));
	connect(&m_timer, SIGNAL(timeout()), this, SLOT(onSendWatchCommand()));

	Mx::setWidgetStyle(this, "qss:MxDemonstrationWindow.qss");
}

MxDemonstrationWindow::~MxDemonstrationWindow()
{
	m_decoder->SendCommand(MxSessionCommand(MxSessionCommand::CT_Exit));
	delete m_decoder;
}

bool MxDemonstrationWindow::eventFilter(QObject* obj,QEvent* event)
{
	if(obj == m_ui.demonstrationWindow && event->type() == QEvent::Paint)
	{
		QPainter painter(m_ui.demonstrationWindow);
		if(m_pixmap.isNull())
		{
			painter.fillRect(m_ui.demonstrationWindow->rect(), QColor(0, 0, 0));
		}
		else
		{
			painter.drawPixmap(m_ui.demonstrationWindow->rect(), m_pixmap, m_pixmap.rect());
		}

		return true;
	}

	return __super::eventFilter(obj, event);
}

void MxDemonstrationWindow::onClickedExitBtn()
{
	emit ExitCurrentWindow(WT_DemonstrationWindow);
}

void MxDemonstrationWindow::showEvent(QShowEvent* event)
{
	m_timer.start(3000);
}

void MxDemonstrationWindow::hideEvent(QHideEvent* event)
{
	m_timer.stop();
	if(m_decoder)
		m_decoder->SendCommand(MxSessionCommand::CT_StopWatch);
}

void MxDemonstrationWindow::onDecodeFinish(const QPixmap& frame)
{
	if(m_hasDemonstration)
	{
		m_pixmap = frame;
		m_ui.demonstrationWindow->repaint();
		m_ui.exitBtn->setVisible(true);
	}
}

void MxDemonstrationWindow::onDemonstrationBegin()
{
	if(m_hasDemonstration == false)
	{
		if(!isVisible())
		{
			SYMessageBox* msgBox = new SYMessageBox(nullptr, CHS(""), CHS("教师发起示教请至网络模块观看"));
			msgBox->showFullScreen();
			msgBox->exec();
		}

		m_ui.exitBtn->setVisible(true);
		m_hasDemonstration = true;

		if(m_decoder && isVisible())
			m_decoder->SendCommand(MxSessionCommand::CT_BeginWatch);
	}
}

void MxDemonstrationWindow::onDemonstrationStop()
{
	if(m_hasDemonstration)
	{
		m_pixmap = QPixmap();
		m_ui.demonstrationWindow->repaint();
		m_ui.exitBtn->setVisible(false);
		m_hasDemonstration = false;

		//if(!isVisible())
		{
			SYMessageBox* msgBox = new SYMessageBox(this, CHS(""), CHS("教师已停止示教"));
			msgBox->showFullScreen();
			msgBox->exec();
		}
	}
}

void MxDemonstrationWindow::closeEvent(QCloseEvent* event)
{

}

void MxDemonstrationWindow::onSendWatchCommand()
{
	if(m_decoder)
	{
		m_decoder->SendCommand(MxSessionCommand::CT_BeginWatch);

		MxSessionCommand cmd;

		cmd.SetCommandType(MxSessionCommand::CT_Update);
		cmd.AddData("UserName", SYUserInfo::Instance()->GetUserName());
		cmd.AddData("RealName", SYUserInfo::Instance()->GetRealName());	//如果无真实姓名，那么此操作无效

		m_decoder->SendCommand(cmd);
	}
}