#pragma once
#include "stdafx.h"
#include "Inception.h"
#include "SYStopBleedTrain.h"
#include "XMLWrapperTraining.h"
#include "ScreenEffect.h"
#include "MXOgreWrapper.h"
#include "EffectManager.h"
#include "MXEventsDump.h"
#include "InputSystem.h"
#include "OgreMaxScene.hpp"
#include "LightMgr.h"
#include "CollisionTools.h"
#include <QMessageBox>
#include <fstream>
#include "MXEvent.h"
#include "MXToolEvent.h"
#include "Instruments/Tool.h"
#include "Instruments/ElectricHook.h"
#include "TextureBloodEffect.h"
#include "math/GoPhysTransformUtil.h"
#include "SYScoreTableManager.h"
// gophysic
#include "PhysicsWrapper.h"
#include "MisMedicOrganOrdinary.h"
#include <time.h>
#include <stdlib.h>

#define NEEDBURNSECONDS 2.0f

REGISTERTRAINING(SYStopBleedTrain, NewBasicTraining, StopBleedTrain)


    SYStopBleedTrain::BleedPoint::BleedPoint(MisMedicOrgan_Ordinary * organ, GFPhysSoftBodyFace * face, float weights[], float radiuscale, float gravAcc)
	{
		m_pOrgan = organ;
		m_bFace = face;
		m_Weights[0] = weights[0];
		m_Weights[1] = weights[1];
		m_Weights[2] = weights[2];
		m_radiusScale = radiuscale;
		m_gravAcc = gravAcc;
		m_BloodTrack = 0;

		m_StopSucced = false;
		m_StopTime = 0;
		m_ContinueBurnTime = 0;
	}

    void SYStopBleedTrain::BleedPoint::OnBleedStopped(float timeAtStop)
    {
	    m_BloodTrack = 0;
	    m_StopSucced = true;
	    m_StopTime = timeAtStop;
    }

    void SYStopBleedTrain::BleedPoint::BeginBleed()
	{
		if (m_BloodTrack)
			return;

		int forigin = m_pOrgan->GetOriginFaceIndexFromUsrData(m_bFace);

		if (forigin >= 0 && forigin < (int)m_pOrgan->m_OriginFaces.size())
		{
			m_BloodTrack = m_pOrgan->GetBloodTextureEffect()->createBloodTrack(m_pOrgan, m_Weights, 1.0f, forigin, m_pOrgan->GetCreateInfo().m_BloodRadius*m_radiusScale, m_gravAcc, 7.0f);
			m_BloodTrack->SetCanScaleBloodDropRadius(false);
		}
	}

SYStopBleedTrain::BleedBatch::BleedBatch()
{
		m_isActive = false;
		m_BatchStartTime = m_BatchEndTime = 0;
		m_BurnPointDeviated = false;
}
	
void SYStopBleedTrain::BleedBatch::AddPointToBleed(MisMedicOrgan_Ordinary * organ, GFPhysSoftBodyFace * face, float weights[], float radiuscale, float gravAcc)
{
		m_bPoints.push_back(BleedPoint(organ , face, weights, radiuscale, gravAcc));
}
	
int  SYStopBleedTrain::BleedBatch::FindBleedPointByTrack(OrganSurfaceBloodTextureTrack * bloodtrack)
{
	for (int c = 0; c < m_bPoints.size(); c++)
	{
		if (m_bPoints[c].m_BloodTrack == bloodtrack)
			return c;
	}
	return -1;
}
void SYStopBleedTrain::BleedBatch::Active(float Time)
{
		if (m_isActive)
			return;

		for (int c = 0; c < (int)m_bPoints.size(); c++)
		{
			m_bPoints[c].BeginBleed();
		}
		m_BatchStartTime = Time;
		m_isActive = true;
}

void SYStopBleedTrain::BleedBatch::Deactive(float Time)
{
		m_BatchEndTime = Time;
}

SYStopBleedTrain::SYStopBleedTrain()
{
	m_pOrgan = NULL;

	m_electricState = NoneElec;
	m_burnRadius = 0.0f;
	m_currStepState = NoneStep;

	m_bAllowAddBleedPoint = false;
	m_CurrActiveBatch = 0;
	
	srand(time(NULL));
}


SYStopBleedTrain::~SYStopBleedTrain(void)
{
	
}

void SYStopBleedTrain::BuildBleedBatches()
{
	float weights[3] = { 0.3333f, 0.33333f, 0.33333f };

	BleedBatch batch1;
	batch1.AddPointToBleed(m_pOrgan , m_bleedFaces[0], weights, 1.5f, 5.0f);
	GFPhysVector3 basePos = (m_bleedFaces[0]->m_Nodes[0]->m_UnDeformedPos + m_bleedFaces[0]->m_Nodes[1]->m_UnDeformedPos + m_bleedFaces[0]->m_Nodes[2]->m_UnDeformedPos)*0.33333f;

	float maxDist = 0;
	int selectedIndex = -1;
	for (int c = 2; c < (int)m_bleedFaces.size(); c++)
	{
		GFPhysVector3 pos = (m_bleedFaces[c]->m_Nodes[0]->m_UnDeformedPos + m_bleedFaces[c]->m_Nodes[1]->m_UnDeformedPos + m_bleedFaces[c]->m_Nodes[2]->m_UnDeformedPos)*0.33333f;

		float dist = (pos - basePos).Length();
		if (dist > maxDist)
		{
			maxDist = dist;
			selectedIndex = c;
		}
	}
	batch1.AddPointToBleed(m_pOrgan, m_bleedFaces[selectedIndex], weights, 0.9f, 2.0f);
	m_bleedFaces[selectedIndex] = 0;

	BleedBatch batch2;
	batch2.AddPointToBleed(m_pOrgan, m_bleedFaces[1], weights, 1.5f, 5.0f);
	basePos = (m_bleedFaces[1]->m_Nodes[0]->m_UnDeformedPos + m_bleedFaces[1]->m_Nodes[1]->m_UnDeformedPos + m_bleedFaces[1]->m_Nodes[2]->m_UnDeformedPos)*0.33333f;
	maxDist = 0;
	selectedIndex = -1;
	for (int c = 2; c < (int)m_bleedFaces.size(); c++)
	{
		if (m_bleedFaces[c] == 0)
			continue;

		GFPhysVector3 pos = (m_bleedFaces[c]->m_Nodes[0]->m_UnDeformedPos + m_bleedFaces[c]->m_Nodes[1]->m_UnDeformedPos + m_bleedFaces[c]->m_Nodes[2]->m_UnDeformedPos)*0.33333f;

		float dist = (pos - basePos).Length();
		if (dist > maxDist)
		{
			maxDist = dist;
			selectedIndex = c;
		}
	}
	batch2.AddPointToBleed(m_pOrgan, m_bleedFaces[selectedIndex], weights, 0.9f, 2.0f);
	GFPhysVector3 basePos2 = (m_bleedFaces[selectedIndex]->m_Nodes[0]->m_UnDeformedPos + m_bleedFaces[selectedIndex]->m_Nodes[1]->m_UnDeformedPos + m_bleedFaces[selectedIndex]->m_Nodes[2]->m_UnDeformedPos)*0.33333f;
	m_bleedFaces[selectedIndex] = 0;
	
	//batch2 point3 
	maxDist = 0;
	selectedIndex = -1;
	for (int c = 2; c < (int)m_bleedFaces.size(); c++)
	{
		if (m_bleedFaces[c] == 0)
			continue;

		GFPhysVector3 pos = (m_bleedFaces[c]->m_Nodes[0]->m_UnDeformedPos + m_bleedFaces[c]->m_Nodes[1]->m_UnDeformedPos + m_bleedFaces[c]->m_Nodes[2]->m_UnDeformedPos)*0.33333f;

		float dist0 = (pos - basePos).Length();
		float dist1 = (pos - basePos2).Length();
		if (dist0*dist1 > maxDist)
		{
			maxDist = dist0*dist1;
			selectedIndex = c;
		}
	}
	batch2.AddPointToBleed(m_pOrgan, m_bleedFaces[selectedIndex], weights, 0.9f, 2.0f);
	m_bleedFaces[selectedIndex] = 0;

	m_BleedBatches.push_back(batch1);
	m_BleedBatches.push_back(batch2);
}

bool SYStopBleedTrain::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{
	bool result = MisNewTraining::Initialize(pTrainingConfig , pToolConfig);
	MisMedicOrgan_Ordinary * Dome_Active = dynamic_cast<MisMedicOrgan_Ordinary*>(m_DynObjMap[EODT_DOME_ACTIVE]);
	Ogre::Vector3 P0 = InputSystem::GetInstance(DEVICETYPE_LEFT)->GetPivotPosition();
	Ogre::Vector3 P1 = InputSystem::GetInstance(DEVICETYPE_RIGHT)->GetPivotPosition();
	std::vector<GFPhysSoftBodyFace*> aroundfaces;
	Dome_Active->DisRigidCollAroundPoint(aroundfaces, OgreToGPVec3(P0), 2.0f, false);
	Dome_Active->DisRigidCollAroundPoint(aroundfaces, OgreToGPVec3(P1), 2.0f, false);

	DynObjMap::iterator itr = m_DynObjMap.find(12);

	m_pOrgan = static_cast<MisMedicOrgan_Ordinary*>(itr->second);
	m_pOrgan->SetMaxBloodTrackCount(10);		// temp set
	m_pOrgan->SetTimeNeedToEletricCut(1800);	//防止电切造成出血
	m_pOrgan->SetMinPunctureDist(1.2f);

	m_burnRadius = m_pOrgan->GetCreateInfo().m_BurnRadius;

	//read face bleed point
	std::ifstream fin;
	fin.open("../Data/module/Basic/StopBleedTrain/facesIndex.txt");
	if (fin.is_open())
	{
		int faceIndex = -1;
		GFPhysSoftBodyFace * pFace = NULL;
		while (!fin.eof())
		{
			fin >> faceIndex;
			pFace = m_pOrgan->m_OriginFaces[faceIndex].m_physface;
			m_bleedFaces.push_back(pFace);
			if (m_bleedFaces.size() == 10)
				break;
		}
		fin.close();
		std::random_shuffle(m_bleedFaces.begin(), m_bleedFaces.end());		//随机初始流血点和渗血点
		
		//add bleed batches
		BuildBleedBatches();
	}
	return true;
}

void SYStopBleedTrain::OnHandleEvent(MxEvent* pEvent)
{
// 	if(m_currStepState == SYStopBleedTrain::Finish)
// 		return;

	MxToolEvent * pToolEvent = NULL;
	switch(pEvent->m_enmEventType)
	{
	case MxEvent::MXET_PunctureSurface:
		 //戳伤器官
		 CTipMgr::Instance()->ShowTip("OperateError2");
		 CScoreMgr::Instance()->Grade("OperateError2");
		 break;
	case MxEvent::MXET_ElecCutStart:
		 //电切器官
		 CTipMgr::Instance()->ShowTip("OperateError3");
		 CScoreMgr::Instance()->Grade("OperateError3");
		 break;
	case MxEvent::MXET_BleedStart:
		 {
			pToolEvent = static_cast<MxToolEvent*>(pEvent);
			TextureBloodTrackEffect * bloodEffect = static_cast<TextureBloodTrackEffect*>(pToolEvent->m_pUserPoint);
			OrganSurfaceBloodTextureTrack * pBloodTrack = bloodEffect->GetLatestTextureBloodTrack();
		    pBloodTrack->SetCanScaleBloodDropRadius(false);
			m_bloodTrackTimeMap.insert(make_pair(pBloodTrack , 0.f));
			break;
		 }
	case MxEvent::MXET_BleedEnd:
		 
		 pToolEvent = static_cast<MxToolEvent*>(pEvent);
		 
		 OrganSurfaceBloodTextureTrack * bloodStoped = (OrganSurfaceBloodTextureTrack *)pToolEvent->m_pUserPoint;
		 
		 BleedBatch & batch = m_BleedBatches[m_CurrActiveBatch];
		 
		 int stoppedNum = 0;
		 
		 bool hasStoppedBatchBleed = false;

		 for (int c = 0; c < (int)batch.m_bPoints.size(); c++)
		 {
			 if ( batch.m_bPoints[c].m_BloodTrack == bloodStoped )
			 {   
				 batch.m_bPoints[c].OnBleedStopped(GetElapsedTime());
				 hasStoppedBatchBleed = true;
			 }

			 if (batch.m_bPoints[c].m_BloodTrack == 0)
			 {
				 stoppedNum++;
			 }
		 }

		 if (hasStoppedBatchBleed)//止血点属于批次出血点（非意外出血点）
		 {
			 if (stoppedNum >= (int)batch.m_bPoints.size())
			 {
				 //record batch finish time
				 m_BleedBatches[m_CurrActiveBatch].Deactive(GetElapsedTime());
				
				 //next batch if have
				 if (m_CurrActiveBatch < (int)m_BleedBatches.size() - 1)
				 {
					 m_CurrActiveBatch++;
					 m_currStepState = SYStopBleedTrain::NeedAdd;
				 }
				 else//this is the final batch
				 {
					 m_currStepState = SYStopBleedTrain::Finish;
					 TrainingFinish();
				 }
			 }

			 CTipMgr::Instance()->ShowTip("EndStopBleed");//提示止血成功
		 }
		 else
		 {
			 CTipMgr::Instance()->ShowTip("StopBleedOk");//止住戳伤造成的血流
		 }

		 m_bloodTrackTimeMap.erase(bloodStoped);
		 break;
	}
}

void SYStopBleedTrain::OnTrainingIlluminated()
{
	MisNewTraining::OnTrainingIlluminated();
	m_currStepState = NeedAdd;
	m_LastTime = 0.0f;
}


OrganSurfaceBloodTextureTrack* SYStopBleedTrain::CreateBloodTrack(GFPhysSoftBodyFace * pFace, float weights[3], float radiusScale, float slipAcc)
{
	OrganSurfaceBloodTextureTrack * pBloodTrack = NULL;
	int forigin = m_pOrgan->GetOriginFaceIndexFromUsrData(pFace);

	if(forigin >= 0 && forigin < (int)m_pOrgan->m_OriginFaces.size())
	{
		float onwerweights[3];
		onwerweights[0] = weights[0];
		onwerweights[1] = weights[1];
		onwerweights[2] = weights[2];
		pBloodTrack = m_pOrgan->GetBloodTextureEffect()->createBloodTrack(m_pOrgan, onwerweights, 1.0f, forigin, m_pOrgan->GetCreateInfo().m_BloodRadius*radiusScale, slipAcc , 7.0f);
	}
	return pBloodTrack;
}

void SYStopBleedTrain::CheckTool(CTool * pTool, float dt, std::set<OrganSurfaceBloodTextureTrack*> & processedTrack)
{
	if (pTool)
	{
		CElectricHook * pElectricHook = dynamic_cast<CElectricHook*>(pTool);
		
		if (pElectricHook)
		{
			if (pElectricHook->GetElectricRightPad())	//电凝
			{
				float touchedWeights[3];
				
				Ogre::Vector2 touchPtTex;
				
				Ogre::Vector2 bleedPtTex;
				
				const GFPhysCollideObject * pCollideObject = pElectricHook->GetDistCollideObject();
				
				if (pCollideObject == m_pOrgan->m_physbody)	//电到器官
				{
					pElectricHook->GetDistPointWeight(touchedWeights);
					const GFPhysSoftBodyFace * pDistFace = pElectricHook->GetDistCollideFace();
					touchPtTex = m_pOrgan->GetTextureCoord(const_cast<GFPhysSoftBodyFace*>(pDistFace), touchedWeights);

					OrganSurfaceBloodTextureTrack * pBurnedTrack = NULL;
					
					//选区离灼烧点最近的出血点
					pBurnedTrack = m_pOrgan->GetBloodTextureEffect()->GetNearestTextureBloodTrackIndex(touchPtTex, bleedPtTex);
					
					if (pBurnedTrack)
					{
						float texSpaceDist = (touchPtTex - bleedPtTex).length();

						bool  isValidBurn = false;

						bool  hasDeviate  = true;

						if (texSpaceDist < m_burnRadius*0.5f)//第一级
						{
							isValidBurn = true;
							hasDeviate  = false;
						}
						else if (texSpaceDist < m_burnRadius)//第二级
						{
							isValidBurn = true;
							hasDeviate  = true;
						}
						
						if (isValidBurn)
						{
							std::map<OrganSurfaceBloodTextureTrack*, float>::iterator bloodItr = m_bloodTrackTimeMap.find(pBurnedTrack);

							bloodItr->second += dt;

							float burnedTime = bloodItr->second;

							if (m_electricState != ElectricBleedPoint)
							{
								m_electricState = ElectricBleedPoint;
								CTipMgr::Instance()->ShowTip("ElecContinue");
							}

							if (burnedTime > NEEDBURNSECONDS)
								pBurnedTrack->SetCanScaleBloodDropRadius(true);

							processedTrack.insert(pBurnedTrack);
						}
						else
						{
							CTipMgr::Instance()->ShowTip("OperateError1");
						}

						if (m_CurrActiveBatch >= 0 && m_CurrActiveBatch < m_BleedBatches.size())
						{
							if (hasDeviate)
								m_BleedBatches[m_CurrActiveBatch].m_BurnPointDeviated = true;

							if (isValidBurn)
							{
								for (int c = 0; c < (int)m_BleedBatches[m_CurrActiveBatch].m_bPoints.size(); c++)
								{
									if (m_BleedBatches[m_CurrActiveBatch].m_bPoints[c].m_BloodTrack == pBurnedTrack)
									{
										m_BleedBatches[m_CurrActiveBatch].m_bPoints[c].m_ContinueBurnTime += dt;
									}
								}
							}
						}
					}
					else
					{
						
					}
				}
				else
				{
					m_electricState = NoneElec;
				}
			}
			else
			{
				m_electricState = NoneElec;

				/*
				if (pElectricHook->GetElectricLeftPad())	//电切
				{
					const GFPhysCollideObject * pCollideObject = pElectricHook->GetDistCollideObject();
					if (pCollideObject && pCollideObject->GetUserPointer() == m_pOrgan)	//电到器官
					{
						const GFPhysSoftBodyFace * pDistFace = pElectricHook->GetDistCollideFace();
						std::map<GFPhysSoftBodyNode*, float>::iterator itr;
						for (int n = 0; n < 3; ++n)
						{
							GFPhysSoftBodyNode * pNode = pDistFace->m_Nodes[n];
							itr = m_nodeTimeMap.find(pNode);
							if (itr == m_nodeTimeMap.end())
							{
								m_nodeTimeMap.insert(make_pair(pNode, dt));
							}
							else
							{
								itr->second += dt;
								if (itr->second > 5.f)					//过度灼烧器官，严重错误操作，退出训练
								{
									m_bSingleEleTime5s = 1;
									//m_pOrgan->PerformElectricCut(pElectricHook,const_cast<GFPhysSoftBodyFace*>(pDistFace),touchedWeights);
									TearOrgan(pElectricHook, pDistFace);
									CTipMgr::Instance()->ShowTip("FatalError");
									//m_currStepState = Finish;
									m_nodeTimeMap.clear();
									TrainingFatalError();
									break;
								}
							}
						}
					}
				}
				*/
			}
		}
	}
}

void SYStopBleedTrain::TearOrgan(CElectricHook * pTool,const GFPhysSoftBodyFace* pFace)
{
	
}

bool SYStopBleedTrain::Update(float dt)
{
	bool result = MisNewTraining::Update(dt);
	
	float currTimeElapsed = GetElapsedTime();

	if (m_currStepState == NeedAdd)
	{
		if (m_bAllowAddBleedPoint)
		{
			m_BleedBatches[m_CurrActiveBatch].Active(currTimeElapsed);
			CTipMgr::Instance()->ShowTip("BeginStopBleed");		//提示开始止血
			m_currStepState = WaitRemove;						//进入检查阶段
			m_bAllowAddBleedPoint = false;						//重置标记
		}
		else
		{
			if (currTimeElapsed - m_LastTime > 1.5f)
				m_bAllowAddBleedPoint = true;
		}
	}
	else if (m_currStepState == WaitRemove)
	{
		CTool * pLeftTool = NULL;
		CTool * pRightTool = NULL;

		pLeftTool = (CTool*)m_pToolsMgr->GetLeftTool();
		pRightTool = (CTool*)m_pToolsMgr->GetRightTool();


		std::set<OrganSurfaceBloodTextureTrack*> processedTrack;
		//CheckTool
		CheckTool(pLeftTool, dt, processedTrack);
		CheckTool(pRightTool, dt, processedTrack);

		std::map<OrganSurfaceBloodTextureTrack*, float>::iterator itor = m_bloodTrackTimeMap.begin();
		while (itor != m_bloodTrackTimeMap.end())
		{
			if (processedTrack.find(itor->first) == processedTrack.end())
			{
				itor->second = 0;//持续电凝结束 reset time
			}
			itor++;
		}
		m_LastTime = currTimeElapsed;
	}

	return true;
}


SYScoreTable * SYStopBleedTrain::GetScoreTable()
{
	return SYScoreTableManager::GetInstance()->GetTable("01101001");
}

void SYStopBleedTrain::OnSaveTrainingReport()
{
	bool allfinish = true;
	bool hasBurnPointDeviate = false;

	int numBPointsBurnInRightTime = 0;

	for (int group = 0; group < m_BleedBatches.size(); group++)
	{
		SYStopBleedTrain::BleedBatch & bloodbatch = m_BleedBatches[group];

		//if (group == 0)
		//{
			int numSucced = bloodbatch.GetNumBleedPointFinished();
			
			bool orderRight = true;

			if (numSucced == 3)
			{
				QString fullcode = "0100" + QString::number(group + 1) + "01510";

				AddScoreItemDetail(fullcode, 0.0f);//

				if (bloodbatch.m_bPoints[0].m_StopTime > bloodbatch.m_bPoints[1].m_StopTime)
					orderRight = false;
				else
					orderRight = true;
			}

			else if (numSucced == 2)
			{
				QString fullcode = "0100" + QString::number(group + 1) + "01510";

				AddScoreItemDetail(fullcode , 0.0f);//
				
				if (bloodbatch.m_bPoints[0].m_StopTime > bloodbatch.m_bPoints[1].m_StopTime)
					orderRight = false;
				else
					orderRight = true;
			}
			
			else if (numSucced == 1)
			{
				QString fullcode = "0100" + QString::number(group + 1) + "01511";

				AddScoreItemDetail(fullcode, 0.0f);//
				if (bloodbatch.m_bPoints[0].m_StopSucced)
					orderRight = true;
				else
					orderRight = false;
			}
			else
			{
				QString fullcode = "0100" + QString::number(group + 1) + "01519";

				AddScoreItemDetail(fullcode, 0.0f);//
			}

			if (numSucced > 0)
			{
				if (orderRight)
				{
					QString fullcode = "0100" + QString::number(group + 1) + "02710";
					AddScoreItemDetail(fullcode, 0.0f);//
				}
				else
				{
					QString fullcode = "0100" + QString::number(group + 1) + "02711";
					AddScoreItemDetail(fullcode, 0.0f);//
				}

				if (bloodbatch.m_BurnPointDeviated)
				{
					QString fullcode = "0100" + QString::number(group + 1) + "02211";
					AddScoreItemDetail(fullcode, 0.0f);//
					hasBurnPointDeviate = true;
				}
				else
				{
					QString fullcode = "0100" + QString::number(group + 1) + "02210";
					AddScoreItemDetail(fullcode, 0.0f);//
				}

				bool isoverBurn = false;
				for (int b = 0; b < (int)bloodbatch.m_bPoints.size(); b++)
				{
					if (bloodbatch.m_bPoints[b].m_ContinueBurnTime > 3)
					{
						isoverBurn = true;
					}
					else if (bloodbatch.m_bPoints[b].m_ContinueBurnTime > 1.5f)
					{
						numBPointsBurnInRightTime++;
					}
				}
				if (!isoverBurn)
				{
					QString fullcode = "0100" + QString::number(group + 1) + "01610";
					AddScoreItemDetail(fullcode, 0.0f);
				}
				else
				{
					QString fullcode = "0100" + QString::number(group + 1) + "01611";
					AddScoreItemDetail(fullcode, 0.0f);
				}
			}

			if (bloodbatch.IsAllBleedPointBeStopped() == false)
				allfinish = false;
	}
	
	AddScoreItemDetail("0100301410", 0.0f);//未造成新的出血点
	
	if (allfinish)
	{
		AddScoreItemDetail("0100400300", 0.0f);//暂时不统计其他止血点
	}

	if (allfinish)
	{
		if (hasBurnPointDeviate == false)
		    AddScoreItemDetail("0100502410", 0.0f);//
		else
			AddScoreItemDetail("0100502411", 0.0f);//
	}

	if (numBPointsBurnInRightTime == 5)
	{
		AddScoreItemDetail("0100602810", 0.0f);
	}
	else if (numBPointsBurnInRightTime >= 3)
	{
		AddScoreItemDetail("0100602811", 0.0f);

	}
	float leftToolSpeed = m_pToolsMgr->GetLeftToolMovedSpeed();
	float rightToolSpeed = m_pToolsMgr->GetRightToolMovedSpeed();
	if (leftToolSpeed > 10 || rightToolSpeed > 10)
	{
		AddScoreItemDetail("0100700802", 0);//移动速度过快，有安全隐患
	}
	else if (leftToolSpeed < 5 && rightToolSpeed < 5)
	{
		AddScoreItemDetail("0100700800", 0);//移动平稳流畅
	}
	else
	{
		AddScoreItemDetail("0100700801", 0);//移动速度较快
	}
	//
	int TimeUsed = GetElapsedTime();
	if (TimeUsed < 120)
		AddScoreItemDetail("0100800500", 0);//1分钟内完成所有操作
	else if (TimeUsed < 180)
		AddScoreItemDetail("0100800501", 0);//在2分钟~3分钟内完成所有操作
	else
		AddScoreItemDetail("0100800502", 0);//完成所有规定操作时超过了3分钟

	__super::OnSaveTrainingReport();
}
