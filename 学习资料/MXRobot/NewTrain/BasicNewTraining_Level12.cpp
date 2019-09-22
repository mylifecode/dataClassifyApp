#include "BasicNewTraining_Level12.h"
#include "MXEventsDump.h"
#include "Inception.h"
#include "Instruments/MisCTool_PluginClamp.h"
#include "MisMedicOrganAttachment.h"
#include "Instruments/ElectricHook.h"
#include "Instruments/GraspingForceps.h"
#include <fstream>
#include <windows.h>
#include "MisMedicObjectUnion.h"
#include "SYScoreTableManager.h"
using namespace std;

#define MARKEDVESSELTEXNAME "training12_smallxueguan.png"
#define ORIGINVESSELTEXNAME "training12_xueguan_d.png"

void NewTrainingHandleEvent12(MxEvent * pEvent, ITraining * pTraining);

CBasicNewTraining_Level12::CBasicNewTraining_Level12(void):m_nSmallTubeStep(10)
{
	srand(GetTickCount());
	m_pSphereOrgan = NULL;
	m_pLeftBigOrgan = NULL;
	m_pRightBigOrgan = NULL;
	m_pSkinOrgan = NULL;
	m_bFinish = false;
	m_nCutSmallTubeCount = 0;
	m_count = 0;
	m_nCutBigTubeCount = 0;
	m_nTeafOffSmallTubeCount = 0;
	m_bLeftBackTubeBreak = false;
	m_bHasTipAlive = false;
	m_fCurrentTipsAliveTime = 0.0f;
}

CBasicNewTraining_Level12::~CBasicNewTraining_Level12(void)
{
	for(OrganToObjectInfoMap::iterator itr = m_organToObjectInfoMap.begin();itr != m_organToObjectInfoMap.end(); ++itr)
	{
		delete itr->second;
	}
	m_organToObjectInfoMap.clear();
	m_pSphereOrgan = NULL;
	m_pLeftBigOrgan = NULL;
	m_pRightBigOrgan = NULL;
	m_pSkinOrgan = NULL;
}

bool CBasicNewTraining_Level12::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{
	bool result = MisNewTraining::Initialize(pTrainingConfig , pToolConfig);

	Init();

	return result;
}





//////////////////////////////////////////////////////////////////////////
//根据undeformedPos将点排序
bool CompareNodeYDESC(const GFPhysSoftBodyNode* lhs, const GFPhysSoftBodyNode* rhs)
{
	return lhs->m_UnDeformedPos.y() >rhs->m_UnDeformedPos.y();
}

bool CompareNodeYASC(const GFPhysSoftBodyNode* lhs, const GFPhysSoftBodyNode* rhs)
{
	return lhs->m_UnDeformedPos.y() <rhs->m_UnDeformedPos.y();
}


bool CompareNodeXDESC(const GFPhysSoftBodyNode* lhs, const GFPhysSoftBodyNode* rhs)
{
	return lhs->m_UnDeformedPos.x() >rhs->m_UnDeformedPos.x();
}

bool CompareNodeXASC(const GFPhysSoftBodyNode* lhs, const GFPhysSoftBodyNode* rhs)
{
	return lhs->m_UnDeformedPos.x() <rhs->m_UnDeformedPos.x();
}
//////////////////////////////////////////////////////////////////////////



void CBasicNewTraining_Level12::Init()
{
	MisMedicOrgan_Ordinary * pDistOrgan = NULL;
	std::vector<MisMedicOrgan_Ordinary*> srcOrgans;

	ReadTrainParam("../Config/Train/Basic/NBT_Training_12/TrainParam.txt");

	//老套路记录各种指针
	DynObjMap::iterator organItr = m_DynObjMap.begin();
	while(organItr != m_DynObjMap.end())
	{
		MisMedicOrgan_Ordinary * organ = dynamic_cast<MisMedicOrgan_Ordinary*>(organItr->second);
		if(organ)
		{
			if(organ->m_OrganID < 10)			//vessel
			{		
				srcOrgans.push_back(organ);
				ObjectOperationInfo *pInfo = new ObjectOperationInfo;
				pInfo->organ = organ;
				
				if(organ->m_OrganID == 3)
				{
					m_pLeftBigOrgan = organ;
					pInfo->setBigVesselParameter(organ);
					organ->setVesselBleedEffectTempalteName("Effect/BleedAfterBigVesselElecCut");
					pInfo->setStopBleedTimeThreshold(3.0f);
				}
				else if(organ->m_OrganID == 6)
				{
					m_pRightBigOrgan = organ;
					pInfo->setBigVesselParameter(organ);
					organ->setVesselBleedEffectTempalteName("Effect/BleedAfterBigVesselElecCut");
					pInfo->setStopBleedTimeThreshold(3.0f);
				}
				else
				{
					m_vecLastTubeIndex.push_back(organ->m_OrganID);
					switch (organ->m_OrganID)
					{
					case 2:
						m_pSmallVesselLeftBack = organ;
						break;
					case 7:
						m_pSmallVesselLeftFront = organ;
						break;
					case 8:
						m_pSmallVesselRightFront = organ;
						break;
					case 5:
						m_pSmallVesselRightBack = organ;
						break;
					default:
						break;
					}
					pInfo->setSmallVesselParameter(organ);
					organ->setVesselBleedEffectTempalteName("Effect/BleedAfterSmallVesselElecCut");
					pInfo->setStopBleedTimeThreshold(1.5f);
				}
				m_Objects.insert(make_pair(organ->m_OrganID, organ));
				m_organToObjectInfoMap.insert(make_pair(organ,pInfo));
			}
			else if(organ->m_OrganID == 100)	//sphere
			{
				pDistOrgan = organ;
				m_pSphereOrgan = organ;
			}
			else if(organ->m_OrganID == 20)    //biaopi
			{
				m_pSkinOrgan = organ;
			}
		}
		++organItr;
	}


	//连接血管与肉球
	if(pDistOrgan && srcOrgans.size())
	{
		for(std::size_t o = 0;o < srcOrgans.size(); ++o)
		{
			MisMedicObjectAdhersion * adhersion = new MisMedicObjectAdhersion();
			adhersion->BuildUniversalLinkFromAToB(*srcOrgans[o] , *pDistOrgan , 0.95f);
			m_ObjAdhersions.push_back(adhersion);

		}
	}

	//init vessel node distance 
	for(std::size_t o = 0;o < srcOrgans.size();++o)
	{
		GFPhysSoftBodyNode * pUpNode = srcOrgans[o]->m_physbody->GetNode(20);
		GFPhysSoftBodyNode * pDownNode = srcOrgans[o]->m_physbody->GetNode(21);
		GFPhysSoftBodyNode * pNode = srcOrgans[o]->m_physbody->GetNodeList();
		while(pNode)
		{
			unsigned int disUp = (unsigned int)(pUpNode->m_CurrPosition.Distance(pNode->m_CurrPosition) * 1000);
			disUp *= 10000;
			unsigned int disDown = (unsigned int(pDownNode->m_CurrPosition.Distance(pNode->m_CurrPosition) * 100)) * 10;	//个位数置零
			unsigned int dis = disUp + disDown + 0;

			PhysNode_Data & nodeData = srcOrgans[o]->GetPhysNodeData(pNode);
			nodeData.m_Dist = dis;
			
			pNode = pNode->m_Next;
		}
	}


	//随机在一根小血管上标记一个绿色区域,故意第一次绿色区域不出现在左后小血管上
	SetCurrentOperateTubeIndex(m_vecLastTubeIndex[rand() % 3 + 1]);

	//////////////////////////////////////////////////////////////////////////

	//获得三角面对

	m_vecFacePairLeftFront = GetFacePairBetweenBigTubeAndSmallTube(m_pLeftBigOrgan, m_pSmallVesselLeftFront);				//左大血管与左前小血管的面对
	m_vecFacePairLeftBack = GetFacePairBetweenBigTubeAndSmallTube(m_pLeftBigOrgan, m_pSmallVesselLeftBack);					//左大血管与左后小血管的面对
	m_vecFacePairRightFront = GetFacePairBetweenBigTubeAndSmallTube(m_pRightBigOrgan, m_pSmallVesselRightFront);			//右大血管与左前小血管的面对
	m_vecFacePairRightBack = GetFacePairBetweenBigTubeAndSmallTube(m_pRightBigOrgan, m_pSmallVesselRightBack);				//右大血管与左后小血管的面对

	//细血管与粗血管建立面面约束
	for(size_t f = 0 ; f < m_vecFacePairLeftFront.size() ; f++)
	{
		std::pair<GFPhysSoftBodyFace*,GFPhysSoftBodyFace*> & facePair = m_vecFacePairLeftFront[f];
		if (m_mapLeftFrontConstraint.find(facePair) == m_mapLeftFrontConstraint.end())
		{
			Face_FaceConnection * ffconnect = new Face_FaceConnection(facePair.first, facePair.second , 0.65f , 0.65f);
			PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(ffconnect);
			m_mapLeftFrontConstraint.insert(make_pair(facePair, ffconnect));
		}
	}

	for(size_t f = 0 ; f < m_vecFacePairLeftBack.size() ; f++)
	{
		std::pair<GFPhysSoftBodyFace*,GFPhysSoftBodyFace*> & facePair = m_vecFacePairLeftBack[f];
		
		if (m_mapLeftBackConstraint.find(facePair) == m_mapLeftBackConstraint.end())
		{
			Face_FaceConnection * ffconnect = new Face_FaceConnection(facePair.first, facePair.second, 0.65f, 0.65f);
			PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(ffconnect);
			m_mapLeftBackConstraint.insert(make_pair(facePair, ffconnect));
		}
	}

	for(size_t f = 0 ; f < m_vecFacePairRightFront.size() ; f++)
	{
		std::pair<GFPhysSoftBodyFace*,GFPhysSoftBodyFace*> & facePair = m_vecFacePairRightFront[f];
		
		if (m_mapRightFrontConstraint.find(facePair) == m_mapRightFrontConstraint.end())
		{
			Face_FaceConnection * ffconnect = new Face_FaceConnection(facePair.first, facePair.second, 0.65f, 0.65f);
			PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(ffconnect);
			m_mapRightFrontConstraint.insert(make_pair(facePair, ffconnect));
		}
		
	}

	for(size_t f = 0 ; f < m_vecFacePairRightBack.size() ; f++)
	{
		std::pair<GFPhysSoftBodyFace*, GFPhysSoftBodyFace*> & facePair = m_vecFacePairRightBack[f];

		if (m_mapRightBackConstraint.find(facePair) == m_mapRightBackConstraint.end())
		{
			Face_FaceConnection * ffconnect = new Face_FaceConnection(facePair.first, facePair.second, 0.65f, 0.65f);
			PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(ffconnect);
			m_mapRightBackConstraint.insert(make_pair(facePair, ffconnect));
		}
	}
    
    m_bShpereBeenClamped = false;
	m_SphereBeClampedNum = 0;
}


bool CBasicNewTraining_Level12::Update( float dt )
{
	bool result = MisNewTraining::Update(dt);
	bool hasGraspSphere = false;					//是否抓住肉球
	MisMedicOrgan_Ordinary* pOrgan = NULL;

	CTool * pLeftTool,*pRightTool;
	//check left tool
	pLeftTool = (CTool *)m_pToolsMgr->GetLeftTool();
	if(pLeftTool)
	{
		if (pLeftTool->GetType() == TT_DISSECTING_FORCEPS)
		{
			MisCTool_PluginClamp * pPluginClamp = pLeftTool->GetClampPlugin();
			if(pPluginClamp)
			{
				std::vector<MisMedicOrgan_Ordinary *> organsClamped;
				pPluginClamp->GetOrgansBeClamped(organsClamped);
				for(size_t c = 0 ; c < organsClamped.size() ; c++)
				{
					if(organsClamped[c] == m_pSphereOrgan)
					{
					   hasGraspSphere = true;
                       //if (!m_bShpereBeenClamped)
                       // {
                       //    m_bShpereBeenClamped = true;
                       // }
					   break;
					}
				}
				if(organsClamped.size() > 0)
                   updateClampedObject(organsClamped[0],pPluginClamp);
			}
		}
		else
		{
			UpdateBurn(pLeftTool,dt);
		}
	}
	
	//check right tool
	pRightTool = (CTool *)m_pToolsMgr->GetRightTool();
	if(pRightTool)
	{
		if (pRightTool->GetType() == TT_DISSECTING_FORCEPS)
		{
			MisCTool_PluginClamp * pPluginClamp = pRightTool->GetClampPlugin(); //GetToolPlugin(pRightTool);
			if(pPluginClamp)
			{
				std::vector<MisMedicOrgan_Ordinary *> organsClamped;
				pPluginClamp->GetOrgansBeClamped(organsClamped);
				for(size_t c = 0 ; c < organsClamped.size() ; c++)
				{
					if(organsClamped[c] == m_pSphereOrgan)
					{
						hasGraspSphere = true;
                        //if (!m_bShpereBeenClamped)
                        //{
                        //   m_bShpereBeenClamped = true;
                        // }
						break;
					}
				}
				if(organsClamped.size() > 0)
				   updateClampedObject(organsClamped[0],pPluginClamp);
			}
		}
		else
		{
			UpdateBurn(pRightTool,dt);
		}
	}

	if (hasGraspSphere && (m_bShpereBeenClamped == false))//new grasp
	{
		m_SphereBeClampedNum++;
	}
	m_bShpereBeenClamped = hasGraspSphere;
	
	
	if (m_Objects.find(m_iCurrentNeedToBurnTubeId) != m_Objects.end() && (!m_CurrMarkTex.isNull()))
	{
		MisMedicOrgan_Ordinary* pCurrentTube = m_Objects[m_iCurrentNeedToBurnTubeId];
		if (hasGraspSphere)
			ApplyTextureToMaterial(pCurrentTube->GetOwnerMaterialPtr(), m_CurrMarkTex, "MixTextureMap");
		else
		{
			Ogre::TexturePtr transTex = Ogre::TextureManager::getSingleton().load("transparent.tga", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D,
				0, 1.0f, false, Ogre::PF_R8G8B8A8);
			ApplyTextureToMaterial(pCurrentTube->GetOwnerMaterialPtr(), transTex, "MixTextureMap");
		}
	}
	//防止2个器械都是抓钳时提示语不对
	if (m_bHasTipAlive)
	{
		m_fCurrentTipsAliveTime += dt;
		if (m_fCurrentTipsAliveTime >= m_fTipsAliveTimeThreshold)
		{
			m_fCurrentTipsAliveTime = 0.0f;
			m_bHasTipAlive = false;
			if (hasGraspSphere)
			{
				CTipMgr::Instance()->ShowTip("PleaseCutTube");
			}
			else
			{
				if(pOrgan == NULL || pOrgan == m_pSkinOrgan)
				{
					CTipMgr::Instance()->ShowTip("TrainingIntro");
				}
			}
		}
	}


	if (m_nCutSmallTubeCount >= 4)
	{
        m_bFinish = true;
		TrainingFinish();
	}

	if (m_nCutBigTubeCount >= 2)
	{
		TrainingFatalError();
	}

	if (m_nTeafOffSmallTubeCount >= 4)
	{
		TrainingFatalError();
	}

	//止血 大血管3秒 小血管1.5秒
	for (std::map<int, MisMedicOrgan_Ordinary*>::iterator iter = m_Objects.begin(); iter != m_Objects.end(); ++iter)
	{
		MisMedicOrgan_Ordinary* pOrgan = iter->second;
		OrganToObjectInfoMap::iterator jter = m_organToObjectInfoMap.find(pOrgan);
		if (jter != m_organToObjectInfoMap.end())
		{
			ObjectOperationInfo* pInfo = jter->second;
			if (!pInfo->bConnected)
			{
				pInfo->fBleedTime += dt;
				if (pInfo->fBleedTime >= pInfo->fStopBleedTimeThreshold)
				{
					pOrgan->stopVesselBleedEffect();
				}
			}
		}
	}


	return result;
}

void CBasicNewTraining_Level12::updateClampedObject(MisMedicOrgan_Ordinary * pOrgan,MisCTool_PluginClamp * pPluginClamp)
{
	if(pOrgan == m_pSphereOrgan)								//抓钳抓住肉球
	{
		for(OrganToObjectInfoMap::iterator itr = m_organToObjectInfoMap.begin();itr != m_organToObjectInfoMap.end();++itr)
		{
			MisMedicOrgan_Ordinary * organ = itr->first;

			ObjectOperationInfo * pInfo = itr->second;

			if(pInfo->bConnected)
			{
				GFPhysSoftBodyNode * pTopNode = pInfo->getTopNode();
				GFPhysSoftBodyNode * pDownNode = pInfo->getDownNode();
				GFPhysSoftBodyNode * pMidNode = pInfo->getMidNode();		//如果结点的物理结点被删除，则返回NULL。已保证当删除物理结点时血管已断
				if(pMidNode == NULL || pTopNode == NULL || pDownNode == NULL)
					continue;

				PhysNode_Data & nodeData = organ->GetPhysNodeData(pMidNode);

				unsigned int dis = (unsigned int)(nodeData.m_Dist);
				
				float disUp = dis / 10000 /1000.f;
				float disDown = (dis % 10000) / 1000.f;
				float curDisUp = pMidNode->m_CurrPosition.Distance(pTopNode->m_CurrPosition);
				float curDisDown = pMidNode->m_CurrPosition.Distance(pDownNode->m_CurrPosition);
				if(curDisUp - disUp > pInfo->breakThreshold || curDisDown - disDown > pInfo->breakThreshold)
				{
					bool bBreak;
					bBreak = BreakVessel(itr->first,pInfo,pMidNode,itr->second->breakRange,pInfo->bConnected);
					if (bBreak)
					{
						if(pInfo->type == ObjectOperationInfo::Small && !m_bFinish)	//small vessel
						{
							MisMedicOrgan_Ordinary* pOrgan = itr->first;
							bool breakRightTube = false;
							if(m_Objects[m_iCurrentNeedToBurnTubeId] == pOrgan)
								breakRightTube = true;
							if (breakRightTube)										//扯断绿色标记的血管，此时应该再剩下的血管中随机标记一根血管为绿色
							{
								///由于需求要求左后细血管是最后一根被标记的血管，所以才有了如下代码。。。
								//刷新出下一个标记血管
								//m_Objects[m_iCurrentNeedToBurnTubeId]->ChangeTexture(ORIGINVESSELTEXNAME , "BaseMap");//SetOrdinaryMatrial("NewTraing12/xueguan");
								for (int k = 0; k != m_vecLastTubeIndex.size(); ++k)
								{
									if (m_iCurrentNeedToBurnTubeId == m_vecLastTubeIndex[k])
									{
										if (m_iCurrentNeedToBurnTubeId == 2)
										{
											m_bLeftBackTubeBreak = true;
										}
										m_vecLastTubeIndex.erase(m_vecLastTubeIndex.begin()+k);
										break;
									}
								}
								if (m_vecLastTubeIndex.size()>0)
								{
									//左后血管的标记区域在最后一次才出现
									int index;
									if (m_bLeftBackTubeBreak)
									{
										index = rand()%m_vecLastTubeIndex.size();
									}
									else
									{
										do 
										{
											if (m_vecLastTubeIndex.size()==1)
											{
												index = 0;
												break;
											}
											index = rand()%m_vecLastTubeIndex.size();
										} while (m_vecLastTubeIndex[index] == 2);
									}
									SetCurrentOperateTubeIndex(m_vecLastTubeIndex[index]);
								}

							}
							else													//扯断的血管并不是绿色标记的血管
							{
								int organID = GetOrganID(pOrgan);
								for (int k = 0; k != m_vecLastTubeIndex.size(); ++k)
								{
									if (organID == m_vecLastTubeIndex[k])
									{
										m_vecLastTubeIndex.erase(m_vecLastTubeIndex.begin()+k);
										if (m_iCurrentNeedToBurnTubeId == 2)
										{
											m_bLeftBackTubeBreak = true;
										}
										break;
									}
								}
							}
						}

						//记录被扯断的血管的个数
						if (pInfo->type == ObjectOperationInfo::Small)
						{
							++m_nCutSmallTubeCount;
						}
						if (pInfo->type == ObjectOperationInfo::Big)
						{
							++m_nCutBigTubeCount;
						}
						CTipMgr::Instance()->ShowTip("BreakVessel");
						m_bHasTipAlive = true;
						itr->second->state = ObjectOperationInfo::FailOperate;		
					}		
				}
			}
		}
	}
	else	//抓钳抓住血管
	{
		OrganToObjectInfoMap::iterator itr = m_organToObjectInfoMap.find(pOrgan);
		if(itr == m_organToObjectInfoMap.end())
			return;
		ObjectOperationInfo * pInfo = itr->second;
		
		MisMedicOrgan_Ordinary * organ = itr->first;

		GFPhysSoftBodyNode * pTopNode = pInfo->getTopNode();
		
		GFPhysSoftBodyNode * pDownNode = pInfo->getDownNode();
		

		for (int c = 0; c < (int)pPluginClamp->m_ClampedOrgans.size(); c++)
		{
			MisCTool_PluginClamp::OrganBeClamped * clampOrgan = pPluginClamp->m_ClampedOrgans[c];

			if (clampOrgan->m_organ != organ)
				continue;

			std::map<GFPhysSoftBodyNode*, MisCTool_PluginClamp::ClampedNodeData> clampedNodesMap = clampOrgan->m_ClampedNodes;

			for (std::map<GFPhysSoftBodyNode*, MisCTool_PluginClamp::ClampedNodeData>::iterator itr = clampedNodesMap.begin(); itr != clampedNodesMap.end(); ++itr)
			{
				GFPhysSoftBodyNode * pNode = itr->first;

				PhysNode_Data & nodeData = organ->GetPhysNodeData(pNode);

				unsigned int dis = (unsigned int)(nodeData.m_Dist);

				if (dis)
				{
					float disUp = dis / 10000 / 1000.f;
					float disDown = (dis % 10000) / 1000.f;
					float curDisUp = pNode->m_CurrPosition.Distance(pTopNode->m_CurrPosition);
					float curDisDown = pNode->m_CurrPosition.Distance(pDownNode->m_CurrPosition);
					float deltaDisUp = curDisUp - disUp;
					float deltaDisDown = curDisDown - disDown;

					bool bBreak = false;
					if (pInfo->bConnected)
					{
						if (deltaDisUp > pInfo->breakThreshold || deltaDisDown > pInfo->breakThreshold)
						{
							bBreak = BreakVessel(pOrgan, pInfo, pNode, pInfo->breakRange, pInfo->bConnected);
							if (bBreak)
							{
								if (pInfo->type == ObjectOperationInfo::Small && !m_bFinish)	//small vessel
								{
									bool breakRightTube = false;
									if (m_Objects[m_iCurrentNeedToBurnTubeId] == pOrgan)
										breakRightTube = true;
									if (breakRightTube)							//扯断绿色标记的血管，此时应该再剩下的血管中随机标记一根血管为绿色
									{
										///由于需求要求左后细血管是最后一根被标记的血管，所以才有了如下代码。。。
										//刷新出下一个标记血管
										//m_Objects[m_iCurrentNeedToBurnTubeId]->ChangeTexture(ORIGINVESSELTEXNAME, "BaseMap");//SetOrdinaryMatrial("NewTraing12/xueguan");
										for (int k = 0; k != m_vecLastTubeIndex.size(); ++k)
										{
											if (m_iCurrentNeedToBurnTubeId == m_vecLastTubeIndex[k])
											{
												if (m_iCurrentNeedToBurnTubeId == 2)
												{
													m_bLeftBackTubeBreak = true;
												}
												m_vecLastTubeIndex.erase(m_vecLastTubeIndex.begin() + k);
												break;
											}
										}
										if (m_vecLastTubeIndex.size() > 0)
										{
											//左后血管的标记区域在最后一次才出现
											int index;
											if (m_bLeftBackTubeBreak)
											{
												index = rand() % m_vecLastTubeIndex.size();
											}
											else
											{
												do
												{
													if (m_vecLastTubeIndex.size() == 1)
													{
														index = 0;
														break;
													}
													index = rand() % m_vecLastTubeIndex.size();
												} while (m_vecLastTubeIndex[index] == 2);
											}
											SetCurrentOperateTubeIndex(m_vecLastTubeIndex[index]);
										}

									}
									else								//扯断的血管并不是绿色标记的血管
									{
										for (int k = 0; k != m_vecLastTubeIndex.size(); ++k)
										{
											if (m_iCurrentNeedToBurnTubeId == m_vecLastTubeIndex[k])
											{
												m_vecLastTubeIndex.erase(m_vecLastTubeIndex.begin() + k);
												if (m_iCurrentNeedToBurnTubeId == 2)
												{
													m_bLeftBackTubeBreak = true;
												}
												break;
											}
										}
									}
								}

								//记录被扯断的血管的个数
								if (pInfo->type == ObjectOperationInfo::Small)
								{
									++m_nTeafOffSmallTubeCount;
									++m_nCutSmallTubeCount;
								}
								if (pInfo->type == ObjectOperationInfo::Big)
								{
									++m_nCutBigTubeCount;
								}
								CTipMgr::Instance()->ShowTip("BreakVessel");
								m_bHasTipAlive = true;
								pInfo->state = ObjectOperationInfo::FailOperate;
								break;
							}
						}
					}
					else
					{
						if (dis & 0x1)
						{
							if (deltaDisDown > pInfo->breakThreshold)    //拉扯下面部分的血管
							{
								bBreak = BreakVessel(pOrgan, pInfo, pNode, pInfo->breakRange, false);
								CTipMgr::Instance()->ShowTip("BreakVesse1");
								m_bHasTipAlive = true;
								break;
							}
						}
						else
						{
							if (deltaDisUp > pInfo->breakThreshold)		//拉扯上面部分的血管
							{
								bBreak = BreakVessel(pOrgan, pInfo, pNode, pInfo->breakRange, false);
								CTipMgr::Instance()->ShowTip("BreakVesse1");
								m_bHasTipAlive = true;
								break;
							}
						}
					}
				}
			}
		}
	}
}


bool CBasicNewTraining_Level12::BreakVessel(MisMedicOrgan_Ordinary * pOrgan,ObjectOperationInfo * pObjectInfo,GFPhysSoftBodyNode * attachedNode,float range,bool bFirstBreak)
{
	if (!pObjectInfo->bConnected)
	{
		return false;
	}
	//1 get the tetrahedrons that need to delete
	int oringinSize,curSize;
	std::vector<Ogre::Vector2> reginInfo;
	oringinSize = reginInfo.size();

	//GFPhysSoftBodyTetrahedron * tetra = pOrgan->m_physbody->GetTetrahedronList();
	GFPhysVectorObj<GFPhysSoftBodyTetrahedron*> vecTearOffTetra;
	
	GFPhysSoftBodyNode * pTopNode = pObjectInfo->getTopNode();
	GFPhysSoftBodyNode * pDownNode = pObjectInfo->getDownNode();
	//while(tetra)
	for(size_t th = 0 ; th < pOrgan->m_physbody->GetNumTetrahedron() ; th++)
	{
		GFPhysSoftBodyTetrahedron * tetra = pOrgan->m_physbody->GetTetrahedronAtIndex(th);

		bool flag = true;
		for(int i = 0;i<4;++i)
		{
			if(tetra->m_TetraNodes[i]->m_UnDeformedPos.Distance(attachedNode->m_UnDeformedPos) > range)
			{
				flag = false;
				break;
			}
			float topRange,downRange;
			float disTop,disDown;
			disTop = pTopNode->m_UnDeformedPos.Distance(tetra->m_TetraNodes[i]->m_UnDeformedPos);
			disDown = pDownNode->m_UnDeformedPos.Distance(tetra->m_TetraNodes[i]->m_UnDeformedPos);
			
			topRange = downRange = 0.5f;
			if(disTop < topRange || disDown < downRange)
			{
				flag = false;
				break;
			}
		}
		if(flag)
			vecTearOffTetra.push_back(tetra);
		//tetra = tetra->m_Next;
	}

	//2 claculate the minPos and maxPos and delete the tetras
	GFPhysVector3 minPos,maxPos;
	if(bFirstBreak)
	{
		if(vecTearOffTetra.size())
		{
			minPos = maxPos = vecTearOffTetra[0]->m_TetraNodes[0]->m_CurrPosition;
			for(int t = 0 ;t < (int)vecTearOffTetra.size();++t)
			{
				for(int n = 0;n<4; ++n)
				{
					GFPhysVector3 & curPos = vecTearOffTetra[t]->m_TetraNodes[n]->m_CurrPosition;
					if(curPos.GetY() > maxPos.GetY())
						maxPos = curPos;
					else if(curPos.GetY() < minPos.GetY())
						minPos = curPos;
				}
			}
		}
	}

	pOrgan->EliminateTetras(vecTearOffTetra);

	//3
	bool bRet = false;
	reginInfo.clear();
	//pOrgan->TestLinkingArea(0,reginInfo);
	curSize = reginInfo.size();


		if(pObjectInfo->bConnected)
		{
			pObjectInfo->bConnected = false;
		}
		
		bRet = true;

		//更新Node所属的集合
		if(bFirstBreak)
		{
			GFPhysSoftBodyNode * pNode = pOrgan->m_physbody->GetNodeList();
			while(pNode)
			{
				PhysNode_Data & nodeData = pOrgan->GetPhysNodeData(pNode);

				unsigned int dis = (unsigned int)(nodeData.m_Dist);
				
				if(dis)
				{
					//算法太过粗糙
					if(pNode->m_CurrPosition.GetY() < minPos.GetY())
						dis += 1;		//down
					else if(pNode->m_CurrPosition.GetY() > maxPos.GetY())
						dis = dis;		//up
					else
					{
						float disMinPos,disMaxPos;
						disMinPos = pNode->m_CurrPosition.Distance2(minPos);
						disMaxPos = pNode->m_CurrPosition.Distance2(maxPos);
						if(disMinPos < disMaxPos)
							dis += 1;
					}
				}
				pNode = pNode->m_Next;
			}
		}

	return bRet;
}

void CBasicNewTraining_Level12::FacesBeRemoved(const GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & faces, GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & createdfaces, MisMedicOrgan_Ordinary * pOrgan)
{
    MisNewTraining::FacesBeRemoved(faces, createdfaces, pOrgan);

	FFCsMap * csMapToCheck[4];
	int numCheck;
	if (pOrgan == m_pLeftBigOrgan)
	{
		csMapToCheck[0] = &m_mapLeftFrontConstraint;
		csMapToCheck[1] = &m_mapLeftBackConstraint;
		numCheck = 2;
	}
	else if (pOrgan == m_pSmallVesselLeftFront)
	{
		csMapToCheck[0] = &m_mapLeftFrontConstraint;
		numCheck = 1;
	}
	else if (pOrgan == m_pSmallVesselLeftBack)
	{
		csMapToCheck[0] = &m_mapLeftBackConstraint;
		numCheck = 1;
	}

	else if (pOrgan == m_pRightBigOrgan)
	{
		csMapToCheck[0] = &m_mapRightFrontConstraint;
		csMapToCheck[1] = &m_mapRightBackConstraint;
		numCheck = 2;
	}
	else if (pOrgan == m_pSmallVesselRightFront)
	{
		csMapToCheck[0] = &m_mapRightFrontConstraint;
		numCheck = 1;
	}
	else if (pOrgan == m_pSmallVesselRightBack)
	{
		csMapToCheck[0] = &m_mapRightBackConstraint;
		numCheck = 1;
	}

	//brute force need optimize
	for(int c = 0 ; c < faces.size() ; c++)
	{
		GFPhysSoftBodyFace * pFace = faces[c];

		for(int n = 0 ; n < numCheck ; n++)
		{
			FFCsMap & csMp = (*csMapToCheck[n]);
			
			FFCsMap::iterator iter = csMp.begin();

			while(iter != csMp.end())
			{
				std::pair<GFPhysSoftBodyFace*,GFPhysSoftBodyFace*> facePair = iter->first;
				if (pFace == facePair.second || pFace == facePair.first)
				{
					PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(iter->second);
					iter->second = NULL;
					iter = csMp.erase(iter);
				}
				else
				{
					iter++;
				}
			}
		}
	}

}
void CBasicNewTraining_Level12::UpdateBurn(CTool * pTool,float dt)
{
	if(!pTool)
		return;
	CElectricHook * pElecHook = dynamic_cast<CElectricHook*>(pTool);
	if(pElecHook)
	{
		std::vector<Ogre::Vector3> burnPos;
		
		float pweights[3];
		
		const GFPhysSoftBodyFace * pFace = pElecHook->GetDistCollideFace();
		
		pElecHook->GetDistPointWeight(pweights);

		MisMedicOrgan_Ordinary * pOrgan = NULL;
		OrganToObjectInfoMap::iterator itr;
		if(pFace)
		{
			//当电凝钩碰到有面面约束的面对时，将该对约束删除，用来模拟分离血管
			const GFPhysCollideObject * pCollideObject = pElecHook->GetDistCollideObject();
			if (pCollideObject)
			{
				pOrgan = (MisMedicOrgan_Ordinary*)pCollideObject->GetUserPointer();
				itr = m_organToObjectInfoMap.find(pOrgan);
				if (itr != m_organToObjectInfoMap.end())
				{
					ObjectOperationInfo * pInfo = itr->second;
					if (pInfo->type == ObjectOperationInfo::Small)
					{
						if (pOrgan == m_pSmallVesselLeftFront)
						{
							for (std::map<std::pair<GFPhysSoftBodyFace*,GFPhysSoftBodyFace*>,GFPhysPositionConstraint*>::iterator iter = m_mapLeftFrontConstraint.begin(); iter != m_mapLeftFrontConstraint.end(); ++iter)
							{
								std::pair<GFPhysSoftBodyFace*,GFPhysSoftBodyFace*> facePair = iter->first;
								if (pFace == facePair.second)
								{
									if (iter->second)
									{
										PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(iter->second);
										delete iter->second;
										iter->second = NULL;
										m_mapLeftFrontConstraint.erase(iter);
										pInfo->m_HasBeDissectioned = true;//面约束只要被删除一个就视为小血管被分离过
										break;
									}
								}
							}
						}
						else if (pOrgan == m_pSmallVesselLeftBack)
						{
							for (std::map<std::pair<GFPhysSoftBodyFace*,GFPhysSoftBodyFace*>,GFPhysPositionConstraint*>::iterator iter = m_mapLeftBackConstraint.begin(); iter != m_mapLeftBackConstraint.end(); ++iter)
							{
								std::pair<GFPhysSoftBodyFace*,GFPhysSoftBodyFace*> facePair = iter->first;
								if (pFace == facePair.second)
								{
									if (iter->second)
									{
										PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(iter->second);
										delete iter->second;
										iter->second = NULL;
										m_mapLeftBackConstraint.erase(iter);
										pInfo->m_HasBeDissectioned = true;//面约束只要被删除一个就视为小血管被分离过
										break;
									}
								}
							}
						}
						else if (pOrgan == m_pSmallVesselRightFront)
						{
							for (std::map<std::pair<GFPhysSoftBodyFace*,GFPhysSoftBodyFace*>,GFPhysPositionConstraint*>::iterator iter = m_mapRightFrontConstraint.begin(); iter != m_mapRightFrontConstraint.end(); ++iter)
							{
								std::pair<GFPhysSoftBodyFace*,GFPhysSoftBodyFace*> facePair = iter->first;
								if (pFace == facePair.second)
								{
									if (iter->second)
									{
										PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(iter->second);
										delete iter->second;
										iter->second = NULL;
										m_mapRightFrontConstraint.erase(iter);
										pInfo->m_HasBeDissectioned = true;//面约束只要被删除一个就视为小血管被分离过
										break;
									}
								}
							}
						}
						else if (pOrgan == m_pSmallVesselRightBack)
						{
							for (std::map<std::pair<GFPhysSoftBodyFace*,GFPhysSoftBodyFace*>,GFPhysPositionConstraint*>::iterator iter = m_mapRightBackConstraint.begin(); iter != m_mapRightBackConstraint.end(); ++iter)
							{
								std::pair<GFPhysSoftBodyFace*,GFPhysSoftBodyFace*> facePair = iter->first;
								if (pFace == facePair.second)
								{
									if (iter->second)
									{
										PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(iter->second);
										delete iter->second;
										iter->second = NULL;
										m_mapRightBackConstraint.erase(iter);
										pInfo->m_HasBeDissectioned = true;//面约束只要被删除一个就视为小血管被分离过
										break;
									}
								}
							}
						}
					}
					else
					{
						if (!m_bHasTipAlive)
						{
							CTipMgr::Instance()->ShowTip("PleaseCutTube");
						}
					}
				}
			}

			if(pElecHook->GetElectricLeftPad()) 
			{
				//是否电到了正确的血管
				bool elecRightTube = false;
				if(m_Objects[m_iCurrentNeedToBurnTubeId] == pOrgan)
					elecRightTube = true;

				if (itr != m_organToObjectInfoMap.end())
				{
					ObjectOperationInfo * pInfo = itr->second;
					if(pInfo->addFaceBurnTime(pFace,dt))											//电的时间是否足够
					{
						bool isElecOnGreenArea = IsElectricBreakOnGreenArea(pOrgan,pFace,pweights);			//电断点是否在绿色区域内

						for(std::size_t n = 0;n<3 ;++n)
						{
							if(BreakVessel(pOrgan,pInfo,pFace->m_Nodes[n],pInfo->burnBreakRange,pInfo->bConnected))
							{
								pInfo->bConnected = false;//mark this vessel as breaked

								if(pInfo->type == ObjectOperationInfo::Small && !m_bFinish)	//small vessel
								{
									if (elecRightTube)
									{
										if (isElecOnGreenArea)
										{
											pInfo->m_BreakType = ObjectOperationInfo::BurnRight;
											m_bHasTipAlive = true;
											CTipMgr::Instance()->ShowTip("ElectricCutOk");
											CScoreMgr::Instance()->Grade("score");						//电切细血管得分
										}
										else
										{
											pInfo->m_BreakType = ObjectOperationInfo::BurnWrong;
											m_bHasTipAlive = true;
											CScoreMgr::Instance()->Grade("ElecBreakErrorSmallVessel");
											CTipMgr::Instance()->ShowTip("BreakVesselPosError");
										}

										//刷新出下一个标记血管
										++m_nCutSmallTubeCount;
										//m_Objects[m_iCurrentNeedToBurnTubeId]->ChangeTexture(ORIGINVESSELTEXNAME , "BaseMap");//SetOrdinaryMatrial("NewTraing12/xueguan");
										
										for (int k = 0; k != m_vecLastTubeIndex.size(); ++k)
										{
											if (m_iCurrentNeedToBurnTubeId == m_vecLastTubeIndex[k])
											{
												if (m_iCurrentNeedToBurnTubeId == 2)
												{
													m_bLeftBackTubeBreak = true;
												}
												m_vecLastTubeIndex.erase(m_vecLastTubeIndex.begin()+k);
												break;
											}
										}
										if (m_vecLastTubeIndex.size()>0)
										{
											//左后血管的标记区域在最后一次才出现
											int index;
											if (m_bLeftBackTubeBreak)
											{
												index = rand()%m_vecLastTubeIndex.size();
											}
											else
											{
												do 
												{
													if (m_vecLastTubeIndex.size()==1)
													{
														index = 0;
														break;
													}
													index = rand()%m_vecLastTubeIndex.size();
												} while (m_vecLastTubeIndex[index] == 2);
											}
											SetCurrentOperateTubeIndex(m_vecLastTubeIndex[index]);
										}
									}
									else
									{
										++m_nCutSmallTubeCount;
										//电错血管
										int organID = GetOrganID(pOrgan);
										for (int k = 0; k != m_vecLastTubeIndex.size(); ++k)
										{
											if (organID == m_vecLastTubeIndex[k])
											{
												m_vecLastTubeIndex.erase(m_vecLastTubeIndex.begin()+k);
												if (m_iCurrentNeedToBurnTubeId == 2)
												{
													m_bLeftBackTubeBreak = true;
												}
												break;
											}
										}
										pInfo->m_BreakType = ObjectOperationInfo::Tear;
										m_bHasTipAlive = true;
										CTipMgr::Instance()->ShowTip("BreakVessel");
										
									}
								}
								else//big vessel breaked
								{
									++m_nCutBigTubeCount;
									m_bHasTipAlive = true;
									CTipMgr::Instance()->ShowTip("BreakVessel");
									CScoreMgr::Instance()->Grade("BigVesselBreak");
									
								}
								break;
							}
						}
					}
					else
					{
						//只有没断的血管才需要计算
						if (pInfo->bConnected)
						{
							if (pInfo->type == ObjectOperationInfo::Type::Small)
							{
								bool isElecOnGreenArea = IsElectricBreakOnGreenArea(pOrgan, pFace, pweights);			//电断点是否在绿色区域内
								if (!isElecOnGreenArea && elecRightTube)
								{
									pInfo->fBurnErrorTime += dt;
									if (pInfo->fBurnErrorTime >= 0.1)
									{
										pInfo->fBurnErrorTime = 0.0f;
										CTipMgr::Instance()->ShowTip("BreakVesselPosError");
										CScoreMgr::Instance()->Grade("SmallVesselBrun");
									}
								}
							}
							else if (pInfo->type == ObjectOperationInfo::Type::Big)
							{
								pInfo->fBurnErrorTime += dt;
								if (pInfo->fBurnErrorTime >= 0.1)
								{
									pInfo->fBurnErrorTime = 0.0f;
									CTipMgr::Instance()->ShowTip("BigVesselBurn");
									CScoreMgr::Instance()->Grade("BigVesselBrun");
								}
							}

						}

					}
				}
				
			} 
			else			//电凝钩没接触到器官时重置burntime
			{
				for (OrganToObjectInfoMap::iterator iter = m_organToObjectInfoMap.begin(); iter != m_organToObjectInfoMap.end(); ++iter)
				{
					(iter->second)->resetBurnTime();
				}
			}
		}
		//pElecHook->OnVeinConnectBurned(burnPos);
	}
}



//随机设置绿色标记区域
void CBasicNewTraining_Level12::SetCurrentOperateTubeIndex(int index)
{
	//标记位置随机出现在血管的第4-6段中任何一段,绿色区域的高度为2截
	m_iCurrentNeedToBurnTubeId = index;
	
	m_fGreenUVAreaBegin = FLT_MAX;
	
	m_fGreenUVAreaEnd = -FLT_MAX;
	
	int temp = rand()%3;

	Ogre::String markPic = "marktubea.tga";
	if (temp == 1)
		markPic = "marktubeb.tga";
	else if (temp == 2)
		markPic = "marktubec.tga";

	m_CurrMarkTex = Ogre::TextureManager::getSingleton().load(markPic, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D,
		                                                                  0, 1.0f, false, Ogre::PF_R8G8B8A8);

	Ogre::HardwarePixelBufferSharedPtr pixelBuffer = m_CurrMarkTex->getBuffer();
	
	pixelBuffer->lock(Ogre::HardwareBuffer::HBL_NORMAL);
	
	const Ogre::PixelBox& pixelBox = pixelBuffer->getCurrentLock();
	
	int width = pixelBox.getWidth();
	
	int height = pixelBox.getHeight();
	
	int depth = pixelBox.getDepth();
	
	unsigned char* imgData = (unsigned char*)pixelBox.data;
	
	int rowOffset = Ogre::PixelUtil::getMemorySize(width, 1, depth, pixelBox.format);
	
	int chanle = rowOffset / width / depth;

    for (int i = 0; i != height; ++i)
	{
		 unsigned char *pRowData = imgData + i*rowOffset;
		 
		 int halfWid = width / 2;

		 unsigned char B = pRowData[chanle*halfWid];
		 unsigned char G = pRowData[chanle*halfWid + 1];
		 unsigned char R = pRowData[chanle*halfWid + 2];
		 unsigned char A = pRowData[chanle*halfWid + 3];
		 if (A > 0)
		 {
			 float texy = (float)i / (float)height;
			 if (texy < m_fGreenUVAreaBegin)
			 {
				 m_fGreenUVAreaBegin = texy;
			 }

			 if (texy > m_fGreenUVAreaEnd)
			 {
				 m_fGreenUVAreaEnd = texy;
			 }
		 }
	}

	pixelBuffer->unlock();

	//set default material for all tube
	std::map<int, MisMedicOrgan_Ordinary*>::iterator itor = m_Objects.begin();
	while (itor != m_Objects.end())
	{
		MisMedicOrgan_Ordinary * pTube = itor->second;
		
		Ogre::TexturePtr transTex = Ogre::TextureManager::getSingleton().load("transparent.tga", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D,
			                                      0, 1.0f, false, Ogre::PF_R8G8B8A8);
		ApplyTextureToMaterial(pTube->GetOwnerMaterialPtr(), transTex, "MixTextureMap");
		itor++;
	}
	//ApplyTextureToMaterial(pCurrentTube->GetOwnerMaterialPtr(), m_CurrMarkTex, "MixTextureMap");
	//int nodeNum = pCurrentTube->m_OrganRendNodes.size();
	//for(int i = 0; i < nodeNum; ++i)	
	//{
	///	pCurrentTube->m_OrganRendNodes[i].m_TextureCoord.x -= m_fGreenUVAreaBegin;
	//	pCurrentTube->m_OrganRendNodes[i].m_TextureCoord.y -= m_fGreenUVAreaBegin;
	//}
}

//是否电切到了绿色区域
bool CBasicNewTraining_Level12::IsElectricBreakOnGreenArea(MisMedicOrgan_Ordinary* pOgran, const GFPhysSoftBodyFace* pFace, float weights[3])
{
	if (pFace&&pOgran)
	{
		int OriginMatID, OriginFaceid;
		pOgran->ExtractFaceIdAndMaterialIdFromUsrData(const_cast<GFPhysSoftBodyFace*>(pFace) , OriginMatID , OriginFaceid);		//获取电切的面的ID

		if (OriginFaceid < pOgran->m_OriginFaces.size())
		{
			//int matchCount = 0;	
			
			MMO_Face face = pOgran->m_OriginFaces[OriginFaceid];
			
			float ycoord = face.GetTextureCoord(0).y * weights[0] 
				         + face.GetTextureCoord(1).y * weights[1]
				         + face.GetTextureCoord(2).y * weights[2];
			

			if (ycoord >= m_fGreenUVAreaBegin - 0.002 && ycoord <= m_fGreenUVAreaEnd + 0.002)
			{
				return true;
			}
			//if (matchCount == 3)
			//{
			//	return true;
			//}
		}
	}
	return false;
}

/*
从血管中筛选出的参与约束的面的排列形状大致如下(*代表点，-,/代表边)：
	*-*
  /	| /
  *-*-*
  /	| /
  *-*-*
  /	| /
  *-*-*
  /	| /
  *-*-*
  /	| /
  *-*-*
  /	| /
  *-*-*
*/
//获得大血管与小血管之间需要建立约束的三角面对
std::vector<std::pair<GFPhysSoftBodyFace*,GFPhysSoftBodyFace*> > CBasicNewTraining_Level12::GetFacePairBetweenBigTubeAndSmallTube(MisMedicOrgan_Ordinary* bigTube, MisMedicOrgan_Ordinary* smallTube)
{
	std::vector<std::pair<GFPhysSoftBodyFace*, GFPhysSoftBodyFace*> >  result;

	smallTube->m_physbody->ManuallyUpdateNodeNormals();

	GFPhysSoftBodyNode* sNode = smallTube->m_physbody->GetNodeList();
	while (sNode)
	{
		Real dist = 1000;
		GFPhysSoftBodyFace * face = bigTube->GetRayIntersectFace(GPVec3ToOgre(sNode->m_UnDeformedPos), GPVec3ToOgre(sNode->m_UnDeformedPos + sNode->m_Normal*0.1f), dist);
		
		if (face)
		{
			GFPhysVector3 faceNorm = (face->m_Nodes[1]->m_UnDeformedPos - face->m_Nodes[0]->m_UnDeformedPos).Cross(face->m_Nodes[2]->m_UnDeformedPos - face->m_Nodes[0]->m_UnDeformedPos);
			faceNorm.Normalize();
			if (dist < 0.05f && faceNorm.Dot(sNode->m_Normal) < -0.92f)
			{
				GFPhysSoftBodyFace * smallChoose = 0;
				float minDot = FLT_MAX;

				std::set<GFPhysSoftBodyFace*>::iterator itor = sNode->m_NodeData->m_IncidentSurfaces.begin();
				
				while (itor != sNode->m_NodeData->m_IncidentSurfaces.end())
				{
					GFPhysSoftBodyFace * smallFace = (*itor);
					if (smallFace->m_FaceNormal.Dot(faceNorm) < minDot)
					{
						minDot = smallFace->m_FaceNormal.Dot(faceNorm);
						smallChoose = smallFace;
					}
					itor++;
				}
				std::pair<GFPhysSoftBodyFace*, GFPhysSoftBodyFace*> tempPair(face, smallChoose);

				result.push_back(tempPair);
			}
		}
		sNode = sNode->m_Next;
	}


	return result;

	/*
	
	std::vector<GFPhysSoftBodyNode*> vecBigVesselNode;
	std::vector<GFPhysSoftBodyNode*> vecSmallVesselNode;
	std::vector<GFPhysSoftBodyNode*> vecSBNode;

	GFPhysSoftBodyNode* pNode = bigTube->m_physbody->GetNodeList();
	while(pNode)
	{
		vecSBNode.push_back(pNode);
		pNode = pNode->m_Next;
	}
	std::sort(vecSBNode.begin(), vecSBNode.end(), CompareNodeXDESC); //根据X降序
	for (int i = 0; i != 11; ++i)
	{
		vecBigVesselNode.push_back(vecSBNode[i]);
	}
	vecSBNode.clear();

	pNode = smallTube->m_physbody->GetNodeList();
	while(pNode)
	{
		vecSBNode.push_back(pNode);
		pNode = pNode->m_Next;
	}
	std::sort(vecSBNode.begin(), vecSBNode.end(), CompareNodeXASC); //根据X升序
	for (int i = 0; i !=11; ++i)
	{
		vecSmallVesselNode.push_back(vecSBNode[i]);
	}
	vecSBNode.clear();


	std::sort(vecBigVesselNode.begin(),vecBigVesselNode.end(), CompareNodeYDESC);		//根据Y降序
	std::sort(vecSmallVesselNode.begin(),vecSmallVesselNode.end(), CompareNodeYDESC);	//根据Y降序

	//最开始的2截和最后的2截的面不参与约束
	vecBigVesselNode.erase(vecBigVesselNode.begin());
	vecBigVesselNode.erase(vecBigVesselNode.begin());
	vecBigVesselNode.erase(vecBigVesselNode.end()-1);
	vecBigVesselNode.erase(vecBigVesselNode.end()-1);

	vecSmallVesselNode.erase(vecSmallVesselNode.begin());
	vecSmallVesselNode.erase(vecSmallVesselNode.begin());
	vecSmallVesselNode.erase(vecSmallVesselNode.end()-1);
	vecSmallVesselNode.erase(vecSmallVesselNode.end()-1);

	std::vector<GFPhysSoftBodyFace*> distFace;				//粗血管上参与约束的面
	std::vector<GFPhysSoftBodyFace*> srcFace;				//细血管上参与约束的面

	///筛选出参与建立面面约束的面对
	//GFPhysSoftBodyFace* pFaces = bigTube->m_physbody->GetFaceList();
	//while(pFaces)
	for(size_t f = 0 ; f < bigTube->m_physbody->GetNumFace() ; f++)
	{
		GFPhysSoftBodyFace * face = bigTube->m_physbody->GetFaceAtIndex(f);

		int count = 0;
		for (int i = 0;i != 3; ++i)
		{
			for (int j = 0; j != vecBigVesselNode.size(); ++j)
			{
				if (vecBigVesselNode[j] == face->m_Nodes[i])
				{
					++count;
				}
			}
		}
		if (count == 2)
		{
			distFace.push_back(face);
		}
		//pFaces = pFaces->m_Next;
	}

	//pFaces = smallTube->m_physbody->GetFaceList();
	//while(pFaces)
	for(size_t f = 0 ; f < smallTube->m_physbody->GetNumFace() ; f++)
	{
		GFPhysSoftBodyFace * face = smallTube->m_physbody->GetFaceAtIndex(f);

		int count = 0;
		for (int i = 0;i != 3; ++i)
		{
			for (int j = 0; j != vecSmallVesselNode.size(); ++j)
			{
				if (vecSmallVesselNode[j] == face->m_Nodes[i])
				{
					++count;
				}
			}
		}
		if (count == 2)
		{
			srcFace.push_back(face);
		}
		//pFaces = pFaces->m_Next;
	}

	int faceNum = min(distFace.size(),srcFace.size());
	for (int i = 0; i != faceNum; ++i)
	{
		std::pair<GFPhysSoftBodyFace*,GFPhysSoftBodyFace*> tempPair(distFace[i], srcFace[i]);
		result.push_back(tempPair);
	}
	return result;
	*/
}


//返回器官ID
int CBasicNewTraining_Level12::GetOrganID(MisMedicOrgan_Ordinary* pOrgan)
{
	int result = -1;
	for (std::map<int, MisMedicOrgan_Ordinary*>::iterator iter = m_Objects.begin(); iter != m_Objects.end(); ++iter)
	{
		if(iter->second == pOrgan)
		{
			result = iter->first;
			break;
		}
	}
	return result;
	 
}


//读取训练相关参数
bool CBasicNewTraining_Level12::ReadTrainParam(const std::string& strFileName)
{
	std::ifstream stream;
	stream.open(strFileName.c_str());
	if(stream.is_open())
	{
		char buffer[200];
		while(stream.getline(buffer,199))
		{
			std::string str = buffer;
			int keyEnd = str.find('(');
			std::string key = str.substr(0,keyEnd);
			if (key == "TipsAliveTime")					//读取剪断面在绿色标记区域内的比值,用于判定是否在绿色标记位置电切
			{
				int valEnd = str.find(')');
				int subValEnd = 0;
				std::string val = str.substr(keyEnd+1,valEnd-(keyEnd+1));
				while (-1 != subValEnd)
				{
					subValEnd = val.find(',');
					std::string valStr = val.substr(0,subValEnd);
					m_fTipsAliveTimeThreshold = atof(valStr.c_str());
					val = val.substr(subValEnd+1);
				}
			}
		}
		return true;
	}
	return false;
}

void CBasicNewTraining_Level12::OnSaveTrainingReport()
{
    Real usedtime = GetElapsedTime();

	if (m_SphereBeClampedNum == 0)
		AddScoreItemDetail("0080101819" , 0.0f);//
	else if (m_SphereBeClampedNum <= 2)
		AddScoreItemDetail("0080101810" , 0.0f);
	else
		AddScoreItemDetail("0080101811" , 0.0f);

	ObjectOperationInfo * small_1_F = m_organToObjectInfoMap[m_pSmallVesselLeftFront];
	ObjectOperationInfo * small_1_B = m_organToObjectInfoMap[m_pSmallVesselLeftBack];

	ObjectOperationInfo * small_2_F = m_organToObjectInfoMap[m_pSmallVesselRightFront];
	ObjectOperationInfo * small_2_B = m_organToObjectInfoMap[m_pSmallVesselRightBack];

	int rightBurnNum = 0;
	int totalBurnNum = 0;
	//第一组物体
	int groupSuccNum = 0;
	{
		if (small_1_F->bConnected == false)
		{
			if (small_1_F->m_BreakType == ObjectOperationInfo::BurnRight)
			{
				groupSuccNum++;
				totalBurnNum++;
				rightBurnNum++;
			}
			else if (small_1_F->m_BreakType == ObjectOperationInfo::BurnWrong)
			{
				groupSuccNum++;
				totalBurnNum++;
			}
		}
		if (small_1_B->bConnected == false)
		{
			if (small_1_B->m_BreakType == ObjectOperationInfo::BurnRight)
			{
				groupSuccNum++;
				totalBurnNum++;
				rightBurnNum++;
			}
			else if (small_1_B->m_BreakType == ObjectOperationInfo::BurnWrong)
			{
				groupSuccNum++;
				totalBurnNum++;
			}
		}

		if (groupSuccNum == 0)
			AddScoreItemDetail("0080302019" , 0.0f);//未完成第一组训练
		else if (groupSuccNum == 1)
			AddScoreItemDetail("0080302011" , 0.0f);//第一组离断1根
		else
			AddScoreItemDetail("0080302010" , 0.0f);//成功完成第一组，离断2根
	}

	{
		groupSuccNum = 0;
		if (small_2_F->bConnected == false)
		{
			if (small_2_F->m_BreakType == ObjectOperationInfo::BurnRight)
			{
				groupSuccNum++;
				totalBurnNum++;
				rightBurnNum++;
			}
			else if (small_2_F->m_BreakType == ObjectOperationInfo::BurnWrong)
			{
				groupSuccNum++;
				totalBurnNum++;
			}
		}
		if (small_2_B->bConnected == false)
		{
			if (small_2_B->m_BreakType == ObjectOperationInfo::BurnRight)
			{
				groupSuccNum++;
				totalBurnNum++;
				rightBurnNum++;
			}
			else if (small_2_B->m_BreakType == ObjectOperationInfo::BurnWrong)
			{
				groupSuccNum++;
				totalBurnNum++;
			}
		}
		if (groupSuccNum == 0)
			AddScoreItemDetail("0080302119", 0.0f);//未完成第二组训练
		else if (groupSuccNum == 1)
			AddScoreItemDetail("0080302111", 0.0f);//第二组离断1根
		else
			AddScoreItemDetail("0080302110", 0.0f);//成功完成第二组，离断2根
	}

	bool dissectGroup[2] = { false, false };

	if (small_1_F->m_HasBeDissectioned || small_1_B->m_HasBeDissectioned)
		dissectGroup[0] = true;

	if (small_2_F->m_HasBeDissectioned || small_2_B->m_HasBeDissectioned)
		dissectGroup[1] = true;

	if (dissectGroup[0] && dissectGroup[1])
		AddScoreItemDetail("0080201910", 0.0f);
	else if(dissectGroup[0] || dissectGroup[1])
		AddScoreItemDetail("0080201911", 0.0f);
	else
		AddScoreItemDetail("0080201919", 0.0f);

	if (totalBurnNum - rightBurnNum > 1)
	{
		AddScoreItemDetail("0080402212", 0.0f); //有多根未在标记位置离断
	}
	else if(totalBurnNum - rightBurnNum > 1)
	{
		AddScoreItemDetail("0080402211", 0.0f); //有一根未在标记位置离断
	}
	else if (totalBurnNum > 0)
	{
		AddScoreItemDetail("0080402210", 0.0f); //均在标记位置离断
	}

	if (totalBurnNum > 0)
	{
		AddScoreItemDetail("0080402310", 0.0f); //temp 默认，逐根离断

		if (m_nCutBigTubeCount == 0)
			AddScoreItemDetail("0080402510", 0.0f); //仅离断标记物体
		else
			AddScoreItemDetail("0080402510", 0.0f); //离断标记以外的物体
	}

	if (totalBurnNum == 4)
	{
		if (rightBurnNum == 4)//4根较细血管均被离断，且均在标记位置离断
		{
			AddScoreItemDetail("0080602400", 0.0f); //
		}
		else
		{
			AddScoreItemDetail("0080602401", 0.0f); //
		}
	}

	//score item
	if (m_pToolsMgr->GetLeftToolMovedDistance() > 10 || m_pToolsMgr->GetRightToolMovedDistance() > 10)
	{
		float leftToolSpeed = m_pToolsMgr->GetLeftToolMovedSpeed();
		float rightToolSpeed = m_pToolsMgr->GetRightToolMovedSpeed();
		if (leftToolSpeed > 10 || rightToolSpeed > 10)
		{
			AddScoreItemDetail("0080700802", 0);//移动速度过快，有安全隐患
		}
		else if (leftToolSpeed < 5 && rightToolSpeed < 5)
		{
			AddScoreItemDetail("0080700800", 0);//移动平稳流畅
		}
		else
		{
			AddScoreItemDetail("0080700801", 0);//移动速度较快
		}
	}
	//
	if (totalBurnNum == 4)
	{
		int TimeUsed = GetElapsedTime();
		if (TimeUsed < 120)
			AddScoreItemDetail("0080800500", 0);//2分钟内完成所有操作
		else if (TimeUsed < 180)
			AddScoreItemDetail("0080800501", 0);//在2分钟~3分钟内完成所有操作
		else
			AddScoreItemDetail("0080800502", 0);//完成所有规定操作时超过了3分钟
	}

	if (rightBurnNum == 4 && m_nCutBigTubeCount == 0)
	{
		AddScoreItemDetail("0080500300", 0);
	}
	else if (totalBurnNum == 4 && m_nCutBigTubeCount == 0)
	{
		AddScoreItemDetail("0080500301", 0);
	}
    if (m_nCutBigTubeCount == 0)
    {
		if (m_bFinish)
		{
			COnLineGradeMgr::Instance()->SendGrade("AccidentalError0", 0, usedtime);
		}
        
        if (m_nCutSmallTubeCount == 4)
        {
            COnLineGradeMgr::Instance()->SendGrade("SeparateTube4", 0, usedtime);
        }
        else if (m_nCutSmallTubeCount == 3)
        {
            COnLineGradeMgr::Instance()->SendGrade("SeparateTube3", 0, usedtime);
        } 
        else if (m_nCutSmallTubeCount == 2)
        {
            COnLineGradeMgr::Instance()->SendGrade("SeparateTube2", 0, usedtime);
        }
        else if (m_nCutSmallTubeCount == 1)
        {
            COnLineGradeMgr::Instance()->SendGrade("SeparateTube1", 0, usedtime);
        }
        else
        {
            COnLineGradeMgr::Instance()->SendGrade("SeparateTube0", 0, usedtime);
        }
    }
    else if (m_nCutBigTubeCount == 1)
    {
        COnLineGradeMgr::Instance()->SendGrade("AccidentalError1", 0, usedtime);        
    }
    else if (m_nCutBigTubeCount == 2)
    {
        COnLineGradeMgr::Instance()->SendGrade("AccidentalError2", 0, usedtime);
    }

    if (m_bShpereBeenClamped)
    {
        COnLineGradeMgr::Instance()->SendGrade("ExposedObject", 0, usedtime);
    }
    else
    {
        COnLineGradeMgr::Instance()->SendGrade("ExposedObject0", 0, usedtime);
    }

    Real totalTime = m_pToolsMgr->GetTotalElectricTime();
    Real validTime = m_pToolsMgr->GetValidElectricTime();
    Real value = 0.0f;
	if (totalTime > 0.0001f)
	{
		value = 100.f * validTime / totalTime;

		if (value > 0.7f)
		{
			COnLineGradeMgr::Instance()->SendGrade("ElecEffecient", 0, usedtime);
		}
		else
		{
			COnLineGradeMgr::Instance()->SendGrade("ElecEffecient0", 0, usedtime);
		}
	}

    //Real toolspeed = m_pToolsMgr->GetLeftToolMovedSpeed();
    //if (!toolspeed)
    //    toolspeed = m_pToolsMgr->GetRightToolMovedSpeed();

    //leftToolSpeed = m_pToolsMgr->GetLeftToolMovedSpeed();
    // rightTooSpeed = m_pToolsMgr->GetRightToolMovedSpeed();
	//Real ToolSpeed = std::max(leftToolSpeed, rightToolSpeed);

    /*if (leftToolSpeed > 0.0f && rightTooSpeed > 0.0f)
    {
    ToolSpeed = (leftToolSpeed + rightTooSpeed) / 2;
    }
    else if (leftToolSpeed == 0.0f && rightTooSpeed > 0.0f)
    {
    ToolSpeed = rightTooSpeed;
    }
    else if (leftToolSpeed > 0.0f && rightTooSpeed == 0.0f)
    {
    ToolSpeed = leftToolSpeed;
    }
    else
    {
    }*/
    //if (ToolSpeed <= 5.0f && ToolSpeed > GP_EPSILON)
     //   COnLineGradeMgr::Instance()->SendGrade("MachineHandle_Normal", 0, usedtime);
    //else if (ToolSpeed > 5.0f && ToolSpeed <= 10.0f)
       // COnLineGradeMgr::Instance()->SendGrade("MachineHandle_Fast", 0, usedtime);
    //else if (ToolSpeed > 10.0f)
      //  COnLineGradeMgr::Instance()->SendGrade("MachineHandle_TooFast", 0, usedtime);
    
    if (m_bFinish)
    {
        if (usedtime < 120)
        {
            COnLineGradeMgr::Instance()->SendGrade("Finished_In2M", 0, usedtime);
            if (m_nCutSmallTubeCount == 4 && m_nCutBigTubeCount == 0)
            {
                COnLineGradeMgr::Instance()->SendGrade("Twohands_Cooperation", 0, usedtime);
            }
        }
        else if (usedtime >= 120 && usedtime < 180)
        {
            COnLineGradeMgr::Instance()->SendGrade("Finished_In3M", 0, usedtime);
        }
    }
    else
    {
        COnLineGradeMgr::Instance()->SendGrade("UnFinish_In3M", 0, usedtime);
    }

    __super::OnSaveTrainingReport();

    m_nCutSmallTubeCount = 0;
    m_nCutBigTubeCount = 0;
    m_nTeafOffSmallTubeCount = 0;
}

//======================================================================================================================
SYScoreTable* CBasicNewTraining_Level12::GetScoreTable()
{
	return SYScoreTableManager::GetInstance()->GetTable("01100801");
}