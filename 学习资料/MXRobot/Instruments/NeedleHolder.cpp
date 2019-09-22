#include "NeedleHolder.h"
#include "XMLWrapperTool.h"
#include "XMLWrapperOrgan.h"
#include "XMLWrapperTraining.h"
#include "MXOgreWrapper.h"
#include <OgreManualObject.h>
#include "MXEvent.h"
#include "MXEventsDump.h"
#include "Helper.h"
#include "Instruments/MisCTool_PluginClamp.h"
#include "Instruments/MisCTool_PluginRigidHold.h"
#include "MisToolCollideDataConfig.h"
using namespace::std;


CNeedleHolder::CNeedleHolder() : CTool()
,m_nSuturesId(-1)
,m_nNeedleId(-1)
,m_pNeedleNode(NULL)
,m_pluginRigidHold(0)
{
	
}

CNeedleHolder::CNeedleHolder(CXMLWrapperTool * pToolConfig) : CTool(pToolConfig)
,m_nSuturesId(-1)
,m_nNeedleId(-1)
,m_pNeedleNode(NULL)
,m_pluginRigidHold(0)
{
	
}

CNeedleHolder::~CNeedleHolder()
{

}

std::string CNeedleHolder::GetCollisionConfigEntryName()
{
	//创建抓钳的碰撞体
	return "Needle Holder";
}
//========================================================================================================
bool CNeedleHolder::Initialize(CXMLWrapperTraining * pTraining)
{
	bool succed = CTool::Initialize(pTraining);

	SetLeftShaftAsideScale(0.0);

	//三角形顶点必须按逆时针顺序排列
	Ogre::Vector2 tri_vertices_right[6] = { Ogre::Vector2(-0.04f,1.25f),
											Ogre::Vector2(-0.14f,0),
											Ogre::Vector2(0.14f,0) ,
											Ogre::Vector2(-0.04f,1.25f),
											Ogre::Vector2(0.14f,0),
											Ogre::Vector2(0.04f,1.25f)
	};
	Ogre::Vector2 tri_vertices_left[6] = {Ogre::Vector2(-0.04f , -0.05f),
										  Ogre::Vector2(-0.14f , -1.3f),
										  Ogre::Vector2(0.14f , -1.3f) ,
										  Ogre::Vector2(-0.04f , -0.05f),
										  Ogre::Vector2(0.14f , -1.3f),
										  Ogre::Vector2(0.04f , -0.05f)
	};

	m_pluginclamp = new MisCTool_PluginClamp(this, 3.0f);
	m_pluginclamp->SetClampRegion(m_righttoolpartconvex,//.m_rigidbody,
								  GFPhysVector3(0 , 0 , 0),
								  GFPhysVector3(1 , 0 , 0),
								  GFPhysVector3(0 , 0 , 1),
								  tri_vertices_right,
								  6,
								  MisCTool_PluginClamp::ClampReg_Right,
								  1.0f
								 );

	m_pluginclamp->SetClampRegion(m_lefttoolpartconvex,//.m_rigidbody,
								  GFPhysVector3(0 , 0 , 0),
								  GFPhysVector3(1 , 0 , 0),
								  GFPhysVector3(0 , 0 , 1),
								  tri_vertices_left,
								  6,
								  MisCTool_PluginClamp::ClampReg_Left,
								  -1.0f
								  ); 
	
	m_pluginclamp->m_ShowClampRegion = false;
	m_ToolPlugins.push_back(m_pluginclamp);

    m_pluginRigidHold = new MisCTool_PluginRigidHold(this); 

	m_pluginRigidHold->SetHoldRegion(m_righttoolpartconvex,//.m_rigidbody,
		GFPhysVector3(0, 0, 0),
		GFPhysVector3(1, 0, 0),
		GFPhysVector3(0, 0, 1),
		tri_vertices_right,
		6,
		MisCTool_PluginRigidHold::HoldReg_Right,
		1.0f
        );

	m_pluginRigidHold->SetHoldRegion(m_lefttoolpartconvex,//.m_rigidbody,
		GFPhysVector3(0, 0, 0),
		GFPhysVector3(1, 0, 0),
		GFPhysVector3(0, 0, 1),
		tri_vertices_left,
		6,
		MisCTool_PluginRigidHold::HoldReg_Left,
		-1.0f
        ); 

    m_ToolPlugins.push_back(m_pluginRigidHold);


    m_leftTempObj = static_cast<GFPhysCollideObject*>(m_lefttoolpartconvex.m_rigidbody);
    m_leftOriginShape = m_leftTempObj->GetCollisionShape();

    m_rightTempObj = static_cast<GFPhysCollideObject*>(m_righttoolpartconvex.m_rigidbody);
    m_rightOriginShape = m_rightTempObj->GetCollisionShape();




	return succed;
}
//=====================================================================================================
GFPhysVector3 CNeedleHolder::CalculateToolCustomForceFeedBack()
{
	//Total Drag force
	GFPhysVector3 TotalDragForce(0, 0, 0);

	for (size_t p = 0; p < m_ToolPlugins.size(); p++)
	{
		MisMedicCToolPluginInterface * clampPlugin = m_ToolPlugins[p];// dynamic_cast<MisCTool_PluginClamp*>(m_ToolPlugins[p]);

		if (clampPlugin)
		{
			TotalDragForce += clampPlugin->GetPluginForceFeedBack()*0.1f;
		}
	}

	return TotalDragForce;
}
//=====================================================================================================
bool CNeedleHolder::Update(float dt)
{
	__super::Update(dt);	
	return true;
}
//=====================================================================================================
void CNeedleHolder::UpdateHeldPoints()
{

}
