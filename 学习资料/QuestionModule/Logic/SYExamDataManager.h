#pragma once
#include "SYExamGlobal.h"

class SYExamDataManager
{
public:
	static SYExamDataManager & GetInstance();

	SYExamDataManager();

	void SelectAllUserMission(int userid, std::vector<ExamMission> & paperlist);

	//获取某张试卷的测试问题列表
	void GetQuestionList(int paperid, std::vector<ExamPaperQuestion> & questList,int number);
	
	//提交测验任务的详细结果 选择答案等
	ExamMission SubmitMissionResultDetail(int userid,
		                           
		                           int paperid,
		                           
								   const QString &papername,
								   
								   const QString & paperassignor,
		                           
								   int questionnum,
								   
								   int correctAnswerNum,
		                          
								   int Score,

								   int examToatlTime,

								   int answertimeused,   
		                           
								   std::vector<ExamPaperQuestion> & missionQuests,bool isFinished=false);

	//返回用户某次测试结果的详情，包括所有题目的回答以及正确答案
	void GetExamMissionResultDetail(int missionid, std::vector<ExamPaperQuestion> & questList);

	//
	void ImportOneQuestionToDB(const QString & questContent ,
		                       const QString & answerA,
							   const QString & answerB, 
							   const QString & answerC, 
							   const QString & answerD,
							   const QString & answerE,
							   const QString & correctAnswer);
	int GetExamMissionPaperId(QString paperName);

	int GetMissionPaperScore(int paperid);

	void QueryMissionInfo(QString paperName, int &queNum, int &examTime);

protected:
	static SYExamDataManager s_Instance;
};