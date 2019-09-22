/*********************************************
FileName:    Cautery.h
FilePurpose: ʵ�ֵ�������ع���
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