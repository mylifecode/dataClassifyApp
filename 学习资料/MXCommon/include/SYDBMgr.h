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
		/// 用户表
		DT_UserInfo,
		/// 用户分数表
		DT_Score,
		/// 分数详情表
		DT_ScoreDetail,
		/// 小组信息表
		DT_Group,
		/// 消息接收表
		DT_Message,
		/// 消息回复表
		DT_MessageReply,
		/// 训练报告表，将取代旧的分数表
		DT_OperateReport,
		/// 操作项记录，记录训练报告表的细节
		DT_OperateItem,
		/// 屏幕截图表
		DT_Screenshot,
		/// 视频文件表
		DT_VideoFile,
		/// 答题分类表
		DT_ExamPaperList,

		DT_ExamPaperDetail,

		DT_ExamMissionList,

		DT_ExamMissionDetail,

		/// 选定的答题表
		DT_Question,
		/// 用户理论答题成绩
		DT_AnswerScore,
		/// 班级信息表
		DT_Class,
		/// 评分表,注意不是分数表
		DT_ScoreTable,
		//训练解锁
		DT_UserTrainLock,

		//训练任务开始
		DT_TRAINFORTASK,
		DT_TRAINTASKLIST,
		DT_TRAINTASKDetail,
		//训练任务结束

	
	};

	static SYDBMgr* Instance();

	/**打开数据库 如果已经打开则忽略**/
	bool Open();

	/**重新打开 如果已经打开则先关闭在打开**/
	bool ReOpen();
	/** 关闭数据库 */
	void Close();

	const QString& GetTableName(DatabaseTable table);

	/** 获取数据库，临时函数，用来兼容以往的数据库操作
		后期将删除该函数！！！ TODO
	*/
	QSqlDatabase& GetDatabase(){return m_database;};

	/**
		执行sql语句，如果查询到结果，则返回true。如果sql错误、或查询到0个记录，则返回false
	*/
	bool Query(const QString& sql,QVector<QSqlRecord>& result);
	
	/* 执行sql语句，如果成功则返回true*/
	bool Exec(const QString& sql);

	/*根据小组号获取相应的小组名 */
	bool GetGroupNameFromId( QString& GroupName, int& GroupId);

	/* 根据小组号获取管理员的用户名*/
	bool GetAdminNameFromId( QString& AdminName,int& GroupId);

	/**
		查询指定表的所有信息
	*/

	bool QueryInfo(DatabaseTable dt,QVector<QSqlRecord> & records);

	/**
		根据用户名，从DT_UserInfo表中查询用户的相关信息
		相关属性值：id、username、password、initPassword、groupId、realName、permission
		如果查询到有结果，则返回ture，否则，返回false
	*/
	bool QueryPaperInfo(int Type, QVector<QSqlRecord>&result);   //查询试卷详情
	bool QueryUserInfo(const QString& userName,QSqlRecord & record);
	
	/*查询理论答题次数和平均分*/
	bool QueryExamNumAndAvgScore(int userid, QSqlRecord & record);
	
	/*查询理论答题及格次数*/
	bool QueryExamPassNum(int userid, int passScore, QSqlRecord & record);
	/**
	如果查询到有结果，则返回ture，否则，返回false
	*/
	bool QueryQuestion(const QString& title, QSqlRecord & record);
	bool QueryQuestion(int id, QSqlRecord &record);

	bool QueryQuestionsNumber(int &number);
	/**
		根据用户id查询用户信息
	*/
	bool QueryUserInfo(int userId,QSqlRecord & record);

	bool QueryAllUserInfo(std::vector<QSqlRecord>& records);

	int QueryNumberOfUser();

	/**
		查询指定小组内的用户信息
	*/
	bool QueryUserInGroup(int groupId, std::vector<QSqlRecord> & records);

	/**
		查询所有未分组的普通用户信息，不包括管理员和超级管理员
	*/
	bool QueryNoGroupUserInfo(std::vector<QSqlRecord> & records);

	/**
		从用户表(DT_UserInfo)中查询所有的管理员信息：教师、超级管理员
	*/
	bool QueryAllManager(QVector<QSqlRecord> & records);

	/**
		根据班级id查询完整的班级信息
	*/
	bool QueryClassInfo(int classId, QSqlRecord& record);

	/**
		从小组表中查询所有的小组信息
	*/
	bool QueryAllGroupInfo(QVector<QSqlRecord> & records);
	/**
		根据提供的管理员id，查询该管理员所拥有的所有小组信息
	*/
	bool QueryGroupInfo(int adminId,QVector<QSqlRecord> & records);

	/**
		根据小组id查询对应的小组信息
	*/
	bool QueryGroupInfo(int groupId,QSqlRecord& record);

	/**
		根据小组名获取相应的小组信息
	*/
	bool QueryGroupInfo(const QString & groupName,QSqlRecord & record);

	/**
		获取指定分数id的得分项
	*/
	bool QueryFullScoreInfo(int scoreId,QSqlRecord & record);

	/** 查询某一用户所有训练的时间总和,失败时返回值为0 */
	int QueryTotalTrainTime(int userId);

	/** 
		查询指定用户的id的基本训练信息,成功返回true，否则false
		userId:用户id
		totalTime：所有训练总用时
		simulationTrainTimes：实物训练次数
		skillingTrainTimes：技能训练次数
		surgeryTrainTimes：手术训练次数
	*/
	bool QueryBasicTrainInfo(int userId, int& totalTime, int& simulationTrainTimes, int& skillingTrainTimes, int& surgeryTrainTimes);

	/** 
		查询指定训练类型的相关信息
		times:训练次数
		totalTime：训练累计时间
		nPeople：训练人数
	*/
	bool QueryBasicTrainInfo(TrainType type, int& times, int& totalTime, int& nPeople);

	/** 查询某一训练的训练次数 */
	bool QueryTrainTimes(TrainType type, const QString& trainCode, int& trainTimes);

	/** 
		查询指定类型的训练数据分布：训练次数分布、训练时间分布
		record：trainTypeName,trainName,times,totalTime
	*/
	bool QueryTrainDataDistribution(TrainType type, std::vector<QSqlRecord>& records);

	/** 查询指定用户id的训练完成个数（统计的是不同的训练的个数）,成功时返回值为大于等于0，失败返回-1 */
	int QueryNumberOfCompletedTrain(int userId);

	/** 查询指定训练类型的所有分数记录，记录按照时间从大到小排序 */
	bool QueryAllScoreRecordOrderByDateDescend(int userId,std::vector<QSqlRecord>& records);

	bool QueryAllScoreRecordOrderByDateDescend(TrainType type, std::vector<QSqlRecord>& records,bool needExtraUserInfo = false);

	bool QueryAllUserTrainTimesOrderByDescend(int tranTypeCode, std::vector<QSqlRecord>& records);

	bool QueryAllUserTrainTimesOrderByDescend(std::vector<QSqlRecord>& records);

	/** 查询所有训练的排名信息 */
	bool QueryAvgScoreRankInfo(std::vector<QSqlRecord>& records);

	/** 查询指定训练类型的排名信息 */
	bool QueryAvgScoreRankInfo(int trainType, std::vector<QSqlRecord>& records, bool needExtraUserInfo = false);

	bool QueryAvgScoreRankInfo(int trainType, const QString& trainCode, std::vector<QSqlRecord>& records, bool needExtraUserInfo = false);

	bool QueryScoreRankInfo(int trainType, const QString& trainCode, std::vector<QSqlRecord>& records, bool needExtraUserInfo = false);

	/** 
		查询指定训练类型的次数排名，结果降序排序 
		record:name,times
	*/
	bool QueryTrainCountRank(TrainType type, std::vector<QSqlRecord>& records);

	/** 
		查询指定训练类型的时间排名，结果降序排序 
		record:name,time
	*/
	bool QueryTrainTimeRank(TrainType type, std::vector<QSqlRecord>& records);

	/** 
		查询夹豆子训练计数排名，结果降序排序 
		record:name,times
	*/
	bool QueryClampPeaTrainCountRank(std::vector<QSqlRecord>& records);

	/** 查询所有的得分细节 */
	bool QueryAllScoreDetail(std::vector<QSqlRecord>& records);

	/** 查询指定类型训练的所有得分细节 */
	bool QueryScoreDetailByTrainType(int trainType, std::vector<QSqlRecord>& records);

	/**
		根据分数id查询得分明细
	*/
	bool QueryScoreDetail(int scoreId,QVector<QSqlRecord> & records);

	/**  
		查询某一用户id的一特定训练的历史分数明细
	*/

	bool QueryHistoryScoreDetail(int userId, const QString& trainTypeCode, const QString& trainCode,QVector<QSqlRecord>& records);

	/**
		根据分数id查询截图信息
	*/
	bool QueryScreenshots(int scoreId, QVector<QSqlRecord> & records);

	/**
		根据分数id查询视频文件
	*/
	bool QueryVideoFiles(int scoreId, QVector<QSqlRecord> & records);

	/**
		查询指定表中，id的最大值，成功将返回该id结构，如果表中无id属性等情况，将返回-1
	*/
	int QueryMaxIdFromTable(DatabaseTable table);

	//查询学生当前任务id
	int QueryStuMissionId(int userid, int paperid);

	/**
		查询指定表中，groupId的最大值，成功将返回该groupId结构，如果表中无属性等情况，将返回-1
	*/
	int QueryMaxgroupIdFromTable(DatabaseTable table);

	/**
		查询评分表中的所有评分项数据
	*/
	bool QueryAllScoreTableItem(QVector<QSqlRecord>& records);

	/**  
		查询指定用户id的消息
	*/
	bool QueryMessagesByReceiverId(int userId,std::vector<QSqlRecord>& records);

	bool QueryMessagesBySenderId(int userId, std::vector<QSqlRecord>& records);
	
	/** 查询关于某条消息的所有回复内容 */
	bool QueryMessageReply(int messageId, std::vector<QSqlRecord>& records);

// 	/**
// 		保存一个得分记录，该得分记录可能会有多个具体的得分细节项ScoreItem
// 		如果操作成功，将返回该分数记录的分数id值，否则，返回-1
// 	*/
// 	int AddScoreRecord(const TrainScoreRecord& scoreRecord,const QMap<QString,QPixmap>& screenshots);

	/**
		保存一个报告记录，该得报告中包含了多个操作项，并且每个操作项有可能有得分点
		如果操作成功，将返回该分数记录的分数id值，否则，返回-1
	*/
	int SaveReportRecord(const SYTrainReportRecord& reportRecord, const QMap<QString, QPixmap>& screenshots,const QVector<QString>& videoFiles);

	/**
		添加一个用户，返回其用户id，如果失败，返回-1
	*/
	int AddUserInfo(const QString& userName,const QString& realName,const QString& password);

	bool AddUserInfo(int id,const QString& userName,const QString& realName,const QString& password);

	/**
		添加一个用户，返回其用户id，如果失败，返回-1
	*/
	int AddUserInfo(const QString& userName,const QString& password,const QString& initPassword,const QString& realName,int permission = 1);

	/**
		根据小组名和管理员id来创建一个小组，如果小组名已经存在或则id无效，则创建失败。
		如果成功，返回该小组的id值，否则返回-1
	*/
	int AddGroup(const QString& groupName, const QString& ownerName, int ownerId);

	/** 
		保存消息，返回1，失败返回-1
	*/
	//int AddMessageInfo(const QString& senderName,const QString& receiverName,bool isGroup,const QString& msgSubject,const QString& msgContent);

	void AddMessageInfo(int senderId, int receiverId, const QString& subject, const QString& content);

	void AddMessageReply(int messageId, int replyerId, const QString& content);

	/**
	保存答题，返回1，失败返回-1
	*/
	int AddQuestionBank(int subject_type, const QString& title, const QString& A, const QString& B, const QString& C, const QString& D, const QString& E, const QString& answer);

	/**
	保存选定的答题，返回1，失败返回-1
	*/
	int AddQuestion(int subject_type, const QString& title, const QString& A, const QString& B, const QString& C, const QString& D, const QString& E,const QString& answer,int id);

	/**
	保存选定的答题，返回1，失败返回-1
	*/
	int AddAnswerScore(int userId, const QString& endAnswerTime, const QString& correctrate);

	/**  
		向评分表里添加一条评分表项，成功返回1，失败返回-1
	*/
	int AddScoreTableItem(const QString& scoreTableCode,const QString& scoreCode,
						  const QString& trainName,const QString& trainCode,const QString& trainTypeCode,
						  const QString& stepName,const QString& stepCode,const QString& stepDetailedDescription,int isKeyStep,
						  const QString& scoreItemContent,const QString& scoreItemCode,int scoreItemValue,
						  const QString& scorePointDetailName,const QString& scorePointDetailCode,int scorePointDetailValue,int abilitys);

	/** 更新题目 */
	int UpdateQuestionBank(const QString& title, const QString& A, const QString& B, const QString& C, const QString& D, const QString& E, const QString& answer);
	
	/** 更新指定用户id的用户名 */
	bool UpdateUserName(int userId,const QString& userName);

	/** 跟新指定用户id的真实姓名 */
	bool UpdateUserRealName(int userId,const QString& realName);

	/** 更新指定用户id的性别 */
	bool UpdateUserSex(int userId, bool isMale);

	/** 更新指定用户id的小组号 */
	bool UpdateUserGroupId(int userId,int groupId);

	/** 更新多个用户的组id和组名字 */
	bool UpdateUsersGroup(std::vector<int> & userIds, int GroupID, const QString & groupName);
	
	/** 更新指定用户id的密码 */
	bool UpdateUserPassword(int userId,const QString& password);

	/** 更新指定用户id的权限 */
	bool UpdateUserPermission(int userId,int permisstion);

	/** 更新登录次数 */
	bool UpdateLoginTimes(int userId,int times);

	/** 更新首次登录时间 */
	bool UpdateFirstLoginTime(int userId, const QDateTime& dataTime);

	/** 更新最近登录时间 */
	bool UpdateLastLoginTime(int userId,const QDateTime & dateTime);

	/** 更新总在线时间 */
	bool UpdateTotalOnlineTime(int userId, unsigned int time);

	/** 更新总训练时间 */
	//bool UpdateTotalTrainTime(int userId, unsigned int time);

	/** 更新用户表中用户的组号 */
	bool UpdateAllUserGroupId(int oldGroupId,int newGroupId);

	/** 更新指定小组id的小明名称 */
	bool UpdateGroupName(int groupId,const QString& newGroupName);

	/** 更改小组名称 */
	bool UpdateGroupName(const QString& oldGroupName,const QString& newGroupName);

	/** 更改小组所属者id，即管理员id */
	bool UpdateGroupAdminId(int groupId,int adminId);

	/** 更改访客id，默认访客id为0 */
	bool UpdateVisitorId();

	/**
		更新评分表里的评分项
	*/
	bool UpdateScoreTableItem(const QString& oldScoreTableCode, const QString& curScoreTableCode,
							  const QString& oldScoreCode, const QString& curScoreCode,
							  const QString& trainName, const QString& trainCode, const QString& trainTypeCode,
							  const QString& stepName, const QString& stepCode, const QString& stepDetailedDescription, int isKeyStep,
							  const QString& scoreItemContent, const QString& scoreItemCode, int scoreItmeValue,
							  const QString& scorePointDetailName, const QString& scorePointDetailCode, int scorePointDetailValue, int abilitys);

	/** 更新消息记录的状态字段 */
	bool UpdateMessageStatus(int messageId, int status);

	/** 更新消息回复记录的状态字段 */
	bool UpdateMessageReplyStatus(int messageReplyId, int status);

	/**更具用户id删除用户表在中的相关信息，如果该用户是管理员，那么将自动删除所拥有的小组信息*/
	bool DeleteUserInfo(int userID);

	/** 根据用户名删除用户表中的相关信息，如果该用户是管理员，那么将自动删除所拥有的小组信息 */
	bool DeleteUserInfo(const QString& userName);

	/** 删除指定id的小组 */
	bool DeleteGroup(int groupId);

	/** 删除管理员的所拥有的所有小组 */
	bool DeleteGroupAccordingAdminId(int adminId);

	/** 删除题目 */
	bool DeleteQuestionBank(const QString& title);

	/** 删除所有选定的题目 */
	bool DeleteQuestion();
	bool DeleteQuestions(int type);
	//删除指定类型的题目
	/* 删除指定的题目吧*/
	bool DeleteQuestions(QVector<QSqlRecord>&records);

	/** 删除指定的评分表 */
	bool DeleteScoreTable(const QString& scoreTableCode);

	/** 删除指定评分表中的某一项 */
	bool DeleteScoreTableItem(const QString& scoreTableCode, const QString& scoreCode);

	/*下拉任务表的所有训练*/
	bool QueryAllTrainsCanBeAssignedToTask(std::vector<QSqlRecord> & records);

	void QueryAllReceiverTasks(int receiverID, std::vector<QSqlRecord> & records);

	void QueryAllAssignorTaskTemplate(int assignor, std::vector<QSqlRecord> & records);

	void QueryAllTrainsInOneTaks(int taskID, std::vector<QSqlRecord> & records);

	int  AddOneTaskEntry(const QString & TaskName, int assignorID, int receiverID, const QString & assignorName, const QString & receiverName, int trainNum, bool isTemplate);

	void AddOneTrainEntryInTask(int taskId, const QString & trainName, const QString & trainGradeCode, int PassNeedScore, int PassNeedTime);
	
	void UpdateTaskTrainData(int id , int TrainMinute , int Score);
	bool QueryPaperSetInfo(DatabaseTable dt, QVector<QSqlRecord> & records);  //查询试卷集信息
	
	bool QueryOneTrainRecords(int userId, const QString trainName,int& score);  //查询用户自己训练的数据

	bool QueryUserTrainIsLock(int userId, QString trainCode);  //查看用户训练是否解锁

	bool UpdateUserSkillTrainLockState(int userId, QString trainCode);  //插入新解锁的记录

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
