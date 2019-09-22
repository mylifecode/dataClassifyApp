#include "SYNewBuiltQuestionWindow.h"
#include "ui_SYNewBuiltQuestionWindow.h"
#include"MxDefine.h"
#include"SYDBMgr.h"
#include"SYMessageBox.h"
#include"SYMessageBox.h"
#include"SYStringTable.h"

SYNewBuiltQuestionWindow::SYNewBuiltQuestionWindow(int type, QSqlRecord* record, QWidget *parent ) :
RbShieldLayer(parent), m_curType(type), m_record(record)

{
    ui.setupUi(this);
	Mx::setWidgetStyle(this, QString("qss:SYNewBuiltQuestionWindow.qss"));
	hideCloseButton();
	hideOkButton();
	if (record)
	{
		InitPage();
		ui.winTitle->setText(CHS("编辑题目"));
	}
	else
	{
		ui.winTitle->setText(CHS("新建题目"));
	}
	connect(ui.cancelBtn, &QPushButton::clicked, this, &SYNewBuiltQuestionWindow::do_btn_Clicked);
	connect(ui.confirmBtn, &QPushButton::clicked, this, &SYNewBuiltQuestionWindow::do_btn_Clicked);
	
}


SYNewBuiltQuestionWindow::~SYNewBuiltQuestionWindow()
{
   
}

void SYNewBuiltQuestionWindow::InitPage()
{

	QString optA = m_record->value("opta").toString();
	QString optB = m_record->value("optb").toString();
	QString optC = m_record->value("optc").toString();
	QString optD = m_record->value("optd").toString();
	QString optE = m_record->value("opte").toString();
	QString questionTitle = m_record->value("title").toString();

	 ui.optALineEdit->setText(optA);
	 ui.optBLineEdit->setText(optB);
	 ui.optCLineEdit->setText(optC);
	 ui.optDLineEdit->setText(optD);
	 ui.optELineEdit->setText(optE);
	 ui.titleLineEdit->setText(questionTitle);

	QString answer = m_record->value("answer").toString();
	QVector<QString> strVector;
	for (std::size_t i = 0; i < answer.size(); i++)
	{
		QString str = answer[i];
		strVector.push_back(str);
	}
	for (std::size_t i = 0; i < strVector.size(); i++)
	{
		if (strVector[i] == "A")
			ui.aBox->setChecked(true);
		if (strVector[i] == "B")
			ui.bBox->setChecked(true);
		if (strVector[i] == "C")
			ui.cBox->setChecked(true);
		if (strVector[i] == "D")
			ui.dBox->setChecked(true);
		if (strVector[i] == "E")
			ui.eBox->setChecked(true);
	}

}

bool SYNewBuiltQuestionWindow::VerifyOption(QString questionTitle,QString optA,QString optB,QString optC,QString optD,QString optE,QString answer)
{
	//选项和答案验证

	if (questionTitle.size() > 0)
	{

			if (optA.size() == 0 || optB.size() == 0)
			{
				QString paperOptionNeedContinuous = SYStringTable::GetString(SYStringTable::STR_PaperOptionNeedContinuous);
				SYMessageBox* box = new SYMessageBox(this, "", paperOptionNeedContinuous, 1);
				box->showFullScreen();
				box->exec();
				return false;
			}


			if (optA.size() > 0 && optB.size() > 0)
			{
				if (optC.size() == 0 && (optD.size() > 0|| optE.size() > 0))
				{
					QString paperOptionNeedContinuous = SYStringTable::GetString(SYStringTable::STR_PaperOptionNeedContinuous);
					SYMessageBox* box = new SYMessageBox(this, "",paperOptionNeedContinuous, 1);
					box->showFullScreen();
					box->exec();
					return false;
				}

				if (optC.size() == 0 && optD.size() == 0 && optE.size() == 0)
				{

					if (ui.cBox->isChecked() || ui.dBox->isChecked() || ui.eBox->isChecked())
					{
						QString paperAnswerSetFault = SYStringTable::GetString(SYStringTable::STR_PaperAnswerSetFault);
						SYMessageBox* box = new SYMessageBox(this, "", paperAnswerSetFault, 1);
						box->showFullScreen();
						box->exec();
						return false;

					}

				}
			}

			if (optA.size() > 0 && optB.size() > 0 && optC.size() >0)
			{
				if (  optD.size() == 0 && optE.size() > 0)
				{
					QString paperOptionNeedContinuous = SYStringTable::GetString(SYStringTable::STR_PaperOptionNeedContinuous);
					SYMessageBox* box = new SYMessageBox(this, "", paperOptionNeedContinuous, 1);
					box->showFullScreen();
					box->exec();
					return false;
				}

				if ( optD.size() == 0 && optE.size() == 0)
				{

					if ( ui.dBox->isChecked() || ui.eBox->isChecked())
					{

						QString paperAnswerSetFault = SYStringTable::GetString(SYStringTable::STR_PaperAnswerSetFault);
						SYMessageBox* box = new SYMessageBox(this, "", paperAnswerSetFault, 1);
						box->showFullScreen();
						box->exec();
						return false;

					}
				}
			}

			if (optA.size() > 0 && optB.size() > 0 && optC.size() >0 && optD.size() > 0)
			{
			
				if (optE.size() == 0)
				{

					if ( ui.eBox->isChecked())
					{

						QString paperAnswerSetFault = SYStringTable::GetString(SYStringTable::STR_PaperAnswerSetFault);
						SYMessageBox* box = new SYMessageBox(this, "", paperAnswerSetFault, 1);
						box->showFullScreen();
						box->exec();
						return false;

					}

				}
			}
	}
	else
	{
		QString paperTitleNotNull = SYStringTable::GetString(SYStringTable::STR_PaperTitleNotNull);
		SYMessageBox* box = new SYMessageBox(this, "", paperTitleNotNull, 1);
		box->showFullScreen();
		box->exec();
		return false;

	}
	if (!ui.bBox->isChecked() && !ui.bBox->isChecked() && !ui.cBox->isChecked() && !ui.dBox->isChecked() && !ui.eBox->isChecked())
	{
		QString paperAnswerNotNull = SYStringTable::GetString(SYStringTable::STR_PaperAnswerNotNull);
		SYMessageBox* box = new SYMessageBox(this, "", paperAnswerNotNull, 1);
		box->showFullScreen();
		box->exec();
		return false;
	}
	return true;
}

//选项和答案必须符合规则
void SYNewBuiltQuestionWindow::do_btn_Clicked()
{
	QPushButton* btn = static_cast<QPushButton*>(sender());
	if (btn == ui.confirmBtn)
	{
		QString optA = ui.optALineEdit->text();
		QString optB = ui.optBLineEdit->text();
		QString optC = ui.optCLineEdit->text();
		QString optD = ui.optDLineEdit->text();
		QString optE = ui.optELineEdit->text();
		QString questionTitle = ui.titleLineEdit->text();

		QString answer = "";
		if (ui.aBox->isChecked())
			answer += ui.aBox->text();
		if (ui.bBox->isChecked())
			answer += ui.bBox->text();
		if (ui.cBox->isChecked())
			answer += ui.cBox->text();
		if (ui.dBox->isChecked())
			answer += ui.dBox->text();
		if (ui.eBox->isChecked())
			answer += ui.eBox->text();

		if (VerifyOption(questionTitle, optA, optB, optC, optD, optE, answer))
		{
			int id;
			if (m_record)
				id = m_record->value("id").toInt();
			
			else
				id = -1;
			int Ret = SYDBMgr::Instance()->AddQuestion(m_curType, questionTitle, optA, optB, optC, optD, optE, answer, id);

			if (Ret == 1)
				done(RC_Ok);
			else
			{
				done(RC_Cancel);
			}

		}
	}
	else
		done(RC_Cancel);

}