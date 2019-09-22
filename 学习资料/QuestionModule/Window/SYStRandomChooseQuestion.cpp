#include"SYStRandomChooseQuestion.h"
#include"SYDBMgr.h"
#include"MxDefine.h"

SYStRandomChooseQuestion::SYStRandomChooseQuestion(QWidget *parent) :
RbShieldLayer(parent)
  
{
    ui.setupUi(this);

	hideCloseButton();
	hideOkButton();


	Initialize();

	Mx::setWidgetStyle(this, QString("qss:SYStRandomChooseQuestion.qss"));
}

void SYStRandomChooseQuestion::Initialize()
{


	connect(ui.confirmBtn, &QPushButton::clicked, this, [=]
	{
		int questionNum = ui.questionNum->text().toInt();
		int examTime = ui.examMinutes->text().toInt();
		emit setPaperInfo(questionNum, examTime);
		done(RC_Ok);
	});

	connect(ui.cancelBtn, &QPushButton::clicked, this, [=]
	{
		done(RC_Cancel);
	}
	);

	connect(ui.numDesc, &QPushButton::clicked, this, [=]
	{
		int number = ui.questionNum->text().toInt();
		if (number > 1)
		{
			number--;
		}
		else
		{
			ui.numDesc->setStyleSheet("border-image:url()");
		}
		ui.questionNum->setText(QString("%1").arg(number));

	});

	int maxmum = 0;
	SYDBMgr::Instance()->QueryQuestionsNumber(maxmum);

	connect(ui.numAscend, &QPushButton::clicked, this, [=]
	{
		int number = ui.questionNum->text().toInt();
		if (number < maxmum)
		{
			number++;
		}
		else
		{
			ui.numAscend->setStyleSheet("border-image:url(icons:/)");
		}
		ui.questionNum->setText(QString("%1").arg(number));

	});





	connect(ui.timeDesc, &QPushButton::clicked, this, [=]
	{
		int time = ui.examMinutes->text().toInt();
		if (time > 1)
		{
			time--;
		}
		else
		{
			//ui.timeDesc->setStyleSheet("border-image:url(icons:/)");
		}
		ui.examMinutes->setText(QString("%1").arg(time));

	});

	int maxmumtime = 60;
	connect(ui.timeAscend, &QPushButton::clicked, this, [=]
	{
		int time = ui.examMinutes->text().toInt();
		if (time < maxmumtime)
		{
			time++;
		}
		else
		{
			//ui.timeAscend->setStyleSheet("border-image:url()");
		}
		ui.examMinutes->setText(QString("%1").arg(time));

	});

}


SYStRandomChooseQuestion::~SYStRandomChooseQuestion()
{

}
