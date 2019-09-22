#ifndef _SUTURENEEDLE_
#define _SUTURENEEDLE_
#include <Ogre.h>
#include "PhysicsWrapper.h"
#include "collision/CollisionShapes/GoPhysCompoundShape.h"
#include "MisMedicTubeShapeRendObject.h"
#include "SutureThread.h"
#include "TrainUtils.h"


using namespace GoPhys;
class MisNewTraining;
class SutureNeedle;

class NeedleActionListener
{
public:
    //Needle
    virtual void OnCreateInAnchor(const GFPhysSoftBodyFace* face,const Real weights[]) = 0 ;
    virtual void OnCreateOutAnchor(const GFPhysSoftBodyFace* face,const Real weights[]) = 0 ;
    virtual void OnRemoveInAnchor(const GFPhysSoftBodyFace* face,const Real weights[]) = 0 ;
    virtual void OnRemoveOutAnchor(const GFPhysSoftBodyFace* face,const Real weights[]) = 0 ;
    virtual void OnSwitchAnchor(const GFPhysSoftBodyFace* face, const Real weights[]) = 0;
    //Rope
    virtual void OnMovingRopeAnchor(const SutureThread * thread, const int index, const Real weights[]) = 0;
    virtual void OnRemoveRopeAnchor(const GFPhysSoftBodyFace* face,const Real weights[]) = 0 ;
    
};
//////////////////////////////////////////////////////////////////////////////////////      
enum AnchorType
{
    State_NULL,
    State_IN,
    State_OUT
};

struct AnchorPos
{
    int segment;    //0<=segment <=size-1
    Real lambda;   //0<=lambda<1
};

class GFPhysFaceNeedleAnchor// : public GFPhysPositionConstraint//GFPhysSoftBodyConstraint
{
public:
    GFPhysFaceNeedleAnchor(GFPhysVector3*,GFPhysSoftBodyFace*,Real*,SutureNeedle*,AnchorType);
    ~GFPhysFaceNeedleAnchor();
    void SetFriction(float friction);
    void PrepareSolveConstraint(Real Stiffness,Real TimeStep);
    void SolveConstraint(Real Stiffniss,Real TimeStep);

    GFPhysSoftBodyFace * GetFace(){return m_Face;}
	
    GFPhysVector3 GetRigidImpulse(){return m_RigidImpulse;}
    GFPhysVector3 GetLocalPoint(){return m_LocalPoint;}
    GFPhysVector3 GetWorldPoint(){return m_LastWorldPoint;}
    AnchorPos GetLastPos(){return m_lastworld;}
public:
	Real			     m_weights[3];
    bool                 m_BHeadOut;
    bool                 m_BTailOut;
	Real				 m_SuperfluousOffset;
    AnchorType           m_Type;
    Real                 m_initAngle;
	GFPhysVector3        m_ForceFeedBack;
	GFPhysVector3        m_ForceFeedBackAlongTan;
protected:
	//
	SutureNeedle*        m_SutNeedle;
    GFPhysSoftBodyFace*  m_Face;
    GFPhysRigidBody*	 m_Rigid;// Body
 
	//for calculate slip friction
	GFPhysVector3        m_TangentDir;
	GFPhysVector3        m_NormalDir[2];
	float                m_AccumFriction;
	int                  m_IterorCount;

	//for calculate rotate friction
	GFPhysQuaternion     m_RelRot;
	GFPhysQuaternion     m_FaceCurrOrient;
    float                m_AccmAngularFrict;

    
    GFPhysVector3		 m_LocalPoint;		// Anchor position in body space
    GFPhysVector3		 m_LastWorldPoint;  // 
    AnchorPos            m_lastworld;
	float                m_LastRation;

   
	//solver data
    GFPhysMatrix3		 m_ImpMat;			// Impulse matrix
    GFPhysVector3		 m_R;			    //anchor relative to Body's Mass center in w.c.s
	GFPhysVector3        m_RLocal;
   // Real				 m_DtInvMa;		    // ima*dt
    Real                 m_Friction;
    GFPhysVector3		 m_RigidImpulse;    
};

class GFPhysFaceRopeAnchor// : public GFPhysPositionConstraint//GFPhysSoftBodyConstraint
{
public:
	GFPhysFaceRopeAnchor(GFPhysSoftBodyFace*,Real*,SutureThread *,int,Real*);
    ~GFPhysFaceRopeAnchor();
	void SetFriction(float friction);
	void PrepareSolveConstraint(Real Stiffness,Real TimeStep);	
    void SolveConstraint(Real Stiffniss,Real TimeStep);

	GFPhysSoftBodyFace * GetFace(){return m_Face;}	
	
    int  GetSegIndex(){return m_NodeIndex;}
    Real GetSegPos()const { return m_NodeIndex + m_ThreadWeights[1]; }
	GFPhysVector3 GetAccumPressure() const;/* { return m_AccumPressure; }*/

	Real					m_weights[3];	
    int						m_NodeIndex;
    SutureThread *			m_thread;
    Real					m_ThreadWeights[2];
    Real                    m_AccumFriction;	
	AnchorType				m_type;
private:
	GFPhysSoftBodyFace*		m_Face;	
	Real					m_Friction;	 	
    GFPhysVector3           m_AccumPressure;
//////////////////////////////////////////////////////////////////////////
	GFPhysVector3			m_Last_Pos;
    GFPhysVector3           m_Tangent;
	int                     m_IterorCount;
};
class SutureRopeAttachPoint
{
public:
    int m_PointIndex;

    float m_InvStiff;

    GFPhysVector3 m_LocalAttachPt;//in thread

    GFPhysMatrix3 m_ImpMat;//solver data

    GFPhysVector3 m_R;//solver data

	GFPhysVector3 m_Lambda;
	float m_InvStiffDivDt;
    float m_DtInvMa;//solver data
};
//===================================================================================================================

struct FaceAnchorInfo
{
    MisMedicOrgan_Ordinary* POrgan;
    GFPhysFaceNeedleAnchor* pAnchor;   
    std::vector<GFPhysSoftBodyFace*> pFacesVector;
};

//===================================================================================================================
class SutureNeedle : public GFPhysPositionConstraint
{
public:
	SutureNeedle();
	
	//SutureNeedle(MisNewTraining * newTrain);

	~SutureNeedle();

    void CreateSutureNeedle(MisNewTraining * newTrain, int threadnodenum, Real restlen, const Ogre::String & needleskeleton);

    void Disappear();

	void CalcNeedleMassProperty(GFPhysVector3 NodePos[] , 
								int   NumNode , 
								float massPerNode ,
								float collideradius,
								float & totalMass ,
								GFPhysVector3 & com , 
								GFPhysMatrix3 & inertiaTesnor);
	
	inline GFPhysRigidBody* GetPhysicBody() { return m_NeedlePhysBody; }

    inline SutureThread* GetSutureThread() {return m_AttachedRope; }

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

    void AddAttachPoint(const GFPhysVector3 & attachlocal , int NodeIndex , float stiffness);
	GFPhysVector3 m_vMeshPosOffset;

    /*退针*/
    void RemoveHeadAnchor();//针尖退出约束    

    /*正常出针*/
    void RemoveTailAnchor();//针尾删除入针约束    

	/*正常出线*/	
	void RemoveThreadAnchor();//线尾删除出线约束
	
	void CreateRopeAnchor(AnchorType type, GFPhysSoftBodyFace* face, Real * weights, Real offset);
    
    void SynchronizeState();
    Real AnchorPos2Ratio(const AnchorPos& pos1);
    AnchorPos Ratio2AnchorPos(const Real& ratio);
	Real ConvertRealDistToRatioDist(Real realdist);

	GFPhysVector3 GetNeedleSegmentTangent(int segment , float lambda);

    AnchorPos CalcNewNeedlePos(const AnchorPos& pos1,const AnchorPos& pos2,const Real& friction);
    void NormalizeFaceNeedleAnchor();
    void NormalizeFaceRopeAnchor();

    inline GFPhysVectorObj<FaceAnchorInfo>& getFaceNeedleAnchors(){return m_FaceNeedleAnchors;}

	void OnRigidBodyCollided(GFPhysCollideObject * ra,
		                     GFPhysCollideObject * rb,
							 const GFPhysManifoldPoint * contactPoints, 
							 int NumContactPoints);

	void OnRigidBodySeperate(GFPhysCollideObject * ra,
		                     GFPhysCollideObject * rb);

	GFPhysVector3 GetForceFeedBack();
    bool Getinterval(const GFPhysVector3& pos, int& i, int& j);
public:
    void addNeedleActionListener( NeedleActionListener* listener );
    void removeNeedleActionListener(NeedleActionListener * listener);
    void removeAllNeedleActionListener();
    
    void notifyCreateInAnchor(const GFPhysSoftBodyFace* face,const Real weights[]);
    void notifyCreateOutAnchor(const GFPhysSoftBodyFace* face,const Real weights[]);
    void notifyRemoveInAnchor(const GFPhysSoftBodyFace* face,const Real weights[]);
    void notifyRemoveOutAnchor(const GFPhysSoftBodyFace* face,const Real weights[]);
    void notifySwitchAnchor(const GFPhysSoftBodyFace* face, const Real weights[]);
    void notifyMovingRopeAnchor(const SutureThread * thread,const int index, const Real weights[]);
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
	//============================================================================

    void GetFaceRopeAnchors(GFPhysVectorObj<GFPhysFaceRopeAnchor*> & ropeanchors)
    {
        for (size_t c = 0; c < m_FaceRopeAnchors.size(); c++)
        {
            ropeanchors.push_back(m_FaceRopeAnchors[c]);
        }
    }
    void SetFaceRopeAnchors(GFPhysVectorObj<GFPhysFaceRopeAnchor*> & ropeanchors)
    {
        for (size_t c = 0; c < ropeanchors.size(); c++)
        {
            m_FaceRopeAnchors.push_back(ropeanchors[c]);
        }
    }
public:

    GFPhysAlignedVectorObj<GFPhysVector3> m_NeedleNodeWorldPos;
    GFPhysAlignedVectorObj<GFPhysVector3> m_NeedleNodeLocalPos;

	GFPhysVector3 m_CenterOfMass;
	std::set<GFPhysCollideObject*> m_CollidedRigid;
    //GFPhysFaceNeedleAnchor* m_CurrFaceNeedleAnchor;

	bool                    m_BForceSeparate;
	MisMedicOrgan_Ordinary* m_SeparateOrgan;

protected:

	std::set<MisMedicOrgan_Ordinary*> m_NeedleAnchorOrgans;
	std::multiset<GFPhysSoftBodyFace*> m_NeedleAnchorDisableFaces;
	void UpdateNeedleNodeWorldPos();

    Real           m_NeedleTotalLen;

    MisNewTraining * m_OwnerTraining;

	float m_CollideRadius;

	GFPhysCompoundShape * m_NeedleCollideShape;

	GFPhysRigidBody * m_NeedlePhysBody;

	SutureThread * m_AttachedRope;

	MisMedicTubeShapeRendObject m_RendObject;
	
	Ogre::SceneNode* m_pNeedleSceneNode;

    GFPhysAlignedVectorObj<SutureRopeAttachPoint> m_AttchPoints;

    GFPhysVectorObj<FaceAnchorInfo> m_FaceNeedleAnchors;

    GFPhysVectorObj<GFPhysFaceRopeAnchor*> m_FaceRopeAnchors;

    GFPhysVectorObj<NeedleActionListener*> m_Listeners;

	float m_NeedleOriginInvMass;
	GFPhysVector3 m_NeedleOriginInvTensor;

	CTool * m_RecentClampTool;//recently clamped tools
	std::set<CTool*> m_ClampeTools;//tool clamp this thread
};

#endif