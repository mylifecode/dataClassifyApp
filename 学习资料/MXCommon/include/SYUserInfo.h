#pragma once
#include <QString>
#include <QIcon>
#include "MxDefine.h"

/**	
	������Ҫ���ڱ��浱ǰ��¼���û�����Ϣ�����û����ͣ�ѧ������ʦ������Ա
*/
class MXCOMMON_API SYUserInfo
{
	enum LoginMode
	{
		LM_Old,
		LM_New
	};

	enum UserSex
	{
		US_Male,
		US_Female
	};

private:
	SYUserInfo(void);
	~SYUserInfo(void);

public:
	static SYUserInfo* Instance();

	void Clear();

	/** ͨ�����øú��������õ�ǰ�û�����Ϣ
		�ɹ��򷵻�true�����򷵻�false��ԭ������Ϣ���ᱻ���
	*/
	bool Login(const QString& userName,const QString& password);
	//bool LoginUseNewMode(const QString& userName,const QString& password);
	//bool LoginUseNewMode(int id,int sigId);
	void Login();
	void Logout();
	void Refresh();

	int GetUserId() {return m_userId;}
	
	int GetClassId() { return m_classId; }
	const QString& GetClassName() { return m_className; }

	int GetGroupId() {return m_groupId;}
	const QString& GetGroupName() { return m_groupName; }

	const QString& GetTeacherName() const { return m_teacherName; }

	const QString& GetUserName() {return m_userName;}
	bool IsMale() const { return m_sex == US_Male; }
	const QString& GetRealName() {return m_realName;}
	const QDateTime& GetFirstLoginTime() { return m_firstLoginTime; }
	const QDateTime& GetLastLoginTime() {return m_lastLoginTime;}
	const QDateTime& GetLoginTime() {return m_loginTime;}

	QIcon GetHeadIcon() const;

	/**  online time */
	std::size_t GetCurOnlineTime();
	std::size_t GetTotalOnlineTime() { return m_preTotalOnlineTime + GetCurOnlineTime(); }
	std::size_t GetTotalTrainTime();

	/**
		�ø���������͵�ǰ������к˶ԣ����Ƿ�ƥ��
	*/
	bool CheckPassword(const QString& password);

	bool IsStudentPermission() {return m_permission == UP_Student;}

	bool IsSuperManager() {return m_permission == UP_SuperManager;}

	bool IsVisitor() {return m_permission == UP_Visitor;}

	bool IsNewLoginMode() {return m_loginMode == LM_New;}

	UserPermission GetUserPermission(){ return m_permission;}

	QString ConvertPermisssionToChinese(UserPermission User_Permission); //���û�Ȩ��ת��Ϊ��Ӧ����������

	void Exit() {exit(100);}

	void UpdateTotalOnlineTimeToDB();

	static QString Encrypt(const QString& userName,const QString& password);

	/**
		��������ֵת��ΪUserPermission���͵�Ȩ��ֵ
	*/
	static UserPermission ConvertValueToPermission(int value);

	static int ConvertPermissionToIntValue(UserPermission up);

	/**
		���Ը�����Ȩ��ֵ�Ƿ�Ϊ����Ա��������������Ա��
	*/
	static bool IsAdminPermission(int permission);

	

public:
//private:
	int		m_userId;
	QString m_userName;
	UserSex m_sex;
	int m_classId;
	QString m_className;
	int		m_groupId;
	QString m_groupName;
	QString m_teacherName;
	QString m_realName;
	/// δ���ܵ�����
	QString m_unencryptedPassword;
	QString m_encryptedPassword;
	UserPermission		m_permission;

	int m_loginTimes;
	QDateTime m_firstLoginTime;
	QDateTime m_lastLoginTime;

	int m_preTotalOnlineTime;

	QDateTime m_loginTime;
	LoginMode m_loginMode;

	static SYUserInfo * s_Instance;
};
