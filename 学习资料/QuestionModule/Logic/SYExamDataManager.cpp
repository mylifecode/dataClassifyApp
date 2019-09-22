#include <WinSock2.h>
#include <stdlib.h>
#include <time.h>
#include <QSound>

#include <QDateTime>
#include "SYExamDataManager.h"
#include "SYDBMgr.h"

SYExamDataManager SYExamDataManager::s_Instance;

ExamPaper::ExamPaper(const QString & name, const QString& auther, int id)
{
	m_PaperName = name;
	m_Auther = auther;
	m_PaperIndex = id;
}

SYExamDataManager & SYExamDataManager::GetInstance()
{
	return s_Instance;
}

SYExamDataManager::SYExamDataManager()
{

}

void SYExamDataManager::SelectAllUserMission(int userid, std::vector<ExamMission> & missionlist)
{
	const QString& classTable = SYDBMgr::Instance()->GetTableName(SYDBMgr::DT_ExamMissionList);
	
	QString sql = QString("select id , paperid ,examtotaltime,  papername , paperassignor , paperquestionnum , rightanswernum , answertimeused , isfinished , date_format(answerdate ,'%Y/%m/%d %H:%i') as 'formatdate' from %1 where userid = %2 order by id desc").arg(classTable).arg(userid);

	QVector<QSqlRecord> result;

	bool bRet = SYDBMgr::Instance()->Query(sql, result);
	
	for (int i = 0; i < result.size(); i++)
	{
		int ID = result[i].value("id").toInt();

		int paperId = result[i].value("paperid").toInt();
	
		QString paperName = result[i].value("papername").toString();

		QString paperAuthor = result[i].value("paperassignor").toString();

		QString AnswerDate = result[i].value("formatdate").toString();
		
		int QuestionNum = result[i].value("paperquestionnum").toInt();
		
		int AccuracyNum = result[i].value("rightanswernum").toInt();
		
		int SecondsUsed = result[i].value("answertimeused").toInt();

		bool isFinished = (result[i].value("isfinished").toInt() == 1 ? true : false);

		int examTime = result[i].value("examtotaltime").toInt();


		ExamMission assignitem(ID , paperName, paperAuthor, paperId, AnswerDate, QuestionNum, AccuracyNum, SecondsUsed, isFinished, examTime);

		missionlist.push_back(assignitem);
	}
}

void SYExamDataManager::GetQuestionList(int paperid, std::vector<ExamPaperQuestion> & questList,int number)
{
	const QString& contentTable = SYDBMgr::Instance()->GetTableName(SYDBMgr::DT_ExamPaperDetail);

	const QString&  questTable = SYDBMgr::Instance()->GetTableName(SYDBMgr::DT_Question);

	QString sql;
	
	if (paperid < 0)
	{
		sql = QString("select * from %1 order by rand() limit %2").arg(questTable).arg(number);

	}
	else
	    sql = QString("select * from %1 where id in (select questionid from %2 where paperid = %3)").arg(questTable).arg(contentTable).arg(paperid);
	
	QVector<QSqlRecord> result;

	bool bRet = SYDBMgr::Instance()->Query(sql, result);

	for (int i = 0; i < result.size(); i++)
	{
		int questionid = result[i].value("id").toInt();

		QString opta = result[i].value("opta").toString();   //item content

		QString optb = result[i].value("optb").toString();
		
		QString optc = result[i].value("optc").toString();
		
		QString optd = result[i].value("optd").toString();

		QString opte = result[i].value("opte").toString();

		QString answer = result[i].value("answer").toString();   //multiply item
		
		QString title  = result[i].value("title").toString();

		//QChar   answerchar;
	//	QString strAnswer;
		
		//if (answer == QString("A"))
		//	answerchar = 'A';
		//
		//else if (answer == QString("B"))
		//	answerchar = 'B';
		//
		//else if(answer == QString("C"))
		//	answerchar = 'C';
		//
		//else if(answer == QString("D"))
		//	answerchar = 'D';

		//else
		//	answerchar = 'E';

		ExamPaperQuestion questItem(title, questionid, answer, opta, optb, optc, optd, opte);

		questList.push_back(questItem);
	}
}
ExamMission SYExamDataManager::SubmitMissionResultDetail(int userid,
	                                              int paperid, 
											      const QString & papername,
												  const QString & paperassignor,
											      int questionnum,
											      int correctanswernum,
												  int Score,
											      int answertimeused,
												  int examTotalTime,
											      std::vector<ExamPaperQuestion> & missionQuests,bool isFinished)
{
	//
	//int userId = SYDBMgr::Instance()->QueryMaxIdFromTable(SYDBMgr::DT_ExamMissionList);
	const QString& missonListTable = SYDBMgr::Instance()->GetTableName(SYDBMgr::DT_ExamMissionList);

	const QString& dataTable = SYDBMgr::Instance()->GetTableName(SYDBMgr::DT_ExamMissionDetail);

	QString sql;
	int missionID=-1;
	if (paperid < 0|| isFinished==true)  //random choose or redo need to create a new record
	{

		sql = QString("select * from %1 order by id ").arg(missonListTable);
		bool ok=SYDBMgr::Instance()->Exec(sql);

		//sql = QString("insert into %1 (userid, paperid , answerdate , papername , paperquestionnum , rightanswernum , answertimeused , isfinished , paperassignor , score, examtotaltime£© values (%2 , %3 , now() , '%4' , %5 , %6 , %7 , 1 ,'%8' , %9, %10)")
		//	.arg(missonListTable).arg(userid).arg(paperid).arg(papername).arg(questionnum).arg(correctanswernum).arg(answertimeused).arg(paperassignor).arg(Score).arg(examTotalTime);
	
		sql = QString("insert into %1 (userid,paperid,answerdate,papername,paperquestionnum,rightanswernum,answertimeused,isfinished,paperassignor,score,examtotaltime)  \
					  					  					  			values( %2, %3 ,now() ,'%4',%5 ,%6 ,%7,1,'%8',%9 ,%10)")
																		.arg(missonListTable)
																		.arg(userid)
																		.arg(paperid)
																		.arg(papername)
																		.arg(questionnum)
																		.arg(correctanswernum)
																		.arg(answertimeused)
																		.arg(paperassignor)
																		.arg(Score)
																		.arg(examTotalTime);

		bool ret=SYDBMgr::Instance()->Exec(sql);
		
		missionID = SYDBMgr::Instance()->QueryMaxIdFromTable(SYDBMgr::DT_ExamMissionList);
	}
	else if (isFinished==0)  //update records in mission list 
	{
		sql = QString("update %1 set answerdate=now() , rightanswernum=%2 , answertimeused=%3 , isfinished=1 , score=%4 where userid=%5 and paperid=%6")
			.arg(missonListTable).arg(correctanswernum).arg(answertimeused).arg(Score).arg(userid).arg(paperid);
		bool ret=SYDBMgr::Instance()->Exec(sql);
		
		missionID = SYDBMgr::Instance()->QueryStuMissionId(userid, paperid);
		
	}

	
	for (int c = 0; c < missionQuests.size(); c++)
	{
		ExamPaperQuestion & quest = missionQuests[c];

		QString sql = QString("insert into %1 (missionid , questionid , useranswer) values(%2 , %3 , '%4')").arg(dataTable).arg(missionID).arg(quest.m_QuestID).arg(quest.m_UserAnswer);
		
		bool bRet = SYDBMgr::Instance()->Exec(sql);
	}

	QDateTime current_time = QDateTime::currentDateTime();
	QString   strDate = current_time.toString("yyyy-MM-dd hh:mm");
	
	ExamMission missionInst(missionID, papername, paperassignor, paperid, strDate, questionnum, correctanswernum, answertimeused, true);

	return missionInst;
}

int SYExamDataManager::GetMissionPaperScore(int paperid)
{
	
	int score = SYDBMgr::Instance()->GetPaperScore(paperid);
	
	return score;
	
}


void SYExamDataManager::GetExamMissionResultDetail(int missionid, std::vector<ExamPaperQuestion> & questList)
{
	const QString&  questTable = SYDBMgr::Instance()->GetTableName(SYDBMgr::DT_Question);

	const QString& missionDetailTable = SYDBMgr::Instance()->GetTableName(SYDBMgr::DT_ExamMissionDetail);

	
	QVector<QSqlRecord> result;

	QString sql1 = QString("select * from %1 where missionid = %2 order by id").arg(missionDetailTable).arg(missionid);

	bool bRet = SYDBMgr::Instance()->Query(sql1, result);

	for (int i = 0; i < result.size(); i++)
	{
		int questionid = result[i].value("questionid").toInt();

		QString userAnswer = result[i].value("useranswer").toString();

		ExamPaperQuestion questItem;
		
		questItem.m_QuestID = questionid;
		questItem.m_UserAnswer = userAnswer;
		
		/*if (userAnswer == QString("A"))
			questItem.m_UserAnswer = 'A';

	    else if (userAnswer == QString("B"))
			questItem.m_UserAnswer = 'B';

		else if (userAnswer == QString("C"))
			questItem.m_UserAnswer = 'C';

		else if (userAnswer == QString("D"))
			questItem.m_UserAnswer = 'D';*/

		questList.push_back(questItem);
	}

	result.clear();

	QString sql0 = QString("select * from %1 where id in (select questionid from %2 where missionid = %3 order by id)").arg(questTable).arg(missionDetailTable).arg(missionid);

	 bRet = SYDBMgr::Instance()->Query(sql0, result);

	for (int i = 0; i < result.size(); i++)
	{
		int questionid = result[i].value("id").toInt();

		for (int c = 0; c < questList.size(); c++)
		{
			if (questList[c].m_QuestID == questionid)
			{
				QString opta = result[i].value("opta").toString();

				QString optb = result[i].value("optb").toString();

				QString optc = result[i].value("optc").toString();

				QString optd = result[i].value("optd").toString();
				QString opte = result[i].value("opte").toString();

				QString answer = result[i].value("answer").toString();

				QString title = result[i].value("title").toString();

				//QChar   answerchar;

				//if (answer == QString("A"))
				//	answerchar = 'A';

				//else if (answer == QString("B"))
				//	answerchar = 'B';

				//else if (answer == QString("C"))
				//	answerchar = 'C';

				//else if (answer == QString("D"))
				//	answerchar = 'D';

				//else
				//	answerchar = 'E';

				questList[c].Set(title, questionid, answer, opta, optb, optc, optd, opte);

				break;
			}
		}
	}
}


void SYExamDataManager::ImportOneQuestionToDB(const QString & questContent,
	const QString & answerA,
	const QString & answerB,
	const QString & answerC,
	const QString & answerD,
	const QString & answerE,
	const QString & correctAnswer)
{
	const QString&  questTable = SYDBMgr::Instance()->GetTableName(SYDBMgr::DT_Question);
	
	QString sql = QString("insert into %1 (title , opta , optb , optc , optd , opte , answer) values ('%2','%3','%4','%5','%6','%7','%8')")
		.arg(questTable)
		.arg(questContent)
		.arg(answerA)
		.arg(answerB)
		.arg(answerC)
		.arg(answerD)
		.arg(answerE)
		.arg(correctAnswer);

	bool bRet = SYDBMgr::Instance()->Exec(sql);

}


int SYExamDataManager::GetExamMissionPaperId(QString paperName)
{
	int paperId,num,examTime;
	SYDBMgr::Instance()->QueryPaperInfo(paperName, paperId,num,examTime);
	return paperId;
}

void SYExamDataManager::QueryMissionInfo(QString paperName,int &queNum, int &examTime)
{
	int paperId;
	SYDBMgr::Instance()->QueryPaperInfo(paperName, paperId, queNum, examTime);

}