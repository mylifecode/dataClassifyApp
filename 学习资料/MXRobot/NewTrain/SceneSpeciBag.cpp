#include "SceneSpeciBag.h"
#include "MisMedicOrganOrdinary.h"
#include "MisMedicRigidPrimtive.h"
#include "ToolSpenetrateMgr.h"
#include "MisNewTraining.h"
#include "Instruments\Tool.h"
SceneSpeciBag::SceneSpeciBag()
{
	//test data
	GFPhysVector3 top(-0.132, 2.844, 0.778);
	GFPhysVector3 bot(-0.117, 2.386, 0.43);

	GFPhysVector3 edgePt(0.736, 2.264, 0.781);

	m_HoleCenter = bot;
	m_HoleNormal = (top - bot).Normalized();
	m_HoleRadius = (edgePt - m_HoleCenter).Length();

	m_ColCupCollisionCat = (MMRC_UserStart << 2);

}

SceneSpeciBag::~SceneSpeciBag()
{
	m_HostTrain->RemoveOrganFromWorld(m_SoftID);
	m_HostTrain->RemoveOrganFromWorld(m_RigidID);
	
	m_SoftPart  = 0;
	m_Collision = 0;

	if (m_RigidRendEntity)
	{
		MXOgreWrapper::Get()->GetDefaultSceneManger()->destroyEntity(m_RigidRendEntity);
		m_RigidRendEntity = 0;
	}

	if (m_RigidRendNode)
	{
		MXOgreWrapper::Get()->GetDefaultSceneManger()->getRootSceneNode()->removeAndDestroyChild(m_RigidRendNode->getName());
		m_RigidRendNode = 0;
	}

}

bool SceneSpeciBag::Create(const MisMedicDynObjConstructInfo & cs, 
	                                           const Ogre::Vector3 & pos, 
											   MisNewTraining * hostTrain,
	                                           Ogre::SceneManager * sceneMgr,  Ogre::String rendMeshName,
	                                           Ogre::String colMeshName , Ogre::String rendMaterial)
{
	static int organId = 10000;
	organId++;
	mCopyCS = cs;
	mCopyCS.m_Position = pos;

	m_SoftPart = dynamic_cast<MisMedicOrgan_Ordinary*> (hostTrain->ManuallyCreateOrgan(mCopyCS));// new MisMedicOrgan_Ordinary(EDOT_NO_TYPE, organId, hostTrain);
	m_SoftPart->m_physbody->SetCollisionMask(m_SoftPart->m_physbody->m_MaskBits & (~m_ColCupCollisionCat));
	//m_SoftPart->m_physbody->EnableDoubleFaceSoftSoftCollision();//标本带打开双面
	
	
	MisMedicDynObjConstructInfo colCs;
	colCs.m_s3mfilename = colMeshName;
	colCs.m_RigidType = "trimesh";
	colCs.m_objTopologyType = DOT_UNDEF;
	colCs.m_Visible = false;
	colCs.m_mass = 0;
	colCs.m_Position = cs.m_Position;
	m_Collision = dynamic_cast<MisMedicRigidPrimtive*> (hostTrain->ManuallyCreateOrgan(colCs));
	m_Collision->m_body->SetCollisionCategory(m_ColCupCollisionCat);

	//m_Collision->m_body->GetWorldTransform().SetOrigin(OgreToGPVec3(cs.m_Position));	
	
	m_RigidRendEntity = sceneMgr->createEntity(rendMeshName);
	m_RigidRendEntity->setMaterialName(rendMaterial);

	m_RigidRendNode = sceneMgr->getRootSceneNode()->createChildSceneNode(cs.m_Position);
	m_RigidRendNode->attachObject(m_RigidRendEntity);

	m_SoftID = m_SoftPart->m_OrganID;
	m_RigidID = m_Collision->m_OrganID;
	m_HostTrain = hostTrain;
	return true;

}

void SceneSpeciBag::ResetPosition(const Ogre::Vector3 & pos)
{
	Ogre::Vector3 deltaPos = pos - mCopyCS.m_Position;
	for (int c = 0; c < m_SoftPart->m_physbody->GetNodesNum(); c++)
	{
		GFPhysSoftBodyNode * node = m_SoftPart->m_physbody->GetNode(c);
		
		node->m_UnDeformedPos += OgreToGPVec3(deltaPos);
		
		node->SetDeformedPos(node->m_UnDeformedPos);//node->m_CurrPosition = node->m_LastPosition = node->m_UnDeformedPos;
		
		node->m_Velocity = GFPhysVector3(0, 0, 0);
	}
	
	m_RigidRendNode->setPosition(pos);
	
	m_Collision->SetPosition(OgreToGPVec3(pos));
	
	mCopyCS.m_Position = pos;
}

void SceneSpeciBag::ScaleBag(float ScaleX, float ScaleY, float ScaleZ)
{
	m_RigidRendNode->setScale(ScaleX, ScaleX, ScaleX);
	m_SoftPart->getSceneNode()->setScale(ScaleX, ScaleX, ScaleX);
	 Ogre::Vector3 center = mCopyCS.m_Position;
	 Ogre::Vector3 newcenter = center - Ogre::Vector3(center.x*ScaleX, center.y*ScaleX, center.z*ScaleX);
	 m_SoftPart->getSceneNode()->setPosition(newcenter);
}

bool SceneSpeciBag::IsClusetInBag(const std::set<GFPhysSoftBodyNode*> & nodeCluster)
{
	GFPhysVector3 worldHoleC = m_HoleCenter + OgreToGPVec3(mCopyCS.m_Position);
	
	//get center of mass
	GFPhysVector3 comass(0, 0, 0);
	int numNode = 0;
	std::set<GFPhysSoftBodyNode*>::iterator itor = nodeCluster.begin();
	while (itor != nodeCluster.end())
	{
		GFPhysSoftBodyNode * node = (*itor);
		comass += node->m_CurrPosition;
		numNode++;
		itor++;
	}
	comass = comass / float(numNode);

	if ((comass - worldHoleC).Dot(m_HoleNormal) > -0.4f)
	{
		return false;
	}
	//we caster some ray from com to see if they intersect
	GFPhysVector3 axis0 = Perpendicular(m_HoleNormal);
	GFPhysVector3 axis1 = m_HoleNormal.Cross(axis0).Normalized();

	for (int c = 0; c < 8; c++)
	{
		float theta = c * GP_2PI / 8.0f;
		
		GFPhysVector3 dir = axis0 * cosf(theta) + axis1 * sinf(theta);
		
		Real dist = 0;

		GFPhysSoftBodyFace * face = m_SoftPart->GetRayIntersectFace(GPVec3ToOgre(comass), GPVec3ToOgre(comass + dir*m_HoleRadius*2.0f), dist);

		if (face == 0)
		{
			return false;
		}
	}
	return true;
}

