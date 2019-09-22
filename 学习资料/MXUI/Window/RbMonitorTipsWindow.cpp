#include "RbMonitorTipsWindow.h"
#include <QDesktopWidget.h>
#include "MxDefine.h"
#include "MxglobalConfig.h"


RbMonitorTipsWindow::RbMonitorTipsWindow(QWidget *parent)
{
	setParent(parent);
	ui.setupUi(this);

	QDesktopWidget *desktop = QApplication::desktop();
	int screenIndex = desktop->primaryScreen();
	QRect dw = desktop->screenGeometry(screenIndex);
	QString strBackPixmap =  MxGlobalConfig::Instance()->GetSkinDir() + "/MonitorWindow/tipsWindow.png";

	this->setBackgroundStyleSheet(QString("QFrame#LayerFrame{\
										  border: 0 0 0 0 0;\
										  margin: 0 0 0 0 0;\
										  min-width: %1px;\
										  min-height: %2px;\
										  max-width: %1px;\
										  max-height: %2px;\
										  border-image: url(%3);}\
										  ").arg(dw.width()).arg(dw.height()+10).arg(strBackPixmap));
	ui.btn_OK->setText(QString::fromLocal8Bit("È·¶¨"));
	this->hideOkButton();
	this->hideCloseButton();
	setWindowState(Qt::WindowFullScreen);
	setAttribute(Qt::WA_TranslucentBackground,true);
	connect(ui.btn_OK, SIGNAL(clicked()), this, SLOT(onBtnOK()));

	Mx::setWidgetStyle(this,"qss:RbMonitorTipsWindow.qss");
}

RbMonitorTipsWindow::~RbMonitorTipsWindow()
{
}

void RbMonitorTipsWindow::onBtnOK()
{
	this->done(1);
}
