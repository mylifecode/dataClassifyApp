#include "SYAdminShowPaperDetailWindow.h"
#include "ui_SYAdminShowPaperDetailWindow.h"
#include"SYAdminPaperContentShowWindow.h"
#include"SYAdminQuestionsMgrWindow.h"
#include"SYMainWindow.h"
#include<qpushbutton.h>
#include "MxDefine.h"
#include"SYDBMgr.h"
#include<qmap.h>

SYAdminShowPaperDetailWindow::SYAdminShowPaperDetailWindow(QString paperName,QWidget *parent) :
cur_PaperName(paperName), RbShieldLayer(parent)
    
{
    ui.setupUi(this);
	Initialize();  //从数据库中读取数据  
	//setWindowFlags(Qt::FramelessWindowHint);
	//setAcceptDrops(false);
	hideCloseButton();
	hideOkButton();
	setAttribute(Qt::WA_DeleteOnClose);
	Mx::setWidgetStyle(this, "qss:SYAdminShowPaperDetailWindow.qss");
	connect(ui.closedBtn, &QPushButton::clicked, this, &SYAdminShowPaperDetailWindow::on_closeBtn_clicked);

	//建立试卷类型和其数据库的映射关系
}

SYAdminShowPaperDetailWindow::~SYAdminShowPaperDetailWindow()
{
}

void SYAdminShowPaperDetailWindow::Initialize()  //从数据库中读取数据
{
	PaperInfoDetail* t_paperInfoDetail;
	SYDBMgr::Instance()->QueryPaperInfo(cur_PaperName, &t_paperInfoDetail, results);
	//设置页面试卷信息
	QString  paperName = cur_PaperName;
	ui.paperTitle_1->setText(paperName);
	ui.paperNum->setText(QString::number(results.size()));
	QString time1 = t_paperInfoDetail->examTime.split(":")[0];
	//QString time2 = t_paperInfoDetail->examTime.split(":")[1];
	int totalTime = QString(time1[0]).toInt() * 10 + QString(time1[1]).toInt();
	QString s_totalTime = QString("%1").arg(totalTime);

	ui.examTotalTime->setText(s_totalTime);
	QString score = t_paperInfoDetail->examScore;
	ui.paperTotalScore->setText(score);

	QListWidget* listWidget = new QListWidget();
	QVBoxLayout* vLayout = new QVBoxLayout;
	for (int i = 0; i< results.size(); i++)
	{
		QuestionFrame* frame = new QuestionFrame(results[i], i);
		QListWidgetItem* item = new QListWidgetItem(listWidget);
		frame->setFixedSize(1315, 0);
		int itemHeight = frame->getHeight();
		item->setSizeHint(QSize(1315, itemHeight));
		item->setBackgroundColor(QColor("#2c3345"));
		listWidget->setItemWidget(item, frame);
	}
	vLayout->addSpacerItem(new QSpacerItem(30, QSizePolicy::Fixed));
	vLayout->addWidget(listWidget);
	vLayout->addSpacerItem(new QSpacerItem(1,  QSizePolicy::Expanding));
	ui.contentWidget->setLayout(vLayout);
	listWidget->setSpacing(5);
	vLayout->setContentsMargins(30, 0, 0, 0);
	listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	//隐藏横向滚动条
	//ui.scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

}


void SYAdminShowPaperDetailWindow::on_closeBtn_clicked()
{

	done(RC_Ok);
}