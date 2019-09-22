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
		ui.titleLabel->setText(CHS("�����ʦ�û�"));
	}
	else{
		m_permission = UP_Student;
		ui.titleLabel->setText(CHS("����ѧ���û�"));
	}
}

void SYImportUserWindow::showEvent(QShowEvent* event)
{
	ui.tipsLabel->clear();
}

void SYImportUserWindow::on_importUserBtn_clicked()
{
	m_fileName = QFileDialog::getOpenFileName(this, CHS("ѡ������ļ�"),"", "xls (*.xls)");
	ui.tipsLabel->setText(m_fileName);
}

void SYImportUserWindow::on_okBtn_clicked()
{
	if(m_fileName.size() == 0){
		InternalMessageBox(CHS("��ʾ"), CHS("��ѡ���ļ����ٲ���"));
		return;
	}

	QVector<QVector<QVariant>> records;
	bool ok = Mx::readDataFromExcelFile(m_fileName, records, 4);
	if(ok == false){
		//InternalMessageBox(CHS("��ʾ"), CHS("��ȡ�ļ�ʧ��"));
		ui.tipsLabel->setText(CHS("��ȡ�ļ�ʧ��"));
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

	InternalMessageBox(CHS("��ʾ"), CHS("��%1�����ݣ��ɹ�����%2��").arg(n).arg(nImportedUser));
	ui.tipsLabel->setText(CHS("��%1�����ݣ��ɹ�����%2��").arg(n).arg(nImportedUser));


	m_fileName.clear();
	done(RbShieldLayer::RC_Ok);
}

void SYImportUserWindow::on_cancelBtn_clicked()
{
	done(RbShieldLayer::RC_Cancel);
}