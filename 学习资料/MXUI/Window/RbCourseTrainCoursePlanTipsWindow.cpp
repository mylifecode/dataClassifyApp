#include "RbCourseTrainCoursePlanTipsWindow.h"
#include <QDesktopWidget.h>
#include "MxDefine.h"
#include "MxGlobalConfig.h"

RbCourseTrainCoursePlanTipsWindow::RbCourseTrainCoursePlanTipsWindow(QWidget *parent, QPoint posBtnEdit, QPoint posBtnAdd,
																	 QPoint posBtnBatchEdit):RbShieldLayer(parent)
{
	setParent(parent);
	ui.setupUi(this);
	ui.btnEdit->move(posBtnEdit);
	ui.btnAdd->move(posBtnAdd);
	ui.btnBatchEdit->move(posBtnBatchEdit);
	ui.btnEdit->setEnabled(false);
	ui.btnAdd->setEnabled(false);
	ui.btnBatchEdit->setEnabled(false);

	QDesktopWidget *desktop = QApplication::desktop();
	int screenIndex = desktop->primaryScreen();
	QRect dw = desktop->screenGeometry(screenIndex);
	QString strBackPixmap =  MxGlobalConfig::Instance()->GetSkinDir() + "/CourseTrain/shieldLayer.png";

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

	Mx::setWidgetStyle(this,"qss:RbCourseTrainCoursePlanTipsWindow.qss");
}

RbCourseTrainCoursePlanTipsWindow::~RbCourseTrainCoursePlanTipsWindow()
{
}

void RbCourseTrainCoursePlanTipsWindow::onBtnOK()
{
	this->done(1);
	emit tipsWindowClose();
}
