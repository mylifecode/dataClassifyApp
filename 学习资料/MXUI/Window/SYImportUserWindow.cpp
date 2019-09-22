#include "SYImportUserWindow.h"
#include <MxDefine.h>
#include <QFileDialog>
#include <QVector>
#include <QVariant>
#include <SYDBMgr.h>

SYImportUserWindow::SYImportUserWindow(QWidget* parent)
	:RbShieldLayer(parent),
	m_permission(UP_Student)
{
	ui.setupUi(this);

	hideCloseButton();
	hideOkButton();

	Mx::setWidgetStyle(this, "qss:SYImportUserWindow.qss");
}

SYImportUserWindow::~SYImportUserWindow()
{

}

void SYImportUserWindow::setUserPermission(UserPermission permission)
{
	if(permission == UP_Teacher){
		m_permission = UP_Teacher;
		ui.titleLabel->setText(CHS("导入教师用户"));
	}
	else{
		m_permission = UP_Student;
		ui.titleLabel->setText(CHS("导入学生用户"));
	}
}

void SYImportUserWindow::showEvent(QShowEvent* event)
{
	ui.tipsLabel->clear();
}

void SYImportUserWindow::on_importUserBtn_clicked()
{
	m_fileName = QFileDialog::getOpenFileName(this, CHS("选择导入的文件"),"", "xls (*.xls)");
	ui.tipsLabel->setText(m_fileName);
}

void SYImportUserWindow::on_okBtn_clicked()
{
	if(m_fileName.size() == 0){
		InternalMessageBox(CHS("提示"), CHS("请选择文件后再操作"));
		return;
	}

	QVector<QVector<QVariant>> records;
	bool ok = Mx::readDataFromExcelFile(m_fileName, records, 4);
	if(ok == false){
		//InternalMessageBox(CHS("提示"), CHS("读取文件失败"));
		ui.tipsLabel->setText(CHS("读取文件失败"));
		return;
	}

	QString userName, realName, password, cs;
	int nImportedUser = 0;
	for(int row = 1; row < records.size(); ++row){
		auto& record = records[row];
		userName = record[0].toString();		//user name

		if(SYDBMgr::Instance()->QueryUserInfo(userName, QSqlRecord())){
			continue;
		}
		else{
			realName = record[1].toString();	//real name
			password = record[2].toString();	//password
			if(password.size() == 0)
				password.append("123456");
			//cs = record[3].toString();			//class

			if(SYDBMgr::Instance()->AddUserInfo(userName, password, password, realName, m_permission)){
				nImportedUser++;
			}
		}
	}

	int n = records.size() - 1;
	if(n < 0)
		n = 0;

	InternalMessageBox(CHS("提示"), CHS("共%1条数据，成功导入%2条").arg(n).arg(nImportedUser));
	ui.tipsLabel->setText(CHS("共%1条数据，成功导入%2条").arg(n).arg(nImportedUser));


	m_fileName.clear();
	done(RbShieldLayer::RC_Ok);
}

void SYImportUserWindow::on_cancelBtn_clicked()
{
	done(RbShieldLayer::RC_Cancel);
}