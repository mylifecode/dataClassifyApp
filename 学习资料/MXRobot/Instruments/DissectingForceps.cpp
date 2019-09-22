#include "DissectingForceps.h"

#include "Topology/GoPhysSoftBodyRestShapeModify.h"
#include "Dynamic/GoPhysDynamicLib.h"
#include "Math/GoPhysTransformUtil.h"

#include "XMLWrapperTool.h"
#include "XMLWrapperOrgan.h"
#include "BasicTraining.h"
#include "MisRobotInput.h"
#include "MXDebugInf.h"
#include "MXEvent.h"
#include "MXEventsDump.h"
#include "EffectManager.h"
#include "XMLWrapperPart.h"
#include "XMLWrapperPursue.h"
#include "InputSystem.h"
#include "SmokeManager.h"
//#include "../NewTrain/CustomConstraint.h"
#include "../NewTrain/PhysicsWrapper.h"
#include "../NewTrain/MisMedicOrganOrdinary.h"
#include "MisCTool_PluginClampConnectPair.h"
#include "MisCTool_PluginClamp.h"
#include "MisCTool_PluginDissectConnectPair.h"
#include "NewTrain/VeinConnectObject.h"


CDissectingForceps::CDissectingForceps():m_pDynamicObject( NULL )
{
	//m_vecIntersectResults.clear();
	m_bHaveCutted = false;
	m_nClampMaxShaftAside = 6;

	m_dReleaseTime = 0;
	//m_nLastShaftAsideForClamp = 0;
	m_bFlagForClampDistance = false;
	m_nExpandNum = 0;
	m_fRaiseDis = 0.0f;
}

CDissectingForceps::CDissectingForceps(CXMLWrapperTool * pToolConfig) : CElectricTool(pToolConfig), m_pDynamicObject( NULL )
{
	m_bHaveCutted = false;
	m_nClampMaxShaftAside = 6;

	m_dReleaseTime = 0;

	m_bFlagForClampDistance = false;
	m_nExpandNum = 0;
	m_fRaiseDis = 0.0f;
}

CDissectingForceps::~CDissectingForceps()
{
	m_pDynamicObject = NULL;
	
}

std::string CDissectingForceps::GetCollisionConfigEntryName()
{
	//创建抓钳的碰撞体
	return "Disection Forceps";
}

bool CDissectingForceps::Initialize(CXMLWrapperTraining * pTraining)
{
	__super::Initialize(pTraining);

	m_CutBladeLeft.m_LinPoints[0]  = GFPhysVector3(-0.25f , -0.001f , 1.25f);
	m_CutBladeLeft.m_LinPoints[1]  = GFPhysVector3(-0.0f , -0.001f , 0.9f);
	m_CutBladeLeft.m_CuttDirection = GFPhysVector3(0 , 1.0f , 0);

	m_CutBladeRight.m_LinPoints[0]  = GFPhysVector3(-0.25f , 0.001f , 1.25f);
	m_CutBladeRight.m_LinPoints[1]  = GFPhysVector3(-0.0f , 0.001f , 0.9f);
	m_CutBladeRight.m_CuttDirection = GFPhysVector3(0 , -1.0f , 0);


	m_cutBlade[0][0] = m_CutBladeLeft;
	m_cutBlade[0][1] = m_CutBladeRight;

	m_cutBlade[1][0].m_LinPoints[0] = GFPhysVector3(-0.10f, -0.001f , 1.10f);
	m_cutBlade[1][0].m_LinPoints[1] = GFPhysVector3(0.08f , -0.001f , 0.7f);
	m_cutBlade[1][0].m_CuttDirection = GFPhysVector3(0 , 1.0f , 0);

	m_cutBlade[1][1].m_LinPoints[0] = GFPhysVector3(-0.10f, 0.001f , 1.10f);
	m_cutBlade[1][1].m_LinPoints[1] = GFPhysVector3(0.08f , 0.001f , 0.7f);
	m_cutBlade[1][1].m_CuttDirection = GFPhysVector3(0 , -1.0f , 0);

	m_cutBlade[2][0].m_LinPoints[0] = GFPhysVector3(0.08f , -0.001f , 0.7f);
	m_cutBlade[2][0].m_LinPoints[1] = GFPhysVector3(0.05f , -0.001f , 0.2f);
	m_cutBlade[2][0].m_CuttDirection = GFPhysVector3(0 , 1.0f , 0);

	m_cutBlade[2][1].m_LinPoints[0] = GFPhysVector3(0.08f , 0.001f , 0.7f);
	m_cutBlade[2][1].m_LinPoints[1] = GFPhysVector3(0.05f , 0.001f , 0.2f);
	m_cutBlade[2][1].m_CuttDirection = GFPhysVector3(0 , -1.0f , 0);
	
	//MisMedicCToolPluginInterface * pluginclampconnect = new MisCTool_PluginClampConnectPair(this);
	//m_ToolPlugins.push_back(pluginclampconnect);

	//三角形顶点必须按逆时针顺序排列
	Ogre::Vector2 rootPt0 = Ogre::Vector2(-0.22 , 0);
	Ogre::Vector2 rootPt1 = Ogre::Vector2(0.23 , 0);

	Ogre::Vector2 middelPt0 = Ogre::Vector2(-0.08,0.65);
	Ogre::Vector2 middelPt1 = Ogre::Vector2(0.22,0.6);

	Ogre::Vector2 topPt0 = Ogre::Vector2(-0.28, 1.25);
	Ogre::Vector2 topPt1 = Ogre::Vector2(-0.00 , 1.08);

   
	Ogre::Vector2 tirVertices[12] = { middelPt0 , middelPt1 , topPt0 ,
		                              middelPt1 , topPt1 , topPt0,
		
		                              rootPt0 , rootPt1 , middelPt0,
		                              rootPt1 , middelPt1, middelPt0
	                                };
	m_pluginclamp = new MisCTool_PluginClamp(this, 3.0f);
	m_pluginclamp->SetClampRegion(m_righttoolpartconvex,//.m_rigidbody,
		GFPhysVector3(0 , 0.02f , 0),
		GFPhysVector3(1 , 0 , 0),
		GFPhysVector3(0 , 0 , 1),
		tirVertices,
		12,
		MisCTool_PluginClamp::ClampReg_Right,
		1.0f
		);
	m_pluginclamp->SetClampRegion(m_lefttoolpartconvex,//.m_rigidbody,
		GFPhysVector3(0 , -0.02f , 0),
		GFPhysVector3(1 , 0 , 0),
		GFPhysVector3(0 , 0 , 1),
		tirVertices,
		12,
		MisCTool_PluginClamp::ClampReg_Left,
		-1.0f
		); 
	m_pluginclamp->m_ShowClampRegion = false;
	m_ToolPlugins.push_back(m_pluginclamp);

    
    m_pluginRigidhold = new MisCTool_PluginRigidHold(this); 

	m_pluginRigidhold->SetHoldRegion(m_righttoolpartconvex,//.m_rigidbody,
		GFPhysVector3(0, 0.02f, 0),
		GFPhysVector3(1, 0, 0),
		GFPhysVector3(0, 0, 1),
		tirVertices,
		12,
		MisCTool_PluginRigidHold::HoldReg_Right,
		1.0f
        );

	m_pluginRigidhold->SetHoldRegion(m_lefttoolpartconvex,//.m_rigidbody,
		GFPhysVector3(0, -0.02f, 0),
		GFPhysVector3(1, 0, 0),
		GFPhysVector3(0, 0, 1),
		tirVertices,
		12,
		MisCTool_PluginRigidHold::HoldReg_Left,
		-1.0f
        ); 

    m_ToolPlugins.push_back(m_pluginRigidhold);


	MisCTool_PluginDissectConnectPair * pDissectPlugin = new MisCTool_PluginDissectConnectPair(this , 13.f);
	pDissectPlugin->SetToolRegion(m_lefttoolpartconvex.m_rigidbody,
		GFPhysVector3(0 , 0.0f , 0),
		GFPhysVector3(0 , -1.0f , 0),
		0.0f,
		0);

	pDissectPlugin->SetToolRegion(m_righttoolpartconvex.m_rigidbody,
		GFPhysVector3(0 , 0.0f , 0),
		GFPhysVector3(0 , 1.0f , 0),
		0.0f,
		1);
	
	m_ToolPlugins.push_back(pDissectPlugin);


	if ( NULL == m_pToolConfig )
	{
		return false;
	}
	if ( "" == m_pToolConfig->m_SubType )
	{
		m_fRaiseDis = 0.1f;
	}
	else if ( "Size10" == m_pToolConfig->m_SubType )
	{
		m_fRaiseDis = 0.67f;
	} 
	
	//尖端位置
	m_tipPoint_Local[0] = GFPhysVector3(-0.3, 0 , 1.25);
	m_tipPoint_Local[1] = GFPhysVector3(-0.3, 0 , 1.25);

	m_MinDistToCollidePoint[0] = FLT_MAX;
	m_MinDistToCollidePoint[1] = FLT_MAX;
	m_MinDistFace[0] = NULL;
	m_MinDistFace[1] = NULL;
	m_MinCollideDistBody[0] = NULL;
	m_MinCollideDistBody[1] = NULL;

	m_time_elapse = 0;

	SmokeManager * smokeMgr = EffectManager::Instance()->GetSmokeManager();
	if(smokeMgr)
	{
		smokeMgr->setAcceleration(6,30,40);
		smokeMgr->setInterval(0.4);
	}

	m_brandTex = Ogre::TextureManager::getSingleton().load("CoagForDissectingForceps.tga", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	return true;
}

void CDissectingForceps::Release()
{
	//m_enmState = TS_RELEASE;
	if (m_enmSide == TSD_LEFT)
	{
		InputSystem::GetInstance(DEVICETYPE_LEFT)->ReleaseClamp();
	}
	else if (m_enmSide == TSD_RIGHT)
	{
		InputSystem::GetInstance(DEVICETYPE_RIGHT)->ReleaseClamp();
	}

	SetMinShaftAside(0);
	m_dReleaseTime = GetTickCount();
}

bool CDissectingForceps::Update(float dt)
{
	__super::Update(dt);

	BreakAdhesion();

	if(m_bElectricButton)
	{
		if (m_pluginclamp && m_pluginclamp->isInClampState())
		{
			bool isCut = false;
			
			if (m_bElectricRightPad)
				isCut = false;
			else if(m_bElectricLeftPad)
				isCut = true;
				
			ElectricClampedFaces(dt , isCut);
			UpdateValidElectricTime(dt);
		}
		else
		{
			ElectricTouchFaces(dt);
		}
		UpdateTotalElectricTime(dt);
	}
	
	return true;
}

Ogre::TexturePtr CDissectingForceps::GetToolBrandTexture()
{
	return m_brandTex;
}

void CDissectingForceps::BreakAdhesion()
{
	if (m_pluginclamp && m_pluginclamp->isInClampState())
	{
		std::vector<MisMedicOrgan_Ordinary *> clampOrgans;
		m_pluginclamp->GetOrgansBeClamped(clampOrgans);

	   for(size_t c = 0 ; c < clampOrgans.size() ; c++)
	   {
		   MisMedicOrgan_Ordinary * pAdhesion = clampOrgans[c];
		   if(pAdhesion->GetOrganType() == EDOT_ADHESION)
		   {
			   if (m_pluginclamp->GetMoveDistAfterClamped() > 0.5)
				   CutOrgan(pAdhesion);
		   }
	   }
	}
}

int CDissectingForceps::TryBurnConnectStrips(VeinConnectObject * stripObj, float BurnRate, std::vector<Ogre::Vector3> & burnpos, std::vector<VeinConnectPair*> & burnPairs)
{
	std::set<int> ClampOrganIDSet;
	
	if (m_pluginclamp && m_pluginclamp->isInClampState())
	{
		std::vector<MisMedicOrgan_Ordinary *> clampOrgans;
		m_pluginclamp->GetOrgansBeClamped(clampOrgans);

		for (int c = 0, nc = clampOrgans.size(); c < nc; c++)
		{
			ClampOrganIDSet.insert((int)clampOrgans[c]->GetOrganType());
		}
	}

	int burncount = stripObj->BurnHookedAndContactedConnectInternal(ClampOrganIDSet,
		                                                            m_lefttoolpartconvex.m_rigidbody,
																	m_righttoolpartconvex.m_rigidbody, 
																	BurnRate, 
																	burnpos, 
																	burnPairs);
    OnVeinConnectBurned(burnpos);

	return false;
}

void CDissectingForceps::InternalSimulationStart(int currStep , int TotalStep , float dt)
{
	CTool::InternalSimulationStart(currStep , TotalStep , dt);

	m_MinDistToCollidePoint[0] = FLT_MAX;
	m_MinDistToCollidePoint[1] = FLT_MAX;
	m_MinDistFace[0] = NULL;
	m_MinDistFace[1] = NULL;
	m_MinCollideDistBody[0] = NULL;
	m_MinCollideDistBody[1] = NULL;

	GFPhysRigidBody *leftRigidBody = m_lefttoolpartconvex.m_rigidbody;
	GFPhysRigidBody *rightRigidBody = m_righttoolpartconvex.m_rigidbody;

	GFPhysTransform leftTranform = leftRigidBody->GetWorldTransform();
	GFPhysTransform rightTranform = rightRigidBody->GetWorldTransform();

	m_tipPoint_World[0] = leftTranform * m_tipPoint_Local[0];
	m_tipPoint_World[1] = rightTranform * m_tipPoint_Local[1];

}

void CDissectingForceps::InternalSimulationEnd(int currStep , int TotalStep , float dt)
{
	CTool::InternalSimulationEnd(currStep , TotalStep , dt);
	
	for(int i = 0 ; i < 3 ; i++)
	{
		m_cutBlade[i][0].m_LinePointsWorld[0] = m_lefttoolpartconvex.m_rigidbody->GetWorldTransform() * m_cutBlade[i][0].m_LinPoints[0];
		m_cutBlade[i][0].m_LinePointsWorld[1] = m_lefttoolpartconvex.m_rigidbody->GetWorldTransform() * m_cutBlade[i][0].m_LinPoints[1];

		m_cutBlade[i][1].m_LinePointsWorld[0] = m_righttoolpartconvex.m_rigidbody->GetWorldTransform() * m_cutBlade[i][1].m_LinPoints[0];
		m_cutBlade[i][1].m_LinePointsWorld[1] = m_righttoolpartconvex.m_rigidbody->GetWorldTransform() * m_cutBlade[i][1].m_LinPoints[1];
	}
}

void CDissectingForceps::onFrameUpdateStarted(float timeelapsed)
{
	CTool::onFrameUpdateStarted(timeelapsed);
	m_time_elapse = timeelapsed;
}

void CDissectingForceps::onEndCheckCollide(GFPhysCollideObject * softobj , GFPhysCollideObject * rigidobj , const GFPhysAlignedVectorObj<GFPhysRSManifoldPoint> & contactPoints)
{
	CTool::onEndCheckCollide(softobj , rigidobj , contactPoints);
	//call all plugins
	if( rigidobj == m_lefttoolpartconvex.m_rigidbody || rigidobj == m_righttoolpartconvex.m_rigidbody)//collide with this tool
	{
		int side = 0;

		if( rigidobj == m_lefttoolpartconvex.m_rigidbody )
			side = 0;
		else 
			side = 1;

		GFPhysRigidBody * rigidbody = GFPhysRigidBody::Upcast(rigidobj);

		for(size_t c = 0 ; c < contactPoints.size(); c++)
		{
			const GFPhysRSManifoldPoint & rsManiPoint = contactPoints[c];

			GFPhysSoftBodyFace * facecollide = rsManiPoint.m_collideface;

			GFPhysVector3 closetPointInTri = ClosestPtPointTriangle(m_tipPoint_World[side],		//here
				facecollide->m_Nodes[0]->m_CurrPosition,
				facecollide->m_Nodes[1]->m_CurrPosition, 
				facecollide->m_Nodes[2]->m_CurrPosition);

			float dist = (closetPointInTri- m_tipPoint_World[side]).Length();
			if( dist < m_MinDistToCollidePoint[side])
			{
				m_MinDistToCollidePoint[side] = dist;
				m_MinDistFace[side] = facecollide;
				m_MinDistPoint[side] = closetPointInTri;
				m_MinCollideDistBody[side] = softobj;
				CalcBaryCentric(m_MinDistFace[side]->m_Nodes[0]->m_CurrPosition , 
					m_MinDistFace[side]->m_Nodes[1]->m_CurrPosition , 
					m_MinDistFace[side]->m_Nodes[2]->m_CurrPosition, 
					m_MinDistPoint[side] ,
					m_MinPointWeights[side][0] , 
					m_MinPointWeights[side][1] , 
					m_MinPointWeights[side][2]);
			}
		}
	}
}

void CDissectingForceps::OnVeinConnectBurned(const std::vector<Ogre::Vector3> & burnpos)
{
	if(burnpos.size() > 0)
	{
		SmokeManager * smokeMgr = EffectManager::Instance()->GetSmokeManager();

		for(size_t i = 0 ; i < burnpos.size(); i++)
			smokeMgr->addSmoke(burnpos[i],0.15,0.1,4);
	}
}

bool CDissectingForceps::ElectricClampedFaces(float dt , bool isCut)
{
	m_canCut = false;
	
	bool isValidElectric = false;
	
	if (m_pluginclamp && m_pluginclamp->isInClampState())
	{
	    GFPhysVectorObj<GFPhysSoftBodyFace*> FacesInClamp;
		std::vector<Ogre::Vector2> TextureCoords;
		std::vector<Ogre::Vector2> TFUV;

		int burn_face_num = 0;

		std::vector<MisMedicOrgan_Ordinary *> clampOrgans;
		m_pluginclamp->GetOrgansBeClamped(clampOrgans);

		m_pluginclamp->CollectPos();
		
        for(size_t c = 0 ; c < clampOrgans.size() ; c++)
		{
			FacesInClamp.clear();
			TextureCoords.clear();
			TFUV.clear();

            MisMedicOrgan_Ordinary * misOrgan = clampOrgans[c];
			m_pluginclamp->GetFacesBeClamped(FacesInClamp, misOrgan);
            
			for(size_t f = 0 ; f < FacesInClamp.size() ; f++)
			{
				GFPhysSoftBodyFace * face = FacesInClamp[f];
				for (int k = 0; k < 3; k++)
				{
					float weis[3];
					weis[0] = (k != 0)?0:1;
					weis[1] = (k != 1)?0:1;
					weis[2] = (k != 2)?0:1;
					Ogre::Vector2 textureCoord = misOrgan->GetTextureCoord(face , weis);
					TextureCoords.push_back(textureCoord);
					float U = 0;
					float V = 0;
					m_pluginclamp->CollectSubFaceInClampRegionUV(face, k, U, V, 1.0f);
					Ogre::Vector2 UV = Ogre::Vector2(U, V);
					TFUV.push_back(UV);
				}
				if(isCut)
				{
					std::map<GFPhysSoftBodyFace*,float>::iterator itor =  m_burn_record.find(face);
					if(itor != m_burn_record.end())
					{
						itor->second += m_time_elapse;
						if(itor->second > 3.5)
						   burn_face_num++;
					}
					else
					{
						m_burn_record.insert(pair<GFPhysSoftBodyFace*,float>(face,0.0f));
					}
				}
				isValidElectric = true;
			}
			misOrgan->ToolElectricClampedFaces(this, TextureCoords, TFUV, dt);
			
			if(isCut && FacesInClamp.size() > 0)
			{
			   if((float)burn_face_num / FacesInClamp.size() > 0.8)
				  m_canCut = true;
			}
		}
	
		//heat clamped tetrahedrons
		for (int c = 0; c < (int)m_pluginclamp->m_ClampedOrgans.size(); c++)
		{
			MisCTool_PluginClamp::OrganBeClamped * clampOrgan = m_pluginclamp->m_ClampedOrgans[c];

			const std::vector<GFPhysSoftBodyTetrahedron*> & tretraBeingClamp = clampOrgan->m_TetrasInClampReg;

			float heatValue = clampOrgan->m_organ->GetCreateInfo().m_BurnRation*dt;

			clampOrgan->m_organ->HeatTetrahedrons(tretraBeingClamp, heatValue);
		}

		//smoke
		if (!FacesInClamp.empty())
		{
			SmokeManager * smokemgr = EffectManager::Instance()->GetSmokeManager();
			GFPhysVector3 pos = (*(FacesInClamp.begin()))->m_Nodes[0]->m_CurrPosition;
			Ogre::Vector3 emitPt = GPVec3ToOgre(pos);
			smokemgr->addSmoke(emitPt, 0.15, 0.1, 5);
		}

		//if(m_canCut && misOrgan->CanBeCut())
		//{
			//CutOrgan(misOrgan);
 			//ReleaseClampedOrgans();
		//}
	}
	return isValidElectric;
}


bool CDissectingForceps::ElectricTouchFaces(float dt)
{
	bool isValidElectric = false;
	for(int i = 0 ; i < 2 ; i++)
	{
		if(m_MinCollideDistBody[i] && m_MinDistFace[i])
		{
			GFPhysSoftBody * sb = GFPhysSoftBody::Upcast(m_MinCollideDistBody[i]);
			if(sb)
			{
				MisMedicOrganInterface * ogranif = m_pOwnerTraining->GetOrgan(sb);

				if(ogranif)
				{
					if (m_bElectricRightPad)
					{
						ogranif->Tool_InElec_TouchFacePoint(this , m_MinDistFace[i] , m_MinPointWeights[i] , 0 , dt);
					}
					else if (m_bElectricLeftPad)
					{
						ogranif->Tool_InElec_TouchFacePoint(this ,  m_MinDistFace[i] , m_MinPointWeights[i] , 1, dt);
					}

					if(m_MinDistFace[i] && (m_bElectricRightPad || m_bElectricLeftPad))
					{	
						GFPhysVector3 materialPos = m_MinDistFace[i]->m_Nodes[0]->m_UnDeformedPos * m_MinPointWeights[i][0]
						+m_MinDistFace[i]->m_Nodes[1]->m_UnDeformedPos * m_MinPointWeights[i][1]
						+m_MinDistFace[i]->m_Nodes[2]->m_UnDeformedPos * m_MinPointWeights[i][2];

						ogranif->HeatAroundUndeformedPoint(materialPos , 0.5f , 1.0f*dt);

						GFPhysVector3 smokepos = m_MinDistFace[i]->m_Nodes[0]->m_CurrPosition*m_MinPointWeights[i][0]
						+m_MinDistFace[i]->m_Nodes[1]->m_CurrPosition * m_MinPointWeights[i][1]
						+m_MinDistFace[i]->m_Nodes[2]->m_CurrPosition * m_MinPointWeights[i][2];

						SmokeManager * somkemgr = EffectManager::Instance()->GetSmokeManager();
						if(somkemgr)
							somkemgr->addSmoke(GPVec3ToOgre(smokepos), 0.15, 0.1, 5);
						isValidElectric = true;
					}
				}
			}
		}
	}

	return isValidElectric;
}

void CDissectingForceps::CutOrgan(MisMedicOrgan_Ordinary *organ)
{
    ReleaseClampedOrgans();
	m_cutBlade[0][0] = m_CutBladeLeft;
	m_cutBlade[0][1] = m_CutBladeRight;

	m_CutBladeLeft = m_cutBlade[1][0];
	m_CutBladeRight = m_cutBlade[1][1];

	organ->CutOrganByTool(this);

#if(0)
	m_CutBladeLeft = m_cutBlade[2][0];
	m_CutBladeRight = m_cutBlade[2][1];
	organ->CutOrganByTool(this);
#endif

	m_CutBladeLeft = m_cutBlade[0][0];
	m_CutBladeRight = m_cutBlade[0][1];
}

