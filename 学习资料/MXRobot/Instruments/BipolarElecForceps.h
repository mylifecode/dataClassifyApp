/*********************************************
FileName:    BipolarElecForceps.h
FilePurpose: 实现双极电凝钳相关功能
Author:      Supo
Email:       chiyou7410@163.com
LastData:    2012.2.23
*********************************************/
#pragma once
#include "ElectricTool.h"


class CXMLWrapperTool;
class MisCTool_PluginClamp;
class CBipolarElecForceps : public CElectricTool
{
public:
	CBipolarElecForceps();
	CBipolarElecForceps(CXMLWrapperTool * pToolConfig);
	virtual ~CBipolarElecForceps(void);
	bool Initialize(CXMLWrapperTraining * pTraining);
	
	std::string GetCollisionConfigEntryName();

	void AddClampConstraint(GFPhysSoftBodyNode * NodeClamp , 
							GFPhysSoftBodyFace * FaceClamp,
							GFPhysRigidBody * ClampRigid ,
							const GFPhysVector3 & ClampNormal ,
							float stiffness = 0.6f);

	//==============================================================================================================

	virtual Ogre::TexturePtr GetToolBrandTexture();

public:
	bool ElectricClampedFaces(float dt);
	int  TryBurnConnectStrips(VeinConnectObject * stripObj, float BurnRate, std::vector<Ogre::Vector3> & burnpos, std::vector<VeinConnectPair*> & burnPairs);

	virtual bool Update(float dt);
	virtual void onFrameUpdateStarted(float timeelapsed);
	//virtual void onFrameUpdateEnded();
private:

	Ogre::TexturePtr m_Tex;

	int	m_counter;

	float m_BurnCutFaceTime;

	bool m_IsLastBurnCutFace;

	//float m_TimeSinceLastSmokeAdd;
};