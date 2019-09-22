/**Author:zx**/
#include "Knotter.h"
#include "XMLWrapperTool.h"
#include "MisMedicThreadRope.h"
#include "MisMedicOrganOrdinary.h"
#include "IObjDefine.h"
#include "MisMedicThreadRope.h"
#include "MisMedicBindedRope.h"
#include "MisNewTraining.h"
#include "collision/NarrowPhase/GoPhysPrimitiveTest.h"
#include "MxOrganBindedEvent.h"

CKnotter::CKnotter() : m_Looper(0) , m_OrganBindRope(0) , m_LoopControlRegion(0)
{
	m_OrganBinded = 0;
	m_ThreadState = TS_NONE;
	m_ShaftStateForTighten = CKnotter::SFT_DICARD;
}

CKnotter::CKnotter(CXMLWrapperTool * pToolConfig) : CTool(pToolConfig) , m_Looper(0) , m_OrganBindRope(0) , m_LoopControlRegion(0)
{
	m_OrganBinded = 0;
	m_ThreadState = TS_NONE;
	m_ShaftStateForTighten = CKnotter::SFT_DICARD;
}

CKnotter::~CKnotter()
{
	MisNewTraining * newTrain = dynamic_cast<MisNewTraining*>(m_pOwnerTraining);
	if(newTrain)
	{
		if(m_Looper)
		{
		   newTrain->RemoveThreadRopeFromWorld(m_Looper);
		   m_Looper = 0;
		}

		if(m_OrganBindRope)//do not use delete this object's memeory belong organ ordinary
			m_OrganBindRope->SetUnConnected();
	}
}

std::string CKnotter::GetCollisionConfigEntryName()
{
	return "Knotter";
}

bool CKnotter::Initialize(CXMLWrapperTraining * pTraining)
{
	__super::Initialize(pTraining);

	//for new train
	
	ObjPhysCatogry threadCat = (m_enmSide == TSD_LEFT ? OPC_THREADLOOPLEFTOOL : OPC_THREADLOOPRIGHTOOL);

	if(m_centertoolpartconvex.m_rigidbody)
	   m_centertoolpartconvex.m_rigidbody->SetCollisionMask(m_centertoolpartconvex.m_rigidbody->m_MaskBits & (~threadCat));
	
	if(m_lefttoolpartconvex.m_rigidbody)
	   m_lefttoolpartconvex.m_rigidbody->SetCollisionMask(m_lefttoolpartconvex.m_rigidbody->m_MaskBits & (~threadCat));
	
	if(m_righttoolpartconvex.m_rigidbody)
	   m_righttoolpartconvex.m_rigidbody->SetCollisionMask(m_righttoolpartconvex.m_rigidbody->m_MaskBits & (~threadCat));

	if(m_pOwnerTraining->m_IsNewTrainMode)
	{
		m_LoopPlaneNormal = Ogre::Vector3(1,0,0);
		m_LoopFixPtLocal  = Ogre::Vector3(0,0,0);

		Ogre::SceneManager * pSMG = MXOgreWrapper::Get()->GetDefaultSceneManger();

		MisNewTraining * newTrain = dynamic_cast<MisNewTraining*>(m_pOwnerTraining);
		if(newTrain)
		{
#if(0)
		   m_Looper = newTrain->CreateSimpleLooper(pSMG);
#else
		   m_Looper = newTrain->CreateRopeThread(pSMG);
#endif
		}
		//m_Looper = m_pOwnerTraining new MisMedicThreadRope(pSMG);

		
	}

	m_KernalNodeInitPos = GetKernelNode()->_getDerivedPosition();
	m_firstTimeSetPosFromInput = true;
	return true;
}
//======================================================================================================
void CKnotter::ChangeToLoopMode()
{
	if(m_ThreadState == CKnotter::TS_LOOP)
	   return;

	if(m_Looper)
	{
		m_ThreadState = CKnotter::TS_LOOP;
		
		Ogre::SceneNode * ToolNode = GetKernelNode();
		
		ObjPhysCatogry threadCat = (m_enmSide == TSD_LEFT ? OPC_THREADLOOPLEFTOOL : OPC_THREADLOOPRIGHTOOL);

		m_Looper->CreateLoopedThread(GFPhysVector3(0,0,1.118f) , GFPhysVector3(1,0,0) , GFPhysVector3(0,0,-1.118f) , ToolNode , threadCat);
	}
}
//==========================================================================================
void CKnotter::OnOrganBeRemoved(MisMedicOrganInterface * organif)
{
	CTool::OnOrganBeRemoved(organif);
	if(m_OrganBinded == organif && m_OrganBinded)//organ will delete attachments so reset pointer to zero
	{
	   m_OrganBindRope = 0;
	}
}
//======================================================================================================
bool CKnotter::CreateBindThread(bool reachMinLen)
{
	if(m_Looper == 0 || m_ThreadState == CKnotter::TS_BINDED || m_ThreadState == CKnotter::TS_CUT)
	   return false;//
	
	int LoopSegNum = m_Looper->GetNumThreadNodes()-1;
	
	if(LoopSegNum < 20)
	   return false;
	//first select binded organ
	const std::vector<TFCollidePair> & threadCollideData = m_Looper->GetCollidePairs();
	
	std::set<GFPhysSoftBody *> CollideBodies;
	
	for(size_t c = 0 ; c < threadCollideData.size() ; c++)
	{
		GFPhysSoftBody * sbInContact = threadCollideData[c].GetCollideBody();

		MisMedicOrganInterface * organInContact = m_pOwnerTraining->GetOrgan(sbInContact);

		if(organInContact && organInContact->CanBeLoop())
		   CollideBodies.insert(sbInContact);
	}

	if(CollideBodies.size() == 0)
	   return false;

	int ValidRegionLeft[2];

	int ValidRegionRight[2];

	ValidRegionLeft[0] = 2;//LoopSegNum / 8;
	ValidRegionLeft[1] = LoopSegNum / 2-1;//LoopSegNum / 8 + LoopSegNum / 4;
	
	ValidRegionRight[0] = (LoopSegNum / 2 + 1);//LoopSegNum-ValidRegionLeft[1];
	ValidRegionRight[1] = (LoopSegNum-2);//LoopSegNum-ValidRegionLeft[0];

	//std::vector<TFCollidePair> collideLeft;
	
	//std::vector<TFCollidePair> collideRight;

	std::set<GFPhysSoftBody *>::iterator itor = CollideBodies.begin();
	
	while(itor != CollideBodies.end())
	{
		GFPhysSoftBody * sb = (*itor);
		
		int LRegionCollideCount = 0;
		
		int RRegionCollideCount = 0;

		Ogre::Vector3 leftAvgNormal(0,0,0);
		
		Ogre::Vector3 rightAvgNormal(0,0,0);

		for(size_t c = 0 ; c < threadCollideData.size() ; c++)
		{
			if(threadCollideData[c].GetCollideBody() == sb)
			{
			   int segIndex = threadCollideData[c].GetCollideSegmentIndex();
			   
			   if(segIndex >= ValidRegionLeft[0] && segIndex <= ValidRegionLeft[1])
			   {
				   if(threadCollideData[c].m_CollideNormal.dotProduct(threadCollideData[c].m_FaceNormal) > 0)
				   {
				      leftAvgNormal += threadCollideData[c].m_FaceNormal;
					  LRegionCollideCount++;
				   }
			   }
			   else if(segIndex >= ValidRegionRight[0] && segIndex <= ValidRegionRight[1])
			   {
				   if(threadCollideData[c].m_CollideNormal.dotProduct(threadCollideData[c].m_FaceNormal) > 0)
				   {
					  rightAvgNormal += threadCollideData[c].m_FaceNormal;
					  RRegionCollideCount++;
				   }
			   }
			}
		}
		leftAvgNormal.normalise();
		rightAvgNormal.normalise();

        float angle = leftAvgNormal.dotProduct(rightAvgNormal);

		bool BindSucced = false;
		
		if(angle < -0.0001f && LRegionCollideCount > 0 && RRegionCollideCount > 0)
		{
			MisMedicOrgan_Ordinary * organToBind = dynamic_cast<MisMedicOrgan_Ordinary*>(m_pOwnerTraining->GetOrgan(sb));

			if(organToBind && organToBind->m_CanBindThread)
			{
				//create 
				Ogre::SceneManager * pSMG = MXOgreWrapper::Get()->GetDefaultSceneManger();

				//step 1 get thread loop plane and intersect with append ' surface
				Ogre::SceneNode * kernalNode = GetKernelNode();

				Ogre::Matrix4 transMat = kernalNode->_getFullTransform();

				Ogre::Vector3 temp = transMat.extractQuaternion() * m_LoopPlaneNormal;
				temp.normalise();
				GFPhysVector3 NormalInWorld  = GFPhysVector3(temp.x , temp.y , temp.z);

				temp = transMat * m_LoopFixPtLocal;
				GFPhysVector3 PlanePtInWorld = GFPhysVector3(temp.x , temp.y , temp.z);

				if(m_OrganBindRope == 0)
				   m_OrganBindRope = new MisMedicBindedRope(MXOgreWrapper::Get()->GetDefaultSceneManger());

				GFPhysAlignedVectorObj<GFPhysVector3> threadPoints;

				int NumNode = m_Looper->GetNumThreadNodes();

				for(int n = 0 ; n < NumNode ; n++)
				{
					threadPoints.push_back(m_Looper->GetThreadNode(n).m_CurrPosition);
				}

				GFPhysVector3 loopfixPos = m_Looper->GetThreadNode(0).m_CurrPosition;

				bool succed = m_OrganBindRope->TryBindThread(*organToBind , 
															 threadPoints , 
															 m_Looper->GetThreadNode(0).m_CurrPosition ,
															 reachMinLen ? 0.5f : m_Looper->GetCollideRadius()+m_Looper->GetUnitLen()

															 );

				if(succed)
				{
					//create stick thread
					MisMedicBindedRope::ThreadBindPoint minTanPt;

					bool attachFinded = m_OrganBindRope->GetClosetBindedPoint(PlanePtInWorld , minTanPt);

					if(attachFinded)
					{
						ObjPhysCatogry threadCat = (m_enmSide == TSD_LEFT ? OPC_THREADLOOPLEFTOOL : OPC_THREADLOOPRIGHTOOL);

						GFPhysVector3 attachPos = minTanPt.m_AttachFace->m_Nodes[0]->m_CurrPosition*minTanPt.m_Weights[0]
												+ minTanPt.m_AttachFace->m_Nodes[1]->m_CurrPosition*minTanPt.m_Weights[1]
												+ minTanPt.m_AttachFace->m_Nodes[2]->m_CurrPosition*minTanPt.m_Weights[2];

						float totalLen = (attachPos-loopfixPos).Length();

						int SegmentNum = 15;

						m_Looper->SetUnitLen(totalLen / SegmentNum * 1.5f);

						m_Looper->CreateOnePointFixThread(SegmentNum , GFPhysVector3(0,0,0) , GFPhysVector3(1,0,0) , GFPhysVector3(0 , 0 , 1) , GetKernelNode() , threadCat);

						m_Looper->AttachNodePointToFace(minTanPt.m_AttachFace , minTanPt.m_Weights);

						m_OrganBindRope->CreateKnotNode(MXOgreWrapper::Get()->GetDefaultSceneManger() , minTanPt.m_AttachFace , minTanPt.m_Weights);

						m_ThreadState = CKnotter::TS_BINDED;

						BindSucced = true;
					}

					//now the object's memory is belong to organ
					//record organ binded
					m_OrganBinded = m_OrganBindRope->GetBindedOrgan();
					m_OrganBinded->AddOrganAttachment(m_OrganBindRope);
					//create event for training
					MxOrganBindedEvent * pEvent = (MxOrganBindedEvent*)MxEvent::CreateEvent(MxEvent::MXET_OrganBinded);
                    pEvent->SetOrgan(m_OrganBinded);
                    pEvent->SetRope(m_OrganBindRope);
				}
				else
				{
					delete m_OrganBindRope;//optimize to check "TryBindThread" with out new An binded rope object.so not delete memory evrey frame
					m_OrganBindRope = 0;
				}
			}
		}

		if(BindSucced == true)
		   return true;
		else
		   itor++;
	}

	return false;
}
MisMedicThreadRope * CKnotter::GetCurrentThread()
{
	return m_Looper;
}
void CKnotter::SetLoopControlRegion(KnotterLoopControlRegion * controlReg)
{
	m_LoopControlRegion = controlReg;
}
bool CKnotter::Update(float dt)
{	
	__super::Update(dt);

	if(m_Looper)
	{
	   if(m_ThreadState == CKnotter::TS_NONE && m_firstTimeSetPosFromInput)
	   {
		   Ogre::Vector3 derivedPos = GetKernelNode()->_getDerivedPosition();
		   
		   if((derivedPos-m_KernalNodeInitPos).length() > 0.1f)
		   {
			   ChangeToLoopMode();
			   m_firstTimeSetPosFromInput = false;
		   }
	   }
	   m_Looper->UpdateMesh();
	}

	bool  WantTightenLoop = false;
	
	float tightAngleLow = 0.5f;
	
	float tightAngleHig = 4.0f;

	float currShaftAside = GetShaftAside();
	  
	if(currShaftAside > tightAngleLow && currShaftAside < tightAngleHig)
	{
		if(m_ShaftStateForTighten == CKnotter::SFT_TIGHTEN)
		   m_ShaftStateForTighten = CKnotter::SFT_DICARD;
	}
	else if(currShaftAside <= tightAngleLow)
	{
		if(m_ShaftStateForTighten == CKnotter::SFT_PREPARED || m_ShaftStateForTighten == CKnotter::SFT_TIGHTEN)
		{
		   m_ShaftStateForTighten = CKnotter::SFT_TIGHTEN;
		   WantTightenLoop = true;
		}
	}
	else
	{
		m_ShaftStateForTighten = CKnotter::SFT_PREPARED;//prepare for tighten
	}
	
	if(WantTightenLoop && m_ThreadState == CKnotter::TS_LOOP)
	   WantTightenLoop = (m_LoopControlRegion == 0 ? true : m_LoopControlRegion->CanControlLoop(this));

	if(WantTightenLoop)
	{		  
		float LoopShrinkRate = 2.5f;//
		  
		float ThreadTightenRate = 2.5f;
		   
		float StartBindLen = 5.0f;

		float MinShrinkLen = 2.0f;

		if(m_Looper)
		{
			    bool reachMinLen = false;

			    float SrcLoopLen = m_Looper->GetTotalLen(false);
				
				float DstLoopLen = SrcLoopLen-LoopShrinkRate*dt;//deltaShat;
				
				if(DstLoopLen < MinShrinkLen)
				{
				   DstLoopLen = MinShrinkLen;
				   reachMinLen = true;
				}

				float percent = DstLoopLen / SrcLoopLen;

				GPClamp(percent, 0.0f, 1.0f);

				m_Looper->Shrink(percent);//SetUnitLen(m_Looper->GetUnitLen() * percent);//0.97f);

				if(DstLoopLen <= StartBindLen+FLT_EPSILON)
				{
					if(m_ThreadState == CKnotter::TS_LOOP)
					{
						bool succed = CreateBindThread(reachMinLen);
						if(succed && m_OrganBindRope)
						{
							float originLen   = m_OrganBindRope->GetBindTotalLength(true);

							float deformedLen = m_OrganBindRope->GetBindTotalLength(false);
							
							m_NodeStartBindLength = originLen;

							if(deformedLen < originLen && originLen > FLT_EPSILON)
							   m_OrganBindRope->TightenBindedOrgan(0.98f);//tight a little at once
						}
					}
					else if(m_ThreadState == CKnotter::TS_BINDED && m_OrganBindRope)
					{
						float currBindLen = m_OrganBindRope->GetBindTotalLength(true);

						float distBindLen = m_NodeStartBindLength * 0.1f;//1.8f;

						float DeltaRate = ThreadTightenRate*dt;//deltaShat;//

						if(currBindLen > distBindLen+FLT_EPSILON)
						{
							float DstLen = currBindLen-DeltaRate;

							if(DstLen < distBindLen)
								DstLen = distBindLen;

							float tightpecnet = DstLen / currBindLen;
							m_OrganBindRope->TightenBindedOrgan(0.98f);
						}
						
						if(m_Looper->IsCutAfterBound())
							m_OrganBindRope->SetUnConnected();
						else
							m_OrganBindRope->SetKnotNodeDir(m_Looper->GetThreadDir());
					}
				}
		}
	}

	return true;
} 
//============================================================================================================
void CKnotter::InternalSimulationStart(int currStep , int TotalStep , float dt)
{
	CTool::InternalSimulationStart( currStep ,  TotalStep ,  dt);
	//if(m_Looper)
	  // m_Looper->SimulateThreadPhysics(dt);
}
void CKnotter::InternalSimulationEnd(int currStep , int TotalStep , float dt)
{
	CTool::InternalSimulationEnd( currStep ,  TotalStep ,  dt);
	//if(m_Looper)
	   //m_Looper->EndSimuolateThreadPhysics(dt);

}
//void CKnotter::ChangeToKnotState()
//{
	//ObjPhysCatogry threadCat = (m_enmSide == TSD_LEFT ? OPC_THREADLOOPLEFTOOL : OPC_THREADLOOPRIGHTOOL);

	//if(m_Looper)
	 //  m_Looper->CreateOnePointFixThread(GFPhysVector3(0,0,0) , GFPhysVector3(1,0,0) , GFPhysVector3(0 , 0 , 1) , GetKernelNode() , threadCat);
//}
GFPhysVector3 CKnotter::CalculateToolCustomForceFeedBack()
{
	if(m_ThreadState == CKnotter::TS_LOOP)
	{
		if(m_Looper)
		{
			return m_Looper->CalcLoopForceFeedBack();// * 0.01f;
		}
	}
	else if(m_ThreadState == CKnotter::TS_LOOP)
	{

	}
	return GFPhysVector3(0,0,0);
}
//============================================================================================================
/*
void CKnotter::KnotThreadInOrgan(MisMedicOrgan_Ordinary & organ)
{
	if(m_OrganPart == 0)
	{
		//create 
		Ogre::SceneManager * pSMG = MXOgreWrapper::Get()->GetDefaultSceneManger();
		m_OrganPart = new MisMedicThreadRope(pSMG);

		//step 1 get thread loop plane and intersect with append ' surface
		Ogre::SceneNode * kernalNode = GetKernelNode();

		Ogre::Matrix4 transMat = kernalNode->_getFullTransform();

		Ogre::Vector3 temp = transMat.extractQuaternion() * m_LoopPlaneNormal;
		temp.normalise();
		GFPhysVector3 NormalInWorld  = GFPhysVector3(temp.x , temp.y , temp.z);

		temp = transMat * m_LoopFixPtLocal;
		GFPhysVector3 PlanePtInWorld = GFPhysVector3(temp.x , temp.y , temp.z);

		m_OrganPart->CreateTangleOrganThread(organ , NormalInWorld , PlanePtInWorld);

		//
		if(m_Looper)
		{
			MisMedicThreadRope::ThreadPassPointInFace minTanPt = m_OrganPart->GetClosetTanglePoint(PlanePtInWorld);
			m_Looper->AttachNodePointToFace(minTanPt.m_AttachFace , minTanPt.m_Weights);
		}
	}
	else
	{
		m_OrganPart->TightenTangledOrgan();
	}
}*/