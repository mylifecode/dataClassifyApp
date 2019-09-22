#include "SYAdminPaperRandomChooseWindow.h"
#include "ui_SYAdminPaperRandomChooseWindow.h"
#include "SYMainWindow.h"
#include<QSqlRecord>
#include"SYMessageBox.h"
#include "MxDefine.h"
#include <SYDBMgr.h>
#include"SYUserInfo.h"
#include<qpushbutton.h>
#include <algorithm>
#include<initializer_list>
#include<time.h>
#include<math.h>
#include "SYStringTable.h"

#define CHS(txt) QString::fromLocal8Bit(txt)

SYAdminPaperRandomChooseWindow::SYAdminPaperRandomChooseWindow(QVector<QVector<QSqlRecord>> all_records, QWidget *parent) :
RbShieldLayer(parent), m_curRecords(all_records)
{
    ui.setupUi(this);

	hideOkButton();
	hideCloseButton();
	setAttribute(Qt::WA_DeleteOnClose);

	Mx::setWidgetStyle(this, QString("qss:SYAdminPaperRandomChooseWindow.qss"));
	
	Initialize();
}

SYAdminPaperRandomChooseWindow::~SYAdminPaperRandomChooseWindow()
{
  
}

void SYAdminPaperRandomChooseWindow::Initialize()
{
	connect(ui.cancelBtn, &QPushButton::clicked, this, &SYAdminPaperRandomChooseWindow::on_Btn_click);
	connect(ui.confirmBtn, &QPushButton::clicked, this, &SYAdminPaperRandomChooseWindow::on_Btn_click);

	connect(ui.paperName, &QLineEdit::textChanged, this, [=](QString text)
	{
		ui.curPaperNum->setText(QString::number(text.size()));
	
	});


	connect(ui.answerTime_lineEdit_1, &QLineEdit::textEdited, this, [=](QString text)
	{

		if (text == "0" || text == "00")
			ui.answerTime_lineEdit_1->clear();


	});
	connect(ui.paperScoreLineEdit, &QLineEdit::textEdited, this, [=](QString text)
	{

		if (text == "0" || text == "00" || text == "000")
			ui.paperScoreLineEdit->clear();
		
	});

	ui.answerTime_lineEdit_1->setValidator(new QIntValidator(1, 60, this));
	ui.paperScoreLineEdit->setValidator(new QIntValidator(1, 100,this));

	ui.questionNum->setValidator(new QIntValidator(1, 10000,this));


}

void SYAdminPaperRandomChooseWindow::on_Btn_click()
{
	QPushButton* Btn = (QPushButton*)sender();

	if (Btn == ui.confirmBtn)
	{
		
		QString paperName = ui.paperName->text();
		if (paperName.isEmpty())
		{
			QString paperNameInputTips = SYStringTable::GetString(SYStringTable::STR_InputPaperName);
			SYMessageBox* messageBox = new SYMessageBox(this, "", paperNameInputTips, 1);
			messageBox->setPicture("Shared/personnel_pic_.png");
			messageBox->showFullScreen();
			messageBox->exec();
			return;
		}
		//输入时间验证
		QString examTotalTime;
		QString txt = ui.answerTime_lineEdit_1->text();
		if (txt.isEmpty())
		{
			SYMessageBox* messageBox = new SYMessageBox(this, "", CHS("考试答题时间不能为空!"), 1);
			messageBox->setPicture("Shared/personnel_pic_.png");
			messageBox->showFullScreen();
			messageBox->exec();
			return;
		}


		//if (txt.size() == 2)
		//{
		//	QRegExp rx("[1-9][0-9]");   //带改
		//	QRegExpValidator v(rx, 0);
		//	int pos = 0;
		//	QValidator::State res;
		//	res = v.validate(txt, pos);
		//	if (QRegExpValidator::Acceptable != res)
		//	{
		//		//QString errorTips = SYStringTable::GetString(SYStringTable::STR_InputDataFomatError);
		//		SYMessageBox* messageBox = new SYMessageBox(this, "", CHS("考试时间设置不正确"), 1);
		//		messageBox->setPicture("Shared/personnel_pic_.png");
		//		messageBox->showFullScreen();
		//		messageBox->exec();
		//		return;
		//	}

		//	int paperTime = txt.toInt();
			/*if (paperTime > 60)
			{
				SYMessageBox* messageBox = new SYMessageBox(this, "", CHS("考试时间设置超过60，默认为60分钟"), 1);
				messageBox->setPicture("Shared/personnel_pic_.png");
				messageBox->showFullScreen();
				messageBox->exec();
				examTotalTime = QString("60") + QString(":00");
			}
			else*/
			
		examTotalTime = ui.answerTime_lineEdit_1->text() + QString(":00");
			
	
		QString paperTotalScore = ui.paperScoreLineEdit->text();

		if (paperTotalScore.isEmpty())
		{
			//ui.paperScoreLineEdit->setStyleSheet(QString("min-height:70px;max-height:70px;font-size:16px;color:#b7c5d8;background-color:#1b1f29;box-shadow:inset 0px 1px 4px 0px #11141a;border-radius:4px;border:solid 1px rgb(255,0,0);"));
			SYMessageBox* messageBox = new SYMessageBox(this, "", CHS("考试总分数不能设置为空!"));
			messageBox->showFullScreen();
			messageBox->exec();
			return;
		}
		//if (p   aperTotalScore.size() == 2)
		//{
		//	QRegExp rx("[1-9][0-9]");   //带改
		//	QRegExpValidator v(rx, 0);
		//	int pos = 0;
		//	QValidator::State res;
		//	res = v.validate(paperTotalScore, pos);
		//	if (QRegExpValidator::Acceptable != res)
		//	{
		//		//QString errorTips = SYStringTable::GetString(SYStringTable::STR_InputDataFomatError);
		//		SYMessageBox* messageBox = new SYMessageBox(this, "", CHS("考试总分数设置不正确"), 1);
		//		messageBox->setPicture("Shared/personnel_pic_.png");
		//		messageBox->showFullScreen();
		//		messageBox->exec();
		//		return;
		//	}
		//}

		//if (paperTotalScore.size() == 3)
		//{
		//	QRegExp rx("[1-9][0-9][0-9]");   //带改
		//	QRegExpValidator v(rx, 0);
		//	int pos = 0;
		//	QValidator::State res;
		//	res = v.validate(paperTotalScore, pos);
		//	if (QRegExpValidator::Acceptable != res)
		//	{
		//		//QString errorTips = SYStringTable::GetString(SYStringTable::STR_InputDataFomatError);
		//		SYMessageBox* messageBox = new SYMessageBox(this, "", CHS("考试总分数设置不正确"), 1);
		//		messageBox->setPicture("Shared/personnel_pic_.png");
		//		messageBox->showFullScreen();
		//		messageBox->exec();
		//		return;
		//	}
		//}

		int questionNum = ui.questionNum->text().toInt();
		//计算各类题型所占比例
		int questionTotalNum=0;
		QVector<QSqlRecord> allRecords;
		std::vector<int> clssNumVec;  //记录各类题目总数量

		srand(unsigned(time(NULL)));  //随机种子

		for (int i = 0; i < m_curRecords.size(); i++)
		{
			for (int j = 0; j < m_curRecords[i].size(); j++)
			{
				questionTotalNum++;
				allRecords.push_back(m_curRecords[i][j]);
			}
			clssNumVec.push_back(m_curRecords[i].size());
		
		}
	
		bool isChosedAllQue = false;
		if (questionNum > questionTotalNum)
		{
			SYMessageBox* messageBox = new SYMessageBox(this, "", CHS("题目数量设置过多，默认为题库题目总数!"), 1);
			messageBox->setPicture("Shared/personnel_pic_.png");
			messageBox->showFullScreen();
			messageBox->exec();
			questionNum = questionTotalNum;
			isChosedAllQue = true;
		}

		QVector<QSqlRecord> randomCreatePaperRecords;
		if (!isChosedAllQue)
		{
			std::vector<int> randomClssNum;  //随机生成各类题目数量
			for (int i = 0; i < clssNumVec.size(); i++)
			{
				int Num = ceil((clssNumVec[i] * questionNum) / (questionTotalNum + 0.001));  //向上取整
				randomClssNum.push_back(Num);
			}

			int cur_questionNum = 0;
			for (int i = 0; i < m_curRecords.size(); i++)
			{
				int diffNum = clssNumVec[i] - randomClssNum[i];

				if (diffNum <= 0)
					continue;
				int index = rand() % (diffNum);
				for (int j = 0; j < randomClssNum[i]; j++)
				{
					if (cur_questionNum >= questionNum)
						break;
					QSqlRecord record = m_curRecords[i][index + j];
					randomCreatePaperRecords.push_back(record);
					cur_questionNum++;
				}
			}

		}
		else
		{
		
			randomCreatePaperRecords = allRecords;

		}

		//记录新试卷信息到Paperlist数据库中
		QString label = SYStringTable::GetString(SYStringTable::STR_MidLevel);
	
		QString questionNumStr = QString::number(questionNum);
		QString creator = SYUserInfo::s_Instance->GetRealName();
	

		std::vector<QString> paper = { paperName, label, questionNumStr, creator, examTotalTime, paperTotalScore };

		bool ret=SYDBMgr::Instance()->CreatePaperInfo( paper, randomCreatePaperRecords);  //生成一条试卷记录
		if (ret)
			done(RbShieldLayer::RC_Ok);
		else
		{
			SYMessageBox* messageBox = new SYMessageBox(this, "", CHS("该试卷已经存在!"), 1);
			messageBox->setPicture("Shared/personnel_pic_.png");
			messageBox->showFullScreen();
			messageBox->exec();
			//ui.paperName->clear();
		}
	}
	else
		done(RbShieldLayer::RC_Cancel);
}

