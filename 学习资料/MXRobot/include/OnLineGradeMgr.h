#pragma once

#include "Singleton.h"
#include "MXOgreWrapper.h"

#include <string>

using namespace std;
class CXMLWrapperOnLineGrade;

typedef void(*pushCodeCallBack)(const char code[20], int begintime, int endtime);

class COnLineGradeMgr : public CSingleT<COnLineGradeMgr>
{
	typedef struct {
		Ogre::String m_strItemCode;
		Ogre::String m_strSheetCode;
		Ogre::String m_description;
		
	} OnLineGradeItemDefine;

public:
	COnLineGradeMgr();
	~COnLineGradeMgr();

	void LoadGrade(const vector<CXMLWrapperOnLineGrade *> & vtScores);
	bool SendGrade(Ogre::String strGradeItem, int beginTime = 0, int endTime = 0);

	void setPushCodeFunc(pushCodeCallBack pushCodeFunc);
private:
	map<Ogre::String, OnLineGradeItemDefine> m_mapGrade;
	pushCodeCallBack m_pushCodeFunc;
};

