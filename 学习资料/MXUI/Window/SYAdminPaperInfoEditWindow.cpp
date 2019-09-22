#include "SYAdminPaperInfoEditWindow.h"
#include "ui_SYAdminPaperInfoEditWindow.h"
#include"SYMainWindow.h"
#include "SYMessageBox.h"
#include"MxDefine.h"
//#include"SYDBMgr.h"
#include<QRegExp>
#include"SYStringTable.h"

#define CHS(txt)  QString::fromLocal8Bit(txt)

SYAdminPaperInfoEditWindow::SYAdminPaperInfoEditWindow(int chosedNum, QWidget *parent, PaperInfoDetail* paperInfoDetail) :
RbShieldLayer(parent), m_paperInfoDetail(paperInfoDetail)

{
    ui.setupUi(this);
	hideCloseButton();
	hideOkButton();

	if (m_paperInfoDetail)
		SetPage();

	//窗口设置

	ui.questionNum->setText(QString::number(chosedNum));  //设置选择试题数量
	connect(ui.paperName, &QLineEdit::textChanged, this, [=](QString text)
	{
		ui.curPaperNum->setText(QString::number(text.size()));

	});

	//样式设置
	Mx::setWidgetStyle(this, "qss:SYAdminPaperInfoEditWindow.qss");
	
	connect(ui.cancelBtn, &QPushButton::clicked, this, &SYAdminPaperInfoEditWindow::on_Btn_clicked);
	connect(ui.confirmBtn, &QPushButton::clicked, this,&SYAdminPaperInfoEditWindow::on_Btn_clicked);
	//setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);

	connect(ui.answerTime_lineEdit_1, &QLineEdit::textEdited, this, [=](QString text)
	{

		if (text == "0" || text == "00")
			ui.answerTime_lineEdit_1->clear();


	});
	connect(ui.paperScoreLineEdit, &QLineEdit::textEdited, this, [=](QString text)
	{

		if (text == "0" || text == "00" || text == "000")
			ui.paperScoreLineEdit->clear();
		if (text.size() > 0)
		{
			ui.paperScoreLineEdit->setStyleSheet(QString("min-height:70px;max-height:70px;font-size:16px;color:#b7c5d8;background-color:#1b1f29;box-shadow:inset 0px 1px 4px 0px #11141a;border-radius:4px;border:solid 1px #46526d;"));
		}

	});

	ui.answerTime_lineEdit_1->setValidator(new QIntValidator(1, 60, this));
	ui.paperScoreLineEdit->setValidator(new QIntValidator(1, 100, this));
	
}

SYAdminPaperInfoEditWindow::~SYAdminPaperInfoEditWindow()
{

}

void SYAdminPaperInfoEditWindow::SetPage()
{
	QString paperName = m_paperInfoDetail->paperName;
	ui.paperName->setText(paperName);
	QString examTime = m_paperInfoDetail->examTime;
	QString secondTime = examTime.split(":", QString::SkipEmptyParts)[0];
	ui.answerTime_lineEdit_1->setText(secondTime);
	
	QString score = m_paperInfoDetail->examScore;
	
	ui.paperScoreLineEdit->setText(score);


}
void SYAdminPaperInfoEditWindow::on_Btn_clicked()
{

	QPushButton* Btn = dynamic_cast<QPushButton*>(sender());
	if (Btn == ui.confirmBtn)
	{

		QString paperName = ui.paperName->text();
		if (paperName.size()==0)
		{
			QString paperNameInputTips = SYStringTable::GetString(SYStringTable::STR_InputPaperName);
			SYMessageBox* messageBox = new SYMessageBox(this, "", paperNameInputTips, 1);
			messageBox->setPicture("Shared/personnel_pic_.png");
			messageBox->showFullScreen();
			messageBox->exec();
			return;
		}

		QString examTime = ui.answerTime_lineEdit_1->text();
		if (examTime.size() == 0)
		{
			SYMessageBox* messageBox = new SYMessageBox(this, "", CHS("考试答题时间不能为空"), 1);
			messageBox->setPicture("Shared/personnel_pic_.png");
			messageBox->showFullScreen();
			messageBox->exec();
			return;
		}


		QString examTotalScore = ui.paperScoreLineEdit->text();
		if (examTotalScore.isEmpty())
		{
			SYMessageBox* messageBox = new SYMessageBox(this, "", CHS("考试总分不能设置为空"), 1);
			messageBox->setPicture("Shared/personnel_pic_.png");
			messageBox->showFullScreen();
			messageBox->exec();
			return;
		}
		
		QString prePaperName = m_paperInfoDetail->paperName;
		bool ret=SYDBMgr::Instance()->QueryPaperExist(paperName);
		if (ret&&prePaperName!=paperName)
		{

			SYMessageBox* messageBox = new SYMessageBox(this, "", CHS("该试卷已经存在!"), 1);
			messageBox->setPicture("Shared/personnel_pic_.png");
			messageBox->showFullScreen();
			messageBox->exec();
			//ui.paperName->clear();
			return;
		}

		examTime = examTime + ":" + "00";
		emit createPaper(paperName, examTime, examTotalScore);

		done(RbShieldLayer::RC_Ok);
	}
	else
	done(RbShieldLayer::RC_Cancel);
}






