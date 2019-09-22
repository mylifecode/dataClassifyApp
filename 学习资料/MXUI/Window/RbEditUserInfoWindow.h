#pragma once
#include "RbShieldLayer.h"
#include "ui_RbEditUserInfoWindow.h"

/**
	此界面类主要用来修改某个用户的个人信息：
		1、修改用户名，可以为真实姓名，如果数据库中已经存在相同的用户名，那么修改失败
		2、修改真实姓名，可以为空、可以和数据库中的数据同名
		3、所属小组
	由于修改用户的个人信息必须由管理员来操作，所以，需要提供管理员的id，此外，如果要修改的
	对象如果不属于管理员的任何一个小组中，那么则操作失败。不包括未分组的对象---可以被添加到已有的小组中！
*/

class RbEditUserInfoWindow : public RbShieldLayer
{
	Q_OBJECT
public:
	/**
		adminId：管理员id，指定修改该管理员所管理的学生
		userName：被修改的对象的用户名
	*/
	RbEditUserInfoWindow(int adminId,const QString& userName,QWidget * pParent);
	~RbEditUserInfoWindow(void);

	bool canOperate() { return m_canUpdate;}

private slots:
	void on_cancelBtn_clicked();
	void on_okBtn_clicked();

private:
	Ui::RbEditUserInfoWindow ui;
	
	bool m_canUpdate;
	int m_userId;
	QString m_originUserName;
	QString m_originRealName;
	int m_originGroupId;
	/// 当前的登录密码
	QString m_curPassword;
	/// 注册账号时使用的初始密码
	QString m_initPassword;
	bool m_isResetPassword;
	
	QMap<int,QString> m_adminGroupMap;
};
