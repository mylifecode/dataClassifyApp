#ifndef _MisMedicTubeBody_
#define _MisMedicTubeBody_
#include <Ogre.h>
#include "Dynamic/GoPhysDynamicLib.h"
#include "PhysicsWrapper.h"
#include "MisMedicTubeShapeRendObject.h"
#include "MisMedicOrganInterface.h"
using namespace GoPhys;

class MisMedicOrgan_Ordinary;
class MisNewTraining;
class MisMedicTubeBody;

enum  TubeSegmentState
{
    TUBEST_INKNOT = 1,
    TUBEST_INSELFCOLLISION = 1<<1,
};

class TubeNode
{
public:TubeNode(const GFPhysVector3 & position)
       {
           m_UnDeformedPos = m_CurrPosition = m_LastPosition = position;
           m_InvMass = 1.0f;
           m_Velocity = GFPhysVector3(0,0,0);
           m_bCollideRigid = m_bCollideSoft = true;
           m_bCanSelfCollision = true;
           m_FrameCollideTag = false;
           m_bAttached = false;
       }	
       TubeNode()
       {
           m_bCollideRigid = m_bCollideSoft = true;
           m_bCanSelfCollision = true;
           m_FrameCollideTag = false;
           m_bAttached = false;
       }
       GFPhysVector3 m_CurrPosition;
       GFPhysVector3 m_LastPosition;
       GFPhysVector3 m_UnDeformedPos;

       GFPhysVector3 m_Velocity;
       float m_InvMass;
       bool  m_FrameCollideTag;

       bool  m_bCollideRigid;
       bool  m_bCollideSoft;

       bool  m_bCanSelfCollision;

       bool  m_bAttached;//是否被夹取，附着

       GFPhysVector3 m_Force0;
       GFPhysVector3 m_Force1;
};
//Tube segment vs rigid body collision
class TUBERIGIDCollidePair
{
public:
    TUBERIGIDCollidePair(
        GFPhysRigidBody * rigid,
        const GFPhysVector3 & rigidWorldPt ,
        const GFPhysVector3 & rigidNormal,
        float SegmentWeight,
        float depth,
        int segIndex
        )
    {
        m_Rigid = rigid;

        m_RigidWorldPoint = rigidWorldPt;
        m_NormalOnRigid = rigidNormal;
        m_SegWeight = SegmentWeight;

        m_Depth = depth;
        m_Segment = segIndex;
        m_TubeImpulseDeta[0] = m_TubeImpulseDeta[1] = 0;
        m_ImpluseNormalOnRigid = 0;
    }

    GFPhysRigidBody * m_Rigid;

    GFPhysVector3 m_RigidWorldPoint;

    GFPhysVector3 m_NormalOnRigid;

    float m_Depth;

    float m_SegWeight;//s[2];//point weights in segment

    int   m_Segment;

    float m_TubeImpulseDeta[2];

    float m_ImpluseNormalOnRigid;
};
//Tube segment vs soft face collision
class TUBESOFTCollidePair
{
public:
    enum TFCollideType
    {
        TFCD_EE,//Edge edge
        TFCD_VF,//vertex face
        TFCD_SF,//segment face
    };
    TUBESOFTCollidePair(GFPhysSoftBody * sb);
    TUBESOFTCollidePair(GFPhysSoftBody * sb , GFPhysSoftBodyFace * face);
    TUBESOFTCollidePair(GFPhysSoftBody * sb , GFPhysSoftBodyFace * face , MisMedicTubeBody * tube,int segment);//add a constructor

    int GetCollideSegmentIndex()  const;
    GFPhysSoftBody * GetCollideBody() const;
    GFPhysSoftBodyFace * GetCollideSoftFace()  const;

    //mark as edge-edge collision pair
    void SetEE_Collision(int e1 , int e2 , int e3 , int e4 , const Ogre::Vector3 & collideNormal , const Ogre::Vector3 & FaceNormal);

    //mark as node-face collision pair
    void SetVF_Collision(int nt , const Ogre::Vector3 & collideNormal , const Ogre::Vector3 & FaceNormal);
    
    //mark as segment-face collision pair
    void SetSF_Collision(const GFPhysVector3 ClosetPoints[2],const Real & depth,const GFPhysVector3 & collideormal);

    TFCollideType m_CollideType;
    int m_e1;//
    int m_e2;//
    int m_e3;//
    int m_e4;//
    int m_e5;//

    int m_SegIndex;
    MisMedicTubeBody * m_Tube;
    
    Ogre::Vector3 m_ImpluseOnTube;
    GFPhysSoftBody * m_CollideBody;
    
    GFPhysSoftBodyFace * m_SoftFace;
    Ogre::Vector3 m_FaceNormal;
    Ogre::Vector3 m_CollideNormal;//

    //GFPhysSoftBodyNode * m_CollideNode[4];
    GFPhysSoftBodyNode * m_CollideNode[5];
    int   m_NumCollideNode;
    //float m_CollideNodeDepth[4];
    float m_CollideNodeDepth[5];
    float m_CollideDepth;
    float m_FaceWeihts[3];
    float m_TubeWeigths[2];
};

//Tube segment vs Tube segment collision
class TUBETUBECollidePair
{
public:
    TUBETUBECollidePair(MisMedicTubeBody * tubeA ,
        MisMedicTubeBody * tubeB , 
        int SegmentA,
        int SegmentB);

    int GetCollideSegmentA()  const;
    int GetCollideSegmentB()  const;

    int m_SegmentA;
    int m_SegmentB;

    float m_CollideDist;
    float m_WeightA;//collide position is pos0*(1-weight0) + pos * weight
    float m_WeightB;
    MisMedicTubeBody * m_TubeA;
    MisMedicTubeBody * m_TubeB;

    GFPhysVector3 m_NormalOnB;
};
class MisMedicTubeBody:public GFPhysSoftBodyConstraint//currently only rend part
{
public:
    enum TubeTopoType
    {
        TTT_LOOP,//线圈
        TTT_FIXONEEND,//单端固定线
        TTT_FREE
    };

    MisMedicTubeBody(Ogre::SceneManager * sceneMgr , MisNewTraining * ownertrain);

    ~MisMedicTubeBody();

    void SetStretchStiffness(float);

    void SetBendingStiffness(float);

    void SetGravity(const GFPhysVector3 & gravity);

    GFPhysVector3 CalcLoopForceFeedBack();

    void DestoryTube();

    /*bool CutTubeByTool(CTool * toolToCut);*/

    void AttachNodePointToFace(GFPhysSoftBodyFace * attachFace , float weights[3]);

    void DetachNodePointFromFace();

    void SimulateTubePhysics(float dt);

    void EndSimuolateTubePhysics(float dt);

    bool GetTubeSegmentNode(TubeNode & n0 , TubeNode & n1 , int segIndex);

    bool GetTubeSegmentNodePos(GFPhysVector3 & n0 , GFPhysVector3 & n1 , int segIndex);

    virtual void SolveAdditionConstraint();

    virtual void UpdateFixedNodes();

    void  SetUnitLen(float);

    float GetUnitLen();

    float GetTotalLen(bool deformed);

    //float GetBindLen();

    TubeTopoType GetTubeTopoType();

    int GetNumTubeNodes();

    int GetNumSegments();

    TubeNode GetTubeNode(int NodeIndex);

    TubeNode & GetTubeNodeRef(int NodeIndex);

    const std::vector<TUBESOFTCollidePair> & GetCollidePairs();

    /*@@ total freed Tube*/
    void CreateFreeTube(const GFPhysVector3 & StartFixPoint , const GFPhysVector3 & EndFixPoint , int segmetnCount);

    void UpdateMesh();

    void UpdateMeshByCustomedRendNodes(std::vector<Ogre::Vector3> & rendPoints);

    float GetCollideRadius()
    {
        return m_TubeCollideRadius;
    }

    void SetRendRadius(float rendradius)
    {
        m_TubeRendRadius = m_TubeCollideRadius;
    }

    void EnableSelfCollision()
    {
        m_EnableSelfCollision = true;
    }

    void DisableSelfCollision()
    {
        m_EnableSelfCollision = false;
    }
    GFPhysVector3& GetTubeNodePositionByIndex(int index);
    bool SetTubeNodePositionByIndex(int index,GFPhysVector3& pos);
    bool IsCollideWithOrgan(Ogre::Plane* plane);

    Ogre::Vector3 GetTubeDir();

    bool IsCutAfterBound() { return m_TopoType == TTT_FIXONEEND && m_IsCutAfterBound; }

    const GFPhysDBVTree & GetSegmentBVTree()
    {
        return m_SegmentBVTree;
    }
    GFPhysVector3 m_ForceFeed;

    bool  m_NeedRend;

    bool  m_bAttached;//是否被夹取，附着

    float m_TubeRSFriction;

    float m_Margin;

	float m_TubeCollideRadius;
	float m_TubeRendRadius;

    float m_SolveMassScale;

    std::vector<int> m_SegmentState;

    bool m_RendSegementTag;

    bool m_UseBendForce;
protected:
    float m_TubeFriction;


    MisNewTraining * m_ownertrain;

    MisMedicTubeShapeRendObject m_RendObject;

    bool m_UseCCD;

    bool m_EnableSelfCollision;//default false

    float m_TubeBendStiffness;

    float m_TubeStrechStiffness;

    int  m_SingleItorNum;//this is firs time itor num
    int  m_TotalItorNum;//Tube iterator num may large to solver num(typically 14 iterator num)
    int  m_TimesPerItor;
    //@@ solver function solver Tube bend and stretch
    void SolveBend(TubeNode & t0 , TubeNode & t1 , TubeNode & t2 , float Stiffness);
    GFPhysVector3 SolveStretch(TubeNode & t0 , TubeNode & t1 , float Stiffness , int interval);

    //@@ solver function solve Tube - soft body collision
    GFPhysVector3 SolveEECollide(const GFPhysVector3 & collideNormal , GFPhysSoftBodyNode * e1 , GFPhysSoftBodyNode * e2 , TubeNode & e3 , TubeNode & e4 , const TUBESOFTCollidePair & cdPair);
    GFPhysVector3 SolveVTCollide(const GFPhysVector3 & collideNormal , GFPhysSoftBodyNode * n1 , GFPhysSoftBodyNode * n2 , GFPhysSoftBodyNode * n3 , TubeNode & tn , const TUBESOFTCollidePair & cdPair);
    GFPhysVector3 SolveSFCollide(const GFPhysVector3 & collideNormal , GFPhysSoftBodyNode * n1 , GFPhysSoftBodyNode * n2 , GFPhysSoftBodyNode * n3 , TubeNode & tn1 ,TubeNode & tn2 , const TUBESOFTCollidePair & cdPair);

    //@ solve Tube attach face 
    void SolveAttachment(GFPhysSoftBodyFace * attachFace , float weights[3] , float stiffness);

    void SolveSoftTubeCollisions();

    virtual void PrepareSolveConstraint(Real Stiffness,Real TimeStep);

    virtual void SolveConstraint(Real Stiffness,Real TimeStep);
    //unit length of the Tube segment
    uint32 m_Catergory;
    GFPhysVector3 m_Gravity;
    float m_UnitLen;
    
    std::vector<TUBESOFTCollidePair> m_TUBESOFTCollidePair;//Tube soft face collide pair
    GFPhysAlignedVectorObj<TubeNode> m_TubeNodes;//node information of the Tube
  
    std::vector<int> m_FixedNodaIndex;
    TubeTopoType   m_TopoType;//type of the Tube
    TubeNode       m_VirtualStickNode;//virtual node in tool
    Ogre::Vector3    m_LoopAxisLocal;
    GFPhysSoftBodyFace * m_AttachedFace;
    float m_AttachWeight[3];
    GFPhysDBVTree m_SegmentBVTree;


    //rend part start
    Ogre::SceneNode * m_ToolKernalNode;//挂在器械上的节点 仅当线圈模式有效

    bool m_IsCutAfterBound;

    float * m_InvMassArray;
    GFPhysVector3 * m_CurrPosArray;
    GFPhysVector3 * m_UndeformPosArray;
    int m_NumSMNode;
};

#endif // _MisMedicTubeBody_


