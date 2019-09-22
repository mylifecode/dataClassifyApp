#include "Instruments/InjectNeedle.h"
#include "XMLWrapperTool.h"
#include "IObjDefine.h"
#include "ScreenEffect.h"
#include "EffectManager.h"
#include "BasicTraining.h"
#include "TrainingMgr.h"
#include "MXEvent.h"
#include "MXEventsDump.h"
#include "../NewTrain/PhysicsWrapper.h"
#include "../NewTrain/MisMedicOrganOrdinary.h"
#include "../NewTrain/CustomConstraint.h"
#include "../NewTrain/MisNewTraining.h"

#define NEEDLE_HEAD_TEST_RADIUS 0.25
#define NEEDLE_HEAD_SEARCH_RADIUS 0.26
#define NEEDLE_HEAD_FAR_AWAY_DIST 0.4
#define PUNCTURE_VALUE 1.3


CInjectNeedle::CInjectNeedle()
{
	PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(this);
}

CInjectNeedle::CInjectNeedle(CXMLWrapperTool * pToolConfig) : CTool(pToolConfig)
{
	PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(this);
}

CInjectNeedle::~CInjectNeedle()
{
	PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(this);
}
std::string CInjectNeedle::GetCollisionConfigEntryName()
{
	//碰撞体
	return "Inject Needle";
}
bool CInjectNeedle::Initialize(CXMLWrapperTraining * pTraining)
{
	__super::Initialize(pTraining);
	
	m_pBodyInserted = NULL;
	m_pFaceInserted = NULL;
	m_IndexOfFaceInjected = -1;
	m_IsInsert = false;
	m_WillInsert = false;

	m_PunctureValue = 0.f;

	m_leftShaftAsideScale = 0 ;
	m_rightShaftAsdieScale = 0;

	if(m_pOwnerTraining->m_IsNewTrainMode)
	{
		Ogre::SceneManager * pSceneManager = MXOgreWrapper::Instance()->GetDefaultSceneManger();

		m_NeedleHeadLocal.m_HeadPos = GFPhysVector3(0.05 , -0.05 , 2.15);
		m_NeedleHeadLocal.m_EndPos = GFPhysVector3(0.05 , -0.05 , -2.0);
	
		m_NeedleHeadLocal.m_Axis0 = GFPhysVector3(1 , 0 , 0);
		m_NeedleHeadLocal.m_Axis1 = GFPhysVector3(0 , 1, 0);
		m_NeedleHeadLocal.m_InsertAxis = GFPhysVector3(0 , 0 , 1);

		m_NeedleHeadLocal.m_Temp1 = m_NeedleHeadLocal.m_HeadPos + GFPhysVector3(0 , 0 , -NEEDLE_HEAD_TEST_RADIUS);
		m_NeedleHeadLocal.m_Temp2 = m_NeedleHeadLocal.m_HeadPos + GFPhysVector3(0 , 0 , -PUNCTURE_VALUE);

		m_NeedleHeadLocal.m_TempX = m_NeedleHeadLocal.m_HeadPos + GFPhysVector3(1 , 0 , 0);
		m_NeedleHeadLocal.m_TempY = m_NeedleHeadLocal.m_HeadPos + GFPhysVector3(0 , 1 , 0);
		m_NeedleHeadLocal.m_TempZ = m_NeedleHeadLocal.m_HeadPos + GFPhysVector3(0 , 0 , 1);



		m_painting.PushBackPoint(CustomPoint(&m_NeedleHeadWorld.m_HeadPos , Ogre::ColourValue::Green , 0.1));
		m_painting.PushBackPoint(CustomPoint(&m_NeedleHeadWorld.m_EndPos , Ogre::ColourValue::Blue , 0.1));

		m_painting.PushBackCoordAxis(CustomCoordAxis(&m_NeedleHeadWorld.m_HeadPos , 
																								&m_NeedleHeadWorld.m_Axis0 , 
																								&m_NeedleHeadWorld.m_Axis1 , 
																								&m_NeedleHeadWorld.m_InsertAxis));

		m_painting.PushBackPoint(CustomPoint(&m_NeedleHeadWorld.m_Temp1 , Ogre::ColourValue::Red , 0.05));
		m_painting.PushBackPoint(CustomPoint(&m_NeedleHeadWorld.m_Temp2 , Ogre::ColourValue::White , 0.05));

		m_Force = GFPhysVector3(0,0,0);

		return true;
	}

	return true;
}
//========================================================================================================
void CInjectNeedle::onFrameUpdateStarted(float timeelapsed)
{
	CTool::onFrameUpdateStarted(timeelapsed);
	
	if(m_WillInsert || m_IsInsert)
		CheckAfterInserting(timeelapsed);
	else
		CheckWhenNotInserted(timeelapsed);

	if(m_IsInsert && m_bElectricButton)
	{
		if(m_pFaceInserted)
		{
			MisMedicOrganInterface * organif = (MisMedicOrganInterface *)m_pBodyInserted->GetUserPointer();
			std::vector<Ogre::Vector2> resultUvs;
			std::vector<Ogre::Vector2> unused;
			organif->InjectSomething(this , m_pFaceInserted , m_PosInsertedWeights , timeelapsed , resultUvs);
			MisNewTraining *pNewTraining = dynamic_cast<MisNewTraining *>(m_pOwnerTraining);
			if (NULL != pNewTraining)
			{
				pNewTraining->receiveCheckPointList(MisNewTraining::OCPT_Inject , resultUvs , unused , this , NULL);
			}
		}
	}

	//m_painting.Update(timeelapsed);

}
//========================================================================================================
void CInjectNeedle::onFrameUpdateEnded()
{
	CTool::onFrameUpdateEnded();
}
//========================================================================================================
bool CInjectNeedle::Update(float dt)
{
	__super::Update(dt);
	

	return true;
}
//============================================================================================================
void CInjectNeedle::InternalSimulationStart(int currStep , int TotalStep , float dt)
{
	CTool::InternalSimulationStart( currStep ,  TotalStep ,  dt);

	GFPhysRigidBody * needleHead = GetRigidBodyOfNeedleHead();

	GFPhysTransform worldTrans = needleHead->GetWorldTransform();

	//GFPhysTransform worldTransForAxis = worldTrans;

	if(m_IsInsert)
	{
		worldTrans.SetRotation(m_QuatAtInsertion);
	}

	m_NeedleHeadWorld.m_HeadPos	= worldTrans*m_NeedleHeadLocal.m_HeadPos;
	m_NeedleHeadWorld.m_EndPos	= worldTrans*m_NeedleHeadLocal.m_EndPos;
	
	m_NeedleHeadWorld.m_InsertAxis = worldTrans.GetBasis() * m_NeedleHeadLocal.m_InsertAxis;
	m_NeedleHeadWorld.m_Axis0 = worldTrans.GetBasis() * m_NeedleHeadLocal.m_Axis0;
	m_NeedleHeadWorld.m_Axis1 = worldTrans.GetBasis() * m_NeedleHeadLocal.m_Axis1;

	if(m_IsInsert)
	{
		float increment  =  (m_NeedleHeadWorld.m_HeadPos - m_LastHeadPos).Dot(m_LastInsertDir);
		m_InsertInfo.m_DistOfHeadToUncorrect += increment;
// 		m_InsertInfo.m_DistOfHeadToUncorrect = ((m_pFaceInserted->m_Nodes[0]->m_UnDeformedPos * m_PosInsertedWeights[0]  //intersecteddir.Length();
// 																					+ m_pFaceInserted->m_Nodes[1]->m_UnDeformedPos * m_PosInsertedWeights[1]
// 																					+ m_pFaceInserted->m_Nodes[2]->m_UnDeformedPos * m_PosInsertedWeights[2]) - m_NeedleHeadWorld.m_HeadPos).Length();
	}

	m_NeedleHeadWorld.m_Temp1	= worldTrans*m_NeedleHeadLocal.m_Temp1;
	m_NeedleHeadWorld.m_Temp2	= worldTrans*m_NeedleHeadLocal.m_Temp2;
}
//============================================================================================================
void CInjectNeedle::InternalSimulationEnd(int currStep , int TotalStep , float dt)
{
	CTool::InternalSimulationEnd( currStep ,  TotalStep ,  dt);

	GFPhysTransform worldTrans = GetRigidBodyOfNeedleHead()->GetWorldTransform();

	if(m_IsInsert)
	{
		worldTrans.SetRotation(m_QuatAtInsertion);
	}
	
	m_LastHeadPos = worldTrans * m_NeedleHeadLocal.m_HeadPos;
	m_LastInsertDir = worldTrans.GetBasis() * m_NeedleHeadLocal.m_InsertAxis;

}
//============================================================================================================
GFPhysVector3 CInjectNeedle::CalculateToolCustomForceFeedBack()
{
	if(m_IsInsert)
	{
// 		float faceArea = (m_pFaceInserted->m_Nodes[1]->m_UnDeformedPos-m_pFaceInserted->m_Nodes[0]->m_UnDeformedPos).Cross(m_pFaceInserted->m_Nodes[2]->m_UnDeformedPos-m_pFaceInserted->m_Nodes[0]->m_UnDeformedPos).Length()*0.5f;
// 		const float times = 20.0f / 3.0f;
		MisMedicOrganInterface * oif = (MisMedicOrganInterface *)m_pBodyInserted->GetUserPointer();
		MisMedicOrgan_Ordinary *organ = dynamic_cast<MisMedicOrgan_Ordinary*>(oif);
		float rate = 1.0f;
		if(organ)
			rate = organ->m_DragForceRate;
		m_Force *= rate * 4;// * times * faceArea;
		return m_Force;
	}
	else
		return GFPhysVector3(0,0,0);
}
//============================================================================================================
void CInjectNeedle::PrepareSolveConstraint(Real Stiffness,Real TimeStep)
{	
	if(m_IsInsert)
	{
		GetRigidBodyOfNeedleHead()->Activate();
	}
}
//============================================================================================================
void CInjectNeedle::SolveConstraint(Real Stiffniss,Real TimeStep)
{
	m_Force = GFPhysVector3(0,0,0);

	if(m_IsInsert)
	{
		GFPhysSoftBodyFace *pFace = m_InsertInfo.m_pFace;
		GFPhysTransform trans = GetRigidBodyOfNeedleHead()->GetWorldTransform();
		trans.SetRotation(m_QuatAtInsertion);
		for(int n = 0 ; n < 3 ; n++)
		{
			//垂直针头平面上
			GFPhysVector3 localDest = m_NeedleHeadLocal.m_HeadPos + m_InsertInfo.m_LocalAnchor[n];

			GFPhysVector3 destPos = trans * localDest;

			GFPhysVector3 correction = (destPos - pFace->m_Nodes[n]->m_CurrPosition);
			
			GFPhysVector3 insertDir = m_NeedleHeadWorld.m_InsertAxis;

			float insertComponent = correction.Dot(insertDir);

			GFPhysVector3 insertDirCorrect = insertComponent*insertDir;

			GFPhysVector3 TangentCorrect = correction - insertDirCorrect;

			//pFace->m_Nodes[n]->m_CurrPosition += TangentCorrect * 0.9;

			//插入方向
			destPos = m_NeedleHeadWorld.m_HeadPos - m_InsertInfo.m_DistOfHeadToUncorrect  * 0.66 * insertDir;

			correction = destPos - pFace->m_Nodes[n]->m_CurrPosition;
			
			insertComponent = correction.Dot(insertDir);
			
			insertDirCorrect =  insertComponent*insertDir ;
			
			GFPhysVector3 corr = TangentCorrect * 0.9 + insertDirCorrect * 0.9;

			pFace->m_Nodes[n]->m_CurrPosition += corr;

			m_Force -= corr;

		}

// 		GFPhysVector3 currPosOfIntersected =  pFace->m_Nodes[0]->m_CurrPosition * m_InsertInfo.m_PosInsertedWeights[0] +
// 																			pFace->m_Nodes[1]->m_CurrPosition  * m_InsertInfo.m_PosInsertedWeights[1] + 
// 																			pFace->m_Nodes[2]->m_CurrPosition  * m_InsertInfo.m_PosInsertedWeights[2];
// 		
// 		GFPhysVector3 currDir = currPosOfIntersected - m_NeedleHeadWorld.m_HeadPos;
// 
// 		float partOfAxis0 = currDir.Dot(m_NeedleHeadWorld.m_Axis0);
// 		float partOfAxis1 = currDir.Dot(m_NeedleHeadWorld.m_Axis1);
// 		float partOfInsertAxis = currDir.Dot(m_NeedleHeadWorld.m_InsertAxis);
// 
// 		float delta0 = m_InsertInfo.m_PartOfAxis0 - partOfAxis0;
// 		float delta1 = m_InsertInfo.m_PartOfAxis1 - partOfAxis1;
// 		float deltaOfInsertDir = m_InsertInfo.m_PartOfInsertAxis - partOfInsertAxis;
// 		
// 		GFPhysVector3 tangImpluse = m_NeedleHeadWorld.m_Axis0 * delta0 +m_NeedleHeadWorld.m_Axis1 * delta1;
// 
// 		float sumOfWW = m_InsertInfo.m_PosInsertedWeights[0] * m_InsertInfo.m_PosInsertedWeights[0]
// 						+ m_InsertInfo.m_PosInsertedWeights[1] * m_InsertInfo.m_PosInsertedWeights[1] 
// 						+ m_InsertInfo.m_PosInsertedWeights[2] * m_InsertInfo.m_PosInsertedWeights[2];
// 
// 		float w[3];
// 		w[0] = m_InsertInfo.m_PosInsertedWeights[0] / sumOfWW;
// 		w[1] = m_InsertInfo.m_PosInsertedWeights[1] / sumOfWW;
// 		w[2] = m_InsertInfo.m_PosInsertedWeights[2] / sumOfWW;
// 
// 		for(int n  = 0 ; n < 3 ; n++)
// 		{
// 			GFPhysVector3 TanCorrect = (tangImpluse * w[n] * 0.4f);
// 			pFace->m_Nodes[n]->m_CurrPosition += TanCorrect;
// 		}
	}
}
//============================================================================================================
GFPhysRigidBody *CInjectNeedle::GetRigidBodyOfNeedleHead()
{
	return m_lefttoolpartconvex.m_rigidbody;
}
//============================================================================================================
bool CInjectNeedle::GetInjectedPosWeights(float weights[])
{
	if(m_IndexOfFaceInjected) 
	{
		std::copy(m_PosInsertedWeights , m_PosInsertedWeights + 3 , weights);
		return true;
	} 
	else
		return false;
}
//============================================================================================================
void CInjectNeedle::ClearInfoOfInsertion()
{
	m_WillInsert = false;
	m_IsInsert = false;
	m_IndexOfFaceInjected = -1;
	m_pFaceInserted = NULL;
	m_pFacesContact.clear();
}
//============================================================================================================
void CInjectNeedle::CheckAfterInserting(float dt)
{
	if(m_WillInsert)
	{
		for(size_t f = 0 ; f < m_pFacesContact.size() ; f++)
		{
			GFPhysSoftBodyFace * pFace = m_pFacesContact[f];
			Real  rayWeight;
			GFPhysVector3 intersectedPoint;
			Real weights[3];
			bool isIntersect = LineIntersectTriangle(pFace->m_Nodes[0]->m_CurrPosition , pFace->m_Nodes[1]->m_CurrPosition , pFace->m_Nodes[2]->m_CurrPosition , 
				m_NeedleHeadWorld.m_EndPos , m_NeedleHeadWorld.m_HeadPos , rayWeight , intersectedPoint , weights);
			if(isIntersect)
			{
				m_pFaceInserted = pFace;
				m_InsertInfo.m_pFace = pFace;
				std::copy(weights , weights + 3 , m_PosInsertedWeights);
				std::copy(weights , weights + 3 , m_InsertInfo.m_PosInsertedWeights);
				
				for(int n = 0 ; n < 3 ; n++)
				{
					Real mass = pFace->m_Nodes[n]->m_Mass;
					pFace->m_Nodes[n]->SetMass(mass * 4);
				}
				
				//save info
				m_QuatAtInsertion = GetRigidBodyOfNeedleHead()->GetWorldTransform().GetRotation();

				GFPhysVector3 intersecteddir =  intersectedPoint - m_NeedleHeadWorld.m_HeadPos;

				m_InsertInfo.m_DistOfHeadToUncorrect =  intersecteddir.Length();
// 																						((pFace->m_Nodes[0]->m_UnDeformedPos * weights[0]  //intersecteddir.Length();
// 																						+ pFace->m_Nodes[1]->m_UnDeformedPos * weights[1]
// 																						+ pFace->m_Nodes[2]->m_UnDeformedPos * weights[2]) - m_NeedleHeadWorld.m_HeadPos).Length();


				m_InsertInfo.m_PartOfAxis0 = intersecteddir.Dot(m_NeedleHeadWorld.m_Axis0);
				m_InsertInfo.m_PartOfAxis1 = intersecteddir.Dot(m_NeedleHeadWorld.m_Axis1);
				m_InsertInfo.m_PartOfInsertAxis = intersecteddir.Dot(m_NeedleHeadWorld.m_InsertAxis);

// 				GFPhysRigidBody node0worldDir = pFace->m_Nodes[0]->m_CurrPosition - m_NeedleHeadWorld.m_HeadPos;
// 				GFPhysRigidBody node1worldDir = pFace->m_Nodes[1]->m_CurrPosition - m_NeedleHeadWorld.m_HeadPos;
// 				GFPhysRigidBody node2worldDir = pFace->m_Nodes[2]->m_CurrPosition - m_NeedleHeadWorld.m_HeadPos;
// 
// 				m_InsertInfo.m_LocalDir[0] =  inverseTransform.GetBasis() * node0worldDir;
// 				m_InsertInfo.m_LocalDir[1] =  inverseTransform.GetBasis() * node0worldDir;
// 				m_InsertInfo.m_LocalDir[2] =  inverseTransform.GetBasis() * node0worldDir;

				GFPhysTransform inverseTransform = GetRigidBodyOfNeedleHead()->GetWorldTransform().Inverse();
				GFPhysVector3 nodePos[3];
				for(int i = 0 ;  i < 3 ; i++){
					nodePos[i] = inverseTransform * pFace->m_Nodes[i]->m_CurrPosition;
					m_InsertInfo.m_LocalAnchor[i] = nodePos[i] - m_NeedleHeadLocal.m_HeadPos;
				}
				
				m_IsInsert = true;
				m_WillInsert = false;
				break;
			}
		}
		//debug
		if(m_IsInsert)
		{
			if(m_pFacesContact.size() > 100)
				int hehe = 0;
			for(size_t f = 0 ; f < m_pFacesContact.size() ; f++)
			{
				GFPhysSoftBodyFace * pFace = m_pFacesContact[f];
				m_painting.ChangeFaceColor(pFace , Ogre::ColourValue::Green);
			}
			m_painting.ChangeFaceColor(m_pFaceInserted, Ogre::ColourValue(1,1,0,1));
		}
		if(!m_IsInsert)
		{
			for(size_t f = 0 ; f < m_pFacesContact.size() ; f++)
			{
				GFPhysSoftBodyFace * pFace = m_pFacesContact[f];
				GFPhysVector3 closestPos = ClosestPtPointTriangle(pFace->m_Nodes[0]->m_CurrPosition , pFace->m_Nodes[1]->m_CurrPosition , pFace->m_Nodes[2]->m_CurrPosition ,m_NeedleHeadWorld.m_HeadPos);
				if(closestPos.Distance(m_NeedleHeadWorld.m_HeadPos) > NEEDLE_HEAD_FAR_AWAY_DIST)
				{
					m_WillInsert = false;
					break;
				}
			}
			if(!m_WillInsert)
			{
				for(size_t f = 0 ; f < m_pFacesContact.size() ; f++)
				{
					GFPhysSoftBodyFace * pFace = m_pFacesContact[f];
					pFace->EnableCollideWithRigid();
					m_painting.EraseFace(pFace);
				}
				m_pFacesContact.clear();
			}
		}
	}
	else if(m_IsInsert)
	{
		m_IsInsert = false;
		for(size_t f = 0 ; f < m_pFacesContact.size() ; f++)
		{
			GFPhysSoftBodyFace * pFace = m_pFacesContact[f];
			Real  rayWeight;
			GFPhysVector3 intersectedPoint;
			Real weights[3];
			bool isIntersect = LineIntersectTriangle(pFace->m_Nodes[0]->m_CurrPosition , pFace->m_Nodes[1]->m_CurrPosition , pFace->m_Nodes[2]->m_CurrPosition , m_NeedleHeadWorld.m_EndPos , m_NeedleHeadWorld.m_HeadPos , rayWeight , intersectedPoint , weights);
			if(isIntersect && rayWeight < 0.99)
			{
				m_IsInsert = true;
				break;
			}
		}
		if(!m_IsInsert)
		{
			for(size_t f = 0 ; f < m_pFacesContact.size() ; f++)
			{
				GFPhysSoftBodyFace * pFace = m_pFacesContact[f];
				pFace->EnableCollideWithRigid();
				m_painting.EraseFace(pFace);
			}
			m_pFacesContact.clear();
		}
	}
}

//============================================================================================================
void CInjectNeedle::CheckWhenNotInserted(float dt)
{
	if(m_WillInsert || m_IsInsert)
		return;

	float minDist = FLT_MAX;
	float maxCos = 0.0f;
	int   minFaceIndex = -1;

	GFPhysSoftBody * minSoftBody = 0;

	GFPhysRigidBody * pRigidBodyOfNeedleHead = GetRigidBodyOfNeedleHead();
	
	if(m_pFacesContact.size() > 100)
		int hehe = 0;

	for(size_t c = 0 ; c < m_ToolColliedFaces.size() ; c++)
	{
		const ToolCollidedFace & collideFace = m_ToolColliedFaces[c];
		GFPhysRigidBody * rigid = GFPhysRigidBody::Upcast(collideFace.m_collideRigid);
		GFPhysSoftBody  * soft  = GFPhysSoftBody::Upcast(collideFace.m_collideSoft);

		if(rigid == pRigidBodyOfNeedleHead)
		{
			float weights[3];

			GFPhysSoftBodyFace * facecollide = collideFace.m_collideFace;

			GFPhysVector3 CdnormalOnFace = collideFace.m_CollideNormal;

			weights[0] = collideFace.m_collideWeights[0];
			weights[1] = collideFace.m_collideWeights[1];
			weights[2] = collideFace.m_collideWeights[2];

			GFPhysVector3 closetPointInTri = ClosestPtPointTriangle(m_NeedleHeadWorld.m_HeadPos, 
				facecollide->m_Nodes[0]->m_CurrPosition,
				facecollide->m_Nodes[1]->m_CurrPosition, 
				facecollide->m_Nodes[2]->m_CurrPosition);

			float dist = (closetPointInTri-m_NeedleHeadWorld.m_HeadPos).Length();
		
			if( dist < NEEDLE_HEAD_TEST_RADIUS)
			{
				if(facecollide->m_GenFace && facecollide->m_GenFace->m_ShareTetrahedrons.size() > 0)
				{
					GFPhysSoftBodyTetrahedron *pTetra = facecollide->m_GenFace->m_ShareTetrahedrons[0].m_Hosttetra;
					if(pTetra)
					{
						GFPhysVector3 faceNorm = (facecollide->m_Nodes[1]->m_UnDeformedPos-facecollide->m_Nodes[0]->m_UnDeformedPos).Cross(facecollide->m_Nodes[2]->m_UnDeformedPos-facecollide->m_Nodes[0]->m_UnDeformedPos);
						faceNorm.Normalize();

						GFPhysVector3 deformderv = GFPhysSoftBody::CalTetraDeformationDerivative(*pTetra , faceNorm);
						float deformValue = deformderv.Dot(faceNorm);

						if(deformValue < 0 && dist < minDist)
						{
							minDist = dist;
							minFaceIndex = c;
							minSoftBody = soft;
						}
					}
				}
			}
		}
	}
	if(minFaceIndex > 0)
	{
		m_pFacesContact.clear();
		MisMedicOrganInterface *pOrganInterface = m_pOwnerTraining->GetOrgan(minSoftBody);
		MisMedicOrgan_Ordinary *pOrgan  = dynamic_cast<MisMedicOrgan_Ordinary *>(pOrganInterface);
		if (NULL != pOrgan)
		{
			pOrgan->SelectPhysFaceAroundPoint(m_pFacesContact , m_NeedleHeadWorld.m_HeadPos , NEEDLE_HEAD_SEARCH_RADIUS , true);
		}

		if(m_pFacesContact.size() > 0)
		{
			for(size_t f = 0 ; f < m_pFacesContact.size() ; f++)
			{
				GFPhysSoftBodyFace * pFace = m_pFacesContact[f];
				pFace->DisableCollideWithRigid();
				m_painting.PushBackFace(CustomFace(pFace , Ogre::ColourValue(0,1,1,1)));
			}
			m_pBodyInserted = minSoftBody;
			m_WillInsert = true;
		}
	}
	else
		m_pFacesContact.clear();

}
//============================================================================================================
