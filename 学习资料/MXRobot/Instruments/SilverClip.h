/*********************************************
FileName:    SilverClip.h
FilePurpose: ʵ��������ع���
Author:      Supo
Email:       chiyou7410@163.com
LastData:    2012.2.15
*********************************************/
#pragma once
#include "ClipApplier.h"

class CXMLWrapperTool;
class CSliverClip : public CClipApplier
{
public:
	CSliverClip();
	CSliverClip(CXMLWrapperTool * pToolConfig);
	virtual ~CSliverClip(void);
};