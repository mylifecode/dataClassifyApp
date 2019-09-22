#pragma once
#include <QString>
class ExamPaper
{
public:
	ExamPaper(const QString & name, const QString& auther, int id);
	QString m_PaperName;
	QString m_Auther;
	int     m_PaperIndex;
};



class ExamMission : public ExamPaper
{
public:
	ExamMission() :ExamPaper("", "" , -1)
	{
		m_QuestionNum = 0;
		m_AccuracyNum = 0;
		m_SecondsUsed = 0;
		m_PaperID = 0;
		m_MissionID = -1;
		m_ExamTime = -1;
	}
	ExamMission(int id, const QString & name, const QString& auther, int paperid, QString & date, int QuestNum, int AccuracyNum, int Seconds, bool IsFinished, int examTime=-1) : ExamPaper(name, auther, id)
	{
		m_AnswerData = date;
		m_QuestionNum = QuestNum;
		m_AccuracyNum = AccuracyNum;
		m_SecondsUsed = Seconds;
		m_IsFinished = IsFinished;
		m_PaperID = paperid;
		m_MissionID = id;
		m_ExamTime = examTime;
	}

	QString m_AnswerData;

	int  m_QuestionNum;

	int  m_AccuracyNum;

	int  m_SecondsUsed;

	int  m_PaperID;

	int  m_MissionID;
	bool m_IsFinished;

	int m_ExamTime;

};


class ExamPaperQuestion
{
public:
	ExamPaperQuestion(const QString & title, int id, QString answer,const QString A="", const QString B="", const QString C="", const QString D="", const QString E="")
	{
		m_title = title;
		m_A = A;
		m_B = B;
		m_C = C;
		m_D = D;
		m_E = E;
		m_QuestID = id;
		m_RightAnswer = answer;
		m_UserAnswer ="";
	}

	ExamPaperQuestion()
	{

	}

	void SetNotAnswered()
	{
		m_UserAnswer ="";
	}
	void Set(const QString & title, int id, QString answer,const QString A="", const QString  B="", const QString C="", const QString  D="", const QString E = "")
	{
		m_title = title;
		m_A = A;
		m_B = B;
		m_C = C;
		m_D = D;
		m_E = E;
		m_QuestID = id;
		m_RightAnswer = answer;
	}
	QString m_title;
	QString m_A, m_B, m_C, m_D, m_E;
	int     m_QuestID;
	QString   m_RightAnswer;
	QString   m_UserAnswer;
};

class ExamUserInFor
{
public:
	ExamUserInFor()
	{
		m_name = "";
		m_userid = 0;
	}
	QString m_name;
	int m_userid;
};

extern ExamMission g_currentMission;
extern ExamUserInFor g_UserInfor;
//extern int g_currentPaperId;