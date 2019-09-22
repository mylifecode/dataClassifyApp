#include "SutureTrainV2.h"
#include "MisMedicOrganOrdinary.h"
#include "MisMedicRigidPrimtive.h"
#include "SutureNeedleV2.h"
#include "collision\NarrowPhase\GoPhysPrimitiveTest.h"
#include "MXOgreGraphic.h"
#include "Instruments\Tool.h"
#include "Instruments\MisCTool_PluginRigidHold.h"
#include "BasicTraining.h"
#include "Instruments\NeedleHolder.h"
#include "MisMedicEffectRender.h"
#include "QKeyEvent"
#define BALLPOSEXTENT 0.6f
#define BASERADIUS 8.873f

#define BASECENTERX 0.0f
#define BASECENTERY 0.0f
#define BASECENTERZ -2.232f

//=============================================================================================
CSutureTrainV2::CSutureTrainV2(void)
{
	m_Needle = 0;
	m_pBaseTerrainRigidBody = 0;
	m_pNeedleTestOrgan = 0;

	m_index[0] = 436;//B2
	m_index[1] = 482;//B1
	m_index[2] = 456;//A2
	m_index[3] = 483;//A1

	m_HolderAngle = -1.0f;///负数为了评分需要
	m_InOrganAngle[0] = m_InOrganAngle[1] = -1.0f;
	m_ABSew = ASew;
	m_BaseBall[0][0] = false;
	m_MinorToolHold = m_MajorToolHold = m_PreMajorToolHold = false;
	m_PreNeedleHoldState = false;
	m_NeedleHoldState = false;
	m_RetreatNeedleNum[0][0] = m_RetreatNeedleNum[0][1] = m_RetreatNeedleNum[1][0] = m_RetreatNeedleNum[1][1] = 0;
	m_FirstHoldSuccess = false;
	m_NeedleLostStateLast = false;
	m_NeedleLostState = false;
	m_NeedleLostCount = 0;

	m_FuzhuFirstHold = true;
	m_TotalHoldCount = 0;
	m_GraspNeedleInit = false;
	m_NeedleInStart = false;
	m_NeedleInDegree = false;
	m_NeedleInPositionTip = false;
	m_NeedleOutPositionTip = false;
	m_PullNeedleOut = false;
	m_NeedleInRepeatErrorTip = false;
	m_NeedleOutRepeatErrorTip = false;
	m_GraspNeedleCarefully = false;
	m_GraspNeedleAgain = false;
	m_FirstPassFinish = false;
	m_AInOutPos[0] = m_AInOutPos[1] = m_BInOutPos[0] = m_BInOutPos[1] = -1;
	m_AInCount = m_BInCount = 0;
	m_TipState = 1;
	m_InoutFace[0] = m_InoutFace[1] = 0;
	m_HoldPosition = false;
	m_bMarkState = true;
}
//=============================================================================================
CSutureTrainV2::~CSutureTrainV2(void)
{
}
//======================================================================================================================
bool CSutureTrainV2::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{
	bool result = MisNewTraining::Initialize(pTrainingConfig, pToolConfig);

	PhysicsWrapper::GetSingleTon().m_dynamicsWorld->GetSolverParameter().m_PositionCorrect = true;

	DynObjMap::iterator itor = m_DynObjMap.begin();

	while (itor != m_DynObjMap.end())
	{
		MisMedicOrganInterface * oif = itor->second;

		MisMedicOrgan_Ordinary * organmesh = dynamic_cast<MisMedicOrgan_Ordinary*>(oif);

		MisMedicDynObjConstructInfo & cs = oif->GetCreateInfo();

		if (organmesh)
		{
			//MisMedicDynObjConstructInfo & cs = organmesh->GetCreateInfo();

			if (cs.m_name == "NeedleTestOrgan")
			{
				organmesh->m_physbody->EnableDoubleFaceCollision();//开启双面碰撞
				//organmesh->m_physbody->SetCollisionCategory(1);//设置分组
				m_pNeedleTestOrgan = organmesh;
				int collisiontag = m_pNeedleTestOrgan->m_physbody->GetCollisionFlags();
			}
		}
		else if (cs.m_name.find("base_terrain") != -1)
		{
			MisMedicRigidPrimtive* pBaseTerrain = dynamic_cast<MisMedicRigidPrimtive*>(oif);
			if (pBaseTerrain)
			{
				m_pBaseTerrainRigidBody = pBaseTerrain->m_body;
			}
		}
		itor++;
	}

	m_Needle = CreateNeedleV2(MXOgre_SCENEMANAGER, m_SutureNodeNum, m_SutureRsLength, m_NeedleSkeletonFile);

	m_Needle->addNeedleActionListener(this);
	InitBallPos();

	m_pNeedleTestOrgan->GetEffectRender().MarkTexBackgClearEveryFrame(true);
	return result;
}
//==========================================================================================================
void CSutureTrainV2::SetCustomParamBeforeCreatePhysicsPart(MisMedicOrgan_Ordinary * organ)
{
	//organ->GetCreateInfo().m_invEdgePhysStiff = 1.0f / 4000.0f;
	//organ->GetCreateInfo().m_EdgePhysDamping = 40.0f;

	//organ->GetCreateInfo().m_invTetraPhysStiff = 1.0f / 20000.0f;
	//organ->GetCreateInfo().m_TetraPhysDamping = 200.0f;
}
//======================================================================================================================
bool CSutureTrainV2::Update(float dt)
{
	if (m_Needle)
	{
		GFPhysVectorObj<FaceAnchorInfoV2>& faceanchors = m_Needle->getFaceNeedleAnchors();

		for (GFPhysVectorObj<FaceAnchorInfoV2>::iterator it = faceanchors.begin();
			it != faceanchors.end();
			it++)
		{
			FaceAnchorInfoV2 anchorinfo = *it;
			if (anchorinfo.pAnchor->m_Type == State_IN)
			{
				m_NeedleInStart = false;
				m_FirstPassFinish = false;
				if (m_ABSew == ASew)
				{
					m_InOrganAngle[0] = anchorinfo.pAnchor->m_initAngle;
					m_NeedleInDegree = true;
				}
				else if (m_ABSew == BSew)
				{
					m_InOrganAngle[1] = anchorinfo.pAnchor->m_initAngle;
					m_NeedleInDegree = true;
				}
				//showDebugInfo("m_InAnchorAngle: ",m_InAnchorAngle);
			}
		}

		//m_Needle->UpdateMesh();        

		//if( m_Needle->GetSutureThread() )
		//{
		//    m_Needle->GetSutureThread()->UpdateMesh();            
		//}
	}

	UpdateInOutBallPos();
	DrawMarkRenderTarget();


	bool result = MisNewTraining::Update(dt);

	DetectNeedleLost();

	DetectNeedleLostCreate();

	///一帧之后把所有合理状态提交到这里，找到最大值，显示该tip，最后运行
	if (m_FirstPassFinish)
	{
		CTipMgr::Instance()->ShowTip("FirstPassFinish");
		m_TipState = 11;
	}
	else if (m_GraspNeedleAgain)
	{
		CTipMgr::Instance()->ShowTip("GraspNeedleAgain");
		m_TipState = 10;
	}
	else if (m_GraspNeedleCarefully)
	{
		CTipMgr::Instance()->ShowTip("GraspNeedleCarefully");
		m_TipState = 9;
	}
	else if (m_NeedleOutRepeatErrorTip)
	{
		CTipMgr::Instance()->ShowTip("NeedleOutRepeatErrorTip");
		m_TipState = 8;
	}
	else if (m_NeedleInRepeatErrorTip)
	{
		CTipMgr::Instance()->ShowTip("NeedleInRepeatErrorTip");
		m_TipState = 7;
	}
	else if (m_PullNeedleOut)
	{
		CTipMgr::Instance()->ShowTip("PullNeedleOut");
		m_TipState = 6;
	}
	else if (m_NeedleOutPositionTip)
	{
		CTipMgr::Instance()->ShowTip("NeedleOutPositionTip");
		m_TipState = 5;
	}
	else if (m_NeedleInPositionTip)
	{
		CTipMgr::Instance()->ShowTip("NeedleInPositionTip");
		m_TipState = 4;
	}
	else if (m_NeedleInDegree)
	{
		if (m_ABSew == ASew)
		{
			CTipMgr::Instance()->ShowTip("NeedleInDegree", int(m_InOrganAngle[0]));
		}
		else if (m_ABSew == BSew)
		{
			CTipMgr::Instance()->ShowTip("NeedleInDegree", int(m_InOrganAngle[1]));
		}
		m_TipState = 3;
	}
	else if (m_NeedleInStart)
	{
		CTipMgr::Instance()->ShowTip("NeedleInStart", int(m_HolderAngle));
		m_TipState = 2;
	}
	else if (m_GraspNeedleInit)
	{
		CTipMgr::Instance()->ShowTip("GraspNeedleInit");
		m_TipState = 1;
	}

	return result;
}

//======================================================================================================================

void CSutureTrainV2::InternalSimulateStart(int currStep, int TotalStep, Real dt)
{
	MisNewTraining::InternalSimulateStart(currStep, TotalStep, dt);
}

//======================================================================================================================

void CSutureTrainV2::InternalSimulateEnd(int currStep, int TotalStep, Real dt)
{
	MisNewTraining::InternalSimulateEnd(currStep, TotalStep, dt);

	GrabNeedleState();
}
//======================================================================================================================
void CSutureTrainV2::InitBallPos()
{
	m_ballnode0 = m_pNeedleTestOrgan->m_physbody->GetNode(m_index[0]);
	m_ballnode1 = m_pNeedleTestOrgan->m_physbody->GetNode(m_index[1]);
	m_ballnode2 = m_pNeedleTestOrgan->m_physbody->GetNode(m_index[2]);
	m_ballnode3 = m_pNeedleTestOrgan->m_physbody->GetNode(m_index[3]);

	int numcount = m_pNeedleTestOrgan->m_OrganRendNodes.size();
	for (int i = 0; i < numcount; ++i)
	{
		GoPhys::GFPhysSoftBodyNode* phynode = m_pNeedleTestOrgan->m_OrganRendNodes[i].m_PhysNode;
		if (phynode == m_ballnode0)
		{
			m_MarkState[0].uvpos = m_pNeedleTestOrgan->m_OrganRendNodes[i].m_TextureCoord;
		}
		else if (phynode == m_ballnode1)
		{
			m_MarkState[1].uvpos = m_pNeedleTestOrgan->m_OrganRendNodes[i].m_TextureCoord;
		}
		else if (phynode == m_ballnode2)
		{
			m_MarkState[2].uvpos = m_pNeedleTestOrgan->m_OrganRendNodes[i].m_TextureCoord;
		}
		else if (phynode == m_ballnode3)
		{
			m_MarkState[3].uvpos = m_pNeedleTestOrgan->m_OrganRendNodes[i].m_TextureCoord;
		}
	}

	for (int i = 0; i < 6; i++)
	{
		m_MarkState[i].visible = true;
		m_MarkState[i].radius = 0.08f;
		m_MarkState[i].matname = "MarkTextureMaterialOrange";
	}

	for (int i = 4; i < 6; i++)
	{
		m_MarkState[i].visible = false;
	}
}
//======================================================================================================================
void CSutureTrainV2::UpdateInOutBallPos()
{
	if (m_MarkState[4].visible && m_bMarkState)
	{
		m_MarkState[4].uvpos = Ogre::Vector2::ZERO;
		int faceId = m_pNeedleTestOrgan->GetOriginFaceIndexFromUsrData(m_InoutFace[0]);
		if (faceId >= 0 && faceId < (int)m_pNeedleTestOrgan->m_OriginFaces.size())
		{
			MMO_Face originface = m_pNeedleTestOrgan->m_OriginFaces[faceId];

			if (originface.m_HasError == false)
			{
				m_MarkState[4].uvpos = originface.GetTextureCoord(0) * m_InoutWeights[0]
					+ originface.GetTextureCoord(1) * m_InoutWeights[1]
					+ originface.GetTextureCoord(2) * m_InoutWeights[2];
			}
		}
	}
	if (m_MarkState[5].visible && m_bMarkState)
	{
		m_MarkState[5].uvpos = Ogre::Vector2::ZERO;
		int faceId = m_pNeedleTestOrgan->GetOriginFaceIndexFromUsrData(m_InoutFace[1]);
		if (faceId >= 0 && faceId < (int)m_pNeedleTestOrgan->m_OriginFaces.size())
		{
			MMO_Face originface = m_pNeedleTestOrgan->m_OriginFaces[faceId];

			if (originface.m_HasError == false)
			{
				m_MarkState[5].uvpos = originface.GetTextureCoord(0) * m_InoutWeights[3]
					+ originface.GetTextureCoord(1) * m_InoutWeights[4]
					+ originface.GetTextureCoord(2) * m_InoutWeights[5];
			}
		}
	}
}
//======================================================================================================================
void CSutureTrainV2::CalcHolderAngle(MisCTool_PluginRigidHold * rigidholdPlugin)
{
	if (rigidholdPlugin == NULL)
		return;
	GFPhysVector3 planeNormal(0.0f, 0.0f, 0.0f);  //弯针所在平面的法向
	GFPhysVector3 toolDir(0.0f, 0.0f, 0.0f);      //器械轴向

	if (!m_PreMajorToolHold)//由非夹持到夹持, 只在跨越的时候调用一次
	{
		if (m_ABSew == ASew && m_TipState <= 2)
		{
			if (m_MarkState[0].matname != "MarkTextureMaterialGreen")
			{
				m_bMarkState = true;
			}
			m_MarkState[0].matname = "MarkTextureMaterialGreen";
			if (m_MarkState[1].matname != "MarkTextureMaterialGreen")
			{
				m_bMarkState = true;
			}
			m_MarkState[1].matname = "MarkTextureMaterialGreen";
		}
	}
	else//一直是夹持状态，调用很多次
	{
		planeNormal = m_Needle->GetNeedleNormalDirction();
		const MisCTool_PluginRigidHold::ToolHoldRegion & BelongRegion = rigidholdPlugin->GetHoldRegion(0);
		toolDir = BelongRegion.m_Axis1World;
		m_HolderAngle = CalcAngleBetweenLineAndFace(toolDir, planeNormal);
	}

	GFPhysRigidBody* rgbody = rigidholdPlugin->GetRigidBodyHolded();
	if (rgbody == m_Needle->GetPhysicBody())
	{
		GFPhysVector3 localpos = rigidholdPlugin->GetRigidBodyHoldLocalPoint();

		GFPhysTransform rgtrans = rgbody->GetCenterOfMassTransform();

		GFPhysVector3 worldpos = rgtrans(localpos);
		//如果worldpos在中间一段(第8节到第11节)则绿色消失 vs m_NeedleNodeWorldPos

		int minindex[2] = { 0, 0 };
		if (m_Needle->Getinterval(worldpos, minindex[0], minindex[1]))
		{
			if ((minindex[0] >= 2 && minindex[0] <= 6) && (minindex[1] >= 2 && minindex[1] <= 6))
			{
				m_Needle->RendGreen(0, -1);
				m_HoldPosition = true;
			}
			else
			{
				m_Needle->RendGreen(2, 6);
				m_HoldPosition = false;
			}
		}
	}

	if (m_FuzhuFirstHold)
	{
		m_FuzhuFirstHold = false;
		if (m_HoldPosition && ((m_HolderAngle >= 85 && m_HolderAngle <= 90) ? 1 : 0))
		{
			m_FirstHoldSuccess = true;
		}
	}
}
//======================================================================================================================

MisCTool_PluginRigidHold* CSutureTrainV2::GrabNeedleStateInternal(CTool* tool)
{
	CNeedleHolder* majorHolder = dynamic_cast<CNeedleHolder*>(tool);
	for (size_t p = 0; p < tool->m_ToolPlugins.size(); p++)
	{
		MisCTool_PluginRigidHold * holdPlugin = dynamic_cast<MisCTool_PluginRigidHold*>(tool->m_ToolPlugins[p]);
		if (holdPlugin && holdPlugin->GetRigidHoldState())
		{
			m_NeedleHoldState = true;
			m_NeedleLostStateLast = false;
			m_NeedleLostState = false;
			if (majorHolder)
			{
				m_MajorToolHold = true;

				if (m_PreMajorToolHold != m_MajorToolHold)
				{
					m_TotalHoldCount++;
				}
				return holdPlugin;
			}
			else
			{
				m_MinorToolHold = true;
			}
			break;
		}
	}
	return NULL;
}

void CSutureTrainV2::GrabNeedleState()
{
	//从左右器械中找到rigidholder插件，从holding = true的插件中计算角度
	CTool * rightTool = (CTool*)(m_pToolsMgr->GetRightTool());
	CTool * leftTool = (CTool*)(m_pToolsMgr->GetLeftTool());

	MisCTool_PluginRigidHold * rigidholdPlugin = NULL;
	m_MinorToolHold = false;
	m_MajorToolHold = false;
	m_NeedleHoldState = false;//也可以用ifelse判断
	if (rightTool)
	{
		rigidholdPlugin = GrabNeedleStateInternal(rightTool);
	}
	if (leftTool && rigidholdPlugin == NULL)
	{
		rigidholdPlugin = GrabNeedleStateInternal(leftTool);
	}

	if (!leftTool && !rightTool)//没有工具 用最后记录值
	{
		m_Needle->RendGreen(0, -1);
	}
	else if (m_MajorToolHold)//夹
	{
		m_GraspNeedleInit = false;
		m_GraspNeedleCarefully = false;
		m_GraspNeedleAgain = false;

		CalcHolderAngle(rigidholdPlugin);
		m_NeedleInStart = true;
	}
	else if (m_MinorToolHold)
	{
		m_GraspNeedleInit = true;
		m_NeedleInStart = false;
		m_GraspNeedleCarefully = false;
		m_GraspNeedleAgain = false;

		m_Needle->RendGreen(2, 6);
		m_HoldPosition = false;
		m_HolderAngle = -1.0f;
	}
	else if (!m_NeedleHoldState)//没夹
	{
		m_GraspNeedleInit = true;
		if (m_PreNeedleHoldState)//由夹持到非夹持,一次
		{
			m_GraspNeedleCarefully = true;
		}

		m_Needle->RendGreen(2, 6);
		m_HoldPosition = false;
		m_HolderAngle = -1.0f;
	}
	m_PreMajorToolHold = m_MajorToolHold;
	m_PreNeedleHoldState = m_NeedleHoldState;
}
//======================================================================================================================
bool CSutureTrainV2::GetBaseBallPos(const int i, GFPhysVector3& pos)
{
	if (i > 3 || i < 0)
	{
		return false;
	}
	else
	{
		pos = m_pNeedleTestOrgan->m_physbody->GetNode(m_index[i])->m_CurrPosition;
		return true;
	}
}
//======================================================================================================================
void CSutureTrainV2::OnCreateInAnchor(const GFPhysSoftBodyFace* face, const Real weights[])
{
	GFPhysVector3 anchorpos = face->m_Nodes[0]->m_CurrPosition * weights[0]
		+ face->m_Nodes[1]->m_CurrPosition * weights[1]
		+ face->m_Nodes[2]->m_CurrPosition * weights[2];

	Ogre::Vector3 ball1pos;
	Ogre::Vector3 ball2pos;
	m_BaseBall[0][0] = m_BaseBall[1][0] = false;

	if (m_ABSew == ASew)
	{
		ball1pos = GPVec3ToOgre(m_ballnode0->m_CurrPosition);
		ball2pos = GPVec3ToOgre(m_ballnode1->m_CurrPosition);
		m_AInCount++;
	}
	else if (m_ABSew == BSew)
	{
		ball1pos = GPVec3ToOgre(m_ballnode2->m_CurrPosition);
		ball2pos = GPVec3ToOgre(m_ballnode3->m_CurrPosition);
		m_BInCount++;
	}

	if (ball1pos.distance(GPVec3ToOgre(anchorpos)) < BALLPOSEXTENT)
	{
		m_BaseBall[0][0] = m_BaseBall[0][1] = true;
	}
	else if (ball2pos.distance(GPVec3ToOgre(anchorpos)) < BALLPOSEXTENT)
	{
		m_BaseBall[1][0] = m_BaseBall[1][1] = true;
	}
	else//显示球
	{
		// 		Ogre::Entity* needleinobj = (Ogre::Entity*)m_pNeedleInBall->getAttachedObject("needleinball$1");
		// 		if (needleinobj)
		// 		{
		// 			needleinobj->setVisible(true);
		// 			needleinobj->setMaterialName("green");
		// 		}
		if ((!m_MarkState[4].visible) || (m_MarkState[4].matname != "MarkTextureMaterialGreen"))
		{
			m_bMarkState = true;
		}
		//m_MarkState[4].visible = true;
		//m_MarkState[4].matname = "MarkTextureMaterialGreen";
//
		m_NeedleInPositionTip = true;

		if (m_ABSew == ASew)
		{
			if (m_RetreatNeedleNum[0][0] >= 2)
			{
				m_NeedleInRepeatErrorTip = true;
			}
		}
		else if (m_ABSew == BSew)
		{
			if (m_RetreatNeedleNum[1][0] >= 2)
			{
				m_NeedleInRepeatErrorTip = true;
			}
		}
	}
	m_InoutFace[0] = (GFPhysSoftBodyFace*)face;
	m_InoutWeights[0] = weights[0]; m_InoutWeights[1] = weights[1]; m_InoutWeights[2] = weights[2];
}
//======================================================================================================================
void CSutureTrainV2::OnCreateOutAnchor(const GFPhysSoftBodyFace* face, const Real weights[])
{
	GFPhysVector3 anchorpos = face->m_Nodes[0]->m_CurrPosition * weights[0]
		+ face->m_Nodes[1]->m_CurrPosition * weights[1]
		+ face->m_Nodes[2]->m_CurrPosition * weights[2];

	m_NeedleInDegree = false;
	m_NeedleInPositionTip = false;
	m_NeedleInRepeatErrorTip = false;

	Ogre::Vector3 ball1pos;
	Ogre::Vector3 ball2pos;
	if (m_ABSew == ASew)
	{
		ball1pos = GPVec3ToOgre(m_ballnode0->m_CurrPosition);
		ball2pos = GPVec3ToOgre(m_ballnode1->m_CurrPosition);
	}
	else if (m_ABSew == BSew)
	{
		ball1pos = GPVec3ToOgre(m_ballnode2->m_CurrPosition);
		ball2pos = GPVec3ToOgre(m_ballnode3->m_CurrPosition);
	}

	if ((ball1pos - GPVec3ToOgre(anchorpos)).length() < BALLPOSEXTENT)
	{
		m_BaseBall[0][0] = true;
		m_BaseBall[0][1] = false;
		m_PullNeedleOut = true;
	}
	else if ((ball2pos - GPVec3ToOgre(anchorpos)).length() < BALLPOSEXTENT)
	{
		m_BaseBall[1][0] = true;
		m_BaseBall[1][1] = false;
		m_PullNeedleOut = true;
	}
	else//显示球
	{
		// 		Ogre::Entity* needleoutobj = (Ogre::Entity*)m_pNeedleOutBall->getAttachedObject("needleoutball$1");
		// 		if (needleoutobj)
		// 		{
		// 			needleoutobj->setVisible(true);
		// 			needleoutobj->setMaterialName("green");
		// 		}
		if ((!m_MarkState[5].visible) || (m_MarkState[5].matname != "MarkTextureMaterialGreen"))
		{
			m_bMarkState = true;
		}
		//m_MarkState[5].visible = true;
		//m_MarkState[5].matname = "MarkTextureMaterialGreen";

		m_NeedleOutPositionTip = true;

		if (m_ABSew == ASew)
		{
			if (m_RetreatNeedleNum[0][1] >= 2)
			{
				m_NeedleOutRepeatErrorTip = true;
			}
		}
		else if (m_ABSew == BSew)
		{
			if (m_RetreatNeedleNum[1][1] >= 2)
			{
				m_NeedleOutRepeatErrorTip = true;
			}
		}
	}
	m_InoutFace[1] = (GFPhysSoftBodyFace*)face;
	m_InoutWeights[3] = weights[0]; m_InoutWeights[4] = weights[1]; m_InoutWeights[5] = weights[2];

	GFPhysVector3 inanchorpos = GFPhysVector3(1000, 1000, 1000);
	if (m_InoutFace[0])
	{
		inanchorpos = m_InoutFace[0]->m_Nodes[0]->m_CurrPosition * m_InoutWeights[0]
			+ m_InoutFace[0]->m_Nodes[1]->m_CurrPosition * m_InoutWeights[1]
			+ m_InoutFace[0]->m_Nodes[2]->m_CurrPosition * m_InoutWeights[2];
	}
	Real inball1pos = inanchorpos.Distance(OgreToGPVec3(ball1pos));
	Real inball2pos = inanchorpos.Distance(OgreToGPVec3(ball2pos));
	Real outball1pos = anchorpos.Distance(OgreToGPVec3(ball1pos));
	Real outball2pos = anchorpos.Distance(OgreToGPVec3(ball2pos));
	int inqiu;//1 2球
	//int outqiu;//1 2

	inqiu = ((inball1pos < inball2pos) ? 1 : 2);
	if (inqiu == 1)
	{
		inqiu = ((outball1pos < inball1pos) ? 2 : 1);
	}
	else if (inqiu == 2)
	{
		inqiu = ((outball2pos < inball2pos) ? 1 : 2);
	}
	if (inqiu == 1)
	{
		if (m_ABSew == ASew)
		{
			if (inball1pos < BALLPOSEXTENT)
			{
				m_AInOutPos[0] = 1;
			}
			else if (inball1pos < 1.3)
			{
				m_AInOutPos[0] = 2;
			}
			else
			{
				m_AInOutPos[0] = 3;
			}
			if (outball2pos < BALLPOSEXTENT)
			{
				m_AInOutPos[1] = 1;
			}
			else if (outball2pos < 1.3)
			{
				m_AInOutPos[1] = 2;
			}
			else
			{
				m_AInOutPos[1] = 3;
			}
		}
		else if (m_ABSew == BSew)
		{
			if (inball1pos < BALLPOSEXTENT)
			{
				m_BInOutPos[0] = 1;
			}
			else if (inball1pos < 1.3)
			{
				m_BInOutPos[0] = 2;
			}
			else
			{
				m_BInOutPos[0] = 3;
			}
			if (outball2pos < BALLPOSEXTENT)
			{
				m_BInOutPos[1] = 1;
			}
			else if (outball2pos < 1.3)
			{
				m_BInOutPos[1] = 2;
			}
			else
			{
				m_BInOutPos[1] = 3;
			}
		}
	}
	else
	{
		if (m_ABSew == ASew)
		{
			if (inball2pos < BALLPOSEXTENT)
			{
				m_AInOutPos[0] = 1;
			}
			else if (inball2pos < 1.3)
			{
				m_AInOutPos[0] = 2;
			}
			else
			{
				m_AInOutPos[0] = 3;
			}
			if (outball1pos < BALLPOSEXTENT)
			{
				m_AInOutPos[1] = 1;
			}
			else if (outball1pos < 1.3)
			{
				m_AInOutPos[1] = 2;
			}
			else
			{
				m_AInOutPos[1] = 3;
			}
		}
		else if (m_ABSew == BSew)
		{
			if (inball2pos < BALLPOSEXTENT)
			{
				m_BInOutPos[0] = 1;
			}
			else if (inball2pos < 1.3)
			{
				m_BInOutPos[0] = 2;
			}
			else
			{
				m_BInOutPos[0] = 3;
			}
			if (outball1pos < BALLPOSEXTENT)
			{
				m_BInOutPos[1] = 1;
			}
			else if (outball1pos < 1.3)
			{
				m_BInOutPos[1] = 2;
			}
			else
			{
				m_BInOutPos[1] = 3;
			}
		}
	}

}
//======================================================================================================================
void CSutureTrainV2::OnRemoveInAnchor(const GFPhysSoftBodyFace* face, const Real weights[])
{
	GFPhysVector3 pos = face->m_Nodes[0]->m_CurrPosition * weights[0]
		+ face->m_Nodes[1]->m_CurrPosition * weights[1]
		+ face->m_Nodes[2]->m_CurrPosition * weights[2];

	m_NeedleInDegree = false;
	m_NeedleInPositionTip = false;
	m_NeedleInRepeatErrorTip = false;

	//m_pNeedleInBall->setVisible(false);
	if (m_MarkState[4].visible)
	{
		m_bMarkState = true;
	}
	m_MarkState[4].visible = false;

	if (m_ABSew == ASew)
	{
		m_RetreatNeedleNum[0][0] ++;
	}
	else if (m_ABSew == BSew)
	{
		m_RetreatNeedleNum[1][0] ++;
	}
}
//======================================================================================================================
void CSutureTrainV2::OnRemoveOutAnchor(const GFPhysSoftBodyFace* face, const Real weights[])
{
	GFPhysVector3 pos = face->m_Nodes[0]->m_CurrPosition * weights[0]
		+ face->m_Nodes[1]->m_CurrPosition * weights[1]
		+ face->m_Nodes[2]->m_CurrPosition * weights[2];

	m_NeedleOutPositionTip = false;
	m_PullNeedleOut = false;
	m_NeedleOutRepeatErrorTip = false;

	if (m_BaseBall[0][0] && (m_BaseBall[0][1] == false))
	{
		m_BaseBall[0][0] = false;
	}
	else if (m_BaseBall[1][0] && (m_BaseBall[1][1] == false))
	{
		m_BaseBall[1][0] = false;
	}
	//m_pNeedleOutBall->setVisible(false);
	if (m_MarkState[5].visible)
	{
		m_bMarkState = true;
	}
	m_MarkState[5].visible = false;

	if (m_ABSew == ASew)
	{
		m_RetreatNeedleNum[0][1] ++;
		m_AInOutPos[0] = m_AInOutPos[1] = 3;//只一个点时候，无法判断是入针点还是出针点
	}
	else if (m_ABSew == BSew)
	{
		m_RetreatNeedleNum[1][1] ++;
		m_BInOutPos[0] = m_BInOutPos[1] = 3;
	}
}
//======================================================================================================================
void CSutureTrainV2::OnSwitchAnchor(const GFPhysSoftBodyFace* face, const Real weights[])
{
	/*GFPhysVector3 pos = face->m_Nodes[0]->m_UnDeformedPos * weights[0]
	+ face->m_Nodes[1]->m_UnDeformedPos * weights[1]
	+ face->m_Nodes[2]->m_UnDeformedPos * weights[2];*/
	/*Ogre::LogManager::getSingleton().logMessage(Ogre::String("the pos of switch anchor is  ")
	+ Ogre::StringConverter::toString(pos.x()) + ","
	+ Ogre::StringConverter::toString(pos.y()) + ","
	+ Ogre::StringConverter::toString(pos.z()) + ".");*/
}
//======================================================================================================================
void CSutureTrainV2::OnMovingRopeAnchor(const SutureThreadV2 * thread, const int index, const Real weights[])
{
}
//======================================================================================================================
void CSutureTrainV2::OnRemoveRopeAnchor(const GFPhysSoftBodyFace* face, const Real weights[])
{
	GFPhysVector3 anchorpos = face->m_Nodes[0]->m_CurrPosition * weights[0]
		+ face->m_Nodes[1]->m_CurrPosition * weights[1]
		+ face->m_Nodes[2]->m_CurrPosition * weights[2];

	m_NeedleInDegree = false;
	m_NeedleOutPositionTip = false;
	m_PullNeedleOut = false;
	m_NeedleOutRepeatErrorTip = false;

	static int roperemoveall = 0;
	if (m_BaseBall[0][0] == true && m_BaseBall[1][0] == true)
	{
		if (m_ABSew == ASew)
		{
			Ogre::Vector3 ball1pos = GPVec3ToOgre(m_ballnode0->m_CurrPosition);
			Ogre::Vector3 ball2pos = GPVec3ToOgre(m_ballnode1->m_CurrPosition);
			if ((ball1pos - GPVec3ToOgre(anchorpos)).length() < (ball2pos - GPVec3ToOgre(anchorpos)).length())
			{
				// 				Ogre::Entity* inEntity = (Ogre::Entity*)m_pBaseBallPos1->getAttachedObject(0);
				// 				inEntity->setMaterialName("orange");
				if (m_MarkState[0].matname != "MarkTextureMaterialOrange")
				{
					m_bMarkState = true;
				}
				m_MarkState[0].matname = "MarkTextureMaterialOrange";
				roperemoveall++;
			}
			else
			{
				//Ogre::Entity* outEntity = (Ogre::Entity*)m_pBaseBallPos2->getAttachedObject(0);
				//outEntity->setMaterialName("orange");
				if (m_MarkState[1].matname != "MarkTextureMaterialOrange")
				{
					m_bMarkState = true;
				}
				m_MarkState[1].matname = "MarkTextureMaterialOrange";
				roperemoveall++;
			}

			if (roperemoveall == 2)
			{
				roperemoveall = 0;
				m_ABSew = BSew;

				if (m_MarkState[0].matname != "MarkTextureMaterialOrange")
				{
					m_bMarkState = true;
				}
				m_MarkState[0].matname = "MarkTextureMaterialOrange";
				if (m_MarkState[1].matname != "MarkTextureMaterialOrange")
				{
					m_bMarkState = true;
				}
				m_MarkState[1].matname = "MarkTextureMaterialOrange";
				if (m_MarkState[2].matname != "MarkTextureMaterialGreen")
				{
					m_bMarkState = true;
				}
				m_MarkState[2].matname = "MarkTextureMaterialGreen";
				if (m_MarkState[3].matname != "MarkTextureMaterialGreen")
				{
					m_bMarkState = true;
				}
				m_MarkState[3].matname = "MarkTextureMaterialGreen";

				m_FirstPassFinish = true;
			}
		}
		else if (m_ABSew == BSew)
		{
			Ogre::Vector3 ball1pos = GPVec3ToOgre(m_ballnode2->m_CurrPosition);
			Ogre::Vector3 ball2pos = GPVec3ToOgre(m_ballnode3->m_CurrPosition);
			if ((ball1pos - GPVec3ToOgre(anchorpos)).length() < (ball2pos - GPVec3ToOgre(anchorpos)).length())
			{
				// 				Ogre::Entity* inEntity = (Ogre::Entity*)m_pBaseBallPos3->getAttachedObject(0);
				// 				inEntity->setMaterialName("orange");
				if (m_MarkState[2].matname != "MarkTextureMaterialOrange")
				{
					m_bMarkState = true;
				}
				m_MarkState[2].matname = "MarkTextureMaterialOrange";
			}
			else
			{
				// 				Ogre::Entity* outEntity = (Ogre::Entity*)m_pBaseBallPos4->getAttachedObject(0);
				// 				outEntity->setMaterialName("orange");
				if (m_MarkState[3].matname != "MarkTextureMaterialOrange")
				{
					m_bMarkState = true;
				}
				m_MarkState[3].matname = "MarkTextureMaterialOrange";
			}
			m_ABSew = SewDone;
			__super::TrainingFinish();
			if (m_ScoreSys)
			{
				m_ScoreSys->SetTrainSucced();
			}
		}
	}
	else
	{
		GFPhysVector3 inpos = GFPhysVector3(1000, 1000, 1000);
		if (m_InoutFace[0])
		{
			inpos = m_InoutFace[0]->m_Nodes[0]->m_CurrPosition * m_InoutWeights[0]
				+ m_InoutFace[0]->m_Nodes[1]->m_CurrPosition * m_InoutWeights[1]
				+ m_InoutFace[0]->m_Nodes[2]->m_CurrPosition * m_InoutWeights[2];
		}
		GFPhysVector3 outpos = GFPhysVector3(1000, 1000, 1000);
		if (m_InoutFace[1])
		{
			outpos = m_InoutFace[1]->m_Nodes[0]->m_CurrPosition * m_InoutWeights[3]
				+ m_InoutFace[1]->m_Nodes[1]->m_CurrPosition * m_InoutWeights[4]
				+ m_InoutFace[1]->m_Nodes[2]->m_CurrPosition * m_InoutWeights[5];
		}

		if ((inpos - anchorpos).Length2() < (outpos - anchorpos).Length2())
		{
			//m_pNeedleInBall->setVisible(false);
			if (m_MarkState[4].visible)
			{
				m_bMarkState = true;
			}
			m_MarkState[4].visible = false;
		}
		else
		{
			if (m_MarkState[5].visible)
			{
				m_bMarkState = true;
			}
			m_MarkState[5].visible = false;
		}
	}
}
//======================================================================================================================
void CSutureTrainV2::OnSaveTrainingReport()
{
	float scaleParam = 0;
	MxOperateItem* operateItem = nullptr;
	float usedtime = GetElapsedTime();
	AddOperateItem("UsedTime", usedtime, false, AM_ReplaceAll, &operateItem);
	scaleParam = (usedtime > 900) ? 0 : ((m_ABSew == SewDone) ? 1 : 0);
	operateItem->ScaleScoreValueOfLastScoreItem(1.0f, scaleParam);

	AddOperateItem("FirstHold_Success", float(m_FirstHoldSuccess), false, AM_ReplaceAll, &operateItem);
	operateItem->ScaleScoreValueOfLastScoreItem(m_FirstHoldSuccess, 1.0f);

	AddOperateItem("HoldNeedle_Times", float(m_TotalHoldCount));

	AddOperateItem("DropNeedle_Times", float(m_NeedleLostCount), false, AM_ReplaceAll, &operateItem);
	operateItem->ScaleScoreValueOfLastScoreItem(float(m_NeedleLostCount), 1.0f);

	AddOperateItem("HoldNeedle_Position", float(m_HoldPosition), false, AM_ReplaceAll, &operateItem);
	operateItem->ScaleScoreValueOfLastScoreItem(m_HoldPosition, 1.0f);

	int holdNeedleAngle = (m_HolderAngle >= 85 && m_HolderAngle <= 90) ? (scaleParam = 1.0, 0) : ((m_HolderAngle >= 60 && m_HolderAngle < 85) ? (scaleParam = 0.8, 1) : ((m_HolderAngle < 0) ? (scaleParam = 0.0, 2) : (scaleParam = 0.2, 2)));
	AddOperateItem("HoldNeedle_Angle", holdNeedleAngle, false, AM_ReplaceAll, &operateItem);
	operateItem->ScaleScoreValueOfLastScoreItem(scaleParam, 1.0f);

	AddOperateItem("ASew_InPosition", float(m_AInOutPos[0] - 1), false, AM_ReplaceAll, &operateItem);
	scaleParam = (m_AInOutPos[0] == 1) ? 1.0 : ((m_AInOutPos[0] == 2) ? 0.8 : ((m_AInOutPos[0] == 3) ? 0.2 : 0.0));
	operateItem->ScaleScoreValueOfLastScoreItem(scaleParam, 1.0f);

	AddOperateItem("ASew_InTimes", float(m_AInCount), false, AM_ReplaceAll, &operateItem);
	scaleParam = (m_AInCount > 3) ? (m_AInCount - 3) : 0;
	operateItem->ScaleScoreValueOfLastScoreItem(scaleParam, 1.0f);

	int inOrganAngle = float((m_InOrganAngle[0] >= 85 && m_InOrganAngle[0] <= 90) ? (scaleParam = 1.0, 0) : ((m_InOrganAngle[0] >= 60 && m_InOrganAngle[0] < 85) ? (scaleParam = 0.8, 1) : ((m_InOrganAngle[0] < 0) ? (scaleParam = 0, 2) : (scaleParam = 0.2, 2))));
	AddOperateItem("ASew_InAngle", float(inOrganAngle), false, AM_ReplaceAll, &operateItem);
	operateItem->ScaleScoreValueOfLastScoreItem(scaleParam, 1.0f);

	AddOperateItem("ASew_RetreatTimes", float(m_RetreatNeedleNum[0][1]), false, AM_ReplaceAll, &operateItem);
	scaleParam = (m_RetreatNeedleNum[0][1] > 3) ? (m_RetreatNeedleNum[0][1] - 3) : 0;
	operateItem->ScaleScoreValueOfLastScoreItem(scaleParam, 1.0f);

	AddOperateItem("ASew_OutPosition", float(m_AInOutPos[1] - 1), false, AM_ReplaceAll, &operateItem);
	scaleParam = (m_AInOutPos[1] == 1) ? 1.0 : ((m_AInOutPos[1] == 2) ? 0.8 : ((m_AInOutPos[1] == 3) ? 0.2 : 0.0));
	operateItem->ScaleScoreValueOfLastScoreItem(scaleParam, 1.0f);

	bool Aright = (m_BaseBall[0][0] && m_BaseBall[1][0]) ? 1 : 0;
	AddOperateItem("ASew_Right", float(Aright), false, AM_ReplaceAll, &operateItem);
	operateItem->ScaleScoreValueOfLastScoreItem(Aright, 1.0f);


	AddOperateItem("BSew_InPosition", float(m_BInOutPos[0] - 1), false, AM_ReplaceAll, &operateItem);
	scaleParam = (m_BInOutPos[0] == 1) ? 1.0 : ((m_BInOutPos[0] == 2) ? 0.8 : ((m_BInOutPos[0] == 3) ? 0.2 : 0.0));
	operateItem->ScaleScoreValueOfLastScoreItem(scaleParam, 1.0f);

	AddOperateItem("BSew_InTimes", float(m_BInCount), false, AM_ReplaceAll, &operateItem);
	scaleParam = (m_BInCount > 3) ? (m_BInCount - 3) : 0;
	operateItem->ScaleScoreValueOfLastScoreItem(scaleParam, 1.0f);

	inOrganAngle = float((m_InOrganAngle[1] >= 85 && m_InOrganAngle[1] <= 90) ? (scaleParam = 1.0, 0) : ((m_InOrganAngle[1] >= 60 && m_InOrganAngle[1] < 85) ? (scaleParam = 0.8, 1) : ((m_InOrganAngle[1] < 0) ? (scaleParam = 0, 2) : (scaleParam = 0.2, 2))));
	AddOperateItem("BSew_InAngle", float(inOrganAngle), false, AM_ReplaceAll, &operateItem);
	operateItem->ScaleScoreValueOfLastScoreItem(scaleParam, 1.0f);

	AddOperateItem("BSew_RetreatTimes", float(m_RetreatNeedleNum[1][1]), false, AM_ReplaceAll, &operateItem);
	scaleParam = (m_RetreatNeedleNum[1][1] > 3) ? (m_RetreatNeedleNum[1][1] - 3) : 0;
	operateItem->ScaleScoreValueOfLastScoreItem(scaleParam, 1.0f);

	AddOperateItem("BSew_OutPosition", float(m_BInOutPos[1] - 1), false, AM_ReplaceAll, &operateItem);
	scaleParam = (m_BInOutPos[1] == 1) ? 1.0 : ((m_BInOutPos[1] == 2) ? 0.8 : ((m_BInOutPos[1] == 3) ? 0.2 : 0.0));
	operateItem->ScaleScoreValueOfLastScoreItem(scaleParam, 1.0f);

	bool Bright = (m_BaseBall[0][1] && m_BaseBall[1][1]) ? 1 : 0;
	AddOperateItem("BSew_Right", float(Bright && Aright), false, AM_ReplaceAll, &operateItem);
	operateItem->ScaleScoreValueOfLastScoreItem(Bright && Aright, 1.0f);

	int fastesttimes = m_pToolsMgr->GetRightToolFastestSpeedTimes() + m_pToolsMgr->GetLeftToolFastestSpeedTimes();
	AddOperateItem("ToolMoveFasterTimes", float(fastesttimes), false, AM_ReplaceAll, &operateItem);
	scaleParam = (fastesttimes > 3) ? (fastesttimes - 3) : 0;
	operateItem->ScaleScoreValueOfLastScoreItem(scaleParam, 1.0f);

	bool closeinst = m_pToolsMgr->ToolIsClosedInsertion() && m_bTrainingIlluminated;
	AddOperateItem("ToolIsClosedInsertion", float(closeinst), false, AM_ReplaceAll, &operateItem);
	operateItem->ScaleScoreValueOfLastScoreItem(closeinst, 1.0f);

	__super::OnSaveTrainingReport();
}
//地和天花板都是一个base_terrain
void CSutureTrainV2::OnCollisionKeep(GFPhysCollideObject * objA, GFPhysCollideObject * objB, const GFPhysManifoldPoint * contactPoints, int NumContactPoints)
{
	__super::OnCollisionKeep(objA, objB, contactPoints, NumContactPoints);

	GFPhysRigidBody * ra = GFPhysRigidBody::Upcast(objA);
	GFPhysRigidBody * rb = GFPhysRigidBody::Upcast(objB);

	GFPhysRigidBody * needleRigid = m_Needle->GetPhysicBody();

	if ((ra == needleRigid && rb == m_pBaseTerrainRigidBody) || (ra == m_pBaseTerrainRigidBody && rb == needleRigid))
	{
		if (m_Needle->m_CenterOfMass.y() < 10 && !m_NeedleHoldState)
		{
			m_NeedleLostState = true;
		}
	}
}
//======================================================================================================================
void CSutureTrainV2::DetectNeedleLost()
{
	if (!m_NeedleLostStateLast && m_NeedleLostState && !m_NeedleHoldState)///只有发生了碰撞才进
	{
		m_NeedleLostCount++;
		m_NeedleLostStateLast = true;
	}
}
//======================================================================================================================
void CSutureTrainV2::DetectNeedleLostCreate()
{
	if (m_NeedleLostState)
	{
		GFPhysVector3 dipanCenter(BASECENTERX, BASECENTERY, BASECENTERZ);
		GFPhysVector3 needleCenter = m_Needle->m_CenterOfMass;
		needleCenter.SetY(0);
		Real dist = needleCenter.Distance(dipanCenter);
		if (dist > BASERADIUS && !m_NeedleHoldState)
		{
			RemoveNeedleFromWorld(m_Needle);
			m_Needle = CreateNeedleV2(MXOgre_SCENEMANAGER, m_SutureNodeNum, m_SutureRsLength, m_NeedleSkeletonFile);
			m_Needle->addNeedleActionListener(this);
			m_GraspNeedleAgain = true;
			m_NeedleLostStateLast = false;
			m_NeedleLostState = false;
		}
	}
}
//======================================================================================================================
void CSutureTrainV2::DrawMarkRenderTarget()
{
	if (m_bMarkState)
	{
		m_pNeedleTestOrgan->GetEffectRender().clearMarkQuadObject();
		for (int c = 0; c < 4; c++)
		{
			m_pNeedleTestOrgan->GetEffectRender().MarkEffectTexture(m_MarkState[c].uvpos, m_MarkState[c].radius, m_MarkState[c].matname);
		}
		if (m_MarkState[4].visible && (m_MarkState[4].uvpos != Ogre::Vector2::ZERO))
		{
			m_pNeedleTestOrgan->GetEffectRender().MarkEffectTexture(m_MarkState[4].uvpos, m_MarkState[4].radius, m_MarkState[4].matname);
		}
		if (m_MarkState[5].visible && (m_MarkState[5].uvpos != Ogre::Vector2::ZERO))
		{
			m_pNeedleTestOrgan->GetEffectRender().MarkEffectTexture(m_MarkState[5].uvpos, m_MarkState[5].radius, m_MarkState[5].matname);
		}
		m_pNeedleTestOrgan->GetEffectRender().MarkTextureFlush();
		m_pNeedleTestOrgan->GetEffectRender().clearMarkQuadObject();
		m_bMarkState = false;
	}
}
//======================================================================================================================
void CSutureTrainV2::KeyPress(QKeyEvent * event)
{
	MisNewTraining::KeyPress(event);
}
//======================================================================================================================
void CSutureTrainV2::OnSimpleUIEvent(const SimpleUIEvent & event)
{
	if (event.m_EventAction == SimpleUIEvent::eEA_MouseReleased)
	{
		Ogre::OverlayElement *pElement = event.m_pEventElement;
		if (pElement->getName() == "SutureOverlayElement/ResetButton")
		{
			RemoveNeedleFromWorld(m_Needle);
			m_Needle = CreateNeedleV2(MXOgre_SCENEMANAGER, m_SutureNodeNum, m_SutureRsLength, m_NeedleSkeletonFile);
			m_Needle->addNeedleActionListener(this);
			m_GraspNeedleAgain = true;
			m_NeedleLostStateLast = false;
			m_NeedleLostState = false;

			m_MarkState[4].visible = false;
			m_MarkState[5].visible = false;
			m_bMarkState = true;
			/*InitBallPos();
			m_InoutFace[0] = m_InoutFace[1] = 0;
			m_bMarkState = true;*/
		}
	}
}
//======================================================================================================================