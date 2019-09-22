#pragma once
#include <QSqlDatabase>
#include <QVector>
#include <QVariant>
#include <QSqlQuery>
#include <QSqlRecord>
#include<QPair>
#include <QMap>
#include "MXCommon.h"
struct TrainScoreRecord;
struct SYTrainReportRecord;
enum TrainType;


typedef struct PaperInfoDetail
{
	QString paperName;
	QString paperRank;
	QString questionNums;
	QString createPaperTime;
	QString examTime;
	QString creator;
	QString examScore;

	PaperInfoDetail(QString t_paperName, QString t_examTime,QString t_paperRank="", QString t_questionNums="", QString t_createPaperTime="", QString t_creator="", QString t_examScore="") :
		paperName(t_paperName), examTime(t_examTime), paperRank(t_paperRank), questionNums(t_questionNums), createPaperTime(t_createPaperTime), creator(t_creator), examScore(t_examScore) {};
	
	
}PaperInfoDetail;


class MXCOMMON_API SYDBMgr
{
private:
	SYDBMgr(void);
	~SYDBMgr(void);
public:
	enum DatabaseTable
	{
		/// �û���
		DT_UserInfo,
		/// �û�������
		DT_Score,
		/// ���������
		DT_ScoreDetail,
		/// С����Ϣ��
		DT_Group,
		/// ��Ϣ���ձ�
		DT_Message,
		/// ��Ϣ�ظ���
		DT_MessageReply,
		/// ѵ���������ȡ���ɵķ�����
		DT_OperateReport,
		/// �������¼����¼ѵ��������ϸ��
		DT_OperateItem,
		/// ��Ļ��ͼ��
		DT_Screenshot,
		/// ��Ƶ�ļ���
		DT_VideoFile,
		/// ��������
		DT_ExamPaperList,

		DT_ExamPaperDetail,

		DT_ExamMissionList,

		DT_ExamMissionDetail,

		/// ѡ���Ĵ����
		DT_Question,
		/// �û����۴���ɼ�
		DT_AnswerScore,
		/// �༶��Ϣ��
		DT_Class,
		/// ���ֱ�,ע�ⲻ�Ƿ�����
		DT_ScoreTable,
		//ѵ������
		DT_UserTrainLock,

		//ѵ������ʼ
		DT_TRAINFORTASK,
		DT_TRAINTASKLIST,
		DT_TRAINTASKDetail,
		//ѵ���������

	
	};

	static SYDBMgr* Instance();

	/**�����ݿ� ����Ѿ��������**/
	bool Open();

	/**���´� ����Ѿ������ȹر��ڴ�**/
	bool ReOpen();
	/** �ر����ݿ� */
	void Close();

	const QString& GetTableName(DatabaseTable table);

	/** ��ȡ���ݿ⣬��ʱ�����������������������ݿ����
		���ڽ�ɾ���ú��������� TODO
	*/
	QSqlDatabase& GetDatabase(){return m_database;};

	/**
		ִ��sql��䣬�����ѯ��������򷵻�true�����sql���󡢻��ѯ��0����¼���򷵻�false
	*/
	bool Query(const QString& sql,QVector<QSqlRecord>& result);
	
	/* ִ��sql��䣬����ɹ��򷵻�true*/
	bool Exec(const QString& sql);

	/*����С��Ż�ȡ��Ӧ��С���� */
	bool GetGroupNameFromId( QString& GroupName, int& GroupId);

	/* ����С��Ż�ȡ����Ա���û���*/
	bool GetAdminNameFromId( QString& AdminName,int& GroupId);

	/**
		��ѯָ�����������Ϣ
	*/

	bool QueryInfo(DatabaseTable dt,QVector<QSqlRecord> & records);

	/**
		�����û�������DT_UserInfo���в�ѯ�û��������Ϣ
		�������ֵ��id��username��password��initPassword��groupId��realName��permission
		�����ѯ���н�����򷵻�ture�����򣬷���false
	*/
	bool QueryPaperInfo(int Type, QVector<QSqlRecord>&result);   //��ѯ�Ծ�����
	bool QueryUserInfo(const QString& userName,QSqlRecord & record);
	
	/*��ѯ���۴��������ƽ����*/
	bool QueryExamNumAndAvgScore(int userid, QSqlRecord & record);
	
	/*��ѯ���۴��⼰�����*/
	bool QueryExamPassNum(int userid, int passScore, QSqlRecord & record);
	/**
	�����ѯ���н�����򷵻�ture�����򣬷���false
	*/
	bool QueryQuestion(const QString& title, QSqlRecord & record);
	bool QueryQuestion(int id, QSqlRecord &record);

	bool QueryQuestionsNumber(int &number);
	/**
		�����û�id��ѯ�û���Ϣ
	*/
	bool QueryUserInfo(int userId,QSqlRecord & record);

	bool QueryAllUserInfo(std::vector<QSqlRecord>& records);

	int QueryNumberOfUser();

	/**
		��ѯָ��С���ڵ��û���Ϣ
	*/
	bool QueryUserInGroup(int groupId, std::vector<QSqlRecord> & records);

	/**
		��ѯ����δ�������ͨ�û���Ϣ������������Ա�ͳ�������Ա
	*/
	bool QueryNoGroupUserInfo(std::vector<QSqlRecord> & records);

	/**
		���û���(DT_UserInfo)�в�ѯ���еĹ���Ա��Ϣ����ʦ����������Ա
	*/
	bool QueryAllManager(QVector<QSqlRecord> & records);

	/**
		���ݰ༶id��ѯ�����İ༶��Ϣ
	*/
	bool QueryClassInfo(int classId, QSqlRecord& record);

	/**
		��С����в�ѯ���е�С����Ϣ
	*/
	bool QueryAllGroupInfo(QVector<QSqlRecord> & records);
	/**
		�����ṩ�Ĺ���Աid����ѯ�ù���Ա��ӵ�е�����С����Ϣ
	*/
	bool QueryGroupInfo(int adminId,QVector<QSqlRecord> & records);

	/**
		����С��id��ѯ��Ӧ��С����Ϣ
	*/
	bool QueryGroupInfo(int groupId,QSqlRecord& record);

	/**
		����С������ȡ��Ӧ��С����Ϣ
	*/
	bool QueryGroupInfo(const QString & groupName,QSqlRecord & record);

	/**
		��ȡָ������id�ĵ÷���
	*/
	bool QueryFullScoreInfo(int scoreId,QSqlRecord & record);

	/** ��ѯĳһ�û�����ѵ����ʱ���ܺ�,ʧ��ʱ����ֵΪ0 */
	int QueryTotalTrainTime(int userId);

	/** 
		��ѯָ���û���id�Ļ���ѵ����Ϣ,�ɹ�����true������false
		userId:�û�id
		totalTime������ѵ������ʱ
		simulationTrainTimes��ʵ��ѵ������
		skillingTrainTimes������ѵ������
		surgeryTrainTimes������ѵ������
	*/
	bool QueryBasicTrainInfo(int userId, int& totalTime, int& simulationTrainTimes, int& skillingTrainTimes, int& surgeryTrainTimes);

	/** 
		��ѯָ��ѵ�����͵������Ϣ
		times:ѵ������
		totalTime��ѵ���ۼ�ʱ��
		nPeople��ѵ������
	*/
	bool QueryBasicTrainInfo(TrainType type, int& times, int& totalTime, int& nPeople);

	/** ��ѯĳһѵ����ѵ������ */
	bool QueryTrainTimes(TrainType type, const QString& trainCode, int& trainTimes);

	/** 
		��ѯָ�����͵�ѵ�����ݷֲ���ѵ�������ֲ���ѵ��ʱ��ֲ�
		record��trainTypeName,trainName,times,totalTime
	*/
	bool QueryTrainDataDistribution(TrainType type, std::vector<QSqlRecord>& records);

	/** ��ѯָ���û�id��ѵ����ɸ�����ͳ�Ƶ��ǲ�ͬ��ѵ���ĸ�����,�ɹ�ʱ����ֵΪ���ڵ���0��ʧ�ܷ���-1 */
	int QueryNumberOfCompletedTrain(int userId);

	/** ��ѯָ��ѵ�����͵����з�����¼����¼����ʱ��Ӵ�С���� */
	bool QueryAllScoreRecordOrderByDateDescend(int userId,std::vector<QSqlRecord>& records);

	bool QueryAllScoreRecordOrderByDateDescend(TrainType type, std::vector<QSqlRecord>& records,bool needExtraUserInfo = false);

	bool QueryAllUserTrainTimesOrderByDescend(int tranTypeCode, std::vector<QSqlRecord>& records);

	bool QueryAllUserTrainTimesOrderByDescend(std::vector<QSqlRecord>& records);

	/** ��ѯ����ѵ����������Ϣ */
	bool QueryAvgScoreRankInfo(std::vector<QSqlRecord>& records);

	/** ��ѯָ��ѵ�����͵�������Ϣ */
	bool QueryAvgScoreRankInfo(int trainType, std::vector<QSqlRecord>& records, bool needExtraUserInfo = false);

	bool QueryAvgScoreRankInfo(int trainType, const QString& trainCode, std::vector<QSqlRecord>& records, bool needExtraUserInfo = false);

	bool QueryScoreRankInfo(int trainType, const QString& trainCode, std::vector<QSqlRecord>& records, bool needExtraUserInfo = false);

	/** 
		��ѯָ��ѵ�����͵Ĵ�������������������� 
		record:name,times
	*/
	bool QueryTrainCountRank(TrainType type, std::vector<QSqlRecord>& records);

	/** 
		��ѯָ��ѵ�����͵�ʱ������������������� 
		record:name,time
	*/
	bool QueryTrainTimeRank(TrainType type, std::vector<QSqlRecord>& records);

	/** 
		��ѯ�ж���ѵ����������������������� 
		record:name,times
	*/
	bool QueryClampPeaTrainCountRank(std::vector<QSqlRecord>& records);

	/** ��ѯ���еĵ÷�ϸ�� */
	bool QueryAllScoreDetail(std::vector<QSqlRecord>& records);

	/** ��ѯָ������ѵ�������е÷�ϸ�� */
	bool QueryScoreDetailByTrainType(int trainType, std::vector<QSqlRecord>& records);

	/**
		���ݷ���id��ѯ�÷���ϸ
	*/
	bool QueryScoreDetail(int scoreId,QVector<QSqlRecord> & records);

	/**  
		��ѯĳһ�û�id��һ�ض�ѵ������ʷ������ϸ
	*/

	bool QueryHistoryScoreDetail(int userId, const QString& trainTypeCode, const QString& trainCode,QVector<QSqlRecord>& records);

	/**
		���ݷ���id��ѯ��ͼ��Ϣ
	*/
	bool QueryScreenshots(int scoreId, QVector<QSqlRecord> & records);

	/**
		���ݷ���id��ѯ��Ƶ�ļ�
	*/
	bool QueryVideoFiles(int scoreId, QVector<QSqlRecord> & records);

	/**
		��ѯָ�����У�id�����ֵ���ɹ������ظ�id�ṹ�����������id���Ե������������-1
	*/
	int QueryMaxIdFromTable(DatabaseTable table);

	//��ѯѧ����ǰ����id
	int QueryStuMissionId(int userid, int paperid);

	/**
		��ѯָ�����У�groupId�����ֵ���ɹ������ظ�groupId�ṹ��������������Ե������������-1
	*/
	int QueryMaxgroupIdFromTable(DatabaseTable table);

	/**
		��ѯ���ֱ��е���������������
	*/
	bool QueryAllScoreTableItem(QVector<QSqlRecord>& records);

	/**  
		��ѯָ���û�id����Ϣ
	*/
	bool QueryMessagesByReceiverId(int userId,std::vector<QSqlRecord>& records);

	bool QueryMessagesBySenderId(int userId, std::vector<QSqlRecord>& records);
	
	/** ��ѯ����ĳ����Ϣ�����лظ����� */
	bool QueryMessageReply(int messageId, std::vector<QSqlRecord>& records);

// 	/**
// 		����һ���÷ּ�¼���õ÷ּ�¼���ܻ��ж������ĵ÷�ϸ����ScoreItem
// 		��������ɹ��������ظ÷�����¼�ķ���idֵ�����򣬷���-1
// 	*/
// 	int AddScoreRecord(const TrainScoreRecord& scoreRecord,const QMap<QString,QPixmap>& screenshots);

	/**
		����һ�������¼���õñ����а����˶�����������ÿ���������п����е÷ֵ�
		��������ɹ��������ظ÷�����¼�ķ���idֵ�����򣬷���-1
	*/
	int SaveReportRecord(const SYTrainReportRecord& reportRecord, const QMap<QString, QPixmap>& screenshots,const QVector<QString>& videoFiles);

	/**
		���һ���û����������û�id�����ʧ�ܣ�����-1
	*/
	int AddUserInfo(const QString& userName,const QString& realName,const QString& password);

	bool AddUserInfo(int id,const QString& userName,const QString& realName,const QString& password);

	/**
		���һ���û����������û�id�����ʧ�ܣ�����-1
	*/
	int AddUserInfo(const QString& userName,const QString& password,const QString& initPassword,const QString& realName,int permission = 1);

	/**
		����С�����͹���Աid������һ��С�飬���С�����Ѿ����ڻ���id��Ч���򴴽�ʧ�ܡ�
		����ɹ������ظ�С���idֵ�����򷵻�-1
	*/
	int AddGroup(const QString& groupName, const QString& ownerName, int ownerId);

	/** 
		������Ϣ������1��ʧ�ܷ���-1
	*/
	//int AddMessageInfo(const QString& senderName,const QString& receiverName,bool isGroup,const QString& msgSubject,const QString& msgContent);

	void AddMessageInfo(int senderId, int receiverId, const QString& subject, const QString& content);

	void AddMessageReply(int messageId, int replyerId, const QString& content);

	/**
	������⣬����1��ʧ�ܷ���-1
	*/
	int AddQuestionBank(int subject_type, const QString& title, const QString& A, const QString& B, const QString& C, const QString& D, const QString& E, const QString& answer);

	/**
	����ѡ���Ĵ��⣬����1��ʧ�ܷ���-1
	*/
	int AddQuestion(int subject_type, const QString& title, const QString& A, const QString& B, const QString& C, const QString& D, const QString& E,const QString& answer,int id);

	/**
	����ѡ���Ĵ��⣬����1��ʧ�ܷ���-1
	*/
	int AddAnswerScore(int userId, const QString& endAnswerTime, const QString& correctrate);

	/**  
		�����ֱ������һ�����ֱ���ɹ�����1��ʧ�ܷ���-1
	*/
	int AddScoreTableItem(const QString& scoreTableCode,const QString& scoreCode,
						  const QString& trainName,const QString& trainCode,const QString& trainTypeCode,
						  const QString& stepName,const QString& stepCode,const QString& stepDetailedDescription,int isKeyStep,
						  const QString& scoreItemContent,const QString& scoreItemCode,int scoreItemValue,
						  const QString& scorePointDetailName,const QString& scorePointDetailCode,int scorePointDetailValue,int abilitys);

	/** ������Ŀ */
	int UpdateQuestionBank(const QString& title, const QString& A, const QString& B, const QString& C, const QString& D, const QString& E, const QString& answer);
	
	/** ����ָ���û�id���û��� */
	bool UpdateUserName(int userId,const QString& userName);

	/** ����ָ���û�id����ʵ���� */
	bool UpdateUserRealName(int userId,const QString& realName);

	/** ����ָ���û�id���Ա� */
	bool UpdateUserSex(int userId, bool isMale);

	/** ����ָ���û�id��С��� */
	bool UpdateUserGroupId(int userId,int groupId);

	/** ���¶���û�����id�������� */
	bool UpdateUsersGroup(std::vector<int> & userIds, int GroupID, const QString & groupName);
	
	/** ����ָ���û�id������ */
	bool UpdateUserPassword(int userId,const QString& password);

	/** ����ָ���û�id��Ȩ�� */
	bool UpdateUserPermission(int userId,int permisstion);

	/** ���µ�¼���� */
	bool UpdateLoginTimes(int userId,int times);

	/** �����״ε�¼ʱ�� */
	bool UpdateFirstLoginTime(int userId, const QDateTime& dataTime);

	/** ���������¼ʱ�� */
	bool UpdateLastLoginTime(int userId,const QDateTime & dateTime);

	/** ����������ʱ�� */
	bool UpdateTotalOnlineTime(int userId, unsigned int time);

	/** ������ѵ��ʱ�� */
	//bool UpdateTotalTrainTime(int userId, unsigned int time);

	/** �����û������û������ */
	bool UpdateAllUserGroupId(int oldGroupId,int newGroupId);

	/** ����ָ��С��id��С������ */
	bool UpdateGroupName(int groupId,const QString& newGroupName);

	/** ����С������ */
	bool UpdateGroupName(const QString& oldGroupName,const QString& newGroupName);

	/** ����С��������id��������Աid */
	bool UpdateGroupAdminId(int groupId,int adminId);

	/** ���ķÿ�id��Ĭ�Ϸÿ�idΪ0 */
	bool UpdateVisitorId();

	/**
		�������ֱ����������
	*/
	bool UpdateScoreTableItem(const QString& oldScoreTableCode, const QString& curScoreTableCode,
							  const QString& oldScoreCode, const QString& curScoreCode,
							  const QString& trainName, const QString& trainCode, const QString& trainTypeCode,
							  const QString& stepName, const QString& stepCode, const QString& stepDetailedDescription, int isKeyStep,
							  const QString& scoreItemContent, const QString& scoreItemCode, int scoreItmeValue,
							  const QString& scorePointDetailName, const QString& scorePointDetailCode, int scorePointDetailValue, int abilitys);

	/** ������Ϣ��¼��״̬�ֶ� */
	bool UpdateMessageStatus(int messageId, int status);

	/** ������Ϣ�ظ���¼��״̬�ֶ� */
	bool UpdateMessageReplyStatus(int messageReplyId, int status);

	/**�����û�idɾ���û������е������Ϣ��������û��ǹ���Ա����ô���Զ�ɾ����ӵ�е�С����Ϣ*/
	bool DeleteUserInfo(int userID);

	/** �����û���ɾ���û����е������Ϣ��������û��ǹ���Ա����ô���Զ�ɾ����ӵ�е�С����Ϣ */
	bool DeleteUserInfo(const QString& userName);

	/** ɾ��ָ��id��С�� */
	bool DeleteGroup(int groupId);

	/** ɾ������Ա����ӵ�е�����С�� */
	bool DeleteGroupAccordingAdminId(int adminId);

	/** ɾ����Ŀ */
	bool DeleteQuestionBank(const QString& title);

	/** ɾ������ѡ������Ŀ */
	bool DeleteQuestion();
	bool DeleteQuestions(int type);
	//ɾ��ָ�����͵���Ŀ
	/* ɾ��ָ������Ŀ��*/
	bool DeleteQuestions(QVector<QSqlRecord>&records);

	/** ɾ��ָ�������ֱ� */
	bool DeleteScoreTable(const QString& scoreTableCode);

	/** ɾ��ָ�����ֱ��е�ĳһ�� */
	bool DeleteScoreTableItem(const QString& scoreTableCode, const QString& scoreCode);

	/*��������������ѵ��*/
	bool QueryAllTrainsCanBeAssignedToTask(std::vector<QSqlRecord> & records);

	void QueryAllReceiverTasks(int receiverID, std::vector<QSqlRecord> & records);

	void QueryAllAssignorTaskTemplate(int assignor, std::vector<QSqlRecord> & records);

	void QueryAllTrainsInOneTaks(int taskID, std::vector<QSqlRecord> & records);

	int  AddOneTaskEntry(const QString & TaskName, int assignorID, int receiverID, const QString & assignorName, const QString & receiverName, int trainNum, bool isTemplate);

	void AddOneTrainEntryInTask(int taskId, const QString & trainName, const QString & trainGradeCode, int PassNeedScore, int PassNeedTime);
	
	void UpdateTaskTrainData(int id , int TrainMinute , int Score);
	bool QueryPaperSetInfo(DatabaseTable dt, QVector<QSqlRecord> & records);  //��ѯ�Ծ���Ϣ
	
	bool QueryOneTrainRecords(int userId, const QString trainName,int& score);  //��ѯ�û��Լ�ѵ��������

	bool QueryUserTrainIsLock(int userId, QString trainCode);  //�鿴�û�ѵ���Ƿ����

	bool UpdateUserSkillTrainLockState(int userId, QString trainCode);  //�����½����ļ�¼

	//get paper id
	int GetPaperScore(int paperid);

	/** cache data */
public:	
	
	const std::vector<QSqlRecord>& GetAllUserData(bool needReload = false);

	bool CreatePaperInfo(std::vector<QString> paperInfo, QVector<QSqlRecord>  records,QString pre_paperName="");
	bool QueryPaperExist(QString paperName);

	void QueryPaperInfo(QString paperName, PaperInfoDetail** t_paperInfoDetail, QVector<QSqlRecord> &records);
	void DeletePaperInfo(QString paperName);
	bool ModifyPaperInfo(QString &paperName,QVector<QSqlRecord>&records,PaperInfoDetail *&t_paperInfo);

	bool QueryPaperInfo(QString paperName, int &paperId, int &num, int &time);
	bool FromExcelUpLoadPaperInfo(QVector<QVector<QString>> paperQuestions);
	bool AssignPapersToGroups(QVector<QString>paperNames, QVector<QString>groups, QString assignor);
	bool AssignPapersToUsers(QVector<QString>paperNames, QVector<QString>userNames, QString assignor);

private:
	QSqlDatabase m_database;
	QSqlQuery m_sqlQuery;
	QVector<QString> m_tableNames;
	const QString m_errorTableName;


	/** cache data */
private:
	std::vector<QSqlRecord> m_allUserData;

};
