/*********************************************
FileName:    ClipHolder.h
FilePurpose: 实现持夹器相关功能
Author:      Supo
Email:       chiyou7410@163.com
LastData:    2012.2.15
*********************************************/
#pragma once
#include "Forceps.h"

class CXMLWrapperTool;
class CClipHolder : public CForceps
{
public:
	CClipHolder();
	CClipHolder(CXMLWrapperTool * pToolConfig);
	virtual ~CClipHolder(void);

};