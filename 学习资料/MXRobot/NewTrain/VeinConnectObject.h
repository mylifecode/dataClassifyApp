#ifndef _VEINCONNECTOBJECT_
#define _VEINCONNECTOBJECT_
#include <Ogre.h>
//#include "collision/GoPhysCollisionlib.h"
#include "dynamic/PhysicBody/GoPhysSoftTube.h"
#include "MisMedicOrganOrdinary.h"
#include "VeinConnectRender.h"
#include "OrganBloodMotionSimulator.h"
#include "BasicTraining.h"
#include "CustomConstraint.h"
//#include "Painting.h"
#define CANHOOKNUM 2
#define MAXHOOKNUM 2

#define MAXREBUILDCOUNT 4

class MisMedcDynamicObject;

class TextureBloodTrackEffect;

class CBasicTraining;

struct MMO_Face;

struct HookInfo
{
	HookInfo():m_cluster_id(0)/*,m_pair_id(0)*/,m_hooked(false) {};
	int m_cluster_id;
	///int m_pair_id;
	bool m_hooked; 
};


class VeinConnectPair
{
	friend class VeinConnectCluster;

public:
	enum PhysConnetType
	{
		PCT_NONE = 0,
		PCT_FF   = 1 , //face face
		PCT_WFF  = 1 << 1,//weak face face
	};
	VeinConnectPair();

	~VeinConnectPair();

	bool IsContact() const;

	float GetRestLength() const;

	float GetCurrLength() const;

	unsigned int m_type;//PhysConnetType
	//remove constraint collide node etc
	void RemovePhysicsPartInternal(GFPhysDBVTree & hostCollideTree);

	GFPhysVector3 GetHookPointForce(const GFPhysVector3 & InvHookSuportOffsetdir , bool IsTestDir = true) const;

	GFPhysVector3 GetPosOnA();

	GFPhysVector3 GetPosOnB();

	void BuildSolveCachedData();

    int m_MMo_faceAID;

    int m_MMo_faceBID;

	GFPhysSoftBodyFace * m_faceA;

	GFPhysSoftBodyFace * m_faceB;

	Ogre::Vector3 m_CurrPointPosA;
    GFPhysVector3 m_UndeformPointPosA;

    Ogre::Vector3 m_CurrPointPosB;
    GFPhysVector3 m_UndeformPointPosB;

	float m_weightsA[3];

	float m_weightsB[3];

	Ogre::Vector2 texcoord[4];

	//VeinFaceFaceWeakConstraint  m_FaceFaceWeakConnect;

	//VeinFaceFaceConnect m_FaceFaceConnect;//[2];

	GFPhysSoftBodyNode * m_StripNodeA;
	GFPhysSoftBodyNode * m_StripNodeB;

	GFPhysSoftBodyNode * m_FaceNodeA[3];
	GFPhysSoftBodyNode * m_FaceNodeB[3];

	Real m_InvFaceAMass;
	Real m_InvNodeAMass;

	Real m_InvFaceBMass;
	Real m_InvNodeBMass;

	GFPhysDBVNode * m_BVNode;

	float m_SuspendDistInFaceA;

	float m_SuspendDistInFaceB;

	int m_ObjAType;
	int m_ObjBType;

	int m_OrganAID;
	int m_OrganBID;//第二个pair的两个id一直是-1，需要重构

	bool m_Valid;

	float m_SpanScale;

	Ogre::ColourValue m_PairColor;

	GFPhysVector3 m_WolrdHookPoint;
		
    float m_TimeSinceLastBlood;
    
    int m_rebuildcount;

    VeinConnectObject* m_OwnObject;

protected:

	void SetAttachStateInternal(GFPhysCollideObject * rigid, const GFPhysVector3 & worldpoint, const GFPhysVector3 & localHookOffset, const GFPhysVector3 & localHookDir);
};

class VeinConnectCluster
{
public:
	enum Mode{
		STAY,
		REDUCE,
		DISCONNECT,
	};

	VeinConnectCluster(int stayNum = 3, int reduceNum = 2) : m_Color(Ogre::ColourValue(1,1,1,1)),
											m_ColorOfHookedPart(Ogre::ColourValue(1,1,1,1)),
											m_SpanScale(1.0f),
											m_SpanScaleOfHookedPart(m_SpanScale * 0.5),
											m_mode(VeinConnectCluster::STAY),
											m_isHooked(false),
											m_IsDetoryed(false),
											m_Valid(true),
											m_ObjAID(-1),
											m_ObjBID(-1)
	{
		m_StayNum = stayNum;
		m_ReduceNum = reduceNum;
		m_StripTetra = 0;
	}
	void RemovePhysicsPartInternal(GFPhysDBVTree & CollideTree);

	void OnAttachFaceChanged();

//for the new mode
	void SetHookOn(GFPhysCollideObject * rigid, const GFPhysVector3 & worldpoint, const GFPhysVector3 & localHookOffset, const GFPhysVector3 & localHookDir);
	
	void SetHookAndContactOff();
	
	void RemoveOne();

	void TotalyRemove();

	Ogre::ColourValue m_Color;
	Ogre::ColourValue m_ColorOfHookedPart;
	float m_SpanScale;
	float m_SpanScaleOfHookedPart;
	
	float m_SpanScaleBak;
	Ogre::ColourValue m_ColorBak;

	int m_StayNum;
	int m_ReduceNum;

	Mode m_mode; 

	bool m_isHooked;

//=============================================

    VeinConnectPair m_pair[2];
    
	GFPhysSoftBodyTetrahedron * m_StripTetra;
	int m_ObjAID;
	int m_ObjBID;
	bool m_Valid;
	GFPhysVector3 m_StickPoint;
	bool m_IsDetoryed;
};



class VeinConnectObject : public MisMedicOrganInterface , public GoPhys::GFPhysSoftBodyConstraint
{
public:
	VeinConnectObject(CBasicTraining * ownertriang);

	~VeinConnectObject();

	void setVisible(bool vis)
	{
		m_Visible = vis;
		if (m_RenderObject)
		{
			m_RenderObject->setVisible(vis);
		}
	}
	void PrepareSolveConstraint(Real Stiffness,Real TimeStep);

	void SolveConstraint(Real globalstiffness,Real TimeStep);

	void SolveFaceNode_NodeAnchor(GFPhysSoftBodyNode ** faceNodes,
		                          float weights[3],float InvFaceM,float InvNodeM,
		                          GFPhysSoftBodyNode * node);

	const VeinConnectPair & GetConnectPair(int clusterId  , int pairId);
	
	void SetConnectPairClampByRigid(int clusterId  , int pairId , GFPhysCollideObject * rigid , const GFPhysVector3 & worldpoint);

	//剪刀剪断与弯分离钳分离时使用
	void DisconnectPair(int clusterId );

	void  ReleaseConnectPair(int clusterId );
	
	void  ReleaseConnectPairWithRigid(int clusterId , GFPhysCollideObject * collideobj);

	virtual void Create(MisMedicDynObjConstructInfo & constructInfo)	{};

	void Create(MisMedicDynObjConstructInfo & constructInfo , DynObjMap & dynmap);

	virtual void RemovePhysicsPart();

	virtual void RemoveGraphicPart();

	virtual void UpdateScene( float dt , Ogre::Camera * camera);

	virtual void InternalSimulateEnd(int currStep , int TotalStep , Real dt);

	virtual void NotifyRigidBodyRemovedFromWorld(GFPhysRigidBody * rb);

	void readVeinConnectTexMapFile(const char * texmap);

	void CreateConnectFromFile(Ogre::SceneManager * scenemgr , Ogre::String connectfile , DynObjMap & dynmap);
	
	void BuildConnectConstraint(DynObjMap & dynmap);

	void CreateCollideTree();

	const GFPhysDBVTree &GetCollideTree();

	void TestCollisionWithBody(GFPhysRigidBody * convexobj , VeinRigidCollisionListener * listener = 0);

	void TryHookStrips(const GFPhysVector3 & end0 , 
					   const GFPhysVector3 & end1 , 
					   const GFPhysVector3 & localoffset , 
					   const GFPhysVector3 & localHookDir ,
					   float radius ,
					   GFPhysRigidBody * convexobj);

	void DestoryCluster(int clusterID);

	void SetClusterColor(int clusterid , const Ogre::ColourValue & color);

	void CreateVeinBlood(VeinConnectPair & pairToBlood , GFPhysVector3 & BloodPoint , float dt , float booldValue = 1.0f);
	//void RemoveConnectInHookState(GFPhysRigidBody * hookrigid);
	
	int BurnHookedAndContactedConnectInternal(std::set<int>& ClampOrganIdSet, GFPhysRigidBody * hookleft, GFPhysRigidBody * hookright, float dt, std::vector<Ogre::Vector3> & burnpos, std::vector<VeinConnectPair*> & burnpair);

	GFPhysVector3 CalculateHookForce(GFPhysRigidBody * hookrigid, const GFPhysVector3 & InvHookSuportOffsetdir);

	void RecalculateHookPointWorldPos();

	void RenderBloodDropOnTexture(Ogre::TexturePtr texture , Ogre::Vector2 textureCoord);

	void SetStiffness(float stiff);

	void SetDampingFactor(float dampfactor);

	//overridden
	virtual void refreshMaterial(const std::string& materialname);

	int GetCurrConnectCount();

	int GetInitConnectCount();

	void SetHookedCount();
	
	void AddHookedCount();

	void ClearHookedCount();

	//void SetClusterHookedOn(int clusterid);

	//void SetClusterHookedOff(int clusterid);

	bool SetHookInfo(int clusterid,int pairid);

	void SetHookOffInfo(int clusterid);

	void SetAllHookOff();

	void SetNewMode();

	int  GetNumPairRigidHooked(GFPhysCollideObject * rigid);

	Ogre::TexturePtr m_BloodTexturePtr;

	Ogre::SceneNode * m_SceneNode;

	//Ogre::ManualObject *m_pManualObject;

	VeinConnectStripsObject * m_RenderObject;

	//Ogre::String materialname;


	GFPhysDBVTree m_CollideTree;

	OrganBloodMotionSimulator m_DynBlood;

	//TextureBloodTrackEffect * m_TextureBloodEffect;

	float m_Stiffness;

	float m_DampingFactor;

	Ogre::String m_MaterialName;

	int m_connectCount;

	int m_InitConnectCount;

	int m_HookedCount;
	int m_HookedLimit;
	bool m_isHookLimitSet;

	int m_HookedLimitCount;

	HookInfo m_hook_info[MAXHOOKNUM];

	std::vector<VeinConnectCluster> m_clusters;

	bool m_CanBeHooked;

	bool m_Disconnected;

	bool m_IsNewMode;

	int m_ConnStayNum;
	int m_ConnReduceNum;

	bool m_Actived;

	bool m_SolveFFVolumeCS;

	bool m_CanBlood;

	float m_TimeSinceLastblood;

	GFPhysSoftBody * m_PhysBody;
protected:
	
	void  UpdateMesh(Ogre::Camera * pCamera);

	void  UpdateConnectPairState(float dt);

	void  UpdateCollideTree(float dt);

	/*void  ConstructWeakFaceFaceConstraint(VeinConnectPair & DstPair0 ,
									      VeinConnectPair & DstPair1 ,
									      MisMedicOrgan_Ordinary & objA , 
								          MisMedicOrgan_Ordinary & objB);

	void  ConstructFaceFaceConnect(VeinConnectPair & DstPair , 
							       GFPhysSoftBodyFace * FaceA , 
								   GFPhysSoftBodyFace * FaceB , 
								   float weightsA[3] , 
								   float weightsB[3] , 
								   MisMedicOrgan_Ordinary & objA , 
								   MisMedicOrgan_Ordinary & objB);*/

	std::map<GFPhysCollideObject*, int> m_RigidHookPairCounts;
	/*
	void CreateFaceTubeConnect(VeinConnectPair & DstPair , 
							   float stiffness,
							   MisMedicOrgan_Ordinary * FaceObj,
							   MisMedicOrgan_Tube * TubeObj,
							   GFPhysSoftBodyFace * physface , 
							   int TubeSeg , 
							   float FaceWeight[3] ,
							   float tubeWeight, 
							   const GFPhysVector3 & offsetFace , 
							   const GFPhysVector3 & offsetTube);

	void CreateTubeTubeConnection(VeinConnectPair & DstPair ,
								  float stiffness,
								  MisMedicOrgan_Tube * dynTubeA,
								  MisMedicOrgan_Tube * dynTubeB,
								  int SegA , int SegB , 
								  float weightA , float weightB,
								  const GFPhysVector3 & offsetA,
								  const GFPhysVector3 & offsetB);
	*/

//public:
//    PaintingTool painting;
};


#if(0)

class TriPrismConnectPoint
{
public:
	GFPhysSoftBodyFace * m_AdhereFace;
	GFPhysSoftBodyNode * m_Node;
	float m_FaceWeights[3];
};

class TriPrismConnectPair
{
public:
	TriPrismConnectPoint m_Point[6];
};

class TriPrismConnects  : public GFPhysSoftBodyConstraint
{
public:
	void SolveConstraint(Real Stiffniss,Real TimeStep)
	{
		for(int p = 0 ; p < m_ConnectPairs.size() ; p++)
		{
			for(int c = 0 ;c < 6 ; c++)
			{
				TriPrismConnectPoint & csPoint = ;

				GFPhysSoftBodyNode * FaceNodes[3] = csPoint.m_AdhereFace->m_Nodes[3];

				GFPhysVector3 p0 = FaceNodes[0]->m_CurrPosition*csPoint.m_FaceWeights[0] 
								  +FaceNodes[1]->m_CurrPosition*csPoint.m_FaceWeights[1]
								  +FaceNodes[2]->m_CurrPosition*csPoint.m_FaceWeights[2];

				GFPhysVector3 p1 = csPoint.m_Node->m_CurrPosition;

				GFPhysVector3 Diff = (p0-p1);

				Real Length = Diff.Length();

				Real diffLen = Length;

				Stiffniss = Stiffniss*m_Stiffness;

				if(Length > FLT_EPSILON)
				{	
					Diff  /= Length;//diff is normalized now 

					GFPhysVector3 grad00 = Diff*csPoint.m_FaceWeights[0];
					GFPhysVector3 grad01 = Diff*csPoint.m_FaceWeights[1];
					GFPhysVector3 grad02 = Diff*csPoint.m_FaceWeights[2];

					GFPhysVector3 gradNoda = -Diff;

					//weighted by invmass 's sum of gradient
					Real sumgrad = csPoint.m_FaceWeights[0]*csPoint.m_FaceWeights[0]*FaceNodes[0]->m_InvM
								  +csPoint.m_FaceWeights[1]*csPoint.m_FaceWeights[1]*FaceNodes[1]->m_InvM
								  +csPoint.m_FaceWeights[2]*csPoint.m_FaceWeights[2]*FaceNodes[2]->m_InvM
								  +csPoint.m_Node->m_InvM;

					Real scale = Stiffniss*(-diffLen) / sumgrad;

					GFPhysVector3 delta00 = scale*grad00*FaceNodes[0]->m_InvM;
					GFPhysVector3 delta01 = scale*grad01*FaceNodes[1]->m_InvM;
					GFPhysVector3 delta02 = scale*grad02*FaceNodes[2]->m_InvM;

					GFPhysVector3 deltaNode = scale*gradNoda*csPoint.m_Node->m_InvM;

					FaceNodes[0]->m_CurrPosition += delta00;
					FaceNodes[1]->m_CurrPosition += delta01;
					FaceNodes[2]->m_CurrPosition += delta02;

					csPoint.m_Node->m_CurrPosition += deltaNode;
				}
			}
		}
	}

	void AddConnectPair(GFPhysVector3 * undeformpos[6] , GFPhysSoftBodyFace * adhersionFace[6] , float weights[6][3])
	{
		 TriPrismConnectPair newPair;
		 for(int n = 0 ; n < 6 ; n++)
		 {
			 newPair.m_Point[n].m_Node = new GFPhysSoftBodyNode();
			 
			 //newPair.m_Point[n].m_Node->m_CurrPosition = newPair.m_Point[n].m_Node->m_UnDeformedPos = newPair.m_Point[n].m_Node->m_LastPosition = undeformpos[n];
			 newPair.m_Point[n].m_Node->SetDeformedPos(undeformpos[n]);
			 newPair.m_Point[n].m_Node->m_UnDeformedPos = undeformpos[n];
			 
			 //
			 newPair.m_Point[n].m_AdhereFace = adhersionFace[n];
			 newPair.m_Point[n].m_FaceWeights[0] = weights[n][0];
			 newPair.m_Point[n].m_FaceWeights[1] = weights[n][1];
			 newPair.m_Point[n].m_FaceWeights[2] = weights[n][2];
		 }
		 m_ConnectPairs.push_back(newPair);
	}

	/*
	void Create(TriPrismConnectPoint[][6] , int NumPair)
	{
		int NodeInitNum = NumPair*6;

		m_NumPair = NumPair;
		
		//m_Node
		//m_PhysNodes = new GFPhysSoftBodyNode[NodeInitNum];

		//m_ConnectNodes = new TriPrismConnectPoint[NodeInitNum];
		//GFPhysVector3 * UndeformPos = new GFPhysVector3[NodeInitNum];
		
		//float * MassNode = new float[NodeInitNum];
		
		for(int n = 0 ; n < NumPair ; n++)
		{
			for(int c = 0 ; c < 6 ; c++)
			{
			//	m_PhysNodes[n*6+c] = TriPrismConnectPoint[n][c].m_UnDeformedPos;
			//	m_ConnectNodes[n*6+c] = TriPrismConnectPoint[n][c];
				//MassNode[n*6+c] = 1.0f;
			}
		}

		/*
		m_PhysBody = new GFPhysSoftBody(UndeformPos , MassNode , NodeInitNum , GFPhysSoftBodyShape::SRCT_NONE , GFPhysSoftBodyShape::SSCT_NONE , false, 1);
		for(int p = 0 ; p < NumPair ; p++)
		{
			int offset = p*6;
			GFPhysSoftBodyTetrahedron * tetrahedron0 = m_PhysBody->AddTetrahedron(offset+0 , offset+1 , offset+2 , offset+4);
			GFPhysSoftBodyTetrahedron * tetrahedron1 = m_PhysBody->AddTetrahedron(offset+0 , offset+2 , offset+4 , offset+5);
			GFPhysSoftBodyTetrahedron * tetrahedron2 = m_PhysBody->AddTetrahedron(offset+3 , offset+4 , offset+5 , offset+0);
		}

		m_PhysBody->SetInternalForceType(GFPhysSoftBody::IFT_LINK |GFPhysSoftBody::IFT_TETRA);

		m_PhysBody->GetSoftBodyShape().SetSSCollideTag(GFPhysSoftBodyShape::SRCT_NONE);

		m_PhysBody->SetStiffness(0.6f);

		m_PhysBody->SetRSContactMode(0);

		m_PhysBody->SetGravity(GFPhysVector3(0,0,0));
		
		m_PhysBody->ApplyGravity();
		
		m_PhysBody->SetVelocityDamping(true , 6.0f , 0.00f , 0.00f);
		
		m_PhysBody->SetVelocityDampingMode(1);
		
		m_PhysBody->SetRSCollideFrictionCoff(4.0f);
		
		m_PhysBody->SetRSCollideStiffness(4.0f);
		
		m_PhysBody->EnableDoubleFaceCollision();
		
		//
		if( PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
		{
			PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddSoftBody(m_PhysBody);
		}
		
	}*/

	std::vector<TriPrismConnectPair> m_ConnectPairs;
	//int m_NumPair;
	//TriPrismConnectPair * m_ConnectPairs;
	//GFPhysSoftBodyNode * m_PhysNodes;
	//TriPrismConnectPoint * m_ConnectNodes;
	//GFPhysSoftBody * m_PhysBody;
	//TriPrismConnectPoint * 
};	
#endif
#endif