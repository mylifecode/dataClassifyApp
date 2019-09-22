#include "ElectricHook.h"
#include "XMLWrapperTool.h"
#include "XMLWrapperOrgan.h"
#include "BasicTraining.h"
#include "EffectManager.h"
#include "MXEventsDump.h"
#include "MXEvent.h"

#include "InputSystem.h"
#include "MisRobotInput.h"
#include "NewTrain/VeinConnectObject.h"
#include "math/GoPhysTransformUtil.h"
#include "Topology/GoPhysPresetCutGeometry.h"
#include "TrainingMgr.h"
#include "MisCTool_PluginHook.h"

#define FIRECOUNT 800
#define FIRETIME 20000
#define RANDNUM 6
extern bool PointInSphere(Ogre::Vector3 v3SphereCenter, float fSphereRadius, Ogre::Vector3 v3Pos);

extern GFPhysVector3 GetClosetPointInSegment(GFPhysSoftTube * softtube,
									const GFPhysAlignedVectorObj<GFPhysVector3> & SectionCenterPos ,
									const ToolCutBlade & blade , 
									int & SectionIndexLocal , 
									int & SegmentIndex,
									float & SegmentPercent,
									float & Dist);

class CGallbladderTraining;

CElectricHook::CElectricHook():m_fAccumTime(0)
,m_fLastTickCount(0)
,m_bLastBtnPress(false)
,m_bTimeEnough(false)
,m_bHook(false)
,m_pDynamicObject(0)
,m_VecToolLastPos(Ogre::Vector3::ZERO)
,m_bFirstFrame(true)
,m_parkfired(false)
,m_lastHookforceFeedBack(0,0,0)
,m_LastElectricTime(0)
,m_ElectricPersistTime(0)
//,m_MinHookDistBody(0)
,m_canEmitSmoke(true)
,m_veinConnectElectrocuted(false)
,m_RandNumForSpark(50)
,m_TimeSliceForSpack(0)
{
	//m_vecIntersectResults.clear();	
	m_vecToolFirstPos=Ogre::Vector3::ZERO;

	m_bCheckBlood=false;
	m_v3ToolPos=Ogre::Vector3::ZERO;
	
	m_iHookCoolDown = 0;

	cutNodeLeft = 0;
	cutNodeRight = 0;

	m_hasRealElectricAttribute = true;
}

CElectricHook::CElectricHook(CXMLWrapperTool * pToolConfig) : CElectricTool(pToolConfig)
,m_fAccumTime(0)
,m_fLastTickCount(0)
,m_bLastBtnPress(false)
,m_bTimeEnough(false)
,m_bHook(false)
,m_pDynamicObject(0)
,m_VecToolLastPos(Ogre::Vector3::ZERO)
,m_bFirstFrame(true)
,m_parkfired(false)
,m_lastHookforceFeedBack(0,0,0)
,m_LastElectricTime(0)
,m_ElectricPersistTime(0)
//,m_MinHookDistBody(0)
{
	//m_vecIntersectResults.clear();	
	m_vecToolFirstPos=Ogre::Vector3::ZERO;

	m_bCheckBlood=false;
	m_v3ToolPos=Ogre::Vector3::ZERO;

	m_iHookCoolDown = 0;
	m_UseNewToolMode = true;
	m_NewCanClampTube = true;
	
	cutNodeLeft = 0;
	cutNodeRight = 0;

	m_hasRealElectricAttribute = true;

	m_Tex = Ogre::TextureManager::getSingleton().load("cogBrandDefault.tga", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
}

CElectricHook::~CElectricHook()
{
}

Ogre::TexturePtr CElectricHook::GetToolBrandTexture()
{
	return m_Tex;
}
static int hookId=1; 

std::string CElectricHook::GetCollisionConfigEntryName()
{
	return "ElectricHook";
}
bool CElectricHook::Initialize(CXMLWrapperTraining * pTraining)
{
	__super::Initialize(pTraining);

	//for new train
	if(m_pOwnerTraining->m_IsNewTrainMode)
	{
		m_lefttoolpartconvex.SetAttachedNode(m_pNodeKernel);
		m_righttoolpartconvex.SetAttachedNode(m_pNodeKernel);
		
		//special for electric hook use blade all attacht to right part
		m_CutBladeLeft.m_AttachedRB = m_CutBladeRight.m_AttachedRB = m_righttoolpartconvex.m_rigidbody;
		m_CutBladeRight.m_LinPoints[0] = GFPhysVector3(0,  0.3f, -0.4f);
		m_CutBladeRight.m_LinPoints[1] = GFPhysVector3(0, -0.3f, -0.4f);
		m_CutBladeRight.m_CuttDirection = GFPhysVector3(0.0f , 0.0f , 1.0f);
		
		m_CutBladeLeft.m_LinPoints[0] = GFPhysVector3(0, 0.3f,  0.2f);
		m_CutBladeLeft.m_LinPoints[1] = GFPhysVector3(0, -0.3f, 0.2f);
		m_CutBladeLeft.m_CuttDirection = GFPhysVector3(0.0f, 0.0f, -1.0f);
		
	

				
		m_hookShape_Local.m_HookLineRadius = 0.04f;
		m_hookShape_Local.m_HookLinePoints[0] = GFPhysVector3(0 , 0.21f  , 0);//0.1f-m_hookShape_Local.m_HookLineRadius);
		m_hookShape_Local.m_HookLinePoints[1] = GFPhysVector3(0 , -0.21f , 0);//0.1f-m_hookShape_Local.m_HookLineRadius);
		m_hookShape_Local.m_HookSupportOffsetVec = GFPhysVector3(0 , 0 , -0.04f);
		m_hookShape_Local.m_HookProbDir = GFPhysVector3(0 , 1 , 0);



		m_CanPunctureOgran = true;
		m_centertoolpartconvex.m_bSharp = false;
		m_lefttoolpartconvex.m_bSharp = false;
		m_righttoolpartconvex.m_bSharp = false;
		GetHookPart().m_bSharp = true;
	}
	m_HookPlugin = new MisCTool_PluginHook(this , GetHookRigidBodyPart());
	m_ToolPlugins.push_back(m_HookPlugin);
	//

	m_pSparkParticle = MXOgre_SCENEMANAGER->createParticleSystem(Ogre::StringConverter::toString(hookId),PT_SPARK_00);
	Ogre::SceneNode* pSparkNode = m_pNodeKernel->createChildSceneNode();
	pSparkNode->attachObject(m_pSparkParticle);
	m_pSparkParticle->setEmitting(false);
	pSparkNode->setPosition(0.0,0.0,0.75);
	hookId++;
	SmokeManager * smokeMgr = EffectManager::Instance()->GetSmokeManager();
	if(smokeMgr)
	{
	   smokeMgr->setAcceleration(6,30,40);
	   smokeMgr->setInterval(0.4);
	}
	return true;
}
//=============================================================================================================
float CElectricHook::GetHeadPartCollisionLen()
{
	return 0.5f;
}
//=============================================================================================================
void CElectricHook::NewToolModeUpdate()
{
	
}
//=============================================================================================================
void CElectricHook::onEndCheckCollide(GFPhysCollideObject * softobj , GFPhysCollideObject * rigidobj , const GFPhysAlignedVectorObj<GFPhysRSManifoldPoint> & contactPoints)
{
	CTool::onEndCheckCollide(softobj , rigidobj , contactPoints);
}
//============================================================================================================
void CElectricHook::InternalSimulationStart(int currStep , int TotalStep , float dt)
{
	 CTool::InternalSimulationStart(currStep , TotalStep , dt);
	
	 GFPhysRigidBody * hookPart = GetHookRigidBodyPart();

	 GFPhysTransform tranform = hookPart->GetWorldTransform();

	 m_hookShape_World.m_HookLinePoints[0] = tranform*m_hookShape_Local.m_HookLinePoints[0];
	 m_hookShape_World.m_HookLinePoints[1] = tranform*m_hookShape_Local.m_HookLinePoints[1];
	 m_hookShape_World.m_HookProbDir = tranform.GetBasis()*m_hookShape_Local.m_HookProbDir;
	 m_hookShape_World.m_HookSupportOffsetVec = tranform.GetBasis()*m_hookShape_Local.m_HookSupportOffsetVec;
	 m_HookPlugin->UpdateHinHookLine(m_hookShape_Local.m_HookLinePoints[0] , m_hookShape_Local.m_HookLinePoints[1] , m_hookShape_Local.m_HookSupportOffsetVec);//((m_hookShape_World.m_HookLinePoints[0]-m_hookShape_World.m_HookLinePoints[1]).Normalized() ,  m_hookShape_World.m_HookSupportOffsetVec);
}
//==================================================================================
void CElectricHook::InternalSimulationEnd(int currStep , int TotalStep , float dt)
{	
	CTool::InternalSimulationEnd( currStep ,  TotalStep ,  dt);
}

//============================================================================================
bool CElectricHook::Update(float dt)
{	
	__super::Update(dt);

	if(m_HookPlugin && m_HookPlugin->m_FaceBeHooked.size() > 0)
	{
		GFPhysSoftBodyFace * hookedFace = m_HookPlugin->m_FaceBeHooked[0].m_PhysFace;

		GFPhysSoftBody * hookedSoftBody = m_HookPlugin->m_FaceBeHooked[0].m_SoftBody;

		float hookFaceWeights[3] = {0.3333f , 0.3333f , 0.3333f};
		
		MisMedicOrganInterface * organif = (MisMedicOrganInterface *)hookedSoftBody->GetUserPointer();

		if (m_bElectricRightPad)
		{
			m_HookPlugin->ToolElectriced(0 , dt);
		}
		else if (m_bElectricLeftPad)
		{
			m_HookPlugin->ToolElectriced(1 , dt);
		}
		if(m_bElectricRightPad || m_bElectricLeftPad)
		{	
			GFPhysVector3 materialPos = hookedFace->m_Nodes[0]->m_UnDeformedPos * hookFaceWeights[0]
			                           +hookedFace->m_Nodes[1]->m_UnDeformedPos * hookFaceWeights[1]
			                           +hookedFace->m_Nodes[2]->m_UnDeformedPos * hookFaceWeights[2];

			float heatValue = organif->GetCreateInfo().m_BurnRation*dt;

			//organif->HeatAroundUndeformedPoint(materialPos , 0.5f , heatValue);

			//SmokeManager * somkemgr = EffectManager::Instance()->GetSmokeManager();
			//if(somkemgr && m_TouchedOrgans.size() > 0)
			//	somkemgr->addSmoke(GPVec3ToOgre(m_TouchedOrgans[0].m_MinDistPoint), 0.15, 0.1, 5);
		}
		return true;
	}

	//record every organ's be touched surface
	GFPhysVector3 hookparboxmin, hookpartboxmax;

	m_righttoolpartconvex.m_rigidbody->GetCollisionShape()->GetLocalAABB(hookparboxmin, hookpartboxmax);
	float margin = m_righttoolpartconvex.m_rigidbody->GetCollisionShape()->GetMargin();

	m_ValidBurnRadiusThres = GPMin(hookpartboxmax.m_x - hookparboxmin.m_x, hookpartboxmax.m_y - hookparboxmin.m_y);
	m_ValidBurnRadiusThres = GPMin(m_ValidBurnRadiusThres, hookpartboxmax.m_z - hookparboxmin.m_z);
	m_ValidBurnRadiusThres = m_ValidBurnRadiusThres*0.5f - margin;

	TryElecBurnTouchedOrgans(m_OrganFaceSelToBurn, dt);

	if (m_OrganFaceSelToBurn.size() > 0)
	{
		if (m_bElectricRightPad || m_bElectricLeftPad)
		{
			BreakAdhesion();

			SmokeManager * somkemgr = EffectManager::Instance()->GetSmokeManager();
			if (somkemgr)
				somkemgr->addSmoke(GPVec3ToOgre(m_OrganFaceSelToBurn[0].m_MinDistPoint), 0.15, 0.1, 5);
		}
	 }
	

	 if(m_bElectricButton == false)
	 {
	    m_parkfired = false;
	    EffectManager::Instance()->SparkEffectOff();
	 }
	 else
	 {
		UpdateTotalElectricTime(dt);
		if (m_OrganFaceSelToBurn.size() > 0 || m_veinConnectElectrocuted)
		   UpdateValidElectricTime(dt);
	 }

	 if(m_TimeSliceForSpack >= 0.5)
	 {
		m_TimeSliceForSpack = 0 ; 
		m_RandNumForSpark = rand() % 99;
	 }
	 else
		m_TimeSliceForSpack += dt;
	
	 //如果器械通电后会对周围的物体造成影响， 则应该调用函数进行数据的统计
	 UpdateAffectTimeOfCheckObject(dt);

	 m_veinConnectElectrocuted = false;

	 return true;
}
//=============================================================================================================
float CElectricHook::GetFaceToElecBladeDist(GFPhysVector3 triVerts[3])
{
	GFPhysVector3  pointOnTri;
	
	GFPhysVector3  pointOnSeg;
	
	GFPhysVector3  collideNormOnTri;

	float dist = ClosetPtSegmentTriangle(triVerts, m_hookShape_World.m_HookLinePoints, pointOnTri, pointOnSeg, collideNormOnTri);

	if (dist < m_ValidBurnRadiusThres * 1.5f)
	{
		return dist;
	}
	else
	{
		return -1.0f;
	}
}
//=============================================================================================================
int CElectricHook::GetCollideVeinObjectBody(GFPhysRigidBody * bodies[3])
{
	GFPhysRigidBody * bodyl = m_lefttoolpartconvex.m_rigidbody;
	bodies[0] = bodyl;
	return 1;
}
//=============================================================================================================
CElectricHook::VeinHookShapeData CElectricHook::GetLocalHookShapeData()
{
	return m_hookShape_Local;
}
//=============================================================================================================
CElectricHook::VeinHookShapeData CElectricHook::GetWorldHookShapeData()
{
	CElectricHook::VeinHookShapeData worldshapeData;
	
	GFPhysRigidBody * hookpart = GetHookRigidBodyPart();
	
	GFPhysTransform RigidPartTransform = hookpart->GetWorldTransform();

	worldshapeData.m_HookLinePoints[0] = RigidPartTransform*m_hookShape_Local.m_HookLinePoints[0];
	
	worldshapeData.m_HookLinePoints[1] = RigidPartTransform*m_hookShape_Local.m_HookLinePoints[1];

	worldshapeData.m_HookSupportOffsetVec = RigidPartTransform.GetBasis()*m_hookShape_Local.m_HookSupportOffsetVec;

	worldshapeData.m_HookProbDir = RigidPartTransform.GetBasis()*m_hookShape_Local.m_HookProbDir;

	worldshapeData.m_HookLineRadius = m_hookShape_Local.m_HookLineRadius;

	return worldshapeData;
}
//=============================================================================================================
GFPhysRigidBody * CElectricHook::GetHookRigidBodyPart()
{
	return GetHookPart().m_rigidbody;
}
//=============================================================================================================
NewTrainToolConvexData& CElectricHook::GetHookPart()
{
	return m_righttoolpartconvex;
}
//=========================================================================================
//GFPhysVector3 CElectricHook::CalculateToolCustomForceFeedBack()
//{
//	return GFPhysVector3(0,0,0);
//}
//======================================================================================================
Ogre::Vector3 CElectricHook::CalcVeinHookForceFeedBack()
{
	if(m_pOwnerTraining == 0)
	   return Ogre::Vector3(0,0,0);

	std::vector<VeinConnectObject*> Connectobjets = m_pOwnerTraining->GetVeinConnectObjects();
	if(Connectobjets.size() == 0)
	   return Ogre::Vector3(0,0,0);

	GFPhysRigidBody * hookpart = GetHookRigidBodyPart();
	if(hookpart == 0)
	   return Ogre::Vector3(0,0,0);
	
	GFPhysVector3 offsetDir = hookpart->GetWorldTransform().GetBasis() * m_hookShape_Local.m_HookSupportOffsetVec;

	offsetDir.Normalize();

	GFPhysVector3 hookVeinForce(0,0,0);

	for(size_t v = 0 ; v < Connectobjets.size() ; v++)
	{
		VeinConnectObject * veinobj = Connectobjets[v];

		GFPhysVector3 temp = veinobj->CalculateHookForce(hookpart , -offsetDir);

		hookVeinForce += temp;
	}

	float lastHookForce = m_lastHookforceFeedBack.Length();

	float currHookMagnitude = hookVeinForce.Length();

	//prevent force increase suddenly
	float MaxIncPercent = 0.15f;
	if((currHookMagnitude - lastHookForce) > (currHookMagnitude*MaxIncPercent) )
	{
		if(currHookMagnitude > GP_EPSILON)
		   hookVeinForce = hookVeinForce * ((lastHookForce + currHookMagnitude*MaxIncPercent) / currHookMagnitude);
	}

	//prevent force dir change suddenly
	GFPhysVector3 currdir = hookVeinForce.Normalized();
	
	GFPhysVector3 lastdir = m_lastHookforceFeedBack.Normalized();

	float currmag = hookVeinForce.Length();

	if(currdir.Dot(lastdir) < 0.99f)
	{
	   currdir = (9.0f*lastdir + currdir) / 10.0f;
	   currdir.Normalize();
	   hookVeinForce = currdir*currmag;
	}

	//prevent total force magnitude too large
	float MaxForce = 1.2f;
	float hookforcemag = hookVeinForce.Length();
	if(hookforcemag > MaxForce)
	   hookVeinForce = hookVeinForce * (MaxForce / hookforcemag);

	m_lastHookforceFeedBack = hookVeinForce;//LeftForceFeedBack += Ogre::Vector3(hookveinForce.x() , hookveinForce.y() , hookveinForce.z());
	return Ogre::Vector3(hookVeinForce.m_x , hookVeinForce.m_y , hookVeinForce.m_z);
}
//=========================================================================================
bool CElectricHook::EmitSpark(const Ogre::Vector3 & position , bool needRand /* = false  */, int probability /* = 40 */)
{
	if(!needRand)
	{
		EffectManager::Instance()->SparkEffectOn(position,false);//(GPVec3ToOgre(m_hookShape_World.m_HookLinePoints[1]),false);
		return true;
	}
	else
	{
		if(m_RandNumForSpark < probability)
		{
			EffectManager::Instance()->SparkEffectOn(position,false);//(GPVec3ToOgre(m_hookShape_World.m_HookLinePoints[1]),false);
			return true;
		}
	}
	return false;
}

bool CElectricHook::EmitSpark(bool needRand /* = false  */, int probability /* = 40 */)
{
	if(!needRand)
	{
		EffectManager::Instance()->SparkEffectOn(GPVec3ToOgre(m_hookShape_World.m_HookLinePoints[1]),false);
		return true;
	}
	else
	{
		if(m_RandNumForSpark < probability)
		{
			EffectManager::Instance()->SparkEffectOn(GPVec3ToOgre(m_hookShape_World.m_HookLinePoints[1]),false);
			return true;
		}
	}
	return false;
}

//=========================================================================================
void CElectricHook::OnVeinConnectBurned(const std::vector<Ogre::Vector3> & burnpos)
{
	if(burnpos.size() > 0)
	{

		//EffectManager::Instance()->SmokeEffectOn(burnpos[0]);
		if(burnpos.size() >= 5)
			EmitSpark(burnpos[0]);
		else
			EffectManager::Instance()->SparkEffectOff();

		SmokeManager * smokeMgr = EffectManager::Instance()->GetSmokeManager();

		for(int i = 0 ; i < burnpos.size(); i++)
			smokeMgr->addSmoke(burnpos[i],0.15,0.1,4);
		m_veinConnectElectrocuted = true;
	}
}
//=====================================================================================================================
float CElectricHook::GetCutWidth()
{
	return 0.1f;
}
//=========================================================================================
bool CElectricHook::ElectricCutOrgan(MisMedicOrgan_Ordinary * organ , GFPhysSoftBodyFace * face  , float weights[3])
{
	//if(m_HookPlugin && m_HookPlugin->m_FaceBeHooked.size() > 0)
	//{
	  // m_HookPlugin->ElectricCutAtHookPoint();
	   //return true;
	//}
	if(organ->m_OrganID == EDOT_APPENDIX 
	|| organ->m_OrganID == EODT_UTERUS
	|| organ->GetOrganType() == EDOT_ADHESION 
	|| organ->GetOrganType() == EDOT_SIGMOIDCUTPART
	|| organ->GetOrganType() == EDOT_LUNG_FASICA
	|| organ->GetOrganType() == EDOT_MESOCOLON
	|| organ->GetOrganType() == EDOT_GEROTAS
	|| organ->GetCreateInfo().m_IsMensentary)//temple 
	{
		CElectricHook::VeinHookShapeData worldHookLine = GetWorldHookShapeData();
		
		int nFind = -1;
		float mindist = FLT_MAX;
		for(int n = 0 ; n < 3 ; n++)
		{
			if((face->m_Nodes[n]->m_CurrPosition-worldHookLine.m_HookLinePoints[0]).Length() < mindist)//if(face->m_Nodes[n]->m_UserDefValue0.m_y > mindist)
			{
				nFind = n;
				mindist = (face->m_Nodes[n]->m_CurrPosition-worldHookLine.m_HookLinePoints[0]).Length();
			}
		}

		GFPhysVector3 ptOnFace = ClosestPtPointTriangle(worldHookLine.m_HookLinePoints[0],
			face->m_Nodes[0]->m_CurrPosition,
			face->m_Nodes[1]->m_CurrPosition,
			face->m_Nodes[2]->m_CurrPosition);

		float weights[3];
		CalcBaryCentric(face->m_Nodes[0]->m_CurrPosition,
			            face->m_Nodes[1]->m_CurrPosition,
			            face->m_Nodes[2]->m_CurrPosition,
			            ptOnFace, weights[0], weights[1], weights[2]);

		organ->DestroyTissueAroundNode(face, weights ,false);// (face->m_Nodes[nFind]->m_CurrPosition, true);


		EmitSpark(true , 40);
		return true;
	}
	else
	{
		if(!organ->CanBeCut())
		   return CTool::ElectricCutOrgan(organ , face  , weights);
		else
		{	
			
			/*
			CElectricHook::VeinHookShapeData data = GetWorldHookShapeData();

			GFPhysVector3 v010 = (data.m_HookLinePoints[0] - data.m_HookLinePoints[1]).Normalized();
			GFPhysVector3 v001 =  data.m_HookSupportOffsetVec.Normalized();
			
			//expand hook line to a rectangle
			GFPhysVector3 p0 = data.m_HookLinePoints[0] + v010 * 0.05f - v001 * 0.1f;// - scissor->m_CutBladeLeft.m_CuttDirectionWord*0.04f;
			GFPhysVector3 p1 = data.m_HookLinePoints[1] - v010 * 0.05f - v001 * 0.1f;// - scissor->m_CutBladeLeft.m_CuttDirectionWord*0.04f;

			GFPhysVector3 p2 = p0 + v001 * 0.5f;
			GFPhysVector3 p3 = p1 + v001 * 0.5f;

			GFPhysPresetCutPolygonSurface CutSurface;

			CutSurface.m_PolyVerts.clear();

			CutSurface.m_Triangles.clear();

			CutSurface.m_PolyVerts.push_back(p0);
			CutSurface.m_PolyVerts.push_back(p1);
			CutSurface.m_PolyVerts.push_back(p2);
			CutSurface.m_PolyVerts.push_back(p3);


			CutSurface.m_Triangles.push_back(GFPhysPresetCutPolygonSurface::ClipTriangle(CutSurface.m_PolyVerts[0] , CutSurface.m_PolyVerts[1] , CutSurface.m_PolyVerts[2]));
			CutSurface.m_Triangles.push_back(GFPhysPresetCutPolygonSurface::ClipTriangle(CutSurface.m_PolyVerts[2] , CutSurface.m_PolyVerts[1] , CutSurface.m_PolyVerts[3]));	

			organ->TearOrganByCustomedGeomtry(&CutSurface);
			*/
			organ->CutOrganByTool(this);
			EmitSpark(true , 40);
			return true;
		}
	}
}
//=========================================================================================
bool CElectricHook::GetForceFeedBack(Ogre::Vector3 & contactForce, Ogre::Vector3 & dragForce)
{
	CTool::GetForceFeedBack(contactForce, dragForce);

	Ogre::Vector3 hookFeedBack = CalcVeinHookForceFeedBack();
	hookFeedBack *= 1.5f;

	dragForce += hookFeedBack;
	
	return true;
	//stickforceepercent = 0;

	//Ogre::Vector3 totalforce = forceFeedBack+hookFeedBack;

	//float magnitude = totalforce.length();

	//if(magnitude > FLT_EPSILON)
	//{
		//Ogre::Vector3 direction = totalforce / magnitude;

		//float contactMag = forceFeedBack.dotProduct(direction);

	//	float stickedMag = hookFeedBack.dotProduct(direction);
		
		//stickforceepercent = stickedMag / (contactMag+stickedMag);

	//}
	//return totalforce;
}

//=========================================================================================
void CElectricHook::onFrameUpdateStarted( float timeelpased )
{
	CElectricTool::onFrameUpdateStarted(timeelpased);
	//m_smokeManager.update(timeelpased);
}
//=========================================================================================
void CElectricHook::BreakAdhesion()
{
	/*if(m_MinDistFace)
	{
		MisMedicOrganInterface * organif = (MisMedicOrganInterface *)m_MinHookDistBody->GetUserPointer();

		if(organif->GetOrganType() != EDOT_ADHESION)
			return;

		MisMedicOrgan_Ordinary *pOrgan = dynamic_cast<MisMedicOrgan_Ordinary*>(organif);

		if(!pOrgan)
			return;
		
		if(m_MinDistFace->m_GenFace && m_MinDistFace->m_GenFace->m_ShareTetrahedrons.size() > 0)
		{
			GFPhysSoftBodyTetrahedron * pTetra = m_MinDistFace->m_GenFace->m_ShareTetrahedrons[0].m_Hosttetra;
			if(pTetra)
			{
				GFPhysVector3 faceNorm = (m_MinDistFace->m_Nodes[1]->m_UnDeformedPos-m_MinDistFace->m_Nodes[0]->m_UnDeformedPos).Cross(
					m_MinDistFace->m_Nodes[2]->m_UnDeformedPos-m_MinDistFace->m_Nodes[0]->m_UnDeformedPos);

				faceNorm.Normalize();

				GFPhysVector3 deformderv = GFPhysSoftBody::CalTetraDeformationDerivative(*pTetra , faceNorm);

				float deformValue = deformderv.Dot(faceNorm);

				if(abs(deformValue) >  10)
				{
					ElectricCutOrgan(pOrgan , m_MinDistFace , m_MinPointWeights);	
				}
			}
		}
	}*/
}
//========================================================================================================================
int CElectricHook::TryBurnConnectStrips(VeinConnectObject * stripObj, float BurnRate, std::vector<Ogre::Vector3> & burnpos, std::vector<VeinConnectPair*> & burnpairs)
{
	std::set<int> emptyset;
	
	GFPhysRigidBody * worldHook = GetHookRigidBodyPart();

	int burncount = stripObj->BurnHookedAndContactedConnectInternal(emptyset, worldHook, 0, BurnRate, burnpos, burnpairs);

	OnVeinConnectBurned(burnpos);

	return burncount;
}