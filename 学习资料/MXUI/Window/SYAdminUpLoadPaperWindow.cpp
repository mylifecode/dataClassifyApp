#include "SYAdminUpLoadPaperWindow.h"
#include "ui_SYAdminUpLoadPaperWindow.h"
#include "SYMessageBox.h"
#include"MxDefine.h"
#include"SYDBMgr.h"
#include<QFileDialog>
#include"SYStringTable.h"

SYAdminUpLoadPaperWindow::SYAdminUpLoadPaperWindow(QWidget *parent) :
RbShieldLayer(parent)
{
    ui.setupUi(this);

	hideOkButton();
	hideCloseButton();
	
	Mx::setWidgetStyle(this, QString("qss:SYAdminUpLoadPaperWindow.qss"));
	setAttribute(Qt::WA_DeleteOnClose);
	
	connect(ui.addFileBtn, &QPushButton::clicked, this, &SYAdminUpLoadPaperWindow::on_addFile_Btn_Clicked);
	connect(ui.cancelBtn, &QPushButton::clicked, this, &SYAdminUpLoadPaperWindow::on_cancel_Btn_Clicked);
	connect(ui.confirmBtn, &QPushButton::clicked, this, &SYAdminUpLoadPaperWindow::on_confirm_Btn_Clicked);

	//Òþ²ØÌáÊ¾
	ui.tipsLabel->clear();
}

SYAdminUpLoadPaperWindow::~SYAdminUpLoadPaperWindow()
{

}


void SYAdminUpLoadPaperWindow::on_addFile_Btn_Clicked()
{
	QString UploadFileTips = SYStringTable::GetString(SYStringTable::STR_UploadFileTips);
	m_fileName = QFileDialog::getOpenFileName(this, UploadFileTips, "", "xls (*.xls *.xlsx)");
	if (m_fileName.size() > 0)
	{
		QString fileName = m_fileName.split("/")[-1];
		ui.tipsLabel->setText(fileName);
	}
	
}

void SYAdminUpLoadPaperWindow::on_cancel_Btn_Clicked()
{

	done(RbShieldLayer::RC_Cancel);

}

void SYAdminUpLoadPaperWindow::on_confirm_Btn_Clicked()
{

	if (m_fileName.size() == 0){
		InternalMessageBox("", SYStringTable::GetString(SYStringTable::STR_AfterChosedFilesOperation));
		return;
	}

	QVector<QVector<QVariant>> datas;
	bool ok = Mx::readDataFromExcelFile(m_fileName, datas, 8);   
	if (ok == false){
		ui.tipsLabel->setText(SYStringTable::GetString(SYStringTable::STR_ReadFilesError));
		return;
	}

	
	QVector<QVector<QString>> paperQuestions;
	for (int i = 0; i < datas.size(); i++)
	{
		QVector<QString> question;
		QString title = datas[i][0].toString();
		QString a = datas[i][1].toString();
		QString b= datas[i][2].toString();
		QString c = datas[i][3].toString();
		QString d = datas[i][4].toString();
		QString e = datas[i][5].toString();
		QString type = datas[i][6].toString();
		QString answer = datas[i][7].toString();

		question.push_back(a);
		question.push_back(b);
		question.push_back(c);
		question.push_back(d);
		question.push_back(e);
		question.push_back(title);
		question.push_back(answer);
		question.push_back(type);
	
		paperQuestions.push_back(question);
	}

	SYDBMgr::Instance()->FromExcelUpLoadPaperInfo(paperQuestions);
	m_fileName.clear();
	done(RbShieldLayer::RC_Ok);

}