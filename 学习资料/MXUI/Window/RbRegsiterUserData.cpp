#include "RbRegsiterUserData.h"
#include <QVariant>
RbRegsiterUserData::RbRegsiterUserData(RbRegsiterUserData *parent /* = NULL */)
:QObject(parent)
,m_parent(parent)
{

}

RbRegsiterUserData::~RbRegsiterUserData()
{

}

QVariant RbRegsiterUserData::data(const QString& key)
{
	QVariant value = m_userPropertyMap[key];
	return value;
}


void RbRegsiterUserData::setData(const QString& key, QVariant value)
{
	m_userPropertyMap[key] = value;
}

void RbRegsiterUserData::setID(int strID)
{
	m_strID = strID;
}

int RbRegsiterUserData::getID()
{
	return m_strID;
}

int RbRegsiterUserData::childCount() const
{
	return m_childItems.count();
}

void RbRegsiterUserData::appendChild(RbRegsiterUserData *item)
{
	m_childItems.append(item);
}

void RbRegsiterUserData::removeChild(RbRegsiterUserData *item)
{
	m_childItems.removeAll(item);
}

void RbRegsiterUserData::removeChild(int row, int count)
{
	QList <RbRegsiterUserData *> pDelDataList;
	for (int i=0; i<count; i++)
	{
		RbRegsiterUserData *pData = m_childItems.at(row+i);
		pDelDataList.append(pData);
	}

	foreach (RbRegsiterUserData *pData, pDelDataList)
	{
		m_childItems.removeAll(pData);
	}//endf
}

RbRegsiterUserData *RbRegsiterUserData::parent()
{
	return m_parent;
}

RbRegsiterUserData *RbRegsiterUserData::child(int row)
{
	return m_childItems.value(row);
}