#include "SYEditPersonPasswordWindow.h"
#include "SYDBMgr.h"
#include <QSqlRecord>
#include "MxDefine.h"
#include "SYUserInfo.h"

SYEditPersonPasswordWindow::SYEditPersonPasswordWindow(QWidget *parent)
:RbShieldLayer(parent),
m_selectedMaleBtn(true)
{
	setParent(parent);
	ui.setupUi(this);

	m_emptyIcon.addFile("icons:/SYEditUserInfo/empty.png", QSize(32, 32));
	m_selectedIcon.addFile("icons:/SYEditUserInfo/selection.png", QSize(32, 32));

	this->hideCloseButton();
	this->hideOkButton();

	connect(ui.bt_ok, SIGNAL(clicked()), this, SLOT(onClickedOK()));
	connect(ui.bt_cancel, SIGNAL(clicked()), this, SLOT(onClickedCancel()));

	Mx::setWidgetStyle(this,"qss:SYEditPersonPasswordWindow.qss");
}

SYEditPersonPasswordWindow::~SYEditPersonPasswordWindow()
{

}

//取消按钮的槽函数
void SYEditPersonPasswordWindow::onClickedCancel()
{
	hide();
}

void SYEditPersonPasswordWindow::on_maleBtn_clicked()
{
	//ui.maleBtn->setIcon(m_selectedIcon);
	//ui.femaleBtn->setIcon(m_emptyIcon);
	ui.maleBtn->setChecked(true);
	ui.femaleBtn->setChecked(false);
	m_selectedMaleBtn = true;
}

void SYEditPersonPasswordWindow::on_femaleBtn_clicked()
{
	//ui.maleBtn->setIcon(m_emptyIcon);
	//ui.femaleBtn->setIcon(m_selectedIcon);
	ui.maleBtn->setChecked(false);
	ui.femaleBtn->setChecked(true);
	m_selectedMaleBtn = false;
}

//确认按钮的槽函数
void SYEditPersonPasswordWindow::onClickedOK()
{
	int userId = SYUserInfo::Instance()->GetUserId();
	bool canRefresh = false;
	bool canClose = false;

	QString oldPwd;									//旧密码
	QString newPwd;									//新密码

	oldPwd = ui.edit_oldPwd->text();
	newPwd = ui.edit_newPwd->text();

	if(oldPwd.length() || newPwd.length()){
		if(oldPwd.length() == 0){
			QMessageBox::information(this, "", QString::fromLocal8Bit("旧密码不能为空!"));
			return;
		}

		if(newPwd.length() == 0){
			QMessageBox::information(this, "", QString::fromLocal8Bit("新密码不能为空!"));
			return;
		}

		bool isRight = SYUserInfo::Instance()->CheckPassword(oldPwd);
		if(!isRight)
		{
			QMessageBox::information(this, "", QString::fromLocal8Bit("旧密码错误!"));
			return;
		}
		else if(isRight && newPwd != oldPwd)
		{
			if(SYDBMgr::Instance()->UpdateUserPassword(SYUserInfo::Instance()->GetUserId(), newPwd))
			{
				canRefresh = true;
				canClose = true;
			}
		}
		else
		{
			QMessageBox::information(this, "", QString::fromLocal8Bit("新旧密码相同!"));
			return;
		}
	}
	
	if(SYUserInfo::Instance()->GetRealName() != ui.edit_realName->text())
	{
		canClose = false;
		if(SYDBMgr::Instance()->UpdateUserRealName(userId, ui.edit_realName->text()))
		{
			canRefresh = true;
			canClose = true;
		}
	}

	bool curIsMale = SYUserInfo::Instance()->IsMale();
	if((curIsMale && m_selectedMaleBtn == false) || ((curIsMale == false) && m_selectedMaleBtn)){
		canClose = false;
		if(SYDBMgr::Instance()->UpdateUserSex(userId, m_selectedMaleBtn)){
			canRefresh = true;
			canClose = true;
		}
	}

	if(canRefresh)
	{
		SYUserInfo::Instance()->Refresh();
		QMessageBox::information(this,"",QString::fromLocal8Bit("修改成功"));
	}

	if(canClose)
		hide();
}

void SYEditPersonPasswordWindow::showEvent(QShowEvent* event)
{
	ui.usernameLabel->setText(SYUserInfo::Instance()->GetUserName());
	ui.edit_realName->setText(SYUserInfo::Instance()->GetRealName());

	//sex
	if(SYUserInfo::Instance()->IsMale()){
		//ui.maleBtn->setIcon(m_selectedIcon);
		//ui.femaleBtn->setIcon(m_emptyIcon);
		ui.maleBtn->setChecked(true);
		ui.femaleBtn->setChecked(false);
		m_selectedMaleBtn = true;
	}
	else{
		//ui.maleBtn->setIcon(m_emptyIcon);
		//ui.femaleBtn->setIcon(m_selectedIcon);
		ui.maleBtn->setChecked(false);
		ui.femaleBtn->setChecked(true);
		m_selectedMaleBtn = false;
	}

	ui.edit_oldPwd->clear();
	ui.edit_newPwd->clear();
}

