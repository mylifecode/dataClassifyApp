#include "HarmonicScalpel.h"

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
#include "VeinConnectObject.h"
//#include "../NewTrain/CustomConstraint.h"
#include "../NewTrain/PhysicsWrapper.h"
#include "../NewTrain/MisMedicOrganOrdinary.h"
#include "MisCTool_PluginClamp.h"
#include "BubbleManager.h"
#include "SmokeManager.h"
CHarmonicScalpel::CHarmonicScalpel() : CTool()
{
	m_hasRealElectricAttribute = true;
	m_CanDoElecCut = true;
  //  m_BurnBody = 0;
}

CHarmonicScalpel::CHarmonicScalpel(CXMLWrapperTool * pToolConfig) : CTool(pToolConfig)
{
	m_hasRealElectricAttribute = true;
	m_CanDoElecCut = true;
   // m_BurnBody = 0;
}

CHarmonicScalpel::~CHarmonicScalpel()
{

}
std::string CHarmonicScalpel::GetCollisionConfigEntryName()
{
	//创建抓钳的碰撞体
	return "Ultrasonic scalpe";

}
bool CHarmonicScalpel::Initialize(CXMLWrapperTraining * pTraining)
{
	bool succed = CTool::Initialize(pTraining);

	m_continueElectricValue = 0;

	SetLeftShaftAsideScale(0.0);
	SetRightShaftAsideScale(1.5);

	m_ToolForceBackRate = 1.0f;

	m_pluginclamp = new MisCTool_PluginClamp(this, 3.0f);
	

	m_pluginclamp->SetClampRegion(m_righttoolpartconvex,//.m_rigidbody,
		GFPhysVector3(0 , -0.1f , 0),
		GFPhysVector3(1 , 0 , 0),
		GFPhysVector3(0 , 0 , 1),
		0.1f,
		0.76f,
		MisCTool_PluginClamp::ClampReg_Right,
		1
		);

	m_pluginclamp->SetClampRegion(m_lefttoolpartconvex,//.m_rigidbody,
		GFPhysVector3(0 , 0.1f , 0),
		GFPhysVector3(1 , 0 , 0),
		GFPhysVector3(0 , 0 , 1),
		0.08f,
		0.76f,
		MisCTool_PluginClamp::ClampReg_Left,
		-1
		);
	m_pluginclamp->m_ShowClampRegion = false;
	m_ToolPlugins.push_back(m_pluginclamp);
	

	//m_burnTime = 0.0f;
	//m_isClampAndBurn = false;
	//m_canCut = false;

	
	m_CutBladeRight.m_LinPoints[0] = GFPhysVector3(0 , -0.02 , 0.72f);
	m_CutBladeRight.m_LinPoints[1] = GFPhysVector3(0 , -0.02 , -0.7f);
	m_CutBladeRight.m_CuttDirection = GFPhysVector3(0 , 1.0f , 0);

	m_CutBladeLeft.m_LinPoints[0] = GFPhysVector3(0 ,   0.02 , 0.72f);
	m_CutBladeLeft.m_LinPoints[1] = GFPhysVector3(0 ,   0.02 , -0.7f);
	m_CutBladeLeft.m_CuttDirection = GFPhysVector3(0 , -1.0f ,0);


	m_Tex = Ogre::TextureManager::getSingleton().load("cogBrandBipolar.tga"/*"cogBrandDefault.tga"*/, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	
	m_canClamp = true;
	m_CooldownTime = 0;
	
	SmokeManager * smokeMgr = EffectManager::Instance()->GetSmokeManager();
	if(smokeMgr)
	{
		//smokeMgr->setAcceleration(6,30,40);
		smokeMgr->setInterval(1.2);
	}
	
	return succed;
}

bool CHarmonicScalpel::IsInElecCutting(MisMedicOrgan_Ordinary * organ)
{
	if (m_bElectricLeftPad || m_bElectricRightPad)
	{
		std::vector<MisMedicOrgan_Ordinary*> clampedOrgans;
		m_pluginclamp->GetOrgansBeClamped(clampedOrgans);

		for (size_t c = 0; c < clampedOrgans.size(); c++)
		{
			if (clampedOrgans[c] == organ)
			{
				return true;
			}
		}
	}
	return false;

}

bool CHarmonicScalpel::IsInElecCogulation(MisMedicOrgan_Ordinary * organ)
{
	if (m_bElectricLeftPad || m_bElectricRightPad)
	{
		std::vector<MisMedicOrgan_Ordinary*> clampedOrgans;
		m_pluginclamp->GetOrgansBeClamped(clampedOrgans);

		for (size_t c = 0; c < clampedOrgans.size(); c++)
		{
			if (clampedOrgans[c] == organ)
			{
				return true;
			}
		}
	}
	return false;
}
bool CHarmonicScalpel::Update(float dt)
{
	__super::Update(dt);

	if(!m_canClamp)
	{
		if(m_CooldownTime > 0.5)
		{
			m_canClamp = true;
			m_pluginclamp->ClearIgnored();
		}
		else
			m_CooldownTime += dt;
	}

	
	if(m_bElectricButton)
	{
		if (m_pluginclamp && m_pluginclamp->isInClampState())
		{
			m_continueElectricValue += dt;

			bool needCut = false;

			if (m_continueElectricValue > 2.0f)
			{
				ReleaseClampedOrgans();//释放后抓取插件仍然保留抓取面和点的信息不会释放可以放心使用

				m_CooldownTime = 0.0;
				m_canClamp = false;

				m_continueElectricValue = 0;
				m_CanDoElecCut = false;

				needCut = true;
			}
			else
			{
				ElectricClampedFaces(dt);
			}

			for (int c = 0; c < (int)m_pluginclamp->m_ClampedOrgans.size(); c++)
			{
				MisCTool_PluginClamp::OrganBeClamped * organClamped = m_pluginclamp->m_ClampedOrgans[c];

				const std::vector<GFPhysSoftBodyTetrahedron*> & tretraBeingClamp = organClamped->m_TetrasInClampReg;

				if (needCut == false)//not cut heat and shrink
				{
					organClamped->m_organ->HeatTetrahedrons(tretraBeingClamp, 1.0f*dt);

					if (organClamped->m_organ->m_BurnShrinkRate < 0.999f)
						organClamped->m_organ->ShrinkTetrahedrons(tretraBeingClamp, organClamped->m_ClampDirInMaterialSpace, organClamped->m_organ->m_BurnShrinkRate);
				}
				else//perform cut
				{
					MisMedicOrgan_Ordinary * OrganToCut = organClamped->m_organ;
					if (organClamped->m_ClampMode == 1)
					{
						MisCTool_PluginClamp::SoftBodyFaceClamped& faceclamped = organClamped->m_ClampedFaces[0];

						GFPhysSoftBodyFace* face = faceclamped.m_PhysFace;

						GFPhysVector3 cutQuadVert[4];
						
						GetToolCutPlaneVerts(cutQuadVert);

						int nFind = -1;
						float mindist = FLT_MAX;
						for (int n = 0; n < 3; n++)
						{
							if ((face->m_Nodes[n]->m_CurrPosition - cutQuadVert[0]).Length() < mindist)
							{
								nFind = n;
								mindist = (face->m_Nodes[n]->m_CurrPosition - cutQuadVert[0]).Length();
							}
						}
						//GFPhysSoftBodyNode * Dnode = face->m_Nodes[nFind];

						GFPhysVector3 ptOnFace = ClosestPtPointTriangle(cutQuadVert[0],
							face->m_Nodes[0]->m_CurrPosition,
							face->m_Nodes[1]->m_CurrPosition,
							face->m_Nodes[2]->m_CurrPosition);

						float weights[3];
						CalcBaryCentric(face->m_Nodes[0]->m_CurrPosition,
							face->m_Nodes[1]->m_CurrPosition,
							face->m_Nodes[2]->m_CurrPosition,
							ptOnFace, weights[0], weights[1], weights[2]);

						OrganToCut->DestroyTissueAroundNode(face, weights , false);// (Dnode->m_UnDeformedPos, false);
					}
					else
					{
						OrganToCut->CutOrganByTool(this);
					}
					m_pluginclamp->SetIgnored(OrganToCut);
				}
			}

			//update valid electric time for statistic
			UpdateValidElectricTime(dt);
		}
		else
		{
			TryElecBurnTouchedOrgans( m_OrganFaceSelToBurn , dt);
			
			m_continueElectricValue = 0;
		}
		UpdateTotalElectricTime(dt);
	}
	else
	{
		m_continueElectricValue = 0;
		m_CanDoElecCut = true;
	}

    UpdateAffectTimeOfCheckObject(dt);
	return true;
}

float CHarmonicScalpel::GetFaceToElecBladeDist(GFPhysVector3 triVerts[3])
{
	GFPhysVector3 pl = ClosestPtPointTriangle(m_CutBladeLeft.m_LinePointsWorld[0],  triVerts[0], triVerts[1], triVerts[2]);
	GFPhysVector3 pr = ClosestPtPointTriangle(m_CutBladeRight.m_LinePointsWorld[0], triVerts[0], triVerts[1], triVerts[2]);

	float dl = (pl - m_CutBladeLeft.m_LinePointsWorld[0]).Length();
	float dr = (pr - m_CutBladeRight.m_LinePointsWorld[0]).Length();

		return dl < dr ? dl : dr;
}
/*bool CHarmonicScalpel::ElectricTouchFaces(float dt)
{
	bool isValidElectric = false;
	
    if (m_BurnFace && m_BurnBody)
	{
	   GFPhysSoftBody * sb = GFPhysSoftBody::Upcast(m_BurnBody);
	   if(sb)
	   {
			MisMedicOrganInterface * ogranif = m_pOwnerTraining->GetOrgan(sb);

			if(ogranif)
			{
				if (m_bElectricRightPad)
				{
					ogranif->Tool_InElec_TouchFacePoint(this , m_BurnFace , m_BurnFaceWeights , 0 , dt);
				}
				else if (m_bElectricLeftPad)
				{
					ogranif->Tool_InElec_TouchFacePoint(this , m_BurnFace , m_BurnFaceWeights , 1, dt);
				}
			}
		}
	}
	return isValidElectric;
}
*/

//=========================================================================================
bool CHarmonicScalpel::ElectricCutOrgan(MisMedicOrgan_Ordinary * organ , GFPhysSoftBodyFace * face  , float weights[3])
{
	if (m_CanDoElecCut == false)
		return false;

	if(organ->m_OrganID == EDOT_APPENDIX 
	|| organ->m_OrganID == EODT_UTERUS 
	|| organ->GetOrganType() == EDOT_ADHESION 
	|| organ->GetOrganType() == EDOT_SIGMOIDCUTPART
	|| organ->GetOrganType() == EDOT_MESOCOLON)//temple 
	{
		int nFind = -1;
		if(weights[0] > weights[1])
		{
			if(weights[0] > weights[2])
			{
				nFind = 0;
			}
			else
			{
				nFind = 2;
			}
		}
		else
		{
			if(weights[1] > weights[2])
			{
				nFind = 1;
			}
			else
			{
				nFind = 2;
			}
		}
		organ->DestroyTissueAroundNode(face , weights , false);// (face->m_Nodes[nFind]->m_CurrPosition, true);

		return true;
	}
	else
        return false;
}

int CHarmonicScalpel::TryBurnConnectStrips(VeinConnectObject * stripObj, float BurnRate, std::vector<Ogre::Vector3> & burnpos, std::vector<VeinConnectPair*> & burnPairs)
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
	
	//addsomke
	if (burnpos.size() > 0)
	{
		SmokeManager * smokeMgr = EffectManager::Instance()->GetSmokeManager();

		for (size_t i = 0; i < burnpos.size(); i++)
			smokeMgr->addSmoke(burnpos[i], 0.15, 0.1, 4);
	}

	return false;
}

void CHarmonicScalpel::onFrameUpdateStarted(float timeelapsed)
{
	CTool::onFrameUpdateStarted(timeelapsed);
// 	if(m_isClampAndBurn)
// 		m_burnTime += timeelapsed;
// 	else
// 	{
// 		m_burnTime = 0;
// 	}
// 	if(!m_canClamp)
// 	{
// 		m_CooldownTime += timeelapsed;
// 	}
	//m_time_elapse = timeelapsed;
}


bool CHarmonicScalpel::ElectricClampedFaces(float dt)
{	
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

			MisMedicOrgan_Ordinary * organInClamp = clampOrgans[c];
			m_pluginclamp->GetFacesBeClamped(FacesInClamp, organInClamp);

		    for(size_t f = 0 ; f < FacesInClamp.size() ; f++)
		    {
			    GFPhysSoftBodyFace * face = FacesInClamp[f];
			    
				float angle0 = fabsf(face->m_FaceNormal.Dot(m_pluginclamp->GetClampRegion(0).m_ClampNormalWorld));
				float angle1 = fabsf(face->m_FaceNormal.Dot(m_pluginclamp->GetClampRegion(1).m_ClampNormalWorld));

				if (angle0 < 0.7 && angle1 < 0.7)
				{
					continue;
				}

				int faceID = organInClamp->GetOriginFaceIndexFromUsrData(face);

				MMO_Face & FaceData = organInClamp->GetMMOFace_OriginPart(faceID);
				if (FaceData.m_HasError == false)
				{
					FaceData.m_BurnValue += 0.1f;
					for (int k = 0; k < 3; k++)
					{
						float weis[3];
						weis[0] = (k != 0) ? 0 : 1;
						weis[1] = (k != 1) ? 0 : 1;
						weis[2] = (k != 2) ? 0 : 1;
						Ogre::Vector2 textureCoord = organInClamp->GetTextureCoord(face, weis);
						TextureCoords.push_back(textureCoord);
						float U = 0;
						float V = 0;
						m_pluginclamp->CollectSubFaceInClampRegionUV(face, k, U, V, 1.2f);
						Ogre::Vector2 UV = Ogre::Vector2(U, V);
						TFUV.push_back(UV);
					}
				}
				else
				{
					MMO_Face & FaceData = organInClamp->GetMMOFace_CutPart(faceID);
					if (FaceData.m_HasError == false)
					{
						FaceData.m_BurnValue += 0.1f;
					}
				}

				
			    isValidElectric = true;
		    }
			organInClamp->ToolElectricClampedFaces(this, TextureCoords, TFUV, dt);
		}
		//smoke
		if (!FacesInClamp.empty())
		{
			SmokeManager * smokemgr = EffectManager::Instance()->GetSmokeManager();
			GFPhysVector3 pos = (*(FacesInClamp.begin()))->m_Nodes[0]->m_CurrPosition;
			Ogre::Vector3 emitPt = GPVec3ToOgre(pos);
			smokemgr->addSmoke(emitPt, 0.15, 0.1, 5);

			//apply bubble effect
			BubbleManager * bubbleMgr = EffectManager::Instance()->GetBubbleManager();
			if (bubbleMgr->BeginRandomAddBubble())
			{
				BubbleControlInfoForClamp controlInfo;
				controlInfo.Left = -m_pluginclamp->GetHalfExt0();
				controlInfo.Right = m_pluginclamp->GetHalfExt0();
				controlInfo.Bottom = 0;
				controlInfo.Top = m_pluginclamp->GetHalfExt1();
				controlInfo.XAllowRange = m_pluginclamp->GetHalfExt0() * 0.2;
				controlInfo.YAllowRange = m_pluginclamp->GetHalfExt1() * 0.15;

				controlInfo.MinExplosionSize = 0.04f;
				controlInfo.MaxExplosionSize = 0.07f;

				controlInfo.ExpandRate = 0.075f;

				for (size_t f = 0; f < FacesInClamp.size(); f++)
				{
					GFPhysSoftBodyFace * face = FacesInClamp[f];

					if (face->IsCutCrossFace() == false)
					{
						Ogre::Vector2 faceVertices2D[3];
						m_pluginclamp->GetFaceVertices2DPos(*face, faceVertices2D);
						float faceArea = face->m_RestAreaMult2;
						int bubbleNum = faceArea*15.0f;
						if (bubbleNum < 1)
							bubbleNum = 1;
						bubbleMgr->addBubblesForClamp(face, faceVertices2D, controlInfo, bubbleNum);
					}
				}
			}
		}
	}
	return isValidElectric;
}

void CHarmonicScalpel::InternalSimulationStart(int currStep , int TotalStep , float dt)
{
	CTool::InternalSimulationStart(currStep , TotalStep , dt);

	//m_BurnFace = 0;
	//m_BurnBody = 0;
}

void CHarmonicScalpel::onEndCheckCollide(GFPhysCollideObject * softobj , GFPhysCollideObject * rigidobj , const GFPhysAlignedVectorObj<GFPhysRSManifoldPoint> & contactPoints)
{
	CTool::onEndCheckCollide(softobj , rigidobj , contactPoints);
	//call all plugins
	/*if( rigidobj == m_lefttoolpartconvex.m_rigidbody || rigidobj == m_righttoolpartconvex.m_rigidbody)//collide with this tool
	{
		if(contactPoints.size() > 0)
		{
			//use the first one
			const GFPhysRSManifoldPoint & rsManiPoint = contactPoints[0];

			m_BurnFace = rsManiPoint.m_collideface;

			m_BurnBody = GFPhysSoftBody::Upcast(softobj);

			m_BurnFaceWeights[0] = rsManiPoint.m_weights[0];
			m_BurnFaceWeights[1] = rsManiPoint.m_weights[1];
			m_BurnFaceWeights[2] = rsManiPoint.m_weights[2];
		}
		
	}*/
}
//================================================================================================================
Ogre::TexturePtr CHarmonicScalpel::GetToolBrandTexture()
{
	return m_Tex;
}
//================================================================================================================
float CHarmonicScalpel::GetCutWidth()
{
	return 0.15f;
}