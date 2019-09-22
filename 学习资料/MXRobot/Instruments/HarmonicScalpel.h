/*********************************************
FileName:    HarmonicScalpel.h
FilePurpose: 实现超声刀相关功能
Author:      Supo
Email:       chiyou7410@163.com
LastData:    2012.3.15
*********************************************/
#pragma once
#include "tool.h"
#include "Dynamic/GoPhysDynamicLib.h"
#include <map>

class MisCTool_PluginClamp;
class CXMLWrapperTool;


class CHarmonicScalpel : public CTool
{
public:
	CHarmonicScalpel();
	CHarmonicScalpel(CXMLWrapperTool * pToolConfig);
	bool Update(float dt);

	virtual ~CHarmonicScalpel(void);
	bool Initialize(CXMLWrapperTraining * pTraining);
	std::string GetCollisionConfigEntryName();

	bool ElectricClampedFaces(float dt);

	//bool ElectricTouchFaces(float dt);

	void InternalSimulationStart(int currStep , int TotalStep , float dt);

	void onEndCheckCollide(GFPhysCollideObject * softobj , GFPhysCollideObject * rigidobj , const GFPhysAlignedVectorObj<GFPhysRSManifoldPoint> & contactPoints);

	//std::set<GFPhysSoftBodyFace*> GetFaceInClamp();
	
	virtual void onFrameUpdateStarted(float timeelapsed);

	virtual Ogre::TexturePtr GetToolBrandTexture();

	virtual float GetCutWidth();

	virtual bool ElectricCutOrgan(MisMedicOrgan_Ordinary * organ , GFPhysSoftBodyFace * face  , float weights[3]);
	
	virtual float GetFaceToElecBladeDist(GFPhysVector3 triVerts[3]);

	bool IsInElecCutting(MisMedicOrgan_Ordinary * organ);

	bool IsInElecCogulation(MisMedicOrgan_Ordinary * organ);

	int  TryBurnConnectStrips(VeinConnectObject * stripObj, float BurnRate, std::vector<Ogre::Vector3> & burnpos, std::vector<VeinConnectPair*> & burnPairs);

private:
	//GFPhysSoftBody * m_BurnBody;
	//GFPhysSoftBodyFace * m_BurnFace;
	//float m_BurnFaceWeights[3];

	float  m_continueElectricValue;
	//bool   m_canCut;
	Ogre::TexturePtr m_Tex;

	//about after cutting the organ can not be clamp
	bool  m_canClamp;
	float m_CooldownTime;
	bool  m_CanDoElecCut;

};