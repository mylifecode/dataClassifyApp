#include "LigatingLoopTrain.h"
#include "MXEventsDump.h"
#include "Inception.h"
#include "MisMedicRigidPrimtive.h"
#include <fstream>

#include "Instruments/MisCTool_PluginClamp.h"
#include <MMSystem.h>
#include "MisMedicOrganAttachment.h"

#include "MisMedicObjectUnion.h"
#include "MisMedicBindedRope.h"
#include "MisMedicThreadRope.h"
#include "Instruments/GraspingForceps.h"
#include "EngineCore.h"
#include "Instruments/Knotter.h"

//=============================================================================================

void NewTrainingHandleEvent_LigatingLoop(MxEvent * pEvent, ITraining * pTraining)
{
	if (!pEvent || !pTraining)
		return;
	if (pEvent->m_enmEventType == MxEvent::MXET_CutThread)
	{
		CLigatingLoopTrain* pTrain = dynamic_cast<CLigatingLoopTrain*>(pTraining);
		pTrain->JudgeRopeCutted();
	}
    else if (pEvent->m_enmEventType == MxEvent::MXET_OrganBinded)
    {
        CLigatingLoopTrain* pTrain = dynamic_cast<CLigatingLoopTrain*>(pTraining);
        pTrain->m_nsumBindCount++;
    }
}



CLigatingLoopTrain::CLigatingLoopTrain(void)
: m_bFinished(false)
, m_area0(0)
, m_area1(0)
, m_area2(0)
, m_area3(0)
, m_nCurrentTubeIndex(0)
, m_fTearOffThreshold(10.0f)
, m_fShowAreaThreshold(7.0f)
, m_counter(0)
, m_bShowLigatingErrorTip(false)
, m_bPerfect(true)
, m_LastTexChangeFlag(-1)
, m_LastChangeTubeIndex(-1)
, m_nsumBindCount(0)
{
}
//=============================================================================================
CLigatingLoopTrain::~CLigatingLoopTrain(void)
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

	for (int i = 0; i != 3; ++i)
	{
		m_TubeTexturPtr[i].setNull();
	}

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
}
//======================================================================================================================
bool CLigatingLoopTrain::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{

	bool result = MisNewTraining::Initialize(pTrainingConfig , pToolConfig);

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

			if (cs.m_OrganType < 3)
			{
				organmesh->setVesselBleedEffectTempalteName(PT_BLEED_BIGVESSEL);
				m_vecTube.push_back(TUBEINFO(organmesh));
			}
		}
		++itor;
	}

	//��ȡ��ز���
	ReadTrainParam("../Config/Train/Basic/LigatingLoop/TrainParam.txt");

	m_Objects[3]->m_CanBindThread = false;
	//��������Ѫ���ཻ�Ĳ���
	for (int i = 0; i != m_vecTube.size(); ++i)
	{
		MisMedicObjectAdhersion * adhersion = new MisMedicObjectAdhersion();
		adhersion->BuildUniversalLinkFromAToB(*m_vecTube[i].m_pTube , *m_Objects[3] , 0.7f);
		m_ObjAdhersions.push_back(adhersion);

		//��Ѫ����͹���ཻ�����������ײ���عر�,��ֹ���Ӳ�λ��˸
		//GFPhysSoftBodyFace* pFaces = m_vecTube[i].m_pTube->m_physbody->GetFaceList();
		//while(pFaces)
		for(size_t f = 0 ; f < m_vecTube[i].m_pTube->m_physbody->GetNumFace() ; f++)
		{
			GFPhysSoftBodyFace * face = m_vecTube[i].m_pTube->m_physbody->GetFaceAtIndex(f);

			bool flag = false;
			for (int j = 0; j != 3; ++j)
			{
				for (int k = 0; k != m_vecTube[i].m_vecDisableRRCollisonPoint.size(); ++k)
				{
					if(face->m_Nodes[j] == m_vecTube[i].m_vecDisableRRCollisonPoint[k])
					{
						flag = true;
						break;
					}
				}
				if (flag)
				{
					break;
				}
			}
			if (flag)
			{
				face->DisableCollideWithSoft();
			}
			//pFaces = pFaces->m_Next;
		}
		//pFaces = NULL;

		//��Ѫ����͹���ཻ�����������ײ���عر�,��ֹ���Ӳ�λ��˸
		//pFaces = m_Objects[3]->m_physbody->GetFaceList();
		//while(pFaces)
		for(size_t f = 0 ; f < m_Objects[3]->m_physbody->GetNumFace() ; f++)
		{
			GFPhysSoftBodyFace * face = m_Objects[3]->m_physbody->GetFaceAtIndex(f);

			bool flag = false;
			for (int j = 0; j != 3; ++j)
			{
				if (face->m_Nodes[i]->m_UnDeformedPos.y() <= -2.5)
				{
					flag = true;
					break;
				}
			}
			if (flag)
			{
				face->DisableCollideWithSoft();
			}
			//pFaces = pFaces->m_Next;
		}
		//pFaces = NULL;
	}


    CMXEventsDump::Instance()->RegisterHandleEventsFunc(NewTrainingHandleEvent_LigatingLoop, this);    

	m_TubeTexturPtr[0] = Ogre::TextureManager::getSingleton().load("ligatingLoopTrain_xueguan1.png" , Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,Ogre::TEX_TYPE_2D,
		0,1.0f,false,Ogre::PF_R8G8B8);
	m_TubeTexturPtr[1] = Ogre::TextureManager::getSingleton().load("ligatingLoopTrain_xueguan2.png" , Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,Ogre::TEX_TYPE_2D,
		0,1.0f,false,Ogre::PF_R8G8B8);
	m_TubeTexturPtr[2] = Ogre::TextureManager::getSingleton().load("ligatingLoopTrain_xueguan3.png" , Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,Ogre::TEX_TYPE_2D,
		0,1.0f,false,Ogre::PF_R8G8B8);

	ChangeTexture(0);					//����ʾ��ɫ���

	//��ʼ��Ѫ���Ƿ񱻳��ϵ�״̬��0Ϊδ���ϣ�1Ϊ�ѳ���
	for (int i = 0; i != m_vecTube.size(); ++i)
	{
		m_bTubeBeTearOff.push_back(0);
	}
    m_nSumBindedRopeCount = 0;
	return result;
}

void CLigatingLoopTrain::SetCustomParamBeforeCreatePhysicsPart(MisMedicOrgan_Ordinary * organ)
{
	if(organ->m_OrganID == 0 || organ->m_OrganID == 1 || organ->m_OrganID == 2)
	{
		organ->GetCreateInfo().m_distributemass = false;
	}
}

bool CLigatingLoopTrain::Update( float dt )
{
	bool result = MisNewTraining::Update(dt);

	//����ÿִ֡��
	if (m_counter%10 == 0)
	{
			LogicProcess();
	}

	//��Ѫ�Լ���ʧ
	for (int i = 0; i != m_vecTube.size(); ++i)
	{
		if (true == m_vecTube[i].m_bHasBeenTearOff)				//�Ƿ񱻳���
		{
			m_vecTube[i].m_fCurrentBleedTime += dt;
			m_vecTube[i].m_fCurrentDisapearTime += dt;

			float f = m_vecTube[i].m_fCurrentBleedTime;

			//������Ѫʱ����ֵ��ֹѪ
			if (m_vecTube[i].m_fCurrentBleedTime >= m_fBleedTimeThreshold)	
			{
				m_vecTube[i].m_pTube->stopVesselBleedEffect();
			}

			//������ʧʱ����ֵ����ʧ
			if (m_vecTube[i].m_bHasCalc == false && m_vecTube[i].m_fCurrentDisapearTime >= m_fDisapearTimeThreshold)
			{
				if(m_vecClusterTetrahedron.size() == 2)
				{
					m_vecTube[i].m_pTube->EliminateTetras(m_vecClusterTetrahedron[1]);
					m_vecClusterTetrahedron.clear();
					m_vecClusterResult.clear();
					m_vecTube[i].m_bHasCalc = true;
		
				}
			}
		}
	}

	++m_counter;
	if (m_counter >= 1000)
	{
		m_counter = 0;
	}

	return result;
}

//�ı���ͼ����,flag=0:������ͼ���ó��ޱ������,flag=1:��ǰѪ�ܱ�ǳ�����
void CLigatingLoopTrain::ChangeTexture(const int& flag)
{
	switch(flag)
	{
	case 0:
		
		if(m_LastTexChangeFlag == 0)
		   return;
		
		for (int i = 0; i != m_vecTube.size(); ++i)
		{
			//����ɫ���ȥ��
			Ogre::HardwarePixelBufferSharedPtr pixelBuffer = m_TubeTexturPtr[i]->getBuffer();
			pixelBuffer->lock(Ogre::HardwareBuffer::HBL_NORMAL);
			const Ogre::PixelBox& pixelBox = pixelBuffer->getCurrentLock();
			int width = pixelBox.getWidth();
			int height = pixelBox.getHeight();
			int depth = pixelBox.getDepth();
			unsigned char* imgData = (unsigned char*)pixelBox.data;
			int rowOffset = Ogre::PixelUtil::getMemorySize(width, 1, depth, pixelBox.format);
			int chanle = rowOffset/width/depth;
			for (int i = 0; i != height; ++i)
			{
				unsigned char *pRowData = imgData+i*rowOffset;
				for (int j = 305; j !=410; ++j)
				{
					pRowData[chanle*j] = pRowData[chanle*(j+305)];					//B
					pRowData[chanle*j+1] = pRowData[chanle*(j+305)+1];				//G
					pRowData[chanle*j+2] = pRowData[chanle*(j+305)+2];				//R
				}

				for (int j = 505; j !=610; ++j)
				{
					pRowData[chanle*j] = pRowData[chanle*(j+105)];					//B
					pRowData[chanle*j+1] = pRowData[chanle*(j+105)+1];				//G
					pRowData[chanle*j+2] = pRowData[chanle*(j+105)+2];				//R
				}
			}

			pixelBuffer->unlock();
		}

		m_LastTexChangeFlag = flag;

		break;
	case 1:
		if(m_LastTexChangeFlag == 1 && m_nCurrentTubeIndex == m_LastChangeTubeIndex)
		   return;
		for (int i = 0; i != m_vecTube.size(); ++i)
		{
			if (m_nCurrentTubeIndex == i)
			{
				//���ϻ�ɫ���
				Ogre::HardwarePixelBufferSharedPtr pixelBuffer = m_TubeTexturPtr[i]->getBuffer();
				pixelBuffer->lock(Ogre::HardwareBuffer::HBL_NORMAL);
				const Ogre::PixelBox& pixelBox = pixelBuffer->getCurrentLock();
				int width = pixelBox.getWidth();
				int height = pixelBox.getHeight();
				int depth = pixelBox.getDepth();
				unsigned char* imgData = (unsigned char*)pixelBox.data;
				int rowOffset = Ogre::PixelUtil::getMemorySize(width, 1, depth, pixelBox.format);
				int chanle = rowOffset/width/depth;
				for (int i = 0; i != height; ++i)
				{
					unsigned char *pRowData = imgData+i*rowOffset;
					for (int j = 305; j !=410; ++j)
					{
						pRowData[chanle*j] = 255;					//B
						pRowData[chanle*j+1] = 222;					//G
						pRowData[chanle*j+2] = 0;					//R
					}

					for (int j = 505; j !=610; ++j)
					{
						pRowData[chanle*j] = 255;					//B
						pRowData[chanle*j+1] = 222;					//G
						pRowData[chanle*j+2] = 0;					//R
					}
				}

				pixelBuffer->unlock();
			}
		}
		m_LastTexChangeFlag = flag;
		m_LastChangeTubeIndex = m_nCurrentTubeIndex;
		break;
	}
}

//�߼��ж�
void CLigatingLoopTrain::LogicProcess()
{
	//����Ѫ�ܶ����ѵ��ʱ����ʾ�������ѵ��
	if (!m_bFinished && m_vecTube[0].MissonComplete() && m_vecTube[1].MissonComplete() && m_vecTube[2].MissonComplete())
	{
		if (/*m_bPerfect &&*/ !m_bTimeOut)//m_bPerfect ����̫�ϸ�
		{
			CScoreMgr::Instance()->Grade("Perfect");
		}
        
		TrainingFinish();
		m_bFinished = true;
		return;
	}




	//����Ѫ�ܶ�������ʱ����ʾ���ز�������
	if (!m_bFinished && m_vecTube[0].m_bHasBeenTearOff && m_vecTube[1].m_bHasBeenTearOff && m_vecTube[2].m_bHasBeenTearOff)
	{
		TrainingFatalError();
		//m_bFinished = true;
	}

	int index = UserHasClamTube();		//�ж��Ƿ�ץסѪ�ܣ�-1��ʾûץס��0Ϊץס��Ѫ�ܣ�1Ϊץס�м�Ѫ�ܣ�2Ϊץס��Ѫ��
	if (index != -1)
	{
		//��ץסѪ������ʾ��ɫ���
		if (ShouldShowArea(index))
		{
			m_nCurrentTubeIndex = index;
			ChangeTexture(1);
			if (!m_vecTube[index].m_bHasBeenTearOff)
			{
				bool hasSelectTip = false;

				if (m_vecTube[index].m_bArea1BindComplete)
				{
					if (!m_vecTube[index].m_bArea1CutComplete)
					{
						CTipMgr::Instance()->ShowTip("LigationTubeOK1");
						hasSelectTip = true;
					}
				}
				else if (m_vecTube[index].m_bArea2BindComplete)
				{
					if (!m_vecTube[index].m_bArea2CutComplete)
					{
						CTipMgr::Instance()->ShowTip("LigationTubeOK2");
						hasSelectTip = true;
					}
				}

				if (hasSelectTip == false && (m_vecTube[index].m_bArea1BindComplete == false || m_vecTube[index].m_bArea2BindComplete == false))
				{
					CTipMgr::Instance()->ShowTip("PleaseligationTube");
				}
			}
		}
	
		//��Ѫ�ܳ��ȳ�����ֵ�򳶶�Ѫ��
		if (!m_vecTube[index].m_bHasBeenTearOff && IsTearOffTheTube(index))
		{
			m_vecTube[index].m_bHasBeenTearOff = true;
			TearOffTheTube(index);
			BFSLinkedAera(index);
			CTipMgr::Instance()->ShowTip("operationError");
			m_bPerfect = false;

			m_bTubeBeTearOff[index] = 1;
		}
	}
	else
	{
		//���ػ�ɫ���
		ChangeTexture(0);
		if (EngineCore::IsUnlocked())
		{
			CTipMgr::Instance()->ShowTip("TrainingIntro");
		}
		
	}

	if (m_nCurrentTubeIndex < m_vecTube.size())
	{
		MisMedicOrgan_Ordinary* pTube = m_vecTube[m_nCurrentTubeIndex].m_pTube;

		//�ж��Ƿ���Ҫ�ж������Ƿ���ȷ
		if (!pTube->m_OrganAttachments.empty() && index != -1)
		{
			//++m_vecTube[m_nCurrentTubeIndex].m_nPreBindedRopeCount;
			JudgeRopeBindOnRightArea(pTube);
		}
		
		if (m_vecTube[m_nCurrentTubeIndex].MissonComplete())
		{
			ChangeTexture(0);
		}
	}
}

//��ȡѵ����ز���
bool CLigatingLoopTrain::ReadTrainParam(const std::string& strFileName)
{
	std::ifstream stream;
	stream.open(strFileName.c_str());
	if(stream.is_open())
	{
		char buffer[100];
		while(stream.getline(buffer,99))
		{
			std::string str = buffer;
			int keyEnd = str.find('(');
			std::string key = str.substr(0,keyEnd);
			if (key == "UVPair1")									//��ɫ���UV
			{
				int valEnd = str.find(')');
				std::string val = str.substr(keyEnd+1, valEnd-(keyEnd+1));
				int val1End = val.find(',');
				std::string val1str = val.substr(0, val1End);
				std::string va12str = val.substr(val1End+1, val.length()-val1End-1);
				m_area0 = atof(val1str.c_str());
				m_area1 = atof(va12str.c_str());
			}
			else if (key == "UVPair2")								//��һ����ɫ���UV
			{
				int valEnd = str.find(')');
				std::string val = str.substr(keyEnd+1, valEnd-(keyEnd+1));
				int val1End = val.find(',');
				std::string val1str = val.substr(0, val1End);
				std::string va12str = val.substr(val1End+1, val.length()-val1End-1);
				m_area2 = atof(val1str.c_str());
				m_area3 = atof(va12str.c_str());
			}
			else if (key == "Tube1TearOffPoint")					//���Ѫ�ܵĳ��ϵ�ID
			{
				std::vector<GFPhysSoftBodyNode*> vecTPoint;
				int valEnd = str.find(')');
				std::string val = str.substr(keyEnd+1,valEnd-(keyEnd+1));
				int val1End = val.find(',');
				std::string val1str = val.substr(0,val1End);
				val = val.substr(val1End+1);
				int val2End = val.find(',');
				std::string val2str = val.substr(0, val2End);
				val = val.substr(val2End+1);
				int val3End = val.find(',');
				std::string val3str = val.substr(0,val3End);
				std::string val4str = val.substr(val3End+1, val.length()-val3End-1);

				m_vecTube[0].AddTearOffPoint(atoi(val1str.c_str()));
				m_vecTube[0].AddTearOffPoint(atoi(val2str.c_str()));
				m_vecTube[0].AddTearOffPoint(atoi(val3str.c_str()));
				m_vecTube[0].AddTearOffPoint(atoi(val4str.c_str()));

			}
			else if (key == "Tube2TearOffPoint")					//�м�Ѫ�ܵĳ���ID
			{
				std::vector<GFPhysSoftBodyNode*> vecTPoint;
				int valEnd = str.find(')');
				std::string val = str.substr(keyEnd+1,valEnd-(keyEnd+1));
				int val1End = val.find(',');
				std::string val1str = val.substr(0,val1End);
				val = val.substr(val1End+1);
				int val2End = val.find(',');
				std::string val2str = val.substr(0, val2End);
				val = val.substr(val2End+1);
				int val3End = val.find(',');
				std::string val3str = val.substr(0,val3End);
				std::string val4str = val.substr(val3End+1, val.length()-val3End-1);

				m_vecTube[1].AddTearOffPoint(atoi(val1str.c_str()));
				m_vecTube[1].AddTearOffPoint(atoi(val2str.c_str()));
				m_vecTube[1].AddTearOffPoint(atoi(val3str.c_str()));
				m_vecTube[1].AddTearOffPoint(atoi(val4str.c_str()));
			}
			else if (key == "Tube3TearOffPoint")					//�Ҳ�Ѫ�ܵĳ���ID
			{
				std::vector<GFPhysSoftBodyNode*> vecTPoint;
				int valEnd = str.find(')');
				std::string val = str.substr(keyEnd+1,valEnd-(keyEnd+1));
				int val1End = val.find(',');
				std::string val1str = val.substr(0,val1End);
				val = val.substr(val1End+1);
				int val2End = val.find(',');
				std::string val2str = val.substr(0, val2End);
				val = val.substr(val2End+1);
				int val3End = val.find(',');
				std::string val3str = val.substr(0,val3End);
				std::string val4str = val.substr(val3End+1, val.length()-val3End-1);

				m_vecTube[2].AddTearOffPoint(atoi(val1str.c_str()));
				m_vecTube[2].AddTearOffPoint(atoi(val2str.c_str()));
				m_vecTube[2].AddTearOffPoint(atoi(val3str.c_str()));
				m_vecTube[2].AddTearOffPoint(atoi(val4str.c_str()));
			}
			else if (key == "Tube1FixEndPoint")						//���Ѫ�ܵ���ʼ�˵㣬���ڼ���Ѫ�ܳ���
			{
				int valEnd = str.find(')');
				std::string val = str.substr(keyEnd+1, valEnd-(keyEnd+1));
				int val1End = val.find(',');
				std::string val1str = val.substr(0, val1End);
				std::string va12str = val.substr(val1End+1, val.length()-val1End-1);
				m_vecTube[0].SetFixAndEndPoint(atoi(val1str.c_str()), atoi(va12str.c_str()));
			}
			else if (key == "Tube2FixEndPoint")						//�м�Ѫ�ܵ���ʼ�˵㣬���ڼ���Ѫ�ܳ���
			{
				int valEnd = str.find(')');
				std::string val = str.substr(keyEnd+1, valEnd-(keyEnd+1));
				int val1End = val.find(',');
				std::string val1str = val.substr(0, val1End);
				std::string va12str = val.substr(val1End+1, val.length()-val1End-1);
				m_vecTube[1].SetFixAndEndPoint(atoi(val1str.c_str()), atoi(va12str.c_str()));
			}
			else if (key == "Tube3FixEndPoint")						//�Ҳ�Ѫ�ܵ���ʼ�˵㣬���ڼ���Ѫ�ܳ���
			{
				int valEnd = str.find(')');
				std::string val = str.substr(keyEnd+1, valEnd-(keyEnd+1));
				int val1End = val.find(',');
				std::string val1str = val.substr(0, val1End);
				std::string va12str = val.substr(val1End+1, val.length()-val1End-1);
				m_vecTube[2].SetFixAndEndPoint(atoi(val1str.c_str()), atoi(va12str.c_str()));
			}
			else if (key == "TubeFixePoint")						//Ѫ�ܵ�fix point
			{
				int valEnd = str.find(')');
				int subValEnd = 0;
				std::string val = str.substr(keyEnd+1,valEnd-(keyEnd+1));
				while (-1 != subValEnd)
				{
					subValEnd = val.find(',');
					std::string valStr = val.substr(0,subValEnd);
					for (int i = 0; i != m_vecTube.size(); ++i)
					{
						m_vecTube[i].AddFixPoint(atoi(valStr.c_str()));
					}
					val = val.substr(subValEnd+1);
				}
			}
			else if (key == "BleedTime")							//��Ѫ����ʱ��
			{
				int valEnd = str.find(')');
				int subValEnd = 0;
				std::string val = str.substr(keyEnd+1,valEnd-(keyEnd+1));
				while (-1 != subValEnd)
				{
					subValEnd = val.find(',');
					std::string valStr = val.substr(0,subValEnd);
					m_fBleedTimeThreshold  = atoi(valStr.c_str());
					val = val.substr(subValEnd+1);
				}
			}
			else if (key == "TubeDispearTime")						//�����Ϻ�Ѫ�ܵĴ��ʱ��
			{
				int valEnd = str.find(')');
				int subValEnd = 0;
				std::string val = str.substr(keyEnd+1,valEnd-(keyEnd+1));
				while (-1 != subValEnd)
				{
					subValEnd = val.find(',');
					std::string valStr = val.substr(0,subValEnd);
					m_fDisapearTimeThreshold  = atoi(valStr.c_str());
					val = val.substr(subValEnd+1);
				}
			}
			else if (key == "TearOffThreshold")						//�ж�Ϊ���ϵ�Ѫ�ܳ�����ֵ
			{
				int valEnd = str.find(')');
				int subValEnd = 0;
				std::string val = str.substr(keyEnd+1,valEnd-(keyEnd+1));
				while (-1 != subValEnd)
				{
					subValEnd = val.find(',');
					std::string valStr = val.substr(0,subValEnd);
					m_fTearOffThreshold  = atof(valStr.c_str());
					val = val.substr(subValEnd+1);
				}
			}
			else if (key == "ShowAreaThreshold")					//��ʾ��ɫ��ǵ�Ѫ�ܳ�����ֵ
			{
				int valEnd = str.find(')');
				int subValEnd = 0;
				std::string val = str.substr(keyEnd+1,valEnd-(keyEnd+1));
				while (-1 != subValEnd)
				{
					subValEnd = val.find(',');
					std::string valStr = val.substr(0,subValEnd);
					m_fShowAreaThreshold  = atof(valStr.c_str());
					val = val.substr(subValEnd+1);
				}
			}
		}
		return true;
	}
	return false;
}


//�Ƿ��������˻�ɫ���������
void CLigatingLoopTrain::JudgeRopeBindOnRightArea(MisMedicOrgan_Ordinary* pTube)
{
    TUBEINFO& tbinfo = m_vecTube[m_nCurrentTubeIndex];

	if (pTube->m_OrganAttachments.size()<=tbinfo.m_nCurrentBindedRopeCount)
	{
		return;
	}

    if (tbinfo.m_bArea1BindComplete && tbinfo.m_bArea2BindComplete)
    {
        return;
    }
	MisMedicBindedRope* attch = dynamic_cast<MisMedicBindedRope*>(pTube->m_OrganAttachments[pTube->m_OrganAttachments.size()-1]);
	if (attch)
	{
		int OriginMatID, OriginFaceid;
        Real threshold = 0.025f;

		int matchCount1 = 0; //����
		
		int matchCount2 = 0; //Զ��

		for (int i = 0; i < attch->GetNumBindPoints(); ++i)
		{
			MisMedicBindedRope::ThreadBindPoint bp = attch->GetBindPoint(i);
			GFPhysSoftBodyFace* pFace = bp.m_AttachFace;
			pTube->ExtractFaceIdAndMaterialIdFromUsrData(pFace , OriginMatID , OriginFaceid);

			if (OriginFaceid < pTube->m_OriginFaces.size())
			{
				MMO_Face face = pTube->m_OriginFaces[OriginFaceid];
			
			    //���ݰ�ס�����ϵĵ��UV���ж��Ƿ�������ȷ
			    Ogre::Vector2 vertTexCoord = face.GetTextureCoord(bp.m_Weights);

				if (!tbinfo.m_bArea1BindComplete && vertTexCoord.x >= m_area0 - threshold && vertTexCoord.x <= m_area1 + threshold)
				{
					++matchCount1;
				}
				if (!tbinfo.m_bArea2BindComplete && vertTexCoord.x >= m_area2 - threshold && vertTexCoord.x <= m_area3 + threshold)
				{
					++matchCount2;
				}
			}
		}

		if (matchCount1 > 0 && (tbinfo.m_bArea1BindComplete == false))
		{
			tbinfo.m_bArea1BindComplete = true;
			//����Ϊ�÷�����ʾ
			CScoreMgr::Instance()->Grade("BindTubeSuccess");
			CTipMgr::Instance()->ShowTip("LigationTubeOK1");
			tbinfo.m_nCurrentBindedRopeCount++;
			m_nSumBindedRopeCount++;
		}
		else if (matchCount2 > 0 && (tbinfo.m_bArea2BindComplete == false))
		{
			tbinfo.m_bArea2BindComplete = true;
			//����Ϊ�÷�����ʾ
			CScoreMgr::Instance()->Grade("BindTubeSuccess");
			CTipMgr::Instance()->ShowTip("LigationTubeOK2");
			tbinfo.m_nCurrentBindedRopeCount++;
			//tbinfo.m_nPreBindedRopeCount++;
			m_nSumBindedRopeCount++;
		}
		else// if (matchCount2 == 0 && matchCount1 == 0)
		{
			tbinfo.m_bindError = true;
			tbinfo.m_bindErrorNum++;

			m_bPerfect = false;
			CTipMgr::Instance()->ShowTip("LigationError");
			return;
		}

	}
}


//�Ƿ��������
void CLigatingLoopTrain::JudgeRopeCutted()
{
	TUBEINFO& tbinfo = m_vecTube[m_nCurrentTubeIndex];    

    if (tbinfo.m_bArea1CutComplete && tbinfo.m_bArea2CutComplete)
    {
        return;
    }

	if (tbinfo.m_bArea1BindComplete && !tbinfo.m_bArea1CutComplete)
	{
		tbinfo.m_bArea1CutComplete = true;
		//�ӷֺ���ʾ
		CScoreMgr::Instance()->Grade("CutTubeSuccess");        
		CTipMgr::Instance()->ShowTip("CutRopeOK");
		ChangeTexture(0);
	}
	
    if (tbinfo.m_bArea2BindComplete && !tbinfo.m_bArea2CutComplete)
	{
		tbinfo.m_bArea2CutComplete = true;
		//�ӷֺ���ʾ
		CScoreMgr::Instance()->Grade("CutTubeSuccess");        
		CTipMgr::Instance()->ShowTip("CutRopeOK");
		ChangeTexture(0);
	}


    if (!tbinfo.m_bArea1BindComplete || !tbinfo.m_bArea2BindComplete)
	{
		CTipMgr::Instance()->ShowTip("PleaseGraspTube");
		m_bPerfect = false;
	}
}

//ץǯ�Ƿ�ץס��Ѫ��,������Ѫ��ID��-1��ʾ��ûץס�κ�Ѫ��
int CLigatingLoopTrain::UserHasClamTube()
{
	CGraspingForceps *pForceps = dynamic_cast<CGraspingForceps*>(m_pToolsMgr->GetLeftTool());
	if (pForceps)
	{
		for (int i = 0; i != pForceps->m_ToolPlugins.size(); ++i)
		{
			MisCTool_PluginClamp *pPlugin = dynamic_cast<MisCTool_PluginClamp*>(pForceps->m_ToolPlugins[i]);
			if (pPlugin)
			{
				/*MisMedicOrgan_Ordinary* pTube = pPlugin->GetOrganBeClamped();
				if (pTube)
				{
					for (int j = 0; j != m_vecTube.size(); ++j)
					{
						if (m_vecTube[j].m_pTube == pTube)
						{
							return j;
						}
					}
				}*/
				std::vector<MisMedicOrgan_Ordinary *> organsClamped;
				pPlugin->GetOrgansBeClamped(organsClamped);

				for(size_t c = 0 ; c < organsClamped.size() ; c++)
				{
					for (int j = 0; j != m_vecTube.size(); ++j)
					{
						if (m_vecTube[j].m_pTube == organsClamped[c])
						{
							return j;
						}
					}
				}
			}
		}
	}

	pForceps = dynamic_cast<CGraspingForceps*>(m_pToolsMgr->GetRightTool());
	if (pForceps)
	{
		for (int i = 0; i != pForceps->m_ToolPlugins.size(); ++i)
		{
			MisCTool_PluginClamp *pPlugin = dynamic_cast<MisCTool_PluginClamp*>(pForceps->m_ToolPlugins[i]);
			if (pPlugin)
			{
				//MisMedicOrgan_Ordinary* pTube = pPlugin->GetOrganBeClamped();
				std::vector<MisMedicOrgan_Ordinary *> organsClamped;
				pPlugin->GetOrgansBeClamped(organsClamped);
				
				for(size_t c = 0 ; c < organsClamped.size() ; c++)
				{
					for (int j = 0; j != m_vecTube.size(); ++j)
					{
						if (m_vecTube[j].m_pTube == organsClamped[c])
						{
							return j;
						}
					}
				}
			}
		}
	}

	CKnotter *pKnotter = dynamic_cast<CKnotter*>(m_pToolsMgr->GetLeftTool());
	
	if (pKnotter)
	{
		if (!pKnotter->m_OrganBindRope)
		{
			return -1;
		}
		const MisMedicOrgan_Ordinary* pTube = pKnotter->m_OrganBindRope->GetBindedOrgan();
		if (pTube)
		{
			for (int j = 0; j != m_vecTube.size(); ++j)
			{
				if (m_vecTube[j].m_pTube == pTube)
				{
					return j;
				}
			}
		}
	}

	pKnotter = dynamic_cast<CKnotter*>(m_pToolsMgr->GetRightTool());
	if (pKnotter)
	{
		if (!pKnotter->m_OrganBindRope)
		{
			return -1;
		}
		const MisMedicOrgan_Ordinary* pTube = pKnotter->m_OrganBindRope->GetBindedOrgan();
		if (pTube)
		{
			for (int j = 0; j != m_vecTube.size(); ++j)
			{
				if (m_vecTube[j].m_pTube == pTube)
				{
					return j;
				}
			}
		}
	}


	return -1;
}


//�����Ƿ����㳶��Ѫ�ܵ�����
bool CLigatingLoopTrain::IsTearOffTheTube(const int& index)
{

	GFPhysVector3 pos1 = m_vecTube[index].m_pFixPoint->m_CurrPosition;
	GFPhysVector3 pos2 = m_vecTube[index].m_pEndPoint->m_CurrPosition;
	float distance = pos1.Distance(pos2);
	if (distance > m_fTearOffThreshold)
	{
		return true;
	}
	return false;
}


//����Ѫ�ܣ���ʵ����ģ���������Ѫ��
void CLigatingLoopTrain::TearOffTheTube(const int& index)
{
	GFPhysVector3 v1 = (m_vecTube[index].m_vecTearOffPoint[0]->m_UnDeformedPos - m_vecTube[index].m_vecTearOffPoint[1]->m_UnDeformedPos).Normalized();
	GFPhysVector3 v2 = (m_vecTube[index].m_vecTearOffPoint[1]->m_UnDeformedPos -m_vecTube[index]. m_vecTearOffPoint[0]->m_UnDeformedPos).Normalized();
	GFPhysVector3 v3 = (m_vecTube[index].m_vecTearOffPoint[0]->m_UnDeformedPos - m_vecTube[index].m_vecTearOffPoint[2]->m_UnDeformedPos).Normalized();
	GFPhysVector3 v4 = (m_vecTube[index].m_vecTearOffPoint[2]->m_UnDeformedPos - m_vecTube[index].m_vecTearOffPoint[0]->m_UnDeformedPos).Normalized();

	GFPhysVector3 cutQuads[4];
	cutQuads[0] = m_vecTube[index].m_vecTearOffPoint[0]->m_UnDeformedPos+v1*20;
	cutQuads[1] = m_vecTube[index].m_vecTearOffPoint[0]->m_UnDeformedPos+v4*20;
	cutQuads[2] = m_vecTube[index].m_vecTearOffPoint[0]->m_UnDeformedPos+v3*20;
	cutQuads[3] = m_vecTube[index].m_vecTearOffPoint[0]->m_UnDeformedPos+v2*20;

	m_vecTube[index].m_pTube->TearOrganBySemiInfinteQuad(cutQuads, false);
}


//�����Ƿ�������ʾ��ɫ��ǵ�����
bool CLigatingLoopTrain::ShouldShowArea(const int& index)
{
	GFPhysVector3 pos1 = m_vecTube[index].m_pFixPoint->m_CurrPosition;
	GFPhysVector3 pos2 = m_vecTube[index].m_pEndPoint->m_CurrPosition;
	float distance = pos1.Distance(pos2);
	if (distance > m_fShowAreaThreshold)
	{
		return true;
	}
	return false;
}

void CLigatingLoopTrain::OnToolCreated(ITool * tool, int side)
{
	CKnotter * knotter = dynamic_cast<CKnotter *>(tool);
	if(knotter)
	{
		MisMedicThreadRope * rope = knotter->GetCurrentThread();
		if(rope)
		{
			rope->SetGravity(GFPhysVector3(0 , -3, 0));
		}
	}
}

//BFS����һ��Ѫ�ܵ���ͨ���򲢷���
int CLigatingLoopTrain::BFSLinkedAera(int index)
{
	m_vecClusterResult.clear();
	m_vecClusterTetrahedron.clear();
	std::vector<GFPhysSoftBodyTetrahedron *> SelectedTetras;
	SelectedTetras.reserve(10000);

	//����Ѫ�ܵ����������� ��������Ϊ0,0��ʾΪû�б����ʹ�
	//GFPhysSoftBodyTetrahedron * tetra = m_vecTube[index].m_pTube->m_physbody->GetTetrahedronList();
	//while(tetra)
	for(size_t th = 0 ; th < m_vecTube[index].m_pTube->m_physbody->GetNumTetrahedron() ; th++)
	{
		GFPhysSoftBodyTetrahedron * tetra = m_vecTube[index].m_pTube->m_physbody->GetTetrahedronAtIndex(th);

		tetra->m_TempData = (void*)0;
		SelectedTetras.push_back(tetra);
		tetra = tetra->m_Next;
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


	//3��nodeΪ�̶��㣬�ҵ�������õ�������岢��������vector��һ��λ��,��Ĭ���������������ͨ��������ķ�����Ϊ1
	for (size_t t = 0 ; t < SelectedTetras.size(); t++)
	{
		bool flag = false;
		GFPhysSoftBodyTetrahedron * tetra = SelectedTetras[t];
		for (int i = 0; i != 4; ++i)
		{
			if (tetra->m_TetraNodes[i] == m_vecTube[index].m_pFixPoint)
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
						GFPhysSoftBodyTetrahedron * t0 = genFace->m_ShareTetrahedrons[0].m_Hosttetra;

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
	return m_vecClusterResult.size();
}

void CLigatingLoopTrain::OnSaveTrainingReport()
{
    Real usedtime = GetElapsedTime();
    //////////////////////////////////////////////////////////////////////////
    for (int i = 0; i < 3; i++)
    {
        Ogre::String strTarget = "Tube" + Ogre::StringConverter::toString(i + 1);

        if (m_vecTube[i].m_bArea1BindComplete && m_vecTube[i].m_bArea2BindComplete)
        {
            COnLineGradeMgr::Instance()->SendGrade((strTarget + "_2_Bind_Success"), 0, usedtime);
            COnLineGradeMgr::Instance()->SendGrade((strTarget + "_Bind_Correct"), 0, usedtime);
        }
        else if ( m_vecTube[i].m_bArea1BindComplete  || m_vecTube[i].m_bArea2BindComplete )
        {
            COnLineGradeMgr::Instance()->SendGrade((strTarget + "_1_Bind_Success"), 0, usedtime);
        }
        else 
        {
            COnLineGradeMgr::Instance()->SendGrade((strTarget + "_0_Bind_Success"), 0, usedtime);
        }

        if (m_vecTube[i].m_bindError && m_vecTube[i].m_bindErrorNum < 3)
        {
            COnLineGradeMgr::Instance()->SendGrade((strTarget + "_Bind_Bias"), 0, usedtime);
        }
        else if (m_vecTube[i].m_bindError && m_vecTube[i].m_bindErrorNum >= 3)
        {
            COnLineGradeMgr::Instance()->SendGrade((strTarget + "_Bind_Error"), 0, usedtime);
        }

        if (m_vecTube[i].m_bArea1CutComplete && m_vecTube[i].m_bArea2CutComplete)
        {
            COnLineGradeMgr::Instance()->SendGrade((strTarget + "_Cut_Success"), 0, usedtime);
        }
        else
        {
            COnLineGradeMgr::Instance()->SendGrade((strTarget + "_Cut_Error"), 0, usedtime);
        }
    }
    //////////////////////////////////////////////////////////////////////////
    //Real toolspeed = m_pToolsMgr->GetLeftToolMovedSpeed();
    //if (!toolspeed)
    //    toolspeed = m_pToolsMgr->GetRightToolMovedSpeed();

    Real leftToolSpeed = m_pToolsMgr->GetLeftToolMovedSpeed();
    Real rightTooSpeed = m_pToolsMgr->GetRightToolMovedSpeed();
    Real ToolSpeed = std::max(leftToolSpeed, rightTooSpeed);

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
    if (ToolSpeed <= 5.0f && ToolSpeed > GP_EPSILON)
        COnLineGradeMgr::Instance()->SendGrade("MachineHandle_Normal", 0, usedtime);
    else if (ToolSpeed > 5.0f && ToolSpeed <= 10.0f)
        COnLineGradeMgr::Instance()->SendGrade("MachineHandle_Fast", 0, usedtime);
    else if (ToolSpeed > 10.0f)
        COnLineGradeMgr::Instance()->SendGrade("MachineHandle_TooFast", 0, usedtime);


    //int n = 0;
    //for (DynObjMap::iterator itr = m_DynObjMap.begin(); itr != m_DynObjMap.end(); ++itr)
    //{
    //    n += itr->second->GetAttachmentCount(MOAType_BindedRope);
    //}
    //assert(m_nsumBindCount == n);

    //int sumErrorCount = m_vecTube[0].m_bindErrorNum + m_vecTube[1].m_bindErrorNum + m_vecTube[2].m_bindErrorNum;
    int sumBingoCount = m_vecTube[0].m_nCurrentBindedRopeCount + m_vecTube[1].m_nCurrentBindedRopeCount + m_vecTube[2].m_nCurrentBindedRopeCount;
    //assert(sumErrorCount + sumBingoCount == m_nsumBindCount);

    Real rate = sumBingoCount * 1.0f / m_nsumBindCount;

    if (rate == 1.0f)
    {
        COnLineGradeMgr::Instance()->SendGrade("Bind_Accuracy_E100", 0, usedtime);
    }
    else if (rate > 0.7f && rate < 1.0f)
    {
        COnLineGradeMgr::Instance()->SendGrade("Bind_Accuracy_G70", 0, usedtime);
    }
    else
    {
        COnLineGradeMgr::Instance()->SendGrade("Bind_Accuracy_L70", 0, usedtime);
    }
    //////////////////////////////////////////////////////////////////////////    

    if (m_bFinished)
    {
        if (sumBingoCount == 6)
        {
            COnLineGradeMgr::Instance()->SendGrade("Bind_Num_Eq6", 0, usedtime);
        }
        else if (sumBingoCount == 7 || sumBingoCount == 8)
        {
            COnLineGradeMgr::Instance()->SendGrade("Bind_Num_Eq78", 0, usedtime);
        }
        else if (sumBingoCount >= 9)
        {
            COnLineGradeMgr::Instance()->SendGrade("Bind_Num_Ge9", 0, usedtime);
        }
    }

    if (m_bFinished)
    {
        if (usedtime < 480)
        {
            COnLineGradeMgr::Instance()->SendGrade("Finished_In8M", 0, usedtime);
            if (usedtime <= 360 && sumBingoCount < 9)
            {
                COnLineGradeMgr::Instance()->SendGrade("Twohands_Cooperation", 0, usedtime);
            }
        }
        else if (usedtime >= 480 && usedtime < 720)
        {
            COnLineGradeMgr::Instance()->SendGrade("Finished_In12M", 0, usedtime);
        }
    }
    else
    {
        COnLineGradeMgr::Instance()->SendGrade("UnFinish_In12M", 0, usedtime);
    }

    __super::OnSaveTrainingReport();
}