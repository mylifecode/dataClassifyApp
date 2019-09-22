#include "ClipApplier.h"
#include "XMLWrapperTool.h"
#include "MXOgreWrapper.h"
#include "XMLWrapperOrgan.h"
#include "XMLWrapperTraining.h"
#include "ITraining.h"
#include "ResourceManager.h"
#include "EffectManager.h"

#include "MXEventsDump.h"
#include "MXEvent.h"
#include "Helper.h"
#include "InputSystem.h"
#include "XMLWrapperPart.h"

#include "ToolsMgr.h"
#include "MisCTool_PluginClipTitanic.h"
#include "MisNewTraining.h"
#include "MisCTool_PluginClamp.h"

CClipApplier::CClipApplier() : CForceps()
,m_fHalfHeadMoveDistance( 0.18f )
,m_nStatus(CLIPAPLLIER_COMPLETELY_OPEN_STATUS)

{
	m_NewCanClampTube = true;
}

CClipApplier::CClipApplier(CXMLWrapperTool * pToolConfig) : CForceps(pToolConfig)
,m_fHalfHeadMoveDistance( 0.18f )
,m_nStatus(CLIPAPLLIER_COMPLETELY_OPEN_STATUS)
{
	m_fClipAccumTime=0;
	m_fClipLastTickCount=0;
	m_bClipClamp=false;
	m_bCheckClipClamp=false;

	m_nShaftAside = pToolConfig->m_MaxShaftAside;
	m_NewCanClampTube = true;
	m_TitanicPlugin = 0;
}

CClipApplier::~CClipApplier()
{			
	ITraining * pTraining = GetOwnerTraining();
	if(pTraining)
	{
		for ( size_t i = 0; i < m_pEmptyClips.size(); ++i )
		{
			m_pEmptyClips[i]->GetKernelNode()->setPosition( m_vDirection*100 );
			if (m_pEmptyClips[i])
			{
				pTraining->m_pToolsMgr->RemoveTool( m_pEmptyClips[i]->GetName() );
			}
		}
	}
}

std::string CClipApplier::GetCollisionConfigEntryName()
{
	return "Clip applicator";
}

bool CClipApplier::Initialize(CXMLWrapperTraining * pTraining)
{
	__super::Initialize(pTraining);

	//test
	Ogre::SceneManager * pSceneManager = MXOgreWrapper::Instance()->GetDefaultSceneManger();

	/*
	Ogre::Vector3 centeroffset = Ogre::Vector3(-0.0585f , 0.35f , -0.041f);
	Ogre::Vector3 boxextends =  Ogre::Vector3(0.112f , 0.131f , 1.112f);
	m_righttoolpartconvex.CreateBoxConvex(centeroffset , boxextends);
	m_righttoolpartconvex.m_contactFaceNormal = Ogre::Vector3(0 , -1.0f , 0);
	m_righttoolpartconvex.CreateDebugDrawable(pSceneManager);
	

	//shit!!!!left node in the 3dmax isn't zero rotate so we should rotate back around z 90 degree
	//correct in 3dmax to reset zero rotation!
	centeroffset = Ogre::Vector3(0.35f , -0.0585f , -0.041f);
	boxextends =  Ogre::Vector3(0.131f , 0.112f , 1.112f);
	m_lefttoolpartconvex.CreateBoxConvex(centeroffset , boxextends);
	m_lefttoolpartconvex.m_contactFaceNormal = Ogre::Vector3(0 , 1.0f , 0);
	m_lefttoolpartconvex.CreateDebugDrawable(pSceneManager);
	

	centeroffset = Ogre::Vector3(-0.0585f , 0 , -1.2f);
	boxextends =  Ogre::Vector3(0.112f , 0.21f , 1.25f);
	m_centertoolpartconvex.CreateBoxConvex(centeroffset , boxextends);
	m_centertoolpartconvex.CreateDebugDrawable(pSceneManager);*/
	
	m_CutBladeRight.m_LinPoints[0] = GFPhysVector3(0, 0.0f, 0.6f);
	m_CutBladeRight.m_LinPoints[1] = GFPhysVector3(0, 0.0f, 0.0f);
	m_CutBladeRight.m_CuttDirection = GFPhysVector3(0 , -1.0f , 0);
	
	m_CutBladeLeft.m_LinPoints[0] = GFPhysVector3(0.0f , 0 , 0.6f);
	m_CutBladeLeft.m_LinPoints[1] = GFPhysVector3(0.0f , 0 , 0.0f);
	m_CutBladeLeft.m_CuttDirection = GFPhysVector3(-1.0f , 0 , 0);
	
	//Titanic Clip Plugin
	m_pluginclamp = new MisCTool_PluginClamp(this, 3.0f);
	m_pluginclamp->SetClampRegion(m_righttoolpartconvex,//.m_rigidbody,
								GFPhysVector3(0 , -0.1f , 0),
								GFPhysVector3(1 , 0 , 0),
								GFPhysVector3(0 , 0 , 1),
								0.04f,
								1.0f,
								MisCTool_PluginClamp::ClampReg_Right,
								1
								);

	m_pluginclamp->SetClampRegion(m_lefttoolpartconvex,//.m_rigidbody,
								GFPhysVector3(-0.1f , 0 , 0),
								GFPhysVector3(0 , 1 , 0),
								GFPhysVector3(0 , 0 , 1),
								0.04f,
								1.0f,
								MisCTool_PluginClamp::ClampReg_Left,
								-1
								);
	m_pluginclamp->m_minShaftRangeUpperValue = 0.5f;
	m_pluginclamp->m_minShaftRangeLowerValue = 0.5f;
	m_ToolPlugins.push_back(m_pluginclamp);
	
	m_TitanicPlugin = new MisCTool_PluginClipTitanic(this);
	m_ToolPlugins.push_back(m_TitanicPlugin);


	if(m_pOwnerTraining->m_IsNewTrainMode)
	   return true;

	m_bClampBeforeRealse = true;

	return true;
}

void CClipApplier::CreateEmptyClip()
{
	ITool * pClip = CResourceManager::Instance()->GetOneTool(TT_HEMOCLIP, false, "");
		
	if ( pClip != NULL )
	{
		pClip->SetOwnerTraining( GetOwnerTraining() );
		pClip->Initialize( this->GetTrainingConfig() );

		MisNewTraining * currTrain = dynamic_cast<MisNewTraining*>(GetOwnerTraining());
		if(currTrain)
		{
		   m_vDirection = currTrain->GetSceneGravityDir().normalisedCopy() * 0.5f;
		}
		
		Ogre::Vector3 v3Pos = GetKernelNode()->_getDerivedPosition();		
		Ogre::Quaternion quat = GetKernelNode()->_getDerivedOrientation();
		Ogre::Vector3 v3Scale = GetKernelNode()->_getDerivedScale();
		Ogre::SceneNode * pHookClipNode = pClip->GetKernelNode();

		pHookClipNode->setPosition( v3Pos );
		pHookClipNode->setOrientation( quat );			
		pHookClipNode->setScale( v3Scale );
	}
	m_pEmptyClips.push_back(pClip);
}
bool CClipApplier::Update(float dt)
{
	__super::Update(dt);	

	for ( size_t i = 0; i < m_pEmptyClips.size(); ++i )
	{
		Ogre::SceneNode * pHookClipNode = m_pEmptyClips[i]->GetKernelNode();
		pHookClipNode->setPosition( pHookClipNode->getPosition() + m_vDirection);
		if ( pHookClipNode->getPosition().z < 0 )
		{
			m_pEmptyClips[i]->GetKernelNode()->setPosition( m_vDirection*100);
			GetOwnerTraining()->m_pToolsMgr->RemoveTool( m_pEmptyClips[i]->GetName() );
			m_pEmptyClips.erase( m_pEmptyClips.begin() + i );
			--i;
		}
		else if (pHookClipNode->getPosition().y < 0)
		{
			m_pEmptyClips[i]->GetKernelNode()->setPosition( m_vDirection*100);
			GetOwnerTraining()->m_pToolsMgr->RemoveTool( m_pEmptyClips[i]->GetName() );
			m_pEmptyClips.erase( m_pEmptyClips.begin() + i );
			--i;
		}
	}
	if (m_nShaftAside == m_nMaxShaftAside)
	{
		m_nStatus = CLIPAPLLIER_COMPLETELY_OPEN_STATUS;
	}
	return true;

}

void CClipApplier::Updates2m()
{
	
}

//==============================================================================================================
void CClipApplier::InternalSimulationStart(int currStep , int TotalStep , float dt)
{
	CTool::InternalSimulationStart( currStep ,  TotalStep ,  dt);
	if (m_pluginclamp->isInClampState() == true)
	{
		//SetMinShaftAside(0.5f);
		//std::vector<MisMedicOrgan_Ordinary *> organsClamped;
		
		//m_ClampPlugin->GetOrgansBeClamped(organsClamped);

		//if(organsClamped.size() > 0)//temply choose the first object be clamped
		//{
		m_TitanicPlugin->m_AppAllowClamp = true;
			
		if(GetShaftAside() <= 1.0f)
		{
			for (int c = 0; c < /*(int)m_ClampPlugin->m_ClampedOrgans.size()*/1; c++)
			{
				std::vector<GFPhysSoftBodyFace*> faceInRegL;

				std::vector<GFPhysSoftBodyFace*> faceInRegR;

				MisCTool_PluginClamp::OrganBeClamped * organClamped = m_pluginclamp->m_ClampedOrgans[c];

				for (size_t c = 0; c < organClamped->m_ClampedFaces.size(); c++)
				{
					if (organClamped->m_ClampedFaces[c].m_Part == 0)
						faceInRegR.push_back(organClamped->m_ClampedFaces[c].m_PhysFace);

					else if (organClamped->m_ClampedFaces[c].m_Part == 1)
						faceInRegL.push_back(organClamped->m_ClampedFaces[c].m_PhysFace);
				}

				m_TitanicPlugin->CustomClipOrgan(organClamped->m_organ, faceInRegL, faceInRegR, organClamped->m_TetrasInClampReg, organClamped->m_ClampDirInMaterialSpace);
			}
		}
		//}
	}
	else
	{
		m_TitanicPlugin->m_AppAllowClamp = false;
	}
}


//夹子夹住物体时是否与物体垂直
bool CClipApplier::IsClampedVertical()
{
	return dynamic_cast<MisCTool_PluginClipTitanic*>(m_ToolPlugins[0])->m_bIsClampedVertical;
}

//是否下半部分可见
bool CClipApplier::IsLowerHalfPartVisiable()
{
	return true;//dynamic_cast<MisCTool_PluginClipTitanic*>(m_ToolPlugins[0])->m_bLowerHalfPartIsVisiable;
}