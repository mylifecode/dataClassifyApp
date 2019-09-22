#include "SYAdminPaperContentShowWindow.h"
//#include"SYAdminPaperContentFrameWindow.h"
#include"SYAdminQuestionsMgrWindow.h"
#include"MxDefine.h"
#include"SYDBMgr.h"
#include<QScrollArea>



SYAdminPaperContentShowWindow::SYAdminPaperContentShowWindow(QVector<QSqlRecord> records, QWidget *parent) :
QWidget(parent), result(records)
{


	SetWidgetContent();
	Mx::setWidgetStyle(this, "qss:SYAdminPaperContentShowWindow.qss");
	
}

SYAdminPaperContentShowWindow::~SYAdminPaperContentShowWindow()
{

}

void  SYAdminPaperContentShowWindow::SetWidgetContent()
{

	QVBoxLayout* vLayout = new QVBoxLayout;
	for (int i = 0;i< result.size();i++)
	{
		vLayout->addSpacerItem(new QSpacerItem(1278, 16, QSizePolicy::Expanding));
		QuestionFrame* frame = new QuestionFrame(result[i], i);
		vLayout->addWidget(frame);
	}
	setLayout(vLayout);

}