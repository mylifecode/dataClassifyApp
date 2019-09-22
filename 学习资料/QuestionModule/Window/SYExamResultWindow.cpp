#include "MxDefine.h"
#include "SYMessageBox.h"
#include "MxGlobalConfig.h"
#include "SYUserInfo.h"
#include "SYExamResultWindow.h"
#include "SYExamRecordItem.h"
#include "SYExamDataManager.h"
#include "SYExamGlobal.h"
#include "MxQuestionModuleApp.h"
#define ITEM_INDEX Qt::UserRole + 1

SYQuestionExamResultWindow::SYQuestionExamResultWindow(QWidget *parent)
: QWidget(parent),
m_preButton(nullptr)
{
	ui.setupUi(this);

	m_userName = SYUserInfo::Instance()->GetUserName();
	m_realName = SYUserInfo::Instance()->GetRealName();

	m_permission = SYUserInfo::Instance()->GetUserPermission();

	SYExamDataManager::GetInstance().GetExamMissionResultDetail(g_currentMission.m_MissionID, m_QuestsList);

	for (int c = 0; c < m_QuestsList.size(); c++)
	{
		QListWidgetItem  * listWidgetItem = new QListWidgetItem(ui.list_examquestion);

		SYExamPaperItem * theWidgetItem = new SYExamPaperItem;

		QString temp = QString::fromLocal8Bit("第") + QString::number(c + 1) + QString::fromLocal8Bit("题");
		
		QString questType=QString::fromLocal8Bit("单选");

		int answerNumber = m_QuestsList[c].m_RightAnswer.size();
		if (answerNumber > 1)
		{
			questType = QString::fromLocal8Bit("多选");
		}

		theWidgetItem->SetContent(temp, questType, QString(m_QuestsList[c].m_UserAnswer));
	
		if (m_QuestsList[c].m_UserAnswer == 0)
		{
			theWidgetItem->SetColumn3Content(QString("?"));
			theWidgetItem->SetColumnLabelStyleSheet(3, "color:#ff0000;");
		}
		else if (m_QuestsList[c].m_UserAnswer != m_QuestsList[c].m_RightAnswer)
			theWidgetItem->SetColumnLabelStyleSheet(3, "color:#ff0000;");
		
		else
			theWidgetItem->SetColumnLabelStyleSheet(3, "color:#00ff00;");

		//Making sure that the listWidgetItem has the same size as the TheWidgetItem
		listWidgetItem->setSizeHint(QSize(370, 70));
		listWidgetItem->setData(ITEM_INDEX, c);

		//Finally adding the itemWidget to the list
		ui.list_examquestion->setItemWidget(listWidgetItem, theWidgetItem);
	}
	
	//
	if (m_realName.size())
		ui.headBtn->setText(m_realName);
	else
		ui.headBtn->setText(m_userName);

	QIcon headIcon = SYUserInfo::Instance()->GetHeadIcon();

	ui.headBtn->setIcon(headIcon);

	QString paperName = g_currentMission.m_PaperName;
	int questNum = g_currentMission.m_QuestionNum;
	int useminutes = g_currentMission.m_SecondsUsed / 60;
	if (useminutes < 1)
		useminutes = 1;
	int userUsedTime = useminutes;
	int userScore = (float)g_currentMission.m_AccuracyNum * 100.0f / (float)g_currentMission.m_QuestionNum;
	int totalScore = SYExamDataManager::GetInstance().GetMissionPaperScore(g_currentMission.m_PaperID);
	int rightNum = g_currentMission.m_AccuracyNum;
	int errorNum = g_currentMission.m_QuestionNum - g_currentMission.m_AccuracyNum;

	setMissionInfo(paperName, questNum, userUsedTime, totalScore, rightNum, errorNum, userScore);

	Mx::setWidgetStyle(this, "qss:SYQuestionExamResultWindow.qss");

	connect(ui.list_examquestion, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onClickeQuestionItem(QListWidgetItem*)));

	m_QuestIndex = 0;

	if (m_QuestsList.size() > 0)
	    refreshQuestionUI();
}


SYQuestionExamResultWindow::~SYQuestionExamResultWindow(void)
{
	m_timer.stop();
}



void SYQuestionExamResultWindow::setMissionInfo(QString paperName, int questNum, int userUsedTime, int examTotalScore, int userRightItemNum, int userErrorItemNum, int userScore)
{
	ui.lb_papername->setText(paperName);

	ui.lb_numquestion->setText(QString::number(questNum));

	ui.lb_timeused->setText(QString::number(userUsedTime));


	ui.lb_score->setText(QString::number(userScore));

	ui.lb_total_score->setText(QString::number(examTotalScore));


	ui.lb_correctnum->setText(QString::number(userRightItemNum) + CHS("题"));
	ui.lb_wrongnum->setText(QString::number(userErrorItemNum) + CHS("题"));



}

void SYQuestionExamResultWindow::onClickeQuestionItem(QListWidgetItem *item)
{
	m_QuestIndex = item->data(ITEM_INDEX).toInt();

	if (m_QuestIndex >= 0 && m_QuestIndex < m_QuestsList.size())
	{
		refreshQuestionUI();
		
	}
}

void SYQuestionExamResultWindow::on_knowledgeLibBtn_clicked()
{
	static_cast<MxQuestionModuleApp*>(qApp)->exitModuleAndShowParentWindow(WT_KnowLibWindow);

}

void SYQuestionExamResultWindow::on_dataCenterBtn_clicked()
{
	static_cast<MxQuestionModuleApp*>(qApp)->exitModuleAndShowParentWindow(WT_DataCenterWindow);
}

void SYQuestionExamResultWindow::on_answerBtn_clicked()
{

}

void SYQuestionExamResultWindow::on_personCenterBtn_clicked()
{
	static_cast<MxQuestionModuleApp*>(qApp)->exitModuleAndShowParentWindow(WT_PersonCenterWindow);
}


void SYQuestionExamResultWindow::on_bt_next_clicked()
{
	if (m_QuestIndex >= 0 && m_QuestIndex < m_QuestsList.size()-1)
		m_QuestIndex++;

	refreshQuestionUI();
}

void SYQuestionExamResultWindow::on_bt_prev_clicked()
{
	if (m_QuestIndex >= 1)
		m_QuestIndex--;

	refreshQuestionUI();
}

void SYQuestionExamResultWindow::refreshQuestionUI()
{
	ExamPaperQuestion & quesst = m_QuestsList[m_QuestIndex];

	ui.frame_answerC->hide();
	ui.frame_answerD->hide();
	ui.frame_answerE->hide();

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

	if (quesst.m_RightAnswer.size() > 1)
	{
		ui.questionType->setText(QString::fromLocal8Bit("多选题"));
	}
	else
	{
		ui.questionType->setText(QString::fromLocal8Bit("单选题"));
	}

	QString tmp = QString::number(m_QuestIndex + 1) + QString(".") + quesst.m_title;

	ui.lb_title->setText(tmp);
	ui.lb_answera->setText(quesst.m_A);
	ui.lb_answerb->setText(quesst.m_B);
	ui.lb_answerc->setText(quesst.m_C);
	ui.lb_answerd->setText(quesst.m_D);
	ui.lb_answere->setText(quesst.m_E);

	QString strBackPixmap = MxGlobalConfig::Instance()->GetSkinDir() + "/syquestion/answer_question_bg.png";

	ui.frame_answerA->setStyleSheet(QString("background-color:#ffffff;"));
	ui.frame_answerB->setStyleSheet(QString("background-color:#ffffff;"));
	ui.frame_answerC->setStyleSheet(QString("background-color:#ffffff;"));
	ui.frame_answerD->setStyleSheet(QString("background-color:#ffffff;"));
	ui.frame_answerE->setStyleSheet(QString("background-color:#ffffff;"));

	QFrame * rightFrame = 0;
	if (quesst.m_RightAnswer.contains("A"))
	{
		rightFrame = ui.frame_answerA;
		if (rightFrame)
			rightFrame->setStyleSheet(QString("background-color:#00ff00;"));
	}	
	else if (quesst.m_RightAnswer.contains("B"))
	{
		rightFrame = ui.frame_answerB;
		if (rightFrame)
			rightFrame->setStyleSheet(QString("background-color:#00ff00;"));
	}
	else if (quesst.m_RightAnswer.contains("C"))
	{
		rightFrame = ui.frame_answerC;
		if (rightFrame)
			rightFrame->setStyleSheet(QString("background-color:#00ff00;"));
	}	
	else if (quesst.m_RightAnswer.contains("D"))
	{

		rightFrame = ui.frame_answerD;
		if (rightFrame)
			rightFrame->setStyleSheet(QString("background-color:#00ff00;"));
	}
		
	else if (quesst.m_RightAnswer.contains("E"))
	{

		rightFrame = ui.frame_answerE;
		if (rightFrame)
			rightFrame->setStyleSheet(QString("background-color:#00ff00;"));
	}

	QFrame * errorFrame = 0;
	for (auto &slAnswer : quesst.m_UserAnswer)
	{
		if (quesst.m_RightAnswer.contains(slAnswer))  //useranswer contains rightanswer
			continue;

		if (slAnswer == "A")
		{
			errorFrame = ui.frame_answerA;
			if (errorFrame)
				errorFrame->setStyleSheet(QString("background-color:#ff0000;"));
			
		}
		
		if (slAnswer == "B")
		{
			errorFrame = ui.frame_answerB;
			if (errorFrame)
				errorFrame->setStyleSheet(QString("background-color:#ff0000;"));
			
		}

		if (slAnswer == "C")
		{
			
			errorFrame = ui.frame_answerC;
			if (errorFrame)
				errorFrame->setStyleSheet(QString("background-color:#ff0000;"));
		
		}
		if (slAnswer == "D")
		{
			
			errorFrame = ui.frame_answerD;
			if (errorFrame)
				errorFrame->setStyleSheet(QString("background-color:#ff0000;"));
			
		}
		if (slAnswer == "E")
		{
			
			errorFrame = ui.frame_answerE;
			if (errorFrame)
				errorFrame->setStyleSheet(QString("background-color:#ff0000;"));
			
		}
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
