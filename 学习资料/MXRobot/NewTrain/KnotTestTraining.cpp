#include "KnotTestTraining.h"
#include "MisMedicOrganOrdinary.h"
#include "MXEventsDump.h"
#include "MXToolEvent.h"
#include "OgreAxisAlignedBox.h"
#include "Instruments/Tool.h"
#include "Instruments/MisCTool_PluginClamp.h"
#include "InputSystem.h"
#include "KeyboardInput.h"
#include "MisMedicThreadKnot.h"
//=============================================================================================
CKnottTestTrain::CKnottTestTrain(void) : m_bFinished(false),m_threadObject(0)
{
	m_Knotter = 0;
	m_bSimulateAction = true;
}
//=============================================================================================
CKnottTestTrain::~CKnottTestTrain(void)
{
	if(m_Knotter)
	{
		delete m_Knotter;
		m_Knotter = 0;
	}
	//delete m_Needle;
	//m_Needle = 0;
}

//======================================================================================================================
bool CKnottTestTrain::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{
	bool result = MisNewTraining::Initialize(pTrainingConfig , pToolConfig);

	//create thread object
	GFPhysVector3 threadStartWorld(-8.0f, 1, -2.0f);
	GFPhysVector3 threadDirWorld(1,0,0);
	threadDirWorld.Normalize();

	GFPhysVector3 threadEndWorld = threadStartWorld + threadDirWorld*14.0f;//GFPhysVector3(14, 0 , 0);

	m_threadObject = CreateRopeThread(MXOgre_SCENEMANAGER);
	m_threadObject->CreateFreeThread(threadStartWorld , threadEndWorld , 60 , 1.0f);//(GFPhysVector3(2 , 5, 0) , GFPhysVector3(10 , 5, 0) , 20);
	m_threadObject->SetGravity(GFPhysVector3(0,-10,0));
	m_threadObject->GetThreadNodeRef(0).SetCanCollideRigid(false);
	m_threadObject->GetThreadNodeRef(0).SetInvMass(0.0f);
	
	//m_threadObject->GetThreadNodeRef(m_threadObject->GetNumThreadNodes()-1).SetCanCollideRigid(false);
	//m_threadObject->GetThreadNodeRef(m_threadObject->GetNumThreadNodes()-1).SetInvMass(0.0f);

	m_threadObject->SetRendRadius(m_threadObject->GetCollideRadius()*0.75f);
	m_threadObject->m_RopeRSFriction = 0.04f;
	//m_threadObject->SetRendRadius(m_threadObject->GetCollideRadius());
	m_threadObject->SetBendingStiffness(0.9f);
	m_threadObject->SetStretchStiffness(0.99f);
	m_threadObject->SetRigidForceRange(5);
	m_threadObject->m_RendSegementTag = true;
	m_threadObject->m_UseBendForce = false;
	m_Knotter = new MisMedicThreadKnot();
	m_Knotter->m_ThreadObject = m_threadObject;
	m_threadObject->m_NeedRend = false;
	m_threadObject->m_DampingRate = 4.0f;
	//also shift local attach point
	
	//DynObjMap::iterator itor = m_DynObjMap.begin();

	
	//m_Needle = new MisMedicNeedle();
	//m_Needle->CreateTestNeedle(this);


	//GFPhysVector3 aabbmin;
	//GFPhysVector3 aabbmax;
	//GFPhysVector3 initpos= m_pNeedleTestOrgan->m_physbody->GetWorldTransform().GetOrigin();
	//m_pNeedleTestOrgan->m_physbody->GetWorldAabb(aabbmin,aabbmax);
	//m_Needle->CreateOrganBBox(initpos,aabbmin,aabbmax);
	return result;
}
//======================================================================================================================
void CKnottTestTrain::onEndCheckCollide(GFPhysCollideObject * softobj , GFPhysCollideObject * rigidobj, const GFPhysAlignedVectorObj<GFPhysRSManifoldPoint> & contactPoints)
{
	
}

//======================================================================================================================
void CKnottTestTrain::AdjustToolPositionAndRotation(ITool * tool)
{
	
}
//======================================================================================================================
void CKnottTestTrain::OnUserAction()
{
	
}
//======================================================================================================================
GFPhysRigidBody* CKnottTestTrain::GetClampedRigidBody(bool& bLeft)
{
	return 0;
}
//======================================================================================================================
bool CKnottTestTrain::BeginRendOneFrame(float timeelapsed)//(const Ogre::FrameEvent& evt)
{
	bool result = MisNewTraining::BeginRendOneFrame(timeelapsed);
	return result;
}
//======================================================================================================================
bool CKnottTestTrain::Update( float dt )
{
	bool result = MisNewTraining::Update(dt);

	if(m_Knotter)
	   m_Knotter->Update(dt);

	//if(m_threadObject)
	 //  m_threadObject->UpdateMesh();

	return result;
}
//======================================================================================================================
void CKnottTestTrain::InternalSimulateStart(int currStep , int TotalStep , Real dt)
{
	MisNewTraining::InternalSimulateStart( currStep ,  TotalStep ,  dt);
	//m_Needle->InternalSimulationStart( currStep ,  TotalStep ,  dt);

}
//======================================================================================================================
void CKnottTestTrain::InternalSimulateEnd(int currStep , int TotalStep , Real dt)
{
	MisNewTraining::InternalSimulateEnd( currStep ,  TotalStep ,  dt);
	//m_Needle->InternalSimulationEnd( currStep ,  TotalStep ,  dt);
}
//keyboard message
void CKnottTestTrain::KeyPress(int whichButton)
{
	if(m_threadObject && whichButton == 0x51)
	{
		m_threadObject->SetBendingStiffness(0.1f);
	}
}
//======================================================================================================================
void CKnottTestTrain::onThreadConvexCollided( GFPhysCollideObject * rigidobj , 
											  MisMedicThreadRope * rope ,
											  int SegIndex,
											  const GFPhysVector3 &   pointOnRigid,
											  const GFPhysVector3 &   normalOnRigid,
											  float depth,
											  float weights
													)
{
	std::vector<CTool*> ToolsInUse;
	ToolsInUse.push_back((CTool*)m_pToolsMgr->GetLeftTool());
	ToolsInUse.push_back((CTool*)m_pToolsMgr->GetRightTool());

	for(size_t t = 0 ; t < ToolsInUse.size() ; t++)
	{
		CTool * tool = ToolsInUse[t];
		if(tool)
		   tool->OnThreadSegmentCollided(rigidobj , rope, SegIndex , pointOnRigid , normalOnRigid , weights , depth);//RSContactConstraints);
	}
}