#ifndef _MISCTOOL_PLUGINHOOK_
#define _MISCTOOL_PLUGINHOOK_
#include "MisMedicCToolPluginInterface.h"

class MisCTool_PluginHook : public MisMedicCToolPluginInterface , public GFPhysSoftBodyConstraint
{
public:

	class FaceBeHookeed
	{
	public:
		FaceBeHookeed()
		{
			m_HookImpluse = GFPhysVector3(0, 0, 0);
		}
		GFPhysSoftBody * m_SoftBody;
		GFPhysSoftBodyFace * m_PhysFace;
		//GFPhysSoftBodyNode * m_Node;
		float m_PointInFaceWeight[3];

		GFPhysRigidBody * m_RigidBody;
		GFPhysVector3 m_PointInRigidLocal;

		std::vector<int> m_HookedFaceUID;

		GFPhysVector3 m_HookImpluse;
	};

	MisCTool_PluginHook(CTool * tool , GFPhysRigidBody * hookPart);

	~MisCTool_PluginHook();

	GFPhysVector3 GetPluginForceFeedBack();

	void UpdateHinHookLine(const GFPhysVector3 & pointHead , const GFPhysVector3 & pointEnd , const GFPhysVector3 & hintOffsetVec);
	
	void ToolElectriced(int touchtype , float dt);
	
	void ElectricCutAtHookPoint();

	void OneFrameUpdateStarted(float timeelapsed);

	void OneFrameUpdateEnded();

	virtual void PrepareSolveConstraint(Real Stiffness,Real TimeStep);

	virtual void SolveConstraint(Real Stiffness,Real TimeStep);

	void onRSContactsBuildBegin(ConvexSoftFaceCollidePair * collidePairs , int NumCollidePair);

	//when a physics update frame end
	virtual void PhysicsSimulationEnd(int currStep , int TotalStep , float dt);

	//when a ordinary frame update end
    void onRSFaceContactsBuildFinish( GFPhysAlignedVectorObj<GFPhysSoftFaceRigidContact> & RSContactConstraints);

	void onRSFaceContactsSolveEnded(const GFPhysAlignedVectorObj<GFPhysSoftFaceRigidContact> & RSContactConstraints);
 
	GFPhysAlignedVectorObj<FaceBeHookeed> m_FaceBeHooked;

private:
	GFPhysRigidBody * m_HookPart;
	GFPhysVector3 m_HookDir;//currently need manually update
	GFPhysVector3 m_HookLineHead;
	GFPhysVector3 m_HookLineEnd;
	GFPhysVector3 m_HookHintOffset;
	void ReleaseHook();

	float m_TimeSinceLastRelease;

	float m_ElectricKeepTime;
};

#endif