#include "MxDefine.h"
#include "SYMessageBox.h"
#include "MxGlobalConfig.h"
#include "SYUserInfo.h"
#include "SYExamWindow.h"
#include "SYExamRecordItem.h"
#include "SYExamDataManager.h"
#include "SYExamGlobal.h"
#include<algorithm>
#include<vector>
#define ITEM_INDEX Qt::UserRole + 1

SYQuestionDoExamWindow::SYQuestionDoExamWindow(QWidget *parent)
: QWidget(parent),
m_preButton(nullptr),
m_timer(new QTimer),
m_updateUITimer(new QTimer)
{
	ui.setupUi(this);

	m_userName = SYUserInfo::Instance()->GetUserName();
	m_realName = SYUserInfo::Instance()->GetRealName();
	m_userName = g_UserInfor.m_name;
	int m_userId = g_UserInfor.m_userid;
	m_permission = SYUserInfo::Instance()->GetUserPermission();

	Mx::setWidgetStyle(this, "qss:SYQuestionDoExamWindow.qss");

	connect(ui.list_examquestion, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onClickeQuestionItem(QListWidgetItem*)));
	//connect(0, SIGNAL(DoExamMission(int)), this , SLOT(GetExamMissionQuestion(int)));
	
	ui.frame_answerA->installEventFilter(this);
	ui.frame_answerB->installEventFilter(this);
	ui.frame_answerC->installEventFilter(this);
	ui.frame_answerD->installEventFilter(this);
	ui.frame_answerE->installEventFilter(this);

	ui.frame_answerA->setProperty("ok", false);
	ui.frame_answerB->setProperty("ok", false);
	ui.frame_answerC->setProperty("ok", false);
	ui.frame_answerD->setProperty("ok", false);
	ui.frame_answerE->setProperty("ok", false);

	m_QuestIndex = 0;

	m_ExamPaperID = -1;

	m_isMissionReDone = false;

	m_StartMilliseconds = GetTickCount();


	if (g_currentMission.m_MissionID == -1 || g_currentMission.m_IsFinished==false)   //m_MissionID=-1 in case random choose or left list item is clicked;
	{
		GetExamQuestionFromPaper(g_currentMission.m_PaperID,g_currentMission.m_QuestionNum);

		m_ExamPaperID = g_currentMission.m_PaperID;

		if (g_currentMission.m_IsFinished ==false)
		{
			
		//	ui.paperName->setText(CHS("%1").arg(paperName));
		}

	}
	else
	{
		GetExamQuestionFromExistMission(g_currentMission.m_MissionID);  //mission state isfinished=1,
		m_isMissionReDone = true;
		m_ExamPaperID = g_currentMission.m_PaperID;
	}

	setMissionInfo();

	int examTime = g_currentMission.m_ExamTime*60*1000;
	//QTimer::singleShot(examTime, this, &SYQuestionDoExamWindow::on_examIsOver);
	//connect(m_timer, SIGNAL(timeout()), this, SLOT(on_examIsOver()));
	m_timer->setSingleShot(true);
	m_timer->setInterval(examTime);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(on_examIsOver()));
	m_timer->start();

	//set 100毫秒刷新
	
	m_updateUITimer->setInterval(100);
	connect(m_updateUITimer,&QTimer::timeout, this, [=]
	{
		int curTime = m_timer->remainingTime();
		int minutes = (curTime/1000)/60;
		int seconds = (curTime / 1000) - minutes*60;
		QString strCurTime = QString("%1:%2").arg(minutes).arg(seconds);
		ui.examTime->setText(strCurTime);
		//m_updateUITimer->start();
	});

	m_updateUITimer->start();




}


SYQuestionDoExamWindow::~SYQuestionDoExamWindow(void)
{
	m_timer->stop();
}


void SYQuestionDoExamWindow::on_examIsOver()
{
	SYMessageBox* messbox = new SYMessageBox(this, CHS(""), CHS("考试时间结束，是否提交考试成绩?"),2);
	messbox->showFullScreen();

	if (messbox->exec() == 2)
	{
		on_bt_finish_clicked();
	}
	
}

void SYQuestionDoExamWindow::setMissionInfo()
{
	QString paperName;
	int questNumber, examTime,totalScore,paperId;
	paperName = g_currentMission.m_PaperName;
	questNumber = g_currentMission.m_QuestionNum;
	examTime = g_currentMission.m_ExamTime;
	paperId = g_currentMission.m_PaperID;

	//随机抽题，每道题默认5分
	if (paperName.size() == 0)
	{
		paperName = QString::fromLocal8Bit("随机抽题");
	}

	if (paperName == QString::fromLocal8Bit("随机抽题"))
	{
		totalScore = questNumber * 5;
	}
	else
	{
		totalScore=SYExamDataManager::GetInstance().GetMissionPaperScore(paperId);
	}

	ui.paperName->setText(paperName);
	ui.questNum->setText(QString::number(questNumber));
	ui.examTimeLb->setText(QString::number(examTime));
	ui.examTotalScore->setText(QString::number(totalScore));


}

void SYQuestionDoExamWindow::GetExamQuestionFromExistMission(int missionID)
{
	m_QuestsList.clear();

	
	SYExamDataManager::GetInstance().GetExamMissionResultDetail(missionID , m_QuestsList);

	for (int c = 0; c < m_QuestsList.size(); c++)
	{
		m_QuestsList[c].SetNotAnswered();

		QListWidgetItem  * listWidgetItem = new QListWidgetItem(ui.list_examquestion);

		SYExamPaperItem * theWidgetItem = new SYExamPaperItem;

		QString temp = QString::fromLocal8Bit("第") + QString::number(c + 1) + QString::fromLocal8Bit("题");

		QString questType = QString::fromLocal8Bit("单选");

		int answerNum = m_QuestsList[c].m_RightAnswer.size();
		if (answerNum > 1)
		{
			questType = QString::fromLocal8Bit("多选");
		}

		theWidgetItem->SetContent(temp, questType, QString(m_QuestsList[c].m_UserAnswer));

		//Making sure that the listWidgetItem has the same size as the TheWidgetItem
		listWidgetItem->setSizeHint(QSize(370, 70));
		listWidgetItem->setData(ITEM_INDEX, c);

		//Finally adding the itemWidget to the list
		ui.list_examquestion->setItemWidget(listWidgetItem, theWidgetItem);
	}
	if (m_QuestsList.size() > 0)
		refreshQuestionUI();
}

void SYQuestionDoExamWindow::GetExamQuestionFromPaper(int paperid,int number)
{
	m_QuestsList.clear();


	SYExamDataManager::GetInstance().GetQuestionList(paperid, m_QuestsList,number);

	for (int c = 0; c < m_QuestsList.size(); c++)
	{
		QListWidgetItem  * listWidgetItem = new QListWidgetItem(ui.list_examquestion);

		SYExamPaperItem * theWidgetItem = new SYExamPaperItem;

		QString temp = QString::fromLocal8Bit("第") + QString::number(c + 1) + QString::fromLocal8Bit("题");

		QString questType = QString::fromLocal8Bit("单选");

		int answerNum = m_QuestsList[c].m_RightAnswer.size();
		if (answerNum > 1)
		{
			questType = QString::fromLocal8Bit("多选");
		}
		theWidgetItem->SetContent(temp, questType, QString(m_QuestsList[c].m_UserAnswer));

		//Making sure that the listWidgetItem has the same size as the TheWidgetItem
		listWidgetItem->setSizeHint(QSize(370, 70));
		listWidgetItem->setData(ITEM_INDEX, c);

		//Finally adding the itemWidget to the list
		ui.list_examquestion->setItemWidget(listWidgetItem, theWidgetItem);
	}
	if (m_QuestsList.size() > 0)
		refreshQuestionUI();

}
void SYQuestionDoExamWindow::onClickeQuestionItem(QListWidgetItem *item)
{
	m_QuestIndex = item->data(ITEM_INDEX).toInt();

	if (m_QuestIndex >= 0 && m_QuestIndex < m_QuestsList.size())
	{
		refreshQuestionUI();
		
		//SYExamPaperItem * widget = dynamic_cast<SYExamPaperItem*>(ui.list_examquestion->itemWidget(item));

		//if (widget && m_QuestsList[m_QuestIndex].m_UserAnswer != QChar(0))
		//{
		//	widget->SetColumn3Content(QString(m_QuestsList[m_QuestIndex].m_UserAnswer));
		//}
	}
}

void SYQuestionDoExamWindow::on_bt_finish_clicked()
{
		
		int correntNum = 0;
		for (int c = 0; c < m_QuestsList.size(); c++)
		{
			if (m_QuestsList[c].m_RightAnswer == m_QuestsList[c].m_UserAnswer)
			{
				correntNum++;
			}
		}

		int currMilliseconds = GetTickCount();

		int secondsUsed = (currMilliseconds - m_StartMilliseconds) / 1000;

		float percentGood = (float)correntNum / (float)m_QuestsList.size();

		int   score = percentGood * 100;

		int examTime = g_currentMission.m_ExamTime;

		if (m_ExamPaperID < 0)  //random need insert to new mission
		{
			g_currentMission = SYExamDataManager::GetInstance().SubmitMissionResultDetail(g_UserInfor.m_userid,
				-1,
				QString::fromLocal8Bit("随机试卷"),
				QString::fromLocal8Bit("随机"),
				m_QuestsList.size(),
				correntNum,
				score,
				examTime,
				secondsUsed,
				m_QuestsList,m_isMissionReDone);
		}
		else  //1.left list item is clicked need to update missionlist  2.right list item is clicked to redo test
		{
			int paperId = g_currentMission.m_PaperID;
			QString paperName = g_currentMission.m_PaperName;
			QString assignor = g_currentMission.m_Auther;

			g_currentMission = SYExamDataManager::GetInstance().SubmitMissionResultDetail(g_UserInfor.m_userid,
					paperId,
					paperName,
					assignor,
					m_QuestsList.size(),
					correntNum,
					score,
					examTime,
					secondsUsed,
					m_QuestsList,m_isMissionReDone);

		}

		SYMessageBox* messageBox = new  SYMessageBox(this, CHS(""), CHS("提交成功！是否查看结果？"), 2);
		
		messageBox->showFullScreen();
		//messageBox->setPicture("")
		
		if(messageBox->exec() == 2)
		{
			emit ReplaceCurrentWindow(WT_ExamResultWindow);
		}
		else
		{
			emit ExitCurrentWindow();
		}	
	

}

void SYQuestionDoExamWindow::on_bt_next_clicked()
{
	if (m_QuestIndex >= 0 && m_QuestIndex < m_QuestsList.size()-1)
		m_QuestIndex++;

	refreshQuestionUI();
}

void SYQuestionDoExamWindow::on_bt_prev_clicked()
{
	if (m_QuestIndex >= 1)
		m_QuestIndex--;

	refreshQuestionUI();
}

bool SYQuestionDoExamWindow::eventFilter(QObject *obj, QEvent *event)
{

	if (event->type() == QEvent::MouseButtonRelease)//mouse button pressed
	{
		int rightAnswerNum = 1;
		if (m_QuestIndex > 0)
		{
			QString strAnswer = m_QuestsList[m_QuestIndex].m_RightAnswer;
			rightAnswerNum = strAnswer.size();
			if (rightAnswerNum == 0)
				rightAnswerNum = 1;
		}

		if (m_QuestIndex >= 0)
		{
			QString strBackPixmap = MxGlobalConfig::Instance()->GetSkinDir() + "/syquestion/answer_question_bg.png";

			QFrame * clikedFrame = 0;
			bool isChosed;
			if (obj == ui.frame_answerA)//指定某个QLabel
			{
				clikedFrame = ui.frame_answerA;
				isChosed = clikedFrame->property("ok").toBool();
				if (rightAnswerNum > 1)
				{
					if (isChosed)
					{
						clikedFrame->setProperty("ok", false);
						QString answer = m_QuestsList[m_QuestIndex].m_UserAnswer;
						m_QuestsList[m_QuestIndex].m_UserAnswer = answer.remove("A");
					}
					else
					{
						clikedFrame->setProperty("ok", true);
						m_QuestsList[m_QuestIndex].m_UserAnswer += "A";
					}

				}
				else
				{
					if (isChosed)
					{
						clikedFrame->setProperty("ok", false);
						m_QuestsList[m_QuestIndex].m_UserAnswer = "";
					}
					else
					{
						clikedFrame->setProperty("ok", true);
						m_QuestsList[m_QuestIndex].m_UserAnswer = "A";
						ui.frame_answerB->setProperty("ok", false);
						ui.frame_answerC->setProperty("ok", false);
						ui.frame_answerD->setProperty("ok", false);
						ui.frame_answerE->setProperty("ok", false);
					}

				}
			}
			else if (obj == ui.frame_answerB)//指定某个QLabel
			{
				clikedFrame = ui.frame_answerB;
				isChosed = clikedFrame->property("ok").toBool();
				if (rightAnswerNum > 1)
				{
					if (isChosed)
					{
						clikedFrame->setProperty("ok", false);
						QString answer = m_QuestsList[m_QuestIndex].m_UserAnswer;
						m_QuestsList[m_QuestIndex].m_UserAnswer = answer.remove("B");
					}
					else
					{
						clikedFrame->setProperty("ok", true);
						m_QuestsList[m_QuestIndex].m_UserAnswer += "B";
					}

				}
				else
				{
					if (isChosed)
					{
						clikedFrame->setProperty("ok", false);
						m_QuestsList[m_QuestIndex].m_UserAnswer = "";
					}
					else
					{
						clikedFrame->setProperty("ok", true);
						m_QuestsList[m_QuestIndex].m_UserAnswer = "B";
						ui.frame_answerA->setProperty("ok", false);
						ui.frame_answerC->setProperty("ok", false);
						ui.frame_answerD->setProperty("ok", false);
						ui.frame_answerE->setProperty("ok", false);
					}

				}
			}
			else if (obj == ui.frame_answerC)//指定某个QLabel
			{
				clikedFrame = ui.frame_answerC;

				isChosed = clikedFrame->property("ok").toBool();
				if (rightAnswerNum > 1)
				{
					if (isChosed)
					{
						clikedFrame->setProperty("ok", false);
						QString answer = m_QuestsList[m_QuestIndex].m_UserAnswer;
						m_QuestsList[m_QuestIndex].m_UserAnswer = answer.remove("C");
					}
					else
					{
						clikedFrame->setProperty("ok", true);
						m_QuestsList[m_QuestIndex].m_UserAnswer += "C";
					}

				}
				else
				{
					if (isChosed)
					{
						clikedFrame->setProperty("ok", false);
						m_QuestsList[m_QuestIndex].m_UserAnswer = "";
					}
					else
					{
						clikedFrame->setProperty("ok", true);
						m_QuestsList[m_QuestIndex].m_UserAnswer = "C";
						ui.frame_answerA->setProperty("ok", false);
						ui.frame_answerB->setProperty("ok", false);
						ui.frame_answerD->setProperty("ok", false);
						ui.frame_answerE->setProperty("ok", false);
					}

				}
			}
			else if (obj == ui.frame_answerD)//指定某个QLabel
			{
				clikedFrame = ui.frame_answerD;
				isChosed = clikedFrame->property("ok").toBool();
				if (rightAnswerNum > 1)
				{
					if (isChosed)
					{
						clikedFrame->setProperty("ok", false);
						QString answer = m_QuestsList[m_QuestIndex].m_UserAnswer;
						m_QuestsList[m_QuestIndex].m_UserAnswer = answer.remove("D");
					}
					else
					{
						clikedFrame->setProperty("ok", true);
						m_QuestsList[m_QuestIndex].m_UserAnswer += "D";
					}

				}
				else
				{
					if (isChosed)
					{
						clikedFrame->setProperty("ok", false);
						m_QuestsList[m_QuestIndex].m_UserAnswer = "";
					}
					else
					{
						clikedFrame->setProperty("ok", true);
						m_QuestsList[m_QuestIndex].m_UserAnswer = "D";
						ui.frame_answerA->setProperty("ok", false);
						ui.frame_answerB->setProperty("ok", false);
						ui.frame_answerC->setProperty("ok", false);
						ui.frame_answerE->setProperty("ok", false);
					}

				}
			}
			else if (obj == ui.frame_answerE)//指定某个QLabel
			{
				clikedFrame = ui.frame_answerE;
				isChosed = clikedFrame->property("ok").toBool();
				if (rightAnswerNum > 1)
				{
					if (isChosed)
					{
						clikedFrame->setProperty("ok", false);
						QString answer = m_QuestsList[m_QuestIndex].m_UserAnswer;
						m_QuestsList[m_QuestIndex].m_UserAnswer = answer.remove("E");
					}
					else
					{
						clikedFrame->setProperty("ok", true);
						m_QuestsList[m_QuestIndex].m_UserAnswer += "E";
					}

				}
				else
				{
					if (isChosed)
					{
						clikedFrame->setProperty("ok", false);
						m_QuestsList[m_QuestIndex].m_UserAnswer = "";
					}
					else
					{
						clikedFrame->setProperty("ok", true);
						m_QuestsList[m_QuestIndex].m_UserAnswer = "E";
						ui.frame_answerA->setProperty("ok", false);
						ui.frame_answerB->setProperty("ok", false);
						ui.frame_answerC->setProperty("ok", false);
						ui.frame_answerD->setProperty("ok", false);
					}

				}

			}

			//update item order bac ->abc
			userAnswerUpdateOrder(m_QuestsList[m_QuestIndex].m_UserAnswer);

			if (clikedFrame)
			{
				ui.frame_answerA->setStyleSheet(QString("background-color:#ffffff;"));
				ui.frame_answerB->setStyleSheet(QString("background-color:#ffffff;"));
				ui.frame_answerC->setStyleSheet(QString("background-color:#ffffff;"));
				ui.frame_answerD->setStyleSheet(QString("background-color:#ffffff;"));
				ui.frame_answerE->setStyleSheet(QString("background-color:#ffffff;"));

				//clikedFrame->
				QString userAnswer = m_QuestsList[m_QuestIndex].m_UserAnswer;
				if (userAnswer.contains("A"))
				{
					ui.frame_answerA->setStyleSheet(QString("border-image:url(%1); background-color:none;").arg(strBackPixmap));
				}
				if (userAnswer.contains("B"))
				{
					ui.frame_answerB->setStyleSheet(QString("border-image:url(%1); background-color:none;").arg(strBackPixmap));
				}
				if (userAnswer.contains("C"))
				{
					ui.frame_answerC->setStyleSheet(QString("border-image:url(%1); background-color:none;").arg(strBackPixmap));
				}
				if (userAnswer.contains("D"))
				{
					ui.frame_answerD->setStyleSheet(QString("border-image:url(%1); background-color:none;").arg(strBackPixmap));
				}
				if (userAnswer.contains("E"))
				{
					ui.frame_answerE->setStyleSheet(QString("border-image:url(%1); background-color:none;").arg(strBackPixmap));
				}
				QListWidgetItem * item = ui.list_examquestion->item(m_QuestIndex);

				SYExamPaperItem * widget = dynamic_cast<SYExamPaperItem*>(ui.list_examquestion->itemWidget(item));

				if (widget)
				{
					widget->SetColumn3Content(userAnswer);
				}
			}

		}
	}
	return QWidget::eventFilter(obj, event);
}


void SYQuestionDoExamWindow::userAnswerUpdateOrder(QString &answer)
{
	QVector<QChar> strVec;
	std::vector<int> strAsscVec;
	for (auto & s : answer)
	{
		strVec.push_back(s);
		if (s == "A")
		{
			strAsscVec.push_back(1);
		}
		else if (s == "B")
		{
			strAsscVec.push_back(2);
		}
		else if (s == "C")
		{
			strAsscVec.push_back(3);
		}
		else if (s == "D")
		{
			strAsscVec.push_back(4);
		}	
		else if (s =="E")
		{
			strAsscVec.push_back(5);
		}
	}
	
	std::sort(strAsscVec.begin(),strAsscVec.end());

	answer = "";
	for (auto str:strAsscVec)
	{
		if (str == 1)
		{
			answer += "A";
		}
		if (str == 2)
		{
			answer += "B";
		}
		if (str == 3)
		{
			answer += "C";
		}
		if (str == 4)
		{
			answer += "D";
		}
		if (str == 5)
		{
			answer += "E";
		}
		
	}
	
}


void SYQuestionDoExamWindow::refreshQuestionUI()
{
	
	//A,B must choose
	ui.frame_answerC->hide();
	ui.frame_answerD->hide();
	ui.frame_answerE->hide();

	ExamPaperQuestion & quesst = m_QuestsList[m_QuestIndex];

	QString tmp = QString::number(m_QuestIndex + 1) + QString(".");

	ui.lb_questindex->setText(tmp);
	ui.lb_title->setText(quesst.m_title);
	ui.lb_answera->setText(quesst.m_A);
	ui.lb_answerb->setText(quesst.m_B);
	if (quesst.m_C.size() > 0)
	{
		ui.lb_answerc->setText(quesst.m_C);
		ui.frame_answerC->show();
	}
	if (quesst.m_D.size() > 0)
	{
		ui.lb_answerd->setText(quesst.m_D);
		ui.frame_answerD->show();

	}
	if (quesst.m_E.size() > 0)
	{
		ui.lb_answere->setText(quesst.m_E);
		ui.frame_answerE->show();
	}
	
	
	QString questType = QString::fromLocal8Bit("单选");

	int answerNum = quesst.m_RightAnswer.size();
	if (answerNum > 1)
	{
		questType = QString::fromLocal8Bit("多选");
	}
	ui.questType->setText(questType);

	QString strBackPixmap = MxGlobalConfig::Instance()->GetSkinDir() + "/syquestion/answer_question_bg.png";

	ui.frame_answerA->setStyleSheet(QString("background-color:#ffffff;"));
	ui.frame_answerB->setStyleSheet(QString("background-color:#ffffff;"));
	ui.frame_answerC->setStyleSheet(QString("background-color:#ffffff;"));
	ui.frame_answerD->setStyleSheet(QString("background-color:#ffffff;"));
	ui.frame_answerE->setStyleSheet(QString("background-color:#ffffff;"));

	if (quesst.m_UserAnswer.contains("A"))
	{
		ui.frame_answerA->setStyleSheet(QString("border-image:url(%1); background-color:none;").arg(strBackPixmap));
	}
	else if (quesst.m_UserAnswer.contains("B"))
	{
		ui.frame_answerB->setStyleSheet(QString("border-image:url(%1); background-color:none;").arg(strBackPixmap));
	}
	else if(quesst.m_UserAnswer.contains("C"))
	{
		ui.frame_answerC->setStyleSheet(QString("border-image:url(%1); background-color:none;").arg(strBackPixmap));
	}
	else if(quesst.m_UserAnswer.contains("D"))
	{
		ui.frame_answerD->setStyleSheet(QString("border-image:url(%1); background-color:none;").arg(strBackPixmap));
	}
	else if (quesst.m_UserAnswer.contains("E"))
	{

		ui.frame_answerE->setStyleSheet(QString("border-image:url(%1); background-color:none;").arg(strBackPixmap));
	}
	QString temp = QString::number(m_QuestIndex + 1) + QString("/") + QString::number(m_QuestsList.size());
	ui.lb_currpage->setText(temp);
	
	if (m_QuestIndex == 0)
		ui.bt_prev->setEnabled(false);
	else
		ui.bt_prev->setEnabled(true);

	if (m_QuestIndex == m_QuestsList.size()-1)
		ui.bt_next->setEnabled(false);
	else
		ui.bt_next->setEnabled(true);


}

void SYQuestionDoExamWindow::showEvent(QShowEvent* event)
{
	
}

void SYQuestionDoExamWindow::mousePressEvent(QMouseEvent* mouseEvent)
{
	
}