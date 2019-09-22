#ifndef _SUTUREKNOTTRAIN_
#define _SUTUREKNOTTRAIN_
#include "MisNewTraining.h"
#include "PhysicsWrapper.h"
#include "MisMedicOrganOrdinary.h"
//#include "MisMedicOrganTube.h"
#include "TrainScoreSystem.h"
#include "CustomConstraint.h"
#include "Memory\GoPhysAlignedObjectArray.h"
#include "MisMedicSutureKnot.h"
#include "SutureNeedle.h"
#include "MisMedicEffectRender.h"

class MisMedicOrgan_Ordinary;
class MisCTool_PluginClamp;
class CTool;
class SutureThread;



//=============================================================================================
class CSutureKnotTrain : public MisNewTraining, public NeedleActionListener
{

public:
	CSutureKnotTrain(void);

	virtual ~CSutureKnotTrain(void);

public:
	virtual bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);
    void InitBallPos();
    bool Update(float dt);
	void OnThreadClampByTool(int ClampedSegGlobalIndex, CTool* toolobject);
	void OnThreadReleaseByTool(int ClampedSegGlobalIndex, CTool* toolobject);
	virtual void ResetNeedle();

    void InternalSimulateStart(int currStep , int TotalStep , Real dt);
    void InternalSimulateEnd(int currStep , int TotalStep , Real dt);

	bool checkCircleOnTool(SutureNeedle* needle, Real threshold);

private:
    void CheckAdjust_Tail();
    void CountThreadSeperate();


private:
    void OnCreateInAnchor(const GFPhysSoftBodyFace* face, const Real weights[]);
    void OnCreateOutAnchor(const GFPhysSoftBodyFace* face, const Real weights[]);
    void OnRemoveInAnchor(const GFPhysSoftBodyFace* face, const Real weights[]);
    void OnRemoveOutAnchor(const GFPhysSoftBodyFace* face, const Real weights[]);
    void OnSwitchAnchor(const GFPhysSoftBodyFace* face, const Real weights[]);
    void OnMovingRopeAnchor(const SutureThread * thread, const int index, const Real weights[]);
    void OnRemoveRopeAnchor(const GFPhysSoftBodyFace* face, const Real weights[]);

private:
    GFPhysVector3 CalcNormalFromCenter(SutureNeedle* needle, const TRCollidePair & pair, const GFPhysVector3 & Pos, const GFPhysVector3 & dir, GFPhysVector3 & foot);   
    void OnRigidClampByTool(GFPhysRigidBody * rigid);
    void OnRigidReleaseByTool(GFPhysRigidBody * rigid);
    void OnSimpleUIEvent(const SimpleUIEvent & event);
    

protected:

    SutureNeedle*  m_Needle[2];
    SutureKnot*    m_Knot[2];

    MisMedicOrgan_Ordinary*  m_pKnotTestOrgan0;
    MisMedicOrgan_Ordinary*  m_pKnotTestOrgan1;
    MisMedicOrgan_Ordinary*  m_pNeedleTestOrgan;
	GFPhysRigidBody* m_pBaseTerrainRigidBody; 


    Real                m_CircleThreshold;
    bool                m_bCircleKnot;
    bool                m_bAdjust_Tail[2];
    bool                m_ThreadOrganConnect;
    unsigned int        m_ThreadSeperateCount;
    bool                m_threadorganconnectLast;
};


class CSutureKnotTrain1 :public CSutureKnotTrain //square knot
{
public:    
    virtual bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);
    void DrawMarkRenderTarget();///每帧都清空
    void InitBallPos();
    void UpdateBaseBallPos();    
    bool Update(float dt);
    void OnThreadClampByTool(int ClampedSegGlobalIndex, CTool* toolobject);
    void OnThreadReleaseByTool(int ClampedSegGlobalIndex, CTool* toolobject);
    void OnRigidClampByTool(GFPhysRigidBody * rigid);
    void OnRigidReleaseByTool(GFPhysRigidBody * rigid);    
    void ResetNeedle();
    void OnSaveTrainingReport();
    void Embed();
protected:
    MarkTextureState m_MarkState[2];    
    bool m_bGraspNeedle;
    bool m_bFirstHalfKnotFinish;
    
    bool m_bFinished;
    bool m_bScoreKnot[2];
};
class CSutureKnotTrain2 :public CSutureKnotTrain1 //surgeons knot
{
public:
    virtual bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);
    bool Update(float dt);
};
class CSutureKnotTrain3 :public CSutureKnotTrain //interrupted suturing
{
    struct InoutBallInfo
    {
        MarkTextureState        dynamicMark;
        MisMedicOrgan_Ordinary* Organ;
        GFPhysSoftBodyFace*		InoutFace;		//入针出针球位置   
        Real					InoutWeights0;
        Real					InoutWeights1;
        Real					InoutWeights2;
    };

public:
    virtual bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);
    void InitBallPos();
    void DrawMarkRenderTarget();///每帧都清空
    void UpdateBaseBallPos();
    void ResetNeedle();
    bool Update(float dt);
    void OnCreateInAnchor(const GFPhysSoftBodyFace* face, const Real weights[]);
    void OnCreateOutAnchor(const GFPhysSoftBodyFace* face, const Real weights[]);
    void OnRemoveInAnchor(const GFPhysSoftBodyFace* face, const Real weights[]);
    void OnRemoveOutAnchor(const GFPhysSoftBodyFace* face, const Real weights[]);
    void OnRemoveRopeAnchor(const GFPhysSoftBodyFace* face, const Real weights[]);

        
    void UpdateStaticMark(const GFPhysVector3& anchorpos);
    void CreatedynamicMark(const GFPhysSoftBodyFace* face, const Real weights[]);
    void DeletedynamicMark(const GFPhysSoftBodyFace* face);

    void OnThreadClampByTool(int ClampedSegGlobalIndex, CTool* toolobject);
    void OnThreadReleaseByTool(int ClampedSegGlobalIndex, CTool* toolobject);
    void OnRigidClampByTool(GFPhysRigidBody * rigid);
    void OnRigidReleaseByTool(GFPhysRigidBody * rigid);

    void CreateAnotherNeedle();

    void OnSaveTrainingReport();
    void GatherPosInfo();
    void DetectNeedleFall();
    void OnCollisionKeep(GFPhysCollideObject * objA, GFPhysCollideObject * objB, const GFPhysManifoldPoint * contactPoints, int NumContactPoints);

	SutureThreadV2* m_sutureV2;
	SutureThread* m_sutureV1;
private:
    MarkTextureState        m_StaticMark[8];    
    unsigned int            m_index[8];
    std::vector<InoutBallInfo>           m_ballinfo;

    bool m_bGraspNeedle;
    bool m_bFirstHalfKnotFinish;
    
    bool m_bFinished;
    bool m_bScoreKnot[4];
    
    Real m_HolderAngle[2];
    Real m_InjectAngle[2];
    Real m_WithDrawAngle[2];

    bool m_NeedleFallStateLast;//上一帧针掉落触地状态
    bool m_NeedleFallState;//当前针掉落触地状态
    int	 m_NeedleFallCount;//针掉落次数
    //////////////////////////////////////////////////////////////////////////
    bool HoldPos[2];//in callback
    GradeType HoldAngle[2];//in callback
    GradeType InjectPos[2];//save
    GradeType InjectAngle[2];//in callback
    GradeType WithDrawPos[2];//save
    GradeType WithDrawAngle[2];//out callback


};
#endif