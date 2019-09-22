/**Author:zx**/
/**����ǯ**/
#pragma once
#include "ElectricTool.h"
#include "MisCTool_PluginRigidHold.h"

class MisCTool_PluginClamp;
class MisMedicOrgan_Ordinary;

class CXMLWrapperTool;
class CDissectingForceps : public CElectricTool
{
public:
	CDissectingForceps();
	CDissectingForceps(CXMLWrapperTool * pToolConfig);
	virtual ~CDissectingForceps(void);

	virtual bool Update(float dt);
	
	virtual void Release();
	
	virtual bool Initialize(CXMLWrapperTraining * pTraining);

	std::string GetCollisionConfigEntryName();

	virtual void InternalSimulationStart(int currStep , int TotalStep , float dt);

	virtual void InternalSimulationEnd(int currStep , int TotalStep , float dt);

	virtual void onFrameUpdateStarted(float timeelapsed);
	
	virtual void onEndCheckCollide(GFPhysCollideObject * softobj , GFPhysCollideObject * rigidobj , const GFPhysAlignedVectorObj<GFPhysRSManifoldPoint> & contactPoints);

	virtual void OnVeinConnectBurned(const std::vector<Ogre::Vector3> & burnpos);
	
	virtual Ogre::TexturePtr GetToolBrandTexture();

	virtual void BreakAdhesion();

	virtual int TryBurnConnectStrips(VeinConnectObject * stripObj, float BurnRate, std::vector<Ogre::Vector3> & burnpos, std::vector<VeinConnectPair*> & burnpair);

protected:
	void * m_pDynamicObject;

private:
	bool ElectricClampedFaces(float dt , bool isCut);
	bool ElectricTouchFaces(float dt);
	void CutOrgan(MisMedicOrgan_Ordinary *organ);

	Ogre::TexturePtr m_brandTex;

	//first index    [0]-- back up for the origin [1]/[2] --the polyline
	//second index   [0]-- left [1]--right
	ToolCutBlade m_cutBlade[3][2];

	//�Ƿ��Ѿ��и����
	bool m_bHaveCutted;
    // �ж��Ƿ������ס�����Ƕ�
	int  m_nClampMaxShaftAside;
    // Releases ʱ��
	size_t m_dReleaseTime;
    //
    //int m_nLastShaftAsideForClamp;

    bool m_bFlagForClampDistance;
    //һ���ſ������Ž��������
	int  m_nExpandNum;
	//���ҽڵ������ˮƽ��ĸ߶ȣ�����DrawTraingle������
	float m_fRaiseDis;
	
    MisCTool_PluginRigidHold * m_pluginRigidhold;
	std::map<GFPhysSoftBodyFace *,float> m_burn_record;
	float m_time_elapse;
	bool m_canCut;


	//[0] -- left [1] -- right
	float m_MinDistToCollidePoint[2];
	GFPhysVector3 m_MinDistPoint[2];
	GFPhysSoftBodyFace * m_MinDistFace[2];
	GFPhysCollideObject * m_MinCollideDistBody[2];

	GFPhysVector3 m_tipPoint_Local[2];
	GFPhysVector3 m_tipPoint_World[2];

	float m_MinPointWeights[2][3];


};
