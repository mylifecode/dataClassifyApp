#pragma once

#include <Ogre.h>
#include "PhysicsWrapper.h"
#include "collision/CollisionShapes/GoPhysCompoundShape.h"
#include "MisMedicTubeShapeRendObject.h"
//#include "SutureThreadV2.h"
#include "TrainUtils.h"


using namespace GoPhys;
enum AnchorTypeV2;

class MisNewTraining;
class SutureNeedleV2;
class SutureThreadV2;

class MisMedicOrgan_Ordinary;
class CTool;
class NeedleActionListenerV2
{
public:
    //Needle
    virtual void OnCreateInAnchor(const GFPhysSoftBodyFace* face,const Real weights[]) = 0 ;
    virtual void OnCreateOutAnchor(const GFPhysSoftBodyFace* face,const Real weights[]) = 0 ;
    virtual void OnRemoveInAnchor(const GFPhysSoftBodyFace* face,const Real weights[]) = 0 ;
    virtual void OnRemoveOutAnchor(const GFPhysSoftBodyFace* face,const Real weights[]) = 0 ;
    virtual void OnSwitchAnchor(const GFPhysSoftBodyFace* face, const Real weights[]) = 0;
    //Rope
    virtual void OnMovingRopeAnchor(const SutureThreadV2* thread, const int index, const Real weights[]) = 0;
    virtual void OnRemoveRopeAnchor(const GFPhysSoftBodyFace* face,const Real weights[]) = 0 ;
    
};
//////////////////////////////////////////////////////////////////////////////////////      


struct AnchorPosV2
{
    int segment;    //0<=segment <=size-1
    Real lambda;   //0<=lambda<1
};

class GFPhysFaceNeedleAnchorV2// : public GFPhysPositionConstraint//GFPhysSoftBodyConstraint
{
public:
    GFPhysFaceNeedleAnchorV2(GFPhysVector3*,GFPhysSoftBodyFace*,Real*,SutureNeedleV2*,AnchorTypeV2);
  
	~GFPhysFaceNeedleAnchorV2();
   
	void SetFriction(float friction);
   
	void PrepareSolveConstraint(Real Stiffness,Real TimeStep);
    
	void SolveConstraint(Real Stiffniss,Real TimeStep);
	
	void SlipAnchor();

    GFPhysSoftBodyFace * GetFace(){return m_Face;}
	
	GFPhysVector3 GetNormalImpulse()
	{ 
		return m_NormalDir[0] * m_AccumNormal[0] + m_NormalDir[1] * m_AccumNormal[1]; 
	}
	
	GFPhysVector3 GetAnchorLocalPos();
	
	GFPhysVector3 GetAnchorWorldPos();
	
	AnchorPosV2   GetAnchorInfo();
	
	void          SetAnchorInfo(AnchorPosV2 info);
public:
	Real			     m_weights[3];
    bool                 m_BHeadOut;
    bool                 m_BTailOut;
	Real				 m_SuperfluousOffset;
    AnchorTypeV2         m_Type;
    Real                 m_initAngle;
	GFPhysVector3        m_ForceFeedBack;
	GFPhysVector3        m_ForceFeedBackAlongTan;
protected:
	//
	SutureNeedleV2*      m_SutNeedle;
    GFPhysSoftBodyFace*  m_Face;
    GFPhysRigidBody*	 m_Rigid;// Body
 
	
    
	GFPhysVector3		 m_AnchorLocalPoint;// Anchor position in body space
    AnchorPosV2          m_AnchorInfo;
	float                m_AnchorRation;

   
	//solver data 1
    GFPhysMatrix3		 m_ImpMat;			// Impulse matrix
    GFPhysVector3		 m_R;			    //anchor relative to Body's Mass center in w.c.s
	GFPhysVector3        m_AnchorWorld;
    Real                 m_Friction;
	//solver data 2 for calculate slip friction
	GFPhysVector3        m_TangentDir;
	GFPhysVector3        m_NormalDir[2];
	float                m_AccumFriction;
	float                m_AccumNormal[2];
	int                  m_IterorCount;
    //GFPhysVector3		 m_RigidImpulse;    
};

struct FaceAnchorInfoV2
{
    MisMedicOrgan_Ordinary* POrgan;
    GFPhysFaceNeedleAnchorV2* pAnchor;   
    std::vector<GFPhysSoftBodyFace*> pFacesVector;
};


//===================================================================================================================
class SutureNeedleV2 : public GFPhysPositionConstraint
{
public:
	SutureNeedleV2();
	
	//SutureNeedleV2(MisNewTraining * newTrain);

	~SutureNeedleV2();

    void CreateSutureNeedleV2(MisNewTraining * newTrain, int threadnodenum, Real restlen, const Ogre::String & needleskeleton);

    void Disappear();

	void CalcNeedleMassProperty(GFPhysVector3 NodePos[] , 
								int   NumNode , 
								float massPerNode ,
								float collideradius,
								float & totalMass ,
								GFPhysVector3 & com , 
								GFPhysMatrix3 & inertiaTesnor);
	
	inline GFPhysRigidBody* GetPhysicBody() { return m_NeedlePhysBody; }

    inline SutureThreadV2* GetSutureThread() {return m_AttachedRope; }

    inline GFPhysCompoundShape * GetCompoundShape(){return m_NeedleCollideShape;}

	void CreateVisibleMesh(const char* meshFileName,int id);

	void SetVisibleMeshMaterial(bool bClamped);

    void UpdateMesh();

    void CreateNeedleAnchor(  const GFPhysAlignedVectorObj<GFPhysSoftFaceRigidContact> & RSContactConstraints );

    void UpdateAnchor();

	void RendGreen(int begin, int end);

	virtual void  InternalSimulationStart(int currStep , int TotalStep , float dt);
	virtual void  InternalSimulationEnd(int currStep , int TotalStep , float dt);

	virtual void PrepareSolveConstraint(Real Stiffness,Real TimeStep);

	virtual void SolveConstraint(Real Stiffness,Real TimeStep);

	int GetNeedleNodeNum();
	
	int GetNeedleSegNum();

	//获得针尖方向
	GFPhysVector3 GetNeedleDirction();

    //获得针所在平面的法向量
    GFPhysVector3 GetNeedleNormalDirction();
	
	GFPhysVector3 GetNeedleNodePosByIndex(int index);

	//获得针尖质心位置(世界坐标系)
	GFPhysVector3 GetNeedleHeadNodeWorldPos();   
	
	//获得针尾质心位置(世界坐标系)
	GFPhysVector3 GetNeedleTailNodeWorldPos();   


	//void ResetNeedlePosition();

	//void ResetNeedleRotation();

	static void GFPhysQuaternionFromEuler(GFPhysQuaternion& q,Real yawDegree,Real pitchDegree,Real rollDegree);
	
	static void OgreQuaternionFromEuler(Ogre::Quaternion& q,const Real yawDegree,const Real pitchDegree,const Real rollDegree);
	
	GFPhysVector3 m_vMeshPosOffset;

 //   /*退针*/
    void RemoveHeadAnchor();//针尖退出约束    

 //   /*正常出针*/
    void RemoveTailAnchor();//针尾删除入针约束    

	///*正常出线*/	--改到线上
	//void RemoveThreadAnchor();//线尾删除出线约束
	//
	//void CreateRopeAnchor(AnchorTypeV2 type, GFPhysSoftBodyFace* face, Real * weights, Real offset);
	//   
    void SynchronizeState();
    
	Real AnchorInfo2Ratio(const AnchorPosV2& pos1);
   
	AnchorPosV2 Ratio2AnchorInfo(const Real& ratio);
	
	Real ConvertRealDistToRatioDist(Real realdist);

	GFPhysVector3 GetNeedleSegmentTangent(int segment , float lambda);
	
	void SlipFaceRopeAnchors();
	
    inline GFPhysVectorObj<FaceAnchorInfoV2>& getFaceNeedleAnchors(){return m_FaceNeedleAnchors;}

	void OnRigidBodyCollided(GFPhysCollideObject * ra,
		                     GFPhysCollideObject * rb,
							 const GFPhysManifoldPoint * contactPoints, 
							 int NumContactPoints);

	void OnRigidBodySeperate(GFPhysCollideObject * ra, 
		                     GFPhysCollideObject * rb);

	GFPhysVector3 GetForceFeedBack();
    bool Getinterval(const GFPhysVector3& pos, int& i, int& j);
public:
    void addNeedleActionListener( NeedleActionListenerV2* listener );
    void removeNeedleActionListener(NeedleActionListenerV2 * listener);
    void removeAllNeedleActionListener();
    
    void notifyCreateInAnchor(const GFPhysSoftBodyFace* face,const Real weights[]);
    void notifyCreateOutAnchor(const GFPhysSoftBodyFace* face,const Real weights[]);
    void notifyRemoveInAnchor(const GFPhysSoftBodyFace* face,const Real weights[]);
    void notifyRemoveOutAnchor(const GFPhysSoftBodyFace* face,const Real weights[]);
    void notifySwitchAnchor(const GFPhysSoftBodyFace* face, const Real weights[]);
    void notifyMovingRopeAnchor(const SutureThreadV2 * thread,const int index, const Real weights[]);
    void notifyRemoveRopeAnchor(const GFPhysSoftBodyFace* face,const Real weights[]);
	
	inline bool GetBeHold()
	{ 
		return m_ClampeTools.size() > 0 ? true : false; 
	}
	//=================================================================================
	void AddClampTool(CTool * tool)
	{
		m_ClampeTools.insert(tool);
		m_RecentClampTool = tool;
	}
	//=================================================================================
	void RemoveClampTool(CTool * tool)
	{
		m_ClampeTools.erase(tool);
		if (m_RecentClampTool == tool)
		    m_RecentClampTool = 0;
	}
	//============================================================================
	CTool * GetRecentClampTool()
	{
		return m_RecentClampTool;
	}
	
public:

    GFPhysAlignedVectorObj<GFPhysVector3> m_NeedleNodeWorldPos;
    GFPhysAlignedVectorObj<GFPhysVector3> m_NeedleNodeLocalPos;

	GFPhysVector3 m_CenterOfMass;
	
	std::set<GFPhysCollideObject*> m_CollidedRigid;

	bool                    m_BForceSeparate;
	MisMedicOrgan_Ordinary* m_SeparateOrgan;

protected:

	std::set<MisMedicOrgan_Ordinary*> m_NeedleAnchorOrgans;
	std::multiset<GFPhysSoftBodyFace*> m_NeedleAnchorDisableFaces;
	void UpdateNeedleNodeWorldPos();

    Real           m_NeedleTotalLen;

    MisNewTraining * m_OwnerTraining;

	Real m_CollideRadius;

	GFPhysCompoundShape * m_NeedleCollideShape;

	GFPhysRigidBody * m_NeedlePhysBody;

	SutureThreadV2 * m_AttachedRope;

	MisMedicTubeShapeRendObject m_RendObject;
	
	Ogre::SceneNode* m_pNeedleSceneNode;

    GFPhysVectorObj<FaceAnchorInfoV2> m_FaceNeedleAnchors;

    GFPhysVectorObj<NeedleActionListenerV2*> m_Listeners;

	float m_NeedleOriginInvMass;
	GFPhysVector3 m_NeedleOriginInvTensor;

	CTool * m_RecentClampTool;//recently clamped tools
	std::set<CTool*> m_ClampeTools;//tool clamp this thread
};