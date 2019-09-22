/*********************************************
FileName:    NeedleHolder.h
FilePurpose: 实现持针器相关功能
Author:      Supo
Email:       chiyou7410@163.com
LastData:    2012.3.19
*********************************************/
#pragma once
#include "Instruments/tool.h"

class CXMLWrapperTool;
class CXMLWrapperTraining;
class MisCTool_PluginClamp;
class MisCTool_PluginRigidHold;
class CNeedleHolder : public CTool
{
public:
	CNeedleHolder();
	CNeedleHolder(CXMLWrapperTool * pToolConfig);
	virtual ~CNeedleHolder(void);

	bool Initialize(CXMLWrapperTraining * pTraining);
	std::string GetCollisionConfigEntryName();

	virtual bool Update(float dt);
	GFPhysVector3 CalculateToolCustomForceFeedBack();
	inline void SetSuturesId(int nSuturesId) { m_nSuturesId = nSuturesId; }
	inline void SetNeedleId(int nNeedleId) { m_nNeedleId = nNeedleId; }
    inline void SetNeedleNode(Ogre::SceneNode *pNode) { m_pNeedleNode = pNode; }
	void UpdateHeldPoints();
	//inline int GetTrianglePointsID(){ return m_nTrianglePointsID;}
	
private:
	int m_nSuturesId;
	int m_nNeedleId;
    Ogre::SceneNode *m_pNeedleNode;

    MisCTool_PluginRigidHold * m_pluginRigidHold;

    GFPhysCollideObject * m_leftTempObj;
    GFPhysCollideShape* m_leftOriginShape;

    GFPhysCollideObject * m_rightTempObj;
    GFPhysCollideShape* m_rightOriginShape;
};
