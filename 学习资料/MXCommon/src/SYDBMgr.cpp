#include "SYDBMgr.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QMessageBox>
#include <QBuffer>
#include <QStringList>
#include "MxDefine.h"
#include "MxGlobalConfig.h"
#include<initializer_list>
#define UseMYSQL 1
#define UseDebug 1
#if(UseDebug)
	#define CheckQuery(ret,checkObject) \
		if(!ret){	\
			QSqlError error = checkObject.lastError();			\
			if(error.isValid())	{								\
				QString errorType = CHS("错误类型：");			\
				switch(error.type()){							\
				case QSqlError::UnknownError:					\
					errorType += "Unknown error";				\
					break;										\
				case QSqlError::TransactionError:				\
					errorType += "Transaction failed error";	\
					break;										\
				case QSqlError::StatementError:					\
					errorType += "SQL statement syntax error";	\
					break;										\
				case QSqlError::ConnectionError:				\
					errorType += "Connection error";			\
					break;										\
				case QSqlError::NoError:						\
					errorType += "No error occurred";			\
					break;										\
				}												\
				Message(errorType,error.databaseText() + "\n" + error.driverText());\
			}\
 		}
#else
	#define CheckQuery(ret,checkObjec)
#endif

#define MIS_V_2017_5_3

#ifdef MIS_V_2017_5_3
QString AdjustDescription(const QString& description)
{
	QString newDescription(description);
	QStringList stringFilters = { CHS("("), CHS("（"), CHS("/"), CHS("次数"), CHS("个") };

	for(const auto& filter : stringFilters)
	{
		int index = newDescription.indexOf(filter); 
		if(index != -1)
		{
			newDescription.remove(index, newDescription.size() - index);
		}
	}

	qDebug() << "AdjustDescription : " << description << " \t-->\t" << newDescription;

	return newDescription;
}
#else
	#define AdjustDescription(description) description
#endif

SYDBMgr::SYDBMgr(void)
:m_errorTableName("errorTableName")
{
	//set table name
	m_tableNames.push_back("UserInfoTable");		//0	DT_UserInfo			用户表
	m_tableNames.push_back("Score");				//1 DT_Score			用户分数表
	m_tableNames.push_back("ScoreDetail");			//2 DT_ScoreDetail		分数详情
	m_tableNames.push_back("GroupTable");			//3 DT_Group			小组信息
	m_tableNames.push_back("message");              //4 DT_Message          消息接收表
	m_tableNames.push_back("messagereply");         //5 DT_MessageReply     消息回复表
	m_tableNames.push_back("OperateReport");        //6 DT_OperateReport    
	m_tableNames.push_back("OperateItem");          //7 DT_OperateItem          
	m_tableNames.push_back("screenshot");           //8 DT_Screenshot    
	m_tableNames.push_back("videofile");			//9 DT_Screenshot    
	m_tableNames.push_back("exampaperlist");        //10 DT_QuestionSet
	m_tableNames.push_back("exampaperdetail");      //11 DT_QuestionSet
	m_tableNames.push_back("exammissionlist");      //12 DT_QuestionBank
	m_tableNames.push_back("exammissiondetail");    //13 DT_QuestionBank
	m_tableNames.push_back("examquestionlib");      //14 DT_Question
	m_tableNames.push_back("answerScore");          //15 DT_AnswerScore
	m_tableNames.push_back("class");				//16 DT_Class
	m_tableNames.push_back("ScoreTable");			//17 DT_ScoreTable
	m_tableNames.push_back("usertrainlock");        //18  DT_UserTrainLock

	m_tableNames.push_back("trainfortask");	    //15 DT_TRAINFORTASK
	m_tableNames.push_back("traintasklist");	    //16 DT_TRAINTASKLIST
	m_tableNames.push_back("traintaskdetail");			//17 DT_TRAINTASKDetail

	//set database
	
	m_database = QSqlDatabase::addDatabase("QMYSQL", QString("sy_connect_%1").arg(time(nullptr)));
	m_database.setHostName(MxGlobalConfig::Instance()->GetDatabaseHostName());
	m_database.setPort(MxGlobalConfig::Instance()->GetDatabasePort());
	m_database.setDatabaseName(MxGlobalConfig::Instance()->GetDatabaseName());
	m_database.setUserName(MxGlobalConfig::Instance()->GetDatabaseUserName());
	m_database.setPassword(MxGlobalConfig::Instance()->GetDatabasePassword());

	if(!Open())
	{
#if(UseDebug)
		CheckQuery(false,m_database);
#else
		Message(CHS("错误"),CHS("连接数据库失败！"));
#endif
	}
}

bool SYDBMgr::Open()
{
	if(m_database.isOpen())
	{
		return true;
	}
	else
	{
		bool openSucceed = m_database.open();
		
		if(openSucceed)
		   m_sqlQuery = QSqlQuery(m_database);

		return openSucceed;
	}
}
bool SYDBMgr::ReOpen()
{
	Close();
	
	bool openSucceed = m_database.open();
	if(openSucceed)
	   m_sqlQuery = QSqlQuery(m_database);

	return openSucceed;
}
SYDBMgr::~SYDBMgr(void)
{
	Close();
}

SYDBMgr* SYDBMgr::Instance()
{
	static SYDBMgr db;
	return &db;
}

void SYDBMgr::Close()
{
	if(m_database.isOpen())
	{
		m_database.commit();
		m_database.close();
	}
}

int SYDBMgr::GetPaperScore(int paperid)
{
	QString questTable = SYDBMgr::Instance()->GetTableName(SYDBMgr::DatabaseTable::DT_ExamPaperList);
	QString sql = QString("select * from %1 where id=%2").arg(questTable).arg(paperid);

	bool ret = m_sqlQuery.exec(sql);
	CheckQuery(ret, m_sqlQuery);

	int score = 100;
	if (ret)
	{
		while (m_sqlQuery.next())
		{
			QSqlRecord record = m_sqlQuery.record();
			score=record.value("examTotalScore").toInt();
			break;

		}
	}
	
	return score;
	
}


const QString& SYDBMgr::GetTableName(DatabaseTable table)
{
	int t = static_cast<int>(table);
	if(t < 0 || table >= m_tableNames.size())
	{
		return m_errorTableName;
	}

	return m_tableNames[t];
}

bool SYDBMgr::QueryInfo(DatabaseTable dt,QVector<QSqlRecord> & records)
{
	QString sql = QString("select * from %1").arg(m_tableNames[dt]);
	bool bRet = m_sqlQuery.exec(sql);
	if(bRet)
	{
		bRet = false;
		while(m_sqlQuery.next())
		{
			records.push_back(m_sqlQuery.record());
			bRet = true;
		}
	}

	return bRet;
}

bool SYDBMgr::QueryPaperSetInfo(DatabaseTable dt, QVector<QSqlRecord> & records)
{

	QString sql = QString("select * from %1").arg(m_tableNames[dt]);
	bool bRet = m_sqlQuery.exec(sql);
	if (bRet)
	{
		bRet = false;
		while (m_sqlQuery.next())
		{
			records.push_back(m_sqlQuery.record());
			bRet = true;
		}
	}

	return bRet;



}


bool SYDBMgr::Query(const QString& sql,QVector<QSqlRecord>& result)  
{
	if(Open())
	{
		bool bRet = m_sqlQuery.exec(sql);
		CheckQuery(bRet,m_sqlQuery);
		//QSqlError error = m_sqlQuery.lastError();	
		while(m_sqlQuery.next())
		{
			result.push_back(m_sqlQuery.record());
			bRet = true;
		}

		return bRet;
	}
	else
		return false;
}

bool SYDBMgr::QueryUserInfo(const QString& userName,QSqlRecord & record)
{
	const QString sql = QString("select * from %1 where userName='%2'")
						.arg(m_tableNames[DT_UserInfo])
						.arg(userName);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet,m_sqlQuery);
	if(bRet)
	{
		if(m_sqlQuery.next())
		{
			record = m_sqlQuery.record();
		}
		else
			bRet = false;
	}

	return bRet;
}

bool SYDBMgr::QueryExamNumAndAvgScore(int userid, QSqlRecord & record)
{
	const QString sql = QString("select avg(score) as AvgScore , count(id) as Count from %1 where userid = %2")
		.arg(m_tableNames[DT_ExamMissionList])
		.arg(userid);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
	if (bRet)
	{
		if (m_sqlQuery.next())
		{
			record = m_sqlQuery.record();
		}
		else
			bRet = false;
	}

	return bRet;
}

bool SYDBMgr::QueryExamPassNum(int userid, int passScore, QSqlRecord & record)
{
	const QString sql = QString("SELECT count(id) AS PassCount FROM %1 WHERE userid = %2 AND score >= %3")
		                .arg(m_tableNames[DT_ExamMissionList]).arg(userid).arg(passScore);
	
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
	if (bRet)
	{
		if (m_sqlQuery.next())
		{
			record = m_sqlQuery.record();
		}
		else
			bRet = false;
	}

	return bRet;
}
bool SYDBMgr::QueryQuestion(const QString& title, QSqlRecord & record)
{
	const QString sql = QString("select * from %1 where title='%2'")
		.arg(m_tableNames[DT_Question])
		.arg(title);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
	if (bRet)
	{
		if (m_sqlQuery.next())
		{
			record = m_sqlQuery.record();
		}
		else
			bRet = false;
	}

	return bRet;
}


bool SYDBMgr::QueryQuestionsNumber(int& number)
{
	const QString sql = QString("select * from %1 order by id").arg(m_tableNames[DT_Question]);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
	while (m_sqlQuery.next())
	{
		number++;
	}

	return bRet;
}

bool SYDBMgr::QueryQuestion(int id, QSqlRecord & record)
{
	const QString sql = QString("select * from %1 where id='%2'")
		.arg(m_tableNames[DT_Question])
		.arg(id);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
	if (bRet)
	{
		if (m_sqlQuery.next())
		{
			record = m_sqlQuery.record();
		}
		else
			bRet = false;
	}

	return bRet;
}

bool SYDBMgr::QueryUserInfo(int userId,QSqlRecord & record)
{
	const QString sql = QString("select * from %1 where id=%2")
						.arg(m_tableNames[DT_UserInfo])
						.arg(userId);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet,m_sqlQuery);
	if(bRet)
	{
		if(m_sqlQuery.next())
			record = m_sqlQuery.record();
		else
			bRet = false;
	}
	return bRet;
}

bool SYDBMgr::QueryAllUserInfo(std::vector<QSqlRecord>& records)
{
	const QString sql = QString("select * from %1 where groupId>0")
		.arg(m_tableNames[DT_UserInfo]);

	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
	if(bRet)
	{
		while(m_sqlQuery.next()){
			records.push_back(m_sqlQuery.record());
		}
	}

	return bRet;
}


//////

bool SYDBMgr::QueryPaperInfo(int Type, QVector<QSqlRecord>&result)   //type指定读取的试题类型
{

	QString sql1 = QString("select * from %1 where type= %2 order by id desc  limit 100").arg("examquestionlib").arg(Type);

	bool bRet = SYDBMgr::Instance()->Query(sql1, result);


	return bRet;
}


bool SYDBMgr::QueryPaperInfo(QString paperName, int &paperId,int &num,int &time)
{

	QString tableName = m_tableNames[DT_ExamPaperList];
	QString sql = QString("select * from %1 where paperName = '%2' order by id ").arg(tableName).arg(paperName);
	bool ok = m_sqlQuery.exec(sql);
	if (ok)
	{
		while (m_sqlQuery.next())
		{
			QSqlRecord record = m_sqlQuery.record();
			paperId = record.value("id").toInt();
			num = record.value("questionNum").toInt();
			QString strTime = record.value("examTotalTime").toString();
			time =strTime.split(":")[0].toInt();
		}
		
		return true;

	}
	else
		paperId = -1;
	return false;
}


void SYDBMgr::QueryPaperInfo(QString paperName, PaperInfoDetail ** t_paperInfoDetail, QVector<QSqlRecord> &records)
{
 	QString tableName = m_tableNames[DT_ExamPaperList];
	QString sql = QString("select * from %1 where paperName = '%2' order by id ").arg(tableName).arg(paperName);
	bool ok= m_sqlQuery.exec(sql);
	if (ok &m_sqlQuery.next())
	{	
		QSqlRecord record = m_sqlQuery.record();
		int paperId = record.value("id").toInt();

		QString paperName = record.value("paperName").toString();
		QString paperRank = record.value("paperRank").toString();
		QString questionNums = record.value("questionNum").toString();;
		QString createPaperTime = record.value("createDatetime").toString();
		QString examTime = record.value("examTotalTime").toString();
		QString creator = record.value("creator").toString();
		QString examScore = record.value("examTotalScore").toString();

		(*t_paperInfoDetail) = new  PaperInfoDetail(paperName, examTime,paperRank, questionNums, createPaperTime, creator, examScore);
		
		QVector<int> questionIds;
		tableName = m_tableNames[DT_ExamPaperDetail];
		sql = QString("select * from %1 where paperid=%2 order by recordid").arg(tableName).arg(paperId);
		ok=m_sqlQuery.exec(sql);

		while (m_sqlQuery.next())
		{
			QSqlRecord record = m_sqlQuery.record();
			int questionid = record.value("questionid").toInt();
			questionIds.push_back(questionid);
		}
		tableName = m_tableNames[DT_Question];
		for (int i = 0; i < questionIds.size(); i++)
		{
			int questionId = questionIds[i];
			sql = QString("select * from %1 where id=%2").arg(tableName).arg(questionId);
			ok=m_sqlQuery.exec(sql);
			ok=m_sqlQuery.next();
			if (ok)
			{
				QSqlRecord record = m_sqlQuery.record();
				records.push_back(record);
			}
		}

	}

}

bool SYDBMgr::AssignPapersToGroups(QVector<QString>paperNames, QVector<QString>groups, QString assignor)
{
	QString tableName = m_tableNames[DT_Group];
	bool rbt;
	for (size_t i = 0; i < groups.size(); i++)
	{
		int addCoureseNum = paperNames.size();
		QString sql = QString("update %1 set coursenum= coursenum+%2 where name = '%3'").arg(tableName).arg(addCoureseNum).arg(groups[i]);
		rbt=m_sqlQuery.exec(sql);
		CheckQuery(rbt, m_sqlQuery);
		tableName = m_tableNames[DT_UserInfo];
		sql = QString("select * from %1 where groupName='%2' order by id").arg(tableName).arg(groups[i]);
		rbt = m_sqlQuery.exec(sql);
		CheckQuery(rbt, m_sqlQuery);
		QVector<QString> realNames;
		while (m_sqlQuery.next())
		{
			QSqlRecord record = m_sqlQuery.record();
			QString realName = record.value("realName").toString();
			realNames.push_back(realName);
		}
		AssignPapersToUsers(paperNames, realNames, assignor);

	}
	
	return rbt;

}


bool SYDBMgr:: AssignPapersToUsers(QVector<QString>paperNames, QVector<QString>userNames,QString assignor)
{

	QString tableName = m_tableNames[DT_ExamPaperList];
	bool ok;
	for (std::size_t i = 0; i < paperNames.size(); i++)
	{
		QString paperName = paperNames[i];
		bool valivator = true;
		QString sql = QString("begin");
		ok= m_sqlQuery.exec(sql);
		valivator = ok&valivator;
		CheckQuery(ok, m_sqlQuery);
		sql = QString("select * from %1 where paperName = '%2' order by id").arg(tableName).arg(paperName);
		 ok = m_sqlQuery.exec(sql);
		valivator = ok&valivator;
		CheckQuery(ok, m_sqlQuery);
		if (ok &m_sqlQuery.next())
		{
				int paperId = m_sqlQuery.record().value("id").toInt();
				QString strTime = m_sqlQuery.record().value("examTotalTime").toString();
				int examTime = strTime.split(':')[0].toInt();  //minutes,分钟
				int questionNum = m_sqlQuery.record().value("questionNum").toInt();

				for (std::size_t j = 0; j < userNames.size(); j++)
				{
					QString userName = userNames[j];
					QString tableName = m_tableNames[DT_UserInfo];
					QString sql = QString("select * from %1 where realName = '%2' order by id").arg(tableName).arg(userName);
					bool ok = m_sqlQuery.exec(sql);
					valivator = ok&valivator;
					CheckQuery(ok, m_sqlQuery);
					if (m_sqlQuery.next())
					{
						int userId = m_sqlQuery.record().value("id").toInt();
						tableName = m_tableNames[DT_ExamMissionList];

						//need to confirm mission whether exists
						sql = QString("select * from %1 where userid=%2 and paperid=%3").arg(tableName).arg(userId).arg(paperId);
						ok = m_sqlQuery.exec(sql);
						CheckQuery(ok, m_sqlQuery);
						if (m_sqlQuery.next())
						{
							continue;
						}
				
						sql = QString("insert into %1 (userid,paperid,papername,paperquestionnum,paperassignor,examtotaltime) values(%2,%3,'%4',%5,'%6',%7)").arg(tableName)
							.arg(userId).arg(paperId).arg(paperName).arg(questionNum).arg(assignor).arg(examTime);
						ok = m_sqlQuery.exec(sql);
						valivator = ok&valivator;
						CheckQuery(ok, m_sqlQuery);
						if (valivator)
						{
							ok = m_sqlQuery.exec("commit");
						}
						else
						{
							ok = m_sqlQuery.exec("rollback");

						}
					}

			}
	   }
	
	}
		
	return ok;
}




bool SYDBMgr::FromExcelUpLoadPaperInfo(QVector<QVector<QString>> paperQuestions)
{

	QString tableName = m_tableNames[DT_Question];
	QString sql;
	bool ok;
	

	sql = QString("begin");
    ok = m_sqlQuery.exec(sql);
	CheckQuery(ok, m_sqlQuery);

//	sql = QString("set names gbk;");
//    	ok = m_sqlQuery.exec(sql);
	CheckQuery(ok, m_sqlQuery);
	for (std::size_t i = 0; i < paperQuestions.size(); i++)
	{
		QString sql = QString("insert into %1 (opta,optb,optc,optd,opte,title,answer,type) values('%2','%3','%4','%5','%6','%7','%8',%9)")
			.arg(tableName)
			.arg(paperQuestions[i][0])
			.arg(paperQuestions[i][1])
			.arg(paperQuestions[i][2])
 			.arg(paperQuestions[i][3])
			.arg(paperQuestions[i][4])
			.arg(paperQuestions[i][5])
			.arg(paperQuestions[i][6])
			.arg(paperQuestions[i][7].toInt());
		
		bool bRet=m_sqlQuery.exec(sql);
		CheckQuery(bRet, m_sqlQuery);
		ok = ok&bRet;
	}
	if (ok)
	{
		ok = m_sqlQuery.exec("commit");
	}
	else
	{
		ok = m_sqlQuery.exec("rollback");
		
	}
	return ok;
}


bool SYDBMgr::QueryPaperExist(QString paperName)
{
	DatabaseTable dt = DT_ExamPaperList;
	QString tableName = m_tableNames[dt];
	QString sql = QString("select * from %1 where paperName='%2'").arg(tableName).arg(paperName);
	bool ret = m_sqlQuery.exec(sql);
	if (ret&&m_sqlQuery.next() == true)
	{
		return true;
	}
	return false;
}

//插入数据
bool SYDBMgr::CreatePaperInfo(std::vector<QString> paperInfo, QVector<QSqlRecord>  records,QString pre_paperName)
{
	DatabaseTable dt = DT_ExamPaperList;
	QString tableName = m_tableNames[dt];

	//验证试卷是否存在
	if (pre_paperName == "")
	{
		bool ok= QueryPaperExist(paperInfo[0]);
		if (ok)
			return !ok;
	}
	if (pre_paperName.size() > 0)
	{
		DeletePaperInfo(pre_paperName);
	}	

	bool valivator = true;
	if (Open())
	{
		bool ok =m_sqlQuery.exec(QString("begin"));  //执行事务

		m_sqlQuery.prepare(QString("insert into %1 (paperName,paperRank,questionNum,createDateTime,creator,examTotalTime,examTotalScore) values \
								   		(:paperName,:paperRank,:questionNum, now(),:creator,:examTotalTime,:examTotalScore)").arg(tableName));
		m_sqlQuery.bindValue(":paperName", paperInfo[0]);
		m_sqlQuery.bindValue(":paperRank", paperInfo[1]);
		m_sqlQuery.bindValue(":questionNum", paperInfo[2].toInt());
		m_sqlQuery.bindValue(":creator", paperInfo[3]);
		m_sqlQuery.bindValue(":examTotalTime", paperInfo[4]);
		m_sqlQuery.bindValue(":examTotalScore", paperInfo[5].toInt());

		valivator = ok&valivator;
		ok= m_sqlQuery.exec();
		valivator = ok&valivator;
		if (ok)
		{
			QString sql = QString("select max(id) id from %1").arg(tableName);
			ok=m_sqlQuery.exec(sql);
			valivator = ok&valivator;

			ok=m_sqlQuery.next();
			valivator = ok&valivator;

			int paperId = m_sqlQuery.record().value("id").toInt();
			
			dt = DatabaseTable::DT_ExamPaperDetail;
			QString tableName = m_tableNames[dt];
			
			for (int i = 0; i < records.size(); i++)
			{
				QSqlRecord result= records[i];
				int questionId = result.value("id").toInt();
				
				QString sql = QString("insert into %1 (paperid,questionid) values (:paperid,:questionid)").arg(tableName);
				m_sqlQuery.prepare(sql);
				m_sqlQuery.bindValue(0, paperId);
				m_sqlQuery.bindValue(1, questionId);
				ok=m_sqlQuery.exec();
				valivator = ok&valivator;
			}
			if (valivator)
			{
				ok=m_sqlQuery.exec(QString("commit"));  //操作记录提交
				return ok;
			}	
		}
		ok=m_sqlQuery.exec(QString("rollback"));  //操作记录提交
		return ok;
	}
	
}


void SYDBMgr::DeletePaperInfo(QString paperName)
{
	bool valivator = true;
	bool ok = m_sqlQuery.exec(QString("begin"));
	valivator = valivator&ok;

	QString tableName = m_tableNames[DatabaseTable::DT_ExamPaperList];

	QString sql = QString("select * from %1 where paperName= '%2'").arg(tableName).arg(paperName);
	ok = m_sqlQuery.exec(sql);
	int paperId;
	if (ok)
	{
		QSqlRecord record;
		while(m_sqlQuery.next())
			record = m_sqlQuery.record();
		paperId = record.value("id").toInt();
	}
	valivator = valivator&ok;
	//删除表paperlist中相关记录
	 sql = QString("delete from %1 where paperName='%2'").arg(tableName).arg(paperName);

	ok = m_sqlQuery.exec(sql);
	valivator = valivator&ok;

	//删除表paperdetail相关记录
	tableName = m_tableNames[DatabaseTable::DT_ExamPaperDetail];

	sql = QString("delete  from %1 where paperid= %2").arg(tableName).arg(paperId);
	ok = m_sqlQuery.exec(sql);
	valivator = ok&valivator;

	if (valivator)
	{
		ok=m_sqlQuery.exec(QString("commit"));

		return;
	}
	ok = m_sqlQuery.exec(QString("rollback"));

}
int SYDBMgr::QueryNumberOfUser()
{
	const QString sql = QString("select count(*) n from %1")
		.arg(m_tableNames[DT_UserInfo]);

	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
	if(bRet)
	{
		if(m_sqlQuery.next())
			return m_sqlQuery.record().value(0).toInt();
	}

	return 0;
}

bool SYDBMgr::QueryUserInGroup(int groupId, std::vector<QSqlRecord> & records)
{
	const QString sql = QString("select * from %1 where groupId=%2")
							.arg(m_tableNames[DT_UserInfo])
							.arg(groupId);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet,m_sqlQuery);
	if(bRet)
	{
		bRet = false;
		while(m_sqlQuery.next())
		{
			records.push_back(m_sqlQuery.record());
			bRet = true;
		}
	}
	return bRet;
}

bool SYDBMgr::QueryNoGroupUserInfo(std::vector<QSqlRecord> & records)
{
	int permission = static_cast<int>(UP_Student);
	const QString sql = QString("select * from %1 where permission = %2 and (groupId < 0 or groupId is null)")
								.arg(m_tableNames[DT_UserInfo])
								.arg(permission);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet,m_sqlQuery);
	if(bRet)
	{
		bRet = false;
		while(m_sqlQuery.next())
		{
			records.push_back(m_sqlQuery.record());
			bRet = true;
		}
	}
	return bRet;
}

bool SYDBMgr::QueryAllManager(QVector<QSqlRecord> & records)
{
	int p1 = static_cast<int>(UP_Teacher);
	int p2 = static_cast<int>(UP_SuperManager);
	const QString sql = QString("select * from %1 where permission=%2 or permission=%3")
						.arg(m_tableNames[DT_UserInfo])
						.arg(p1)
						.arg(p2);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet,m_sqlQuery);
	
	bRet = false;
	while(m_sqlQuery.next())
	{
		records.push_back(m_sqlQuery.record());
		bRet = true;
	}

	return bRet;
}

bool SYDBMgr::QueryClassInfo(int classId, QSqlRecord& record)
{
	const QString sql = QString("select * from %1 where id=%2")
		.arg(m_tableNames[DT_Class])
		.arg(classId);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
	if(bRet)
	{
		if(m_sqlQuery.next())
			record = m_sqlQuery.record();
		else
			bRet = false;
	}
	return bRet;
}

bool SYDBMgr::QueryAllGroupInfo(QVector<QSqlRecord> & records)
{
	const static QString sql = QString("select * from %1").arg(m_tableNames[DT_Group]);
	bool bRet = m_sqlQuery.exec(sql);
	if(bRet)
	{
		bRet = false;
		while(m_sqlQuery.next())
		{
			records.push_back(m_sqlQuery.record());
			bRet = true;
		}
	}
	m_sqlQuery.clear();
	return bRet;
}

bool SYDBMgr::QueryGroupInfo(int adminId,QVector<QSqlRecord>& records)
{
	const QString sql = QString("select * from %1 where adminId=%2")
						.arg(m_tableNames[DT_Group])
						.arg(adminId);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet,m_sqlQuery);

	bRet = false;
	while(m_sqlQuery.next())
	{
		records.push_back(m_sqlQuery.record());
		bRet = true;
	}

	return bRet;
}

bool SYDBMgr::QueryGroupInfo(int groupId,QSqlRecord & record)
{
	const QString sql = QString("select * from %1 where id=%2")
							.arg(m_tableNames[DT_Group])
							.arg(groupId);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet,m_sqlQuery);
	if(bRet)
	{
		if(m_sqlQuery.next())
			record = m_sqlQuery.record();
		else
			bRet = false;
	}
	return bRet;
}

bool SYDBMgr::QueryGroupInfo(const QString& groupName,QSqlRecord & record)
{
	const QString sql = QString("select * from %1 where Name='%2'")
							.arg(m_tableNames[DT_Group])
							.arg(groupName);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet,m_sqlQuery);
	if(bRet)
	{
		if(m_sqlQuery.next())
			record = m_sqlQuery.record();
		else
			bRet = false;
	}
	return bRet;
}

bool SYDBMgr::QueryFullScoreInfo(int scoreId,QSqlRecord & record)
{
	const QString sql = QString("select * from %1 where id = %2")
						.arg(m_tableNames[DT_Score])
						.arg(scoreId);

	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet , m_sqlQuery);
	if(bRet)
	{
		if(m_sqlQuery.next())
			record = m_sqlQuery.record();
		else
			bRet = false;
	}
	return bRet;
}

int SYDBMgr::QueryTotalTrainTime(int userId)
{
	int time = 0;
	const QString sql2 = QString("select sum(costTime) from %1 where userId = %2")
		.arg(m_tableNames[DT_Score])
		.arg(userId);

	bool bRet = m_sqlQuery.exec(sql2);
	CheckQuery(bRet, m_sqlQuery);
	if(bRet){
		if(m_sqlQuery.next()){
			time = m_sqlQuery.record().value(0).toInt();
		}
	}
	
	return time;
}

bool SYDBMgr::QueryBasicTrainInfo(int userId, int& totalTime, int& simulationTrainTimes, int& skillingTrainTimes, int& surgeryTrainTimes)
{
	const QString sql1 = QString("select trainTypeCode,count(trainTypeCode) from %1 where userId = %2 group by trainTypeCode")
		.arg(m_tableNames[DT_Score])
		.arg(userId);

	bool bRet = m_sqlQuery.exec(sql1);
	CheckQuery(bRet, m_sqlQuery);
	if(bRet){
		//set times
		QSqlRecord record;
		simulationTrainTimes = 0;
		skillingTrainTimes = 0;
		surgeryTrainTimes = 0;

		if(m_sqlQuery.size() > 0){
			while(m_sqlQuery.next()){
				record = m_sqlQuery.record();
				int times = record.value(1).toInt();	//times
				switch(record.value(0).toInt())			//type code
				{
				case 0:
					simulationTrainTimes = times;
					break;
				case 1:
					skillingTrainTimes = times;
					break;
				case 2:
					surgeryTrainTimes = times;
					break;
				}
			}
		}

// 		const QString sql2 = QString("select sum(costTime) from %1 where userId = %2")
// 			.arg(m_tableNames[DT_Score])
// 			.arg(userId);
// 		bRet = m_sqlQuery.exec(sql2);
// 		CheckQuery(bRet, m_sqlQuery);
// 
// 		if(bRet){
// 			totalTime = 0;
// 			if(m_sqlQuery.next()){
// 				record = m_sqlQuery.record();
// 				totalTime = record.value(0).toInt();
// 			}
// 		}
// 		else{
// 			simulationTrainTimes = 0;
// 			skillingTrainTimes = 0;
// 			surgeryTrainTimes = 0;
// 		}

		totalTime = QueryTotalTrainTime(userId);
	}

	return bRet;
}


bool SYDBMgr::QueryUserTrainIsLock(int userId,QString trainCode)
{
	QString tableName = m_tableNames[DT_UserTrainLock];
	QString sql = QString("select * from %1 where userId=%2 and trainCode='%3'").arg(tableName).arg(userId).arg(trainCode);

	bool ret = m_sqlQuery.exec(sql);
	CheckQuery(ret, m_sqlQuery);
	if (m_sqlQuery.next())
	{
		return true;
	}
	else
	{
		return false;
	}

}


bool SYDBMgr::UpdateUserSkillTrainLockState(int userId, QString trainCode)
{
	QString tableName = m_tableNames[DT_UserTrainLock];
	QString sql = QString("insert into %1 (userId,trainCode) values(%2,'%3')").arg(tableName).arg(userId).arg(trainCode);

	bool ret = m_sqlQuery.exec(sql);
	CheckQuery(ret, m_sqlQuery);

	return ret;
}


bool SYDBMgr::QueryOneTrainRecords(int userId, const QString trainCode, int& score)
{
	bool rbet;
	QString sql = QString("select * from %1 where userId=%2 and trainCode='%3'").arg(m_tableNames[DT_Score]).arg(userId).arg(trainCode);

	rbet=m_sqlQuery.exec(sql);
	CheckQuery(rbet, m_sqlQuery);
	sql = QString("select max(score) score from %1 where userId=%2").arg(m_tableNames[DT_Score]).arg(userId);
	rbet=m_sqlQuery.exec(sql);
	CheckQuery(rbet, m_sqlQuery);
	if (m_sqlQuery.next())
	{
		score = m_sqlQuery.record().value("score").toInt();
	}
	else
		score = 0;

	return rbet;
}

bool SYDBMgr::QueryBasicTrainInfo(TrainType type, int& times, int& totalTime, int& nPeople)
{
	bool bRet = false;
	QString sql = QString("select count(id),sum(costTime) from %1 where trainTypeCode=%2")
		.arg(m_tableNames[DT_Score])
		.arg(type);

	bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
	if(bRet){
		if(m_sqlQuery.next()){
			QSqlRecord record = m_sqlQuery.record();
			times = record.value(0).toInt();
			totalTime = record.value(1).toInt();

			nPeople = 0;
			bRet = m_sqlQuery.exec(QString("select * from %1 where trainTypeCode=%2 group by userId").arg(m_tableNames[DT_Score]).arg(type));
			if(bRet && m_sqlQuery.next()){
				nPeople = m_sqlQuery.size();
				if(nPeople < 0)
					nPeople = 0;
			}
		}
	}
	else{
		times = 0;
		totalTime = 0;
		nPeople = 0;
	}

	return bRet;
}

bool SYDBMgr::QueryTrainTimes(TrainType type, const QString& trainCode, int& trainTimes)
{
	bool bRet = false;
	QString sql = QString("select count(id) from %1 where trainTypeCode='%2' and trainCode='%3'")
		.arg(m_tableNames[DT_Score])
		.arg(type)
		.arg(trainCode);

	trainTimes = 0;
	bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
	if(bRet){
		if(m_sqlQuery.next())
			trainTimes = m_sqlQuery.record().value(0).toInt();
	}

	return bRet;

}

bool SYDBMgr::QueryTrainDataDistribution(TrainType type, std::vector<QSqlRecord>& records)
{
	bool bRet = false;

	QString sql = QString("select trainTypeName,trainName,count(id) times,sum(costTime) totalTime from %1 where trainTypeCode=%2 group by trainCode")
		.arg(m_tableNames[DT_Score])
		.arg(type);

	bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
	if(bRet){
		while(m_sqlQuery.next())
			records.push_back(m_sqlQuery.record());
	}

	return bRet;
}

int SYDBMgr::QueryNumberOfCompletedTrain(int userId)
{
	const QString sql = QString("select count(B.trainCode) from		\
								(select DISTINCT trainCode,trainTypeCode from %1 where userId=%2) B")
		.arg(m_tableNames[DT_Score])
		.arg(userId);

	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
	if(bRet){
		if(m_sqlQuery.next())
			return m_sqlQuery.record().value(0).toInt();
		else
			return 0;
	}
	else
		return -1;
}

bool SYDBMgr::QueryAllScoreRecordOrderByDateDescend(int userId, std::vector<QSqlRecord>& records)
{
	const QString sql = QString("select * from %1 where userId=%2 order by beginTime desc")
		.arg(m_tableNames[DT_Score])
		.arg(userId);

	bool bRet = m_sqlQuery.exec(sql);
	if(bRet){
		while(m_sqlQuery.next())
			records.push_back(m_sqlQuery.record());
	}

	return bRet;
}

bool SYDBMgr::QueryAllScoreRecordOrderByDateDescend(TrainType type, std::vector<QSqlRecord>& records, bool needExtraUserInfo)
{
	bool bRet = false;

	const QString sql1 = QString("select *,id scoreId from %1 where trainTypeCode=%2 order by beginTime desc")
		.arg(m_tableNames[DT_Score])
		.arg(type);
	const QString sql2 = QString("select *,B.id scoreId from %1 A,%2 B where trainTypeCode=%3 and A.id=B.userId order by beginTime desc")
		.arg(m_tableNames[DT_UserInfo])
		.arg(m_tableNames[DT_Score])
		.arg(type);

	if(needExtraUserInfo)
		bRet = m_sqlQuery.exec(sql2);
	else
		bRet = m_sqlQuery.exec(sql1);

	CheckQuery(bRet, m_sqlQuery);
	if(bRet){
		while(m_sqlQuery.next())
			records.push_back(m_sqlQuery.record());
	}

	return bRet;
}

bool SYDBMgr::QueryAllUserTrainTimesOrderByDescend(int trainTypeCode, std::vector<QSqlRecord>& records)
{
	const QString sql = QString("select userId,count(userId) times from %1 where trainTypeCode=%2 group by userId order by times desc")
		.arg(m_tableNames[DT_Score])
		.arg(trainTypeCode);

	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
	if(bRet){
		while(m_sqlQuery.next())
			records.push_back(m_sqlQuery.record());
	}

	return bRet;
}

bool SYDBMgr::QueryAllUserTrainTimesOrderByDescend(std::vector<QSqlRecord>& records)
{
	const QString sql = QString("select userId,count(userId) times from %1 group by userId order by times desc")
		.arg(m_tableNames[DT_Score]);

	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
	if(bRet){
		while(m_sqlQuery.next())
			records.push_back(m_sqlQuery.record());
	}

	return bRet;
}

bool SYDBMgr::QueryAvgScoreRankInfo(std::vector<QSqlRecord>& records)
{
	const QString sql = QString("select userId , avg(score) score, count(userId) times from %1 group by userId order by score desc , times desc")
		.arg(m_tableNames[DT_Score]);

	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);

	if(bRet){
		while(m_sqlQuery.next())
			records.push_back(m_sqlQuery.record());
	}

	return bRet;
}

bool SYDBMgr::QueryAvgScoreRankInfo(int trainType, std::vector<QSqlRecord>& records, bool needExtraUserInfo /* = false */)
{
	const QString sql1 = QString("select userId ,realName,groupName, avg(score) score, count(userId) times from %1 A,%2 B where trainTypeCode=%3 and A.id=B.userId group by userId order by score desc , times desc")
		.arg(m_tableNames[DT_UserInfo])
		.arg(m_tableNames[DT_Score])
		.arg(trainType);

	const QString sql2 = QString("select userId , avg(score) score, count(userId) times from %1 where trainTypeCode=%2 group by userId order by score desc , times desc")
		.arg(m_tableNames[DT_Score])
		.arg(trainType);

	bool bRet;
	if(needExtraUserInfo)
		bRet = m_sqlQuery.exec(sql1);
	else
		bRet = m_sqlQuery.exec(sql2);


	CheckQuery(bRet, m_sqlQuery);

	if(bRet){
		while(m_sqlQuery.next())
			records.push_back(m_sqlQuery.record());
	}

	return bRet;
}

bool SYDBMgr::QueryAvgScoreRankInfo(int trainType, const QString& trainCode, std::vector<QSqlRecord>& records, bool needExtraUserInfo /* = false */)
{
	const QString sql1 = QString("select userId ,realName,groupName, avg(score) score, count(userId) times \
								 from %1 A,%2 B \
								 where trainTypeCode=%3 and trainCode='%4' and A.id=B.userId \
								 group by userId \
								 order by score desc , times desc")
		.arg(m_tableNames[DT_UserInfo])
		.arg(m_tableNames[DT_Score])
		.arg(trainType)
		.arg(trainCode);

	const QString sql2 = QString("select userId , avg(score) score, count(userId) times \
								 from %1 \
								 where trainTypeCode=%2 and trainCode='%3'\
								 group by userId \
								 order by score desc , times desc")
		.arg(m_tableNames[DT_Score])
		.arg(trainType)
		.arg(trainCode);

	bool bRet;
	if(needExtraUserInfo)
		bRet = m_sqlQuery.exec(sql1);
	else
		bRet = m_sqlQuery.exec(sql2);


	CheckQuery(bRet, m_sqlQuery);

	if(bRet){
		while(m_sqlQuery.next())
			records.push_back(m_sqlQuery.record());
	}

	return bRet;
}

bool SYDBMgr::QueryScoreRankInfo(int trainType, const QString& trainCode, std::vector<QSqlRecord>& records, bool needExtraUserInfo /* = false */)
{
	const QString sql1 = QString("select userId ,realName,groupName, score \
								 from %1 A,%2 B \
								 where trainTypeCode=%3 and trainCode='%4' and A.id=B.userId \
								 order by score desc")
		.arg(m_tableNames[DT_UserInfo])
		.arg(m_tableNames[DT_Score])
		.arg(trainType)
		.arg(trainCode);

	const QString sql2 = QString("select userId , score \
								 from %1 \
								 where trainTypeCode=%2 and trainCode='%3'\
								 order by score desc")
		.arg(m_tableNames[DT_Score])
		.arg(trainType)
		.arg(trainCode);

	bool bRet;
	if(needExtraUserInfo)
		bRet = m_sqlQuery.exec(sql1);
	else
		bRet = m_sqlQuery.exec(sql2);


	CheckQuery(bRet, m_sqlQuery);

	if(bRet){
		while(m_sqlQuery.next())
			records.push_back(m_sqlQuery.record());
	}

	return bRet;
}

bool SYDBMgr::QueryTrainCountRank(TrainType type, std::vector<QSqlRecord>& records)
{
	const QString sql = QString("select A.realName name,count(userId) times from %1 A,%2 B where trainTypeCode=%3 and A.id=B.userId group by userId order by times desc")
		.arg(m_tableNames[DT_UserInfo])
		.arg(m_tableNames[DT_Score])
		.arg(type);

	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);

	if(bRet){
		while(m_sqlQuery.next())
			records.push_back(m_sqlQuery.record());
	}

	return bRet;
}

bool SYDBMgr::QueryTrainTimeRank(TrainType type, std::vector<QSqlRecord>& records)
{
	const QString sql = QString("select A.realName name,sum(costTime) time from %1 A,%2 B where trainTypeCode=%3 and A.id=B.userId group by userId order by time desc")
		.arg(m_tableNames[DT_UserInfo])
		.arg(m_tableNames[DT_Score])
		.arg(type);

	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);

	if(bRet){
		while(m_sqlQuery.next())
			records.push_back(m_sqlQuery.record());
	}

	return bRet;

}

bool SYDBMgr::QueryClampPeaTrainCountRank(std::vector<QSqlRecord>& records)
{
	const QString sql = QString("select A.realName name,count(userId) times from %1 A,%2 B where trainTypeCode=%3 and A.id=B.userId and trainCode='026'group by userId order by times desc")
		.arg(m_tableNames[DT_UserInfo])
		.arg(m_tableNames[DT_Score])
		.arg(TT_EntityTrain);

	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);

	if(bRet){
		while(m_sqlQuery.next())
			records.push_back(m_sqlQuery.record());
	}

	return bRet;
}

bool SYDBMgr::QueryAllScoreDetail(std::vector<QSqlRecord>& records)
{
	const QString sql = QString("select * from %1")
		.arg(m_tableNames[DT_ScoreDetail]);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);

	bRet = false;
	while(m_sqlQuery.next())
	{
		records.push_back(m_sqlQuery.record());
		bRet = true;
	}

	return bRet;
}

bool SYDBMgr::QueryScoreDetailByTrainType(int trainType, std::vector<QSqlRecord>& records)
{
	const QString sql = QString("select * from %1 A,%2 B where A.trainTypeCode=%3 and A.id=B.scoreId")
		.arg(m_tableNames[DT_Score])
		.arg(m_tableNames[DT_ScoreDetail])
		.arg(trainType);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);

	bRet = false;
	while(m_sqlQuery.next())
	{
		records.push_back(m_sqlQuery.record());
		bRet = true;
	}

	return bRet;
}

bool SYDBMgr::QueryScoreDetail(int scoreId,QVector<QSqlRecord> & records)
{
	const QString sql = QString("select * from %1 where scoreId=%2 order by scoreCode asc")
						.arg(m_tableNames[DT_ScoreDetail])
						.arg(scoreId);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet , m_sqlQuery);

	bRet = false;
	while(m_sqlQuery.next())
	{
		records.push_back(m_sqlQuery.record());
		bRet = true;
	}

	return bRet;
}

bool SYDBMgr::QueryHistoryScoreDetail(int userId, const QString& trainTypeCode, const QString& trainCode, QVector<QSqlRecord>& records)
{
	const QString sql = QString("select scoreId,scoreTableCode,scoreCode from %1 t1,%2 t2 where userId=%3 and trainTypeCode='%4' and trainCode='%5' and t1.id=t2.scoreId")
		.arg(m_tableNames[DT_Score])
		.arg(m_tableNames[DT_ScoreDetail])
		.arg(userId)
		.arg(trainTypeCode)
		.arg(trainCode);

	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);

	bRet = false;
	while(m_sqlQuery.next())
	{
		records.push_back(m_sqlQuery.record());
		bRet = true;
	}

	return bRet;
}

bool SYDBMgr::QueryScreenshots(int scoreId, QVector<QSqlRecord> & records)
{
	const QString sql = QString("select * from %1 where scoreId=%2")
		.arg(m_tableNames[DT_Screenshot])
		.arg(scoreId);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);

	bRet = false;
	while(m_sqlQuery.next())
	{
		records.push_back(m_sqlQuery.record());
		bRet = true;
	}
	return bRet;
}

bool SYDBMgr::QueryVideoFiles(int scoreId, QVector<QSqlRecord> & records)
{
	const QString sql = QString("select * from %1 where scoreId=%2")
		.arg(m_tableNames[DT_VideoFile])
		.arg(scoreId);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);

	bRet = false;
	while(m_sqlQuery.next())
	{
		records.push_back(m_sqlQuery.record());
		bRet = true;
	}
	return bRet;
}

bool SYDBMgr::GetGroupNameFromId(QString& GroupName,  int& GroupId)
{
	QString groupTable=SYDBMgr::Instance()->GetTableName(SYDBMgr::DT_Group);
	QString userInfoTable=SYDBMgr::Instance()->GetTableName(SYDBMgr::DT_UserInfo);
	QString sql=QString("select Name,%1.id from %1,%2 where %1.id = %2.groupId and %2.groupId = %3")
							.arg(groupTable)
							.arg(userInfoTable)
							.arg(GroupId);
	bool bRet=m_sqlQuery.exec(sql);
	//GroupName = m_sqlQuery.value(0).toString();
	if(m_sqlQuery.next())
		GroupName = m_sqlQuery.value(0).toString();
	else
		GroupName = QString::fromLocal8Bit("~无组织");
	
	return bRet;
}

bool SYDBMgr::GetAdminNameFromId(QString& AdminName, int& GroupId)
{
	int Permission_Admin=2;
	int Permission_SuperAdmin=3;
	QString groupTable=SYDBMgr::Instance()->GetTableName(SYDBMgr::DT_Group);
	QString userInfoTable=SYDBMgr::Instance()->GetTableName(SYDBMgr::DT_UserInfo);
	QString sql=QString("select userName from %1,%5 where %1.groupId=%2 and (%1.Permission = %3 or %1.Permission = %4) and %1.groupId = %5.id and %1.id = %5.adminId")
						.arg(userInfoTable)
						.arg(GroupId)
						.arg(Permission_Admin)
						.arg(Permission_SuperAdmin)
						.arg(groupTable);
	bool bRet=m_sqlQuery.exec(sql);
	//AdminName=m_sqlQuery.value(0).toString();
	if(m_sqlQuery.next())
		AdminName = m_sqlQuery.value(0).toString();
	else
		AdminName = QString::fromLocal8Bit("未知管理员");
	
	return bRet;
}
bool SYDBMgr::Exec(const QString& sql)
{
	QSqlError error;
	
	QString   errorText;
	
	bool succed = m_sqlQuery.exec(sql);

	CheckQuery(succed, m_sqlQuery);

	if (succed == false)
	{
		error = m_sqlQuery.lastError();
		errorText = error.databaseText();
	}

	return succed;
}

int SYDBMgr::QueryStuMissionId(int userid,int paperid)
{

	const QString& tableName = m_tableNames[SYDBMgr::DatabaseTable::DT_ExamMissionList];
	if (tableName == m_errorTableName)
		return -1;

	int id = -1;
	QString sql = QString("select * from %1 where userid=%2 and paperid=%3").arg(tableName).arg(userid).arg(paperid);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
	if (bRet)
	{
		if (m_sqlQuery.next())
			id = m_sqlQuery.record().value("id").toInt();
		else
			id = 0;
	}
	return id;


}


int SYDBMgr::QueryMaxIdFromTable(DatabaseTable table)
{
	const QString& tableName = GetTableName(table);
	if(tableName == m_errorTableName)
		return -1;

	int id = -1;
	QString sql = QString("select max(id) id from %1").arg(tableName);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet,m_sqlQuery);
	if(bRet)
	{
		if(m_sqlQuery.next())
			id = m_sqlQuery.record().value("id").toInt();
		else
			id = 0;
	}
	return id;
}

int SYDBMgr::QueryMaxgroupIdFromTable(DatabaseTable table)
{
	const QString& tableName = GetTableName(table);
	if(tableName == m_errorTableName)
		return -1;

	int groupId = -1;
	QString sql = QString("select max(groupId) groupId from %1").arg(tableName);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet,m_sqlQuery);
	if(bRet)
	{
		if(m_sqlQuery.next())
			groupId = m_sqlQuery.record().value("groupId").toInt();
		else
			groupId = 1;
	}

	return groupId;
}

bool SYDBMgr::QueryAllScoreTableItem(QVector<QSqlRecord>& records)
{
	const QString sql = QString("select * from %1 ")
		.arg(m_tableNames[DT_ScoreTable]);

	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);

	bRet = false;
	while(m_sqlQuery.next())
	{
		records.push_back(m_sqlQuery.record());
		bRet = true;
	}

	return bRet;
}

bool SYDBMgr::QueryMessagesByReceiverId(int userId, std::vector<QSqlRecord>& records)
{
	const QString sql = QString("select * from %1 where receiverId=%2 order by sendTime desc")
		.arg(m_tableNames[DT_Message])
		.arg(userId);

	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);

	bRet = false;
	while(m_sqlQuery.next())
	{
		records.push_back(m_sqlQuery.record());
		bRet = true;
	}

	return bRet;
}

bool SYDBMgr::QueryMessagesBySenderId(int userId, std::vector<QSqlRecord>& records)
{
	const QString sql = QString("select * from %1 where senderId=%2 order by sendTime desc")
		.arg(m_tableNames[DT_Message])
		.arg(userId);

	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);

	bRet = false;
	while(m_sqlQuery.next())
	{
		records.push_back(m_sqlQuery.record());
		bRet = true;
	}

	return bRet;
}

bool SYDBMgr::QueryMessageReply(int messageId, std::vector<QSqlRecord>& records)
{
	const QString sql = QString("select * from %1 where messageId=%2 order by time asc")
		.arg(m_tableNames[DT_MessageReply])
		.arg(messageId);

	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);

	bRet = false;
	while(m_sqlQuery.next())
	{
		records.push_back(m_sqlQuery.record());
		bRet = true;
	}

	return bRet;
}


// int SYDBMgr::AddScoreRecord(const TrainScoreRecord& scoreRecord, const QMap<QString,QPixmap>& screenshots)
// {
// 	if(Open() == false)
// 	   return -1;
// 
// 	//有具体的分数细节时才保存成绩
// 	//if(scoreRecord.scoreItems.size())
// 	{
// 		//TODO 网络数据库时，应考虑加锁问题！
// 		int scoreId = QueryMaxIdFromTable(DT_Score);
// 		if(scoreId == -1)
// 			return -1;
// 
// 		scoreId += 1;
// 		// 	QString sql = QString("insert into %1 (%2,%3,%4,%5,%6,%7,%8, \
// 		// 											%9,%10,%11,%12,%13,%14) \
// 		// 											values(%15,%16,'%17','%18','%19','%20',%21,\
// 		// 													%22,%23,%24,%25,%26,%27)")
// 		QString costTime = QString("%1''%2\"")
// 					.arg((int)(scoreRecord.operateTime / 60),2,10,QChar('0'))
// 					.arg(((int)scoreRecord.operateTime) % 60,2,10,QChar('0'));
// 		QString sql = QString("insert into %1 values(%2,%3,'%4','%5','%6','%7',%8, \
// 							  %9,%10,%11,%12,%13,%14,'%15')")
// 							  .arg(m_tableNames[DT_Score])
// 							  .arg(scoreId).arg(scoreRecord.userId).arg(scoreRecord.trainCategoryName).arg(scoreRecord.trainName).arg(scoreRecord.beginTime).arg(costTime).arg(scoreRecord.totalScore)
// 							  .arg(scoreRecord.speed_l).arg(scoreRecord.speed_r)
// 							  .arg(scoreRecord.clipCount_l).arg(scoreRecord.clipCount_r)
// 							  .arg(scoreRecord.moveDist_l).arg(scoreRecord.moveDist_r)
// 							  .arg(QString::fromLocal8Bit(scoreRecord.videoFileName.c_str()));
// 		bool bRet = m_sqlQuery.exec(sql);
// 		CheckQuery(bRet,m_sqlQuery);
// 		if(bRet)
// 		{
// 			for(std::size_t i = 0;i < scoreRecord.scoreItems.size();++i)
// 			{
// 				int scoreDetailId = QueryMaxIdFromTable(DT_ScoreDetail);
// 				if(scoreDetailId != -1)
// 				{
// 					scoreDetailId += 1;
// // 					sql = QString("insert into %1 values(%2,%3,'%4','%5',%6,%7,%8,%9)")
// // 								.arg(m_tableNames[DT_ScoreDetail])
// // 								.arg(scoreDetailId)
// // 								.arg(scoreId)
// // 								.arg(QString::fromUtf8(scoreRecord.scoreItems[i].name.c_str()))
// // 								.arg(QString::fromUtf8(scoreRecord.scoreItems[i].description.c_str()))
// // 								.arg(scoreRecord.scoreItems[i].score)
// // 								.arg(scoreRecord.scoreItems[i].timeScore)
// // 								.arg(scoreRecord.scoreItems[i].time)
// // 								.arg(scoreRecord.scoreItems[i].times);
// 					sql = QString("insert into %1 values(%2,%3,%4,'%5','%6',%7,%8,%9,%10)")
// 						.arg(m_tableNames[DT_ScoreDetail])
// 						.arg(scoreDetailId)
// 						.arg(scoreId)
// 						.arg(0)			//为了兼容现在的操作项而新增的字段
// 						.arg(QString::fromUtf8(scoreRecord.scoreItems[i].name.c_str()))
// 						.arg(QString::fromUtf8(scoreRecord.scoreItems[i].description.c_str()))
// 						.arg(scoreRecord.scoreItems[i].score)
// 						.arg(scoreRecord.scoreItems[i].timeScore)
// 						.arg(scoreRecord.scoreItems[i].time)
// 						.arg(scoreRecord.scoreItems[i].times);
// 					bRet = m_sqlQuery.exec(sql);
// 					CheckQuery(bRet , m_sqlQuery);
// 				}
// 			}
// 
// 			//保存截图数据
// 			int reportId = 0;
// 			sql = QString("insert into %1 values(?,?,?,?,?)").arg(m_tableNames[DT_Screenshot]);
// 			for(auto itr = screenshots.begin(); itr != screenshots.end();++itr)
// 			{
// 				QBuffer imageBuffer;
// 				int pictureId = QueryMaxIdFromTable(DT_Screenshot) + 1;
// 				const auto& pixmap = itr.value();
// 				pixmap.save(&imageBuffer, "jpg");
// 
// 				m_sqlQuery.prepare(sql);
// 				m_sqlQuery.addBindValue(pictureId);
// 				m_sqlQuery.addBindValue(itr.key());
// 				m_sqlQuery.addBindValue(scoreId);
// 				m_sqlQuery.addBindValue(reportId);
// 				m_sqlQuery.addBindValue(imageBuffer.buffer());
// 				
// 				bRet = m_sqlQuery.exec();
// 				CheckQuery(bRet, m_sqlQuery);
// 			}
// 
// 			return scoreId;
// 		}
// 	}
// 
// 	return -1;
// }

int SYDBMgr::SaveReportRecord(const SYTrainReportRecord& reportRecord, const QMap<QString, QPixmap>& screenshots,const QVector<QString>& videoFiles)
{
	//1 save score record
	int scoreId = QueryMaxIdFromTable(DT_Score);
	if(scoreId == -1)
		return -1;
	scoreId += 1;

	QString sql = QString("insert into %1 (id , userId, trainTypeCode , trainTypeName , trainCode , trainName , beginTime ,costTime,\
						                   score, movespeedleft , movespeedright , closecountleft, closecountright,movedistanceleft,movedistanceright) \
										   values(%2,%3,'%4','%5','%6','%7','%8',%9,%10,%11,%12,%13,%14,%15,%16)")
												.arg(m_tableNames[DT_Score])
												.arg(scoreId).arg(reportRecord.m_userId)
												.arg(reportRecord.m_trainTypeCode).arg(reportRecord.m_trainTypeName)
												.arg(reportRecord.m_trainCode).arg(reportRecord.m_trainName)
												.arg(reportRecord.m_beginTime).arg(reportRecord.m_costedTime)
												.arg(reportRecord.m_score)
												.arg(reportRecord.m_LeftToolSpeed).arg(reportRecord.m_RightToolSpeed)//speed
												.arg(0).arg(0)//clipCount
												.arg(reportRecord.m_LeftToolMovDist).arg(reportRecord.m_RightToolMovDist);//moveDist
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);

	if(bRet){
		//2 save score item details
		
		for(const auto& scoreItemDetail : reportRecord.m_scoreItemDetails)
		{
			int scoreDetailId = QueryMaxIdFromTable(DT_ScoreDetail);
			if(scoreDetailId != -1){
				scoreDetailId += 1;
				sql = QString("insert into %1 values(%2,%3,'%4','%5')")
					.arg(m_tableNames[DT_ScoreDetail])
					.arg(scoreDetailId)
					.arg(scoreId)
					.arg(scoreItemDetail.GetScoreTableCode())
					.arg(scoreItemDetail.GetScoreCode());

				bRet = m_sqlQuery.exec(sql);
				CheckQuery(bRet, m_sqlQuery);
			}
			else{
				qDebug() << "get id of score item detail fail";
				break;
			}
		}

		//3 save screenshots
		//保存截图数据
		sql = QString("insert into %1 (name,scoreId,reportId,data) values(?,?,?,?)").arg(m_tableNames[DT_Screenshot]);
 		for(auto itr = screenshots.begin(); itr != screenshots.end(); ++itr)
 		{
			QBuffer imageBuffer;
 			//int pictureId = QueryMaxIdFromTable(DT_Screenshot) + 1;
 			const auto& image = itr.value();
 			image.save(&imageBuffer, "jpg");
 
 			bRet = m_sqlQuery.prepare(sql);
 			//m_sqlQuery.addBindValue(pictureId);
 			m_sqlQuery.addBindValue(itr.key());
 			m_sqlQuery.addBindValue(scoreId);
 			m_sqlQuery.addBindValue(scoreId);
 			m_sqlQuery.addBindValue(imageBuffer.buffer());
 
 			bRet = m_sqlQuery.exec();
 			CheckQuery(bRet, m_sqlQuery);
 		}

		//4 save video files
		for(const auto& videoFile : videoFiles){

			sql = QString("insert into %1 values(%2,'%3')")
				.arg(m_tableNames[DT_VideoFile])
				.arg(scoreId)
				.arg(videoFile);

			bRet = m_sqlQuery.exec(sql);
			CheckQuery(bRet, m_sqlQuery);
		}

		return scoreId;
	}
	
	return -1;
}

int SYDBMgr::AddUserInfo(const QString& userName,const QString& realName,const QString& password)
{
	if(userName.isEmpty() || password.isEmpty())
	{
		Message(CHS("提示"),CHS("用户名或密码不能为空！"));
		return -1;
	}

	QSqlRecord record;
	if(QueryUserInfo(userName,record))
		return -1;

	int userId = QueryMaxIdFromTable(DT_UserInfo);
	if(userId != -1)
	{
		userId += 1;
		QString sql = QString("insert into %1 (id,userName,realName,password) values(%2,'%3','%4','%5')")
			.arg(m_tableNames[DT_UserInfo])
			.arg(userId)
			.arg(userName)
			.arg(realName)
			.arg(password);
		bool bRet = m_sqlQuery.exec(sql);
		CheckQuery(bRet,m_sqlQuery);
		if(!bRet)
			return -1;
	}

	return userId;
}

bool SYDBMgr::AddUserInfo(int id,const QString& userName,const QString& realName,const QString& password)
{
	QSqlRecord record;

	if(id <= 0 )
	{
		Message(CHS("提示"),CHS("id(%1)值应为整数！").arg(id));
		return false;
	}

	if(userName.isEmpty() || password.isEmpty())
	{
		Message(CHS("提示"),CHS("用户名或密码不能为空！"));
		return false;
	}

	if(QueryUserInfo(id,record))
		return false;

	if(QueryUserInfo(userName,record))
		return false;

	QString sql = QString("insert into %1 (id,userName,realName,password) values(%2,'%3','%4','%5')")
		.arg(m_tableNames[DT_UserInfo])
		.arg(id)
		.arg(userName)
		.arg(realName)
		.arg(password);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
	
	return bRet;

}

int SYDBMgr::AddUserInfo(const QString& userName,
						 const QString& password,
						 const QString& initPassword /* = "123456" */,
						 const QString& realName /* = "" */,
						 int permission /* = 1 */)
{
	if(userName.isEmpty() || password.isEmpty())
	{
		Message(CHS("提示"),CHS("用户名或密码不能为空！"));
		return -1;
	}

	QSqlRecord record;
	if(QueryUserInfo(userName,record))
		return -1;

	int userId = QueryMaxIdFromTable(DT_UserInfo);
	if(userId != -1)
	{
		userId += 1;
		QString sql = QString("insert into %1 (id,userName,password,initPassword,realName,permission) values(%2,'%3','%4','%5','%6',%7)")
			.arg(m_tableNames[DT_UserInfo])
			.arg(userId)
			.arg(userName)
			.arg(password)
			.arg(initPassword)
			.arg(realName)
			.arg(permission);

		bool bRet = m_sqlQuery.exec(sql);
		CheckQuery(bRet,m_sqlQuery);
		if(!bRet)
			return -1;
	}

	return userId;
}

int SYDBMgr::AddGroup(const QString& groupName, const QString& ownerName, int ownerId)
{
	if (groupName == "" || ownerId <= 0)
		return -1;

	QSqlRecord record;
	if (QueryGroupInfo(groupName, record) || QueryUserInfo(ownerId, record) == false)
		return -1;

	int groupId = -1;//
	
	QString sql = QString("insert into %1 (name,ownername,foundedTime,ownerId) values('%2','%3', now() ,%4)")
						.arg(m_tableNames[DT_Group])
						.arg(groupName)
						.arg(ownerName)
						.arg(ownerId);

	bool bRet = m_sqlQuery.exec(sql);
		
	CheckQuery(bRet,m_sqlQuery);
		
	if(!bRet)
		return -1;

	m_sqlQuery.exec("select last_insert_id() as last_id");
		
	if (m_sqlQuery.next())
	{
		QSqlRecord record = m_sqlQuery.record();

		groupId = record.value("last_id").toInt();
	}
	else
	{
		return -1;
	}

	return groupId;
}

bool SYDBMgr::UpdateUserName(int userId,const QString& userName)
{
	QString sql = QString("update %1 set userName='%2' where id=%3")
					.arg(m_tableNames[DT_UserInfo])
					.arg(userName)
					.arg(userId);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet,m_sqlQuery);
	return bRet;
}

bool SYDBMgr::UpdateUserRealName(int userId,const QString& realName)
{
	QString sql = QString("update %1 set realName='%2' where id=%3")
		.arg(m_tableNames[DT_UserInfo])
		.arg(realName)
		.arg(userId);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet,m_sqlQuery);
	return bRet;
}

bool SYDBMgr::UpdateUserSex(int userId, bool isMale)
{
	//男性为0，女性为1
	int sex = isMale ? 0 : 1;
	QString sql = QString("update %1 set sex=%2 where id=%3")
		.arg(m_tableNames[DT_UserInfo])
		.arg(sex)
		.arg(userId);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
	return bRet;
}

bool SYDBMgr::UpdateUserGroupId(int userId,int userGroupId)
{
	QString sql = QString("update %1 set groupId=%2 where id=%3")
		.arg(m_tableNames[DT_UserInfo])
		.arg(userGroupId)
		.arg(userId);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet,m_sqlQuery);
	return bRet;
}

bool SYDBMgr::UpdateAllUserGroupId(int oldGroupId,int newGroupId)
{
	QString sql = QString("update %1 set groupId=%2 where groupId=%3")
		.arg(m_tableNames[DT_UserInfo])
		.arg(newGroupId)
		.arg(oldGroupId);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet,m_sqlQuery);
	return bRet;
}

bool SYDBMgr::UpdateUsersGroup(std::vector<int> & userIds, int GroupID, const QString & groupName)
{
	QString sql = QString("UPDATE %1 set groupId=%2 , groupname='%3' where id IN(").arg(m_tableNames[DT_UserInfo]).arg(GroupID).arg(groupName);

	for (int c = 0; c < (int)userIds.size(); c++)
	{
		sql += QString::number(userIds[c]);

		if (c < (int)userIds.size()-1)
		    sql += QString(",");
		else
		    sql += QString(")");
	}

	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
	return bRet;

	
}
bool SYDBMgr::UpdateUserPermission(int userId,int permission)
{
	QString sql = QString("update %1 set permission='%2' where id=%3")
		.arg(m_tableNames[DT_UserInfo])
		.arg(permission)
		.arg(userId);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
	return bRet;
}

bool SYDBMgr::UpdateUserPassword(int userId,const QString& password)
{
	QString sql = QString("update %1 set password='%2' where id=%3")
		.arg(m_tableNames[DT_UserInfo])
		.arg(password)
		.arg(userId);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet,m_sqlQuery);
	return bRet;
}

bool SYDBMgr::UpdateLoginTimes(int userId,int loginTimes)
{
	if(loginTimes < 0)
		loginTimes = 0;
	QString sql = QString("update %1 set loginTimes=%2 where id=%3")
							.arg(m_tableNames[DT_UserInfo])
							.arg(loginTimes)
							.arg(userId);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet,m_sqlQuery);
	return bRet;
}

bool SYDBMgr::UpdateFirstLoginTime(int userId, const QDateTime& dateTime)
{
	QString sql = QString("update %1 set firstLoginTime='%2' where id=%3 ")
		.arg(m_tableNames[DT_UserInfo])
		.arg(dateTime.toString("yyyy-MM-dd HH:mm:ss.zzz"))
		.arg(userId);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
	return bRet;
}

bool SYDBMgr::UpdateLastLoginTime(int userId,const QDateTime& dateTime)
{
	QString sql = QString("update %1 set lastLoginTime='%2' where id=%3 ")
							.arg(m_tableNames[DT_UserInfo])
							.arg(dateTime.toString("yyyy-MM-dd HH:mm:ss.zzz"))
							.arg(userId);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet,m_sqlQuery);
	return bRet;
}

bool SYDBMgr::UpdateTotalOnlineTime(int userId, unsigned int time)
{
	QString sql = QString("update %1 set totalOnlineTime='%2' where id=%3 ")
		.arg(m_tableNames[DT_UserInfo])
		.arg(time)
		.arg(userId);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
	return bRet;
}

// bool SYDBMgr::UpdateTotalTrainTime(int userId, unsigned int time)
// {
// 	QString sql = QString("update %1 set totalTrainTime='%2' where id=%3 ")
// 		.arg(m_tableNames[DT_UserInfo])
// 		.arg(time)
// 		.arg(userId);
// 	bool bRet = m_sqlQuery.exec(sql);
// 	CheckQuery(bRet, m_sqlQuery);
// 	return bRet;
// }

bool SYDBMgr::UpdateGroupName(int groupId,const QString& newGroupName)
{
	QString sql = QString("update %1 set name='%2' where id=%3 ")
		.arg(m_tableNames[DT_Group])
		.arg(newGroupName)
		.arg(groupId);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet,m_sqlQuery);
	return bRet;
}

bool SYDBMgr::UpdateGroupName(const QString& oldGroupName,const QString& newGroupName)
{
	QString sql = QString("update %1 set name='%2' where name='%3' ")
		.arg(m_tableNames[DT_Group])
		.arg(newGroupName)
		.arg(oldGroupName);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet,m_sqlQuery);
	return bRet;
}

bool SYDBMgr::UpdateGroupAdminId(int groupId,int adminId)
{
	QString sql = QString("update %1 set adminId=%2 where id=%3 ")
		.arg(m_tableNames[DT_Group])
		.arg(adminId)
		.arg(groupId);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet,m_sqlQuery);
	return bRet;
}

bool SYDBMgr::UpdateVisitorId()
{
	int visitorId = 0;
	int id = -2;
	QString sql = QString("update %1 set userId=%2 where userId=%3 ")
		.arg(m_tableNames[DT_Score])
		.arg(visitorId)
		.arg(id);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);

	if (bRet)
	{
		sql = QString("update %1 set userId=%2 where userId=%3 ")
			.arg(m_tableNames[DT_OperateReport])
			.arg(visitorId)
			.arg(id);
		bRet = m_sqlQuery.exec(sql);
		CheckQuery(bRet, m_sqlQuery);
	}
	return bRet;
}

bool SYDBMgr::UpdateScoreTableItem(const QString& oldScoreTableCode, const QString& curScoreTableCode,
								   const QString& oldScoreCode, const QString& curScoreCode,
								   const QString& trainName, const QString& trainCode, const QString& trainTypeCode,
								   const QString& stepName, const QString& stepCode, const QString& stepDetailedDescription, int isKeyStep,
								   const QString& scoreItemContent, const QString& scoreItemCode, int scoreItemValue,
								   const QString& scorePointDetailName, const QString& scorePointDetailCode, int scorePointDetailValue, int abilitys)
{

	QString sql = QString("update %1 set\
						  scoreTableCode='%2',scoreCode='%3',\
						  trainName='%4',trainCode='%5',trainTypeCode='%6',\
						  stepName='%7',stepCode='%8',stepDetailedDescritpion='%9',isKeyStep=%10,\
						  scoreItemContent='%11',scoreItemCode='%12',scoreItemValue=%13,\
						  scorePointDetailName='%14',scorePointDetailCode='%15',scorePointDetailValue=%16,abilitys=%17 \
						  where scoreTableCode='%18' and scoreCode='%19'")
		.arg(m_tableNames[DT_ScoreTable])
		.arg(curScoreTableCode).arg(curScoreCode)
		.arg(trainName).arg(trainCode).arg(trainTypeCode)
		.arg(stepName).arg(stepCode).arg(stepDetailedDescription).arg(isKeyStep)
		.arg(scoreItemContent).arg(scoreItemCode).arg(scoreItemValue)
		.arg(scorePointDetailName).arg(scorePointDetailCode).arg(scorePointDetailValue).arg(abilitys)
		.arg(oldScoreTableCode).arg(oldScoreCode);

	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
	
	return bRet;
}

bool SYDBMgr::UpdateMessageStatus(int messageId, int status)
{
	QString sql = QString("update %1 set status=%2  where id=%3")
		.arg(m_tableNames[DT_Message])
		.arg(status)
		.arg(messageId);

	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
	
	return bRet;
}

bool SYDBMgr::UpdateMessageReplyStatus(int messageReplyId, int status)
{
	QString sql = QString("update %1 set status=%2  where id=%3")
		.arg(m_tableNames[DT_MessageReply])
		.arg(status)
		.arg(messageReplyId);

	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);

	return bRet;
}

int SYDBMgr::UpdateQuestionBank(const QString& title, const QString& A, const QString& B, const QString& C, const QString& D, const QString& E, const QString& answer)
{
	/*QString sql = QString("update %1 set A='%2',B='%3',C='%4',D='%5',E='%6',answer='%7' where title='%8' ")
		.arg(m_tableNames[DT_QuestionBank])
		.arg(A)
		.arg(B)
		.arg(C)
		.arg(D)
		.arg(E)
		.arg(answer)
		.arg(title);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
	return bRet;*/
	return 0;
}

bool SYDBMgr::DeleteUserInfo(int userId)
{
	bool bRet = false;
	QSqlRecord record;
	if(QueryUserInfo(userId,record))
	{
		//删除有拥有的所有小组
		if(record.value("permission").toInt() > 1)
		{
			DeleteGroupAccordingAdminId(userId);
		}

		QString sql = QString("delete from %1 where id=%2")
								.arg(m_tableNames[DT_UserInfo])
								.arg(userId);
		bRet = m_sqlQuery.exec(sql);
		CheckQuery(bRet,m_sqlQuery);
	}
	
	return bRet;
}

bool SYDBMgr::DeleteUserInfo(const QString& userName)
{

	bool bRet = false;
	QSqlRecord record;
	if(QueryUserInfo(userName,record))
	{
		//删除有拥有的所有小组
		int userId = record.value("id").toInt();
		if(record.value("permission").toInt() > 1)
		{
			DeleteGroupAccordingAdminId(userId);
		}

		QString sql = QString("delete from %1 where id=%2")
								.arg(m_tableNames[DT_UserInfo])
								.arg(userId);
		bRet = m_sqlQuery.exec(sql);
		CheckQuery(bRet,m_sqlQuery);
	}

	return bRet;
}

bool SYDBMgr::DeleteGroup(int groupId)
{
	QString sql = QString("delete from %1 where id=%2")
					.arg(m_tableNames[DT_Group])
					.arg(groupId);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet,m_sqlQuery);
	if(bRet)
	{
		return UpdateAllUserGroupId(groupId,-1);
	}
	return bRet;
}

bool SYDBMgr::DeleteGroupAccordingAdminId(int adminId)
{
	//后面修改
	return true;

	bool bRet = false;

	//设置教师创建的组内学员为为分组状态
	QString sql = QString("UPDATE %1 SET groupId=0 WHERE groupId IN (SELECT id FROM %2 WHERE ownerId=%3)")
								.arg(m_tableNames[DT_UserInfo])
								.arg(m_tableNames[DT_Group])
								.arg(adminId);
	bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);

	//2 从小组表中删除属于该教师的小组
	sql = QString("DELETE FROM %1 WHERE ownerId=%2").arg(m_tableNames[DT_Group]).arg(adminId);
	bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet,m_sqlQuery);

	return bRet;
}

bool SYDBMgr::DeleteQuestionBank(const QString& title)
{
	/*
	bool bRet = false;
	QSqlRecord record;
	if (QueryQuestionBank(title, record))
	{
		QString sql = QString("delete from %1 where title='%2'")
			.arg(m_tableNames[DT_QuestionBank])
			.arg(title);
		bRet = m_sqlQuery.exec(sql);
		CheckQuery(bRet, m_sqlQuery);
	}
	return bRet;
	*/
	return false;
}

bool SYDBMgr::DeleteQuestion()
{
	bool bRet = false;
	QSqlRecord record;
	QString sql = QString("delete from %1").arg(m_tableNames[DT_Question]);
	bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
	return bRet;
}


bool SYDBMgr::DeleteQuestions(QVector<QSqlRecord>&records)
{
	bool bRet = false;
	QSqlRecord record;
	QString tableName =m_tableNames[DT_Question];
	for (std::size_t i = 0; i < records.size(); i++)
	{
		QSqlRecord record = records[i];
		int id = record.value("id").toInt();
		QString sql = QString("delete from %1 where id=%2").arg(tableName).arg(id);
		bRet = m_sqlQuery.exec(sql);
		CheckQuery(bRet, m_sqlQuery);
	}
	
	return bRet;

}


bool SYDBMgr::DeleteQuestions(int type)
{
	bool bRet = false;
	QString tableName = m_tableNames[DT_Question];

	QString sql = QString("delete from %1 where type=%2").arg(tableName).arg(type);
	bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);

	return bRet;

}

bool SYDBMgr::ModifyPaperInfo(QString &pre_paperName,QVector<QSqlRecord>&records, PaperInfoDetail *&paperInfo)
{

	bool bRet = true;
	bool valivator = true;
	QString paperInfoTableName = m_tableNames[DT_ExamPaperList];
	QString paperDetailTableName = m_tableNames[DT_ExamPaperDetail];

	QString sql = QString("set sql_safe_updates=0");
	bRet = m_sqlQuery.exec(sql);
	valivator = valivator&bRet;
	CheckQuery(bRet, m_sqlQuery);

	if (paperInfo)
	{
		QString paperName = paperInfo->paperName;
		QString examTime = paperInfo->examTime;
		//	QString questionNums = paperInfo->questionNums;
		//	QString examScore = paperInfo->examScore;
		//QString paperRank = paperInfo->paperRank;
		//QString createPaperTime = paperInfo->createPaperTime;
		//QString creator = paperInfo->creator;
		QString sql = QString("update %1 set paperName='%2',examTotalTime='%3' where paperName='%4'").arg(paperInfoTableName).arg(paperName).arg(examTime).arg(pre_paperName);
		bRet = m_sqlQuery.exec(sql);
		valivator = valivator&bRet;
		CheckQuery(bRet, m_sqlQuery);	
		pre_paperName = paperName;
	}
	
	if (records.size() == 0)
	{
		return bRet;
	}

	//事务处理
	bRet=m_sqlQuery.exec("begin");
	valivator = valivator&bRet;
	CheckQuery(bRet,m_sqlQuery);

	bRet = m_sqlQuery.exec("drop trigger if exists delt_info");
	valivator = valivator&bRet;
	CheckQuery(bRet, m_sqlQuery);

	//设置触发器
	sql = QString("create trigger delt_info  \
						  		after delete on %1 for each row  \
										update %2 set questionNum=questionNum-1 where id=old.paperid").arg(paperDetailTableName).arg(paperInfoTableName);
	bRet = m_sqlQuery.exec(sql);
	valivator = valivator&bRet;
	CheckQuery(bRet, m_sqlQuery);

	for (std::size_t i = 0; i < records.size(); i++)
	{
		QSqlRecord record = records[i];
		int id = record.value("id").toInt();
		
		QString sql = QString("delete from %1 where questionid=%2").arg(paperDetailTableName).arg(id);
		bRet = m_sqlQuery.exec(sql);
		valivator = valivator&bRet;
		CheckQuery(bRet, m_sqlQuery);
	}

	bRet = m_sqlQuery.exec("drop trigger if exists delt_info");
	valivator = valivator&bRet;
	CheckQuery(bRet, m_sqlQuery);

	if (valivator)
	{
		bRet=m_sqlQuery.exec("commit");
		CheckQuery(bRet, m_sqlQuery);
	}
	else
	{
		bRet=m_sqlQuery.exec("rollback");
		CheckQuery(bRet, m_sqlQuery);
	}

	return valivator;
}

bool SYDBMgr::DeleteScoreTable(const QString& scoreTableCode)
{
	QString sql = QString("delete from %1 where scoreTableCode=%2")
		.arg(m_tableNames[DT_ScoreTable])
		.arg(scoreTableCode);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
	return bRet;
}

bool SYDBMgr::DeleteScoreTableItem(const QString& scoreTableCode,const QString& scoreCode)
{
	QString sql = QString("delete from %1 where scoreTableCode=%2 and scoreCode=%3")
		.arg(m_tableNames[DT_ScoreTable])
		.arg(scoreTableCode)
		.arg(scoreCode);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
	return bRet;
}


bool SYDBMgr::QueryAllTrainsCanBeAssignedToTask(std::vector<QSqlRecord> & records)
{
	const QString sql = QString("select * from %1").arg(m_tableNames[DT_TRAINFORTASK]);
	
	bool bRet = m_sqlQuery.exec(sql);
	
	CheckQuery(bRet, m_sqlQuery);
	
	if (bRet)
	{
		bRet = false;
		while (m_sqlQuery.next())
		{
			records.push_back(m_sqlQuery.record());
			bRet = true;
		}
	}
	return bRet;
}

void SYDBMgr::QueryAllReceiverTasks(int receiverID, std::vector<QSqlRecord> & records)
{
	const QString sql = QString("SELECT * FROM %1 WHERE ReceiverID = %2 ORDER BY id DESC").arg(m_tableNames[DT_TRAINTASKLIST]).arg(receiverID);

	bool bRet = m_sqlQuery.exec(sql);

	CheckQuery(bRet, m_sqlQuery);

	if (bRet)
	{
		bRet = false;
		while (m_sqlQuery.next())
		{
			records.push_back(m_sqlQuery.record());
			bRet = true;
		}
	}
	return;
}

void SYDBMgr::QueryAllAssignorTaskTemplate(int assignorID, std::vector<QSqlRecord> & records)
{
	QString sql = QString("SELECT * FROM %1 WHERE AssignorID = %2 AND IsTemplate = 1 ORDER BY id DESC").arg(m_tableNames[DT_TRAINTASKLIST]).arg(assignorID);

	if (assignorID == -1)
		sql = QString("SELECT * FROM %1 WHERE IsTemplate = 1 ORDER BY id DESC").arg(m_tableNames[DT_TRAINTASKLIST]);

	bool bRet = m_sqlQuery.exec(sql);

	CheckQuery(bRet, m_sqlQuery);

	if (bRet)
	{
		bRet = false;
		while (m_sqlQuery.next())
		{
			records.push_back(m_sqlQuery.record());
			bRet = true;
		}
	}
	return;
}
void SYDBMgr::QueryAllTrainsInOneTaks(int taskID, std::vector<QSqlRecord> & records)
{
	const QString sql = QString("SELECT * FROM %1 WHERE taskid = %2").arg(m_tableNames[DT_TRAINTASKDetail]).arg(taskID);

	bool bRet = m_sqlQuery.exec(sql);

	if (bRet)
	{
		bRet = false;
		while (m_sqlQuery.next())
		{
			records.push_back(m_sqlQuery.record());
			bRet = true;
		}
	}
	return;
}

int SYDBMgr::AddOneTaskEntry(const QString & TaskName, int assignorID, int receiverID, const QString & assignorName, const QString & receiverName, int trainNum , bool isTemplate)
{
	QString sql = QString("INSERT INTO %1 (TaskName,AssignorID,AssignorName,ReceiverID,ReceiverName,TrainNum,IsTemplate) values('%2',%3,'%4',%5,'%6',%7,%8)")
		.arg(m_tableNames[DT_TRAINTASKLIST])
		.arg(TaskName)
		.arg(assignorID)
		.arg(assignorName)
		.arg(receiverID)
		.arg(receiverName)
		.arg(trainNum)
		.arg(isTemplate ? 1 : 0);
	
	bool bRet = m_sqlQuery.exec(sql);
	
	CheckQuery(bRet, m_sqlQuery);
	
	if (!bRet)
		return -1;
	else
	{
		m_sqlQuery.exec("select last_insert_id() as last_id");

		if (m_sqlQuery.next())
		{
			QSqlRecord record = m_sqlQuery.record();

			int insertid = record.value("last_id").toInt();
			return insertid;
		}
		else
		{
			return -1;
		}
	}
}

void SYDBMgr::AddOneTrainEntryInTask(int taskId, const QString & trainName, const QString & trainGradeCode, int PassNeedScore, int PassNeedTime)
{
	QString sql = QString("INSERT INTO %1 (taskid, trainname, traingradecode, passneedscore, passneedtimes) values(%2,'%3','%4',%5,%6)")
		.arg(m_tableNames[DT_TRAINTASKDetail])
		.arg(taskId)
		.arg(trainName)
		.arg(trainGradeCode)
		.arg(PassNeedScore)
		.arg(PassNeedTime);

	bool bRet = m_sqlQuery.exec(sql);
}
void SYDBMgr::UpdateTaskTrainData(int id, int TrainMinute, int Score)
{
	QString sql = QString("UPDATE %1 SET traintimeused = traintimeused + %2 , currpasstimes = IF(passneedscore <= %3, currpasstimes + 1 , currpasstimes)  where id=%4")
		.arg(m_tableNames[DT_TRAINTASKDetail])
		.arg(TrainMinute)
		.arg(Score)
		.arg(id);
	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
	return;
}
// int SYDBMgr::AddMessageInfo(const QString& senderName,const QString& receiverName,bool isGroup,const QString& msgSubject,const QString& msgContent)
// {
// 	int messageId = QueryMaxIdFromTable(DT_Message)+1;
// 	QDateTime sendTime = QDateTime::currentDateTime();
// 
// 	QString sql = QString("insert into %1 (id,senderName,receiverName,sendTime,subject,content,isGroup) values(%2,'%3','%4','%5','%6','%7',%8)")
// 		.arg(m_tableNames[DT_Message])
// 		.arg(messageId)
// 		.arg(senderName)
// 		.arg(receiverName)
// 		.arg(sendTime.toString("yyyy-MM-dd HH:mm:ss.zzz"))
// 		.arg(msgSubject)
// 		.arg(msgContent)
// 		.arg(isGroup);
// 		bool Ret = m_sqlQuery.exec(sql);
// 		if(Ret)
// 			return 1;
// 		else
// 			return -1;
// 	
// }

void SYDBMgr::AddMessageInfo(int senderId, int receiverId, const QString& subject, const QString& content)
{
	QString sql = QString("insert into %1 (receiverId,senderId,subject,content) value(%2,%3,'%4','%5')")
		.arg(m_tableNames[DT_Message])
		.arg(receiverId)
		.arg(senderId)
		.arg(subject)
		.arg(content);

	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
}

void SYDBMgr::AddMessageReply(int messageId, int replyerId, const QString& content)
{
	QString sql = QString("insert into %1 (messageId,replyerId,content) value(%2,%3,'%4')")
		.arg(m_tableNames[DT_MessageReply])
		.arg(messageId)
		.arg(replyerId)
		.arg(content);

	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
}

int SYDBMgr::AddQuestionBank(int subject_type, const QString& title, const QString& A, const QString& B, const QString& C, const QString& D, const QString& E, const QString& answer)
{
	/*
	QSqlRecord record;
	if (QueryQuestionBank(title, record))
		return -1;

	int bankId = QueryMaxIdFromTable(DT_QuestionBank) + 1;
	QString sql = QString("insert into %1 (id,subject_type,title,A,B,C,D,E,answer) values(%2,%3,'%4','%5','%6','%7','%8','%9','%10')")
		.arg(m_tableNames[DT_QuestionBank])
		.arg(bankId)
		.arg(subject_type)
		.arg(title)
		.arg(A)
		.arg(B)
		.arg(C)
		.arg(D)
		.arg(E)
		.arg(answer);
	bool Ret = m_sqlQuery.exec(sql);
	if (Ret)
		return 1;
	else
		return -1;
		*/
	return 0;
}


int SYDBMgr::AddQuestion(int type, const QString& title, const QString& A, const QString& B, const QString& C, const QString& D, const QString& E, const QString& answer,int id)
{
	QSqlRecord record;
	if (id != -1)
	{
		if (QueryQuestion(id, record))
		{
			QString sql = QString("update %1 set title='%2',opta='%3',optb='%4',optc='%5',optd='%6',opte='%7',answer='%8' where id=%9 ")
				.arg(m_tableNames[DT_Question])
				.arg(title)
				.arg(A)
				.arg(B)
				.arg(C)
				.arg(D)
				.arg(E)
				.arg(answer)
				.arg(id);
			bool ret = m_sqlQuery.exec(sql);
			CheckQuery(ret, m_sqlQuery);
			return -1;
		}
	}
	int bankId = QueryMaxIdFromTable(DT_Question) + 1;
	QString sql = QString("insert into %1 (id,type,title,opta,optb,optc,optd,opte,answer) values(%2,%3,'%4','%5','%6','%7','%8','%9','%10')")
		.arg(m_tableNames[DT_Question])
		.arg(bankId)
		.arg(type)
		.arg(title)
		.arg(A)
		.arg(B)
		.arg(C)
		.arg(D)
		.arg(E)
		.arg(answer);
	bool Ret = m_sqlQuery.exec(sql);
	CheckQuery(Ret, m_sqlQuery);
	if (Ret)
		return 1;
	else
		return -1;
}

int SYDBMgr::AddAnswerScore(int userId, const QString& endAnswerTime, const QString& correctrate)
{
	int id = QueryMaxIdFromTable(DT_AnswerScore) + 1;
	QString sql = QString("insert into %1 (id,userId,endAnswerTime,correctrate) values(%2,%3,'%4','%5')")
		.arg(m_tableNames[DT_AnswerScore])
		.arg(id)
		.arg(userId)
		.arg(endAnswerTime)
		.arg(correctrate);
	bool Ret = m_sqlQuery.exec(sql);
	if (Ret)
		return 1;
	else
		return -1;
}

int SYDBMgr::AddScoreTableItem(const QString& scoreTableCode,const QString& scoreCode, 
							   const QString& trainName,const QString& trainCode,const QString& trainTypeCode, 
							   const QString& stepName,const QString& stepCode,const QString& stepDetailedDescription,int isKeyStep, 
							   const QString& scoreItemContent, const QString& scoreItemCode, int scoreItemValue,
							   const QString& scorePointDetailName,const QString& scorePointDetailCode,int scorePointDetailValue,int abilitys)
{
	QString sql = QString("insert into %1 (\
						  scoreTableCode,scoreCode,\
						  trainName,trainCode,trainTypeCode,\
						  stepName,stepCode,stepDetailedDescritpion,isKeyStep,\
						  scoreItemContent,scoreItemCode,scoreItemValue,\
						  scorePointDetailName,scorePointDetailCode,scorePointDetailValue,abilitys) \
						  values(\
						  '%2','%3',\
						  '%4','%5','%6',\
						  '%7','%8','%9',%10,\
						 '%11','%12',%13,\
						 '%14',%15,%16,%17)")
		.arg(m_tableNames[DT_ScoreTable])
		.arg(scoreTableCode).arg(scoreCode)
		.arg(trainName).arg(trainCode).arg(trainTypeCode)
		.arg(stepName).arg(stepCode).arg(stepDetailedDescription).arg(isKeyStep)
		.arg(scoreItemContent).arg(scoreItemCode).arg(scoreItemValue)
		.arg(scorePointDetailName).arg(scorePointDetailCode).arg(scorePointDetailValue).arg(abilitys);

	bool bRet = m_sqlQuery.exec(sql);
	CheckQuery(bRet, m_sqlQuery);
	if(bRet)
		return 1;
	else
		return -1;
}

const std::vector<QSqlRecord>& SYDBMgr::GetAllUserData(bool needReload)
{
	if(needReload || m_allUserData.size() == 0){
		m_allUserData.clear();

		QueryAllUserInfo(m_allUserData);
	}

	return m_allUserData;
}