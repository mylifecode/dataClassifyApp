#ifndef _MISCTOOL_PLUGINDISSECTCONNECTPAIR_
#define _MISCTOOL_PLUGINDISSECTCONNECTPAIR_
#include <map>
#include <set>
#include<ogre.h>
#include "MXOgreWrapper.h"
#include "ITraining.h"
#include "ToolsMgr.h"
#include "OgreMaxScene.hpp"
#include "MXOgreWrapper.h"
#include "IObjDefine.h"
#include "Instruments/MisMedicCToolPluginInterface.h"
#include "ScoreMgr.h"
#include "TipMgr.h"
#include "Painting.h"

class MisCTool_PluginDissectConnectPair :  public MisMedicCToolPluginInterface
{
public:
	class ToolRegion
	{
	public: 
		ToolRegion();

		void UpdateToWorldSpace();

		//void CalcRegionWorldAABB(float margin , GFPhysVector3 & aabbmin , GFPhysVector3 & aabbmax);

		//GFPhysVector3 GetLocalClampNormal();

		//GFPhysVector3 GetLocalClampCoordOrigin();

		GFPhysRigidBody * m_AttachRigid;

		//float m_HalfExt0;	
		//float m_HalfExt1;	

		GFPhysVector3 m_CenterLocal;
		GFPhysVector3 m_CenterWorld;

		GFPhysVector3 m_Axis0Local;//Coord0Local;
		GFPhysVector3 m_Axis1Local;//Coord1Local;

		GFPhysVector3 m_Axis0World;
		GFPhysVector3 m_Axis1World;

		GFPhysVector3 m_OutNormalLocal;
		GFPhysVector3 m_OutNormalWorld;
		
		float m_Thickness;

		Real m_normalSign;
	};

	struct  ConnPairContact
	{
		ConnPairContact(VeinConnectObject * veinObj , int clusterId , int pairId , float toolShaft ,  int regionIndex) 
			: m_VeinObj(veinObj) , 
			m_ClusterId(clusterId) , 
			m_PairId(pairId) ,
			m_ShaftContact(toolShaft) ,
			m_LastShaft(toolShaft) ,
			m_RegionIndex(regionIndex){ }

		//int m_VeinId;
		VeinConnectObject * m_VeinObj;
		int m_ClusterId;
		int m_PairId;
	
		float m_ShaftContact;
		float m_LastShaft;
		int m_RegionIndex;
	};
	 
	MisCTool_PluginDissectConnectPair(CTool * tool , float shaftDifference = 13.f);

	~MisCTool_PluginDissectConnectPair();

	void SetToolRegion(GFPhysRigidBody * attachRigid,
		const GFPhysVector3 & center,
		const GFPhysVector3 & axis0,
		const GFPhysVector3 & axis1,
		int	RegionIndex,
		Real	normalSign);

	void SetToolRegion(GFPhysRigidBody * attachRigid,
		const GFPhysVector3 & centerLocal,
		const GFPhysVector3 & OutNormalLocal,
		const float thickness,
		int	RegionIndex);

	virtual void PhysicsSimulationStart(int currStep , int TotalStep , float dt);

	virtual void OneFrameUpdateStarted(float timeelapsed);

	virtual void OneFrameUpdateEnded();

	virtual void CollideVeinConnectPair(VeinConnectObject * veinobject ,
		GFPhysCollideObject * convexobj,
		int cluster , 
		int pair,
		const GFPhysVector3 & collidepoint);



protected:
	float m_ShaftDifferenceCanDissect;

	ToolRegion m_ToolRegion[2];
	
	std::list<ConnPairContact> m_ConnPairsContact;
	
	//debug
//	Ogre::ManualObject * m_pManual;
};










#endif