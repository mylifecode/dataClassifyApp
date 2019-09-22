#include "SYAdminPaperContentFrameWindow.h"
#include "ui_SYAdminPaperContentFrameWindow.h"
#include "MxDefine.h"
#include"SYStringTable.h"
SYAdminPaperContentFrameWindow::SYAdminPaperContentFrameWindow(QSqlRecord record, int index, QWidget *parent) :
QWidget(parent), cur_record(record), cur_index(index), chosed(false)

{
    ui.setupUi(this);
    Mx::setWidgetStyle(this, "qss:SYAdminPaperContentFrameWindow.qss");
	Initalize();
}

void SYAdminPaperContentFrameWindow::Initalize()
{
	
	QString title = cur_record.value("title").toString();
    ui.title->setText(QString("%1.").arg(cur_index+1)+title);

	QString opta = cur_record.value("opta").toString();
	ui.option1->setText(QString("A.")+opta);
	QString optb = cur_record.value("optb").toString();
	ui.option2->setText(QString("B.") + optb);
	QString optc = cur_record.value("optc").toString();
	ui.option3->setText(QString("C.") + optc);
	QString opte = cur_record.value("opte").toString();
	ui.option4->setText(QString("D.") + opte);
	QString answer = cur_record.value("answer").toString();

	QString Right_Answer = SYStringTable::GetString(SYStringTable::STR_Right_Answer);
	ui.answer->setText(Right_Answer + answer);
}                                                                                   


 SYAdminPaperContentFrameWindow::~SYAdminPaperContentFrameWindow()
{

}
