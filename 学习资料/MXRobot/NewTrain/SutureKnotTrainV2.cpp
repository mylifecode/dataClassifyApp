#include "SutureKnotTrainV2.h"
#include "MisMedicRigidPrimtive.h"
#include "MXOgreGraphic.h"
#include "Instruments\Tool.h"
#include "Instruments\MisCTool_PluginRigidHold.h"
#include "Instruments\MisCTool_PluginClamp.h"
#include "Instruments\NeedleHolder.h"
#include "MisMedicSutureKnotV2.h"
#include "qevent.h"
#include "SutureThreadV2.h"

#define BALLPOSEXTENT 0.6f
#define ANGLECORRECT 85.0f
#define ANGLEBIAS 65.0f

#define BASERADIUS 8.873f

#define BASECENTERX 0.0f
#define BASECENTERY 0.0f
#define BASECENTERZ -2.232f
//=============================================================================================
CSutureKnotV2Train::CSutureKnotV2Train(void)
{
	for (int i = 0; i < 2; i++)
	{
		m_Needle[i] = 0;
		m_Knot[i] = 0;
	}

	m_pBaseTerrainRigidBody = 0;
	m_pKnotTestOrgan0 = 0;
	m_pKnotTestOrgan1 = 0;
}
//=============================================================================================
CSutureKnotV2Train::~CSutureKnotV2Train(void)
{
	for (int i = 0; i < 2; i++)
	{
		if (m_Knot[i])
		{
			delete m_Knot[i];
			m_Knot[i] = 0;
		}
	}
}
//======================================================================================================================
bool CSutureKnotV2Train::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
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

			if (cs.m_name == "Box001")
			{
				organmesh->m_physbody->EnableDoubleFaceCollision();//开启双面碰撞
				//organmesh->m_physbody->SetCollisionCategory(1);//设置分组
				m_pKnotTestOrgan0 = organmesh;
				int collisiontag0 = m_pKnotTestOrgan0->m_physbody->GetCollisionFlags();
			}

			if (cs.m_name == "Box002")
			{
				organmesh->m_physbody->EnableDoubleFaceCollision();//开启双面碰撞
				//organmesh->m_physbody->SetCollisionCategory(1);//设置分组
				m_pKnotTestOrgan1 = organmesh;
				int collisiontag1 = m_pKnotTestOrgan1->m_physbody->GetCollisionFlags();
			}

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

	m_Needle[0] = CreateNeedleV2(MXOgre_SCENEMANAGER, m_SutureNodeNum, m_SutureRsLength, m_NeedleSkeletonFile);
	m_Needle[0]->addNeedleActionListener(this);
	SutureThreadV2* attachrope = m_Needle[0]->GetSutureThread();
	if (attachrope)
	{
		m_Knot[0] = new SutureKnotV2();
		m_Knot[0]->SetThread(attachrope);
		attachrope->m_KnotsInThread = m_Knot[0];
		//attachrope->m_NeedleAttchedThread = m_Needle;

#if 0
		m_Knot[0]->m_bHasKnot = true;

		if (0)
		{
			int min = 11;
			int max = 19;
			KnotInSutureRopeV2 deadknot = KnotInSutureRopeV2(knotpbdV2(attachrope, min - 2, max), knotpbdV2(attachrope, min, max + 2));
			deadknot.m_Angle = GP_2PI;

			m_Knot[0]->m_deadKnots.push_back(deadknot);

			min = 7;
			max = 23;
			m_Knot[0]->m_currentKnot = KnotInSutureRopeV2(knotpbdV2(attachrope, min - 2, max), knotpbdV2(attachrope, min, max + 2));
			m_Knot[0]->m_currentKnot.m_Angle = GP_2PI;

		}
		else
		{
			int min = 11;
			int max = 40;
			m_Knot[0]->m_currentKnot = KnotInSutureRopeV2(knotpbdV2(attachrope, min - 2, max), knotpbdV2(attachrope, min, max + 2));
			m_Knot[0]->m_currentKnot.m_Angle = GP_2PI;
		}


#endif
	}
	m_Needle[1] = NULL;
	m_Knot[1] = NULL;

	InitBallPos();
	m_bCircleKnot = false;
	m_CircleThreshold = 1.5f;
	m_bAdjust_Tail[0] = false;
	m_bAdjust_Tail[1] = false;
	m_ThreadSeperateCount = 0;
	m_ThreadOrganConnect = true;
	m_threadorganconnectLast = false;
	return result;
}
//======================================================================================================================
void CSutureKnotV2Train::InitBallPos()
{

}
//======================================================================================================================
bool CSutureKnotV2Train::Update(float dt)
{
	bool result = MisNewTraining::Update(dt);

	CountThreadSeperate();
	CheckAdjust_Tail();
	return result;
}
//======================================================================================================================

void CSutureKnotV2Train::InternalSimulateStart(int currStep, int TotalStep, Real dt)
{
	MisNewTraining::InternalSimulateStart(currStep, TotalStep, dt);
}

//======================================================================================================================

void CSutureKnotV2Train::InternalSimulateEnd(int currStep, int TotalStep, Real dt)
{
	MisNewTraining::InternalSimulateEnd(currStep, TotalStep, dt);
	GFPhysVectorObj<KnotInSutureRopeV2> Knots;
	for (int i = 0; i < 2; i++)
	{
		if (m_Needle[i] && m_Needle[i]->GetSutureThread())
		{
			if (m_Needle[i]->GetSutureThread()->GetCollidePairsWithRigid().size() == 0)
			{
				m_bCircleKnot = false;
			}
			m_Needle[i]->GetSutureThread()->m_KnotsInThread->GetDeadKnots(Knots);
			if (Knots.size() == 2)
			{
				continue;
			}
			else
			{
				if (checkCircleOnTool(m_Needle[i], m_CircleThreshold))
				{
					m_bCircleKnot = true;
				}
			}
			Knots.clear();
		}
	}

	for (int i = 0; i < 2; i++)
	{
		if (m_Knot[i])
		{
			m_Knot[i]->Update(dt, m_bCircleKnot);
		}
	}
}
//======================================================================================================================

void CSutureKnotV2Train::OnCreateInAnchor(const GFPhysSoftBodyFace* face, const Real weights[])
{

}
//======================================================================================================================

void CSutureKnotV2Train::OnCreateOutAnchor(const GFPhysSoftBodyFace* face, const Real weights[])
{

}
//======================================================================================================================

void CSutureKnotV2Train::OnRemoveInAnchor(const GFPhysSoftBodyFace* face, const Real weights[])
{

}
//======================================================================================================================

void CSutureKnotV2Train::OnRemoveOutAnchor(const GFPhysSoftBodyFace* face, const Real weights[])
{

}
//======================================================================================================================

void CSutureKnotV2Train::OnSwitchAnchor(const GFPhysSoftBodyFace* face, const Real weights[])
{

}
//======================================================================================================================

void CSutureKnotV2Train::OnRemoveRopeAnchor(const GFPhysSoftBodyFace* face, const Real weights[])
{

}
//======================================================================================================================

void CSutureKnotV2Train::OnMovingRopeAnchor(const SutureThreadV2 * thread, const int index, const Real weights[])
{

}
//======================================================================================================================

GFPhysVector3 CSutureKnotV2Train::CalcNormalFromCenter(SutureNeedleV2* needle, const STVRGCollidePair & pair, const GFPhysVector3 & Pos, const GFPhysVector3 & dir, GFPhysVector3 & foot)
{
	GFPhysVector3 target = GFPhysVector3(0.0f, 0.0f, 0.0f);
	GFPhysVector3 ponrigid = pair.m_RigidWorldPoint;

	if (GPCalcPerpendicularFootOntoLine(ponrigid, dir, Pos, target))
	{
		foot = target;
		GFPhysSoftTubeNode& refNode0 = needle->GetSutureThread()->GetThreadNodeRef(pair.m_Segment);
		GFPhysSoftTubeNode& refNode1 = needle->GetSutureThread()->GetThreadNodeRef(pair.m_Segment + 1);

		GFPhysVector3 ponline = refNode0.m_CurrPosition * (1.0f - pair.m_SegWeight) + refNode1.m_CurrPosition * pair.m_SegWeight;

		GFPhysVector3 normalfromCenter = ponline - target;
		return normalfromCenter.Normalized();
	}
	else
		foot = GFPhysVector3(0.0f, 0.0f, 0.0f);
	return GFPhysVector3(0.0f, 0.0f, 0.0f);
}
//======================================================================================================================

bool CSutureKnotV2Train::checkCircleOnTool(SutureNeedleV2* needle, Real threshold)
{
	if (m_pToolsMgr->GetRightTool())
	{
		CTool* rightTool = dynamic_cast<CTool*>(m_pToolsMgr->GetRightTool());
		if (rightTool->m_centertoolpartconvex.m_rigidbody)
		{
			Ogre::Vector3 v3OriginalPos = rightTool->GetKernelNode()->_getDerivedPosition();
			Ogre::Quaternion quatOriginal = rightTool->GetKernelNode()->_getDerivedOrientation();

			Ogre::Vector3 rotate = quatOriginal * rightTool->m_centertoolpartconvex.m_CollideShapeRelRot * Ogre::Vector3(0, 0, 1);


			GFPhysVector3 GFphysv3OriginalPos = OgreToGPVec3(v3OriginalPos);
			GFPhysVector3 GFphysrotate = OgreToGPVec3(rotate);

			const GFPhysAlignedVectorObj<STVRGCollidePair> & TRPairs = needle->GetSutureThread()->GetCollidePairsWithRigid();

			GFPhysVector3 foot = GFPhysVector3(0, 0, 0);

			if (TRPairs.size() > 1)
			{
				GFPhysVectorObj<Real> Angle_tr;

				GFPhysVector3 normal_0 = CalcNormalFromCenter(needle, TRPairs[0], GFphysv3OriginalPos, GFphysrotate, foot);

				for (size_t t = 1, nt = TRPairs.size(); t < nt; t++)
				{
					GFPhysVector3 normal_t = CalcNormalFromCenter(needle, TRPairs[t], GFphysv3OriginalPos, GFphysrotate, foot);//由m_RigidWorldPoint得到rigid圆柱轴线上的点，做连线。

					Angle_tr.push_back(CalcAngleDiff(normal_0, normal_t));


					int b = -1;
					int e = -1;
					Real sumAngle = FindGreatestSumOfSubArray(Angle_tr, b, e);


					for (int s = b; s <= e; s++)
					{
						if (TRPairs[s].m_Rigid != rightTool->m_centertoolpartconvex.m_rigidbody)
						{
							return false;//必须绕在同一侧器械上，针上不算。
						}
					}

					Real ratio = fabsf(sumAngle) / GP_2PI;

					if (ratio >= threshold)
					{
						return true;
					}
					normal_0 = normal_t;
				}
			}
		}
	}

	if (m_pToolsMgr->GetLeftTool())
	{
		CTool* leftTool = dynamic_cast<CTool*>(m_pToolsMgr->GetLeftTool());
		if (leftTool->m_centertoolpartconvex.m_rigidbody)
		{
			Ogre::Vector3 v3OriginalPos = leftTool->GetKernelNode()->_getDerivedPosition();
			Ogre::Quaternion quatOriginal = leftTool->GetKernelNode()->_getDerivedOrientation();

			Ogre::Vector3 rotate = quatOriginal * leftTool->m_centertoolpartconvex.m_CollideShapeRelRot * Ogre::Vector3(0, 0, 1);


			GFPhysVector3 GFphysv3OriginalPos = OgreToGPVec3(v3OriginalPos);
			GFPhysVector3 GFphysrotate = OgreToGPVec3(rotate);


			const GFPhysAlignedVectorObj<STVRGCollidePair> & TRPairs = needle->GetSutureThread()->GetCollidePairsWithRigid();

			GFPhysVector3 foot = GFPhysVector3(0, 0, 0);

			if (TRPairs.size() > 1)
			{
				GFPhysVectorObj<Real> Angle_tr;

				GFPhysVector3 normal_0 = CalcNormalFromCenter(needle, TRPairs[0], GFphysv3OriginalPos, GFphysrotate, foot);

				for (size_t t = 1, nt = TRPairs.size(); t < nt; t++)
				{
					GFPhysVector3 normal_t = CalcNormalFromCenter(needle, TRPairs[t], GFphysv3OriginalPos, GFphysrotate, foot);//由m_RigidWorldPoint得到rigid圆柱轴线上的点，做连线。

					Angle_tr.push_back(CalcAngleDiff(normal_0, normal_t));

					int b = -1;
					int e = -1;
					Real sumAngle = FindGreatestSumOfSubArray(Angle_tr, b, e);

					for (int s = b; s <= e; s++)
					{
						if (TRPairs[s].m_Rigid != leftTool->m_centertoolpartconvex.m_rigidbody)
						{
							return false;//必须绕在同一侧器械上，针上不算。
						}
					}

					Real ratio = fabsf(sumAngle) / GP_2PI;

					if (ratio >= threshold)
					{
						return true;
					}
					normal_0 = normal_t;
				}
			}
		}
	}
	return false;
}
//======================================================================================================================

void CSutureKnotV2Train::OnRigidClampByTool(GFPhysRigidBody * rigid)
{

}

void CSutureKnotV2Train::OnThreadClampByTool(int ClampedSegGlobalIndex, CTool* toolobject)
{

}

void CSutureKnotV2Train::OnThreadReleaseByTool(int ClampedSegGlobalIndex, CTool* toolobject)
{

}

void CSutureKnotV2Train::OnRigidReleaseByTool(GFPhysRigidBody * rigid)
{

}
//======================================================================================================================

void CSutureKnotV2Train::OnSimpleUIEvent(const SimpleUIEvent & event)
{
	if (event.m_EventAction == SimpleUIEvent::eEA_MouseReleased)
	{
		Ogre::OverlayElement *pElement = event.m_pEventElement;
		if (pElement->getName() == "SutureOverlayElement/ResetButton")
		{
			ResetNeedle();
		}
	}
}
//======================================================================================================================

void CSutureKnotV2Train::CheckAdjust_Tail()
{
	for (int i = 0; i < 2; i++)
	{
		if (!m_bAdjust_Tail[i])
		{
			GFPhysVectorObj<GFPhysFaceRopeAnchorV2*> activeRopeAnchors;
			if (m_Needle[i])
			{
				m_Needle[i]->GetSutureThread()->GetFaceRopeAnchors(activeRopeAnchors);
				if (activeRopeAnchors.size() > 0)
				{
					int maxAnchorIndex = activeRopeAnchors[0]->GetSegIndex();
					int RopeSegNum = m_Needle[i]->GetSutureThread()->GetNumThreadNodes();
					if (maxAnchorIndex > (int)(RopeSegNum * 0.66f))
					{
						m_bAdjust_Tail[i] = true;
					}
				}
			}
		}
	}
}

//======================================================================================================================

void CSutureKnotV2Train::CountThreadSeperate()
{
	for (int i = 0; i < 2; i++)
	{
		GFPhysVectorObj<GFPhysFaceRopeAnchorV2*> activeRopeAnchors;
		if (m_Needle[i])
		{
			m_Needle[i]->GetSutureThread()->GetFaceRopeAnchors(activeRopeAnchors);
			if (activeRopeAnchors.size() == 0 && m_ThreadOrganConnect != m_threadorganconnectLast)
			{
				m_ThreadOrganConnect = false;
				m_threadorganconnectLast = true;
			}
			else if (activeRopeAnchors.size() > 0)
			{
				m_ThreadOrganConnect = true;
			}
		}
	}

	if (m_ThreadOrganConnect != m_threadorganconnectLast && m_ThreadOrganConnect == false)
	{
		m_ThreadSeperateCount++;
		m_threadorganconnectLast = false;
	}
}
//======================================================================================================================

void CSutureKnotV2Train::ResetNeedle()
{
	if (m_pToolsMgr)
	{
		m_pToolsMgr->AllFixedToolsRelease();
	}

	if (m_Needle[0] && m_Needle[1] == NULL)
	{
		delete m_Knot[0];
		RemoveNeedleFromWorld(m_Needle[0]);
		m_Needle[0] = CreateNeedleV2(MXOgre_SCENEMANAGER, m_SutureNodeNum, m_SutureRsLength, m_NeedleSkeletonFile);
		m_Needle[0]->addNeedleActionListener(this);

		SutureThreadV2* attachrope = m_Needle[0]->GetSutureThread();
		if (attachrope)
		{
			m_Knot[0] = new SutureKnotV2();
			m_Knot[0]->SetThread(attachrope);
			attachrope->m_KnotsInThread = m_Knot[0];
		}
	}


	if (m_Needle[0] && m_Needle[1])
	{
		delete m_Knot[1];
		RemoveNeedleFromWorld(m_Needle[1]);
		m_Needle[1] = CreateNeedleV2(MXOgre_SCENEMANAGER, m_SutureNodeNum, m_SutureRsLength, m_NeedleSkeletonFile);
		m_Needle[1]->addNeedleActionListener(this);

		SutureThreadV2* attachrope = m_Needle[1]->GetSutureThread();
		if (attachrope)
		{
			m_Knot[1] = new SutureKnotV2();
			m_Knot[1]->SetThread(attachrope);
			attachrope->m_KnotsInThread = m_Knot[1];
		}
	}
}

//======================================================================================================================


bool CSutureKnotV2Train1::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{
	bool result = CSutureKnotV2Train::Initialize(pTrainingConfig, pToolConfig);

	Embed();

	InitBallPos();
	m_bGraspNeedle = false;
	m_bFirstHalfKnotFinish = false;

	m_bFinished = false;
	m_bScoreKnot[0] = false;
	m_bScoreKnot[1] = false;
	return result;
}
//======================================================================================================================

void CSutureKnotV2Train1::InitBallPos()
{
	for (int i = 0; i < 2; i++)
	{
		m_MarkState[i].visible = true;
		m_MarkState[i].radius = 0.04f;
		m_MarkState[i].matname = "MarkTextureMaterialGreen";
	}
}
//======================================================================================================================

void CSutureKnotV2Train1::UpdateBaseBallPos()//更新定位球
{
	if (m_MarkState[0].visible)
	{
		MMO_Face originface = m_pNeedleTestOrgan->m_OriginFaces[1108];

		if (originface.m_HasError == false)
		{

			m_MarkState[0].uvpos = originface.GetTextureCoord(0) * 0.33f
				+ originface.GetTextureCoord(1) * 0.33f
				+ originface.GetTextureCoord(2) * 0.33f;
		}
	}

	if (m_MarkState[1].visible)
	{
		MMO_Face originface = m_pNeedleTestOrgan->m_OriginFaces[2451];

		if (originface.m_HasError == false)
		{
			m_MarkState[1].uvpos = originface.GetTextureCoord(0) * 0.33f
				+ originface.GetTextureCoord(1) * 0.33f
				+ originface.GetTextureCoord(2) * 0.33f;
		}
	}
}
//======================================================================================================================

void CSutureKnotV2Train1::DrawMarkRenderTarget()
{

	m_pNeedleTestOrgan->GetEffectRender().clearMarkQuadObject();

	if (m_MarkState[0].visible && (m_MarkState[0].uvpos != Ogre::Vector2::ZERO))
	{
		m_pNeedleTestOrgan->GetEffectRender().MarkEffectTexture(m_MarkState[0].uvpos, m_MarkState[0].radius, m_MarkState[0].matname);
	}
	if (m_MarkState[1].visible && (m_MarkState[1].uvpos != Ogre::Vector2::ZERO))
	{
		m_pNeedleTestOrgan->GetEffectRender().MarkEffectTexture(m_MarkState[1].uvpos, m_MarkState[1].radius, m_MarkState[1].matname);
	}
	m_pNeedleTestOrgan->GetEffectRender().MarkTextureFlush();
	m_pNeedleTestOrgan->GetEffectRender().clearMarkQuadObject();
}
//======================================================================================================================

bool CSutureKnotV2Train1::Update(float dt)
{
	bool result = CSutureKnotV2Train::Update(dt);
	UpdateBaseBallPos();
	DrawMarkRenderTarget();

	GFPhysVectorObj<GFPhysFaceRopeAnchorV2*> activeRopeAnchors;
	m_Needle[0]->GetSutureThread()->GetFaceRopeAnchors(activeRopeAnchors);
	if (activeRopeAnchors.size() == 0)
	{
		//TrainingFatalError("SutureThreadDeattach");
		CTipMgr::Instance()->ShowTip("SutureThreadDeattach");
	}

	SutureThreadV2* thread = m_Needle[0]->GetSutureThread();
	if (thread)
	{
		GFPhysVectorObj<KnotInSutureRopeV2> Knots;

		thread->m_KnotsInThread->GetDeadKnots(Knots);

		if (Knots.size() == 1)
		{
			if (Knots[0].m_Angle == GP_2PI)
			{
				m_bFirstHalfKnotFinish = true;
			}
			if (Knots[0].m_Angle == 2.0f * GP_2PI)
			{
				m_bFirstHalfKnotFinish = true;
			}
		}
		else if (Knots.size() == 2)
		{
			m_bFinished = true;
			__super::TrainingFinish();
		}
	}

	if (m_bTrainingIlluminated)
	{
		if (!m_bGraspNeedle && activeRopeAnchors.size() == 2 && !m_bFirstHalfKnotFinish)
		{
			CTipMgr::Instance()->ShowTip("AdjustThreadTail");
		}
		else if (m_bFirstHalfKnotFinish)
		{
			//////////////////////////////////////////////////////////////////////////
			//make sure the first knot is the type defined
			CTipMgr::Instance()->ShowTip("SecondHalfKnot");
		}
	}

	return result;
}
//======================================================================================================================

void CSutureKnotV2Train1::OnThreadClampByTool(int ClampedSegIndex, CTool* toolobject)
{
	CSutureKnotV2Train::OnThreadClampByTool(ClampedSegIndex, toolobject);
	//绕圈成功+抓住近线尾
	GFPhysVectorObj<GFPhysFaceRopeAnchorV2*> activeRopeAnchors;
	m_Needle[0]->GetSutureThread()->GetFaceRopeAnchors(activeRopeAnchors);
	if (activeRopeAnchors.size() > 0)
	{
		int maxAnchorIndex = activeRopeAnchors[0]->GetSegIndex();
		if (m_bCircleKnot && /*ClampedSegIndex > maxAnchorIndex &&*/ !m_bFirstHalfKnotFinish)//双手夹线打结时这个条件不满足
		{
			m_bScoreKnot[0] = true;
			CTipMgr::Instance()->ShowTip("PullThreadThroughCircle");

			SetNextTip("FirstHalfKnot", 3.0f);
		}

		if (m_bCircleKnot && /*ClampedSegIndex > maxAnchorIndex &&*/ m_bFirstHalfKnotFinish)
		{
			m_bScoreKnot[1] = true;

			CTipMgr::Instance()->ShowTip("PullThreadThroughCircle");

			SetNextTip("GetSquareKnot", 3.0f);
		}
	}
}
//======================================================================================================================

void CSutureKnotV2Train1::OnRigidClampByTool(GFPhysRigidBody * rigid)
{
	if (rigid == m_Needle[0]->GetPhysicBody())
	{
		m_bGraspNeedle = true;
		SetNextTip("TakeRound", 3.0f);
	}
}
//======================================================================================================================

void CSutureKnotV2Train1::OnRigidReleaseByTool(GFPhysRigidBody * rigid)
{
	if (rigid == m_Needle[0]->GetPhysicBody())
	{
		m_bGraspNeedle = false;
	}
}
//======================================================================================================================

void CSutureKnotV2Train1::OnThreadReleaseByTool(int ClampedSegGlobalIndex, CTool* toolobject)
{
	CSutureKnotV2Train::OnThreadReleaseByTool(ClampedSegGlobalIndex, toolobject);
	GFPhysVectorObj<GFPhysFaceRopeAnchorV2*> activeRopeAnchors;
	m_Needle[0]->GetSutureThread()->GetFaceRopeAnchors(activeRopeAnchors);
	if (activeRopeAnchors.size() > 0)
	{
		int maxAnchorIndex = activeRopeAnchors[0]->GetSegIndex();

		if (m_bCircleKnot /*&& ClampedSegIndex > maxAnchorIndex*/)
		{
			CTipMgr::Instance()->ShowTip("PullThreadThroughCircle");
		}
	}

}

//======================================================================================================================

void CSutureKnotV2Train1::ResetNeedle()
{
	__super::ResetNeedle();

	Embed();

	m_bFirstHalfKnotFinish = false;
	m_bGraspNeedle = false;
	m_bFinished = false;
	m_bScoreKnot[0] = false;
	m_bScoreKnot[1] = false;
	m_ThreadOrganConnect = true;
	m_threadorganconnectLast = false;
}
//======================================================================================================================

void CSutureKnotV2Train1::OnSaveTrainingReport()
{
	Real usedtime = GetElapsedTime();

	if (m_bAdjust_Tail[0])
	{
		COnLineGradeMgr::Instance()->SendGrade("Adjust_Tail_Length", 0, usedtime);
	}
	else
	{
		COnLineGradeMgr::Instance()->SendGrade("NoAdjust_Tail_Length", 0, usedtime);
	}

	if (m_bScoreKnot[0])
	{
		COnLineGradeMgr::Instance()->SendGrade("FirstHalfKnotRoundSuccess", 0, usedtime);
	}
	else
	{
		COnLineGradeMgr::Instance()->SendGrade("FirstHalfKnotRoundFail", 0, usedtime);
	}

	if (m_bScoreKnot[1])
	{
		COnLineGradeMgr::Instance()->SendGrade("SecondHalfKnotRoundSuccess", 0, usedtime);
	}
	else
	{
		COnLineGradeMgr::Instance()->SendGrade("SecondHalfKnotRoundFail", 0, usedtime);
	}

	GFPhysVectorObj<KnotInSutureRopeV2> Knots;

	m_Needle[0]->GetSutureThread()->m_KnotsInThread->GetDeadKnots(Knots);

	if (Knots.size() == 1)
	{
		if (Knots[0].m_Angle == GP_2PI)
		{
			COnLineGradeMgr::Instance()->SendGrade("FirstHalfKnotGotten", 0, usedtime);
			COnLineGradeMgr::Instance()->SendGrade("SecondHalfKnotFail", 0, usedtime);
		}
	}
	else if (Knots.size() == 2)
	{
		if (Knots[0].m_Angle == GP_2PI)
		{
			COnLineGradeMgr::Instance()->SendGrade("FirstHalfKnotGotten", 0, usedtime);
			COnLineGradeMgr::Instance()->SendGrade("SecondHalfKnotGotten", 0, usedtime);
			//打结正确or错误
			if (Knots[0].m_Clockwise == Knots[1].m_Clockwise)
			{
				COnLineGradeMgr::Instance()->SendGrade("TieCorrect", 0, usedtime);
			}
			else
			{
				COnLineGradeMgr::Instance()->SendGrade("TieError", 0, usedtime);
			}
		}

		if (Knots[0].m_Angle == 2.0f * GP_2PI)
		{
			COnLineGradeMgr::Instance()->SendGrade("FirstHalfKnotGotten", 0, usedtime);
			COnLineGradeMgr::Instance()->SendGrade("SecondHalfKnotGotten", 0, usedtime);
			//打结正确or错误
			if (Knots[0].m_Clockwise == Knots[1].m_Clockwise)
			{
				COnLineGradeMgr::Instance()->SendGrade("TieCorrect", 0, usedtime);
			}
			else
			{
				COnLineGradeMgr::Instance()->SendGrade("TieError", 0, usedtime);
			}
		}
	}
	else/* if (Knots.size() == 0)*/
	{
		COnLineGradeMgr::Instance()->SendGrade("FirstHalfKnotFail", 0, usedtime);
		COnLineGradeMgr::Instance()->SendGrade("SecondHalfKnotFail", 0, usedtime);
	}




	//线拉出组织的次数

	if (m_ThreadSeperateCount == 0)
	{
		COnLineGradeMgr::Instance()->SendGrade("ThreadSeperateCount0", 0, usedtime);
	}
	else  if (m_ThreadSeperateCount == 1)
	{
		COnLineGradeMgr::Instance()->SendGrade("ThreadSeperateCount1", 0, usedtime);
	}
	else if (m_ThreadSeperateCount >= 2)
	{
		COnLineGradeMgr::Instance()->SendGrade("ThreadSeperateCount2", 0, usedtime);
	}


	Real leftToolSpeed = m_pToolsMgr->GetLeftToolMovedSpeed();
	Real rightTooSpeed = m_pToolsMgr->GetRightToolMovedSpeed();
	Real ToolSpeed = std::max(leftToolSpeed, rightTooSpeed);

	if (ToolSpeed <= 5.0f && ToolSpeed > GP_EPSILON)
		COnLineGradeMgr::Instance()->SendGrade("MachineHandle_Normal", 0, usedtime);
	else if (ToolSpeed > 5.0f && ToolSpeed <= 10.0f)
		COnLineGradeMgr::Instance()->SendGrade("MachineHandle_Fast", 0, usedtime);
	else if (ToolSpeed > 10.0f)
		COnLineGradeMgr::Instance()->SendGrade("MachineHandle_TooFast", 0, usedtime);

	if (m_pToolsMgr->ToolIsClosedInsertion() && m_bTrainingIlluminated)
	{
		COnLineGradeMgr::Instance()->SendGrade("MachineHandle_Close", 0, usedtime);
	}
	else
	{
		COnLineGradeMgr::Instance()->SendGrade("MachineHandle_Open", 0, usedtime);
	}

	if (m_bFinished)
	{
		if (usedtime < 360)
		{
			COnLineGradeMgr::Instance()->SendGrade("Finished_In6M", 0, usedtime);
			if (usedtime <= 240)
			{
				COnLineGradeMgr::Instance()->SendGrade("Twohands_Cooperation", 0, usedtime);
			}
		}
		else if (usedtime >= 360)
		{
			COnLineGradeMgr::Instance()->SendGrade("Finished_In10M", 0, usedtime);
		}
	}
	else
	{
		COnLineGradeMgr::Instance()->SendGrade("UnFinish_In10M", 0, usedtime);
	}
	__super::OnSaveTrainingReport();
}
//======================================================================================================================

void CSutureKnotV2Train1::Embed()
{	
	SutureThreadV2* attachrope = m_Needle[0]->GetSutureThread();
	if (NULL == attachrope)
	{
		return;
	}
	if (attachrope->m_islock)
	{
		int num = attachrope->GetNumSegments();
		attachrope->ReleaseNodeAsFix(num-1);
		attachrope->m_islock = false;
	}

	GFPhysVectorObj<GFPhysFaceRopeAnchorV2::GFPhysFRAnchorConstructInFo> ropefaceanchors;
	//m_Needle[0]->GetSutureThread()->GetFaceRopeAnchors(ropefaceanchors);

	Real weights0[3] = { 0.333f, 0.333f, 0.333f };
	Real weights1[3] = { 0.333f, 0.333f, 0.333f };

	GFPhysSoftBodyFace* face0 = m_pNeedleTestOrgan->m_physbody->GetFaceAtIndex(1108);//GetFaceByUID
	GFPhysSoftBodyFace* face1 = m_pNeedleTestOrgan->m_physbody->GetFaceAtIndex(2451);

	int n0 = 18;
	int n1 = 14;

	Real threadweights0[2] = { 0.5f, 0.5f };
	Real threadweights1[2] = { 0.5f, 0.5f };

	GFPhysFaceRopeAnchorV2::GFPhysFRAnchorConstructInFo(face0, weights0, n0, threadweights0[0], STATE_IN);

	///GFPhysFaceRopeAnchorV2* anchor0 = new GFPhysFaceRopeAnchorV2(STATE_IN,face0, weights0, m_Needle[0]->GetSutureThread(), n0, threadweights0);
	//anchor0->m_FixFaceNormal = GFPhysVector3(0, -1, 0);

	//GFPhysFaceRopeAnchorV2* anchor1 = new GFPhysFaceRopeAnchorV2(STATE_OUT,face1, weights1, m_Needle[0]->GetSutureThread(), n1, threadweights1);
	//anchor1->m_FixFaceNormal = GFPhysVector3(0, 1, 0);

	ropefaceanchors.push_back(GFPhysFaceRopeAnchorV2::GFPhysFRAnchorConstructInFo(face0, weights0, n0, threadweights0[0], STATE_IN));
	ropefaceanchors.push_back(GFPhysFaceRopeAnchorV2::GFPhysFRAnchorConstructInFo(face1, weights1, n1, threadweights1[0], STATE_OUT));

	m_Needle[0]->GetSutureThread()->SetFaceRopeAnchors(ropefaceanchors);
}

//CSutureKnotV2Train1::~CSutureKnotV2Train1(void)
//{
//    CSutureKnotV2Train::~CSutureKnotV2Train();
//}
//======================================================================================================================

bool CSutureKnotV2Train2::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{
	bool result = CSutureKnotV2Train1::Initialize(pTrainingConfig, pToolConfig);

	//SutureThreadV2* rope = m_Needle[0]->GetSutureThread();
	//GFPhysSoftTubeNode& node0 = rope->GetThreadNodeRef(42);
	//node0.SetInvMassScale(0.0f);
	//node0.m_CurrPosition = GFPhysVector3(3.88f, 6.46f, -1.06f);

	//GFPhysSoftTubeNode& node1 = rope->GetThreadNodeRef(47);
	//node1.SetInvMassScale(0.0f);
	//node1.m_CurrPosition = GFPhysVector3(-0.68f, 3.09f, -6.22f);

	return result;
}
//======================================================================================================================

bool CSutureKnotV2Train2::Update(float dt)
{
	bool result = CSutureKnotV2Train1::Update(dt);

	KnotInSutureRopeV2 & Knot = m_Needle[0]->GetSutureThread()->m_KnotsInThread->GetCurrKnot();

	if (Knot.m_Angle == GP_2PI)
	{
		//CTipMgr::Instance()->ShowTip("TakeRoundError");
		//Knot.m_Angle == 2.0f * GP_2PI;
		//TrainingErrorWithoutQuit("TakeRoundError");
	}
	return result;
}
//======================================================================================================================
bool CSutureKnotV2Train3::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{
	bool result = CSutureKnotV2Train::Initialize(pTrainingConfig, pToolConfig);

	//   1--3  2--0
	//   5--7  6--4
	m_index[0] = 752;
	m_index[1] = 762;
	m_index[2] = 384;
	m_index[3] = 836;

	m_index[4] = 156;
	m_index[5] = 166;
	m_index[6] = 394;
	m_index[7] = 893;

	InitBallPos();

	m_bGraspNeedle = false;
	m_bFirstHalfKnotFinish = false;

	m_bFinished = false;
	for (int i = 0; i < 4; i++)
	{
		m_bScoreKnot[i] = false;
	}

	for (int i = 0; i < 2; i++)
	{
		m_HolderAngle[i] = 0.0f;
		m_InjectAngle[i] = 0.0f;
		m_WithDrawAngle[i] = 0.0f;

		HoldPos[i] = false;
		HoldAngle[i] = NO_VALUE;
		InjectPos[i] = NO_VALUE;
		InjectAngle[i] = NO_VALUE;
		WithDrawPos[i] = NO_VALUE;
		WithDrawAngle[i] = NO_VALUE;
	}
	m_NeedleFallStateLast = false;
	m_NeedleFallState = false;
	m_NeedleFallCount = 0;

	m_sutureV1 = NULL;
	m_sutureV2 = NULL;
	if (false)
	{
		m_sutureV1 = new SutureThreadV2(MXOgre_SCENEMANAGER, this);

		GFPhysVector3 StartFixPoint = GFPhysVector3(-4.730560362f, 5.14702272f, 1.55101287f);
		GFPhysVector3 EndFixPoint = GFPhysVector3(2.969433248f, 5.14702272f, 1.55101287f);
		int segmentCount = 10;
		Real masspernode = 0.05f;

		m_sutureV1->Create(StartFixPoint, EndFixPoint, segmentCount, masspernode, masspernode,false);

		GFPhysSoftTubeNode& node0 = m_sutureV1->GetThreadNodeRef(0);
		//node0.SetInvMass(0.0f);

		//SutureRopeNode& node5 = m_sutureV1->GetThreadNodeRef(5);
		//node5.m_CurrPosition = GFPhysVector3(4.969433248f, 5.14702272f, 1.55101287f);
		//node5.SetInvMass(0.0f);




		//////////////////////////////////////////////////////////////////////////
		m_sutureV2 = new SutureThreadV2(MXOgre_SCENEMANAGER, this);

		GFPhysVector3 StartFixPoint2 = GFPhysVector3(-4.730560362f, 6.14702272f, 1.55101287f);
		GFPhysVector3 EndFixPoint2 = GFPhysVector3(2.969433248f, 6.14702272f, 1.55101287f);
		int segmentCount2 = 10;
		Real masspernode2 = 0.05f;
		Real rotMassPerSeg = 0.01f;

		m_sutureV2->Create(StartFixPoint2, EndFixPoint2, segmentCount2, masspernode2, rotMassPerSeg, false);


		//m_sutureV2->SetNodeAsFix(0);
		m_sutureV2->SetSegmentAsFix(0);

	}

	//m_sutureV2->m_TubeNodes[5]->SetPosition(GFPhysVector3(4.969433248f, 6.14702272f, 1.55101287f));	
	//SutureThreadNodeV2* node = (SutureThreadNodeV2*)(m_sutureV2->m_TubeNodes[5]);
	//node->m_invmassSclae = 2.0f;

	//m_TubeNodes存的是指针，可以强转。存变量就不行

	//m_sutureV2->SetSegmentAsFix(4);
	//////////////////////////////////////////////////////////////////////////
	return result;
}
//======================================================================================================================
void CSutureKnotV2Train3::InitBallPos()
{
	for (int i = 0; i < 8; i++)
	{
		m_StaticMark[i].visible = true;
		m_StaticMark[i].radius = 0.04;
		m_StaticMark[i].matname = "MarkTextureMaterialOrange";
	}

	for (int i = 0; i < 4; i++)
	{
		if (m_StaticMark[2 * i].visible)
		{
			MMO_Face originface = m_pKnotTestOrgan0->m_OriginFaces[m_index[2 * i]];

			if (originface.m_HasError == false)
			{
				m_StaticMark[2 * i].uvpos = originface.GetTextureCoord(0) * 0.33f
					+ originface.GetTextureCoord(1) * 0.33f
					+ originface.GetTextureCoord(2) * 0.33f;
			}
		}
	}

	for (int i = 1; i < 5; i++)
	{
		if (m_StaticMark[2 * i - 1].visible)
		{
			MMO_Face originface = m_pKnotTestOrgan1->m_OriginFaces[m_index[2 * i - 1]];

			if (originface.m_HasError == false)
			{
				m_StaticMark[2 * i - 1].uvpos = originface.GetTextureCoord(0) * 0.33f
					+ originface.GetTextureCoord(1) * 0.33f
					+ originface.GetTextureCoord(2) * 0.33f;
			}
		}
	}
	m_ballinfo.clear();

	//SutureThreadV2* rope = m_Needle[0]->GetSutureThread();
	//SutureRopeNode& node0 = rope->GetThreadNodeRef(42);
	//node0.SetSolverInvMassScale(0.0f);
	//node0.m_CurrPosition = 10.0f*GFPhysVector3(3.88f, 6.46f, -1.06f);

	//SutureRopeNode& node1 = rope->GetThreadNodeRef(47);
	//node1.SetSolverInvMassScale(0.0f);
	//node1.m_CurrPosition = 10.0f*GFPhysVector3(-0.68f, 3.09f, -6.22f);

}
//======================================================================================================================
void CSutureKnotV2Train3::ResetNeedle()
{
	m_ballinfo.clear();
	__super::ResetNeedle();
	m_NeedleFallStateLast = false;
	m_NeedleFallState = false;
	m_bGraspNeedle = false;

	//SutureThreadV2* rope = m_Needle[0]->GetSutureThread();
	//SutureRopeNode& node0 = rope->GetThreadNodeRef(42);
	//node0.SetSolverInvMassScale(0.0f);
	//node0.m_CurrPosition = GFPhysVector3(3.88f, 6.46f, -1.06f);

	//SutureRopeNode& node1 = rope->GetThreadNodeRef(47);
	//node1.SetSolverInvMassScale(0.0f);
	//node1.m_CurrPosition = GFPhysVector3(-0.68f, 3.09f, -6.22f);
}
//======================================================================================================================
void CSutureKnotV2Train3::CreateAnotherNeedle()
{
	m_Needle[1] = CreateNeedleV2(MXOgre_SCENEMANAGER, m_SutureNodeNum, m_SutureRsLength, m_NeedleSkeletonFile);
	m_Needle[1]->addNeedleActionListener(this);
	SutureThreadV2* attachrope = m_Needle[1]->GetSutureThread();
	if (attachrope)
	{
		m_Knot[1] = new SutureKnotV2();
		m_Knot[1]->SetThread(attachrope);
		attachrope->m_KnotsInThread = m_Knot[1];
	}
	m_NeedleFallStateLast = false;
	m_NeedleFallState = false;
	m_bGraspNeedle = false;
}
//======================================================================================================================
void CSutureKnotV2Train3::DrawMarkRenderTarget()
{
	m_pKnotTestOrgan0->GetEffectRender().clearMarkQuadObject();
	for (int i = 0; i < 4; i++)
	{
		if (m_StaticMark[2 * i].visible && m_StaticMark[2 * i].uvpos != Ogre::Vector2::ZERO)
		{
			m_pKnotTestOrgan0->GetEffectRender().MarkEffectTexture(m_StaticMark[2 * i].uvpos, m_StaticMark->radius, m_StaticMark[i].matname);
		}
	}

	for (int i = 0, ni = m_ballinfo.size(); i < ni; i++)
	{
		if (m_ballinfo[i].Organ == m_pKnotTestOrgan0 && m_ballinfo[i].dynamicMark.visible && m_ballinfo[i].dynamicMark.uvpos != Ogre::Vector2::ZERO)
		{
			m_pKnotTestOrgan0->GetEffectRender().MarkEffectTexture(m_ballinfo[i].dynamicMark.uvpos, 1.5f * m_ballinfo[i].dynamicMark.radius, m_ballinfo[i].dynamicMark.matname);
		}
	}
	m_pKnotTestOrgan0->GetEffectRender().MarkTextureFlush();
	m_pKnotTestOrgan0->GetEffectRender().clearMarkQuadObject();

	//////////////////////////////////////////////////////////////////////////
	m_pKnotTestOrgan1->GetEffectRender().clearMarkQuadObject();
	for (int i = 1; i < 5; i++)
	{
		if (m_StaticMark[2 * i - 1].visible && m_StaticMark[2 * i - 1].uvpos != Ogre::Vector2::ZERO)
		{
			m_pKnotTestOrgan1->GetEffectRender().MarkEffectTexture(m_StaticMark[2 * i - 1].uvpos, m_StaticMark->radius, m_StaticMark[i].matname);
		}
	}

	for (int i = 0, ni = m_ballinfo.size(); i < ni; i++)
	{
		if (m_ballinfo[i].Organ == m_pKnotTestOrgan1 && m_ballinfo[i].dynamicMark.visible && m_ballinfo[i].dynamicMark.uvpos != Ogre::Vector2::ZERO)
		{
			m_pKnotTestOrgan1->GetEffectRender().MarkEffectTexture(m_ballinfo[i].dynamicMark.uvpos, 1.5f * m_ballinfo[i].dynamicMark.radius, m_ballinfo[i].dynamicMark.matname);
		}
	}
	m_pKnotTestOrgan1->GetEffectRender().MarkTextureFlush();
	m_pKnotTestOrgan1->GetEffectRender().clearMarkQuadObject();
}
//======================================================================================================================
bool CSutureKnotV2Train3::Update(float dt)
{
	bool result = CSutureKnotV2Train::Update(dt);



	//////////////////////////////////////////////////////////////////////////
	/*m_sutureV1->UpdateMesh();
	m_sutureV1->BeginSimulateSutureThreadPhysics(0.01f);
v2	m_sutureV1->SolveConstraint(1.0f, 0.01f);
	m_sutureV1->EndSimulateThreadPhysics(0.01f);*/

	//////////////////////////////////////////////////////////////////////////
	if (m_sutureV2)
	{
		//m_sutureV2->UpdateMesh();
		
		GFPhysSoftTubeNode& wa = m_sutureV2->GetThreadNodeRef(6);

		wa.m_CurrPosition = GFPhysVector3(4.969433248f, 5.14702272f, 1.55101287f);
	}


	//////////////////////////////////////////////////////////////////////////

	UpdateBaseBallPos();
	DrawMarkRenderTarget();

	DetectNeedleFall();

	//for debug
	//GFPhysVectorObj<GFPhysFaceRopeAnchorV2*> activeRopeAnchors;
	//if (m_Needle[0] && m_Needle[1] == NULL)
	//{
	//	m_Needle[0]->GetSutureThread()->GetFaceRopeAnchors(activeRopeAnchors);
	//	if (activeRopeAnchors.size() > 3)
	//	{
	//		m_Needle[0]->Disappear();
	//		CreateAnotherNeedle();
	//	}
	//}

	GFPhysVectorObj<KnotInSutureRopeV2> Knots;

	if (m_Needle[1] == NULL && m_Needle[0]->GetSutureThread())
	{
		m_Needle[0]->GetSutureThread()->m_KnotsInThread->GetDeadKnots(Knots);

		if (Knots.size() == 1)
		{
			m_bFirstHalfKnotFinish = true;
		}
		else if (Knots.size() == 2)
		{
			CreateAnotherNeedle();
			Knots.clear();
			if (m_Needle[0]->GetPhysicBody())
			{
				//RELEASE()
				if (m_pToolsMgr->GetLeftTool())
				{
					CTool* leftTool = dynamic_cast<CTool*>(m_pToolsMgr->GetLeftTool());
					for (size_t p = 0, np = leftTool->m_ToolPlugins.size(); p < np; p++)
					{
						MisCTool_PluginRigidHold * holdPlugin = dynamic_cast<MisCTool_PluginRigidHold*>(leftTool->m_ToolPlugins[p]);
						if (holdPlugin && holdPlugin->GetRigidHoldState())
						{
							holdPlugin->ReleaseHoldingRigid();
						}

						MisCTool_PluginClamp * clampPlugin = dynamic_cast<MisCTool_PluginClamp*>(leftTool->m_ToolPlugins[p]);
						if (clampPlugin && clampPlugin->GetRopeBeClamped())
						{
							clampPlugin->ReleaseClampedRopeV2();
						}
					}
				}

				if (m_pToolsMgr->GetRightTool())
				{
					CTool* rightTool = dynamic_cast<CTool*>(m_pToolsMgr->GetRightTool());
					for (size_t p = 0, np = rightTool->m_ToolPlugins.size(); p < np; p++)
					{
						MisCTool_PluginRigidHold * holdPlugin = dynamic_cast<MisCTool_PluginRigidHold*>(rightTool->m_ToolPlugins[p]);
						if (holdPlugin && holdPlugin->GetRigidHoldState())
						{
							holdPlugin->ReleaseHoldingRigid();
						}

						MisCTool_PluginClamp * clampPlugin = dynamic_cast<MisCTool_PluginClamp*>(rightTool->m_ToolPlugins[p]);
						if (clampPlugin && clampPlugin->GetThreadClampState())
						{
							clampPlugin->ReleaseClampedRopeV2();
						}
					}
				}

				m_Needle[0]->Disappear();
			}
		}
		m_bFirstHalfKnotFinish = false;
	}

	else if (m_Needle[1] && m_Needle[1]->GetSutureThread())
	{
		m_Needle[1]->GetSutureThread()->m_KnotsInThread->GetDeadKnots(Knots);

		if (Knots.size() == 1)
		{
			m_bFirstHalfKnotFinish = true;
		}
		else if (Knots.size() == 2)
		{
			m_bFinished = true;
			__super::TrainingFinish();
		}
	}

	//GFPhysVectorObj<FaceAnchorInfo>& faceanchors = m_Needle[0]->getFaceNeedleAnchors();

	//if (faceanchors.size() == 4 && m_Needle1 == NULL)
	//{
	//    CreateAnotherNeedle();
	//}    

	if (m_bTrainingIlluminated)
	{
		GFPhysVectorObj<GFPhysFaceRopeAnchorV2*> activeRopeAnchors;

		if (m_Needle[0] && m_Needle[1] == NULL)
		{
			m_Needle[0]->GetSutureThread()->GetFaceRopeAnchors(activeRopeAnchors);

			GFPhysVectorObj<FaceAnchorInfoV2>& faceanchors = m_Needle[0]->getFaceNeedleAnchors();

			if (m_bGraspNeedle && activeRopeAnchors.size() >= 3 && !m_bFirstHalfKnotFinish)
			{
				CTipMgr::Instance()->ShowTip("AdjustThreadTail");
				if (m_bAdjust_Tail[0]/*true*/)
				{
					SetNextTip("TakeRound", 3.0f);
				}
			}

			if (m_bGraspNeedle && activeRopeAnchors.size() == 0)
			{
				CTipMgr::Instance()->ShowTip("InjectNeedleCorrect");
			}

			if (!m_bGraspNeedle && !m_bFirstHalfKnotFinish
				&& 0 == activeRopeAnchors.size() && 0 == faceanchors.size())
			{
				CTipMgr::Instance()->ShowTip("GraspNeedleCorrect");
			}
			else if (m_bFirstHalfKnotFinish)
			{
				CTipMgr::Instance()->ShowTip("SecondHalfKnot");
				m_bFirstHalfKnotFinish = false;
			}
		}

		if (m_Needle[0] && m_Needle[1])
		{
			m_Needle[1]->GetSutureThread()->GetFaceRopeAnchors(activeRopeAnchors);

			GFPhysVectorObj<FaceAnchorInfoV2>& faceanchors = m_Needle[1]->getFaceNeedleAnchors();

			if (m_bGraspNeedle && activeRopeAnchors.size() >= 3 && !m_bFirstHalfKnotFinish)
			{
				CTipMgr::Instance()->ShowTip("AdjustThreadTail");
				if (m_bAdjust_Tail[1]/*true*/)
				{
					SetNextTip("TakeRound", 3.0f);
				}
			}
			if (m_bGraspNeedle && activeRopeAnchors.size() == 0)
			{
				CTipMgr::Instance()->ShowTip("InjectNeedleCorrect");
			}
			if (!m_bGraspNeedle && !m_bFirstHalfKnotFinish
				&& 0 == activeRopeAnchors.size() && 0 == faceanchors.size())
			{
				CTipMgr::Instance()->ShowTip("GraspNeedleCorrect");
			}
			else if (m_bFirstHalfKnotFinish)
			{
				CTipMgr::Instance()->ShowTip("SecondHalfKnot");
			}
		}

	}

	return result;
}
//======================================================================================================================
void CSutureKnotV2Train3::UpdateBaseBallPos()
{
	for (int i = 0, ni = m_ballinfo.size(); i < ni; i++)
	{
		int faceId = m_ballinfo[i].Organ->GetOriginFaceIndexFromUsrData(m_ballinfo[i].InoutFace);

		if (faceId >= 0 && faceId < (int)m_ballinfo[i].Organ->m_OriginFaces.size())
		{
			MMO_Face originface = m_ballinfo[i].Organ->m_OriginFaces[faceId];

			if (originface.m_HasError == false)
			{
				m_ballinfo[i].dynamicMark.uvpos =
					originface.GetTextureCoord(0) * m_ballinfo[i].InoutWeights0
					+ originface.GetTextureCoord(1) * m_ballinfo[i].InoutWeights1
					+ originface.GetTextureCoord(2) * m_ballinfo[i].InoutWeights2;
			}
		}
	}
}
//======================================================================================================================
void CSutureKnotV2Train3::OnCreateInAnchor(const GFPhysSoftBodyFace* face, const Real weights[])
{
	GFPhysVector3 anchorpos = face->m_Nodes[0]->m_CurrPosition * weights[0]
		+ face->m_Nodes[1]->m_CurrPosition * weights[1]
		+ face->m_Nodes[2]->m_CurrPosition * weights[2];
	UpdateStaticMark(anchorpos);

	CreatedynamicMark(face, weights);

	if (m_pToolsMgr->GetRightTool())
	{
		CTool* rightTool = dynamic_cast<CTool*>(m_pToolsMgr->GetRightTool());
		for (size_t p = 0, np = rightTool->m_ToolPlugins.size(); p < np; p++)
		{
			MisCTool_PluginRigidHold * holdPlugin = dynamic_cast<MisCTool_PluginRigidHold*>(rightTool->m_ToolPlugins[p]);
			if (holdPlugin && holdPlugin->GetRigidHoldState())
			{
				const MisCTool_PluginRigidHold::ToolHoldRegion & BelongRegion = holdPlugin->GetHoldRegion(0);
				GFPhysVector3 toolDir = BelongRegion.m_Axis1World;
				if (m_Needle[1] == NULL)
				{
					GFPhysVectorObj<FaceAnchorInfoV2>& faceanchors = m_Needle[0]->getFaceNeedleAnchors();
					size_t k = faceanchors.size();
					GFPhysFaceNeedleAnchorV2* lastAnchor = faceanchors[k - 1].pAnchor;

					GFPhysSoftBodyFace * face = lastAnchor->GetFace();
					MisMedicOrgan_Ordinary* Organ = (MisMedicOrgan_Ordinary*)face->m_Nodes[0]->m_SoftBody->GetUserPointer();
					if (Organ == m_pKnotTestOrgan0)
					{
						m_InjectAngle[0] = lastAnchor->m_initAngle;
						//////////////////////////////////////////////////////////////////////////

						GFPhysVector3 planeNormal = m_Needle[0]->GetNeedleNormalDirction();
						m_HolderAngle[0] = CalcAngleBetweenLineAndFace(toolDir, planeNormal);
						//////////////////////////////////////////////////////////////////////////
						{
							GFPhysVector3 localpos = holdPlugin->GetRigidBodyHoldLocalPoint();
							GFPhysRigidBody* rgbody = holdPlugin->GetRigidBodyHolded();
							if (rgbody == m_Needle[0]->GetPhysicBody())
							{
								GFPhysTransform rgtrans = rgbody->GetCenterOfMassTransform();
								GFPhysVector3 worldpos = rgtrans(localpos);
								int i = 0;
								int j = 0;
								if (m_Needle[0]->Getinterval(worldpos, i, j))
								{
									int num = m_Needle[0]->m_NeedleNodeWorldPos.size();
									if ((i > 1 && i < (int)(num * 0.333f + 1)) && (j > 1 && j < (int)(num * 0.333f + 1)))
									{
										HoldPos[0] = true;
									}
								}
							}
						}
					}
				}
				else
				{

					GFPhysVectorObj<FaceAnchorInfoV2>& faceanchors = m_Needle[1]->getFaceNeedleAnchors();
					size_t k = faceanchors.size();
					GFPhysFaceNeedleAnchorV2* lastAnchor = faceanchors[k - 1].pAnchor;

					GFPhysSoftBodyFace * face = lastAnchor->GetFace();
					MisMedicOrgan_Ordinary* Organ = (MisMedicOrgan_Ordinary*)face->m_Nodes[0]->m_SoftBody->GetUserPointer();
					if (Organ == m_pKnotTestOrgan0)
					{
						m_InjectAngle[1] = lastAnchor->m_initAngle;

						//////////////////////////////////////////////////////////////////////////

						GFPhysVector3 planeNormal = m_Needle[1]->GetNeedleNormalDirction();
						m_HolderAngle[1] = CalcAngleBetweenLineAndFace(toolDir, planeNormal);
						//////////////////////////////////////////////////////////////////////////
						{
							GFPhysVector3 localpos = holdPlugin->GetRigidBodyHoldLocalPoint();
							GFPhysRigidBody* rgbody = holdPlugin->GetRigidBodyHolded();
							if (rgbody == m_Needle[0]->GetPhysicBody())
							{
								GFPhysTransform rgtrans = rgbody->GetCenterOfMassTransform();
								GFPhysVector3 worldpos = rgtrans(localpos);
								int i = 0;
								int j = 0;
								if (m_Needle[0]->Getinterval(worldpos, i, j))
								{
									int num = m_Needle[1]->m_NeedleNodeWorldPos.size();
									if (i < (int)(num * 0.333f + 1) || j < (int)(num * 0.333f + 1))
									{
										HoldPos[1] = true;
									}
								}
							}
						}
					}
				}
			}
		}
	}
}
//======================================================================================================================
void CSutureKnotV2Train3::OnCreateOutAnchor(const GFPhysSoftBodyFace* face, const Real weights[])
{
	GFPhysVector3 anchorpos = face->m_Nodes[0]->m_CurrPosition * weights[0]
		+ face->m_Nodes[1]->m_CurrPosition * weights[1]
		+ face->m_Nodes[2]->m_CurrPosition * weights[2];
	UpdateStaticMark(anchorpos);

	CreatedynamicMark(face, weights);

	if (m_pToolsMgr->GetLeftTool())
	{
		CTool* leftTool = dynamic_cast<CTool*>(m_pToolsMgr->GetLeftTool());
		for (size_t p = 0, np = leftTool->m_ToolPlugins.size(); p < np; p++)
		{
			MisCTool_PluginRigidHold * holdPlugin = dynamic_cast<MisCTool_PluginRigidHold*>(leftTool->m_ToolPlugins[p]);
			if (holdPlugin && holdPlugin->GetRigidHoldState())
			{
				const MisCTool_PluginRigidHold::ToolHoldRegion & BelongRegion = holdPlugin->GetHoldRegion(0);
				GFPhysVector3 toolDir = BelongRegion.m_Axis1World;
				if (m_Needle[1] == NULL)
				{
					GFPhysVectorObj<FaceAnchorInfoV2>& faceanchors = m_Needle[0]->getFaceNeedleAnchors();
					size_t k = faceanchors.size();
					GFPhysFaceNeedleAnchorV2* lastAnchor = faceanchors[k - 1].pAnchor;

					GFPhysSoftBodyFace * face = lastAnchor->GetFace();
					MisMedicOrgan_Ordinary* Organ = (MisMedicOrgan_Ordinary*)face->m_Nodes[0]->m_SoftBody->GetUserPointer();
					if (Organ == m_pKnotTestOrgan1)
					{
						m_WithDrawAngle[0] = lastAnchor->m_initAngle;
					}
				}
				else
				{
					GFPhysVectorObj<FaceAnchorInfoV2>& faceanchors = m_Needle[1]->getFaceNeedleAnchors();
					size_t k = faceanchors.size();
					GFPhysFaceNeedleAnchorV2* lastAnchor = faceanchors[k - 1].pAnchor;

					GFPhysSoftBodyFace * face = lastAnchor->GetFace();
					MisMedicOrgan_Ordinary* Organ = (MisMedicOrgan_Ordinary*)face->m_Nodes[0]->m_SoftBody->GetUserPointer();
					if (Organ == m_pKnotTestOrgan1)
					{
						m_WithDrawAngle[1] = lastAnchor->m_initAngle;
					}
				}
			}
		}
	}
}
//======================================================================================================================
void CSutureKnotV2Train3::OnRemoveInAnchor(const GFPhysSoftBodyFace* face, const Real weights[])
{
	GFPhysVector3 anchorpos = face->m_Nodes[0]->m_CurrPosition * weights[0]
		+ face->m_Nodes[1]->m_CurrPosition * weights[1]
		+ face->m_Nodes[2]->m_CurrPosition * weights[2];
	UpdateStaticMark(anchorpos);

	DeletedynamicMark(face);
}
//======================================================================================================================
void CSutureKnotV2Train3::OnRemoveOutAnchor(const GFPhysSoftBodyFace* face, const Real weights[])
{
	GFPhysVector3 anchorpos = face->m_Nodes[0]->m_CurrPosition * weights[0]
		+ face->m_Nodes[1]->m_CurrPosition * weights[1]
		+ face->m_Nodes[2]->m_CurrPosition * weights[2];
	UpdateStaticMark(anchorpos);

	DeletedynamicMark(face);
}
//======================================================================================================================
void CSutureKnotV2Train3::OnRemoveRopeAnchor(const GFPhysSoftBodyFace* face, const Real weights[])
{
	GFPhysVector3 anchorpos = face->m_Nodes[0]->m_CurrPosition * weights[0]
		+ face->m_Nodes[1]->m_CurrPosition * weights[1]
		+ face->m_Nodes[2]->m_CurrPosition * weights[2];
	UpdateStaticMark(anchorpos);

	DeletedynamicMark(face);
}
//======================================================================================================================
void CSutureKnotV2Train3::UpdateStaticMark(const GFPhysVector3& anchorpos)
{
	return; //static mark is always rendered
	for (int i = 0; i < 4; i++)
	{
		MMO_Face originface = m_pKnotTestOrgan0->m_OriginFaces[m_index[2 * i]];
		GFPhysVector3 pos = originface.m_physface->m_Nodes[0]->m_CurrPosition * 0.33f
			+ originface.m_physface->m_Nodes[1]->m_CurrPosition * 0.33f
			+ originface.m_physface->m_Nodes[2]->m_CurrPosition * 0.33f;
		if (pos.Distance(anchorpos) < BALLPOSEXTENT)
		{
			m_StaticMark[2 * i].visible = !m_StaticMark[2 * i].visible;
		}
	}

	for (int i = 1; i < 5; i++)
	{
		MMO_Face originface = m_pKnotTestOrgan1->m_OriginFaces[m_index[2 * i - 1]];
		GFPhysVector3 pos = originface.m_physface->m_Nodes[0]->m_CurrPosition * 0.33f
			+ originface.m_physface->m_Nodes[1]->m_CurrPosition * 0.33f
			+ originface.m_physface->m_Nodes[2]->m_CurrPosition * 0.33f;
		if (pos.Distance(anchorpos) < BALLPOSEXTENT)
		{
			m_StaticMark[2 * i - 1].visible = !m_StaticMark[2 * i - 1].visible;
		}

	}
}
//======================================================================================================================
void CSutureKnotV2Train3::CreatedynamicMark(const GFPhysSoftBodyFace* face, const Real weights[])
{
	MarkTextureState dynamicMark;
	dynamicMark.visible = true;
	dynamicMark.radius = 0.06f;
	dynamicMark.matname = "MarkTextureMaterialGreen";

	GFPhysSoftBodyFace* InoutFace = (GFPhysSoftBodyFace*)face;

	MisMedicOrgan_Ordinary* Organ = (MisMedicOrgan_Ordinary*)face->m_Nodes[0]->m_SoftBody->GetUserPointer();

	Real InoutWeights[3];
	InoutWeights[0] = weights[0];
	InoutWeights[1] = weights[1];
	InoutWeights[2] = weights[2];

	InoutBallInfo info = { dynamicMark, Organ, InoutFace, InoutWeights[0], InoutWeights[1], InoutWeights[2] };

	m_ballinfo.push_back(info);
}
//======================================================================================================================
void CSutureKnotV2Train3::DeletedynamicMark(const GFPhysSoftBodyFace* face)
{
	std::vector<InoutBallInfo>::iterator iter;
	for (iter = m_ballinfo.begin(); iter != m_ballinfo.end();)
	{
		InoutBallInfo info = *iter;
		if (info.InoutFace == face)
		{
			iter = m_ballinfo.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}
//======================================================================================================================
void CSutureKnotV2Train3::OnThreadClampByTool(int ClampedSegIndex, CTool* toolobject)
{
	CSutureKnotV2Train::OnThreadClampByTool(ClampedSegIndex, toolobject);
	//绕圈成功+抓住近线尾
	GFPhysVectorObj<GFPhysFaceRopeAnchorV2*> activeRopeAnchors;
	for (int i = 0; i < 2; i++)
	{
		if (m_Needle[i] == NULL)
		{
			return;
		}
		m_Needle[i]->GetSutureThread()->GetFaceRopeAnchors(activeRopeAnchors);
		if (activeRopeAnchors.size() > 0)
		{
			int maxAnchorIndex = activeRopeAnchors[0]->GetSegIndex();
			if (m_bCircleKnot && /*ClampedSegIndex > maxAnchorIndex &&*/ !m_bFirstHalfKnotFinish)
			{
				if (i == 0)
				{
					m_bScoreKnot[0] = true;
				}
				if (i == 1)
				{
					m_bScoreKnot[2] = true;
				}

				CTipMgr::Instance()->ShowTip("PullThreadThroughCircle");

				SetNextTip("FirstHalfKnot", 3.0f);
			}

			if (m_bCircleKnot && /*ClampedSegIndex > maxAnchorIndex &&*/ m_bFirstHalfKnotFinish)
			{
				if (i == 0)
				{
					m_bScoreKnot[1] = true;
				}
				if (i == 1)
				{
					m_bScoreKnot[3] = true;
				}

				CTipMgr::Instance()->ShowTip("PullThreadThroughCircle");

				SetNextTip("GetSquareKnot", 3.0f);
			}
		}
	}

}
//======================================================================================================================

void CSutureKnotV2Train3::OnRigidClampByTool(GFPhysRigidBody * rigid)
{
	for (int i = 0; i < 2; i++)
	{

		if (m_Needle[i] && rigid == m_Needle[i]->GetPhysicBody())
		{
			m_bGraspNeedle = true;

			m_NeedleFallStateLast = false;
			m_NeedleFallState = false;


			GFPhysVectorObj<GFPhysFaceRopeAnchorV2*> activeRopeAnchors;
			m_Needle[i]->GetSutureThread()->GetFaceRopeAnchors(activeRopeAnchors);
			if (activeRopeAnchors.size() < 3)
			{
				CTipMgr::Instance()->ShowTip("InjectNeedleCorrect");
			}
		}
	}
}
//======================================================================================================================

void CSutureKnotV2Train3::OnRigidReleaseByTool(GFPhysRigidBody * rigid)
{
	for (int i = 0; i < 2; i++)
	{
		if (m_Needle[i] && rigid == m_Needle[i]->GetPhysicBody())
		{
			m_bGraspNeedle = false;
		}
	}
}
//======================================================================================================================

void CSutureKnotV2Train3::OnThreadReleaseByTool(int ClampedSegGlobalIndex, CTool* toolobject)
{
	CSutureKnotV2Train::OnThreadReleaseByTool(ClampedSegGlobalIndex, toolobject);

	for (int i = 0; i < 2; i++)
	{
		GFPhysVectorObj<GFPhysFaceRopeAnchorV2*> activeRopeAnchors;
		if (m_Needle[i])
		{
			m_Needle[i]->GetSutureThread()->GetFaceRopeAnchors(activeRopeAnchors);
			if (activeRopeAnchors.size() > 0)
			{
				int maxAnchorIndex = activeRopeAnchors[0]->GetSegIndex();

				if (m_bCircleKnot /*&& ClampedSegIndex > maxAnchorIndex*/)
				{
					CTipMgr::Instance()->ShowTip("PullThreadThroughCircle");
				}
			}
		}
	}
}

void CSutureKnotV2Train3::OnCollisionKeep(GFPhysCollideObject * objA, GFPhysCollideObject * objB, const GFPhysManifoldPoint * contactPoints, int NumContactPoints)
{
	__super::OnCollisionKeep(objA, objB, contactPoints, NumContactPoints);

	GFPhysRigidBody * ra = GFPhysRigidBody::Upcast(objA);
	GFPhysRigidBody * rb = GFPhysRigidBody::Upcast(objB);

	for (int i = 0; i < 2; i++)
	{
		if (m_Needle[i])
		{
			GFPhysRigidBody * needleRigid = m_Needle[i]->GetPhysicBody();

			GFPhysVector3 dipanCenter(BASECENTERX, BASECENTERY, BASECENTERZ);
			GFPhysVector3 needleCenter = m_Needle[i]->m_CenterOfMass;
			needleCenter.SetY(0);
			Real dist = needleCenter.Distance(dipanCenter);

			if ((ra == needleRigid && rb == m_pBaseTerrainRigidBody) || (ra == m_pBaseTerrainRigidBody && rb == needleRigid))
			{
				if (m_Needle[i]->m_CenterOfMass.y() < 10.0f && dist > BASERADIUS && !m_bGraspNeedle)
				{
					m_NeedleFallState = true;
				}
			}
		}
	}

}
void CSutureKnotV2Train3::DetectNeedleFall()
{
	if (!m_NeedleFallStateLast && m_NeedleFallState && !m_bGraspNeedle)///只有发生了碰撞才进
	{
		m_NeedleFallCount++;
		m_NeedleFallStateLast = true;
		//m_Needle[0]->Disappear();
		ResetNeedle();
	}
}

void CSutureKnotV2Train3::GatherPosInfo()
{
	for (int i = 0; i < 2; i++)
	{
		if (m_HolderAngle[i] > ANGLECORRECT)
		{
			HoldAngle[i] = CORRECT;
		}
		else if (m_HolderAngle[i] > ANGLEBIAS)
		{
			HoldAngle[i] = BIAS;
		}
		else
		{
			HoldAngle[i] = MISS;
		}

		if (m_InjectAngle[i] > ANGLECORRECT)
		{
			InjectAngle[i] = CORRECT;
		}
		else if (m_InjectAngle[i] > ANGLEBIAS)
		{
			InjectAngle[i] = BIAS;
		}
		else
		{
			InjectAngle[i] = MISS;
		}

		if (m_WithDrawAngle[i] > ANGLECORRECT)
		{
			WithDrawAngle[i] = CORRECT;
		}
		else if (m_WithDrawAngle[i] > ANGLEBIAS)
		{
			WithDrawAngle[i] = BIAS;
		}
		else
		{
			WithDrawAngle[i] = MISS;
		}
	}



	Real dis = 0.0f;
	GFPhysVector3 anchorpos = GFPhysVector3(0.0f, 0.0f, 0.0f);
	MMO_Face originface0 = m_pKnotTestOrgan0->m_OriginFaces[m_index[0]];
	GFPhysVector3 pos0 = originface0.m_physface->m_Nodes[0]->m_CurrPosition * 0.33f
		+ originface0.m_physface->m_Nodes[1]->m_CurrPosition * 0.33f
		+ originface0.m_physface->m_Nodes[2]->m_CurrPosition * 0.33f;

	MMO_Face originface1 = m_pKnotTestOrgan0->m_OriginFaces[m_index[1]];
	GFPhysVector3 pos1 = originface1.m_physface->m_Nodes[0]->m_CurrPosition * 0.33f
		+ originface1.m_physface->m_Nodes[1]->m_CurrPosition * 0.33f
		+ originface1.m_physface->m_Nodes[2]->m_CurrPosition * 0.33f;

	MMO_Face originface4 = m_pKnotTestOrgan0->m_OriginFaces[m_index[4]];
	GFPhysVector3 pos4 = originface4.m_physface->m_Nodes[0]->m_CurrPosition * 0.33f
		+ originface4.m_physface->m_Nodes[1]->m_CurrPosition * 0.33f
		+ originface4.m_physface->m_Nodes[2]->m_CurrPosition * 0.33f;

	MMO_Face originface5 = m_pKnotTestOrgan0->m_OriginFaces[m_index[5]];
	GFPhysVector3 pos5 = originface5.m_physface->m_Nodes[0]->m_CurrPosition * 0.33f
		+ originface5.m_physface->m_Nodes[1]->m_CurrPosition * 0.33f
		+ originface5.m_physface->m_Nodes[2]->m_CurrPosition * 0.33f;

	GFPhysVectorObj<GFPhysFaceRopeAnchorV2*> activeRopeAnchors;

	m_Needle[0]->GetSutureThread()->GetFaceRopeAnchors(activeRopeAnchors);
	if (activeRopeAnchors.size() > 1)
	{
		anchorpos = activeRopeAnchors[0]->GetFace()->m_Nodes[0]->m_CurrPosition * activeRopeAnchors[0]->m_weights[0] +
			activeRopeAnchors[0]->GetFace()->m_Nodes[1]->m_CurrPosition * activeRopeAnchors[0]->m_weights[1] +
			activeRopeAnchors[0]->GetFace()->m_Nodes[2]->m_CurrPosition * activeRopeAnchors[0]->m_weights[2];

		Real dis0 = anchorpos.Distance(pos0);

		Real dis4 = anchorpos.Distance(pos4);

		dis = std::min(dis0, dis4);

		if (dis < BALLPOSEXTENT)
		{
			InjectPos[0] = CORRECT;
		}
		else if (dis > BALLPOSEXTENT && dis < 3 * BALLPOSEXTENT)
		{
			InjectPos[0] = BIAS;
		}
		else if (dis > 3 * BALLPOSEXTENT)
		{
			InjectPos[0] = MISS;
		}

		anchorpos = activeRopeAnchors[activeRopeAnchors.size() - 1]->GetFace()->m_Nodes[0]->m_CurrPosition * activeRopeAnchors[activeRopeAnchors.size() - 1]->m_weights[0] +
			activeRopeAnchors[activeRopeAnchors.size() - 1]->GetFace()->m_Nodes[1]->m_CurrPosition * activeRopeAnchors[activeRopeAnchors.size() - 1]->m_weights[1] +
			activeRopeAnchors[activeRopeAnchors.size() - 1]->GetFace()->m_Nodes[2]->m_CurrPosition * activeRopeAnchors[activeRopeAnchors.size() - 1]->m_weights[2];

		Real dis1 = anchorpos.Distance(pos1);

		Real dis5 = anchorpos.Distance(pos5);

		dis = std::min(dis1, dis5);

		if (dis < BALLPOSEXTENT)
		{
			WithDrawPos[0] = CORRECT;
		}
		else if (dis > BALLPOSEXTENT && dis < 3 * BALLPOSEXTENT)
		{
			WithDrawPos[0] = BIAS;
		}
		else if (dis > 3 * BALLPOSEXTENT)
		{
			WithDrawPos[0] = MISS;
		}
	}

	if (m_Needle[1] != NULL)
	{
		m_Needle[1]->GetSutureThread()->GetFaceRopeAnchors(activeRopeAnchors);
		if (activeRopeAnchors.size() > 1)
		{
			anchorpos = activeRopeAnchors[0]->GetFace()->m_Nodes[0]->m_CurrPosition * activeRopeAnchors[0]->m_weights[0] +
				activeRopeAnchors[0]->GetFace()->m_Nodes[1]->m_CurrPosition * activeRopeAnchors[0]->m_weights[1] +
				activeRopeAnchors[0]->GetFace()->m_Nodes[2]->m_CurrPosition * activeRopeAnchors[0]->m_weights[2];

			Real dis0 = anchorpos.Distance(pos0);

			Real dis4 = anchorpos.Distance(pos4);

			dis = std::min(dis0, dis4);

			if (dis < BALLPOSEXTENT)
			{
				InjectPos[1] = CORRECT;
			}
			else if (dis > BALLPOSEXTENT && dis < 3 * BALLPOSEXTENT)
			{
				InjectPos[1] = BIAS;
			}
			else if (dis > 3 * BALLPOSEXTENT)
			{
				InjectPos[1] = MISS;
			}

			anchorpos = activeRopeAnchors[activeRopeAnchors.size() - 1]->GetFace()->m_Nodes[0]->m_CurrPosition * activeRopeAnchors[activeRopeAnchors.size() - 1]->m_weights[0] +
				activeRopeAnchors[activeRopeAnchors.size() - 1]->GetFace()->m_Nodes[1]->m_CurrPosition * activeRopeAnchors[activeRopeAnchors.size() - 1]->m_weights[1] +
				activeRopeAnchors[activeRopeAnchors.size() - 1]->GetFace()->m_Nodes[2]->m_CurrPosition * activeRopeAnchors[activeRopeAnchors.size() - 1]->m_weights[2];

			Real dis1 = anchorpos.Distance(pos1);

			Real dis5 = anchorpos.Distance(pos5);

			dis = std::min(dis1, dis5);

			if (dis < BALLPOSEXTENT)
			{
				WithDrawPos[1] = CORRECT;
			}
			else if (dis > BALLPOSEXTENT && dis < 3 * BALLPOSEXTENT)
			{
				WithDrawPos[1] = BIAS;
			}
			else if (dis > 3 * BALLPOSEXTENT)
			{
				WithDrawPos[1] = MISS;
			}
		}
	}

}
void CSutureKnotV2Train3::OnSaveTrainingReport()
{

	GatherPosInfo();


	Real usedtime = GetElapsedTime();

	if (InjectPos[0] == CORRECT)
	{
		COnLineGradeMgr::Instance()->SendGrade("InjectPosCorrect", 0, usedtime);
	}
	else if (InjectPos[0] == BIAS)
	{
		COnLineGradeMgr::Instance()->SendGrade("InjectPosBias", 0, usedtime);
	}
	else if (InjectPos[0] == MISS)
	{
		COnLineGradeMgr::Instance()->SendGrade("InjectPosError", 0, usedtime);
	}

	if (WithDrawPos[0] == CORRECT)
	{
		COnLineGradeMgr::Instance()->SendGrade("WithdrawPosCorrect", 0, usedtime);
	}
	else if (WithDrawPos[0] == BIAS)
	{
		COnLineGradeMgr::Instance()->SendGrade("WithdrawPosBias", 0, usedtime);
	}
	else if (WithDrawPos[0] == MISS)
	{
		COnLineGradeMgr::Instance()->SendGrade("WithdrawPosError", 0, usedtime);
	}
	//////////////////////////////////////////////////////////////////////////
	if (InjectPos[1] == CORRECT)
	{
		COnLineGradeMgr::Instance()->SendGrade("2InjectPosCorrect", 0, usedtime);
	}
	else if (InjectPos[1] == BIAS)
	{
		COnLineGradeMgr::Instance()->SendGrade("2InjectPosBias", 0, usedtime);
	}
	else if (InjectPos[1] == MISS)
	{
		COnLineGradeMgr::Instance()->SendGrade("2InjectPosError", 0, usedtime);
	}

	if (WithDrawPos[1] == CORRECT)
	{
		COnLineGradeMgr::Instance()->SendGrade("2WithdrawPosCorrect", 0, usedtime);
	}
	else if (WithDrawPos[1] == BIAS)
	{
		COnLineGradeMgr::Instance()->SendGrade("2WithdrawPosBias", 0, usedtime);
	}
	else if (WithDrawPos[1] == MISS)
	{
		COnLineGradeMgr::Instance()->SendGrade("2WithdrawPosError", 0, usedtime);
	}


	if (m_bAdjust_Tail[0])
	{
		COnLineGradeMgr::Instance()->SendGrade("Adjust_Tail_Length", 0, usedtime);
	}
	else
	{
		COnLineGradeMgr::Instance()->SendGrade("NoAdjust_Tail_Length", 0, usedtime);
	}

	if (m_bAdjust_Tail[1])
	{
		COnLineGradeMgr::Instance()->SendGrade("2Adjust_Tail_Length", 0, usedtime);
	}
	else
	{
		COnLineGradeMgr::Instance()->SendGrade("2NoAdjust_Tail_Length", 0, usedtime);
	}

	if (m_bScoreKnot[0])
	{
		COnLineGradeMgr::Instance()->SendGrade("FirstHalfKnotRoundSuccess", 0, usedtime);
	}
	else
	{
		COnLineGradeMgr::Instance()->SendGrade("FirstHalfKnotRoundFail", 0, usedtime);
	}

	if (m_bScoreKnot[1])
	{
		COnLineGradeMgr::Instance()->SendGrade("SecondHalfKnotRoundSuccess", 0, usedtime);
	}
	else
	{
		COnLineGradeMgr::Instance()->SendGrade("SecondHalfKnotRoundFail", 0, usedtime);
	}


	if (m_bScoreKnot[2])
	{
		COnLineGradeMgr::Instance()->SendGrade("2FirstHalfKnotRoundSuccess", 0, usedtime);
	}
	else
	{
		COnLineGradeMgr::Instance()->SendGrade("2FirstHalfKnotRoundFail", 0, usedtime);
	}

	if (m_bScoreKnot[3])
	{
		COnLineGradeMgr::Instance()->SendGrade("2SecondHalfKnotRoundSuccess", 0, usedtime);
	}
	else
	{
		COnLineGradeMgr::Instance()->SendGrade("2SecondHalfKnotRoundFail", 0, usedtime);
	}

	GFPhysVectorObj<KnotInSutureRopeV2> Knots;

	for (int i = 0; i < 2; i++)
	{
		if (m_Needle[i])
		{
			m_Needle[i]->GetSutureThread()->m_KnotsInThread->GetDeadKnots(Knots);

			if (Knots.size() == 1)
			{
				COnLineGradeMgr::Instance()->SendGrade(Ogre::StringConverter::toString(i + 1) + "FirstHalfKnotGotten", 0, usedtime);
				COnLineGradeMgr::Instance()->SendGrade(Ogre::StringConverter::toString(i + 1) + "SecondHalfKnotFail", 0, usedtime);
			}
			else if (Knots.size() == 2)
			{
				COnLineGradeMgr::Instance()->SendGrade(Ogre::StringConverter::toString(i + 1) + "FirstHalfKnotGotten", 0, usedtime);
				COnLineGradeMgr::Instance()->SendGrade(Ogre::StringConverter::toString(i + 1) + "SecondHalfKnotGotten", 0, usedtime);
				//打结正确or错误
				if (Knots[0].m_Clockwise == Knots[1].m_Clockwise)
				{
					COnLineGradeMgr::Instance()->SendGrade(Ogre::StringConverter::toString(i + 1) + "TieCorrect", 0, usedtime);
				}
				else
				{
					COnLineGradeMgr::Instance()->SendGrade(Ogre::StringConverter::toString(i + 1) + "TieError", 0, usedtime);
				}
			}
			else/* if (Knots.size() == 0)*/
			{
				COnLineGradeMgr::Instance()->SendGrade(Ogre::StringConverter::toString(i + 1) + "FirstHalfKnotFail", 0, usedtime);
				COnLineGradeMgr::Instance()->SendGrade(Ogre::StringConverter::toString(i + 1) + "SecondHalfKnotFail", 0, usedtime);
			}
			Knots.clear();
		}
	}


	//针掉落的次数    
	if (m_NeedleFallCount == 0)
	{
		COnLineGradeMgr::Instance()->SendGrade("NeedleFallCount0", 0, usedtime);
	}
	else  if (m_NeedleFallCount == 1)
	{
		COnLineGradeMgr::Instance()->SendGrade("NeedleFallCount1", 0, usedtime);
	}
	else if (m_NeedleFallCount >= 2)
	{
		COnLineGradeMgr::Instance()->SendGrade("NeedleFallCount2", 0, usedtime);
	}

	Real leftToolSpeed = m_pToolsMgr->GetLeftToolMovedSpeed();
	Real rightTooSpeed = m_pToolsMgr->GetRightToolMovedSpeed();
	Real ToolSpeed = std::max(leftToolSpeed, rightTooSpeed);

	if (ToolSpeed <= 5.0f && ToolSpeed > GP_EPSILON)
		COnLineGradeMgr::Instance()->SendGrade("MachineHandle_Normal", 0, usedtime);
	else if (ToolSpeed > 5.0f && ToolSpeed <= 10.0f)
		COnLineGradeMgr::Instance()->SendGrade("MachineHandle_Fast", 0, usedtime);
	else if (ToolSpeed > 10.0f)
		COnLineGradeMgr::Instance()->SendGrade("MachineHandle_TooFast", 0, usedtime);


	if (m_pToolsMgr->ToolIsClosedInsertion() && m_bTrainingIlluminated)
	{
		COnLineGradeMgr::Instance()->SendGrade("MachineHandle_Close", 0, usedtime);
	}
	else
	{
		COnLineGradeMgr::Instance()->SendGrade("MachineHandle_Open", 0, usedtime);
	}

	if (m_bFinished)
	{
		if (usedtime < 360)
		{
			COnLineGradeMgr::Instance()->SendGrade("Finished_In6M", 0, usedtime);
			if (usedtime <= 240)
			{
				COnLineGradeMgr::Instance()->SendGrade("Twohands_Cooperation", 0, usedtime);
			}
		}
		else if (usedtime >= 360)
		{
			COnLineGradeMgr::Instance()->SendGrade("Finished_In10M", 0, usedtime);
		}
	}
	else
	{
		COnLineGradeMgr::Instance()->SendGrade("UnFinish_In10M", 0, usedtime);
	}
	__super::OnSaveTrainingReport();
}