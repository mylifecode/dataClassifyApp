#include "SYAddUserWindow.h"
#include <MxDefine.h>
#include <QRegExp>
#include <QValidator>
#include <SYDBMgr.h>

SYAddUserWindow::SYAddUserWindow(QWidget* parent)
	:RbShieldLayer(parent),
	m_permission(UP_Student)
{
	ui.setupUi(this);

	QRegExp reg1("[a-zA-Z]+$");
	QValidator* validator1 = new QRegExpValidator(reg1, this);
	ui.userNameLineEdit->setValidator(validator1);

	QRegExp reg2("[a-zA-Z0-9]+$");
	QValidator* validator2 = new QRegExpValidator(reg2, this);
	ui.initPasswordLineEdit->setValidator(validator2);

	hideOkButton();
	hideCloseButton();

	Mx::setWidgetStyle(this, "qss:SYAddUserWindow.qss");
}

SYAddUserWindow::~SYAddUserWindow()
{

}

void SYAddUserWindow::setUserPermission(UserPermission up)
{
	if(up == UP_Teacher){
		m_permission = UP_Teacher;
		ui.titleLabel->setText(CHS("�½���ʦ"));
	}
	else{
		m_permission = UP_Student;
		ui.titleLabel->setText(CHS("�½�ѧԱ"));
	}
}

void SYAddUserWindow::showEvent(QShowEvent* event)
{
	ui.userNameLineEdit->clear();
	ui.realNameLineEdit->clear();
	ui.initPasswordLineEdit->clear();
	ui.classLineEdit->clear();
}

void SYAddUserWindow::on_cancelBtn_clicked()
{
	done(RC_Cancel);
}

void SYAddUserWindow::on_okBtn_clicked()
{
	//user name
	QString userName = ui.userNameLineEdit->text();
	if(userName.size() == 0){
		InternalMessageBox("error", CHS("�������û���"));
		return;
	}
	else if(userName.size() > 15){
		InternalMessageBox("error", CHS("�û������Ȳ��Ϸ�"));
		return;
	}

	//real name
	QString realName = ui.realNameLineEdit->text();
	if(realName.size() == 0){
		InternalMessageBox("error", CHS("��������ʵ����"));
		return;
	}
	else if(realName.size() > 5){
		InternalMessageBox("error", CHS("��ʵ�������Ȳ��Ϸ�"));
		return;
	}

	//password
	QString initPassword = ui.initPasswordLineEdit->text();
	if(initPassword.size() == 0){
		InternalMessageBox("error", CHS("�������ʼ����"));
		return;
	}
	else if(initPassword.size() > 10){
		InternalMessageBox("error", CHS("��ʼ���볤�Ȳ��Ϸ�"));
		return;
	}

	//class
	QString cs = ui.classLineEdit->text();


	//begin add user info
	QSqlRecord record;
	bool found = false;
	
	found = SYDBMgr::Instance()->QueryUserInfo(userName, record);
	if(found){
		InternalMessageBox("error", CHS("�û����Ѵ���"));
		return;
	}

	int ret = SYDBMgr::Instance()->AddUserInfo(userName, initPassword, initPassword, realName, m_permission);
	if(ret == -1){
		InternalMessageBox("error", CHS("����û�ʧ��"));
		return;
	}

	done(RC_Ok);
}