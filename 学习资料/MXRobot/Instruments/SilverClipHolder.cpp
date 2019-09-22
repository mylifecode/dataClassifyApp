#include "SilverClipHolder.h"
#include "XMLWrapperTool.h"
#include "MXOgreWrapper.h"
#include "XMLWrapperOrgan.h"
#include "XMLWrapperTraining.h"
#include "ITraining.h"
#include "ResourceManager.h"
#include "EffectManager.h"
#include "BasicTraining.h"
#include "InputSystem.h"
#include "../NewTrain/MisToolCollideDataConfig.h"
#include "MisCTool_PluginSilverClip.h"
#include "MisCTool_PluginClamp.h"


CSilverClipHolder::CSilverClipHolder()
{

}

CSilverClipHolder::CSilverClipHolder(CXMLWrapperTool * pToolConfig) : CForceps(pToolConfig)
{
	m_leftShaftAsideScale = 0.0f;
}

CSilverClipHolder::~CSilverClipHolder()
{

}

std::string CSilverClipHolder::GetCollisionConfigEntryName()
{
	//collision part
	return "Y-Clip applier";

}

bool CSilverClipHolder::Initialize(CXMLWrapperTraining * pTraining)
{
	bool succed = CForceps::Initialize(pTraining);
	
	GFPhysVector3 center = OgreToGPVec3(m_lefttoolpartconvex.m_CollideShapesData[0].m_boxcenter);
	GFPhysQuaternion rotQuat = OgreToGPQuaternion(m_lefttoolpartconvex.m_CollideShapesData[0].m_boxrotate);
	GFPhysMatrix3 rotMat;
	rotMat.SetRotation(rotQuat);

	m_pluginclamp = new MisCTool_PluginClamp(this, 3.0f);
	m_pluginclamp->SetClampRegion(m_righttoolpartconvex,//.m_rigidbody,
								  GFPhysVector3(0 , -0.08f , 0.0f),
								  GFPhysVector3(1 , 0 , 0),
								  GFPhysVector3(0 , 0 , 1),
								  0.12f,
								  0.3f,
								  MisCTool_PluginClamp::ClampReg_Right,
								  1
								  );

	m_pluginclamp->SetClampRegion(m_lefttoolpartconvex,//.m_rigidbody,
		                          GFPhysVector3(0 , -0.24f , -0.54f),//(rotMat.Inverse() * (GFPhysVector3(0 , -0.24f , -0.58f)-center)),//GFPhysVector3(0 , -0.24f , -0.58f),
		                          GFPhysVector3(1 , 0 , 0),//(rotMat.Inverse() * GFPhysVector3(1 , 0 , 0)),
		                          GFPhysVector3(0 , 0 , 1),//(rotMat.Inverse() * GFPhysVector3(0 , 0 , 1)),
		                          0.15f,
		                          0.3f,
		                          MisCTool_PluginClamp::ClampReg_Left,
		                          -1
		                          );

	m_ToolPlugins.push_back(m_pluginclamp);
	m_pluginclamp->m_ShowClampRegion = false;

	m_SilverPlugin = new MisCTool_PluginSilverClip(this);
	m_ToolPlugins.push_back(m_SilverPlugin);

	m_CutBladeRight.m_LinPoints[0]  = GFPhysVector3(0 , -0.08f , 0.3f);
	m_CutBladeRight.m_LinPoints[1]  = GFPhysVector3(0 , -0.08f , -0.3f);
	m_CutBladeRight.m_CuttDirection = GFPhysVector3(0 , -1.0f , 0);


	

	m_CutBladeLeft.m_LinPoints[0]  = (rotMat.Inverse() * (GFPhysVector3(0 , -0.24f , -0.28f)-center));//GFPhysVector3(0 , -0.24f , -0.28f);
	m_CutBladeLeft.m_LinPoints[1]  = (rotMat.Inverse() * (GFPhysVector3(0 , -0.24f , -0.88f)-center));//GFPhysVector3(0 , -0.24f , -0.88f);
	m_CutBladeLeft.m_CuttDirection = (rotMat.Inverse() * (GFPhysVector3(0.0f , 1.0f , 0)));//GFPhysVector3(0.0f , 1.0f , 0);

	return succed;
}

//==============================================================================================================
void CSilverClipHolder::InternalSimulationStart(int currStep , int TotalStep , float dt)
{
	CTool::InternalSimulationStart( currStep ,  TotalStep ,  dt);
	
	if (m_pluginclamp->isInClampState() == true)
	{
		SetMinShaftAside(1.0f);
		
		if(GetShaftAside() <= 2.0f)// && organsClamped.size() > 0)
		{
			for (int c = 0; c < 1/*(int)m_ClampPlugin->m_ClampedOrgans.size()*/; c++)
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
				GFPhysVector3 clampNormalR = m_pluginclamp->GetClampRegNormal(0);
				GFPhysVector3 clampNormalL = m_pluginclamp->GetClampRegNormal(1);
				m_SilverPlugin->TryApplySilverClip(organClamped->m_organ, faceInRegL, faceInRegR, organClamped->m_TetrasInClampReg, organClamped->m_ClampDirInMaterialSpace);
			}
		}
	}
}

//========================================================================================
void CSilverClipHolder::CreateClip()
{
	ITool * pSilverClip = CResourceManager::Instance()->GetOneTool(TT_SILVERCLIP, false, ""); 
	
	// avoid crashing when hemoclip delete config file. totoro
	pSilverClip->SetTrainingConfig(this->GetTrainingConfig());

	if (pSilverClip)
	{		
		// set clip position and orientation according to silver clip holder
		Ogre::SceneNode * pKernelNode = GetKernelNode();
		Ogre::Vector3 ToolWorldPos = pKernelNode->_getDerivedPosition();		
		Ogre::Quaternion ToolWorldOrient = pKernelNode->_getDerivedOrientation();
		Ogre::Vector3 ToolWorlScale = pKernelNode->_getDerivedScale();
		Ogre::SceneNode * pSilverClipNode = pSilverClip->GetKernelNode();

		if (pSilverClipNode)
		{
			Ogre::Vector3 ClipLocalPos = pSilverClipNode->getPosition();//0 , -0.161f ,-1.696f);
			Ogre::Vector3 ClipWorldPos = ToolWorldPos + ToolWorldOrient * ClipLocalPos;
			pSilverClipNode->setPosition(ClipWorldPos);
			pSilverClipNode->setOrientation(ToolWorldOrient);
			pSilverClipNode->setScale(ToolWorlScale);
		}
	}
}
//========================================================================================
void CSilverClipHolder::onFrameUpdateStarted(float timeelpased)
{
	CTool::onFrameUpdateStarted(timeelpased); 

	//使银夹钳上的推子只在z轴方向上发生平移
	Ogre::Vector3 v3Normal = Ogre::Vector3::UNIT_X;
	Ogre::Quaternion quatRight(Ogre::Radian(Ogre::Degree(m_nShaftAside)*(-1.5)), Ogre::Vector3::UNIT_X);
	Ogre::Quaternion quatReset(Ogre::Radian(Ogre::Degree(0)), Ogre::Vector3::UNIT_X);
	Ogre::Node::ChildNodeIterator iter = GetRightNode()->getChildIterator();
	GetRightNode()->setOrientation(quatReset);
	while (iter.hasMoreElements())
	{
		std::string name = iter.getNext()->getName();
		if (name.find("applier01-2")!= string::npos)
		{
			Ogre::SceneNode* pClip = dynamic_cast<Ogre::SceneNode*>(GetRightNode()->getChild(name));
			if (pClip)
			{
				pClip->setOrientation(quatRight);					//银夹上部分夹片绕X轴旋转
			}
		}
		else if (name.find("applier03")!= string::npos)
		{
			float radio = m_nShaftAside/GetMaxShaftAside();
			Ogre::SceneNode* pPusher = dynamic_cast<Ogre::SceneNode*>(GetRightNode()->getChild(name));
			if (pPusher)
			{
				pPusher->setPosition(0.0f,0.0f,-0.3f*radio-0.05);		//银夹推子沿Z轴平移
			}
		}
	}
	
}
