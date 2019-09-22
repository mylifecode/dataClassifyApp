#include "stdafx.h"
#include "TrainScoreSystem.h"
#include "IObjDefine.h"
#include "ScoreMgr.h"
#include "TipMgr.h"
#include "GallNewTraining.h"
#include "VeinconnectObject.h"
#include "MXEventsDump.h"
#include "MXToolEvent.h"
#include "MisMedicOrganAttachment.h"
#include "TextureBloodEffect.h"
#include "MisMedicObjectUnion.h"
#include "TrainingCommon.h"
#include "MXOgreGraphic.h"
#include "MxSliceOffOrganEvent.h"
#include "MisMedicObjLink_Approach.h"
#include "OnLineGradeMgr.h"

TrainScoreSystem::TrainScoreSystem(CBasicTraining * HostTrain)
:m_safeCutSpacing(0.15f),
m_safeClipSpacing(.5f)
{
	m_TubeCutStepFinished = false;
	m_TrainRunning = false;
	m_HostTrain = HostTrain;
	m_IsSeperateVeinFinish = false;
	m_IsSeperateVeinBottomFinish = false;
	m_bSevereErrorForTrain = false;
	m_bIsTrainFinished = false;

	//for operate item
	m_canAddOperateItem = false;
	m_damageBiliaryTimes = 0;

    m_bCysticDuctCutted = false;
    m_bArteriaeCysticaCutted = false;
}
//=========================================================================================
TrainScoreSystem::~TrainScoreSystem()
{
}
//=========================================================================================
void TrainScoreSystem::ProcessTrainEvent(MxEvent * pEvent)
{
	if (!pEvent || !m_HostTrain)
		return;

	MxToolEvent * pToolEvent = (MxToolEvent *)pEvent;

	MisMedicOrganInterface * organ = (MisMedicOrganInterface*)pToolEvent->m_pUserPoint;

	TextureBloodTrackEffect *track = NULL;

	switch( pToolEvent->m_enmEventType )
	{
	case MxEvent::MXET_Cut:
		organ = pToolEvent->GetOrgan();

		if(organ->GetCreateInfo().m_objTopologyType == DOT_TUBE)
		  ScoreForCutTube(organ);
		else if(organ->GetCreateInfo().m_objTopologyType == DOT_VOLMESH)
		  ScoreForCutTube(organ);
		break;
	case MxEvent::MXET_AddHemoClip:
		organ = pToolEvent->GetOrgan();
        
		if (organ == NULL)
        {
            TitanicClipInfo::s_clipEmptyCount++;
            return;
        }
		if(organ->GetCreateInfo().m_objTopologyType == DOT_TUBE)
		   ScoreForAddHemoClip(organ->m_OrganID);
		else if(organ->GetCreateInfo().m_objTopologyType == DOT_VOLMESH)
		   ScoreForAddHemoClip(organ->m_OrganID);
		break;
	case MxEvent::MXET_AddSilverClip:
		ScoreForAddSilverClip(organ->m_OrganID);
		break;
	case MxEvent::MEXT_Coaulation_DeleteLine:
		if(organ->GetCreateInfo().m_objTopologyType == DOT_VEINCONNECT)
		   ScoreForCongulateConnect((VeinConnectObject*)organ , organ->m_OrganID);
		break;
	case MxEvent::MXET_LIVER_DAMAGE:
		  HintForDamageLiver();
		break;
	case MxEvent::MXET_PunctureSurface:
		 HintForPunctureOrgan(organ->GetOrganType());
		
		break;
	case MxEvent::MXET_BleedStart:
		{
			track = (TextureBloodTrackEffect*)pToolEvent->m_pUserPoint;
			organ = track->m_OrganIf;
			AddBleedOperateItem(organ->m_OrganID);
		}
		break;
	case MxEvent::MXET_SliceOffOrgan:
		{
			MxSliceOffOrganEvent* pTearOrganEvent = static_cast<MxSliceOffOrganEvent*>(pToolEvent);
			ScoreForSliceOffOrgan(pTearOrganEvent);
		}
		break;
	default:
		break;
	}

	if ( !m_bSevereErrorForTrain )
	{
		if (m_TubeCutStepFinished && m_IsSeperateVeinBottomFinish )
		{
			m_bIsTrainFinished = true;
		}
	}
	return;
}
//=========================================================================================
void TrainScoreSystem::SetTrainRunning(bool run)
{
	m_TrainRunning = run;
}
//=========================================================================================
void TrainScoreSystem::ScoreForCutTube(MisMedicOrganInterface * pOrganInterface)
{
	if(!m_canAddOperateItem)
		return;

	CNewGallTraining * pTraining = (CNewGallTraining *)m_HostTrain;
	MisMedicOrgan_Ordinary *pOrgan = (MisMedicOrgan_Ordinary*)pOrganInterface;
	DynamicObjType curOrganType = pOrgan->GetOrganType();
	int curOrganId = pOrgan->m_OrganID;
	//设置被剪的标志
	pOrgan->setProperty(OPN_AlreadyBeCut,true);

	if(curOrganId != EDOT_BRAVERY_ARTERY && curOrganId != EDOT_CYSTIC_DUCT)
	{
		switch(curOrganId)
		{
			case EDOT_COMMON_BILE_DUCT:
				 CScoreMgr::Instance()->Grade("CommonBileDuctCutFailed");
				 pTraining->TrainingFatalError("CommonBileDuctCutFailed");
				 break;
		}
		return;
	}

	
	if(!pOrgan->getProperty(OPN_BeCutOnIncorrectArea).ToBool())
	{
		std::vector<MisMedicOrganAttachment*> attachments;
		pOrgan->GetAttachment(MOAType_TiantumClip,attachments);

		int nAttachment = attachments.size();
		if(nAttachment > 1)
		{
			if(pOrgan->m_CutCrossFacesInLastCut.size() == 0)
				return ;

			GFPhysSoftBodyFace * physFace = *(pOrgan->m_CutCrossFacesInLastCut.begin());

			GFPhysSoftBodyNode * pNode = physFace->m_Nodes[0];
			GFPhysVector3 lastCutPoint = pNode->m_UnDeformedPos;
			GFPhysVector3 nearPoint,farPoint,pt3;
			MisMedicTitaniumClampV2 * pAttachment1 = NULL;
			MisMedicTitaniumClampV2 * pAttachment2 = NULL;
			MisMedicTitaniumClampV2 * pAttachment3 = NULL;
			float weights[3] = {0.333f,0.333f,0.333f};
			bool isCorrectCutPoint = false;


			for(int a1 = 0;a1 < attachments.size() - 1;++a1)
			{
				for(std::size_t a2 = a1 + 1;a2 < attachments.size();++a2)
				{
					pAttachment1 = (MisMedicTitaniumClampV2*)attachments[a1];
					pAttachment2 = (MisMedicTitaniumClampV2*)attachments[a2];
					if (pAttachment1->getRelativeFace() && pAttachment2->getRelativeFace())
					{
						nearPoint = pAttachment1->getRelativeFace()->GetUndeformedMassCenter(weights);
						farPoint = pAttachment2->getRelativeFace()->GetUndeformedMassCenter(weights);

						GFPhysVector3 p12 = farPoint - nearPoint;
						GFPhysVector3 p13 = lastCutPoint - nearPoint;
						GFPhysVector3 p23 = lastCutPoint - farPoint;

						if (p12.Dot(p13) * p12.Dot(p23) < 0)
						{
							isCorrectCutPoint = true;
							//只针对释放3个钛夹的情况
							if (nAttachment == 3)
							{
								int index = 3 - a1 - a2;
								pAttachment3 = (MisMedicTitaniumClampV2*)attachments[index];

								//进一步缩放范围
								if (pAttachment3->getRelativeFace())
								{
									pt3 = pAttachment3->getRelativeFace()->GetUndeformedMassCenter(weights);
									p13 = pt3 - nearPoint;
									p23 = pt3 - farPoint;
									if (p13.Dot(p23) < 0)
									{
										GFPhysVector3 p10 = lastCutPoint - nearPoint;
										GFPhysVector3 p30 = lastCutPoint - pt3;

										if (p13.Dot(p10) * p13.Dot(p30) < 0)
										{
											std::swap(pAttachment3, pAttachment2);
											std::swap(pt3, farPoint);
										}
										else
										{
											std::swap(pAttachment3, pAttachment1);
											std::swap(pt3, nearPoint);
										}
									}
								}
							}
							break;
						}
					}
				}

				if(isCorrectCutPoint)
					break;
			}
			
			//剪到正确的位置
			if(isCorrectCutPoint)
			{
				if(pOrgan->GetNumSubParts() < 2)
					return;
				MisMedicOrgan_Ordinary * adhersionOrgan;
				std::vector<MisMedicObjLink_Approach*> adhersions;
				MisMedicObjLink_Approach * curAdhersion = NULL;
				GFPhysVector3 connectPoint;
				bool canScore = false;
				bool canFullScore = false;

				//获取器官之间的连接
				if(curOrganType == EDOT_BRAVERY_ARTERY)
					adhersionOrgan = (MisMedicOrgan_Ordinary*)pTraining->GetOrgan(EDOT_HEPATIC_ARTERY);
				else
					adhersionOrgan = (MisMedicOrgan_Ordinary*)pTraining->GetOrgan(EDOT_COMMON_BILE_DUCT);

				pTraining->GetObjectLink_Approach(pOrgan->m_OrganID,adhersionOrgan->m_OrganID,adhersions);
				SY_ASSERT(adhersions.size() !=0 && "adhersions.size() !=0");
				curAdhersion = adhersions[0];

				//获取连接出的一个节点位置
				GFPhysVector3 deformedPos;

				for(size_t c = 0 ; c < curAdhersion->m_NodeToNodeLinks.size() ; c++)
				{
					if(curAdhersion->m_NodeToNodeLinks[c].m_IsValid)
					{
						if(curAdhersion->m_ConnectOrganA == adhersionOrgan)
						{
							connectPoint = curAdhersion->m_NodeToNodeLinks[c].m_NodeInA->m_UnDeformedPos;
							deformedPos  = curAdhersion->m_NodeToNodeLinks[c].m_NodeInA->m_CurrPosition;
						}
						else
						{
							connectPoint = curAdhersion->m_NodeToNodeLinks[c].m_NodeInB->m_UnDeformedPos;
							deformedPos  = curAdhersion->m_NodeToNodeLinks[c].m_NodeInB->m_CurrPosition;
						}
						break;
					}
				}
				
				
				float nearDis = nearPoint.Distance(connectPoint);
				float farDis = farPoint.Distance(connectPoint);
				if(nearDis > farDis)
				{
					std::swap(nearPoint,farPoint);
					std::swap(pAttachment1,pAttachment2);
					std::swap(nearDis,farDis);
				}

				//是否安全剪断
				float dis;
				dis = nearPoint.Distance(farPoint);
				if((dis = lastCutPoint.Distance(nearPoint)) > m_safeCutSpacing)
				{
					if(nAttachment == 2)
					{
						canScore = true;
					}
					else if(nAttachment == 3)
					{
						canScore = true;
						if(pt3.Distance(connectPoint) < farDis)
							canFullScore = true;
					}
				}

				//新增用剪刀离断的情况
				dis = nearPoint.Distance(lastCutPoint) * 10;
                if (curOrganType == EDOT_CYSTIC_DUCT)
                {
                    pTraining->AddOperateItem("SafeSliceOffCysticDuct", dis, false, MisNewTraining::AM_ReplaceAll);
                    if (!m_bCysticDuctCutted)
                    {
                        COnLineGradeMgr::Instance()->SendGrade("CutCysticDuctRightPlace");
                        m_bCysticDuctCutted = true;
                    }
                    
                }
				else if (curOrganType == EDOT_CYSTIC_DUCT)
                {
                    pTraining->AddOperateItem("SafeSliceOffBraveryArtery", dis, false, MisNewTraining::AM_ReplaceAll);
                    if (!m_bArteriaeCysticaCutted)
                    {
                        COnLineGradeMgr::Instance()->SendGrade("CutArteriaeCysticaRightPlace");
                        m_bArteriaeCysticaCutted = true;
                    }
                }
				//记录分数
				if(canScore)
				{
					if(canFullScore)
					{
						if(curOrganType == EDOT_BRAVERY_ARTERY)
						{
							CTipMgr::Instance()->ShowTip("CutBraveryArteryFinish");
							pTraining->AddOperateItem("CutBraveryArtery",1,true);
							pTraining->m_HasCysticArteryBeCutInRightPlace = 0;
						}
						else if (curOrganType == EDOT_CYSTIC_DUCT)
						{
							CTipMgr::Instance()->ShowTip("CutCysticDuctFinish");
							pTraining->AddOperateItem("CutCysticDuct",1,true);
							pTraining->m_HasCysticDuctBeCutInRightPlace = 0;
						}
					}
					else
					{
						if(curOrganType == EDOT_BRAVERY_ARTERY)
						{
							CTipMgr::Instance()->ShowTip("CutBraveryArteryFinish");

							MxOperateItem * pOperateItem = NULL;
							pTraining->AddOperateItem("CutBraveryArtery",1,true,MisNewTraining::AM_ReplaceAll,&pOperateItem);
							if(pOperateItem)
								pOperateItem->ScaleScoreValueOfLastScoreItem(0.5f,0.5f);
							pTraining->m_HasCysticArteryBeCutInRightPlace = 0;
						}
						else if (curOrganType == EDOT_CYSTIC_DUCT)
						{
							CTipMgr::Instance()->ShowTip("CutCysticDuctFinish");

							MxOperateItem * pOperateItem = NULL;
							pTraining->AddOperateItem("CutCysticDuct",1,true,MisNewTraining::AM_ReplaceAll,&pOperateItem);
							if(pOperateItem)
								pOperateItem->ScaleScoreValueOfLastScoreItem(0.5f,0.5f);
							pTraining->m_HasCysticDuctBeCutInRightPlace = 0;
						}
					}
				}

				pOrgan->setProperty(OPN_FinishCut,true);
			}
			else
			{
				m_bSevereErrorForTrain = true;
				pOrgan->setProperty(OPN_BeCutOnIncorrectArea,true);
				
				if (curOrganType == EDOT_BRAVERY_ARTERY)
				{
					pTraining->m_HasCysticArteryBeCutInRightPlace = 1;
				}
				else if (curOrganType == EDOT_CYSTIC_DUCT)
				{
					pTraining->m_HasCysticDuctBeCutInRightPlace = 1;
				}
			}
		}
		else
		{
			m_bSevereErrorForTrain = true;
			pOrgan->setProperty(OPN_BeCutOnIncorrectArea,true);
		}


		if(m_bSevereErrorForTrain)
		{
			if (curOrganType == EDOT_BRAVERY_ARTERY)
			{
				pTraining->TrainingFatalError("BraveryArteryCutNoClipFailed");
				pTraining->AddOperateItem("ErrorCutBraveryArtery",1,true);
			}
			if (curOrganType == EDOT_CYSTIC_DUCT)
			{
				pTraining->TrainingFatalError("CysticDuctCutNoClipFailed");
				pTraining->AddOperateItem("ErrorCutCysticDuct",1,true);
			}
			return;
		}
	}

	
	MisMedicOrgan_Ordinary * braveryArtery,*cysticDuct;
	if(curOrganType == EDOT_BRAVERY_ARTERY)
	{
		braveryArtery = pOrgan;
		cysticDuct = (MisMedicOrgan_Ordinary*)pTraining->GetOrgan(EDOT_CYSTIC_DUCT);
	}
	else
	{
		braveryArtery = (MisMedicOrgan_Ordinary*)pTraining->GetOrgan(EDOT_BRAVERY_ARTERY);
		cysticDuct = pOrgan;
	}

 	if (braveryArtery && braveryArtery->GetNumSubParts() >= 2 &&
		cysticDuct && cysticDuct->GetNumSubParts() >= 2 &&
		m_TubeCutStepFinished == false)
 	{
 		m_TubeCutStepFinished = true;
 		if (m_TrainRunning)
 			CTipMgr::Instance()->ShowTip("SeparateVeinBottom");

		//剪断三角后也算分离三角
		if(!m_IsSeperateVeinFinish)
		{
			VeinConnectObject* connectObject = dynamic_cast<VeinConnectObject*>(pTraining->GetOrgan(EODT_VEINCONNECT));
			if(connectObject)
			{
				int n0 = connectObject->GetCurrConnectCount();
				int n1 = connectObject->GetInitConnectCount();
				if(n1)
				{
					float ratio = 1.f * n0 / n1;
					if ( ratio < 0.95f)
					{	
						MxOperateItem * pOperateItem = NULL;
						m_IsSeperateVeinFinish = true;
						pTraining->AddOperateItem("SeparateVein",1,true,MisNewTraining::AM_Add,&pOperateItem);
						if(pOperateItem)
						{
							int time = pOperateItem->GetLastOperateTime();
							if(time < 0)
								time = rand() % 20  + 10;
							else
								time -= 15;
							if(time < 0)
								time = 1;
							pOperateItem->SetLastOperateTime(time);
						}
					}
				}
			}
		}
 	}
}

void TrainScoreSystem::ScoreForAddHemoClip(int typeId)
{
	CNewGallTraining * pTraining = (CNewGallTraining*)m_HostTrain;

	if (typeId == EDOT_COMMON_BILE_DUCT || typeId == EODT_UTERUS )
	{
		CScoreMgr::Instance()->Grade("CommonBileDuctClipFailed");
		//m_HostTrain->TrainingFinish("CommonBileDuctClipFailed");
	}
	else if ( typeId == EDOT_HEPATIC_ARTERY )
	{
		CScoreMgr::Instance()->Grade("HepaticArteryClipFailed");
		//m_HostTrain->TrainingFinish("HepaticArteryClipFailed");
	}
	else if ( typeId == EDOT_BRAVERY_ARTERY ||  typeId == EDOT_CYSTIC_DUCT  )
	{
		MisMedicOrganInterface * organIf = m_HostTrain->GetOrgan(typeId);

		int OrganHemiClipCount = organIf->GetAttachmentCount(MOAType_TiantumClip);

		if (typeId == EDOT_BRAVERY_ARTERY)
		{
			pTraining->m_NumClipInCysticArtery = OrganHemiClipCount;
		}
		else if (typeId == EDOT_CYSTIC_DUCT)
		{
			pTraining->m_NumClipInCysticDuct = OrganHemiClipCount;
		}

		if (m_TrainRunning && OrganHemiClipCount > 1 && m_canAddOperateItem)
		{
			if(organIf->getProperty(OPN_AlreadyBeCut).ToBool() == false)
			{
				vector<MisMedicOrganAttachment*> organAttachments;
				organIf->GetAttachment(MOAType_TiantumClip,organAttachments);

				float weight[3] = {0.333f,0.333f,0.333f};
				MisMedicTitaniumClampV2 * clamp1,*clamp2;
				clamp1 = (MisMedicTitaniumClampV2*)organAttachments[0];
				clamp2 = (MisMedicTitaniumClampV2*)organAttachments[1];

				
				if ( OrganHemiClipCount == 2 )
				{
					if(clamp1 && clamp2)
					{
						if (clamp1->getRelativeFace() && clamp2->getRelativeFace())
						{
							GFPhysVector3 pos1 = clamp1->getRelativeFace()->GetUndeformedMassCenter(weight);
							GFPhysVector3 pos2 = clamp2->getRelativeFace()->GetUndeformedMassCenter(weight);
							float dis = pos1.Distance(pos2);

							//if(dis >= m_safeClipSpacing)
							{
								if (typeId == EDOT_BRAVERY_ARTERY)
								{
									CTipMgr::Instance()->ShowTip("ClipBraveryArteryFinish");

									MxOperateItem* pOperateItem = NULL;
									pTraining->AddOperateItem("ClipBraveryArtery", 1.f, true, MisNewTraining::AM_ReplaceAll, &pOperateItem);
									COnLineGradeMgr::Instance()->SendGrade("CutArteriaeCysticaWithClipApplicator");
									if (pOperateItem)
										pOperateItem->ScaleScoreValueOfLastScoreItem(0.5f, 0.5f);
									pTraining->m_HasClipCysticArteryGood = true;
									
								}
								else
								{
									CTipMgr::Instance()->ShowTip("ClipCysticDuctFinish");

									MxOperateItem* pOperateItem = NULL;
									pTraining->AddOperateItem("ClipCysticDuct", 1.f, true, MisNewTraining::AM_ReplaceAll, &pOperateItem);
									COnLineGradeMgr::Instance()->SendGrade("CutCysticDuctWithClipApplicator");
									if (pOperateItem)
										pOperateItem->ScaleScoreValueOfLastScoreItem(0.5f, 0.5f);

									pTraining->m_HasClipCysticDuctGood = true;
								}
							}
						}
					}
				}
				else if(OrganHemiClipCount == 3)
				{
					MisMedicTitaniumClampV2* clamp3 = (MisMedicTitaniumClampV2*)organAttachments[2];

					if(clamp1 && clamp2 && clamp3)
					{
						if (clamp1->getRelativeFace() && clamp2->getRelativeFace() && clamp3->getRelativeFace())
						{
							GFPhysVector3 pos1 = clamp1->getRelativeFace()->GetUndeformedMassCenter(weight);
							GFPhysVector3 pos2 = clamp2->getRelativeFace()->GetUndeformedMassCenter(weight);
							GFPhysVector3 pos3 = clamp3->getRelativeFace()->GetUndeformedMassCenter(weight);

							float len12 = pos1.Distance(pos2);
							float len13 = pos1.Distance(pos3);
							float len23 = pos2.Distance(pos3);

							float min1, min2, min3;
							min1 = std::min(len12, len13);
							min2 = std::min(len12, len23);
							min3 = std::min(len13, len23);

							//if(min1 >= m_safeClipSpacing || min2 >= m_safeClipSpacing || min3 >= m_safeClipSpacing)
							{
								if (typeId == EDOT_BRAVERY_ARTERY)
								{
									CTipMgr::Instance()->ShowTip("ExtendClipBraveryArteryFinish");
									pTraining->AddOperateItem("ClipBraveryArtery", 1.f, true, MisNewTraining::AM_ReplaceAll);
									pTraining->m_HasClipCysticArteryGood = true;
								}
								else
								{
									CTipMgr::Instance()->ShowTip("ExtendClipCysticDuctFinish");
									pTraining->AddOperateItem("ClipCysticDuct", 1.f, true, MisNewTraining::AM_ReplaceAll);
									pTraining->m_HasClipCysticDuctGood = true;
								}
							}
						}
					}
				}
				else
				{
					//移除得分
					if(typeId == EDOT_BRAVERY_ARTERY)
					{
						pTraining->RemoveLastOperateItem("ClipBraveryArtery");
					}
					else
					{
						pTraining->RemoveLastOperateItem("ClipCysticDuct");
					}
				}
			}
		}

		if ( OrganHemiClipCount <= 3 )
			CTipMgr::Instance()->ShowTip("AddHemoClip");
		else
			CTipMgr::Instance()->ShowTip("AddHemoClipError");
	}
}

void TrainScoreSystem::ScoreForAddSilverClip(int typeId)
{
	CTipMgr::Instance()->ShowTip("AddSilverClip");
}

void TrainScoreSystem::ScoreForCongulateConnect(VeinConnectObject * connobj , int typeId)
{
	if(m_canAddOperateItem)
	{
		CNewGallTraining * pTraining = dynamic_cast<CNewGallTraining*>(m_HostTrain);
	
		if (typeId == EODT_VEINCONNECT || typeId == EODT_VEINBOTTOMCONNECT)
		{
			if (m_TrainRunning)
			{
				if (typeId == EODT_VEINCONNECT )
				{
					int currConnectCount = connobj->GetCurrConnectCount();

					int initConnectCount = connobj->GetInitConnectCount();

					float fPercent = (float) currConnectCount / (float)initConnectCount;

					if ( fPercent < 0.55f  && !m_IsSeperateVeinFinish)
					{	
						pTraining->m_CholeyStripPercent = fPercent;
						m_IsSeperateVeinFinish = true;
						CScoreMgr::Instance()->Grade("SeparateVein");
						if(!m_TubeCutStepFinished)
						{
							CTipMgr::Instance()->ShowTip("ClipBraveryArteryAndCysticDuct");
							CTipMgr::Instance()->ShowTip("SeparateVeinFinish");
						}
						pTraining->AddOperateItem("SeparateVein",1,true);
						pTraining->showDebugInfo(fPercent);
					}
					else
						pTraining->showDebugInfo("separation finish : ",fPercent);
				}
				else if (typeId == EODT_VEINBOTTOMCONNECT)
				{
					int currConnectCount = connobj->GetCurrConnectCount();

					//int initConnectCount = connobj->GetInitConnectCount();

					//float fPercent = (float) currConnectCount / (float)initConnectCount;

                    if (currConnectCount == 0 && !m_IsSeperateVeinBottomFinish)
					{	
						m_IsSeperateVeinBottomFinish = true;
						CTipMgr::Instance()->ShowTip("SeparateVeinBottomFinish");
						pTraining->AddOperateItem("SeparateVeinBottom",1,true);
						pTraining->AddOperateItem("TakeOutGallbladder",1,true);
					}
				}
			}
		}
	}
}

void TrainScoreSystem::ScoreForSliceOffOrgan(MxSliceOffOrganEvent* pEvent)
{
	MisMedicOrganInterface * pOrgan = pEvent->GetOrgan();
	DynamicObjType organType = pOrgan->GetOrganType();

	if(organType == EDOT_CYSTIC_DUCT || organType == EDOT_BRAVERY_ARTERY)
	{
		bool hasError = true;
		std::vector<MisMedicOrganAttachment*> attachments;
		pOrgan->GetAttachment(MOAType_TiantumClip,attachments);
		int n = attachments.size();

		if(n)
		{
			MisMedicTitaniumClampV2 * pAttachment1 = NULL;
			MisMedicTitaniumClampV2 * pAttachment2 = NULL;
			float weights[3] = {0.333f,0.333f,0.333f};
			GFPhysVector3 point1,point2;
			GFPhysVector3 slicePoint = pEvent->GetSlicePoint();

			for(int a1 = 0;a1 < attachments.size() - 1;++a1)
			{
				for(std::size_t a2 = a1 + 1;a2 < attachments.size();++a2)
				{
					pAttachment1 = (MisMedicTitaniumClampV2*)attachments[a1];
					pAttachment2 = (MisMedicTitaniumClampV2*)attachments[a2];
					if (pAttachment1->getRelativeFace() && pAttachment2->getRelativeFace())
					{
						point1 = pAttachment1->getRelativeFace()->GetUndeformedMassCenter(weights);
						point2 = pAttachment2->getRelativeFace()->GetUndeformedMassCenter(weights);

						GFPhysVector3 p12 = point2 - point1;
						GFPhysVector3 p13 = slicePoint - point1;
						GFPhysVector3 p23 = slicePoint - point2;

						if (p12.Dot(p13) * p12.Dot(p23) < 0)
						{
							hasError = false;
							break;
						}
					}
				}
			}

			if(!hasError)
			{
				float minDis = 10000.f;
				for(std::size_t a = 0;a < attachments.size();++a)
				{
					pAttachment1 = (MisMedicTitaniumClampV2*)attachments[a];
					point1 = pAttachment1->getRelativeFace()->GetUndeformedMassCenter(weights);
					float dis = point1.Distance(slicePoint);
					if(dis < minDis)
						minDis = dis;
				}

                minDis *= 10;
                MisNewTraining * pTraining = static_cast<MisNewTraining*>(m_HostTrain);
                if (organType == EDOT_CYSTIC_DUCT)
                {
                    pTraining->AddOperateItem("SafeSliceOffCysticDuct", minDis, false, MisNewTraining::AM_ReplaceAll);
                    if (!m_bCysticDuctCutted)
                    {
                        COnLineGradeMgr::Instance()->SendGrade("CutCysticDuctRightPlace");
                        m_bCysticDuctCutted = true;
                    }
                }
                else
                {
                    pTraining->AddOperateItem("SafeSliceOffBraveryArtery", minDis, false, MisNewTraining::AM_ReplaceAll);
                    if (!m_bArteriaeCysticaCutted)
                    {
                        COnLineGradeMgr::Instance()->SendGrade("CutArteriaeCysticaRightPlace");
                        m_bArteriaeCysticaCutted = true;
                    }
                }
            }
            else
            {
                MisNewTraining * pTraining = static_cast<MisNewTraining*>(m_HostTrain);
                if (organType == EDOT_CYSTIC_DUCT)
                {   
                    if (!m_bCysticDuctCutted)
                    {
                        COnLineGradeMgr::Instance()->SendGrade("CutCysticDuctWrongPlace");
                        m_bCysticDuctCutted = true;
                    }
                }
                else
                {                                        
                    if (!m_bArteriaeCysticaCutted)
                    {
                        COnLineGradeMgr::Instance()->SendGrade("CutArteriaeCysticaWrongPlace");
                        m_bArteriaeCysticaCutted = true;
                    }
                }
            }
		}

		if(hasError)
		{
			CNewGallTraining * pTraining = static_cast<CNewGallTraining*>(m_HostTrain);

			if(organType == EDOT_CYSTIC_DUCT)
			{
				pTraining->TrainingFatalError("CysiticDuctElecBreak");
				pTraining->AddOperateItem("ErrorCutCysticDuct",1,true);
				pTraining->m_HasCysticDuctBeFatalError = true;
			}
			else
			{
				pTraining->TrainingFatalError("BraveryArteryElecBreak");
				pTraining->AddOperateItem("ErrorCutBraveryArtery",1,true);
				pTraining->m_HasCysticArteryBeFatalError = true;
			}

			m_bSevereErrorForTrain = true;
		}
	}

	//for slice organ also check artery cut finish condition
	MisNewTraining * pTraining = (MisNewTraining*)m_HostTrain;

	MisMedicOrgan_Ordinary * braveryArtery, *cysticDuct;

	cysticDuct = (MisMedicOrgan_Ordinary*)pTraining->GetOrgan(EDOT_CYSTIC_DUCT);

	braveryArtery = (MisMedicOrgan_Ordinary*)pTraining->GetOrgan(EDOT_BRAVERY_ARTERY);

	if (braveryArtery && braveryArtery->GetNumSubParts() >= 2 &&
		cysticDuct && cysticDuct->GetNumSubParts() >= 2)
	{
		m_TubeCutStepFinished = true;
	}
}

void TrainScoreSystem::HintForDamageLiver()
{
	CScoreMgr::Instance()->Grade("LiverElecError");
	CTipMgr::Instance()->ShowTip("LiverElecError");
}
void TrainScoreSystem::HintForPunctureOrgan(int organid)
{
	CScoreMgr::Instance()->Grade("LiverElecError");
	CTipMgr::Instance()->ShowTip("LiverElecError");

	CNewGallTraining * pTraining = dynamic_cast<CNewGallTraining*>(m_HostTrain);
	
	if (organid == EDOT_CYSTIC_DUCT)
	{
		pTraining->m_HasCysticDuctBeIngure = true;
	}
	else if (organid == EDOT_BRAVERY_ARTERY)
	{
		pTraining->m_HasCysticArteryBeIngure = true;
	}
}

void TrainScoreSystem::SetEnabledAddOperateItem(bool enable)
{
	if(enable && dynamic_cast<MisNewTraining*>(m_HostTrain))
	{
		m_canAddOperateItem = true;
	}
	else
		m_canAddOperateItem = false;
}

int TrainScoreSystem::GetNumberOfActiveBleedPoint()
{
	int n = 0;
	std::vector<MisMedicOrganInterface*> organs;
	MisNewTraining * pTraining = (MisNewTraining*)m_HostTrain;

	pTraining->GetAllOrgan(organs);
	//获取未处理的流血点
	for(std::size_t i = 0;i < organs.size();++i)
	{
		switch(organs[i]->GetOrganType())
		{
		case EDOT_LIVER:
// 		case EDOT_BRAVERY_ARTERY:
// 		case EDOT_HEPATIC_ARTERY:
			n += organs[i]->GetNumberOfActiveBleedPoint();
			break;
		}
	}

	return n;
}

void TrainScoreSystem::AddBleedOperateItem(int organId)
{
	if(!m_canAddOperateItem)
		return;
	MisNewTraining * pTraining = (MisNewTraining*)m_HostTrain;
	bool canCalculate = false;

	switch(organId)
	{
	case EDOT_LIVER:
		canCalculate = true;
		break;
	case EDOT_GALLBLADDER:
		//损伤胆囊一次
		pTraining->AddOperateItem("damageGallbladder",1,true,MisNewTraining::AM_MergeValue);
		++m_damageBiliaryTimes;
		break;
	case EDOT_CYSTIC_DUCT:
		//损伤胆囊管一次
		pTraining->AddOperateItem("damageCysticDuct",1,true,MisNewTraining::AM_MergeValue);
		++m_damageBiliaryTimes;
		break;
	case EDOT_COMMON_BILE_DUCT:
		//损伤胆总管一次
		pTraining->AddOperateItem("damageBileDuct",1,true,MisNewTraining::AM_MergeValue);
		++m_damageBiliaryTimes;
		break;
	case EDOT_BRAVERY_ARTERY:
		//损伤胆囊动脉一次
		pTraining->AddOperateItem("damageBraveryArtery",1,true,MisNewTraining::AM_MergeValue);
		++m_damageBiliaryTimes;
		break;
	case EDOT_HEPATIC_ARTERY:
		//损伤肝动脉一次
		pTraining->AddOperateItem("damageHepaticArtery",1,true,MisNewTraining::AM_MergeValue);
		break;
	}
	
	//统计流血次数
	if(canCalculate)
		pTraining->AddOperateItem("BleedTimes",1,true,MisNewTraining::AM_MergeValue);
}

void TrainScoreSystem::SaveOperateItems()
{
	if(!m_canAddOperateItem)
		return;
	MisNewTraining * pTraining = (MisNewTraining*)m_HostTrain;

	//胆道系统损伤次数
	pTraining->AddOperateItem("damageBiliaryTrackTimes",m_damageBiliaryTimes,false,MisNewTraining::AM_MergeValue);
	int n = GetNumberOfActiveBleedPoint();
	for(int i = 0 ;i < n;++i)
		pTraining->AddOperateItem("unhandleBleedPointTimes",1,false,MisNewTraining::AM_MergeAll);
}