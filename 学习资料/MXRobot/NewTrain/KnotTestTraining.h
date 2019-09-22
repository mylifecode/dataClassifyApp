#ifndef _KNOTTERTESTTRIAN_
#define _KNOTTERTESTTRIAN_
#include "MisNewTraining.h"
#include "PhysicsWrapper.h"
#include "MisMedicOrganOrdinary.h"

#include "TrainScoreSystem.h"
#include "CustomConstraint.h"
class MisMedicOrgan_Ordinary;
class VeinConnectObject;
class CElectricHook;
class CScissors;
class MisCTool_PluginClamp;
class CTool;
class MisMedicNeedle;
class MisMedicThreadKnot;
class CKnottTestTrain : public MisNewTraining
{

public:
	CKnottTestTrain(void);

	virtual ~CKnottTestTrain(void);

public:
	virtual bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);

	virtual bool Update(float dt);

	virtual void InternalSimulateStart(int currStep , int TotalStep , Real dt);

	virtual void InternalSimulateEnd(int currStep , int TotalStep , Real dt);

	virtual bool BeginRendOneFrame(float timeelapsed);//(const Ogre::FrameEvent& evt);

	virtual void onEndCheckCollide(GFPhysCollideObject * softobj , GFPhysCollideObject * rigidobj, const GFPhysAlignedVectorObj<GFPhysRSManifoldPoint> & contactPoints);//end check collision

	virtual void KeyPress(int key);

	//bacon add
	virtual void OnUserAction();

	//end
	virtual void onThreadConvexCollided( GFPhysCollideObject * rigidobj , 
		MisMedicThreadRope * rope ,
		int SegIndex,
		const GFPhysVector3 &   pointOnRigid,
		const GFPhysVector3 &   normalOnRigid,
		float depth,
		float weights
		);

	int FindSoftNodeIndex(GFPhysSoftBodyNode* p);

	void DrawDebugCollisionNormal(Ogre::ManualObject* pManualObj,GFPhysVector3& basePos,GFPhysVector3& normal);

	void AdjustNeedlePositionAndRotation();//for debug

	void AdjustToolPositionAndRotation(ITool * tool);

	void AddCollideConstraint(ClampPointConstraint* p);

	void ClearCollideConstaint();

private:

	void SelectPhysFaceAroundPoint(std::vector<GFPhysSoftBodyFace*> & ResultFaces , const GFPhysVector3 & pointPos , float radius , GFPhysSoftBody * sb , bool deformedspace = false);

	GFPhysRigidBody* GetClampedRigidBody(bool& bLeft);


	MisCTool_PluginClamp* GetToolPlugin(CTool* tool);

	bool m_bFinished;

	bool m_bSimulateAction;

	MisMedicThreadRope * m_threadObject;

	MisMedicThreadKnot * m_Knotter;
};

#endif