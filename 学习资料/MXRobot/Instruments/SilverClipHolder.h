/*********************************************
FileName:    SilverClip.h
FilePurpose: 实现带银夹的持夹器相关功能
Author:      Supo
Email:       chiyou7410@163.com
LastData:    2012.2.29
*********************************************/
#pragma once
#include "Forceps.h"

class CXMLWrapperTool;
class MisCTool_PluginClamp;
class MisCTool_PluginSilverClip;
class CSilverClipHolder : public CForceps
{
public:
	CSilverClipHolder();
	CSilverClipHolder(CXMLWrapperTool * pToolConfig);
	virtual ~CSilverClipHolder(void);

	bool Initialize(CXMLWrapperTraining * pTraining);
	std::string GetCollisionConfigEntryName();

	void CreateClip();

	void InternalSimulationStart(int currStep , int TotalStep , float dt);

	virtual void onFrameUpdateStarted(float timeelpased);

	MisCTool_PluginSilverClip * m_SilverPlugin;
};