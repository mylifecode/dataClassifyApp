#include "math/GoPhysTransformUtil.h"
#include "Dynamic/Constraint/GoPhysSoftFixpointConstraint.h"
#include "Utility/GoPhysSoftBodyUtil.h"
#include "collision/NarrowPhase/GoPhysPrimitiveTest.h"
#include "PhysicsWrapper.h"
#include "MisMedicOrganInterface.h"

#include "DynamicObjectRenderable.h"
#include "MisMedicOrganAttachment.h"
#include "OrganBloodMotionSimulator.h"
#include "XMLWrapperOrgan.h"
#include "XMLWrapperTraining.h"
#include "CustomConstraint.h"
#include "MisMedicOrganAttachment.h"

#include "MXOgreGraphic.h"

MisSoftBodyHomingForce::MisSoftBodyHomingForce(GFPhysSoftBody * softBody)
{
	m_SoftBody = softBody;
}

MisSoftBodyHomingForce::~MisSoftBodyHomingForce()
{

}

void MisSoftBodyHomingForce::PrepareSolveConstraint(Real Stiffness,Real TimeStep)
{
	if(m_SoftBody)
	{
		GFPhysSoftBodyNode * softNode = m_SoftBody->GetSoftBodyShape().GetNodeList();
		while(softNode)
		{
			if(softNode->m_InvM > GP_EPSILON && (softNode->m_StateFlag & GPSESF_COLLIDRIGID) == 0 && (softNode->m_StateFlag & EMMP_ClampByTool) == 0)
			{
				softNode->m_CurrPosition += (softNode->m_UnDeformedPos-softNode->m_CurrPosition)*m_Stiffness;//m_Stiffness;
			}
			softNode = softNode->m_Next;
		}
	}
}

void MisSoftBodyHomingForce::SolveConstraint(Real Stiffness,Real TimeStep)
{
	
}
//===========================================================================================================
MisSoftBodyNodesFixForce::MisSoftBodyNodesFixForce(const std::vector<GFPhysSoftBodyNode*> & nodes)
{
	m_Nodes = nodes;
}

MisSoftBodyNodesFixForce::~MisSoftBodyNodesFixForce()
{

}

void MisSoftBodyNodesFixForce::PrepareSolveConstraint(Real Stiffness,Real TimeStep)
{

}
void MisSoftBodyNodesFixForce::SolveConstraint(Real Stiffness,Real TimeStep)
{
	for(size_t n = 0 ; n < m_Nodes.size() ; n++)
	{
		GFPhysSoftBodyNode * fixNode = m_Nodes[n];
		GFPhysVector3 deltaPos = (fixNode->m_UnDeformedPos - fixNode->m_CurrPosition);
		deltaPos.m_x *= m_AsyStiff.m_x;
		deltaPos.m_y *= m_AsyStiff.m_y;
		deltaPos.m_z *= m_AsyStiff.m_z;

		fixNode->m_CurrPosition += deltaPos;
	}	
}
//===========================================================================================================
MisSoftBodyExpandForce::MisSoftBodyExpandForce(const std::vector<GFPhysSoftBodyNode*> & nodes,const GFPhysAlignedVectorObj<GFPhysVector3> & nodesPos)
{
    m_Nodes = nodes;
    m_NodesPos = nodesPos;
}

MisSoftBodyExpandForce::~MisSoftBodyExpandForce()
{

}

void MisSoftBodyExpandForce::PrepareSolveConstraint(Real Stiffness,Real TimeStep)
{

}
void MisSoftBodyExpandForce::SolveConstraint(Real Stiffness,Real TimeStep)
{
    for(size_t n = 0 ; n < m_Nodes.size() ; n++)
    {
        GFPhysSoftBodyNode * expandNode = m_Nodes[n];
        expandNode->m_CurrPosition += (m_NodesPos[n] - expandNode->m_CurrPosition) * 1.2f;
        //expandNode->m_CurrPosition = m_NodesPos[n];
    }
}

//=======================================================================================================
MisMedicOrganInterface::MisMedicOrganInterface(DynObjType type , DynamicObjType organtype , int oragnId,CBasicTraining * ownertraing)
: /*m_TopolgyType(type) ,*/
  m_OrganType(organtype),
  m_OrganID(oragnId),
  m_OwnerTrain(ownertraing)
  ,m_Flag(0x1000)
{
	m_congulateradius = 0.038f;
	m_ForceFeedBackRation = 1.0f;
	m_CanBeGrasp = false;
	m_AttchmentsFlag = 0;
	m_Visible = true;
	m_Transparent = false;
	m_homingforcecs = 0;
	m_NodesFixForces = 0;
    m_ExpandForce = 0;
	//m_PoseRigidForce = 0;
	m_RSCollideAlgoType = 0;
	m_CanBeLoop = true;
	//m_MultiMassToCalcContactForce = false;
	m_MinPuncturedDist = 0.3f;
	//m_IsGrasped = false;
	m_FaceMoveIncrementInClamp = 0.f;
	m_FaceMoveSpeedInClamp = 0.f;
	m_IsSucked = false;
	m_FaceMoveIncrementInSuction = 0.f;
	m_FaceMoveSpeedInSuction = 0.f;
	m_IsAlreadyReadFile = false;
	m_IsInContainer = false;

	m_BeClampedByLTool = false;
	m_BeClampedByRTool = false;
}	
//=======================================================================================================
MisMedicOrganInterface::~MisMedicOrganInterface()
{
	for(size_t i = 0 ; i < m_OrganAttachments.size() ; i++)
	{
		delete m_OrganAttachments[i];
	}
	m_OrganAttachments.clear();
}
//=======================================================================================================
void MisMedicOrganInterface::AddOrganActionListener(OrganActionListener * listener)
{
	for(size_t c = 0 ; c < m_OrganActionListeners.size() ; c++)
	{
		if(m_OrganActionListeners[c] == listener)
			return;
	}
	m_OrganActionListeners.push_back(listener);
}

void MisMedicOrganInterface::RemoveOrganActionListener(OrganActionListener * listener)
{
	for(size_t c = 0 ; c < m_OrganActionListeners.size() ; c++)
	{
		if(m_OrganActionListeners[c] == listener)
		{
			m_OrganActionListeners.erase(m_OrganActionListeners.begin()+c);
			return;
		}
	}
}
//=======================================================================================================
void MisMedicOrganInterface::RemovePhysicsPart()
{
	if(m_NodesFixForces)
	{
		if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
		{
			PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(m_NodesFixForces);
		}
		delete m_NodesFixForces;
		m_NodesFixForces = 0;
	}

	if(m_homingforcecs)
	{
		if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
		{
			PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(m_homingforcecs);
		}
		delete m_homingforcecs;
		m_homingforcecs = 0;
	}

	//if(m_PoseRigidForce)
	//{
		//if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
		//{
		 //  PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(m_PoseRigidForce);
		//}
		//delete m_PoseRigidForce;
		//m_PoseRigidForce = 0;
	//}

    if(m_ExpandForce)
    {
        if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
        {
            PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(m_ExpandForce);
        }
        delete m_ExpandForce;
        m_ExpandForce = 0;
    }
}
//=======================================================================================================
void MisMedicOrganInterface::RemoveGraphicPart()
{

}
//=======================================================================================================
int  MisMedicOrganInterface::GetAttachmentCount(MisOrganAttachmentType type)
{
	int count = 0;
	for(size_t c = 0 ; c < m_OrganAttachments.size(); c++)
	{
		if(m_OrganAttachments[c]->m_type == type)
		   count++;
	}
	return count;
}

void MisMedicOrganInterface::GetAttachment(MisOrganAttachmentType type,std::vector<MisMedicOrganAttachment*> & attachments)
{
	for(size_t i = 0;i < m_OrganAttachments.size();++i)
	{
		if(m_OrganAttachments[i]->m_type == type)
		{
			attachments.push_back(m_OrganAttachments[i]);
		}
	}
}
//=======================================================================================================
void MisMedicOrganInterface::CutOrganByTool(CTool * tool)
{

}
//=======================================================================================================
void MisMedicOrganInterface::SetBurnWhiteNoiseTexture(Ogre::TexturePtr burnTexture)
{
	//if(m_EffectRender.m_ComposedEffectMaterialPtr.isNull())
	////	return;
	//ApplyTextureToMaterial(m_EffectRender.m_ComposedEffectMaterialPtr,burnTexture,"BurnWhiteChannelMap");
}
//=======================================================================================================
void MisMedicOrganInterface::SetBurnWhiteColor(Ogre::ColourValue color0 , Ogre::ColourValue color1)
{
	//if(m_EffectRender.m_ComposedEffectMaterialPtr.isNull())
	//	return;
	//Ogre::Technique * tech = m_EffectRender.m_ComposedEffectMaterialPtr->getTechnique(0);
	//if(tech->getNumPasses() > 0)
	//{
	//	Ogre::Pass * pass = tech->getPass(0);
		//Ogre::GpuProgramParametersSharedPtr p = pass->getFragmentProgramParameters();
		//p->setNamedConstant("colorWhite",color0);
		//p->setNamedConstant("colorYellow",color1);
	//}
}
//=======================================================================================================
Ogre::Vector2 MisMedicOrganInterface::GetTextureCoord(GFPhysSoftBody * sb ,GFPhysSoftBodyFace * face , float weights[3])
{
	return Ogre::Vector2::ZERO;
}
//=======================================================================================================
// void MisMedicOrganInterface::SetContulateRadius(float radius)
// {
// 	m_congulateradius = radius;
// }
//=======================================================================================================
void MisMedicOrganInterface::Tool_InElec_TouchFacePoint(ITool * tool , GFPhysSoftBodyFace * face , float weights[3], int touchtype , float dt)
{

}
//=======================================================================================================
void MisMedicOrganInterface::InjectSomething(ITool *tool , GFPhysSoftBodyFace * face , float weights[3] , float dt , std::vector<Ogre::Vector2> & resultUv)
{

}
//=======================================================================================================
void MisMedicOrganInterface::HeatTetrahedrons(const GFPhysVectorObj<GFPhysSoftBodyTetrahedron*> & tetrasToHeat , float deltavalue)
{

}
void MisMedicOrganInterface::HeatAroundUndeformedPoint(const GFPhysVector3 & point , float radius , float heatValue)
{

}
//=======================================================================================================
void MisMedicOrganInterface::ToolPunctureSurface(ITool * tool , GFPhysSoftBodyFace * face , const float weights[3])
{

}
void MisMedicOrganInterface::NotifyRigidBodyRemovedFromWorld(GFPhysRigidBody * rb)
{

}
void MisMedicOrganInterface::PreUpdateScene(float dt , Ogre::Camera * camera)
{

}
//=======================================================================================================
void MisMedicOrganInterface::UpdateScene(float dt , Ogre::Camera * camera)
{
	for(size_t i = 0 ; i < m_OrganAttachments.size() ; i++)
	{
		m_OrganAttachments[i]->Update(dt);
	}
}
//=======================================================================================================
bool MisMedicOrganInterface::CanBeGrasp()
{
	return m_CanBeGrasp;
}
bool MisMedicOrganInterface::CanBeCut()
{
	return m_CreateInfo.m_CanCut;
}
bool MisMedicOrganInterface::CanBeLoop()
{
	return m_CanBeLoop;
}
bool MisMedicOrganInterface::CanBeHook()
{
    return m_CreateInfo.m_CanHook;
}
//=======================================================================================================
float MisMedicOrganInterface::GetForceFeedBackRation()
{
	return m_ForceFeedBackRation;
}
//=======================================================================================================
void  MisMedicOrganInterface::SetForceFeedBackRation(float ration)
{
	m_ForceFeedBackRation = ration;
}

//==================================================================================
// void MisMedicOrganInterface::ApplyTextureToMaterial(Ogre::TexturePtr texturetoapp , Ogre::String textureunitname)
// {
// 	Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName(materialname);
// 
// 	if(material.isNull() == false && texturetoapp.isNull() == false)
// 	{
// 		Ogre::Technique * tech = material->getTechnique(0);
// 
// 		if(tech->getNumPasses() > 0)
// 		{
// 			Ogre::Pass * pass = tech->getPass(0);
// 			for (int  t = 0; t < pass->getNumTextureUnitStates() ; t++)
// 			{
// 				Ogre::TextureUnitState * texunit = pass->getTextureUnitState(t);
// 				if(texunit->getTextureNameAlias() == textureunitname )
// 					texunit->setTextureName(texturetoapp->getName());
// 			}
// 		}
// 	}
// }

//void MisMedicOrganInterface::CutByElectricTouched( ITool * tool , GFPhysSoftBodyFace * face , float weights[3])
//{

//}

void MisMedicOrganInterface::SetHomingForce(float stiffness, GFPhysSoftBody *sb)
{
	if(m_homingforcecs == 0 && PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
	{
		m_homingforcecs = new MisSoftBodyHomingForce(sb);
		m_homingforcecs->m_Stiffness = stiffness;//SetStiffness(stiffness);
		PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(m_homingforcecs);
	}
	
}
void MisMedicOrganInterface::SetNodeFixForce(const GFPhysVector3 &  stiffness, const std::vector<GFPhysSoftBodyNode*> & fixNodes)
{
	if(m_NodesFixForces == 0 && PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
	{
		m_NodesFixForces = new MisSoftBodyNodesFixForce(fixNodes);

		m_NodesFixForces->m_AsyStiff.m_x = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(stiffness.m_x);
		m_NodesFixForces->m_AsyStiff.m_y = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(stiffness.m_y);
		m_NodesFixForces->m_AsyStiff.m_z = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(stiffness.m_z);

		PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(m_NodesFixForces);
	}
}

void MisMedicOrganInterface::SetNodeExpandForce(const std::vector<GFPhysSoftBodyNode*> & expandNodes,const GFPhysAlignedVectorObj<GFPhysVector3> & expandNodesPos)
{
    if(m_ExpandForce == 0 && PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
    {
        m_ExpandForce = new MisSoftBodyExpandForce(expandNodes,expandNodesPos);        
        PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(m_ExpandForce);
    }
}

void MisMedicOrganInterface::setAttchmentFlag( int flag, bool isRemove)
{
	if (isRemove == false)
	{
		m_AttchmentsFlag |= flag;
	}
	else
	{
		m_AttchmentsFlag = m_AttchmentsFlag & (~flag);
	}
}

void MisMedicOrganInterface::setFlag( int flag, bool isRemove)
{
	if (isRemove == false)
	{
		m_Flag |= flag;
	}
	else
	{
		m_Flag = m_Flag & (~flag);
	}
}

int MisMedicOrganInterface::getFlag_MaterialId()
{
	bool bL = false;
	bool bR = false;
	if((m_Flag & EMMOI_Touching_By_Tool_Left)|(m_Flag & EMMOI_Touching_By_Tool_Left_Lock))
	{
		bL = true;
	}
	if((m_Flag & EMMOI_Touching_By_Tool_Right)|(m_Flag & EMMOI_Touching_By_Tool_Right_Lock))
	{
		bR = true;
	}
	if (bL == false && bR == false)
	{
		return 0;
	}
	else if (bL == true && bR == true)
	{
		return 1;
	}
	else
	{
		return 2;
	}
}

int MisMedicOrganInterface::GetNumberOfActiveBleedPoint()
{
	int n = 0;

	for(size_t i = 0;i < m_BleedingRecords.size();++i)
	{
		if(m_BleedingRecords[i].m_IsStopped == false)
			++n;
	}

	return n;
}


//=======================================================================================================
MisMedicDynObjConstructInfo::MisMedicDynObjConstructInfo()
{
	m_OrganType = EDOT_NO_TYPE;
	m_OrganId = -1;
	m_objTopologyType = DOT_VOLMESH;
	m_mass = 40.0f;
	m_stiffness = 0.7f;
	
	m_invEdgePhysStiff = -1.0f;
	m_invTetraPhysStiff = -1.0f;
	m_invFacePhysStiff = -1.0f;

	m_EdgePhysDamping = 0.0f;
	m_TetraPhysDamping = 0.0f;
	m_FacePhysDamping = 0.0f;

    m_poissonrate = 0.1f;
	m_FurtherStiffness = 0.0f;
	m_contactmode = 0;
	m_DoubleFaceCollide = false;
	m_veldamping = 2.0f;
	m_perelementdamping = 0.0f;
	m_lineardamping = 0.00f;
	m_angulardampint = 0.00f;

	m_frictcoeff = 0.08f;
	m_collidehardness = 0.5f;
	m_surfacemassmultifactor = 1.0f;
	m_RSMaxPenetrate = -0.3f;
	m_collideRSMargin = 0.02f;
	m_distributemass = false;
	m_hardfixpoint = false;
	m_FixPointStiffness = GFPhysVector3(0.8f , 0.8f , 0.8f);
	m_ConnectMass = 1.0f;
	m_enableSSCollide = true;
	m_CollideWithTool = true;
	m_collidealgo = 0;
	//m_UserPoint = 0;
	
// 	m_pSceneMgr = 0;
	m_bCanBeGrasp = false;
	m_bCanPuncture = true;
	m_CanHook = false;
	m_CheckToolPenetrateForce = false;
	//m_isgallber = false;
	

	m_Position = Ogre::Vector3::ZERO;
	m_InitSize = Ogre::Vector3(1,1,1);

	m_effTexWid = 512;
	m_effTexHei = 512;

	m_ForceFeedBackRation = 1.0f;

	m_BurnRadius = 0.038f;
	m_BurnRation = 4.0f;
	m_TubeRadius = 0.16f;
	
	m_TubeRendRadius = 0.16f;

	//m_TubeFixSection = -1;
	
	//m_TubeAttachObject = -1;
	
	//m_TubeAttachSection = -1;

	//m_TubeAttach = 0;

	//m_TubeAttTube = 0;

	//m_IncTubeRootRadius = false;
	
	//m_IncTubeRootWeight = false;

	m_CutThreshold = 0.08f;

	m_CrossDir = -1.0f;

	m_bCanBluntDissection = false;
	
	//m_CutMaterialName = "MisMedical/DefaultCutFace";//"CutEffect/UteriCutFace";

	m_LayerName = "default";

	m_CanCut = false;

	m_BloodTex = "surface_blood.dds";

	m_BloodRadius = 0.0027f;
	m_WaterRadius = 0.0027f;

	m_IsStaticObject = false;

	m_HomingForce = 0;

	m_Visible = true;

	//m_unionedobjid = -1;

	//m_unionedobjstiffness = 0.0f;

	//m_unionobjAdditionBendForce = 0.0f;
	m_AdditionBendForce = 0.0f;

	m_IsMensentary = false;
	//m_unionobjMenstary = false;
	m_RigidFactor = 2;

	m_GravityValue = 0.0f;

	m_HasCustomGravityDir = false;
	m_CustomGravityDir = Ogre::Vector3(0,0,0);

	//used for new connect

	m_GallSideWeight = 0.2;

	m_LiverSideWeight = 1.0;

	m_ConnectStiffnessScale = 1.0;

	m_ConnectNodeDistanceStiffness = 0.95;

	m_CoonectTetraVolumeStiffness = 0.5;

	m_VeinObjNewMode = false;

	m_ConnStayNum = 3 ; 
	m_ConnReduceNum = 2;
    
    m_ExpandValue = 0.0f;
    m_ExpandCenterPos = GFPhysVector3(0.0f, 0.0f, 0.0f);
    m_ExpandPlanePoint1 = GFPhysVector3(0.0f, 0.0f, 0.0f);
    m_ExpandPlanePoint2 = GFPhysVector3(0.0f, 0.0f, 0.0f);
    m_ExpandPlanePoint3 = GFPhysVector3(0.0f, 0.0f, 0.0f);

	m_bleedingSpeed = 0.f;

	m_RestScale = 1.0f;

	m_CanBlood = true;
}
//=======================================================================================================
void MisMedicDynObjConstructInfo::ReadParameter(CXMLWrapperOrgan * organmain, CXMLWrapperOrgan * organunioned)
{
	ReadParameter(organmain);
	//m_unionedobjid = organunioned->m_Type;
	//m_unionedmmsfilename = organunioned->m_PhysicFile;
	//m_unionedobjstiffness = organunioned->m_Stiffness;
	//m_unionobjAdditionBendForce = organunioned->m_AddBendForce;
	//m_unionobjMenstary = organunioned->m_IsMensentary;
}
//=======================================================================================================
void MisMedicDynObjConstructInfo::ReadParameter(CXMLWrapperOrgan * organcfg)//CXMLWrapperTraining * pTrainingConfig , unsigned int uiIndex )
{
	//CXMLWrapperOrgan * organcfg = pTrainingConfig->m_DynamicScene[uiIndex];

	if(organcfg->m_flag_StaticObject)
		m_IsStaticObject = organcfg->m_StaticObject;

	if(organcfg->m_flag_HomingForce)
		m_HomingForce = organcfg->m_HomingForce;

	if (organcfg->m_flag_DistributeMass)
		m_distributemass = organcfg->m_DistributeMass;

	if(organcfg->m_flag_ShowMesh)
		m_Visible = organcfg->m_ShowMesh;

	if(organcfg->m_flag_InitPos)
		m_Position = organcfg->m_InitPos;

	if(organcfg->m_flag_InitSize)
	   m_InitSize = organcfg->m_InitSize;

	if(organcfg->m_flag_DisableSSCollide)
	   m_enableSSCollide = (!organcfg->m_DisableSSCollide);
	
	if(organcfg->m_flag_CanCut)
	   m_CanCut = organcfg->m_CanCut;

	if(organcfg->m_flag_CanHook)
	   m_CanHook = organcfg->m_CanHook;

	if(organcfg->m_flag_Type)
	   m_OrganType = (DynamicObjType)organcfg->m_Type;
	if(organcfg->m_flag_ID)
		m_OrganId = organcfg->m_ID;
	else
		m_OrganId = (int)m_OrganType;

	if(organcfg->m_flag_Name)
	   m_name = organcfg->m_Name;

	if(organcfg->m_flag_Stiffness)
	   m_stiffness = organcfg->m_Stiffness;

	if (organcfg->m_flag_EdgePhysStiff)
		m_invEdgePhysStiff = (organcfg->m_EdgePhysStiff > 0 ? 1.0f / organcfg->m_EdgePhysStiff : 0.0f);
	else
		m_invEdgePhysStiff = -1.0f;

	if (organcfg->m_flag_TetraPhysStiff)
		m_invTetraPhysStiff = (organcfg->m_TetraPhysStiff > 0 ? 1.0f / organcfg->m_TetraPhysStiff : 0.0f);
	else
		m_invTetraPhysStiff = -1.0f;

	if (organcfg->m_flag_FacePhysStiff)
		m_invFacePhysStiff = (organcfg->m_FacePhysStiff > 0 ? 1.0f / organcfg->m_FacePhysStiff : 0.0f);
	else
		m_invFacePhysStiff = -1.0f;

	if (organcfg->m_flag_EdgePhysDamp)
		m_EdgePhysDamping = organcfg->m_EdgePhysDamp;

	if (organcfg->m_flag_TetraPhysDamp)
		m_TetraPhysDamping = organcfg->m_TetraPhysDamp;

	if (organcfg->m_flag_FacePhysDamp)
		m_FacePhysDamping = organcfg->m_FacePhysDamp;

	
	
	if(organcfg->m_flag_Poissonrate)
        m_poissonrate = organcfg->m_Poissonrate;

	if(organcfg->m_flag_FurtherStrength)
	   m_FurtherStiffness = organcfg->m_FurtherStrength;
	
	if(organcfg->m_flag_PointDamp)
	{
		Ogre::vector<Ogre::String>::type strVec = Ogre::StringUtil::split(organcfg->m_PointDamp , ",");
		if(strVec.size() > 0)
		{
		   m_veldamping = Ogre::StringConverter::parseReal(strVec[0]);
		}
		if(strVec.size() > 1)
		{
			m_perelementdamping = Ogre::StringConverter::parseReal(strVec[1]);
		}
	   //m_veldamping = organcfg->m_PointDamp;
	}

	if(organcfg->m_flag_Mass)
	   m_mass = organcfg->m_Mass;
	
	if(organcfg->m_flag_RSCollideHardNess)
	   m_collidehardness = organcfg->m_RSCollideHardNess;

    if (organcfg->m_flag_AttachStaticMesh)
    {
        m_AttachStaticMesh = organcfg->m_AttachStaticMesh;
        m_AttachStaticMeshThresHold = organcfg->m_AttachStaticMeshThresHold;
    }
        

	if(organcfg->m_flag_RSfrictcoeff)
	   m_frictcoeff = organcfg->m_RSfrictcoeff;

	if(organcfg->m_flag_LinearDamping)
	   m_lineardamping = organcfg->m_LinearDamping;

	if(organcfg->m_flag_AngularDamping)
	   m_angulardampint = organcfg->m_AngularDamping;

	if(organcfg->m_flag_RSContactMode)
	{
		Ogre::String rscontactmode = organcfg->m_RSContactMode;
		Ogre::vector<Ogre::String>::type modes = Ogre::StringUtil::split(rscontactmode,",");
		if(modes.size() > 0)
		{
		   m_contactmode = Ogre::StringConverter::parseInt(modes[0]);
		}
		if(modes.size() > 1)
		{
			m_DoubleFaceCollide = Ogre::StringConverter::parseBool(modes[1]);
		}
		if(modes.size() > 2)
		{
			m_collidealgo = Ogre::StringConverter::parseInt(modes[2]);
		}
		
		//(int)organcfg->m_RSContactMode;
	}

	if(organcfg->m_flag_RSMaxPenetrate)
	   m_RSMaxPenetrate = organcfg->m_RSMaxPenetrate;

	if(organcfg->m_flag_SurfaceNodeMassMultiply)
	   m_surfacemassmultifactor = organcfg->m_SurfaceNodeMassMultiply;

	if(organcfg->m_flag_FixPointStiffness)
	{
	   Ogre::vector<Ogre::String>::type stringVec = Ogre::StringUtil::split(organcfg->m_FixPointStiffness, ",");

	   if (stringVec.size() >= 1)
	   {
		   m_FixPointStiffness = GFPhysVector3(0, 0, 0);
		   for (int c = 0; c < stringVec.size(); c++)
		   {
			   float stiff = Ogre::StringConverter::parseReal(stringVec[c]);
			   m_FixPointStiffness[c] = stiff;
		   }
		   if (stringVec.size() == 1)
		   {
			   m_FixPointStiffness[1] = m_FixPointStiffness[0];
			   m_FixPointStiffness[2] = m_FixPointStiffness[0];

		   }
		 
		   if (m_FixPointStiffness.Length() <= 0)
			   m_hardfixpoint = true;
		   else
			   m_hardfixpoint = false;
	   }
	}
	else
	   m_hardfixpoint = true;

	if (organcfg->m_flag_RestScaleRatio)
	{
		m_RestScale = organcfg->m_RestScaleRatio;
	}
    if(organcfg->m_flag_ExpandValue)
    {
        m_ExpandValue = organcfg->m_ExpandValue;
    }
    if (organcfg->m_flag_ExpandCenterPos)
    {
        m_ExpandCenterPos = OgreToGPVec3(organcfg->m_ExpandCenterPos);
    }
    if (organcfg->m_flag_ExpandPlanePoint1)
    {
        m_ExpandPlanePoint1 = OgreToGPVec3(organcfg->m_ExpandPlanePoint1);
    }   
    if (organcfg->m_flag_ExpandPlanePoint2)
    {
        m_ExpandPlanePoint2 = OgreToGPVec3(organcfg->m_ExpandPlanePoint2);
    }
    if (organcfg->m_flag_ExpandPlanePoint3)
    {
        m_ExpandPlanePoint3 = OgreToGPVec3(organcfg->m_ExpandPlanePoint3);
    }

	if (organcfg->m_flag_CanBlood)
		m_CanBlood = organcfg->m_CanBlood;

	if(organcfg->m_flag_BleedingSpeed)
		m_bleedingSpeed = organcfg->m_BleedingSpeed;

	if(organcfg->m_flag_ConnectMass)
	   m_ConnectMass = organcfg->m_ConnectMass;

	if (organcfg->m_PhysicFile != "" )	
		m_s4mfilename = organcfg->m_PhysicFile;

	if (organcfg->m_S3MFile != "" )
		m_s3mfilename = organcfg->m_S3MFile;

	if (organcfg->m_MXFile != "" )
		m_t2filename = organcfg->m_MXFile;

	///if(organcfg->m_UnionedObjFile != "" )
	  /// m_unionedmmsfilename = organcfg->m_UnionedObjFile;

	//if(organcfg->m_flag_UnionedObjID)
	  // m_unionedobjid = organcfg->m_UnionedObjID;

	if (organcfg->m_MaterialName != "" )
	    m_materialname[0] = organcfg->m_MaterialName;

	if (organcfg->m_MaterialNameArray != "" )
	{
		Ogre::vector<Ogre::String>::type  fixeds = Ogre::StringUtil::split(organcfg->m_MaterialNameArray , ",");
		for(size_t f = 0 ; f < fixeds.size() && f < 3; f++)
		{
			Ogre::String str = fixeds[f];
			m_materialname[1 + f] = str;
		}
	}

	if (organcfg->m_flag_LayerName)
	{
		m_LayerName = organcfg->m_LayerName;
	}

	if(organcfg->m_flag_TopolgyType)
	{
		if(organcfg->m_TopolgyType == "Mesh")
		   m_objTopologyType = DOT_VOLMESH;
		
		else if(organcfg->m_TopolgyType == "Brane")
		   m_objTopologyType = DOT_MEMBRANE;

		else if(organcfg->m_TopolgyType == "Tube")
		   m_objTopologyType = DOT_TUBE;

		else if(organcfg->m_TopolgyType == "Connect")
		   m_objTopologyType = DOT_VEINCONNECT;
		
		//else if(organcfg->m_TopolgyType == "NewConnect")
		//   m_objTopologyType = DOT_NEWVEINCONNECT;
		//
		//else if(organcfg->m_TopolgyType == "ConnectV2")
		//   m_objTopologyType = DOT_VEINCONNECTV2;

       // else if(organcfg->m_TopolgyType == "RIGIDBrane")
         //   m_objTopologyType = DOT_RIGIDMESH;
	}
	else
	{
		m_objTopologyType = DOT_UNDEF;
	}

	if(organcfg->m_flag_EffectTexSize)
	   m_effTexHei = m_effTexWid = organcfg->m_EffectTexSize;

	if(organcfg->m_flag_CanClamp)
	   m_bCanBeGrasp = organcfg->m_CanClamp;

	if (organcfg->m_flag_CanPuncture)
		m_bCanPuncture = organcfg->m_CanPuncture;

	if(organcfg->m_flag_ForceFeedbackRatio)
	   m_ForceFeedBackRation = organcfg->m_ForceFeedbackRatio;

	if(organcfg->m_flag_RSCollideMargin)
	   m_collideRSMargin = organcfg->m_RSCollideMargin;

	if(organcfg->m_flag_BurnRadius)
	  m_BurnRadius = organcfg->m_BurnRadius;

	if(organcfg->m_flag_BurnRation)
	   m_BurnRation = organcfg->m_BurnRation;

	if(organcfg->m_flag_FixedPoints)
	{
		std::set<int> ExistsFixedIndex;

		Ogre::String strFixedPoints = organcfg->m_FixedPoints;
		Ogre::vector<Ogre::String>::type  fixeds = Ogre::StringUtil::split(strFixedPoints , ",");
		for(size_t f = 0 ; f < fixeds.size() ; f++)
		{
			Ogre::String str = fixeds[f];
			int index = Ogre::StringConverter::parseInt(str);

			if(ExistsFixedIndex.find(index) == ExistsFixedIndex.end())
			{
				m_FixPointsIndex.push_back(index);
				ExistsFixedIndex.insert(index);
			}
			
		}
	}

    if(organcfg->m_flag_ExpandedPoints)
    {
        std::set<int> ExistsExpandedIndex;

        Ogre::String strExpandedPoints = organcfg->m_ExpandedPoints;
        Ogre::vector<Ogre::String>::type  expandnodes = Ogre::StringUtil::split(strExpandedPoints , ",");
        for(size_t f = 0 ; f < expandnodes.size() ; f++)
        {
            Ogre::String str = expandnodes[f];
            int index = Ogre::StringConverter::parseInt(str);

            if(ExistsExpandedIndex.find(index) == ExistsExpandedIndex.end())
            {
                m_ExpandPointsIndex.push_back(index);
                ExistsExpandedIndex.insert(index);
            }
        }                
    }

	if(organcfg->m_flag_CutActionParticleParam)
	{
		Ogre::String strCutActionParticleParam = organcfg->m_CutActionParticleParam;
		Ogre::vector<Ogre::String>::type  fixeds = Ogre::StringUtil::split(strCutActionParticleParam , ",");
		m_CutActionParticleParam[0] = Ogre::StringConverter::parseInt(fixeds[0]);
		m_CutActionParticleParam[1] = Ogre::StringConverter::parseInt(fixeds[1]);
		if (fixeds[2] == "Bile")
		{
			m_CutActionParticleParam[2] = 0;
		}
		else if (fixeds[2] == "Blood")
		{
			m_CutActionParticleParam[2] = 1;
		}
		else
			m_CutActionParticleParam[2] = Ogre::StringConverter::parseInt(fixeds[2]);
	}
	else
	{
		m_CutActionParticleParam[0] = -1;
		m_CutActionParticleParam[1] = -1;
		m_CutActionParticleParam[2] = -1;
	}

	if (organcfg->m_flag_BloodFlowFrameNum)
	{
		m_BloodFlowVelocity = 1.0f / organcfg->m_BloodFlowFrameNum;
	}
	else
		m_BloodFlowVelocity = 0.01f;

	if(organcfg->m_flag_AnimationFile)
	   m_animationfile = organcfg->m_AnimationFile;
	if(organcfg->m_flag_FFDFile)
	   m_FFDFile = organcfg->m_FFDFile;

	if(organcfg->m_flag_TubeRadius)
	   m_TubeRadius = organcfg->m_TubeRadius;

	if(organcfg->m_flag_TubeRendRadius)
	   m_TubeRendRadius = organcfg->m_TubeRendRadius;

	//if(organcfg->m_flag_TubeFixSection)
	//	m_TubeFixSection = organcfg->m_TubeFixSection;

	//if(organcfg->m_flag_IncTubeRootRadius)
	 //  m_IncTubeRootRadius = organcfg->m_IncTubeRootRadius;

	//if(organcfg->m_flag_IncTubeRootWeight)
	 //  m_IncTubeRootWeight = organcfg->m_IncTubeRootWeight;

	//if(organcfg->m_flag_TubeAttachs)
	//{
		//Ogre::String attachstr = organcfg->m_TubeAttachs;
		
		//Ogre::vector<Ogre::String>::type attachs = Ogre::StringUtil::split(attachstr , ";");

		//if(attachs.size() > 0)
		//{
			//Ogre::String firstattach = attachs[0];
			//Ogre::vector<Ogre::String>::type attachpair = Ogre::StringUtil::split(firstattach , ",");
			//if(attachpair.size() > 1)
			//{
				//m_TubeAttachObject  = Ogre::StringConverter::parseInt(attachpair[0]);
				//m_TubeAttachSection = Ogre::StringConverter::parseInt(attachpair[1]);
			//}
		//}
	//}

	if(organcfg->m_flag_CutThreshold)
	   m_CutThreshold = organcfg->m_CutThreshold;
	
	if (organcfg->m_flag_CrossDir)
		m_CrossDir = organcfg->m_CrossDir;

	if (organcfg->m_flag_CanBluntDissection)
		m_bCanBluntDissection = organcfg->m_CanBluntDissection;

	//if(organcfg->m_flag_CutMaterialName)
	   //m_CutMaterialName = organcfg->m_CutMaterialName;

	if (organcfg->m_flag_GravityValue)
	{
		m_GravityValue = organcfg->m_GravityValue;//Ogre::Vector3(organcfg->m_OOGravity.x, organcfg->m_OOGravity.y, organcfg->m_OOGravity.z);	
	}
	else
	{
		m_GravityValue = 0.0f;//Ogre::Vector3(0,0,-1.25f);
	}

	if(organcfg->m_flag_CustomGravityDir)
	{
		m_HasCustomGravityDir = true;
		m_CustomGravityDir = organcfg->m_CustomGravityDir.normalisedCopy();
	}

	if (organcfg->m_flag_SceneMeshInitPosOffset)
	{
		m_SceneMeshInitPosOffset = Ogre::Vector3(organcfg->m_SceneMeshInitPosOffset.x, organcfg->m_SceneMeshInitPosOffset.y, organcfg->m_SceneMeshInitPosOffset.z);	
	}
	else
	{
		m_SceneMeshInitPosOffset = Ogre::Vector3(0,0,0);
	}

	if (organcfg->m_flag_SceneMeshInitScaleOffset)
	{
		m_SceneMeshInitScaleOffset = Ogre::Vector3(organcfg->m_SceneMeshInitScaleOffset.x, organcfg->m_SceneMeshInitScaleOffset.y, organcfg->m_SceneMeshInitScaleOffset.z);	
	}
	else
	{
		m_SceneMeshInitScaleOffset = Ogre::Vector3(1,1,1);
	}

	if(organcfg->m_flag_AddBendForce)
	   m_AdditionBendForce = organcfg->m_AddBendForce;

	if(organcfg->m_flag_IsMensentary)
	   m_IsMensentary = organcfg->m_IsMensentary;

	m_RigidType = organcfg->m_RigidType;

	if (organcfg->m_flag_CollideWithTool)
	{
		m_CollideWithTool = organcfg->m_CollideWithTool;
	}
	//used for new connect
	if(organcfg->m_flag_GallSideWeight)
		m_GallSideWeight = organcfg->m_GallSideWeight;

	if(organcfg->m_flag_LiverSideWeight)
		m_LiverSideWeight = organcfg->m_LiverSideWeight;

	if(organcfg->m_flag_ConnectStiffnessScale)
		m_ConnectStiffnessScale = organcfg->m_ConnectStiffnessScale;

	if(organcfg->m_flag_ConnectNodeDistanceStiffness)
		m_ConnectNodeDistanceStiffness = organcfg->m_ConnectNodeDistanceStiffness;

	if(organcfg->m_flag_CoonectTetraVolumeStiffness)
		m_CoonectTetraVolumeStiffness = organcfg->m_CoonectTetraVolumeStiffness;

	if(organcfg->m_flag_VeinObjNewMode)
		m_VeinObjNewMode = organcfg->m_VeinObjNewMode;

	if(organcfg->m_flag_ConnStayNum)
		m_ConnStayNum = organcfg->m_ConnStayNum;
	
	if(organcfg->m_flag_ConnReduceNum)
		m_ConnReduceNum = organcfg->m_ConnReduceNum;

	if (organcfg->m_flag_PulsePoints)
		m_pulsePoints = organcfg->m_PulsePoints;
}


//构建Entity对象和Node结点获取基于世界位置的顶点数据和索引数据
void MisMedicOrganInterface::ExtractOgreMeshInfo(Ogre::MeshPtr mesh,
	                                             std::vector<Ogre::Vector3> & vertices, 
												 std::vector<Ogre::Vector2> & textures,
											     std::vector<unsigned int> & indices)
{
	 vertices.clear();
	 indices.clear();

	 bool added_shared = false;
	 size_t current_offset = 0;
	 size_t shared_offset = 0;
	 size_t next_offset = 0;
	 size_t index_offset = 0;

	 int vertex_count = 0;
	 int index_count = 0;

	 // Calculate how many vertices and indices we're going to need
	 for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
	 {
		Ogre::SubMesh* submesh = mesh->getSubMesh(i);

		// We only need to add the shared vertices once
		if (submesh->useSharedVertices)
		{
			if (!added_shared)
			{
				vertex_count += mesh->sharedVertexData->vertexCount;
				added_shared = true;
			}
		}
		else
		{
			vertex_count += submesh->vertexData->vertexCount;
		}

		// Add the indices
		index_count += submesh->indexData->indexCount;
	}

	added_shared = false;

	// Run through the submeshes again, adding the data into the arrays
	for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
	{
		Ogre::SubMesh* submesh = mesh->getSubMesh(i);

		Ogre::VertexData* vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;

		if ((!submesh->useSharedVertices) || (submesh->useSharedVertices && !added_shared))
		{
			if (submesh->useSharedVertices)
			{
				added_shared = true;
				shared_offset = current_offset;
			}

			const Ogre::VertexElement* posElem =
				vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);

			Ogre::HardwareVertexBufferSharedPtr vbuf =
				vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());

			const Ogre::VertexElement* texElem =
				vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_TEXTURE_COORDINATES);

			Ogre::HardwareVertexBufferSharedPtr tbuf =
				vertex_data->vertexBufferBinding->getBuffer(texElem->getSource());

			unsigned char* vertexPos =
				static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

			unsigned char* vertexTex =
				static_cast<unsigned char*>(tbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
			// There is _no_ baseVertexPointerToElement() which takes an Ogre::Real or a double
			//  as second argument. So make it float, to avoid trouble when Ogre::Real will
			//  be comiled/typedefed as double:
			//      Ogre::Real* pReal;
			float* pReal;

			for (size_t j = 0; 
				j < vertex_data->vertexCount; 
				++j, vertexPos += vbuf->getVertexSize(), vertexTex += tbuf->getVertexSize())
			{
				posElem->baseVertexPointerToElement(vertexPos, &pReal);
				Ogre::Vector3 pt(pReal[0], pReal[1], pReal[2]);
				vertices.push_back(pt);// (orientation * (pt * scale)) + translate);

				texElem->baseVertexPointerToElement(vertexTex, &pReal);
				Ogre::Vector2 tc(pReal[0], pReal[1]);
				textures.push_back(tc);
			}

			vbuf->unlock();
			tbuf->unlock();
			next_offset += vertex_data->vertexCount;
		}


		Ogre::IndexData* index_data = submesh->indexData;
		size_t numTris = index_data->indexCount / 3;
		Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;

		bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);

		unsigned long*  pLong = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
		unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);


		size_t offset = (submesh->useSharedVertices) ? shared_offset : current_offset;

		if (use32bitindexes)
		{
			for (size_t k = 0; k < numTris * 3; ++k)
			{
				//indices[index_offset++] = pLong[k] + static_cast<unsigned long>(offset);
				indices.push_back(pLong[k] + static_cast<unsigned long>(offset));
			}
		}
		else
		{
			for (size_t k = 0; k < numTris * 3; ++k)
			{
				//indices[index_offset++] = static_cast<unsigned long>(pShort[k]) + static_cast<unsigned long>(offset);
				indices.push_back(static_cast<unsigned long>(pShort[k]) + static_cast<unsigned long>(offset));
			}
		}

		ibuf->unlock();
		current_offset = next_offset;
	}
}