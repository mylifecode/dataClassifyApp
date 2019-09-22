#pragma once

#include "Instruments/MisMedicCToolPluginInterface.h"
#include "Dynamic/Constraint/GoPhysSoftBodyConstraint.h"
#include "Tool.h"
#include "Dynamic/Constraint/GoPhysFixJoint.h"
#include "Painting.h"

ATTRIBUTE_ALIGNED16(class) RigidClampCellData
{
public:
	GFPhysTransform m_transform;
	GFPhysVector3 m_localmin;
	GFPhysVector3 m_localmax;

	GFPhysVector3 m_worldmin;
	GFPhysVector3 m_worldmax;

	GFPhysVector3 m_CellVertsReg0[3];
	GFPhysVector3 m_CellVertsReg1[3];

	//
	GFPhysTransform m_InvTrans;//for fast use
	GFPhysVector3 m_CellVertsWorldSpace[6];

	bool m_AABBTriOverlap;//for use by selectorgan clamp
};

class MisCTool_PluginRigidHold : public MisMedicCToolPluginInterface// , public GFPhysSoftBodyConstraint
{
public:
    enum HoldRegSide
    {
        HoldReg_Right = 0,
        HoldReg_Left  = 1,
        HoldReg_UnKnow = 8,
    };
    class ToolHoldRegion
    {
    public:
        ToolHoldRegion()
        {
            m_normalSign = 1.0f;
            m_RegSide = HoldReg_UnKnow;
        }

        void UpdateToWorldSpace();

        GFPhysRigidBody * m_AttachRigid;

		GFPhysVector3 m_CenterLocal;
        GFPhysVector3 m_Axis0Local;//Coord0Local;
        GFPhysVector3 m_Axis1Local;//Coord1Local;
		float m_HalfExt0;	//delete
		float m_HalfExt1;	//delete

		float m_axis0Min, m_axis0Max;
		float m_axis1Min, m_axis1Max;
        GFPhysVector3 m_Axis0World;
        GFPhysVector3 m_Axis1World;
		GFPhysVector3 m_CenterWorldPrev;
		GFPhysVector3 m_CenterWorld;
        GFPhysVector3 m_HoldNormalWorld;

		//GFPhysVector3 m_Masscenter;

        //
        Real m_normalSign;		
        HoldRegSide m_RegSide;
		std::vector<Ogre::Vector2> m_triVertices;
    };

	GFPhysAlignedVectorObj<RigidClampCellData> m_RigidClampSpaceCells;
	Real          m_RigidClampRegThickNess;

	bool m_IsRect;


    MisCTool_PluginRigidHold(CTool * tool);

    ~MisCTool_PluginRigidHold();

	void OnRigidBodyBeRemovedFromWorld(GFPhysRigidBody * rigidbody);
	//void PhysicsSimulationStart(int currStep , int TotalStep , float dt);
    //void PrepareSolveConstraint(Real Stiffness,Real TimeStep);
   // void SolveConstraint(Real Stiffniss,Real TimeStep);	

	void OneFrameUpdateEnded();
	void UpdateRigidClampRegions();
    GFPhysRigidBody* GetRigidBodyHolded(){return m_RigidBodyBeHold;}
    GFPhysRigidBody* GetToolHold(){return m_RigidBodyTool;}    
    bool CheckRigidBodyBeHold();
	bool CalculateNeedleSegmentsInClampRegions();
	bool CalculateNeedleV2SegmentsInClampRegions();
    void ReleaseHoldingRigid();
    virtual void PhysicsSimulationStart(int currStep , int TotalStep , float dt);
    virtual void PhysicsSimulationEnd(int currStep , int TotalStep , float dt);
    const MisCTool_PluginRigidHold::ToolHoldRegion & GetHoldRegion(int index);
	void SetHoldRegion(NewTrainToolConvexData &  attachRigid,
		const GFPhysVector3 & center,
		const GFPhysVector3 & axis0,
		const GFPhysVector3 & axis1,
		Ogre::Vector2 triVertices[],
		int		numVertices,
		HoldRegSide regSide,
		Real    normalSign);
	inline bool GetRigidHoldState(){ return m_RigidBodyBeHold; }
    inline GFPhysVector3 GetRigidBodyHoldLocalPoint(){return m_localPointOther;}
	virtual GFPhysVector3 GetPluginForceFeedBack();
private:
    ToolHoldRegion m_HoldReg[2];
    Real m_minShaftAside;
	float m_OpenShaftAisde;
	bool  m_IsOpenLargeEnough;
    //bool m_holding;
    GFPhysQuaternion m_relRot;
    uint32 m_RigidMask;
    
    GFPhysRigidBody* m_RigidBodyBeHold;
    GFPhysRigidBody* m_RigidBodyTool;
    GFPhysVector3 m_localPointOther;
    GFPhysVector3 m_localPointTool;

	PaintingTool m_painttool;
//////////////////////////////////////////////////////////////////////////
    GFPhysFixJoint * m_fixjoint;
};

