#pragma once
#include "RbShieldLayer.h"
#include "ui_RbEditUserInfoWindow.h"

/**
	�˽�������Ҫ�����޸�ĳ���û��ĸ�����Ϣ��
		1���޸��û���������Ϊ��ʵ������������ݿ����Ѿ�������ͬ���û�������ô�޸�ʧ��
		2���޸���ʵ����������Ϊ�ա����Ժ����ݿ��е�����ͬ��
		3������С��
	�����޸��û��ĸ�����Ϣ�����ɹ���Ա�����������ԣ���Ҫ�ṩ����Ա��id�����⣬���Ҫ�޸ĵ�
	������������ڹ���Ա���κ�һ��С���У���ô�����ʧ�ܡ�������δ����Ķ���---���Ա���ӵ����е�С���У�
*/

class RbEditUserInfoWindow : public RbShieldLayer
{
	Q_OBJECT
public:
	/**
		adminId������Աid��ָ���޸ĸù���Ա�������ѧ��
		userName�����޸ĵĶ�����û���
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
	/// ��ǰ�ĵ�¼����
	QString m_curPassword;
	/// ע���˺�ʱʹ�õĳ�ʼ����
	QString m_initPassword;
	bool m_isResetPassword;
	
	QMap<int,QString> m_adminGroupMap;
};
