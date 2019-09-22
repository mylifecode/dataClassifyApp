/*********************************************
FileName:    HarmonicScalpel.h
FilePurpose: 实现活检钳相关功能
Author:      
Email:       
LastData:    2015.9.22
*********************************************/
#pragma once
#include "tool.h"
#include "Dynamic/GoPhysDynamicLib.h"
#include "Forceps.h"
#include <map>
#include "MisMedicCToolPluginInterface.h"
#include "MisCTool_PluginClamp.h"

class CXMLWrapperTool;


class CBiopsyForceps : public CForceps
{
public:
	CBiopsyForceps();
	CBiopsyForceps(CXMLWrapperTool * pToolConfig);
	bool Update(float dt);

	virtual ~CBiopsyForceps(void);
	bool Initialize(CXMLWrapperTraining * pTraining);

	std::string GetCollisionConfigEntryName();

	std::set<GFPhysSoftBodyFace*> GetFaceInClamp();
	bool HasGraspSomeThing();
	inline float GetClampedTime(){return m_fClampedTime;}
	//inline MisMedicOrgan_Ordinary* GetClampedOrgan(){return m_pluginClamp->GetOrganBeClamped();}
	//inline void CollectTetrasInClampRegion(std::vector<GFPhysSoftBodyTetrahedron*> & m_tetras){m_pluginClamp->CollectTetrasInClampRegion(m_tetras);}

	virtual void onFrameUpdateStarted(float timeelapsed);

private:

	int m_GraspMode;

	float m_fClampedTime;

	//for debug
	Ogre::ManualObject *m_pManualObjectForDebugLeft;
	Ogre::SceneNode *m_pSceneNodeForDebugLeft;


};