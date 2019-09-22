#pragma once
#include "SYExamGlobal.h"

class SYExamDataManager
{
public:
	static SYExamDataManager & GetInstance();

	SYExamDataManager();

	void SelectAllUserMission(int userid, std::vector<ExamMission> & paperlist);

	//��ȡĳ���Ծ�Ĳ��������б�
	void GetQuestionList(int paperid, std::vector<ExamPaperQuestion> & questList,int number);
	
	//�ύ�����������ϸ��� ѡ��𰸵�
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

	//�����û�ĳ�β��Խ�������飬����������Ŀ�Ļش��Լ���ȷ��
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