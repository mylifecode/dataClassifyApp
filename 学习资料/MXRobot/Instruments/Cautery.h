/*********************************************
FileName:    Cautery.h
FilePurpose: 实现电凝刀相关功能
Author:      Supo
Email:       chiyou7410@163.com
LastData:    2012.3.15
*********************************************/
#pragma once
#include "Tool.h"
#include "ElectricTool.h"

class CXMLWrapperTool;
class CCautery : public CElectricTool
{
public:
	CCautery();
	CCautery(CXMLWrapperTool * pToolConfig);
	virtual ~CCautery(void);

public:
	virtual bool Update(float dt);
	
private:

};