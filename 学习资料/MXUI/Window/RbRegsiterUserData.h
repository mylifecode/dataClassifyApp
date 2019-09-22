#ifndef RBREGISTERUSERDATA_H
#define RBREGISTERUSERDATA_H


#include <QObject>
#include <QMap>
#include <QList>

struct userDataProfile
{
	int id;
	QString strUserName;
	int userPassword;
	int Permission;
};

class RbRegsiterUserData : public QObject
{
	Q_OBJECT
public:
	RbRegsiterUserData(RbRegsiterUserData *parent = NULL);
	~RbRegsiterUserData();
public:
	void setID(int strID);
	int getID();
	void setData(const QString& key, QVariant value);
	QVariant data(const QString& key);
	RbRegsiterUserData *parent();
	void appendChild(RbRegsiterUserData *item);
	void removeChild(RbRegsiterUserData *item);
	void removeChild(int row, int count);
	int childCount() const;
	RbRegsiterUserData *child(int row);
private:
	int m_strID;
	QMap<QString, QVariant> m_userPropertyMap;
	QList <RbRegsiterUserData*> m_childItems;
	RbRegsiterUserData *m_parent;
};
#endif