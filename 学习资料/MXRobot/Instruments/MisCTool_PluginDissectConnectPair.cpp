#include "MisCTool_PluginDissectConnectPair.h"
#include "Tool.h"
#include "VeinConnectObject.h"
#include "CustomCollision.h"
#include "MXOgreGraphic.h"

#define DISSECT_SHAFT 15.f

MisCTool_PluginDissectConnectPair::ToolRegion::ToolRegion()
: m_AttachRigid(NULL)
{

}

void MisCTool_PluginDissectConnectPair::ToolRegion::UpdateToWorldSpace()
{
	const GFPhysTransform & worldtrans = m_AttachRigid->GetWorldTransform();
	const GFPhysMatrix3 & worldRotate = worldtrans.GetBasis();

	//m_Axis0World  = worldRotate*m_Axis0Local;
	//m_Axis1World  = worldRotate*m_Axis1Local;

	m_CenterWorld = worldtrans*m_CenterLocal;
	m_OutNormalWorld = worldRotate * m_OutNormalLocal;
}

MisCTool_PluginDissectConnectPair::MisCTool_PluginDissectConnectPair(CTool * tool , float shaftDifference)
: MisMedicCToolPluginInterface(tool) , 
m_ShaftDifferenceCanDissect(shaftDifference)
{
	//debug
// 	m_pManual = MXOgreWrapper::Get()->GetDefaultSceneManger()->createManualObject();
// 	MXOgreWrapper::Get()->GetDefaultSceneManger()->getRootSceneNode()->attachObject(m_pManual);
// 	m_pManual->setDynamic(true);
}

MisCTool_PluginDissectConnectPair::~MisCTool_PluginDissectConnectPair()
{
	
}

void MisCTool_PluginDissectConnectPair::SetToolRegion(GFPhysRigidBody * attachRigid, 
													const GFPhysVector3 & center, 
													const GFPhysVector3 & axis0, 
													const GFPhysVector3 & axis1, 
													int RegionIndex, Real normalSign){}

void MisCTool_PluginDissectConnectPair::SetToolRegion(GFPhysRigidBody * attachRigid, 
													  const GFPhysVector3 & centerLocal, 
													  const GFPhysVector3 & OutNormalLocal, 
													  const float thickness, 
													  int RegionIndex)
{
	ToolRegion & toolRegion = m_ToolRegion[RegionIndex];
	toolRegion.m_AttachRigid = attachRigid;
	toolRegion.m_CenterLocal = centerLocal;
	toolRegion.m_OutNormalLocal = OutNormalLocal;
	toolRegion.m_Thickness = thickness;

}

void MisCTool_PluginDissectConnectPair::PhysicsSimulationStart(int currStep , int TotalStep , float dt)
{
	m_ToolRegion[0].UpdateToWorldSpace();
	m_ToolRegion[1].UpdateToWorldSpace();
}

void MisCTool_PluginDissectConnectPair::OneFrameUpdateStarted(float timeelapsed)
{
	//debug
// 	Ogre::ColourValue color[2]  = {Ogre::ColourValue::Blue , Ogre::ColourValue::Green};
// 	m_pManual->clear();
// 	m_pManual->begin("clampdebug",Ogre::RenderOperation::OT_LINE_LIST);
// 	for(int i = 0 ; i < 2 ; i++)
// 	{
// 		m_pManual->position(GPVec3ToOgre(m_ToolRegion[i].m_CenterWorld));
// 		m_pManual->colour(color[i]);
// 		m_pManual->position(GPVec3ToOgre(m_ToolRegion[i].m_CenterWorld + m_ToolRegion[i].m_OutNormalWorld * m_ToolRegion[i].m_Thickness));
// 		m_pManual->colour(color[i]);
// 	}
// 	m_pManual->end();

	bool leftContact = false;
	bool rightContact = false;

	//测试器械左右两边都有连接接触 且删掉不合条件的连接
	std::list<ConnPairContact>::iterator itor = m_ConnPairsContact.begin();
	for( ; itor != m_ConnPairsContact.end() ; )
	{
		bool isErase = true;
		ConnPairContact & pairInfo = *itor;
		int clusterId = pairInfo.m_ClusterId;
		int pairId = pairInfo.m_PairId;
		const VeinConnectPair & pair = pairInfo.m_VeinObj->GetConnectPair(clusterId , pairId);
		if(pair.m_Valid)
		{
			if(pair.m_BVNode)
			{
				VeinCollideData * cdata = (VeinCollideData *)pair.m_BVNode->m_UserData;
				if(cdata->m_InContact)
				{
					int regionIndex = -1;
					if(cdata->m_contactRigid == m_ToolObject->m_lefttoolpartconvex.m_rigidbody)
					{
						regionIndex = 0;
						leftContact = true;
					}
					else if(cdata->m_contactRigid == m_ToolObject->m_righttoolpartconvex.m_rigidbody)
					{
						regionIndex = 1;
						rightContact = true;
					}

					if(regionIndex == 0 || regionIndex == 1)
						isErase = false;
				}
			}
		}
		if(isErase)
			itor = m_ConnPairsContact.erase(itor);
		else
			++itor;
	}
	
	if(!(leftContact && rightContact))
		return;

	itor = m_ConnPairsContact.begin();
	for( ; itor != m_ConnPairsContact.end() ; ++itor)
	{
		//bool isErase = true;
		ConnPairContact & pairInfo = *itor;
		int clusterId = pairInfo.m_ClusterId;
		int pairId = pairInfo.m_PairId;
		const VeinConnectPair & pair = pairInfo.m_VeinObj->GetConnectPair(clusterId , pairId);
		if(pair.m_Valid)
		{
			if(pair.m_BVNode)
			{
				VeinCollideData * cdata = (VeinCollideData *)pair.m_BVNode->m_UserData;
				if(cdata->m_InContact)
				{
					float toolShaft = m_ToolObject->GetShaftAside();
					if(toolShaft > pairInfo.m_LastShaft)
					{
						float deltaL = pair.GetCurrLength() - pair.GetRestLength();
						if(toolShaft >= DISSECT_SHAFT/*(toolShaft - pairInfo.m_ShaftContact) >= m_ShaftDifferenceCanDissect*/)
							pairInfo.m_VeinObj->DisconnectPair(clusterId);
					}
					else
					{
						
					}
					pairInfo.m_LastShaft = toolShaft;
				}	
			}
		}
	}
}

void MisCTool_PluginDissectConnectPair::OneFrameUpdateEnded()
{
	std::list<ConnPairContact>::iterator itor = m_ConnPairsContact.begin();
	for( ; itor != m_ConnPairsContact.end() ; ++itor )
	{
		ConnPairContact & pair = *itor;
	}
}

void MisCTool_PluginDissectConnectPair::CollideVeinConnectPair(VeinConnectObject * veinobject , 
															   GFPhysCollideObject * convexobj, 
															   int cluster , 
															   int pair, 
															   const GFPhysVector3 & collidepoint)
{
	if(!m_ToolObject->m_lefttoolpartconvex.m_rigidbody || !m_ToolObject->m_righttoolpartconvex.m_rigidbody)
		return;

	int regionIndex = 0;
	if(convexobj == m_ToolObject->m_lefttoolpartconvex.m_rigidbody)
	{
		regionIndex = 0;
	}
	else if(convexobj == m_ToolObject->m_righttoolpartconvex.m_rigidbody)
	{
		regionIndex = 1;
	}
	else
		return;

	//判断碰撞点在钳外
	GFPhysVector3 dirVec =  collidepoint - m_ToolRegion[regionIndex].m_CenterWorld;
	float height = dirVec.Dot(m_ToolRegion[regionIndex].m_OutNormalWorld);
	if(height < m_ToolRegion[regionIndex].m_Thickness)
		return;

	float toolShaft = m_ToolObject->GetShaftAside();

	bool isExisted = false;
	std::list<ConnPairContact>::iterator itor = m_ConnPairsContact.begin();
	for( ; itor != m_ConnPairsContact.end() ; ++itor)
	{
		ConnPairContact & pairContact = *itor;
		if(pairContact.m_VeinObj == veinobject && pairContact.m_ClusterId == cluster && pairContact.m_PairId == pair)
		{	
			isExisted = true;
			break;
		}
	}

	if(!isExisted)
	{
		ConnPairContact pairContact(veinobject , cluster , pair , toolShaft , regionIndex);
		m_ConnPairsContact.push_back(pairContact);
	}
}