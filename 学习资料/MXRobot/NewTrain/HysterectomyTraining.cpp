#include "HysterectomyTraining.h"
#include "MisMedicOrganOrdinary.h"
#include "InputSystem.h"
#include "MXToolEvent.h"
#include "MisMedicBindedRope.h"
#include "TrainingCommon.h"
#include "MxOrganBindedEvent.h"
#include "Inception.h"
#include "DeferredRendFrameWork.h"
#include "qevent.h"
#include "CallBackForUnion.h"
WombManipulator::WombManipulator(MisMedicOrgan_Ordinary * womb, const GFPhysVector3 & rootPos, const GFPhysVector3 & headPos)
{
	m_Womb = womb;
	m_RootInitPos = rootPos;
	m_HeadInitPos = headPos;

	m_InitCoordVec[2] = m_CoordVec[2] = (m_HeadInitPos - m_RootInitPos).Normalized();
	m_InitCoordVec[0] = m_CoordVec[0] = Perpendicular(m_CoordVec[2]);
	m_InitCoordVec[1] = m_CoordVec[1] = m_CoordVec[0].Cross(m_CoordVec[2]).Normalized();
	m_Radius = 1.5f;

	PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(this);

}
WombManipulator::~WombManipulator()
{
	PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(this);
}

void WombManipulator::Rotate(const GFPhysQuaternion & rot)
{
	GFPhysVector3 rotatedCenterLine = QuatRotate(rot, m_InitCoordVec[2]).Normalized();

	GFPhysQuaternion shortRotQuat;

	GFPhysQuaternion shortQuatRot = ShortestArcQuat(m_InitCoordVec[2], rotatedCenterLine);

	m_CoordVec[0] = QuatRotate(shortQuatRot, m_InitCoordVec[0]);
	m_CoordVec[1] = QuatRotate(shortQuatRot, m_InitCoordVec[1]);
	m_CoordVec[2] = QuatRotate(shortQuatRot, m_InitCoordVec[2]);

}

void WombManipulator::PrepareSolveConstraint(Real Stiffness, Real dt)
{
	for (int c = 0; c < (int)m_TetrasManipulated.size(); c++)
	{
		NodeBeManipulated & unit = m_TetrasManipulated[c];
		unit.m_Lambda = 0;
	}
}
void WombManipulator::SolveConstraint(Real Stiffness, Real dt)
{
	for (int c = 0; c < (int)m_TetrasManipulated.size(); c++)
	{
		NodeBeManipulated & unit = m_TetrasManipulated[c];

		if (unit.m_Node == 0)
			continue;

		if (unit.m_PointLocalCoord[2] < 0)
			continue;

		if (unit.m_Node->m_insurface)
			continue;

		GFPhysVector3 pNode = unit.m_Node->m_CurrPosition;

		GFPhysVector3 pAttach = unit.m_PointLocalCoord[0] * m_CoordVec[0]
			+ unit.m_PointLocalCoord[1] * m_CoordVec[1]
			+ unit.m_PointLocalCoord[2] * m_CoordVec[2]
			+ m_RootInitPos;

#if(0)
		GFPhysVector3 Diff = (pNode - pAttach);

		float diffLen = Diff.Length();

		if (diffLen > FLT_EPSILON)
		{
			float stiffness = 0.01f;
			unit.m_Node->m_CurrPosition -= Diff * stiffness;
		}
#else
		float Sor = 0.1f;

		float PhysStiff = 500.0f;

		float damping = 0.0f;

		float InvStiff = 1.0f / PhysStiff;
		InvStiff /= (dt*dt);

		Real w = unit.m_Node->m_InvM;

		GFPhysVector3 Grad = unit.m_Node->m_CurrPosition - pAttach;
		Real Length = Grad.Length();

		if (Length > GP_EPSILON)
			Grad /= Length;
		else
			Grad = GFPhysVector3(0, 0, 0);

		Real gamma = damping / dt;

		Real denom = (w > GP_EPSILON ? 1.0f / ((1 + gamma) * w + InvStiff) : 0.0f);

		Real GradDotVelDt = Grad.Dot(unit.m_Node->m_CurrPosition - unit.m_Node->m_LastPosition);

		Real dampingPart = gamma * GradDotVelDt;

		Real hi = Length + InvStiff * unit.m_Lambda + dampingPart;

		Real dLambda = (-hi) * denom * Sor;

		//update lambda
		unit.m_Lambda += dLambda;

		//update position
		unit.m_Node->m_CurrPosition += Grad * (w * dLambda);
	
#endif
	}

	for (int c = 0; c < m_ManipulatorCupPoints.size(); c++)
	{
		if (m_ManipulatorCupPoints[c].m_NodeInWomb)
		{
			GFPhysVector3 delta = m_ManipulatorCupPoints[c].m_NodeInM->m_CurrPosition - m_ManipulatorCupPoints[c].m_NodeInWomb->m_CurrPosition;
			m_ManipulatorCupPoints[c].m_NodeInWomb->m_CurrPosition += delta * 0.95f;
		}
	}
}
void WombManipulator::CalculateTetrasBeManipulated()
{
	if (m_Womb)
	{
		GFPhysSoftBodyShape & sbshape = m_Womb->m_physbody->GetSoftBodyShape();

		GFPhysVector3 TetraVerts[4];

		GFPhysVector3 closetPtInTetra;
		for (int c = 0; c < sbshape.GetNumTetrahedron(); c++)
		{
			GFPhysSoftBodyTetrahedron * tetra = sbshape.GetTetrahedronAtIndex(c);
			TetraVerts[0] = tetra->m_TetraNodes[0]->m_UnDeformedPos;
			TetraVerts[1] = tetra->m_TetraNodes[1]->m_UnDeformedPos;
			TetraVerts[2] = tetra->m_TetraNodes[2]->m_UnDeformedPos;
			TetraVerts[3] = tetra->m_TetraNodes[3]->m_UnDeformedPos;

			GFPhysVector3  pa, pb;
			bool intersected = LineSegmentTetraIntersect(m_RootInitPos, m_HeadInitPos, TetraVerts, pa, pb);

			bool choosed = false;

			if (intersected)
			{
				closetPtInTetra = (pa + pb) * 0.5f;
				choosed = true;
			}
			else
			{
				GFPhysVector3 faceVerts[3];

				GFPhysVector3 segVertPos[2];

				segVertPos[0] = m_RootInitPos;

				segVertPos[1] = m_HeadInitPos;

				float minDist = FLT_MAX;

				GFPhysVector3 closetPoint;
				for (int f = 0; f < 4; f++)
				{
					faceVerts[0] = TetraVerts[f];
					faceVerts[1] = TetraVerts[(f + 1) % 4];
					faceVerts[2] = TetraVerts[(f + 2) % 4];

					GFPhysVector3  pointOnTri;
					GFPhysVector3  pointOnSeg;
					GFPhysVector3  collideNormOnTri;

					Real dist = ClosetPtSegmentTriangle(faceVerts, segVertPos, pointOnTri, pointOnSeg, collideNormOnTri);

					if (dist < minDist)
					{
						minDist = dist;
						closetPoint = pointOnTri;
					}
				}

				if (minDist <= m_Radius)
				{
					closetPtInTetra = closetPoint;
					choosed = true;
				}
			}

			if (choosed)
			{
				for (int n = 0; n < 4; n++)
				{
					GFPhysSoftBodyNode * node = tetra->m_TetraNodes[n];

					bool existNode = false;

					for (int c = 0; c < (int)m_TetrasManipulated.size(); c++)
					{
						if (m_TetrasManipulated[c].m_Node == node)
						{
							existNode = true;
							break;
						}
					}
					if (existNode == false)
					{
						NodeBeManipulated unit;
						unit.m_Node = node;
						unit.m_PointLocalCoord[0] = (node->m_UnDeformedPos - m_RootInitPos).Dot(m_CoordVec[0]);
						unit.m_PointLocalCoord[1] = (node->m_UnDeformedPos - m_RootInitPos).Dot(m_CoordVec[1]);
						unit.m_PointLocalCoord[2] = (node->m_UnDeformedPos - m_RootInitPos).Dot(m_CoordVec[2]);

						m_TetrasManipulated.push_back(unit);
					}
				}
			}
		}

	}
}

void HysterectomyTraining::KeyPress(QKeyEvent * event)
{
	if (event->key() == Qt::Key_1)//B1C
	{
		m_wombManitor->Rotate(GFPhysQuaternion(GFPhysVector3(1, 0, 0), 3.1415926f / 4.0f));//test rotate 30 degree

	}
	else if (event->key() == Qt::Key_2)//B1C
	{
		m_wombManitor->Rotate(GFPhysQuaternion(GFPhysVector3(1, 0, 0), -3.1415926f / 4.0f));//test rotate 30 degree

	}
	else if (event->key() == Qt::Key_F12)
	{
		delete m_wombManitor;
		m_wombManitor = 0;

		if (m_ManiCup)
		{
			RemoveOrganFromWorld(m_ManiCup->GetOrganType());
			m_ManiCup = 0;
		}
		/*for (std::size_t i = 0; i < m_ObjAdhersions.size(); ++i)
		{
			MisMedicObjLink_Approach * curAdhersion = dynamic_cast<MisMedicObjLink_Approach*>(m_ObjAdhersions[i]);
			if (curAdhersion)
			{
				if ((curAdhersion->m_ConnectOrganA->GetOrganType() == EODT_UTERUS 
				  && curAdhersion->m_ConnectOrganB->GetOrganType() == 200) ||
				    (curAdhersion->m_ConnectOrganA->GetOrganType() == 200
				  && curAdhersion->m_ConnectOrganB->GetOrganType() == EODT_UTERUS))
				{
					curAdhersion->m_NodeToNodeLinks.clear();
				}
			}
		}*/

	}
	/*else if (event->key() == Qt::Key_3)//B1C
	{
		m_wombManitor->Rotate(GFPhysQuaternion(GFPhysVector3(0, 1, 0), -3.1415926f / 4.0f));//test rotate 30 degree

	}
	else if (event->key() == Qt::Key_4)//B1C
	{
		m_wombManitor->Rotate(GFPhysQuaternion(GFPhysVector3(0, 1, 0), 3.1415926f / 4.0f));//test rotate 30 degree

	}*/
}
//=============================================================================================
HysterectomyTraining::HysterectomyTraining(const Ogre::String & strName)
{
	
}
//=============================================================================================
HysterectomyTraining::~HysterectomyTraining(void)
{
	MisNewTrainingDebugObject::GetInstance()->SetTraining(NULL);
}
//==========================================================================================
MisMedicOrganInterface * HysterectomyTraining::LoadOrganism(MisMedicDynObjConstructInfo & cs, MisNewTraining* ptrain)
{
	return MisNewTraining::LoadOrganism(cs, ptrain);
}
//======================================================================================================================
void HysterectomyTraining::CreateTrainingScene(CXMLWrapperTraining * pTrainingConfig)
{
	MisNewTraining::CreateTrainingScene(pTrainingConfig);
}
//======================================================================================================================
bool HysterectomyTraining::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{  
	bool result = MisNewTraining::Initialize(pTrainingConfig , pToolConfig);

	PhysicsWrapper::GetSingleTon().m_dynamicsWorld->SoftCollisionHashGridSize(1.0f);///----------

	//DeferredRendFrameWork::Get()->SetBloomBrightPassThreshold(0.92f);

	m_Womb = dynamic_cast<MisMedicOrgan_Ordinary *>(m_DynObjMap[EODT_UTERUS]);
	m_Womb->SetEletricCutParameter(0.25f, 10);//organ->SetTetraCollapseParam(0.3f, 1);
	m_Womb->m_MinSubPartVolThresHold = 0.3f*0.3f*0.3f;


	m_ManiCup = dynamic_cast<MisMedicOrgan_Ordinary *>(m_DynObjMap[200]);

	DeferredRendFrameWork::Instance()->SetSSAOParameter(Ogre::Vector3(0.3f , 0.8f , 0.0f));


	//
	m_wombManitor = new WombManipulator(m_Womb, GFPhysVector3(0.151, 1.484, 3.904), GFPhysVector3(0.102, 2.604, 1.027));
	m_wombManitor->CalculateTetrasBeManipulated();
	BuildManipulatorTouchPoints(m_ManiCup, m_Womb);

	//test

	//float rotAngle = 
	m_wombManitor->Rotate(GFPhysQuaternion(GFPhysVector3(1, 0, 0), 3.1415926f / 4.0f));//test rotate 30 degree
	//
		//
	return result;
}
bool HysterectomyTraining::BeginRendOneFrame(float timeelapsed)
{
	bool result = MisNewTraining::BeginRendOneFrame(timeelapsed);
	return result;
}
bool HysterectomyTraining::Update(float dt)
{
	bool result = MisNewTraining::Update(dt);
	return result;
}

void HysterectomyTraining::OnTrainingIlluminated()
{
	__super::OnTrainingIlluminated();
}

void HysterectomyTraining::DisplaceWombManipulate()
{
	
}
void HysterectomyTraining::BuildManipulatorTouchPoints(MisMedicOrgan_Ordinary * manipulator, MisMedicOrgan_Ordinary * womb)
{
	if (m_wombManitor == 0)
		return;

	GFPhysSoftBody * bodyA = manipulator->m_physbody;
	
	GFPhysSoftBody * bodyB = womb->m_physbody;

	float NodeThresHold = 0.02f;

	GFPhysVector3 Extend(NodeThresHold, NodeThresHold, NodeThresHold);
	GFPhysDBVTree NodeTreeA;
	GFPhysDBVTree NodeTreeB;

	//insert undeformed aabb to treeA
	GFPhysSoftBodyNode * NodeA = bodyA->GetNodeList();
	while (NodeA)
	{
		GFPhysVector3 NodePos = NodeA->m_UnDeformedPos;
		GFPhysDBVNode * dbvn = NodeTreeA.InsertAABBNode(NodePos - Extend, NodePos + Extend);
		dbvn->m_UserData = NodeA;
		NodeA = NodeA->m_Next;
	}

	GFPhysSoftBodyNode * NodeB = bodyB->GetNodeList();
	while (NodeB)
	{
		GFPhysVector3 NodePos = NodeB->m_UnDeformedPos;
		GFPhysDBVNode * dbvn = NodeTreeB.InsertAABBNode(NodePos - Extend, NodePos + Extend);
		dbvn->m_UserData = NodeB;
		NodeB = NodeB->m_Next;
	}


	//test coincide node with node threshold
	LinkedOrganNodeCallBack nodeCallBack(NodeThresHold);
	NodeTreeA.CollideWithDBVTree(NodeTreeB, &nodeCallBack);

	//node pair coincide in 2 organ
	const std::map<GFPhysSoftBodyNode *, NearestNode> & AdherNodeMap = nodeCallBack.m_NearestNodes;
	std::map<GFPhysSoftBodyNode *, NearestNode>::const_iterator itor = AdherNodeMap.begin();


	while (itor != AdherNodeMap.end())
	{
		GFPhysSoftBodyNode * NodeA = itor->first;
		GFPhysSoftBodyNode * NodeB = itor->second.m_Node;

		WombManipulator::ManipulatorCupPoint tPoint;
		tPoint.m_NodeInM = NodeA;
		tPoint.m_NodeInWomb = NodeB;

		m_wombManitor->m_ManipulatorCupPoints.push_back(tPoint);
		itor++;
	}
}

void HysterectomyTraining::NodesBeRemoved(const GFPhysAlignedVectorObj<GFPhysSoftBodyNode*> & nodes, MisMedicOrgan_Ordinary * organ)
{
	 MisNewTraining::NodesBeRemoved(nodes, organ);

	 if (m_wombManitor == 0)
		 return;

	 if (organ == m_Womb)
	 {
		 for (int c = 0; c < nodes.size(); c++)
		 {
			 for (int t = 0; t < m_wombManitor->m_ManipulatorCupPoints.size(); t++)
			 {
				 if (m_wombManitor->m_ManipulatorCupPoints[t].m_NodeInWomb == nodes[c])
				 {
					 m_wombManitor->m_ManipulatorCupPoints[t].m_NodeInWomb = 0;
					 break;
				 }
			 }

			 for (int t = 0; t < m_wombManitor->m_TetrasManipulated.size(); t++)
			 {
				 if (m_wombManitor->m_TetrasManipulated[t].m_Node == nodes[c])
				 {
					 m_wombManitor->m_TetrasManipulated[t].m_Node = 0;
					 break;
				 }
			 }
		 }
	 }
}