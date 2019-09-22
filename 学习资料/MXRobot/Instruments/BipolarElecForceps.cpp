#include "BipolarElecForceps.h"

#include "Topology/GoPhysSoftBodyRestShapeModify.h"
#include "Dynamic/GoPhysDynamicLib.h"
#include "Math/GoPhysTransformUtil.h"

#include "XMLWrapperTool.h"
#include "BasicTraining.h"
//#include "DynamicObject.h"
#include "EffectManager.h"
#include "MXEventsDump.h"
#include "MXEvent.h"

#include "../NewTrain/CustomConstraint.h"
#include "../NewTrain/PhysicsWrapper.h"
#include "../NewTrain/MisMedicOrganOrdinary.h"
#include "MisCTool_PluginClamp.h"
#include "MXOgreWrapper.h"
#include "AcessoriesCutTrain.h"
#include "ElectrocoagulationTrain.h"
#include "TrainingMgr.h"
#include "BubbleManager.h"
#include "SmokeManager.h"
#include "VeinConnectObject.h"
#define ORIGINMATERIALID 1
#define CUTMATERIALID 2
//inline Ogre::Vector3 GPVec3ToOgre(const GFPhysVector3 & gpvec3)
//{
//	/return Ogre::Vector3(gpvec3.m_x , gpvec3.m_y , gpvec3.m_z);
//}

CBipolarElecForceps::CBipolarElecForceps() : m_counter(-1)
{
	m_BurnCutFaceTime = 0.f;
	m_IsLastBurnCutFace = false;
//	m_TimeSinceLastSmokeAdd = 0;
}

CBipolarElecForceps::CBipolarElecForceps(CXMLWrapperTool * pToolConfig) : CElectricTool(pToolConfig)
{
	m_BurnCutFaceTime = 0.f;
	m_IsLastBurnCutFace = false;
	m_counter = -1;
	//m_TimeSinceLastSmokeAdd = 0;
}

CBipolarElecForceps::~CBipolarElecForceps()
{
}

std::string CBipolarElecForceps::GetCollisionConfigEntryName()
{
	return "BipolarElecForceps";
}
bool CBipolarElecForceps::Initialize(CXMLWrapperTraining * pTraining)
{
	__super::Initialize(pTraining);

	//tool rigid
	if(m_pOwnerTraining->m_IsNewTrainMode)
	{
		//add clamp plug in
		if (m_pluginclamp == 0)
		{
			m_pluginclamp = new MisCTool_PluginClamp(this, 3.0f);
			//m_pluginClamp->m_ShowClampRegion = true;
			m_pluginclamp->SetClampRegion(m_righttoolpartconvex,//.m_rigidbody,
				GFPhysVector3(0 , -0.05f , 0),
				GFPhysVector3(1 , 0 , 0),
				GFPhysVector3(0 , 0 , 1),
				0.18f,
				0.65f,
				MisCTool_PluginClamp::ClampReg_Right,
				1
				);

			m_pluginclamp->SetClampRegion(m_lefttoolpartconvex,//.m_rigidbody,
				GFPhysVector3(0 , 0.05f , 0),
				GFPhysVector3(1 , 0 , 0),
				GFPhysVector3(0 , 0 , 1),
				0.18f,
				0.65f,
				MisCTool_PluginClamp::ClampReg_Left,
				-1
				);

			m_ToolPlugins.push_back(m_pluginclamp);
		}
		
//		m_righttoolpartconvex.CreateDebugDrawable(MXOgre_SCENEMANAGER);

		m_Tex = Ogre::TextureManager::getSingleton().load("cogBrandBipolar.tga", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		
		SmokeManager * smokeMgr = EffectManager::Instance()->GetSmokeManager();
		if (smokeMgr)
		{
			smokeMgr->setInterval(0.8);
		}
		return true;
	}
	else
	{
		return true;
	}
}
//============================================================================================================
void CBipolarElecForceps::onFrameUpdateStarted(float timeelapsed)
{
	CTool::onFrameUpdateStarted(timeelapsed);
}
//============================================================================================================
bool CBipolarElecForceps::ElectricClampedFaces(float dt)
{
	bool isValidElectric = false;
	
	MisNewTraining * pNewTraining = dynamic_cast<MisNewTraining*>(CTrainingMgr::Get()->GetCurTraining());

	if (m_pluginclamp && m_pluginclamp->isInClampState())
	{
		m_pluginclamp->CollectPos();

	   std::vector<MisMedicOrgan_Ordinary *> organsBeClamped;
	   m_pluginclamp->GetOrgansBeClamped(organsBeClamped);
      
	   GFPhysVectorObj<GFPhysSoftBodyFace*> FacesInClamp;
	   std::vector<Ogre::Vector2> TextureCoords;
	   std::vector<Ogre::Vector2> TFUV;

	   std::vector<Ogre::Vector2> texCoordsForOriginFaces;
	   std::vector<Ogre::Vector2> TFUVForOriginFaces;

	   std::vector<Ogre::Vector2> texCoordsForCutFaces;
	   std::vector<Ogre::Vector2> TFUVForCutFaces;

	   bool hasCutFaceBurned = false;
       bool timeEnoughSinceLastUpdate = false;
	   if (m_counter > 15)//reset count
	   {
		   m_counter = 0;
		   timeEnoughSinceLastUpdate = true;
	   }
	   for(size_t c = 0 ; c < organsBeClamped.size() ; c++)
	   {
           MisMedicOrgan_Ordinary * organInClamp = organsBeClamped[c];
           
		   FacesInClamp.clear();
		   //TextureCoords.clear();
		   //TFUV.clear();
		   texCoordsForOriginFaces.clear();
		   TFUVForOriginFaces.clear();
		  
		   texCoordsForCutFaces.clear();
		   TFUVForCutFaces.clear();

		   m_pluginclamp->GetFacesBeClamped(FacesInClamp, organInClamp);

		   organInClamp->RemoveInjuryPointsOnFaces(FacesInClamp);

		   for (size_t f = 0; f < FacesInClamp.size(); f++)
		   {
			   GFPhysSoftBodyFace * face = FacesInClamp[f];

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

					   float U = 0;
					   float V = 0;
					   m_pluginclamp->CollectSubFaceInClampRegionUV(face, k, U, V, 1.0f);
					   Ogre::Vector2 UV = Ogre::Vector2(U, V);
					   texCoordsForOriginFaces.push_back(textureCoord);
					   TFUVForOriginFaces.push_back(UV);
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

			   if(!texCoordsForCutFaces.empty())
			       hasCutFaceBurned = true;
		   }
		   organInClamp->ToolElectricClampedFaces(this, texCoordsForOriginFaces, TFUVForOriginFaces, dt);
		  
		   if (timeEnoughSinceLastUpdate)
		   {
			   CAcessoriesCutTraining * cutTrain = dynamic_cast<CAcessoriesCutTraining*>(pNewTraining);
			   
			   CElectroCoagulationTrain * elecTrain = dynamic_cast<CElectroCoagulationTrain*>(pNewTraining);
			   
			   if (cutTrain)
			   {
				   cutTrain->receiveCheckPointList(MisNewTraining::OCPT_Burn, TextureCoords, TFUV , this , organInClamp);
			   }
			   else if(elecTrain)
			   {
				   elecTrain->receiveCheckPointList(MisNewTraining::OCPT_Burn_Origin_Face , texCoordsForOriginFaces, TFUVForOriginFaces , this , organInClamp);
			   }
			   else
			   {
				   pNewTraining->receiveCheckPointList(MisNewTraining::OCPT_Burn_Cut_Face , texCoordsForCutFaces , TFUVForCutFaces , this , organInClamp);
			   }
		   }
	   }

	   //logic process send event
	   if(hasCutFaceBurned)
	   {
		   if(!m_IsLastBurnCutFace)
		   {
			   m_BurnCutFaceTime = 0.f;
			   MxToolEvent * pEvent = CMXEventsDump::Instance()->CreateEventNew(MxEvent::MXET_BurnCutFaceBegin, this , NULL);
			   CMXEventsDump::Instance()->PushEvent(pEvent);
		   }
		   else
		   {
			   m_BurnCutFaceTime += dt;
		   }
		   m_IsLastBurnCutFace = true;
	   }
	   else
	   {
		   if(m_IsLastBurnCutFace)
		   {
			   m_BurnCutFaceTime += dt;

			   MxToolEvent * pEvent = CMXEventsDump::Instance()->CreateEventNew(MxEvent::MXET_BurnCutFaceEnd, this , NULL);

			   pEvent->m_UserData = int(m_BurnCutFaceTime * 1000);

			   CMXEventsDump::Instance()->PushEvent(pEvent);

			   m_BurnCutFaceTime = 0.f;

			   m_IsLastBurnCutFace = false;
		   }
	   }

	   //电凝起泡
	   GFPhysVector3 center , rightV , up ,smokeP;
	   m_pluginclamp->GetClampCoordSys(center, rightV, up);
	   smokeP = center + up * 0.3;
	   Ogre::Vector3 emitpoint = GPVec3ToOgre(smokeP);
		
	   //apply somke effect
	   SmokeManager * smokeMgr = EffectManager::Instance()->GetSmokeManager();
	   smokeMgr->addSmoke(emitpoint, 0.15, 0.1, 5);


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

			controlInfo.MinExplosionSize = 0.03f;
			controlInfo.MaxExplosionSize = 0.06f;

			controlInfo.ExpandRate = 0.075f;

			for(size_t f = 0;f < FacesInClamp.size();f++)
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
	return isValidElectric;
}
//============================================================================================================
bool CBipolarElecForceps::Update(float dt)
{
	__super::Update(dt);
	//electronic
	if(m_bElectricButton)
	{
		if (m_pluginclamp && m_pluginclamp->isInClampState())
		{

//			m_bubbleManager.inBurn();
			ElectricClampedFaces(dt);

			//被夹住的四面体因为加热会发生体积缩小
			for (int c = 0; c < (int)m_pluginclamp->m_ClampedOrgans.size(); c++)
			{
				MisCTool_PluginClamp::OrganBeClamped * organClamp = m_pluginclamp->m_ClampedOrgans[c];
				//if (organToShrink)
				//{
				const std::vector<GFPhysSoftBodyTetrahedron*> & tetraBeClamped = organClamp->m_TetrasInClampReg;

#if(0)

				organToShrink->BiteTetrasFromOrgan(tetraBeClamped);
#else
				float heatValue = organClamp->m_organ->GetCreateInfo().m_BurnRation*dt;

				organClamp->m_organ->HeatTetrahedrons(tetraBeClamped, heatValue);

				organClamp->m_organ->ShrinkTetrahedrons(tetraBeClamped, organClamp->m_ClampDirInMaterialSpace, organClamp->m_organ->m_BurnShrinkRate);
#endif
			    // }
		    }
		    m_counter++;
		    UpdateValidElectricTime(dt);
		}
		else 
		{
			m_counter = -1;
			//m_bubbleManager.stopBurn();
		}
		UpdateTotalElectricTime(dt);
		return true;
	}
	else
	{
		if (m_counter > 0)
		{
			m_counter--;
		}
		else
		{
			m_counter = 0;
		}
//		m_bubbleManager.stopBurn();

		if(m_IsLastBurnCutFace)
		{
			m_BurnCutFaceTime += dt;
			MxToolEvent * pEvent = CMXEventsDump::Instance()->CreateEventNew(MxEvent::MXET_BurnCutFaceEnd, this , NULL);
			pEvent->m_UserData = int(m_BurnCutFaceTime * 1000);
			CMXEventsDump::Instance()->PushEvent(pEvent);
			m_BurnCutFaceTime = 0.f;
			m_IsLastBurnCutFace = false;
		}
	}
	
	return true;
}
//================================================================================================================
int CBipolarElecForceps::TryBurnConnectStrips(VeinConnectObject * stripObj, float BurnRate, std::vector<Ogre::Vector3> & burnpos, std::vector<VeinConnectPair*> & burnPairs)
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
//================================================================================================================
Ogre::TexturePtr CBipolarElecForceps::GetToolBrandTexture()
{
	return m_Tex;
}