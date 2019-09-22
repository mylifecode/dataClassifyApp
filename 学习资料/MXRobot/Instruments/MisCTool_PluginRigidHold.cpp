#include "MisCTool_PluginRigidHold.h"
#include "PhysicsWrapper.h"
#include "MisNewTraining.h"
#include "SutureThreadV2.h"
#define USEPOSITIONSOLVER 0
#define RELEASELOCK 1

void MisCTool_PluginRigidHold::UpdateRigidClampRegions()
{
	m_RigidClampSpaceCells.clear();

	GFPhysAlignedVectorObj<GFPhysVector3> Reg0TriVerts;
	GFPhysAlignedVectorObj<GFPhysVector3> Reg1TriVerts;

	for (int r = 0; r < 2; r++)
	{
		ToolHoldRegion & reg = m_HoldReg[r];
		reg.UpdateToWorldSpace();

		if (m_IsRect)
		{
			GFPhysVector3 v00 = reg.m_CenterWorld + reg.m_Axis0World*reg.m_axis0Min + reg.m_Axis1World*reg.m_axis1Min;
			GFPhysVector3 v10 = reg.m_CenterWorld + reg.m_Axis0World*reg.m_axis0Max + reg.m_Axis1World*reg.m_axis1Min;
			GFPhysVector3 v11 = reg.m_CenterWorld + reg.m_Axis0World*reg.m_axis0Max + reg.m_Axis1World*reg.m_axis1Max;
			GFPhysVector3 v01 = reg.m_CenterWorld + reg.m_Axis0World*reg.m_axis0Min + reg.m_Axis1World*reg.m_axis1Max;

			if (r == 0)
			{
				Reg0TriVerts.push_back(v00);
				Reg0TriVerts.push_back(v10);
				Reg0TriVerts.push_back(v11);

				Reg0TriVerts.push_back(v00);
				Reg0TriVerts.push_back(v11);
				Reg0TriVerts.push_back(v01);
			}
			else
			{
				Reg1TriVerts.push_back(v00);
				Reg1TriVerts.push_back(v10);
				Reg1TriVerts.push_back(v11);

				Reg1TriVerts.push_back(v00);
				Reg1TriVerts.push_back(v11);
				Reg1TriVerts.push_back(v01);
			}
		}
		else
		{
			for (size_t c = 0; c < reg.m_triVertices.size(); c++)
			{
				GFPhysVector3 physPos = reg.m_Axis0World * reg.m_triVertices[c].x + reg.m_Axis1World * reg.m_triVertices[c].y;
				physPos = physPos + reg.m_CenterWorld;
				if (r == 0)
					Reg0TriVerts.push_back(physPos);
				else
					Reg1TriVerts.push_back(physPos);
			}
		}
	}

	//build every cell
	int cellNum = (Reg0TriVerts.size() <= Reg1TriVerts.size() ? Reg0TriVerts.size() : Reg1TriVerts.size());
	cellNum /= 3;
	for (int c = 0; c < cellNum; c++)
	{
		RigidClampCellData celldata;

		celldata.m_CellVertsWorldSpace[0] = Reg0TriVerts[c * 3];
		celldata.m_CellVertsWorldSpace[1] = Reg0TriVerts[c * 3 + 1];
		celldata.m_CellVertsWorldSpace[2] = Reg0TriVerts[c * 3 + 2];

		celldata.m_CellVertsWorldSpace[3] = Reg1TriVerts[c * 3];
		celldata.m_CellVertsWorldSpace[4] = Reg1TriVerts[c * 3 + 1];
		celldata.m_CellVertsWorldSpace[5] = Reg1TriVerts[c * 3 + 2];


		GFPhysVector3 center;
		GFPhysMatrix3 rotMat;
		GFPhysVector3 halfextend;

		bool succed = CalConvexHullBestFitFrame(celldata.m_CellVertsWorldSpace, 6, center, rotMat, halfextend);
		if (succed)
		{
			celldata.m_localmin = -halfextend;

			celldata.m_localmax = halfextend;

			celldata.m_transform = GFPhysTransform(rotMat, center);

			GFPhysTransform invTrans = celldata.m_transform.Inverse();

			//transform back
			celldata.m_CellVertsReg0[0] = invTrans(celldata.m_CellVertsWorldSpace[0]);
			celldata.m_CellVertsReg0[1] = invTrans(celldata.m_CellVertsWorldSpace[1]);
			celldata.m_CellVertsReg0[2] = invTrans(celldata.m_CellVertsWorldSpace[2]);

			celldata.m_CellVertsReg1[0] = invTrans(celldata.m_CellVertsWorldSpace[3]);
			celldata.m_CellVertsReg1[1] = invTrans(celldata.m_CellVertsWorldSpace[4]);
			celldata.m_CellVertsReg1[2] = invTrans(celldata.m_CellVertsWorldSpace[5]);

			//
			GFPhysVector3 testmin(FLT_MAX, FLT_MAX, FLT_MAX);
			GFPhysVector3 testmax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

			for (int t = 0; t < 3; t++)
			{
				testmin.SetMin(celldata.m_CellVertsReg0[t]);
				testmax.SetMax(celldata.m_CellVertsReg0[t]);

				testmin.SetMin(celldata.m_CellVertsReg1[t]);
				testmax.SetMax(celldata.m_CellVertsReg1[t]);
			}

			//
		}
		else
		{
			celldata.m_transform.SetIdentity();

			celldata.m_CellVertsReg0[0] = celldata.m_CellVertsWorldSpace[0];
			celldata.m_CellVertsReg0[1] = celldata.m_CellVertsWorldSpace[1];
			celldata.m_CellVertsReg0[2] = celldata.m_CellVertsWorldSpace[2];

			celldata.m_CellVertsReg1[0] = celldata.m_CellVertsWorldSpace[3];
			celldata.m_CellVertsReg1[1] = celldata.m_CellVertsWorldSpace[4];
			celldata.m_CellVertsReg1[2] = celldata.m_CellVertsWorldSpace[5];

			celldata.m_localmin = celldata.m_localmax = celldata.m_CellVertsWorldSpace[0];

			for (int t = 1; t < 6; t++)
			{
				celldata.m_localmin.SetMin(celldata.m_CellVertsWorldSpace[t]);
				celldata.m_localmax.SetMax(celldata.m_CellVertsWorldSpace[t]);
			}
		}

		//save inverse trans for further use
		celldata.m_InvTrans = celldata.m_transform.Inverse();

		//add expand region thickness in to account
		GFPhysVector3 clampThick(m_RigidClampRegThickNess, m_RigidClampRegThickNess, m_RigidClampRegThickNess);
		celldata.m_localmin -= clampThick;
		celldata.m_localmax += clampThick;

		//
		m_RigidClampSpaceCells.push_back(celldata);
	}
}

void MisCTool_PluginRigidHold::ToolHoldRegion::UpdateToWorldSpace()
{
	const GFPhysTransform & worldtrans = m_AttachRigid->GetWorldTransform();
	const GFPhysMatrix3 & worldRotate = worldtrans.GetBasis();

	//m_Masscenter = worldtrans.GetOrigin();
	m_CenterWorldPrev = m_CenterWorld;

	//update world axis and normal etc
	m_Axis0World = worldRotate*m_Axis0Local;
	m_Axis1World = worldRotate*m_Axis1Local;
	m_CenterWorld = worldtrans*m_CenterLocal;

	m_HoldNormalWorld = m_Axis0World.Cross(m_Axis1World).Normalized() * m_normalSign;
}

void MisCTool_PluginRigidHold::SetHoldRegion(NewTrainToolConvexData &  attachRigid,
	const GFPhysVector3 & center,
	const GFPhysVector3 & axis0,
	const GFPhysVector3 & axis1,
	Ogre::Vector2 triVertices[], 
	int		numVertices,
	HoldRegSide regSide,
	Real    normalSign)
{
	GFPhysVector3 invOffset(0, 0, 0);
	GFPhysMatrix3 invRot;
	invRot.SetIdentity();

	if (attachRigid.m_CompoundShape == 0 && attachRigid.m_CollideShapesData.size() > 0)//tool may refit convex vertex , compound shape not involve since they are in child transform
	{
		if (attachRigid.m_CollideShapesData[0].m_ShapeType == 1)
		{
			invOffset = -(OgreToGPVec3(attachRigid.m_CollideShapesData[0].m_boxcenter));

			GFPhysQuaternion quat = (OgreToGPQuaternion(attachRigid.m_CollideShapesData[0].m_boxrotate));

			invRot.SetRotation(quat.Inverse());
		}
	}
	m_HoldReg[regSide].m_AttachRigid = attachRigid.m_rigidbody;
	m_HoldReg[regSide].m_CenterLocal = (invRot * (center + invOffset));//center;

	m_HoldReg[regSide].m_Axis0Local = invRot*axis0;
	m_HoldReg[regSide].m_Axis1Local = invRot*axis1;
	m_HoldReg[regSide].m_normalSign = normalSign;	

	m_HoldReg[regSide].m_RegSide = regSide;
	m_IsRect = false;

	for (int i = 0; i < numVertices; ++i)
	{
		m_HoldReg[regSide].m_triVertices.push_back(triVertices[i]);
		m_HoldReg[regSide].m_axis0Min = min(m_HoldReg[regSide].m_axis0Min, triVertices[i].x);
		m_HoldReg[regSide].m_axis0Max = max(m_HoldReg[regSide].m_axis0Max, triVertices[i].x);
		m_HoldReg[regSide].m_axis1Min = min(m_HoldReg[regSide].m_axis1Min, triVertices[i].y);
		m_HoldReg[regSide].m_axis1Max = max(m_HoldReg[regSide].m_axis1Max, triVertices[i].y);
	}

	m_HoldReg[regSide].UpdateToWorldSpace();

}

MisCTool_PluginRigidHold::MisCTool_PluginRigidHold(CTool * tool) : MisMedicCToolPluginInterface(tool)
{
	m_minShaftAside = 10.0f;
	//m_holding = false;
	m_relRot.SetIdentity();
	m_RigidMask = 0;
	m_RigidBodyBeHold = NULL;
	m_RigidBodyTool = NULL;
	m_localPointOther = GFPhysVector3(0.0f, 0.0f, 0.0f);
	m_localPointTool = GFPhysVector3(0.0f, 0.0f, 0.0f);
	//m_clampedNewObj = false;
    m_fixjoint = new GFPhysFixJoint(m_RigidBodyBeHold, m_RigidBodyTool);
	m_IsOpenLargeEnough = false;
	m_OpenShaftAisde = 1.0f;	
}
//==================================================================================================
MisCTool_PluginRigidHold::~MisCTool_PluginRigidHold()
{
    //fix bug 夹住针的时候切换器械无法碰撞
    if (m_RigidBodyBeHold)
    {
		ReleaseHoldingRigid();
    }
}
//==================================================================================================
void MisCTool_PluginRigidHold::OnRigidBodyBeRemovedFromWorld(GFPhysRigidBody * rigidbody)
{	
	if (m_RigidBodyBeHold == rigidbody)
	{
		ReleaseHoldingRigid();
	}
}

void MisCTool_PluginRigidHold::OneFrameUpdateEnded()
{
	MisNewTraining * misTrain = dynamic_cast<MisNewTraining*>(m_ToolObject->GetOwnerTraining());
	


	if (m_IsOpenLargeEnough == false)
	{
		float shaftTool = m_ToolObject->GetShaftAside();
		if (shaftTool >= m_OpenShaftAisde)
		{
			m_IsOpenLargeEnough = true;
		}
	}

	const std::vector<SutureNeedle *> & Needles = misTrain->GetSutureNeedles();
	if (m_RigidBodyBeHold)//check if a more recently other tool has clamped needle I clamped if so i should release
    {
		SutureNeedle * holdedNeedle = 0;
        for (size_t c = 0; c < Needles.size(); c++)
        {
			if (Needles[c]->GetPhysicBody() == m_RigidBodyBeHold
			 && Needles[c]->GetRecentClampTool() != m_ToolObject)
			{
				holdedNeedle = Needles[c];
				break;
			}
        }
		if (holdedNeedle)
		{
			ReleaseHoldingRigid();
			m_OpenShaftAisde = m_ToolObject->GetShaftAside() + 2.0f;
			m_IsOpenLargeEnough = false;
		}
    }
    
	const std::vector<SutureNeedleV2 *> & NeedlesV2 = misTrain->GetSutureNeedlesV2();
	if (m_RigidBodyBeHold)//check if a more recently other tool has clamped needle I clamped if so i should release
	{
		SutureNeedleV2 * holdedNeedle = 0;
		for (size_t c = 0; c < NeedlesV2.size(); c++)
		{
			if (NeedlesV2[c]->GetPhysicBody() == m_RigidBodyBeHold
				&& NeedlesV2[c]->GetRecentClampTool() != m_ToolObject)
			{
				holdedNeedle = NeedlesV2[c];
				break;
			}
		}
		if (holdedNeedle)
		{
			ReleaseHoldingRigid();
			m_OpenShaftAisde = m_ToolObject->GetShaftAside() + 2.0f;
			m_IsOpenLargeEnough = false;
		}
	}
}
//-----------------------------------------------------------------------------------
bool MisCTool_PluginRigidHold::CheckRigidBodyBeHold()
{
	
#if 1

	for (size_t f = 0; f < m_ToolObject->m_ToolCollidedRigids.size(); f++)
	{
		const ToolCollideRigidBodyPoint & datacd = m_ToolObject->m_ToolCollidedRigids[f];
		GFPhysRigidBody * rigidbody = datacd.m_collideRigidOther;

		if (COMPOUND_SHAPE_PROXYTYPE == rigidbody->GetCollisionShape()->GetShapeType())
		{
			//直接调用抓取区域碰撞检测的细测过程，检测是否和针的某几段碰撞
			if (CalculateNeedleSegmentsInClampRegions() || CalculateNeedleV2SegmentsInClampRegions())
			{
				m_RigidBodyBeHold = datacd.m_collideRigidOther;//待改进，只支持一个刚体被抓
				m_RigidBodyTool = datacd.m_collideRigidTool;
				m_localPointOther = datacd.m_localPointOther;
				m_localPointTool = datacd.m_localPointTool;

				GFPhysTransform trans = m_RigidBodyBeHold->GetWorldTransform();

				GFPhysQuaternion clampBodyRot = trans.GetRotation();
				GFPhysQuaternion invToolRot = m_RigidBodyTool->GetWorldTransform().GetRotation().Inverse();

				m_relRot = invToolRot*clampBodyRot;

				m_RigidMask = m_RigidBodyBeHold->m_MaskBits;

				uint32 toolcat = (m_ToolObject->GetToolSide() == ITool::TSD_LEFT ? MMRC_LeftTool : MMRC_RightTool);
				m_RigidBodyBeHold->SetCollisionMask(m_RigidMask & (~toolcat));

				float clampopenAngle = 4.0f;
				if (m_ToolObject->GetShaftAside() > clampopenAngle)
				{
					clampopenAngle = m_ToolObject->GetShaftAside();
				}

				m_ToolObject->SetMinShaftAside(clampopenAngle);
				m_ToolObject->AssignShaftValueDirectly(clampopenAngle);

				MisNewTraining * misTrain = dynamic_cast<MisNewTraining*>(m_ToolObject->GetOwnerTraining());

				const std::vector<SutureNeedle *> & Needles = misTrain->GetSutureNeedles();

				for (size_t c = 0; c < Needles.size(); c++)
				{
					SutureThread* attachrope = Needles[c]->GetSutureThread();
					if (attachrope && attachrope->m_islock)
					{
#if RELEASELOCK
						int num = attachrope->GetNumSegments();
						attachrope->UnLockRopeNode(num);
						attachrope->m_islock = false;
#endif
					}

					if (Needles[c]->GetPhysicBody() == m_RigidBodyBeHold)
					{
						Needles[c]->AddClampTool(m_ToolObject);
						break;
					}
				}

				//////////////////////////////////////////////////////////////////////////

				const std::vector<SutureNeedleV2 *> & NeedlesV2 = misTrain->GetSutureNeedlesV2();

				for (size_t c = 0; c < NeedlesV2.size(); c++)
				{
					SutureThreadV2* attachrope = NeedlesV2[c]->GetSutureThread();
					if (attachrope && attachrope->m_islock)
					{
#if RELEASELOCK
						int num = attachrope->GetNumSegments();
						attachrope->ReleaseNodeAsFix(num - 1);
						attachrope->m_islock = false;
#endif
					}

					if (NeedlesV2[c]->GetPhysicBody() == m_RigidBodyBeHold)
					{
						NeedlesV2[c]->AddClampTool(m_ToolObject);
						break;
					}
				}

				//m_clampedNewObj = true;
				//train call back

				if (misTrain)
				{
					misTrain->OnRigidClampByTool(m_RigidBodyBeHold);
				}

				//build clamp joint
				m_fixjoint->m_RigidBodyA = m_RigidBodyBeHold;
				m_fixjoint->m_RigidBodyB = m_RigidBodyTool;

				Real TimeStep = 1.0f / 120.0f;
				Real kspring = 10.f;
				Real kdamping = 1.0f;

				//m_fixjoint->SetErp(TimeStep * kspring / (TimeStep*kspring + kdamping));
				m_fixjoint->SetLinearcfm(0.0f);// 1.0f / (TimeStep*kspring + kdamping));
				m_fixjoint->SetAngularcfm(0.0025f); //(1.0f / (TimeStep*kspring + kdamping));
				//m_fixjoint->SetAngularSoftness(0.1f);

				GFPhysVector3 worldAnchorTool = m_RigidBodyTool->GetWorldTransform() * m_localPointTool;
				GFPhysMatrix3 worldRotateTool = m_RigidBodyTool->GetWorldTransform().GetBasis();
				m_fixjoint->SetAnchorAndRotate(worldAnchorTool, worldAnchorTool, worldRotateTool, worldRotateTool);

				m_RigidBodyBeHold->SetBeStaticWhenMeetSoft(true);
				PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddJointConstraint(m_fixjoint, true);

				//float mass = m_RigidBodyBeHold->GetInvMass();
				//GFPhysVector3 intertial = m_RigidBodyBeHold->GetLocalInvInertiaTensor();

				//m_RigidBodyBeHold->SetinvInertiaTensorWorldFactor(0.01f);
				//m_RigidBodyBeHold->SetinvMassScale(0.01f);
				//m_holding = true;
				return true;
			}
		}
	}
	return false;

#else



	if (m_ToolObject->m_ToolCollidedRigids.size() != 0)
	{
		std::map<GFPhysRigidBody*, std::pair<int, int>> rigidBeHold;

		for (size_t i = 0; i < m_ToolObject->m_ToolCollidedRigids.size(); i++)
		{
			const ToolCollideRigidBodyPoint & datacd = m_ToolObject->m_ToolCollidedRigids[i];
			GFPhysRigidBody * rigidbody = datacd.m_collideRigidTool;
			GFPhysVector3 dircollide = datacd.m_normalWorldOnTool;

			int k = m_ToolObject->GetRigidBodyPart(rigidbody);
			if (k == 0)//LEFT
			{
				Real dotleft = dircollide.Dot(m_HoldReg[1].m_HoldNormalWorld);
				if (dotleft > 0.9f)
				{
					rigidBeHold[datacd.m_collideRigidOther].first++;
				}
			}
			else if (k == 1)//RIGHT
			{
				Real dotright = dircollide.Dot(m_HoldReg[0].m_HoldNormalWorld);
				if (dotright > 0.9f)
				{
					rigidBeHold[datacd.m_collideRigidOther].second++;
				}
			}
			if (rigidBeHold[datacd.m_collideRigidOther].first > 0 && rigidBeHold[datacd.m_collideRigidOther].second > 0)
			{
				m_RigidBodyBeHold = datacd.m_collideRigidOther;//待改进，只支持一个刚体被抓
				m_RigidBodyTool = datacd.m_collideRigidTool;
				m_localPointOther = datacd.m_localPointOther;
				m_localPointTool = datacd.m_localPointTool;
				
				GFPhysTransform trans = m_RigidBodyBeHold->GetWorldTransform();

				GFPhysQuaternion clampBodyRot = trans.GetRotation();
				GFPhysQuaternion invToolRot = m_RigidBodyTool->GetWorldTransform().GetRotation().Inverse();

				m_relRot = invToolRot*clampBodyRot;

				m_RigidMask = m_RigidBodyBeHold->m_MaskBits;

				uint32 toolcat = (m_ToolObject->GetToolSide() == ITool::TSD_LEFT ? MMRC_LeftTool : MMRC_RightTool);
				m_RigidBodyBeHold->SetCollisionMask(m_RigidMask & (~toolcat));

				float clampopenAngle = 4.0f;
				if (m_ToolObject->GetShaftAside() > clampopenAngle)
				{
					clampopenAngle = m_ToolObject->GetShaftAside();
				}

				m_ToolObject->SetMinShaftAside(clampopenAngle);
				m_ToolObject->AssignShaftValueDirectly(clampopenAngle);

				MisNewTraining * misTrain = dynamic_cast<MisNewTraining*>(m_ToolObject->GetOwnerTraining());

				const std::vector<SutureNeedle *> & Needles = misTrain->GetSutureNeedles();

				for (size_t c = 0; c < Needles.size(); c++)
				{
					SutureThread* attachrope = Needles[c]->GetSutureThread();
					if (attachrope && attachrope->m_islock)
                    {
#if RELEASELOCK
						int num = attachrope->GetNumSegments();
						attachrope->UnLockRopeNode(num);
						attachrope->m_islock = false;
#endif
                    }

					if (Needles[c]->GetPhysicBody() == m_RigidBodyBeHold)
					{
						Needles[c]->AddClampTool(m_ToolObject);
						break;
					}
				}

				//////////////////////////////////////////////////////////////////////////

				const std::vector<SutureNeedleV2 *> & NeedlesV2 = misTrain->GetSutureNeedlesV2();

				for (size_t c = 0; c < NeedlesV2.size(); c++)
				{
					SutureThreadV2* attachrope = NeedlesV2[c]->GetSutureThread();
					if (attachrope && attachrope->m_islock)
					{
#if RELEASELOCK
						int num = attachrope->GetNumSegments();
						attachrope->ReleaseNodeAsFix(num-1);
						attachrope->m_islock = false;
#endif
					}

					if (NeedlesV2[c]->GetPhysicBody() == m_RigidBodyBeHold)
					{
						NeedlesV2[c]->AddClampTool(m_ToolObject);
						break;
					}
				}

				//m_clampedNewObj = true;
                //train call back

                if (misTrain)
                {
                    misTrain->OnRigidClampByTool(m_RigidBodyBeHold);
                }

                //build clamp joint
                m_fixjoint->m_RigidBodyA = m_RigidBodyBeHold;
                m_fixjoint->m_RigidBodyB = m_RigidBodyTool;

                Real TimeStep = 1.0f / 120.0f;
                Real kspring = 10.f;
                Real kdamping = 1.0f;

                //m_fixjoint->SetErp(TimeStep * kspring / (TimeStep*kspring + kdamping));
				m_fixjoint->SetLinearcfm(0.0f);// 1.0f / (TimeStep*kspring + kdamping));
				m_fixjoint->SetAngularcfm(0.0025f); //(1.0f / (TimeStep*kspring + kdamping));
				//m_fixjoint->SetAngularSoftness(0.1f);

                GFPhysVector3 worldAnchorTool = m_RigidBodyTool->GetWorldTransform() * m_localPointTool;
                GFPhysMatrix3 worldRotateTool = m_RigidBodyTool->GetWorldTransform().GetBasis();
                m_fixjoint->SetAnchorAndRotate(worldAnchorTool, worldAnchorTool, worldRotateTool, worldRotateTool);
				
				PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddJointConstraint(m_fixjoint, true);
				//m_holding = true;
				return true;
			}
		}
	}
	return false;
#endif
}
//-----------------------------------------------------------------------------------
void MisCTool_PluginRigidHold::PhysicsSimulationStart(int currStep, int TotalStep, float dt)
{
	UpdateRigidClampRegions();

	MisNewTraining * misnewtrain = dynamic_cast<MisNewTraining*>(m_ToolObject->GetOwnerTraining());

	std::vector<SutureNeedleV2 *> & sutureNeedls = misnewtrain->GetSutureNeedlesV2();

	SutureNeedleV2 * trainNeedle = (sutureNeedls.size() > 0 ? sutureNeedls[0] : 0);
	
	if(trainNeedle)
	{
		if (m_RigidBodyBeHold == trainNeedle->GetPhysicBody())
		{
			int numStaticCollid = 0;

			std::set<GFPhysCollideObject*>::iterator itor = trainNeedle->m_CollidedRigid.begin();

			while (itor != trainNeedle->m_CollidedRigid.end())
			{
				if (m_ToolObject->GetRigidBodyPart(*itor) == -1)
				{
					numStaticCollid++;
				}
				itor++;
			}
	
			if (numStaticCollid > 0)
			{
				m_fixjoint->SetMaxAngularJointForce(500.0f);
			}
			else
			{
				m_fixjoint->SetMaxAngularJointForce(FLT_MAX);
			}
		}
	}
	
	
	//m_painttool.PushBackPoint(CustomPoint(&(m_HoldReg[0].m_Masscenter + 0.5f * m_HoldReg[0].m_HoldNormalWorld), Ogre::ColourValue::Blue, 0.1f));
	//m_painttool.PushBackPoint(CustomPoint(&(m_HoldReg[1].m_Masscenter + 0.5f * m_HoldReg[1].m_HoldNormalWorld), Ogre::ColourValue::Red, 0.1f));

	//m_painttool.Update(dt);

	//m_painttool.ClearPoints();
}
//-----------------------------------------------------------------------------------
void MisCTool_PluginRigidHold::PhysicsSimulationEnd(int currStep, int TotalStep, float dt)
{
	float shaftTool = m_ToolObject->GetShaftAside();

	//if (m_holding)
	//{
	if (shaftTool > m_minShaftAside && m_RigidBodyBeHold)
	{
		ReleaseHoldingRigid();
	}
	//}
	else if (shaftTool <= m_minShaftAside && !m_RigidBodyBeHold && m_IsOpenLargeEnough)
	{
		CheckRigidBodyBeHold();
	}
}
//-----------------------------------------------------------------------------------
void MisCTool_PluginRigidHold::ReleaseHoldingRigid()
{
	if (m_RigidBodyBeHold == 0)
		return;

	MisNewTraining * misTrain = dynamic_cast<MisNewTraining*>(m_ToolObject->GetOwnerTraining());

	if (misTrain)
	{
		misTrain->OnRigidReleaseByTool(m_RigidBodyBeHold);

		//sep check for needle
		const std::vector<SutureNeedle *> & Needles = misTrain->GetSutureNeedles();

		for (size_t c = 0; c < Needles.size(); c++)
		{
			if (Needles[c]->GetPhysicBody() == m_RigidBodyBeHold)
			{
				Needles[c]->RemoveClampTool(m_ToolObject);				
			}
		}

		const std::vector<SutureNeedleV2 *> & NeedlesV2 = misTrain->GetSutureNeedlesV2();

		for (size_t c = 0; c < NeedlesV2.size(); c++)
		{
			if (NeedlesV2[c]->GetPhysicBody() == m_RigidBodyBeHold)
			{
				NeedlesV2[c]->RemoveClampTool(m_ToolObject);				
			}
		}
	}

	if (PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
	{
		//PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(this);
    	PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemoveJointConstraint(m_fixjoint);
    }
	m_RigidBodyBeHold->SetBeStaticWhenMeetSoft(false);
	///m_RigidBodyBeHold->SetinvInertiaTensorWorldFactor(1.0f);
	//m_RigidBodyBeHold->SetinvMassScale(1.0f);
	//m_clampedNewObj = false;

	m_RigidMask = m_RigidBodyBeHold->m_MaskBits;
	uint32 toolcat = (m_ToolObject->GetToolSide() == ITool::TSD_LEFT ? MMRC_LeftTool : MMRC_RightTool);
	m_RigidBodyBeHold->SetCollisionMask(m_RigidMask |= toolcat);
	
	m_RigidBodyBeHold = NULL;
	m_RigidBodyTool = NULL;
	//m_holding = false;

	m_ToolObject->SetMinShaftAside(0.0f);
}
//-----------------------------------------------------------------------------------
const MisCTool_PluginRigidHold::ToolHoldRegion & MisCTool_PluginRigidHold::GetHoldRegion(int index)
{
	return m_HoldReg[index];
}
//-----------------------------------------------------------------------------------
GFPhysVector3 MisCTool_PluginRigidHold::GetPluginForceFeedBack()
{
	GFPhysVector3 force(0, 0, 0);
	if (m_RigidBodyBeHold)//need refractory
	{
		MisNewTraining * misTrain = dynamic_cast<MisNewTraining*>(m_ToolObject->GetOwnerTraining());

		const std::vector<SutureNeedle *> & Needles = misTrain->GetSutureNeedles();
		
		Real NeedleMassPerSegment = 0.25f * 0.002f;
		for (size_t c = 0; c < Needles.size(); c++)
		{
			GFPhysVector3 NeedleGravity = Needles[c]->GetPhysicBody()->m_gravity;
			//针每段的重力
			for (int i = 0; i < Needles[c]->GetNeedleNodeNum(); i++)
			{
				force += NeedleMassPerSegment * NeedleGravity;
			}

			if (Needles[c]->GetPhysicBody() == m_RigidBodyBeHold)
			{
				force += Needles[c]->GetForceFeedBack()*40.0f;
				break;
			}
		}
	}
	return force;
}
//-----------------------------------------------------------------------------------

bool MisCTool_PluginRigidHold::CalculateNeedleSegmentsInClampRegions()
{
	ITraining *currtrain = m_ToolObject->GetOwnerTraining();
	MisNewTraining * pMisNewTraining = dynamic_cast<MisNewTraining*>(currtrain);
	if (pMisNewTraining)
	{
		std::vector<SutureNeedle *> & Needles = pMisNewTraining->GetSutureNeedles();
		Real seperateDist = 0.05f;
		bool isFound = false;
		for (int i = 0, ni = Needles.size(); i < ni; i++)
		{
			SutureNeedle* currNeedle = Needles[i];

			if (currNeedle->GetPhysicBody() == NULL)
			{
				continue;
			}
			//遍历针的每一段看是否在夹闭区域内
			
			for (int j = 0; j < currNeedle->m_NeedleNodeWorldPos.size() - 1; j++)
			{
				GFPhysVector3 VertsWorld[2];
				VertsWorld[0] = currNeedle->m_NeedleNodeWorldPos[j];
				VertsWorld[1] = currNeedle->m_NeedleNodeWorldPos[j + 1];

				GFPhysTransform transReg;
				GFPhysTransform transTri;

				transReg.SetIdentity();
				transTri.SetIdentity();

				GFPhysVector3 ClampRegionVert[6];

				//test every pair of clamp region represent by triangle pair
				for (size_t c = 0; c < m_RigidClampSpaceCells.size(); c++)
				{
					ClampRegionVert[0] = m_RigidClampSpaceCells[c].m_CellVertsWorldSpace[0];
					ClampRegionVert[1] = m_RigidClampSpaceCells[c].m_CellVertsWorldSpace[1];
					ClampRegionVert[2] = m_RigidClampSpaceCells[c].m_CellVertsWorldSpace[2];
										 
					ClampRegionVert[3] = m_RigidClampSpaceCells[c].m_CellVertsWorldSpace[3];
					ClampRegionVert[4] = m_RigidClampSpaceCells[c].m_CellVertsWorldSpace[4];
					ClampRegionVert[5] = m_RigidClampSpaceCells[c].m_CellVertsWorldSpace[5];


					GFPhysVector3 closetPointReg;
					GFPhysVector3 closetPointTri;

					float closetDist = GetConvexsClosetPoint(ClampRegionVert, 6, 0,
						VertsWorld, 2, 0,
						transReg, transTri,
						closetPointReg, closetPointTri,
						seperateDist + 0.1f);

					if (closetDist <= seperateDist)//0)//consider triangle in contact with region
					{
						isFound = true;
					}

					if (isFound)
						return isFound;
				}
			}			
		}
		return isFound;
	}
}
//-----------------------------------------------------------------------------------

bool MisCTool_PluginRigidHold::CalculateNeedleV2SegmentsInClampRegions()
{
	ITraining *currtrain = m_ToolObject->GetOwnerTraining();
	MisNewTraining * pMisNewTraining = dynamic_cast<MisNewTraining*>(currtrain);
	if (pMisNewTraining)
	{
		std::vector<SutureNeedleV2 *> & Needles = pMisNewTraining->GetSutureNeedlesV2();
		Real seperateDist = 0.05f;
		bool isFound = false;
		for (int i = 0, ni = Needles.size(); i < ni; i++)
		{
			SutureNeedleV2* currNeedle = Needles[i];
			//遍历针的每一段看是否在夹闭区域内
			if (currNeedle->GetPhysicBody() == NULL)
				continue;
			for (int j = 0; j < currNeedle->m_NeedleNodeWorldPos.size() - 1; j++)
			{
				GFPhysVector3 VertsWorld[2];
				VertsWorld[0] = currNeedle->m_NeedleNodeWorldPos[j];
				VertsWorld[1] = currNeedle->m_NeedleNodeWorldPos[j + 1];

				GFPhysTransform transReg;
				GFPhysTransform transTri;

				transReg.SetIdentity();
				transTri.SetIdentity();

				GFPhysVector3 ClampRegionVert[6];

				//test every pair of clamp region represent by triangle pair
				for (size_t c = 0; c < m_RigidClampSpaceCells.size(); c++)
				{
					ClampRegionVert[0] = m_RigidClampSpaceCells[c].m_CellVertsWorldSpace[0];
					ClampRegionVert[1] = m_RigidClampSpaceCells[c].m_CellVertsWorldSpace[1];
					ClampRegionVert[2] = m_RigidClampSpaceCells[c].m_CellVertsWorldSpace[2];

					ClampRegionVert[3] = m_RigidClampSpaceCells[c].m_CellVertsWorldSpace[3];
					ClampRegionVert[4] = m_RigidClampSpaceCells[c].m_CellVertsWorldSpace[4];
					ClampRegionVert[5] = m_RigidClampSpaceCells[c].m_CellVertsWorldSpace[5];


					GFPhysVector3 closetPointReg;
					GFPhysVector3 closetPointTri;

					float closetDist = GetConvexsClosetPoint(ClampRegionVert, 6, 0,
						VertsWorld, 2, 0,
						transReg, transTri,
						closetPointReg, closetPointTri,
						seperateDist + 0.1f);

					if (closetDist <= seperateDist)//0)//consider triangle in contact with region
					{
						isFound = true;
					}

					if (isFound)
						return isFound;
				}
			}
			
		}
		return isFound;
	}
}
