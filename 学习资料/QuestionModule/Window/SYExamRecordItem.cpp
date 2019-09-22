#include "MxDefine.h"
#include "SYExamRecordItem.h"
#include <tchar.h>
#include"SYExamDataManager.h"
void FormatTime(float time, QChar outtimeStr[7])
{
	int minutes = time / 60;

	int seconds = (time - minutes * 60);

	int  minute0 = minutes / 10;
	int  minute1 = minutes - 10 * minute0;
	outtimeStr[0] = minute0 + _T('0');
	outtimeStr[1] = minute1 + _T('0');
	outtimeStr[2] = _T('·Ö');

	int  second0 = seconds / 10;
	int  second1 = seconds - 10 * second0;
	outtimeStr[3] = second0 + _T('0');
	outtimeStr[4] = second1 + _T('0');
	outtimeStr[5] = _T('Ãë');
	outtimeStr[6] = 0;
}

SYExamRecordItem::SYExamRecordItem(QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this);
	Mx::setWidgetStyle(this, "qss:SYQuestionExamRecordItem.qss");
}

SYExamRecordItem::~SYExamRecordItem()
{

}

void SYExamRecordItem::SetContent(const QString & date, const QString & name, int QuestNum, int AccuracyNum, int UseSeconds)
{
	ui.lb_data->setText(date);
	ui.lb_name->setText(name);

	QString temp = QString::number(AccuracyNum) + QString("/") + QString::number(QuestNum);
	ui.lb_percent->setText(temp);

	QChar outtimeStr[7];
	FormatTime(UseSeconds, outtimeStr);

	temp = QString(outtimeStr);
	ui.lb_usedtime->setText(temp);

	int accuracyRate = 100*(QuestNum > 0 ? (float)AccuracyNum / (float)QuestNum : 0.0f);
	temp = QString::number(accuracyRate) + QString("%");
	ui.lb_rightpercent->setText(temp);
}
void SYExamRecordItem::on_bt_desc_clicked()
{
	emit OnDescButtonClicked(this);
}

void SYExamRecordItem::on_bt_test_clicked()
{
	emit OnReDoButtonClicked(this);
}

SYExamPaperItem::SYExamPaperItem(QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this);

	Mx::setWidgetStyle(this, "qss:SYQuestionExamRecordItem.qss");
}

SYExamPaperItem::~SYExamPaperItem()
{

}

void SYExamPaperItem::SetContent(QString & name, QString & num, QString & author)
{
	ui.lb_name->setText(name);
	ui.lb_num->setText(num);
	ui.lb_author->setText(author);

}

int SYExamPaperItem::GetPaperId()
{
	int paperId;
	QString paperName = ui.lb_name->text();
	paperId = SYExamDataManager::GetInstance().GetExamMissionPaperId(paperName);

	return paperId;

}

QString SYExamPaperItem::GetPaperName()
{
	QString paperName = ui.lb_name->text();
	return paperName;
}

void SYExamPaperItem::SetColumn3Content(QString & clo3)
{
	ui.lb_author->setText(clo3);
}

void SYExamPaperItem::SetColumnLabelStyleSheet(int colum, const QString & style)
{
	if (colum == 3)
	    ui.lb_author->setStyleSheet(style);
}

QPushButton* SYExamPaperItem::GetReturnBtn()
{

	if (ui.lb_name)
	{
		return ui.lb_name;
	}
	return NULL;
}