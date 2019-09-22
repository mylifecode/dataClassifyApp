#include "OnLineGradeMgr.h"
#include "XMLWrapperOnLineGrade.h"


COnLineGradeMgr::COnLineGradeMgr()
{
	m_pushCodeFunc = 0;
}


COnLineGradeMgr::~COnLineGradeMgr()
{
	m_mapGrade.clear();
}

void COnLineGradeMgr::LoadGrade(const vector<CXMLWrapperOnLineGrade *> & vtGrades)
{
	m_mapGrade.clear();

	for (vector<CXMLWrapperOnLineGrade*>::const_iterator it = vtGrades.begin(); it != vtGrades.end(); it++)
	{
		CXMLWrapperOnLineGrade* pXMLGrade = *it;
		OnLineGradeItemDefine& grade = m_mapGrade[pXMLGrade->m_Name];

		grade.m_description = pXMLGrade->m_Description;
		grade.m_strItemCode = pXMLGrade->m_ItemCode;
	
	}

}

bool COnLineGradeMgr::SendGrade(Ogre::String strGradeItem, int beginTime, int endTime)
{
	if (m_mapGrade.count(strGradeItem) == 0)
	{
		Ogre::LogManager::getSingletonPtr()->logMessage("*** SendOnLineGradeCode***  Invalid Item"  + strGradeItem);
		return false;
	}

	OnLineGradeItemDefine& grade = m_mapGrade[strGradeItem];

	if (m_pushCodeFunc)
	{
		QString strCode = grade.m_strItemCode.c_str();

		if (beginTime == 0)
		{
			beginTime = GetCurrentTime();
			endTime = 0;
		}
			
		m_pushCodeFunc(strCode.toLocal8Bit().data(), beginTime, endTime);
		Ogre::LogManager::getSingletonPtr()->logMessage("*** SendOnLineGradeCode*** " + grade.m_strItemCode + "  " + strGradeItem);
	}
	
	//send(grade.m_strItemCode, grade.m_strSheetCode);
	
}

void COnLineGradeMgr::setPushCodeFunc(pushCodeCallBack pushCodeFunc)
{
	m_pushCodeFunc = pushCodeFunc;
}