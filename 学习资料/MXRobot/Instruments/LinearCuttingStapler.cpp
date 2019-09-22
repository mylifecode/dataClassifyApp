#include "LinearCuttingStapler.h"
#include "XMLWrapperTool.h"
#include "XMLWrapperOrgan.h"
#include "BasicTraining.h"
#include "EffectManager.h"
#include "OgreMaterialManager.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreSceneManager.h"
#include "ResourceManager.h"

#include "MXEventsDump.h"
#include "MXEvent.h"
#include "Helper.h"
#include "InputSystem.h"
#include "MisCTool_PluginCut.h"
#include "MisMedicOrganOrdinary.h"
#include "Topology\GoPhysSoftBodyRestShapeModify.h"
#include "SmokeManager.h"
#include "MxSoundManager.h"

CLinearCuttingStapler::CLinearCuttingStapler()
{	
}

CLinearCuttingStapler::CLinearCuttingStapler(CXMLWrapperTool * pToolConfig) : CForceps(pToolConfig)
{
    m_canCut = false;
	m_ApplyClipTime = 0;
}

CLinearCuttingStapler::~CLinearCuttingStapler()
{
	MxSoundManager::GetInstance()->StopSound("../res/audio/LinearCuttingStaplerWorking.wav");
}

//================================================================================================================
Ogre::TexturePtr CLinearCuttingStapler::GetToolBrandTexture()
{
	return m_Tex;
}
std::string CLinearCuttingStapler::GetCollisionConfigEntryName()
{
	//Åö×²Ìå
	return "LinearCuttingStapler";
}
//================================================================================================================
bool CLinearCuttingStapler::Initialize(CXMLWrapperTraining * pTraining)
{
	bool succed = CTool::Initialize(pTraining);

	m_Tex = Ogre::TextureManager::getSingleton().load("cogBrandBipolar.tga", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	SetLeftShaftAsideScale(0.8);

	SetRightShaftAsideScale(0.0);

	if (m_pOwnerTraining->m_IsNewTrainMode)
	{
		m_CutBladeRight.m_LinPoints[0] = GFPhysVector3(0.0f, 0.35f, -1.1f);//positivie z means outward
		m_CutBladeRight.m_LinPoints[1] = GFPhysVector3(0.0f, 0.35f, -6.2f);
		m_CutBladeRight.m_CuttDirection = GFPhysVector3(0, -1.0f, 0);

		m_CutBladeLeft.m_LinPoints[0] = GFPhysVector3(-0.0f, -0.3f, 5.5f);
		m_CutBladeLeft.m_LinPoints[1] = GFPhysVector3(-0.0f, -0.3f, 0.0f);
		m_CutBladeLeft.m_CuttDirection = GFPhysVector3(0, 1.0f, 0);

		m_CanPunctureOgran = false;

		//Titanic Clip Plugin
		m_pluginclamp = new MisCTool_PluginClamp(this, 10.0f);
		m_pluginclamp->m_ShowClampRegion = false;
		m_pluginclamp->SetClampRegion(m_righttoolpartconvex,//.m_rigidbody,
			GFPhysVector3(0, 0.16f, -3.45),
			GFPhysVector3(1, 0, 0),
			GFPhysVector3(0, 0, 1),
			0.28f,//clamp width    
			2.3f,//3.1f,            
			MisCTool_PluginClamp::ClampReg_Right,
			1
			);

		m_pluginclamp->SetClampRegion(m_lefttoolpartconvex,//.m_rigidbody,
			GFPhysVector3(0, 0.12f, 3.3f),
			GFPhysVector3(1, 0, 0),
			GFPhysVector3(0, 0, 1),
			0.28f,//clamp width        
			2.3f,//3.8f,            
			MisCTool_PluginClamp::ClampReg_Left,
			-1
			);
		m_pluginclamp->m_minShaftRangeUpperValue = 3.0f;
		m_pluginclamp->m_minShaftRangeLowerValue = 2.0f;
		m_pluginclamp->m_CheckClampForAllOrgans = true;
		m_pluginclamp->m_CanClampLargeFace = false;

		m_ToolPlugins.push_back(m_pluginclamp);

		m_EndoGiaPlugin = new MisCTool_PluginEndoGia(this);
		m_ToolPlugins.push_back(m_EndoGiaPlugin);

		Ogre::SceneNode * leftNode = dynamic_cast<Ogre::SceneNode*>(GetLeftNode());
		Ogre::Node::ChildNodeIterator itorchild = leftNode->getChildIterator();

		while (itorchild.hasMoreElements())
		{
			Ogre::SceneNode * pNode = (Ogre::SceneNode *)itorchild.getNext();
			if (pNode->getName().find("fa") != Ogre::String::npos)
			{
				m_NodeFaMen = pNode;
				break;
			}
		}

		return true;
	}
	return succed;
}
void CLinearCuttingStapler::SetCutterCenterlineDirection(const GFPhysVector3& dir)
{
    m_CutterCenterlineDirection = dir;
}

bool CLinearCuttingStapler::Update(float dt)
{
	bool ret;
	int i;
    
	ret=__super::Update(dt);

	float shaft = GetShaftAside();
	if (shaft > 6.0f && m_pluginclamp)
		m_pluginclamp->SetIgnored(0);

	if (m_pluginclamp->isInClampState() && (m_bElectricRightPad || m_bElectricLeftPad))
    {
	   m_ApplyClipTime += dt;

	   float FaMenMoveDist = m_ApplyClipTime*2.5f;

	   if(m_ApplyClipTime > 1.5f)
	   {
		  ReleaseClampedOrgans();//release clamped face first

          CutClampedOrgans();
          
		  FaMenMoveDist = 0;
	   }

	   if(m_NodeFaMen)//update famen
	   {
		  Ogre::Vector3 nodePos = m_NodeFaMen->getPosition();
		  m_NodeFaMen->setPosition(nodePos.x , nodePos.y , FaMenMoveDist);
	   }
       MxSoundManager::GetInstance()->Play("../res/audio/LinearCuttingStaplerWorking.wav",false);
    }
    else
    {        
        MxSoundManager::GetInstance()->StopSound("../res/audio/LinearCuttingStaplerWorking.wav");
		m_ApplyClipTime = 0;
    }

	return true;
}

bool CLinearCuttingStapler::CutClampedOrgans()
{
#if(1)
	GFPhysHashMap<GFPhysHashNode , GFPhysSoftBodyNode*> NodesAdded;

	for (int c = 0; c < (int)m_pluginclamp->m_ClampedOrgans.size(); c++)
	{
		MisMedicOrgan_Ordinary * OrganToCut = m_pluginclamp->m_ClampedOrgans[c]->m_organ;
		
		if (OrganToCut->CanBeCut() == false)
		    continue;

		//@ step 1 Cut
		OrganToCut->CutOrganByTool(this);
		
		if (OrganToCut->GetCreateInfo().m_IsMensentary == true)
		    continue;//mensentary not apply clip

		//@ setp 2 compress
		NodesAdded.clear();
		m_NodesToBeCompressed.clear();
		m_FacesCreatedUpper.clear();
		m_FacesCreatedLower.clear();
		m_NodesUpper.clear();
		m_NodesLower.clear();

		GFPhysVector3 clampNormalR = m_pluginclamp->GetClampRegNormal(0);
		GFPhysVector3 clampNormalL = m_pluginclamp->GetClampRegNormal(1);

		clampNormalR = m_pluginclamp->m_ClampedOrgans[c]->m_ClampDirInMaterialSpace;
		clampNormalL = -1 * clampNormalR;

		std::set<GFPhysSoftBodyFace*> & FacesCreated = OrganToCut->m_CutCrossFacesInLastCut;
		
		std::set<GFPhysSoftBodyFace*>::const_iterator pos = FacesCreated.begin();
		
		while(pos != FacesCreated.end())
		{
			GFPhysSoftBodyFace * physFace = (*pos);
			
			for (int i = 0; i < 3; i++)
			{
				if (NodesAdded.find(GFPhysHashNode(physFace->m_Nodes[i])) == 0)
				{
					NodesAdded.insert(GFPhysHashNode(physFace->m_Nodes[i]),physFace->m_Nodes[i]);
					m_NodesToBeCompressed.push_back(physFace->m_Nodes[i]);
				}
			}        
			pos++;
		}
		//Ogre::LogManager::getSingleton().logMessage(Ogre::String("the num of NodesToBeCompressed is  ")+Ogre::StringConverter::toString(m_NodesToBeCompressed.size())+".");
	    
		//////////////////////////////////////////////////////////////////////////////////////      
		
		std::set<GFPhysSoftBodyFace*>::const_iterator facepos = FacesCreated.begin();
		while(facepos != FacesCreated.end()) 
		{
			GFPhysSoftBodyFace * face = (*facepos);
			Real dis0 = 0.0f;
			Real dis1 = 0.0f;
			Real dis2 = 0.0f;

			if (true == GPDistanceFromPointToSurface(face->m_Nodes[0]->m_CurrPosition, m_CutBladeRight.m_LinePointsWorld[0], m_CutBladeRight.m_LinePointsWorld[1],m_CutBladeLeft.m_LinePointsWorld[0], dis0)
			  &&true == GPDistanceFromPointToSurface(face->m_Nodes[1]->m_CurrPosition, m_CutBladeRight.m_LinePointsWorld[0], m_CutBladeRight.m_LinePointsWorld[1],m_CutBladeLeft.m_LinePointsWorld[0], dis1)
			  &&true == GPDistanceFromPointToSurface(face->m_Nodes[2]->m_CurrPosition, m_CutBladeRight.m_LinePointsWorld[0], m_CutBladeRight.m_LinePointsWorld[1],m_CutBladeLeft.m_LinePointsWorld[0], dis2))
			{
				if (dis0 > 0.0f && dis1 > 0.0f && dis2 > 0.0f)
				{
					m_FacesCreatedUpper.insert(face);                
				}
				else if (dis0 < 0.0f && dis1 < 0.0f && dis2 < 0.0f)
				{
					m_FacesCreatedLower.insert(face);
				}
			}
			facepos++;
		}
	    //////////////////////////////////////////////////////////////////////////

		
		GFPhysVectorObj<GFPhysSoftBodyNode *>::const_iterator iter = m_NodesToBeCompressed.begin(); 
		while(iter != m_NodesToBeCompressed.end())  
		{
			GFPhysSoftBodyNode * node = (*iter);
			Real dis;
			if (true == GPDistanceFromPointToSurface(node->m_CurrPosition, m_CutBladeRight.m_LinePointsWorld[0], m_CutBladeRight.m_LinePointsWorld[1],m_CutBladeLeft.m_LinePointsWorld[0], dis))
			{
				if (dis > 0.0f)
				{
					m_NodesUpper.push_back(node);                
				}
				else
				{
					m_NodesLower.push_back(node);
				}
			}
			iter++;
		}

		GFPhysVector3 localdirUp,localdirLow;

		if (false == CalcLocalCompressDir(m_NodesUpper, m_pluginclamp->m_ClampedOrgans[c]->m_ClampDirInMaterialSpace, localdirUp))
		{
			continue;
		}
		if (false == CalcLocalCompressDir(m_NodesLower, m_pluginclamp->m_ClampedOrgans[c]->m_ClampDirInMaterialSpace, localdirLow))
		{
			continue;
		}

		GFPhysVectorObj<CompressNodesAndDir*> pair;
		struct CompressNodesAndDir subpair1 =  {m_NodesUpper,localdirUp};
		struct CompressNodesAndDir subpair2 =  {m_NodesLower,localdirLow};
		pair.push_back(&subpair1);
		pair.push_back(&subpair2);
		m_EndoGiaPlugin->TryApplyEndoGia(OrganToCut , pair,  0.5f);  

	}
#endif

	MisMedicOrgan_Ordinary *organs[10];
	for (int i = 0; i < 10; i++) organs[i] = 0;
	for (int c = 0; c < (int)m_pluginclamp->m_ClampedOrgans.size(); c++) organs[c] = m_pluginclamp->m_ClampedOrgans[c]->m_organ;

	m_EndoGiaPlugin->tryApplyClips(organs);

	return true;
}

bool CLinearCuttingStapler::SetNailsState(bool IsShow)
{
    int nIndex = OgreMax::OgreMaxScene::s_nLoadCount;
    char szIndex[256];
    sprintf_s(szIndex, "%d", nIndex);

    Ogre::SceneManager * pSceneManager = MXOgreWrapper::Get()->GetDefaultSceneManger();

    Ogre::String & strFather = Ogre::String("Linear_cutter_B_group") + "$" + szIndex;
    Ogre::SceneNode * pNodeFather = (Ogre::SceneNode *)pSceneManager->getSceneNode(strFather);
    if (pNodeFather)
    {
        Ogre::SceneNode::ChildNodeIterator iterChild = pNodeFather->getChildIterator();
        while ( iterChild.hasMoreElements() )
        {
            Ogre::SceneNode * pNodeFind = (Ogre::SceneNode*)iterChild.getNext();
            if (pNodeFind)
            {
                const std::string StrNodeFind = pNodeFind->getName();
                if (-1 != StrNodeFind.find("nail") )
                {
                    pNodeFind->setVisible(IsShow);
                }
            }

        }

    }	
    return true;
}

void CLinearCuttingStapler::InternalSimulationStart(int currStep , int TotalStep , float dt)
{
    CTool::InternalSimulationStart( currStep ,  TotalStep ,  dt);
}

float CLinearCuttingStapler::GetCutWidth()
{
    return 0.1f;
}

void CLinearCuttingStapler::onFrameUpdateStarted(float timeelapsed)
{
    CTool::onFrameUpdateStarted(timeelapsed);
    //m_time_elapse = timeelapsed;
}

GFPhysVector3 CLinearCuttingStapler::CalculateToolCustomForceFeedBack()
{
    //return GFPhysVector3(0,0,0);
    //return CTool::CalculateToolCustomForceFeedBack();

    //Total Drag force
    GFPhysVector3 TotalDragForce(0,0,0);

    for(size_t p = 0 ; p < m_ToolPlugins.size() ; p++)
    {
        MisCTool_PluginClamp * clampPlugin = dynamic_cast<MisCTool_PluginClamp*>(m_ToolPlugins[p]);

        if(clampPlugin)
        {
			TotalDragForce += clampPlugin->GetPluginForceFeedBack()*0.01f;
        }
    }

    return TotalDragForce;
}


bool CLinearCuttingStapler::CalcLocalCompressDir(const GFPhysVectorObj<GFPhysSoftBodyNode *>& nodesToBeCompressed,const GFPhysVector3 & anthorchoice,GFPhysVector3 & Localdir)
{
    GFPhysAlignedVectorObj<GFPhysVector3> positions;
    GFPhysVectorObj<GFPhysSoftBodyNode*>::const_iterator itor = nodesToBeCompressed.begin();
    while(itor != nodesToBeCompressed.end())
    {
        GFPhysSoftBodyNode * node = (*itor);
        positions.push_back(node->m_UnDeformedPos);
        itor++;
    }

    GFPhysVector3 localnormal , com;
    //if (false == CalcPlaneNormalByRegress(positions,localnormal))
    //{
    //    return false;   
    //}
    if (false == CalcPlaneNormalBySVD(positions,localnormal,com))
    {
        return false;   
    }

    
    localnormal.Normalize();
    
    GFPhysVector3 anthorchoiceproject = anthorchoice -  (anthorchoice.Dot(localnormal)) * localnormal;
    anthorchoiceproject.Normalize();
    Localdir = anthorchoiceproject;
    return true;
}



