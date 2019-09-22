#include "SYUserInfo.h"
#include <QCryptographicHash>
#include "SYDBMgr.h"

SYUserInfo * SYUserInfo::s_Instance = 0;

SYUserInfo::SYUserInfo(void)
	:m_userId(0),
	m_userName(""),
	m_sex(US_Male),
	m_realName(""),
	m_classId(0),
	m_className(""),
	m_groupId(0),
	m_groupName(""),
	m_unencryptedPassword(""),
	m_encryptedPassword(""),
	m_permission(UP_Error),
	m_loginMode(LM_Old),
	m_preTotalOnlineTime(0)
{

}

SYUserInfo::~SYUserInfo(void)
{

}

SYUserInfo* SYUserInfo::Instance()
{
	if (s_Instance == 0)
	{
		s_Instance = new SYUserInfo();
	}
	return s_Instance;
	//static SYUserInfo userInfo;
	//return &userInfo;
}

void SYUserInfo::Clear()
{
	m_userId = 0;
	m_userName.clear();
	m_sex = US_Male;
	m_realName.clear();
	m_classId = 0;
	m_className.clear();
	m_groupId = 0;
	m_groupName.clear();
	m_teacherName.clear();
	m_unencryptedPassword.clear();
	m_encryptedPassword.clear();
	m_permission = UP_Error;
	m_loginTimes = 0;
	m_firstLoginTime = QDateTime::currentDateTime();
	m_lastLoginTime = m_firstLoginTime;
	m_loginTime = m_firstLoginTime;
	m_preTotalOnlineTime = 0;
}

bool SYUserInfo::Login(const QString& userName,const QString& password)
{
	Clear();

	QString sql = QString("select * from %1 where userName='%2' or realName='%3'")
					.arg(SYDBMgr::Instance()->GetTableName(SYDBMgr::DT_UserInfo))
					.arg(userName);	
	bool success;
	QVector<QSqlRecord> result;
	SYDBMgr::Instance()->Query(sql,result);
	if(result.size())
	{
		const QSqlRecord& record = result[0];
		//m_encryptedPassword = Encrypt(userName,password);			//后期修改
		m_encryptedPassword = password;
		if(m_encryptedPassword.compare(record.value("password").toString()) == 0)
		{
			m_userId = record.value("id").toInt();
			m_userName = record.value("userName").toString();
			if(record.value("sex").toInt() == US_Male)
				m_sex = US_Male;
			else
				m_sex = US_Female;

			//class info
			m_classId = record.value("classId").toInt();
			m_className.clear();
			if(m_classId > 0){
				QSqlRecord classRecord;
				success = SYDBMgr::Instance()->QueryClassInfo(m_classId, classRecord);
				if(success)
					m_className = classRecord.value("name").toString();
			}

			//group info
			m_groupId = record.value("groupId").toInt();
			m_groupName.clear();
			m_teacherName.clear();
			if(m_groupId > 0){
				QSqlRecord groupRecord;
				success = SYDBMgr::Instance()->QueryGroupInfo(m_groupId, groupRecord);
				if(success){
					//group name
					m_groupName = groupRecord.value("name").toString();

					//teacher name
					int teacherId = groupRecord.value("teacherId").toInt();
					QSqlRecord teacherRecord;
					success = SYDBMgr::Instance()->QueryUserInfo(teacherId, teacherRecord);
					if(success)
						m_teacherName = teacherRecord.value("userName").toString();
				}
			}

			m_realName = record.value("realName").toString();
			m_unencryptedPassword = password;
			m_permission = ConvertValueToPermission(record.value("permission").toInt());

			//time etc.
			m_firstLoginTime = record.value("firstLoginTime").toDateTime();
			m_lastLoginTime = record.value("lastLoginTime").toDateTime();
			m_preTotalOnlineTime = record.value("totalOnlineTime").toInt();	//second
			m_loginTime = QDateTime::currentDateTime();
			m_loginTimes = record.value("loginTimes").toInt() + 1;

			if(m_firstLoginTime.isValid() == false || m_firstLoginTime.isNull()){
				SYDBMgr::Instance()->UpdateFirstLoginTime(m_userId, m_loginTime);
				m_firstLoginTime = m_loginTime;
				m_lastLoginTime = m_loginTime;
			}

			//存入登录数据
			SYDBMgr::Instance()->UpdateLastLoginTime(m_userId,m_loginTime);
			SYDBMgr::Instance()->UpdateLoginTimes(m_userId,m_loginTimes);				//更新登录次数
			return true;
		}
		else
			m_encryptedPassword.clear();
	}

	return false;
}

void SYUserInfo::Login()
{
	Clear();
	Login("SimLap-Visitor", "123456");
	//SYDBMgr::Instance()->UpdateVisitorId();
// 	m_userId = -2;
// 	m_userName = "SimLap-Visitor";
// 	m_groupId = 0;
// 	m_realName = "SimLap-Visitor";
// 	m_unencryptedPassword = "";
// 	m_encryptedPassword = "";
// 	m_permission = UP_Visitor;
}

void SYUserInfo::Logout()
{
	UpdateTotalOnlineTimeToDB();

	Clear();
}

// bool SYUserInfo::LoginUseNewMode(const QString& userName,const QString& password)
// {
// 	Clear();
// 	m_loginMode = LM_New;
// 
// 	QSqlRecord record;
// 	if(SYDBMgr::Instance()->QueryUserInfo(userName,record))
// 	{
// 		m_userId = record.value("id").toInt();
// 		m_userName = userName;
// 		if(record.value("sex").toInt() == US_Male)
// 			m_sex = US_Male;
// 		else
// 			m_sex = US_Female;
// 		m_groupId = record.value("groupId").toInt();
// 		m_realName = record.value("realName").toString();
// 		m_unencryptedPassword = password;
// 		m_encryptedPassword = password;
// 		m_permission = ConvertValueToPermission(record.value("permission").toInt());
// 		m_lastLoginTime = record.value("lastLoginTime").toDateTime();
// 		m_loginTime = QDateTime::currentDateTime();
// 		m_loginTimes = record.value("loginTimes").toInt() + 1;
// 
// 		//存入登录数据
// 		SYDBMgr::Instance()->UpdateLastLoginTime(m_userId,m_loginTime);
// 		SYDBMgr::Instance()->UpdateLoginTimes(m_userId,m_loginTimes);				//更新登录次数
// 		return true;
// 	}
// 	else
// 	{
// 		m_userId = SYDBMgr::Instance()->AddUserInfo(userName,"",password);
// 
// 		if(m_userId != -1)
// 		{
// 			m_userName = userName;
// 			m_groupId = 0;
// 			m_realName = "";
// 			m_unencryptedPassword = password;
// 			m_encryptedPassword = password;
// 			m_loginTime = QDateTime::currentDateTime();
// 			m_lastLoginTime = m_loginTime;
// 			m_loginTimes = 1;
// 
// 			if(userName == "admin" || userName == "adm")
// 				m_permission = UP_SuperManager;
// 			else
// 				m_permission = UP_Student;
// 			SYDBMgr::Instance()->UpdateUserPermission(m_userId,m_permission);
// 
// 			SYDBMgr::Instance()->UpdateLastLoginTime(m_userId,m_loginTime);
// 			SYDBMgr::Instance()->UpdateLoginTimes(m_userId,m_loginTimes);				//更新登录次数
// 			return true;
// 		}
// 		else
// 		{
// 			Clear();
// 			return false;
// 		}
// 	}
// }
// 
// bool SYUserInfo::LoginUseNewMode(int id,int sigId)
// {
// 	Clear();
// 	m_loginMode = LM_New;
// 
// 	QSqlRecord record;
// 	if(SYDBMgr::Instance()->QueryUserInfo(id,record))
// 	{
// 		m_userId = id;
// 		m_userName = record.value("userName").toString();
// 		if(record.value("sex").toInt() == US_Male)
// 			m_sex = US_Male;
// 		else
// 			m_sex = US_Female;
// 		m_groupId = record.value("groupId").toInt();
// 		m_realName = record.value("realName").toString();
// 		m_unencryptedPassword = record.value("password").toString();
// 		m_encryptedPassword = m_encryptedPassword;
// 		m_permission = ConvertValueToPermission(record.value("permission").toInt());
// 		m_lastLoginTime = record.value("lastLoginTime").toDateTime();
// 		m_loginTime = QDateTime::currentDateTime();
// 		m_loginTimes = record.value("loginTimes").toInt() + 1;
// 
// 		//存入登录数据
// 		SYDBMgr::Instance()->UpdateLastLoginTime(m_userId,m_loginTime);
// 		SYDBMgr::Instance()->UpdateLoginTimes(m_userId,m_loginTimes);				//更新登录次数
// 		return true;
// 	}
// 	else
// 	{
// 		QString password = "123456";
// 		QString realName = "";
// 		QString content = "user%1";
// 		bool isOk = false;
// 		for(int index = 1;;++index)
// 		{
// 			m_userName = content.arg(index);
// 
// 			if(!SYDBMgr::Instance()->QueryUserInfo(m_userName,record))
// 			{
// 				if(SYDBMgr::Instance()->AddUserInfo(id,m_userName,realName,password))
// 				{
// 					m_groupId = 0;
// 					m_realName = realName;
// 					m_unencryptedPassword = password;
// 					m_encryptedPassword = password;
// 					m_loginTime = QDateTime::currentDateTime();
// 					m_lastLoginTime = m_loginTime;
// 					m_loginTimes = 1;
// 					m_permission = UP_Student;
// 
// 					SYDBMgr::Instance()->UpdateLastLoginTime(m_userId,m_loginTime);
// 					SYDBMgr::Instance()->UpdateLoginTimes(m_userId,m_loginTimes);				//更新登录次数
// 					isOk = true;
// 				}
// 				break;
// 			}
// 		}
// 
// 		if(!isOk)
// 			Clear();
// 		return isOk;
// 	}
// }

void SYUserInfo::Refresh()
{
	QSqlRecord record;
	bool success;
	if(SYDBMgr::Instance()->QueryUserInfo(m_userId, record))
	{
		m_realName = record.value("realName").toString();
		m_userName = record.value("userName").toString();
		if(record.value("sex").toInt() == US_Male)
			m_sex = US_Male;
		else
			m_sex = US_Female;
		//class info
		m_classId = record.value("classId").toInt();
		m_className.clear();
		if(m_classId > 0){
			QSqlRecord classRecord;
			success = SYDBMgr::Instance()->QueryClassInfo(m_classId, classRecord);
			if(success)
				m_className = classRecord.value("name").toString();
		}

		//group info
		m_groupId = record.value("groupId").toInt();
		m_groupName.clear();
		m_teacherName.clear();
		if(m_groupId > 0){
			QSqlRecord groupRecord;
			success = SYDBMgr::Instance()->QueryGroupInfo(m_groupId, groupRecord);
			if(success){
				//group name
				m_groupName = groupRecord.value("name").toString();

				//teacher name
				int teacherId = groupRecord.value("teacherId").toInt();
				QSqlRecord teacherRecord;
				success = SYDBMgr::Instance()->QueryUserInfo(teacherId, teacherRecord);
				if(success)
					m_teacherName = teacherRecord.value("userName").toString();
			}
		}

		m_realName = record.value("realName").toString();
		m_unencryptedPassword = record.value("password").toString();
		m_encryptedPassword = m_unencryptedPassword;
		m_permission = ConvertValueToPermission(record.value("permission").toInt());
 	}
}

QIcon SYUserInfo::GetHeadIcon() const
{
	QIcon icon;

	if(IsMale())
		icon.addFile("icons:/mainWindow/man2.png");
	else
		icon.addFile("icons:/mainWindow/women2.png");

	return icon;
}

std::size_t SYUserInfo::GetCurOnlineTime()
{
	//计算时间差值，返回时间
	std::size_t t = m_loginTime.secsTo(QDateTime::currentDateTime());
	return t;
}

std::size_t SYUserInfo::GetTotalTrainTime()
{
	return SYDBMgr::Instance()->QueryTotalTrainTime(m_userId);
}

//账户密码加密
QString SYUserInfo::Encrypt(const QString& userName,const QString& password)
{
	return QCryptographicHash::hash((userName + password).toLocal8Bit(),QCryptographicHash::Md5).toHex();
}

bool SYUserInfo::CheckPassword(const QString& password)
{
	return m_unencryptedPassword == password;
}

UserPermission SYUserInfo::ConvertValueToPermission(int value)
{
	switch(value)
	{
	case 0:
		return UP_Visitor;
	case 1:
		return UP_Student;
	case 2:
		return UP_Teacher;
	case 3:
		return UP_SuperManager;
	default:
		return UP_Error;
	}
}

int SYUserInfo::ConvertPermissionToIntValue(UserPermission up)
{
	return static_cast<int>(up);
}

QString SYUserInfo::ConvertPermisssionToChinese(UserPermission User_Permission)
{
	if(User_Permission==UP_Student)
		return QString::fromLocal8Bit("学员");
	else if(User_Permission==UP_Teacher)
		return QString::fromLocal8Bit("管理员");
	else if(User_Permission==UP_SuperManager)
		return QString::fromLocal8Bit("超级管理员");
	else
		return QString::fromLocal8Bit("权限错误");
}

bool SYUserInfo::IsAdminPermission(int permission)
{
	UserPermission up = ConvertValueToPermission(permission);
	return up == UP_Teacher || up == UP_SuperManager;
}

void SYUserInfo::UpdateTotalOnlineTimeToDB()
{
	std::size_t curOnlineTime = GetCurOnlineTime();
	SYDBMgr::Instance()->UpdateTotalOnlineTime(m_userId, curOnlineTime + m_preTotalOnlineTime);
}