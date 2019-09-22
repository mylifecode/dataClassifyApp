#include "SYCuttingTubeTrain.h"
#include "MisMedicOrganOrdinary.h"

#include "VeinConnectObject.h"
#include "TextureBloodEffect.h"
#include "MisMedicEffectRender.h"
#include "MXEventsDump.h"
#include "MXToolEvent.h"
#include "OgreAxisAlignedBox.h"
#include "Instruments/Tool.h"
#include "Inception.h"
#include "MisMedicRigidPrimtive.h"
#include "CustomCollision.h"
#include "LightMgr.h"
#include <fstream>
#include "Instruments/GraspingForceps.h"
#include "Instruments/Scissors.h"
#include "Instruments/MisCTool_PluginClamp.h"
#include <MMSystem.h>
#include "MisMedicOrganAttachment.h"
#include "CustomConstraint.h"
#include <stack>
#include "MXEvent.h"
#include "Instruments/HarmonicScalpel.h"
#include "MisMedicObjectUnion.h"
#include "EngineCore.h"
#include "DeferredRendFrameWork.h"
#include "SceneSpeciBag.h"
#include "SYScoreTableManager.h"

    //=============================================================================================
	void NewTrainingHandleEvent_Cutting(MxEvent * pEvent, ITraining * pTraining)
	{
		if (!pEvent || !pTraining)
			return;
		
		/*
		if (pEvent->m_enmEventType == MxEvent::MXET_Cut)
		{
			SYCuttingTubeTrain* pTrain = dynamic_cast<SYCuttingTubeTrain*>(pTraining);
			pTrain->BFSLinkedAera();
			pTrain->IsCutOnRightPlace();
			pTrain->ProcessAfterCut();
		}
		*/
		SYCuttingTubeTrain* pTrain = dynamic_cast<SYCuttingTubeTrain*>(pTraining);

		MxToolEvent * pToolEvent = NULL;
		
		switch (pEvent->m_enmEventType)
		{
		case MxEvent::MXET_Cut:
			 {
			    pTrain->BFSLinkedAera();
			    pTrain->IsCutOnRightPlace();
			    pTrain->ProcessAfterCut();
			 }
			 break;

		case MxEvent::MXET_ElecCoagEnd:
		case MxEvent::MXET_ElecCutEnd:
			 pToolEvent = (MxToolEvent*)(pEvent);
			
			 if (pToolEvent->m_DurationTime > pTrain->GetGradLogicData().m_MaxContinueElecTime)
				 pTrain->GetGradLogicData().m_MaxContinueElecTime = pToolEvent->m_DurationTime;

			 break;
		}
	}


    SYCuttingTubeTrain::GradLogicData::GradLogicData(MisNewTraining * train)
	{
		Reset();
		m_train = train;
		m_IsTearOff = false;
		m_CutNum = 0;
		m_NumSegmentThrowSucced = 0;
		//m_continElecTime = 0;
		m_MaxContinueElecTime = 0;
//		m_IsInElec = false;
	}

	void SYCuttingTubeTrain::GradLogicData::Reset()
	{
		m_IsGraspInRightPlace = false;
		m_IsCutOff = false;
		m_IsCutInWrongPlace = false;
		m_ValidGraspTexRange = Ogre::Vector2(FLT_MAX, -FLT_MAX);
		m_IsThrowBageSucceed = false;
		m_HasSubmitItem = false;
	}

	void SYCuttingTubeTrain::GradLogicData::SendStepDetailScoreAndReset()
	{
		//send item score first
		if (m_HasSubmitItem)
		{
			QString StepPrefix1;
			QString StepPrefix2;
			if (m_CutNum == 0)
			{
				StepPrefix1 = "015010";
				StepPrefix2 = "015020";
			}
			else if (m_CutNum == 1)
			{
				StepPrefix1 = "015030";
				StepPrefix2 = "015040";
			}
			else
			{
				StepPrefix1 = "015050";
				StepPrefix2 = "015060";
			}
			if (m_IsGraspInRightPlace)
				m_train->AddScoreItemDetail(StepPrefix1 + "0610", 0);
			else
				m_train->AddScoreItemDetail(StepPrefix1 + "0611", 0);

			if (m_IsCutOff)
			{
				if (m_IsCutInWrongPlace)
					m_train->AddScoreItemDetail(StepPrefix1 + "3411", 0);
				else
					m_train->AddScoreItemDetail(StepPrefix1 + "3410", 0);
			}
			else
				m_train->AddScoreItemDetail(StepPrefix1 + "3419", 0);

			if (m_IsThrowBageSucceed)
				m_train->AddScoreItemDetail(StepPrefix2 + "2200", 0);
			else
				m_train->AddScoreItemDetail(StepPrefix2 + "2209", 0);
		}

		//reset
		Reset();
		m_CutNum++;
	}

	void SYCuttingTubeTrain::GradLogicData::SetCutOff()
	{
		m_IsCutOff = true;
		Dirty();
	}
	void SYCuttingTubeTrain::GradLogicData::SetCutWrongPlace()
	{
		m_IsCutInWrongPlace = true;
		Dirty();
	}
	void SYCuttingTubeTrain::GradLogicData::SetGraspInRightPlace()
	{
		m_IsGraspInRightPlace = true;
		Dirty();
	}

	void SYCuttingTubeTrain::GradLogicData::SetThrowSucceed()
	{
		m_IsThrowBageSucceed = true;
		m_NumSegmentThrowSucced++;
		Dirty();
	}

	void SYCuttingTubeTrain::GradLogicData::Dirty()
	{
		m_HasSubmitItem = true;
	}

	void SYCuttingTubeTrain::GradLogicData::SetBeTeared()
	{
		m_IsTearOff = true;
	}

	SYCuttingTubeTrain::SYCuttingTubeTrain(void)
		: m_bNeedThrowInBag(false)
		, m_step(15)
		, m_pTerrain(NULL)
		, m_pTube(NULL)
		, m_currentStep(0)
		, m_bCompeletlyCut(false)
		, m_iClusterNum(0)
		, m_fScale(1.0f / 21.0f)
		, m_fTearOffThreshold(8.0f)
		, m_CutCount(0)
		, m_bPerfectInLevel1(true)
		, m_bPerfectInLevel2(true)
		, m_bScaleBag(false)
		, m_bCutTestPass(false)
		, m_throwErrorCount(0)
		, m_bHasGraspTube(false)
		, m_bScaling(false)
		, m_fBagScaleX(1)
		, m_fBagScaleY(1)
		, m_fBagScaleZ(1)
		, m_bNeedEnterNextLevel(false)
		, m_bPreInGround(false)
		, m_bInGround(false)
		, m_bShouldShowCutArea(false)
		, m_bHasBeenTearOffTube(false)
		, m_iCurrentBagID(10)
		, m_iCurrentBagPosIndex(0)
		, m_fCurrentTime(0.0f)
		, m_LastTexChangeFlag(-1)
		, m_LastTexChangeOffset(FLT_MAX)
		, m_TagOffsetPercent(0)
		, m_pBag(0)
		, m_gradLogicData(this)
{
	InitStep();
	memset(m_usedDone, 0, 6 * sizeof(bool));
	memset(m_cuttedPosCorrect, 0, 6 * sizeof(int));
	memset(m_isTearoff, 0, 6 * sizeof(bool));
	memset(m_fallCount, 0, 6 * sizeof(int));
	memset(m_showGreenTime, 0, 6 * sizeof(float));
	memset(m_cuttedPosTime, 0, 6 * sizeof(float));

	m_trainbegintime = 0;
	m_currentCut = 0;
	//m_CupCol = 0;
	//m_pBagCircle = 0;
}
//=============================================================================================
SYCuttingTubeTrain::~SYCuttingTubeTrain(void)
{
	m_TubeTexturPtr.setNull();

	for (int i = 0; i != m_ObjAdhersions.size(); ++i)
	{
		if (m_ObjAdhersions[i])
		{
			delete m_ObjAdhersions[i];
			m_ObjAdhersions[i] = NULL;
		}
	}
	m_ObjAdhersions.clear();


	ITool * pLeftTool,*pRightTool;
	pLeftTool = m_pToolsMgr->GetLeftTool();
	pRightTool = m_pToolsMgr->GetRightTool();
	if(pLeftTool && pLeftTool->GetType() == TT_GRASPING_FORCEPS)
	{
		CGraspingForceps * pTool = (CGraspingForceps*)pLeftTool;
		pTool->ReleaseClampedOrgans();
	}
	if(pRightTool && pRightTool->GetType() == TT_GRASPING_FORCEPS)
	{
		CGraspingForceps * pTool = (CGraspingForceps*)pRightTool;
		pTool->ReleaseClampedOrgans();
	}

	if (m_pBag)
	{
		delete m_pBag;
		m_pBag = NULL;
	}
}
//======================================================================================================================
bool SYCuttingTubeTrain::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{

	bool result = MisNewTraining::Initialize(pTrainingConfig , pToolConfig);

	PhysicsWrapper::GetSingleTon().m_dynamicsWorld->SoftCollisionHashGridSize(0.8f);///----------

	DynObjMap::iterator itor = m_DynObjMap.begin();
	while(itor != m_DynObjMap.end())
	{
		MisMedicOrganInterface *oif = itor->second;
		MisMedicOrgan_Ordinary * organmesh = dynamic_cast<MisMedicOrgan_Ordinary*>(oif);
		if(organmesh)
		{
			MisMedicDynObjConstructInfo & cs = oif->GetCreateInfo();
			m_Objects.insert(make_pair(cs.m_OrganType, organmesh));
			m_ConstructInfoMap.insert(make_pair(cs.m_OrganType, cs));
		}
		++itor;
	}

	Ogre::Node::ChildNodeIterator iter = m_pOms->GetRootNode()->getChildIterator();
	while (iter.hasMoreElements())
	{
		std::string name = iter.getNext()->getName();
		/*if (name.find("daizi")!= string::npos)
		{
			Ogre::Entity *ent = m_pOms->GetSceneManager()->getEntity(name);  
			m_pBagForScale = ent->getParentSceneNode();  
			Ogre::Vector3 pos = m_ConstructInfoMap[m_iCurrentBagID].m_Position;
			m_pBagForScale->setPosition(pos);
			m_pBagForScale->setVisible(false);
		}*/
		/*if (name.find("TorusJinshu") != string::npos)
		{
			Ogre::Entity *ent = m_pOms->GetSceneManager()->getEntity(name);
			m_pBagCircle = ent->getParentSceneNode();
		}*/
		
	}


	LeaveLevel1();
	LeaveLevel2();

	itor = m_DynObjMap.begin();
	while (itor != m_DynObjMap.end())
	{
		MisMedicRigidPrimtive * rigidPrim = dynamic_cast<MisMedicRigidPrimtive*>(itor->second);
		if (rigidPrim && rigidPrim->GetCollisionTriMesh() != 0)
		{
			//m_CupCol = rigidPrim;
		}
		itor++;
	}

	CMXEventsDump::Instance()->RegisterHandleEventsFunc(NewTrainingHandleEvent_Cutting, this);

	m_TubeTexturPtr = Ogre::TextureManager::getSingleton().load("marktube.tga" , Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,Ogre::TEX_TYPE_2D,
		0,1.0f,false,Ogre::PF_R8G8B8A8);

	RemoveBagFromWorld();
	ReadTrainParam("../Data/Module/Basic/Cutting/TrainParam.txt");
	
	//DeferredRendFrameWork::Get()->SetGammaCorrectInModulate(1.0f);

	return result;
}
//======================================================================================================================
SYScoreTable* SYCuttingTubeTrain::GetScoreTable()
{
	return SYScoreTableManager::GetInstance()->GetTable("01101501");
}

void SYCuttingTubeTrain::OnTimerTimeout(int timerId, float dt, void* userData)
{
	
}
bool SYCuttingTubeTrain::Update( float dt )
{
	bool result = MisNewTraining::Update(dt);

	LogicProcess(dt);

	return result;
}

void SYCuttingTubeTrain::OnSSContactBuildFinish(const GFPhysAlignedVectorObj<GFPhysSSContactPoint> & ssContact, int numSSContact)
{
	m_TubeNodesContactTerrain.clear();
	if (numSSContact > 0)
	{
		for (int c = 0; c < numSSContact; c++)
		{
			const GFPhysSSContactPoint & cp = ssContact[c];
			if (cp.m_SoftNode->m_SoftBody == m_pTube->m_physbody && cp.m_CollideFaceNodes[0]->m_SoftBody == m_pTerrain->m_physbody)
			{
				m_TubeNodesContactTerrain.insert(cp.m_SoftNode);
			}
		}
	}
}
bool SYCuttingTubeTrain::LogicProcess(float dt)
{
	trainingStep&	st = m_steps[m_currentStep];

	if (st.stepType == eST_Add)
	{
		//退出第一个场景，进入第二个场景
		if (m_currentStep == 3)			
		{
			LeaveLevel1();
			EnterLevel2();
		}
		else	//进入第一个场景
		{
			EnterLevel1();
		} 
		m_area0 = (m_step)*m_fScale;
		m_area1 = (m_step+2)*m_fScale;
		UpdateColor(m_pTube, m_area0, m_area1);
		++m_currentStep;
		m_bCutTestPass = false;
		m_throwErrorCount = 0;
	}
	else if (st.stepType == eST_waitCheck)
	{
		if (m_pTube != NULL)
		{
			//if (m_gradLogicData.m_IsInElec)
			//	m_gradLogicData.m_continElecTime += dt;

			bool isCutOnRightPlace = false;
			bool isGraspOnRightPlace = false;
			m_usedDone[m_currentCut] = true;

			///需要跳转至下一场景时开始计时，并重置状态变量
			if (m_bNeedEnterNextLevel)
			{
				m_fCurrentTime += dt;
				if (m_fCurrentTime >= m_fEnterNextLevelTimeThreshold)
				{
					m_iCurrentBagPosIndex = 3;
					m_currentStep += 1;
					m_fCurrentTime = 0.0f;
					m_bNeedEnterNextLevel = false;
					m_bCutTestPass = false;
					m_bPreInGround = false;
					m_bInGround = false;
					m_bHasBeenTearOffTube = false;
					m_vecClusterResult.clear();
					m_vecClusterTetrahedron.clear();
				}
			}
			else
			{
				if (PlayerHasGraspTube())
				{
					if (IsClampInTheSky())
					{
						m_bShouldShowCutArea = true;
						if (!m_showGreenTime[m_currentCut])
							m_showGreenTime[m_currentCut] = GetElapsedTime();
					}

					if (!m_bCutTestPass && !m_bHasBeenTearOffTube)
					{///拿起来还没剪切的状态
						CTipMgr::Instance()->ShowTip("PleaseElectricTube");///请电切血管绿色标记位置
					}

					//判断是否扯断，并进行处理
					if (m_iClusterNum <= 1 && !m_bHasBeenTearOffTube && IsTearOffTheTube())
					{
						m_bHasBeenTearOffTube = true;
						TearOffTheTube(isGraspOnRightPlace);
						m_pTube->resumeVesselBleedEffect();
						CTipMgr::Instance()->ShowTip("TearOffTube");
						m_isTearoff[m_currentCut] = true;

						m_gradLogicData.SetBeTeared();
						//第一个场景内扯断血管后跳至第二个场景
						//第二个场景内扯断血管训练结束，并提示严重错误
						if (m_currentStep == 1)
						{
							//出提示消息并直接跳到场景2
							m_bNeedEnterNextLevel = true;
							m_bPerfectInLevel1 = false;
							m_iClusterNum = 0;
							m_CutCount = 0;
							m_currentCut = 3;
							return true;
						}
						else if(m_currentStep == 4)	
						{
							//出提示消息，并等待退出
							m_bPerfectInLevel2 = false;
							m_iClusterNum = 0;
							m_CutCount = 0;
							TrainingFatalError();
							return true;
						}
					}
				}
				else///起始时候 器械没有夹着血管
				{
					if (EngineCore::IsUnlocked() && !m_bCutTestPass)///false通常表示还没有剪切
					{
						CTipMgr::Instance()->ShowTip("PleaseGraspTube");///请用抓钳抓取血管橙色标记位置
					}

					m_bShouldShowCutArea = false;///绿色标志不要显示
				}

				//是否应该显示绿色区域 
				if (m_bShouldShowCutArea)
				{
					ChangeTexture(0);
				}
				else
					ChangeTexture(1);

				if (m_iClusterNum >= 2)///已经剪成2段以上了
				{
					if (m_bCutTestPass && IsThrowInBag())//判断是否放入了标本袋
					{
						m_bNeedThrowInBag = true;
						m_bScaling = true;

						HideCutedPartOrgan();
						m_pTube->stopVesselBleedEffect();
						m_iClusterNum = 0;
						++m_CutCount;

						CTipMgr::Instance()->ShowTip("ThrowOK");
						m_gradLogicData.SetThrowSucceed();//CScoreMgr::Instance()->Grade("throwOK");
						m_gradLogicData.SendStepDetailScoreAndReset();
					}
					else
					{
						if (m_bCutTestPass && !m_bHasGraspTube && m_bInGround)
						{
							if (m_bInGround != m_bPreInGround)
							{
								m_bPreInGround = m_bInGround;
								CTipMgr::Instance()->ShowTip("ThrowError");
								CScoreMgr::Instance()->Grade("tubeDrop");
								m_fallCount[m_currentCut]++;
							}
						}
					}
				}
			}
		}


		//袋子缩放动画
		if (m_bScaling && m_bNeedThrowInBag)
		{
			ScaleTheBag();
		}
		else if(!m_bScaling && m_bCutTestPass && m_bNeedThrowInBag)
		{
			RemoveBagFromWorld();
			m_bNeedThrowInBag = false;
			m_bCutTestPass = false;
			//ResetUV(1);

			//移动黄色和绿色区域，这种实现方法是个坑。。。
			switch (m_CutCount)
			{
			case 1:
				m_step -= 8;
				break;
			case 2:
				m_step -= 4;
				break;
			}
			m_fScale = 1.0f/float(m_step+6);
			m_area0 = (m_step)*m_fScale;
			m_area1 = (m_step+2)*m_fScale;
			UpdateColor(m_pTube,m_area0, m_area1);

			++m_currentCut;
			m_iClusterNum = 0;
			++m_iCurrentBagPosIndex;

			//如果该场景内3个绿色标记都成功操作则进入下一个场景，否则创建袋子并设置袋子位置
			if (m_CutCount == 3)
			{
				++m_currentStep;
				m_iClusterNum = 0;
				m_CutCount = 0;
				m_fCurrentTime = 0.0f;
			}
			else///正常三个剪切端 都在eST_WaitCheck
			{
				AddBagToWorld(m_iCurrentBagPosIndex);
				ChangeBagPosition();
			}
			m_gradLogicData.Reset();//next
		}
	}
	else if (st.stepType == eST_remove)
	{
		if (m_fCurrentTime >= m_fEnterNextLevelTimeThreshold)
		{
			//暂时避免因为抓钳钳住管子而导致野指针的问题,当抓钳抓住管子的时候只有松开抓钳到一定程度时才删除object
			ITool *pLeftTool = m_pToolsMgr->GetLeftTool();
			ITool *pRightTool = m_pToolsMgr->GetRightTool();
			bool canRemove = false;
			int GraspingForcepsCount = 0;
			if (pLeftTool && "GraspingForceps" == m_pToolsMgr->GetLeftTool()->GetType())
			{
				++GraspingForcepsCount;
				if(!dynamic_cast<CGraspingForceps*>(pLeftTool)->HasGraspSomeThing())
					canRemove = true;
			}
			if (pRightTool && "GraspingForceps" == m_pToolsMgr->GetRightTool()->GetType())
			{
				++GraspingForcepsCount;
				if(!dynamic_cast<CGraspingForceps*>(pRightTool)->HasGraspSomeThing())
					canRemove = true;
			}

			if (GraspingForcepsCount == 0)
			{
				canRemove = true;
			}
			if (canRemove)
			{
				RemoveBagFromWorld();
				m_currentStep++;
			}
			m_fCurrentTime = 0.0f;
		}
		else
		{
			m_fCurrentTime += dt;
		}
	}
	else if (st.stepType == eST_Finish)
	{
		if (m_bPerfectInLevel1)
		{
			CScoreMgr::Instance()->Grade("completely_1");
		}
		if (m_bPerfectInLevel2)
		{
			CScoreMgr::Instance()->Grade("completely_2");
		}
		TrainingFinish();
		return false;
	}
	return true;
}


GoPhys::GFPhysVector3 SYCuttingTubeTrain::GetTrainingForceFeedBack( ITool* tool )
{
	return GFPhysVector3(0, 0, 0);

	GoPhys::GFPhysVector3 result = GoPhys::GFPhysVector3(0,0,0);
	CTool* pTool = dynamic_cast<CTool*>(tool);

	GFPhysRigidBody * part[3];
	part[0] = pTool->m_lefttoolpartconvex.m_rigidbody;///-------------------
	part[1]  = pTool->m_righttoolpartconvex.m_rigidbody;
	part[2]  = pTool->m_centertoolpartconvex.m_rigidbody;

	for (int i = 0; i < 3; ++i)
	{
		if (part[i])
		{
			GFPhysVector3 aabbMin;
			GFPhysVector3 aabbMax;
			part[i]->GetAabb(aabbMin, aabbMax);
			float dirY = aabbMin.GetY();
			dirY = max(dirY,-1.0f);
			if (dirY < 0)
			{
				result = GFPhysVector3(0,-dirY,0);
			}
		}
	}


	return result * 1.5f;
}

void SYCuttingTubeTrain::ProcessAfterCut()
{
	if (IsGraspOnRightPlace(m_gradLogicData.m_ValidGraspTexRange))//只要抓正确一次给他分数目前
	{
		m_gradLogicData.SetGraspInRightPlace();
	}

	//判断是否剪对了部位 begin
	if (m_iClusterNum >= 2)
	{
		m_gradLogicData.SetCutOff();//提交剪断操作

		m_cuttedPosTime[m_currentCut] = GetElapsedTime();
		m_pTube->stopVesselBleedEffect();
		if (m_bCutTestPass && m_bShouldShowCutArea)
		{
			AfterCutTubeCompletely();
			///m_bCutTestPass = true;
			CTipMgr::Instance()->ShowTip("PleaseCutTube");
			CScoreMgr::Instance()->Grade("tube");
			m_cuttedPosCorrect[m_currentCut] = 1;
		}
		//判断是否剪对了部位 endf
		else///剪错了位置
		{
			m_gradLogicData.SetCutWrongPlace();//提交剪断位置错误

			m_cuttedPosCorrect[m_currentCut] = 2;
			if (m_currentStep == 1)
			{
				//出提示消息并直接跳到场景2
				m_bNeedEnterNextLevel = true;
				CTipMgr::Instance()->ShowTip("CutError1");
				m_bPerfectInLevel1 = false;
				m_iClusterNum = 0;
				m_currentCut = 3;
			}
			else if (m_currentStep == 4)
			{
				//出提示消息，并等待退出
				m_bPerfectInLevel2 = false;
				m_iClusterNum = 0;
				TrainingFatalError();
			}
		}
	}
}

void SYCuttingTubeTrain::InitStep()
{
	m_currentStep = 3;//use 0 to begin scene1 3 scene 2
	m_steps.clear();

	std::ifstream stream;
	stream.open("../Data/module/Basic/Cutting/TrainingTasks.txt");
	if(stream.is_open())
	{
		char buffer[100];
		stream.getline(buffer,99);
		std::string str = buffer;
		int len = str.find("TaskLineNUM:");
		std::string temp = str.substr(12);
		int num = atoi(temp.c_str());
		std::string sstr[3];
		int dotIndex[2];
		for(int i = 0; i< num; i++)
		{
			stream.getline(buffer,99);
			str = buffer;
			len = str.length();
			temp = str.substr(13, len - 13 - 2);
			dotIndex[0] = temp.find(",");
			dotIndex[1] = temp.find_last_of(",");
			sstr[0] = temp.substr(0, dotIndex[0]);
			sstr[1] = temp.substr(dotIndex[0] + 1, dotIndex[1] - dotIndex[0]-1);
			sstr[2] = temp.substr(dotIndex[1] + 1);

			if (sstr[0] == "eST_Add")
			{
				m_steps.push_back(trainingStep(eST_Add,atoi(sstr[1].c_str()),atoi(sstr[2].c_str())));
			}
			else if (sstr[0] == "eST_waitCheck")
			{
				if (sstr[2] == "ITool::TSD_RIGHT")
					m_steps.push_back(trainingStep(eST_waitCheck,atoi(sstr[1].c_str()),ITool::TSD_RIGHT));
				else
					m_steps.push_back(trainingStep(eST_waitCheck,atoi(sstr[1].c_str()),ITool::TSD_LEFT));
			}
			else if (sstr[0] == "eST_remove")
			{
				m_steps.push_back(trainingStep(eST_remove,atoi(sstr[1].c_str()),atoi(sstr[2].c_str())));
			}
			else
			{
				m_steps.push_back(trainingStep(eST_Finish,0,0));
			}
		}

	}
	else
	{
		m_steps.push_back(trainingStep(eST_Add,1,7));
		m_steps.push_back(trainingStep(eST_waitCheck,100,ITool::TSD_LEFT));
		m_steps.push_back(trainingStep(eST_remove,1,0));
		m_steps.push_back(trainingStep(eST_Add,1,12));
		m_steps.push_back(trainingStep(eST_waitCheck,100,ITool::TSD_RIGHT));
		m_steps.push_back(trainingStep(eST_remove,1,0));
	}
	m_steps.push_back(trainingStep(eST_Finish,0,0));
	stream.close();
}

void SYCuttingTubeTrain::UpdateColor( MisMedicOrgan_Ordinary * m, float area0, float area1 )
{
	if (m == NULL)
	{
		return;
	}
	m_area0 = area0;
	m_area1 = area1;
	//int nodeNum = m->m_OrganRendNodes.size();
	//for(int i = 0; i < nodeNum; ++i)
	//{
		//m->m_OrganRendNodes[i].m_TextureCoord.x += m_area0;
		//m->m_OrganRendNodes[i].m_TextureCoord.y += m_area0;
	//}
	m_TagOffsetPercent = m_area0;
}	

//选择cutfaces上的点与orignface上的点距离很小的点，并判断是否在UV范围内
bool SYCuttingTubeTrain::IsCutOnRightPlace()///--------------
{
	if (m_pTube->m_CutCrossfaces.size() > 0)
	{
		int totalFaceNum = 0;
		int inRightPlaceFaceNum = 0;
		const float distThreshold = 0.001f;

		for (int i = 0; i != m_pTube->m_CutCrossfaces.size(); ++i)
		{
			for (int j = 0; j != 3; ++j)
			{
				GFPhysVector3 pos;
				if (m_pTube->m_CutCrossfaces[i].m_physface)
				{
					if(m_pTube->m_CutCrossfaces[i].m_physface->m_Nodes[j])
					{
						pos = m_pTube->m_CutCrossfaces[i].m_physface->m_Nodes[j]->m_CurrPosition;
					}
				}

				MMO_Face closetFace;
				for (int k = 0; k != m_pTube->m_OriginFaces.size(); ++k)
				{
					GFPhysSoftBodyFace *pFace = m_pTube->m_OriginFaces[k].m_physface;
					if (pFace)
					{
						for (int p = 0; p != 3; ++p)
						{
							float dist = pFace->m_Nodes[p]->m_CurrPosition.Distance(pos);
							if(dist <= distThreshold)
							{
								closetFace = m_pTube->m_OriginFaces[k];
								++totalFaceNum;

								int matchPoint = 0;
								for (int k = 0; k != 3; ++k)
								{
									Ogre::Vector2 vertTexCoord = closetFace.GetTextureCoord(k);

									float range0 = 230.0f / 256.0f - m_TagOffsetPercent;
									float range1 = 256.0f / 256.0f - m_TagOffsetPercent;
								
									if (vertTexCoord.x > range0 && vertTexCoord.x < range1)
									{
										++matchPoint;
									}
								}
								if (matchPoint == 3)
								{
									++inRightPlaceFaceNum;
								}

							}
						}
					}
				}
			}
		}

		if (inRightPlaceFaceNum*1.0f/(totalFaceNum*1.0f) >= m_fCutRightRatio)
		{
			m_bCutTestPass = true;
			return true;
		}
	}
	return false;
}

//是否扔进袋子
bool SYCuttingTubeTrain::IsThrowInBag()
{
	CGraspingForceps *p = dynamic_cast<CGraspingForceps*>(m_pToolsMgr->GetRightTool());
	if (!p)
	{
		p = dynamic_cast<CGraspingForceps*>(m_pToolsMgr->GetLeftTool());
	}
	if (p)
	{
		m_bHasGraspTube = p->HasGraspSomeThing();
		
		if (!m_bHasGraspTube)
		{
			for (int i = 1; i < m_vecClusterResult.size(); ++i)
			{
				int incount = 0, fallcount = 0;

				//check whether fall bag
				if (m_pBag->IsClusetInBag(m_vecClusterResult[i]))
				{
					m_bPreInGround = m_bInGround = false;
					return true;
				}

				//if segment is not fall in bag check whether in ground
				std::set<GFPhysSoftBodyNode*>::iterator iter = m_vecClusterResult[i].begin();
				for (iter; iter != m_vecClusterResult[i].end(); ++iter)
				{
					if (m_TubeNodesContactTerrain.find((*iter)) != m_TubeNodesContactTerrain.end())
					{
						m_bInGround = true;
						return false;
					}
				}

				/*
				

				for (iter; iter != m_vecClusterResult[i].end(); ++iter)
				{
					if(PosInBagAABB((*iter)->m_CurrPosition))
					{
						++incount;
					}
					if (PosInGround((*iter)->m_CurrPosition))
					{
						++fallcount;
					}
				}
				float ratio = float(incount)/float(m_vecClusterResult[i].size());
				if (ratio >= 0.9)
				{
					m_bPreInGround = m_bInGround = false;
					//m_pBag->getSceneNode()->setVisible(false);
					return true;
				}
				if (fallcount)
				{
					m_bInGround = true;
					return false;
				}*/
			}
		}
	}
	m_bPreInGround = m_bInGround = false;
	return false;
}

int SYCuttingTubeTrain::BFSLinkedAera()
{
	m_vecClusterResult.clear();
	m_vecClusterTetrahedron.clear();
	std::vector<GFPhysSoftBodyTetrahedron *> SelectedTetras;
	SelectedTetras.reserve(10000);

	//遍历血管的所有四面体 并将其标记为0,0表示为没有被访问过
	//GFPhysSoftBodyTetrahedron * tetra = m_pTube->m_physbody->GetTetrahedronList();
	//while(tetra)
	for(size_t th = 0 ; th < m_pTube->m_physbody->GetNumTetrahedron() ; th++)
	{
		GFPhysSoftBodyTetrahedron * tetra = m_pTube->m_physbody->GetTetrahedronAtIndex(th);

		tetra->m_TempData = (void*)0;
		SelectedTetras.push_back(tetra);
		//tetra = tetra->m_Next;
	}

	int totalNodeNum = 0;
	int maxRegin = 0;
	int	nodeNum;
	int nodeNum_Mark;
	//

	std::set<GFPhysSoftBodyFace*> ClusterFaceSet;
	std::set<GFPhysSoftBodyNode*> ClusterNodesSet;
	std::deque<GFPhysSoftBodyTetrahedron*> QueueTetras;

	int GlobalClusterID = 1;


	//3号node为固定点，找到与包含该点的四面体并将他放在vector第一的位置,并默认与该四面体相联通的四面体的分类标记为1
	for (size_t t = 0 ; t < SelectedTetras.size(); t++)
	{
		bool flag = false;
		GFPhysSoftBodyTetrahedron * tetra = SelectedTetras[t];
		for (int i = 0; i != 4; ++i)
		{
			if (tetra->m_TetraNodes[i] == m_pFixPoint)
			{
				GFPhysSoftBodyTetrahedron* temp = SelectedTetras[0];
				SelectedTetras[0] = SelectedTetras[t];
				SelectedTetras[t] = temp;
				flag = true;
				break;
			}
		}
		if (flag)
		{
			break;
		}
	}

	for(size_t t = 0 ; t < SelectedTetras.size(); t++)
	{
		GFPhysSoftBodyTetrahedron * tetra = SelectedTetras[t];

		if(tetra->m_TempData == 0)//this tetra not belong any cluster
		{
			GFPhysVectorObj<GFPhysSoftBodyTetrahedron*> teras;
			QueueTetras.clear();
			ClusterNodesSet.clear();
			nodeNum = 0;
			nodeNum_Mark = 0;
			tetra->m_TempData = (void*)GlobalClusterID;
			QueueTetras.push_back(tetra);

			while(QueueTetras.size() > 0)
			{	
				GFPhysSoftBodyTetrahedron * QTetra = QueueTetras.front();
				QueueTetras.pop_front();

				ClusterNodesSet.insert(QTetra->m_TetraNodes[0]);
				ClusterNodesSet.insert(QTetra->m_TetraNodes[1]);
				ClusterNodesSet.insert(QTetra->m_TetraNodes[2]);
				ClusterNodesSet.insert(QTetra->m_TetraNodes[3]);

				teras.push_back(QTetra);

				//add 4 neighbor tetrahedron in cluster if not added yet
				for(int nb = 0 ; nb < 4 ; nb++)
				{
					GFPhysGeneralizedFace * genFace = QTetra->m_TetraFaces[nb];

					if(genFace && genFace->m_ShareTetrahedrons.size() > 1)
					{
						GFPhysSoftBodyTetrahedron * t0 = genFace->m_ShareTetrahedrons[0].m_Hosttetra;///--------------

						GFPhysSoftBodyTetrahedron * t1 = genFace->m_ShareTetrahedrons[1].m_Hosttetra;

						GFPhysSoftBodyTetrahedron * NBTetra = (t0 == QTetra ? t1 : t0);

						if(NBTetra->m_TempData == 0)
						{
							NBTetra->m_TempData = (void*)GlobalClusterID;
							QueueTetras.push_back(NBTetra);
						}
					}
				}
			}

			m_vecClusterTetrahedron.push_back(teras);
			m_vecClusterResult.push_back(ClusterNodesSet);
			GlobalClusterID++;//advance
		}
	}
	m_iClusterNum = m_vecClusterResult.size();
	return m_vecClusterResult.size();
}

void SYCuttingTubeTrain::ChangeBagPosition()
{
	/*if (m_iCurrentBagID >= 16)
	{
		return;
	}
	//m_pBagForScale->setScale(1,1,1);
	Ogre::Vector3 orignPos = m_pBag->getSceneNode()->getPosition();
	float posX = float(rand()%2000)/1000.0f-1;
	float posZ = float(rand()%2000)/1000.0f-1;
	if (posX<0)
	{
		posX-=1;
	}
	else
		posX+=1;

	if (posZ<0)
	{
		posZ-=1;
	}
	else
		posZ+=1;
	//m_pBagForScale->setPosition(m_ConstructInfoMap[m_iCurrentBagID].m_Position);*/
}

//隐藏被剪下来的部分血管
void SYCuttingTubeTrain::HideCutedPartOrgan()
{
	for (int i = 1; i < m_vecClusterTetrahedron.size(); ++i)
	{
		m_pTube->EliminateTetras(m_vecClusterTetrahedron[i]);
	}
}

bool SYCuttingTubeTrain::PosInBagAABB(const GFPhysVector3& pos)
{
	/*Ogre::Vector3 position;
	position.x = pos.x();
	position.y = pos.y();
	position.z = pos.z();

	Ogre::AxisAlignedBox box = m_pBag->getSceneNode()->_getWorldAABB();

	return box.contains(position);*/
	return false;
}

bool SYCuttingTubeTrain::PosInGround(const GFPhysVector3& pos)
{
	if (pos.y() <= 3.5f)
	{
		return true;
	}
	return false;
}

bool SYCuttingTubeTrain::TubeHasCutCompeletly()
{
	if (m_vecClusterResult.size() > 1)
	{
		return true;
	}
	return false;
}


//判断血管是否被扯断
bool SYCuttingTubeTrain::IsTearOffTheTube()
{
	GFPhysVector3 pos1 = m_pFixPoint->m_CurrPosition;
	GFPhysVector3 pos2 = m_pEndPoint->m_CurrPosition;
	float distance = pos1.Distance(pos2);
	if (distance > m_fTearOffThreshold)
	{
		return true;
	}
	return false;
}

//扯断血管
void SYCuttingTubeTrain::TearOffTheTube(bool graspOnRightPlace)
{
	GFPhysVector3 v1 = (m_vecTearOffPoint[0]->m_UnDeformedPos - m_vecTearOffPoint[1]->m_UnDeformedPos).Normalized();///-------------
	GFPhysVector3 v2 = (m_vecTearOffPoint[1]->m_UnDeformedPos - m_vecTearOffPoint[0]->m_UnDeformedPos).Normalized();
	GFPhysVector3 v3 = (m_vecTearOffPoint[0]->m_UnDeformedPos - m_vecTearOffPoint[2]->m_UnDeformedPos).Normalized();
	GFPhysVector3 v4 = (m_vecTearOffPoint[2]->m_UnDeformedPos - m_vecTearOffPoint[0]->m_UnDeformedPos).Normalized();

	GFPhysVector3 cutQuads[4];
	cutQuads[0] = m_vecTearOffPoint[0]->m_UnDeformedPos+v1*20;
	cutQuads[1] = m_vecTearOffPoint[0]->m_UnDeformedPos+v4*20;
	cutQuads[2] = m_vecTearOffPoint[0]->m_UnDeformedPos+v3*20;
	cutQuads[3] = m_vecTearOffPoint[0]->m_UnDeformedPos+v2*20;

	m_pTube->TearOrganBySemiInfinteQuad(cutQuads , false);
}

void SYCuttingTubeTrain::BFSTearOffPointTetrahedron(GFPhysSoftBodyTetrahedron* tetra, int level, GFPhysVectorObj<GFPhysSoftBodyTetrahedron*>& result)
{
	if (tetra == NULL || level <= 0)
	{
		return;
	}
	std::stack<std::pair<GFPhysSoftBodyTetrahedron*, int>> QueueTetras;
	QueueTetras.push(std::pair<GFPhysSoftBodyTetrahedron*, int>(tetra, 0));

	//遍历血管的所有四面体 并将其标记为0,0表示为没有被访问过
	//GFPhysSoftBodyTetrahedron * headtetra = m_pTube->m_physbody->GetTetrahedronList();
	//while(headtetra)
	for(size_t th = 0 ; th < m_pTube->m_physbody->GetNumTetrahedron() ; th++)
	{
		GFPhysSoftBodyTetrahedron * headtetra = m_pTube->m_physbody->GetTetrahedronAtIndex(th);

		headtetra->m_TempData = (void*)0;
		//headtetra = headtetra->m_Next;
	}

	while(QueueTetras.size() > 0)
	{	
		GFPhysSoftBodyTetrahedron * QTetra = QueueTetras.top().first;
		int nlevel = QueueTetras.top().second;
		QueueTetras.pop();
		result.push_back(QTetra);
		if (nlevel >= level)
		{
			return;
		}
		for(int nb = 0 ; nb < 4 ; nb++)
		{
			GFPhysGeneralizedFace * genFace = QTetra->m_TetraFaces[nb];

			if(genFace && genFace->m_ShareTetrahedrons.size() > 1)
			{
				GFPhysSoftBodyTetrahedron * t0 = genFace->m_ShareTetrahedrons[0].m_Hosttetra;///------------

				GFPhysSoftBodyTetrahedron * t1 = genFace->m_ShareTetrahedrons[1].m_Hosttetra;

				GFPhysSoftBodyTetrahedron * NBTetra = (t0 == QTetra ? t1 : t0);

				if (NBTetra->m_TempData == (void*)0)
				{
					NBTetra->m_TempData = (void*)1;
					QueueTetras.push(std::pair<GFPhysSoftBodyTetrahedron*, int>(NBTetra, nlevel+1));
				}
			}
		}
	}
}

void SYCuttingTubeTrain::LoadLevel1TearOffPoint()
{
	m_vecTearOffPoint.clear();

	for (int i = 0; i != m_vecTube1TearOffPtID.size(); ++i)
	{
		m_vecTearOffPoint.push_back(m_pTube->m_physbody->GetNode(m_vecTube1TearOffPtID[i]));
	}

}

void SYCuttingTubeTrain::LoadLevel2TearOffPoint()
{
	m_vecTearOffPoint.clear();

	for (int i = 0; i != m_vecTube2TearOffPtID.size(); ++i)
	{
		m_vecTearOffPoint.push_back(m_pTube->m_physbody->GetNode(m_vecTube2TearOffPtID[i]));
	}


}

bool SYCuttingTubeTrain::IsGraspOnRightPlace(Ogre::Vector2 validTexURange)
{
	CGraspingForceps * pGraspForceps = dynamic_cast<CGraspingForceps*>(m_pToolsMgr->GetRightTool());
	if (!pGraspForceps)
	{
		pGraspForceps = dynamic_cast<CGraspingForceps*>(m_pToolsMgr->GetLeftTool());
	}
	if (pGraspForceps)
	{
		GFPhysVectorObj<GFPhysSoftBodyFace*> graspedFace;
		
		MisCTool_PluginClamp * pluginClamp = pGraspForceps->GetClampPlugin();
			
		pluginClamp->GetFacesBeClamped(graspedFace, m_pTube);
		
		int numFaceClamped = 0;
		
		for (int i = 0; i != graspedFace.size(); ++i)
		{
			MMO_Face & face = m_pTube->GetMMOFace(graspedFace[i]);
			
			Ogre::Vector2 tex0 = face.GetTextureCoord(0);
			
			Ogre::Vector2 tex1 = face.GetTextureCoord(1);
			
			Ogre::Vector2 tex2 = face.GetTextureCoord(2);

			if (tex0.x > validTexURange.y && tex1.x > validTexURange.y && tex2.x > validTexURange.y)
			{
				continue;
			}

			if (tex0.x < validTexURange.x && tex1.x < validTexURange.x && tex2.x < validTexURange.x)
			{
				continue;
			}


			numFaceClamped++;
		}

		if (numFaceClamped > graspedFace.size() * 0.25f)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	return false;
}

//电断血管后更新扯断阈值
void SYCuttingTubeTrain::AfterCutTubeCompletely()
{
	if (m_iClusterNum >= 2)
	{
		for (std::set<GFPhysSoftBodyNode*>::iterator iter = m_vecClusterResult[0].begin(); iter != m_vecClusterResult[0].end(); ++iter)
		{
			for (int i = 0; i != m_pTube->m_CutCrossfaces.size(); ++i)
			{
				GFPhysSoftBodyFace* pFace = m_pTube->m_CutCrossfaces[i].m_physface;
				for (int j = 0; j != 3; ++j)
				{
					if(pFace && pFace->m_Nodes[j] == *iter)
					{
						m_pEndPoint = *iter;
						GFPhysVector3 pos1 = m_pFixPoint->m_CurrPosition;
						GFPhysVector3 pos2 = m_pEndPoint->m_CurrPosition;
						float distance = pos1.Distance(pos2);
						m_fTearOffThreshold = distance*1.6;
						return;
					}
				}
			}
		}
	}
}

void SYCuttingTubeTrain::AddObjectToWorld(int id)
{
	MisMedicDynObjConstructInfo& cs = m_ConstructInfoMap[id];
	if(cs.m_OrganType == 0 || cs.m_OrganType == 1)
	   cs.m_distributemass = false;
	else
	   cs.m_distributemass = false;
	MisMedicOrgan_Ordinary * organobject = new MisMedicOrgan_Ordinary(cs.m_OrganType ,cs.m_OrganId, this);
	organobject->Create(cs);
	organobject->BuildTetrahedronNodeTextureCoord(GFPhysVector3(0, 0, 0), false);

	m_DynObjMap.insert(make_pair(organobject->m_OrganID , organobject));
	m_Objects[cs.m_OrganType] = organobject;

}

void SYCuttingTubeTrain::RemoveObjectFromWorld(int id)
{
	MisMedicOrgan_Ordinary * m = m_Objects[id];
	if (m == NULL)
	{
		return;
	}
	m_Objects[id] = NULL;
	RemoveOrganFromWorld(m);
}


void SYCuttingTubeTrain::EnterLevel1()
{
	m_TagOffsetPercent = 0;
	m_LastTexChangeFlag = -1;
	AddObjectToWorld(0);
	AddObjectToWorld(3);
	m_pTube = m_Objects[0];
	m_pTerrain = m_Objects[3];

	m_pTube->setVesselBleedEffectTempalteName(PT_BLEED_02);

	//将血管与凸起相交的面的软体碰撞开关关闭,防止连接部位闪烁
	for(size_t f = 0 ; f < m_pTube->m_physbody->GetNumFace() ; f++)
	{
		GFPhysSoftBodyFace * face = m_pTube->m_physbody->GetFaceAtIndex(f);

		bool flag = false;
		
		for (int i = 0; i != 3; ++i)
		{
			if(face->m_Nodes[i]->m_UnDeformedPos.x() <= -2.1)
			{
				flag = true;
				break;
			}
		}
		if (flag)
		{
			face->DisableCollideWithSoft();
			face->m_Nodes[0]->m_CollideFlag &= (~GPSECD_SOFT);
			face->m_Nodes[1]->m_CollideFlag &= (~GPSECD_SOFT);
			face->m_Nodes[2]->m_CollideFlag &= (~GPSECD_SOFT);
		}
	}


	//3号node为固定点，找到与包含该点的四面体并将他放在vector第一的位置,并默认与该四面体相联通的四面体的分类标记为1
	m_pFixPoint = m_pTube->m_physbody->GetNode(210);
	//46号node为自由端端点，用于判定是否扯断
	m_pEndPoint = m_pTube->m_physbody->GetNode(39);

	MisMedicObjectAdhersion * adhersion = new MisMedicObjectAdhersion();
	adhersion->BuildUniversalLinkFromAToB(*m_pTube , *m_pTerrain , 1.0f);
	m_ObjAdhersions.push_back(adhersion);

	AddBagToWorld(m_iCurrentBagPosIndex);

	LoadLevel1TearOffPoint();
}

void SYCuttingTubeTrain::LeaveLevel1()
{
	RemoveBagFromWorld();
	for (int i = 0; i != m_ObjAdhersions.size(); ++i)
	{
		if (m_ObjAdhersions[i])
		{
			delete m_ObjAdhersions[i];
			m_ObjAdhersions[i] = NULL;
		}
	}
	m_ObjAdhersions.clear();

	RemoveObjectFromWorld(0);
	RemoveObjectFromWorld(3);
}

void SYCuttingTubeTrain::EnterLevel2()
{
	m_TagOffsetPercent = 0;
	m_LastTexChangeFlag = -1;
	m_step = 15;
	m_fScale = 1.0f/21.0f;
	m_fTearOffThreshold = 8.0f;
	AddObjectToWorld(1);
	AddObjectToWorld(2);
	m_pTube = m_Objects[1];
	m_pTerrain = m_Objects[2];

	m_pTube->setVesselBleedEffectTempalteName(PT_BLEED_02);
	
	//将血管与凸起相交的面的软体碰撞开关关闭,防止连接部位闪烁
	for(size_t f = 0 ; f < m_pTube->m_physbody->GetNumFace() ; f++)
	{
		GFPhysSoftBodyFace * face = m_pTube->m_physbody->GetFaceAtIndex(f);

		bool flag = false;
		for (int i = 0; i != 3; ++i)
		{
			if(face->m_Nodes[i]->m_UnDeformedPos.x() >= 1.9)
			{
				flag = true;
				break;
			}
		}
		if (flag)
		{
			face->DisableCollideWithSoft();
			face->m_Nodes[0]->m_CollideFlag &= (~GPSECD_SOFT);
			face->m_Nodes[1]->m_CollideFlag &= (~GPSECD_SOFT);
			face->m_Nodes[2]->m_CollideFlag &= (~GPSECD_SOFT);
		}
	}

	//3号node为固定点，找到与包含该点的四面体并将他放在vector第一的位置,并默认与该四面体相联通的四面体的分类标记为1
	m_pFixPoint = m_pTube->m_physbody->GetNode(210);
	//46号node为自由端端点，用于判定是否扯断
	m_pEndPoint = m_pTube->m_physbody->GetNode(39);

	MisMedicObjectAdhersion * adhersion = new MisMedicObjectAdhersion();
	adhersion->BuildUniversalLinkFromAToB(*m_pTube , *m_pTerrain , 1.0f);
	m_ObjAdhersions.push_back(adhersion);

	AddBagToWorld(m_iCurrentBagPosIndex);

	//m_pBagForScale->setPosition(m_ConstructInfoMap[m_iCurrentBagID].m_Position);

	LoadLevel2TearOffPoint();
}

void SYCuttingTubeTrain::LeaveLevel2()
{
	for (int i = 0; i != m_ObjAdhersions.size(); ++i)
	{
		if (m_ObjAdhersions[i])
		{
			delete m_ObjAdhersions[i];
			m_ObjAdhersions[i] = NULL;
		}
	}
	m_ObjAdhersions.clear();

	RemoveObjectFromWorld(1);
	RemoveObjectFromWorld(2);
}

//是否抓住血管
bool SYCuttingTubeTrain::PlayerHasGraspTube()
{
	bool result = false;
	CTool * tool = dynamic_cast<CTool*>(m_pToolsMgr->GetRightTool());
	if (tool)
	{
		result = tool->HasGraspSomeThing();
		if (result)
		{
			return result;
		}
	}

	tool = dynamic_cast<CTool*>(m_pToolsMgr->GetLeftTool());
	if (tool)
	{
		result = tool->HasGraspSomeThing();
		if (result)
		{
			return result;
		}
	}
	return result;
}

//缩放袋子
void SYCuttingTubeTrain::ScaleTheBag()
{
	if (m_fBagScaleX > 0.5f + m_fXScalePerFrame && m_fBagScaleZ >= 0.5f + m_fZScalePerFrame)
	{
		m_fBagScaleX -= m_fXScalePerFrame*0.1f;
		m_fBagScaleZ -= m_fZScalePerFrame*0.1f;
		m_pBag->ScaleBag(m_fBagScaleX, m_fBagScaleY, m_fBagScaleZ);
	}
	else
	{
		m_pBag->ScaleBag(1, 1, 1);

		m_fBagScaleX = 1;
		m_fBagScaleY = 1;
		m_fBagScaleZ = 1;
		m_bScaling = false;
	}
	
}

//修改贴图，flag=0 显示绿色标记区域  flag=1 隐藏绿色标记区域
void SYCuttingTubeTrain::ChangeTexture(int flag)
{
	if(m_LastTexChangeFlag != -1 
	&& m_LastTexChangeFlag == flag
	&& fabsf(m_LastTexChangeOffset - m_TagOffsetPercent) < 0.001f)//optimize no need unused update
	   return;

	
	Ogre::HardwarePixelBufferSharedPtr pixelBuffer = m_TubeTexturPtr->getBuffer();
	pixelBuffer->lock(Ogre::HardwareBuffer::HBL_NORMAL);
	const Ogre::PixelBox& pixelBox = pixelBuffer->getCurrentLock();
	int width = pixelBox.getWidth();
	int height = pixelBox.getHeight();
	int depth = pixelBox.getDepth();
	unsigned char* imgData = (unsigned char*)pixelBox.data;
	int rowOffset = Ogre::PixelUtil::getMemorySize(width, 1, depth, pixelBox.format);
	int chanle = rowOffset/width/depth;

	int offsetPixel = m_TagOffsetPercent * width;

	float alpha = 0.7f;
	if (0 == flag)
	{
		for (int i = 0; i != height; ++i)
		{
			unsigned char *pRowData = imgData+i*rowOffset;

			for (int j = 0; j < width; ++j)///----------------
			{

				pRowData[chanle*j]   = 0;				//A
				pRowData[chanle*j+1] = 0;				//B
				pRowData[chanle*j+2] = 0;				//G
				pRowData[chanle*j+3] = 0;           //R
			}

			for (int j = 230 - offsetPixel; j < width - offsetPixel; ++j)
			{
				pRowData[chanle*j]   = 0;				//B
				pRowData[chanle*j+1] = 64;// 255 * alpha + pRowData[chanle*j + 1] * (1 - alpha);//G
				pRowData[chanle*j+2] = 0;//R
				pRowData[chanle*j+3] = alpha * 255;				
			}
		}
	}
	else
	{
		for (int i = 0; i != height; ++i)
		{
			unsigned char *pRowData = imgData+i*rowOffset;
			for (int j = 0; j < width; ++j)
			{
				
				pRowData[chanle*j]   = 0;//pRowData[chanle*j]*(1-alpha)+139*alpha;				//B
				pRowData[chanle*j+1] = 0;//pRowData[chanle*j+1]*(1-alpha)+139*alpha;				//G
				pRowData[chanle*j+2] = 0;//pRowData[chanle*j+2]*(1-alpha)+139*alpha;				//R
				pRowData[chanle*j+3] = 0;
			}

			for (int j = 203 - offsetPixel; j < 230 - offsetPixel; ++j)
			{
				pRowData[chanle*j] = 0;				//B
				pRowData[chanle*j+1] = 46;				//G
				pRowData[chanle*j+2] = 116;				//R
				pRowData[chanle*j+3] = 255;             //A
			}
		}
		m_gradLogicData.m_ValidGraspTexRange.x = float(203 - offsetPixel) / (float)width;
		m_gradLogicData.m_ValidGraspTexRange.y = float(230 - offsetPixel) / (float)width;
	}
	pixelBuffer->unlock();

	m_LastTexChangeFlag = flag;
	m_LastTexChangeOffset = m_TagOffsetPercent;
}

//是否抓起血管到一定长度
bool SYCuttingTubeTrain::IsClampInTheSky()
{
	GFPhysVector3 pos1 = m_pFixPoint->m_CurrPosition;
	GFPhysVector3 pos2 = m_pEndPoint->m_CurrPosition;
	float distance = pos1.Distance(pos2);
	if (distance > m_fTearOffThreshold*0.5)
	{
		return true;
	}
	return false;
}

//添加动态袋子
void SYCuttingTubeTrain::AddBagToWorld(int posIndex)
{
	if (posIndex >= m_vecBagPosition.size())
	{
		return;
	}
	/*
	MisMedicDynObjConstructInfo& cs = m_ConstructInfoMap[m_iCurrentBagID];
	cs.m_Position = Ogre::Vector3(0.078f, 1.96f, -2.334f);
	//m_vecBagPosition[posIndex];
	MisMedicOrgan_Ordinary * organobject = new MisMedicOrgan_Ordinary(cs.m_OrganType ,cs.m_OrganId, this);
	organobject->Create(cs);
	organobject->m_physbody->EnableDoubleFaceSoftSoftCollision();//标本带打开双面

	m_DynObjMap.insert(make_pair(organobject->m_OrganID , organobject));
	m_Objects[cs.m_OrganType] = organobject;
	m_pBag = organobject;

	if (m_CupCol)
	    m_CupCol->m_body->GetWorldTransform().SetOrigin(OgreToGPVec3(cs.m_Position));

	if (m_pBagCircle)
		m_pBagCircle->setPosition(cs.m_Position);
		*/
	if (m_pBag != 0)
	{
		m_pBag->ResetPosition(Ogre::Vector3(-1.078f, 1.96f, -2.334f));
	}
	else
	{
		for (int c = 0; c < m_reservedConstructInfos.size(); c++)
		{
			if (m_reservedConstructInfos[c].m_OrganType == 10)
			{
				m_pBag = new SceneSpeciBag();
				m_pBag->Create(m_reservedConstructInfos[c], Ogre::Vector3(0.078f, 1.96f, -2.334f), this, MXOgre_SCENEMANAGER, "TorusJinshu.mesh", "col_cup.mms", "jinshu");//m_Objects[m_iCurrentBagPosIndex];
				m_pTerrain->m_physbody->SetCollisionMask(m_pTerrain->m_physbody->m_MaskBits & (~m_pBag->m_ColCupCollisionCat));
				break;
			}
		}
	}
}

//删除动态袋子
void SYCuttingTubeTrain::RemoveBagFromWorld()
{
	/*MisMedicOrgan_Ordinary * m = m_Objects[m_iCurrentBagID];
	if (m == NULL)
	{
		return;
	}
	
	m_Objects[m_iCurrentBagID] = NULL;
	RemoveOrganFromWorld(m);*/

	
}

//删除所有袋子
void SYCuttingTubeTrain::RemoveAllBagFromWorld()
{
	for (int id = 10; id != 16; ++id)
	{
		MisMedicOrgan_Ordinary * m = m_Objects[id];
		if (m == NULL)
		{
			continue;
		}

		m_Objects[id] = NULL;
		RemoveOrganFromWorld(m);
	}
	m_pBag = NULL;
}


//读取训练相关参数
bool SYCuttingTubeTrain::ReadTrainParam(const std::string& strFileName)
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
			if (key == "CutRightRatio")					///读取剪断面在绿色标记区域内的比值,用于判定是否在绿色标记位置电切
			{
				int valEnd = str.find(')');
				int subValEnd = 0;
				std::string val = str.substr(keyEnd+1,valEnd-(keyEnd+1));
				while (-1 != subValEnd)
				{
					subValEnd = val.find(',');
					std::string valStr = val.substr(0,subValEnd);
					m_fCutRightRatio = atof(valStr.c_str());
					val = val.substr(subValEnd+1);
				}
			}
			else if (key == "TearOffPointIndex1")					//读取场景1中血管扯断点的ID
			{
				int valEnd = str.find(')');
				int subValEnd = 0;
				std::string val = str.substr(keyEnd+1,valEnd-(keyEnd+1));
				while (-1 != subValEnd)
				{
					subValEnd = val.find(',');
					std::string valStr = val.substr(0,subValEnd);
					m_vecTube1TearOffPtID.push_back(atoi(valStr.c_str()));
					val = val.substr(subValEnd+1);
				}
			}
			else if (key == "TearOffPointIndex2")					//读取场景2中血管扯断点的ID
			{
				int valEnd = str.find(')');
				int subValEnd = 0;
				std::string val = str.substr(keyEnd+1,valEnd-(keyEnd+1));
				while (-1 != subValEnd)
				{
					subValEnd = val.find(',');
					std::string valStr = val.substr(0,subValEnd);
					m_vecTube2TearOffPtID.push_back(atoi(valStr.c_str()));
					val = val.substr(subValEnd+1);
				}
			}
			else if (key == "BagPosition")							//袋子出现的位置
			{
				int valEnd = str.find(')');
				int subValEnd = 0;
				std::string val = str.substr(keyEnd+1,valEnd-(keyEnd+1));
				//循环读取以逗号分隔的值
				while (-1 != subValEnd)
				{
					Real pos[3] = {0,0,0}; //x,y,z
					for (int i = 0; i != 3; ++i)
					{
						subValEnd = val.find(',');
						std::string valStr = val.substr(0,subValEnd);
						pos[i] = atof(valStr.c_str());
						val = val.substr(subValEnd+1);
					}
					m_vecBagPosition.push_back(Ogre::Vector3(pos));
				}
			}
			else if (key == "EnterNextLevelTimeThreshold")			//进入下一场景的时间阈值
			{
				int valEnd = str.find(')');
				int subValEnd = 0;
				std::string val = str.substr(keyEnd+1,valEnd-(keyEnd+1));
				while (-1 != subValEnd)
				{
					subValEnd = val.find(',');
					std::string valStr = val.substr(0,subValEnd);
					m_fEnterNextLevelTimeThreshold = atof(valStr.c_str());
					val = val.substr(subValEnd+1);
				}
			}
			else if (key == "XScalePerFrame")						//袋子沿X轴方向每帧缩放的量
			{
				int valEnd = str.find(')');
				int subValEnd = 0;
				std::string val = str.substr(keyEnd+1,valEnd-(keyEnd+1));
				while (-1 != subValEnd)
				{
					subValEnd = val.find(',');
					std::string valStr = val.substr(0,subValEnd);
					m_fXScalePerFrame = atof(valStr.c_str());
					val = val.substr(subValEnd+1);
				}
			}
			else if (key == "ZScalePerFrame")						//袋子沿Z轴方向每帧缩放的量
			{
				int valEnd = str.find(')');
				int subValEnd = 0;
				std::string val = str.substr(keyEnd+1,valEnd-(keyEnd+1));
				while (-1 != subValEnd)
				{
					subValEnd = val.find(',');
					std::string valStr = val.substr(0,subValEnd);
					m_fZScalePerFrame = atof(valStr.c_str());
					val = val.substr(subValEnd+1);
				}
			}
		}
		return true;
	}
	return false;
}

void SYCuttingTubeTrain::OnSaveTrainingReport()
{
	//score item
	float leftToolMovDist  = m_pToolsMgr->GetLeftToolMovedDistance();
	float rightToolMovDist = m_pToolsMgr->GetRightToolMovedDistance();

	float leftToolSpeed = m_pToolsMgr->GetLeftToolMovedSpeed();
	float rightToolSpeed = m_pToolsMgr->GetRightToolMovedSpeed();

	if (leftToolMovDist > 10 || rightToolMovDist > 10)
	{
		if (leftToolSpeed > 10 || rightToolSpeed > 10)
		{
			AddScoreItemDetail("0151100802", 0);//移动速度过快，有安全隐患
		}
		else if (leftToolSpeed < 5 && rightToolSpeed < 5)
		{
			AddScoreItemDetail("0151100800", 0);//移动平稳流畅
		}
		else
		{
			AddScoreItemDetail("0151100801", 0);//移动速度较快
		}
	}
	//

	if (m_gradLogicData.m_NumSegmentThrowSucced == 3)
	{
		int TimeUsed = GetElapsedTime();
		if (TimeUsed < 60)
			AddScoreItemDetail("0151200500", 0);
		else if (TimeUsed < 90)
			AddScoreItemDetail("0151200501", 0);
		else
			AddScoreItemDetail("0151200502", 0);
	}

	if (GetGradLogicData().m_MaxContinueElecTime > 0)
	{
		if (GetGradLogicData().m_MaxContinueElecTime < 3.0f)
			AddScoreItemDetail("0150901610", 0);//持续通电时间适中
		else
			AddScoreItemDetail("0150901611", 0);//
	}

	if (m_gradLogicData.HasBeTeared())
		AddScoreItemDetail("0150803508", 0);
	else
		AddScoreItemDetail("0150803500", 0);

	
	int numFallCount = 0;
	for (int c = 0; c < 6; c++)
	{
		numFallCount += m_fallCount[c];
	}

	if (m_gradLogicData.m_NumSegmentThrowSucced > 0)
	{
		if (numFallCount > 0)
			AddScoreItemDetail("0150701401", 0);//入袋前有掉落
		else
			AddScoreItemDetail("0150701400", 0);
	}

	if (m_gradLogicData.m_NumSegmentThrowSucced == 3)
	{
		if (GetGradLogicData().m_MaxContinueElecTime < 3.0f)
		{
			AddScoreItemDetail("0151000300", 0); 
		}
		else
		{
			AddScoreItemDetail("0151000301", 0);
		}
	}
	//if (m_CutNum == 0)//
	MisNewTraining::OnSaveTrainingReport();
}

//void SYCuttingTubeTrain::OnHandleEvent(MxEvent* pEvent)
//{
	
//}