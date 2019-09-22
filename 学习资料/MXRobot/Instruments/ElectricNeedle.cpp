#include "ElectricNeedle.h"
#include "XMLWrapperTool.h"
#include "XMLWrapperOrgan.h"
#include "BasicTraining.h"
#include "EffectManager.h"
#include "MXEventsDump.h"
#include "MXEvent.h"

#include "InputSystem.h"
#include "MisRobotInput.h"
#include "MisMedicCToolPluginInterface.h"
#include "math/GoPhysTransformUtil.h"
#include "TrainingMgr.h"
#include "MisMedicOrganOrdinary.h"

//============================================================================================================
CElectricNeedle::CElectricNeedle()
{	
	m_vecToolFirstPos=Ogre::Vector3::ZERO;
}
//============================================================================================================
CElectricNeedle::CElectricNeedle(CXMLWrapperTool * pToolConfig) : CElectricTool(pToolConfig)
{	
	m_vecToolFirstPos=Ogre::Vector3::ZERO;
	m_UseNewToolMode = true;
	m_NewCanClampTube = true;
}
//============================================================================================================
CElectricNeedle::~CElectricNeedle()
{
}
std::string CElectricNeedle::GetCollisionConfigEntryName()
{
	return "ElectricNeedle";
}
//============================================================================================================
bool CElectricNeedle::Initialize(CXMLWrapperTraining * pTraining)
{
	__super::Initialize(pTraining);

	//for new train
	if(m_pOwnerTraining->m_IsNewTrainMode)
	{
		m_lefttoolpartconvex.SetAttachedNode(m_pNodeKernel);
		m_righttoolpartconvex.SetAttachedNode(m_pNodeKernel);

		m_NeedleHead = GFPhysVector3(0, 0, 0);
		m_NeedleRoot = GFPhysVector3(0, 0, -1.0f);
		
		m_CanPunctureOgran = true;
		m_BubbleWhenBurn = true;
	}
	
	return true;
}
//============================================================================================================

void CElectricNeedle::NewToolModeUpdate()
{
	
}

//============================================================================================================
void CElectricNeedle::InternalSimulationStart(int currStep , int TotalStep , float dt)
{
	 CTool::InternalSimulationStart(currStep , TotalStep , dt);
}
//==================================================================================
void CElectricNeedle::InternalSimulationEnd(int currStep , int TotalStep , float dt)
{	
	CTool::InternalSimulationEnd(currStep , TotalStep , dt);
}

//============================================================================================
bool CElectricNeedle::Update(float dt)
{	
	__super::Update(dt);

	m_NeedleRoot_WCS = OgreToGPVec3(m_pNodeKernel->_getDerivedPosition() 
		                          + m_pNodeKernel->_getDerivedOrientation() * GPVec3ToOgre(m_NeedleRoot));

	m_NeedleHead_WCS = OgreToGPVec3(m_pNodeKernel->_getDerivedPosition()
		                          + m_pNodeKernel->_getDerivedOrientation() * GPVec3ToOgre(m_NeedleHead));


	TryElecBurnTouchedOrgans(m_OrganFaceSelToBurn, dt);

	return true;
}
//=================================================================================================================
float CElectricNeedle::GetFaceToElecBladeDist(GFPhysVector3 triVerts[3])
{
	GFPhysVector3  pointOnTri;

	GFPhysVector3  pointOnSeg;

	GFPhysVector3  collideNormOnTri;

	GFPhysVector3 NeedlePos[2];
	NeedlePos[0] = m_NeedleHead_WCS;
	NeedlePos[1] = m_NeedleRoot_WCS;

	float dist = ClosetPtSegmentTriangle(triVerts, NeedlePos, pointOnTri, pointOnSeg, collideNormOnTri);

	if (dist < 0.05f * 1.5f)
		return dist;
	else
		return -1;
}
//===============================================================================================================
bool CElectricNeedle::IsRightPartConductElectric()
{ 
	return false; 
}
//=========================================================================================
bool CElectricNeedle::ElectricCutOrgan(MisMedicOrgan_Ordinary * organ, GFPhysSoftBodyFace * face, float weights[3])
{
	if (organ->m_OrganID == EDOT_APPENDIX
		|| organ->m_OrganID == EODT_UTERUS
		|| organ->GetOrganType() == EDOT_ADHESION
		|| organ->GetOrganType() == EDOT_SIGMOIDCUTPART
		|| organ->GetOrganType() == EDOT_LUNG_FASICA
		|| organ->GetOrganType() == EDOT_MESOCOLON
		|| organ->GetOrganType() == EDOT_GEROTAS
		|| organ->GetCreateInfo().m_IsMensentary)//temple 
	{
		
		GFPhysVector3 triVerts[3];
		triVerts[0] = face->m_Nodes[0]->m_CurrPosition;
		triVerts[1] = face->m_Nodes[1]->m_CurrPosition;
		triVerts[2] = face->m_Nodes[2]->m_CurrPosition;

		organ->DestroyTissueAroundNode(face, weights, false);
		
		return true;
	}
	else
	{
		return false;
	}
}

//=========================================================================================
void CElectricNeedle::onFrameUpdateStarted( float timeelpased )
{
	CElectricTool::onFrameUpdateStarted(timeelpased);
}