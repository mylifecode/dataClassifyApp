#include "SYLoginWindow.h"
#include "SYDBMgr.h"
#include "SYUserInfo.h"
#include "MxGlobalConfig.h"
#include "SYMainWindow.h"
#include "SYStringTable.h"

#define MAKESKINDIR (MxGlobalConfig::Instance()->GetSkinDir())


SYLoginWindow::SYLoginWindow(QWidget * parent, Qt::WindowFlags flag) : QWidget(parent, flag)
{
	ui.setupUi(this);	

	ui.titlelabel->setText(SYStringTable::GetString(SYStringTable::STR_SYSTEMNAME));
	ui.userNameLineEdit->setPlaceholderText(QString::fromLocal8Bit("用户名"));	
	ui.passwordLineEdit->setEchoMode( QLineEdit::Password );
	ui.passwordLineEdit->setPlaceholderText(QString::fromLocal8Bit("密码"));
	ui.userloginBtn->setDefault(true);
	ui.userloginBtn->setFocus();
	ui.userloginBtn->setShortcut(QKeySequence::InsertParagraphSeparator);

	//ui.userNameLineEdit->setNormalPic(QString(MAKESKINDIR + "/Login/userNameEdit_normal.png"));
	//ui.userNameLineEdit->setTextChangedPic(QString(MAKESKINDIR + "/Login/userNameEdit_changed.png"));

	//ui.passwordLineEdit->setNormalPic(QString(MAKESKINDIR + "/Login/passwordEdit_normal.png"));
	//ui.passwordLineEdit->setTextChangedPic(QString(MAKESKINDIR + "/Login/passwordEdit_changed.png"));

	bool visible = true;
	if(MxGlobalConfig::Instance()->GetUserMode() == MxGlobalConfig::UM_Normal)
		visible = false;
	ui.guestloginBtn->setVisible(visible);

	connect(ui.userNameLineEdit, &QLineEdit::returnPressed, this, &SYLoginWindow::on_userloginBtn_clicked);
	connect(ui.passwordLineEdit, &QLineEdit::returnPressed, this, &SYLoginWindow::on_userloginBtn_clicked);

	Mx::setWidgetStyle(this, "qss:SYLoginWindow.qss");
}

SYLoginWindow::~SYLoginWindow(void)
{

}

void SYLoginWindow::on_userloginBtn_clicked()
{
	QString name = ui.userNameLineEdit->text();			//user name or real name
	QString pwd = ui.passwordLineEdit->text();

	if (name.isEmpty() || pwd.isEmpty())
	{
		ui.tipLabel->setText(QString::fromLocal8Bit("用户名或密码不能为空"));
		return;
	}

	// 登录用户
	SYDBMgr::Instance()->ReOpen();
	bool bRet = SYUserInfo::Instance()->Login(name, pwd);

	if(bRet)
	{
		UserPermission permission = SYUserInfo::Instance()->GetUserPermission();

		if (permission >= UP_Teacher)
			emit showNextWindow(WT_AdminPortalWindow);
		else
			emit showNextWindow(WT_TrainingCenterWindow);
	}
	else
	{
		ui.tipLabel->setText(QString::fromLocal8Bit("用户名或密码有误"));
	}

}

void SYLoginWindow::on_guestloginBtn_clicked()
{
	//匿名登录
	SYUserInfo::Instance()->Login();

	emit showNextWindow(WT_TrainingCenterWindow);
}

void SYLoginWindow::showEvent(QShowEvent* e)
{
	ui.userNameLineEdit->clear();
	ui.passwordLineEdit->clear();
	ui.tipLabel->setText(QString(""));
}