#include "math/GoPhysTransformUtil.h"
#include "collision/NarrowPhase/GoPhysPrimitiveTest.h"
#include "IObjDefine.h"
#include "MisMedicOrganOrdinary.h"
#include "MisMedicObjectUnion.h"
#include "CustomCollision.h"
#include "PhysicsWrapper.h"
#include "MXOgreGraphic.h"
#include "ITraining.h"
#include "stdafx.h"
#include "BasicTraining.h"
#include "MisNewTraining.h"
#include "MXOgreWrapper.h"

#include "Instruments/Tool.h"
#include "MisCTool_PluginContainer.h"
#include "CallBackForUnion.h"
#include "MxEvent.h"
#include "MxEventsDump.h"






void MisCTool_PluginContainer::ContainerRegion::UpdateToWorldSpace()
{
	const GFPhysTransform & worldtrans = m_pAttachRigid->GetWorldTransform();
	const GFPhysMatrix3 & worldRotate = worldtrans.GetBasis();

	m_OriginWorld = worldtrans * m_OriginLocal;
	m_XAxisWorld = worldRotate * m_XAxisLocal;
	m_YAxisWorld = worldRotate * m_YAxisLocal;
	m_ZAxisWorld = worldRotate * m_ZAxisLocal;



	m_TestMin =  m_LocalMin.m_x * m_XAxisWorld + m_LocalMin.m_y * m_YAxisWorld + m_LocalMin.m_z * m_ZAxisWorld + m_OriginWorld;
	m_TestMax =  m_LocalMax.m_x * m_XAxisWorld + m_LocalMax.m_y * m_YAxisWorld + m_LocalMax.m_z * m_ZAxisWorld + m_OriginWorld;

}
//========================================================================================
Ogre::Vector3 MisCTool_PluginContainer::ContainerRegion::GetLocalCoord(GFPhysSoftBodyNode * pNode)
{
	 GFPhysVector3 diff = pNode->m_CurrPosition - m_OriginWorld;
	 Ogre::Vector3 coord;
	 coord.x = diff.Dot(m_XAxisWorld);
	 coord.y = diff.Dot(m_YAxisWorld);
	 coord.z = diff.Dot(m_ZAxisWorld);
	
	 return coord;
}

GFPhysVector3 MisCTool_PluginContainer::ContainerRegion::GetLocalCoord(const GFPhysVector3 & worldCoord)
{
	GFPhysVector3 diff = worldCoord - m_OriginWorld;
	GFPhysVector3 coord;
	coord.m_x = diff.Dot(m_XAxisWorld);
	coord.m_y = diff.Dot(m_YAxisWorld);
	coord.m_z = diff.Dot(m_ZAxisWorld);
	
	return coord;
}
//========================================================================================
GFPhysVector3 MisCTool_PluginContainer::ContainerRegion::GetWorldPos(const Ogre::Vector3 & localCoord)
{
	return m_OriginWorld + m_XAxisWorld * localCoord.x + m_YAxisWorld * localCoord.y + m_ZAxisWorld * localCoord.z;
}
//========================================================================================
bool MisCTool_PluginContainer::ContainerRegion::IsInside(const Ogre::Vector3 & node , float allowRange /* = 0.0f */)
{
	//if(node.x < m_LocalMax.m_x && node.x > m_LocalMin.m_x)
		//if(node.y < m_LocalMax.m_y && node.y > m_LocalMin.m_y)
	if(node.z < (m_LocalMax.m_z + allowRange)&& node.z > (m_LocalMin.m_z - allowRange))
		return true;
	return false;
}
//========================================================================================
MisCTool_PluginContainer::MisCTool_PluginContainer(CTool * tool)
: MisMedicCToolPluginInterface(tool) 
{
	if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
		PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(this);

	m_OrganRatio = 0;
}
//========================================================================================
MisCTool_PluginContainer::~MisCTool_PluginContainer()
{
	if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
		PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(this);

	std::map<MisMedicOrgan_Ordinary*,OrganInfo>::iterator OrganItor = m_OrganMap.begin();
	
	while (OrganItor != m_OrganMap.end())
	{
		OrganInfo & organInfo = OrganItor->second;
		
		for (int c = 0; c < (int)organInfo.m_pOrgan->GetNumSubParts(); c++)
		{
			GFPhysSubConnectPart * subpart = organInfo.m_pOrgan->GetSubPart(c);

			//check separate part center of mass inside
			if ((subpart->m_PartStateFlag & (GPSESF_CONNECTED)) == 0)
			{
				GFPhysVector3 com(0, 0, 0);
				
				int numNodeInPart = (int)subpart->m_Nodes.size();

				if (numNodeInPart > 0)
				{
					for (int n = 0; n < numNodeInPart; n++)
					{
						com += subpart->m_Nodes[n]->m_CurrPosition;
					}
					com /= numNodeInPart;

					//is inside
					GFPhysVector3 coord = m_ContainerRegion.GetLocalCoord(com);
					
					if ((coord.m_z > m_ContainerRegion.m_LocalMin.m_z && coord.m_x < m_ContainerRegion.m_LocalMax.m_z) &&
						(coord.m_x > m_ContainerRegion.m_LocalMin.m_x && coord.m_x < m_ContainerRegion.m_LocalMax.m_x) &&
						(coord.m_y > m_ContainerRegion.m_LocalMin.m_y && coord.m_y < m_ContainerRegion.m_LocalMax.m_y))
					{
						organInfo.m_pOrgan->RemoveSubParts(c);
					}
				}

			}

		}
		//if(organInfo.m_InsideState == CONTAINER_INSIDE_FIXED)
		{
// 			ITraining *pTraining = m_ToolObject->GetOwnerTraining();
// 			MisNewTraining *pNewTraining = dynamic_cast<MisNewTraining*>(pTraining);
// 			pNewTraining->RemoveOrganFromWorld(organInfo.m_pOrgan);
//让训练自己处理
			MxToolEvent * pEvent = CMXEventsDump::Instance()->CreateEventNew(MxEvent::MXET_TakeOutOrganWithSpecimenBag, m_ToolObject , (void*)organInfo.m_pOrgan->m_OrganID);
			pEvent->m_UserData = m_OrganRatio;
			CMXEventsDump::Instance()->PushEvent(pEvent);
		}
		OrganItor++;
	}
}
//========================================================================================
void MisCTool_PluginContainer::SetContainerRegion(GFPhysRigidBody * pAttachRigid, 
												  const GFPhysVector3 & localOrigin, 
												  const GFPhysVector3 & localX, 
												  const GFPhysVector3 & localY, 
												  const GFPhysVector3 & localZ, 
												  const GFPhysVector3 & containMin ,
												  const GFPhysVector3 & containMax)
{
	m_ContainerRegion.m_pAttachRigid = pAttachRigid;
	m_ContainerRegion.m_OriginLocal = localOrigin;
	m_ContainerRegion.m_XAxisLocal = localX;
	m_ContainerRegion.m_YAxisLocal = localY;
	m_ContainerRegion.m_ZAxisLocal = localZ;
	m_ContainerRegion.m_LocalMin = containMin;
	m_ContainerRegion.m_LocalMax = containMax;
	
// 	m_painting.PushBackCoordAxis(CustomCoordAxis(&m_ContainerRegion.m_OriginWorld , 
// 																					&m_ContainerRegion.m_XAxisWorld, 
// 																					&m_ContainerRegion.m_YAxisWorld , 
// 																					&m_ContainerRegion.m_ZAxisWorld));
// 
// 	m_painting.PushBackPoint(CustomPoint(&m_ContainerRegion.m_TestMin , Ogre::ColourValue::Blue));
// 	m_painting.PushBackPoint(CustomPoint(&m_ContainerRegion.m_TestMax , Ogre::ColourValue::White));	


}
//========================================================================================
void MisCTool_PluginContainer::OnOrganBeRemovedFromWorld(MisMedicOrganInterface * organif)
{
	MisMedicOrgan_Ordinary *pOrgan = dynamic_cast<MisMedicOrgan_Ordinary*>(organif);
	if(pOrgan)
	{
		std::map<MisMedicOrgan_Ordinary*,OrganInfo>::iterator OrganItor = m_OrganMap.find(pOrgan);
		if(OrganItor != m_OrganMap.end())
			m_OrganMap.erase(OrganItor);
	}
}
//===========================================================================================================

void MisCTool_PluginContainer::PrepareSolveConstraint(Real Stiffness,Real TimeStep)
{
	/*
	std::map<MisMedicOrgan_Ordinary*,OrganInfo>::iterator OrganItor = m_OrganMap.begin();
	while (OrganItor != m_OrganMap.end())
	{
		OrganInfo & organInfo = OrganItor->second;
		if(organInfo.m_InsideState == CONTAINER_INSIDE_FREE)
		{
			std::list<InsideFaceInfo> & faces = organInfo.m_FacesInside;

			std::list<InsideFaceInfo>::iterator faceItor = faces.begin();
			for( ; faceItor != faces.end() ; )
			{
				InsideFaceInfo  & faceInfo = *faceItor;
				GFPhysSoftBodyFace *pFace = faceInfo.m_pFace;
				bool isInside = false;
				for(int n = 0 ; n < 3; n++)
				{
					faceInfo.m_Coords[n] = m_ContainerRegion.GetLocalCoord(pFace->m_Nodes[n]);
				}
				for(int n = 0 ; n < 3 ; n++)
				{
					if(m_ContainerRegion.IsInside(faceInfo.m_Coords[n], 0.2))
					{
						isInside = true;
						break;
					}
				}
				if(!isInside)
				{
					pFace->EnableCollideWithRigid();
					faceItor = faces.erase(faceItor);
				}
				else
					++faceItor;
			}
		}
		if(organInfo.m_FacesInside.size() == 0)//no face in side
		{
		   OrganItor = m_OrganMap.erase(OrganItor);
		}
		else
		   OrganItor++;
	}
	*/
}
//===========================================================================================================
void MisCTool_PluginContainer::SolveConstraint(Real Stiffniss,Real TimeStep)
{
	/*
	return;

	std::map<MisMedicOrgan_Ordinary*,OrganInfo>::iterator OrganItor = m_OrganMap.begin();
	while (OrganItor != m_OrganMap.end())
	{
		OrganInfo & organInfo = OrganItor->second;

		if(organInfo.m_InsideState == CONTAINER_INSIDE_FREE)
		{
			std::list<InsideFaceInfo> & faces = organInfo.m_FacesInside;

			std::list<InsideFaceInfo>::iterator faceItor = faces.begin();

			float minHeight = FLT_MAX;
			float maxHeight = -FLT_MAX;

			for( ; faceItor != faces.end() ; ++faceItor)
			{
				InsideFaceInfo  & faceInfo = *faceItor;
				GFPhysSoftBodyFace *pFace = faceInfo.m_pFace;
				for(int n = 0 ; n < 3; n++)
				{
					GFPhysSoftBodyNode *pNode = pFace->m_Nodes[n];
					Ogre::Vector3 & coord = faceInfo.m_Coords[n];

					GFPhysVector3 corr = GFPhysVector3(0,0,0);

					if(coord.x < m_ContainerRegion.m_LocalMin.m_x)
					{
						corr += (m_ContainerRegion.m_LocalMin.m_x - coord.x) * m_ContainerRegion.m_XAxisWorld;
						coord.x = m_ContainerRegion.m_LocalMin.m_x;
					}
					else if(coord.x > m_ContainerRegion.m_LocalMax.m_x)
					{
						corr += (m_ContainerRegion.m_LocalMax.m_x - coord.x) * m_ContainerRegion.m_XAxisWorld;
						coord.x = m_ContainerRegion.m_LocalMax.m_x;
					}

					if(coord.y < m_ContainerRegion.m_LocalMin.m_y)
					{
						corr += (m_ContainerRegion.m_LocalMin.m_y - coord.y) * m_ContainerRegion.m_YAxisWorld;
						coord.y = m_ContainerRegion.m_LocalMin.m_y;
					}
					else if(coord.y > m_ContainerRegion.m_LocalMax.m_y)
					{
						corr += (m_ContainerRegion.m_LocalMax.m_y - coord.y) * m_ContainerRegion.m_YAxisWorld;
						coord.y = m_ContainerRegion.m_LocalMax.m_y;
					}

					if(coord.z > m_ContainerRegion.m_LocalMax.m_z)
					{
						corr += (m_ContainerRegion.m_LocalMax.m_z - coord.z) * m_ContainerRegion.m_ZAxisWorld;
						coord.z = m_ContainerRegion.m_LocalMax.m_z;
					}

					pNode->m_CurrPosition += corr;

					if(coord.z > maxHeight)
					{
						maxHeight = coord.z;
						organInfo.m_BottomFace = pFace;
					}
					if(coord.z < minHeight)
					{
						minHeight = coord.z;
						organInfo.m_TopFace = pFace;
					}
					if((coord.z > m_ContainerRegion.m_LocalMax.m_z - 0.005) && organInfo.m_CanFix)
					{
						organInfo.m_InsideState = CONTAINER_INSIDE_FIXED;
						organInfo.m_BottomFace = pFace;
					}
				}
			}

			for(int n = 0 ; n < 3; n++)
			{
				organInfo.m_TopFaceCoords[n] = m_ContainerRegion.GetLocalCoord(organInfo.m_TopFace->m_Nodes[n]);
				organInfo.m_BottomFaceCoords[n] = m_ContainerRegion.GetLocalCoord(organInfo.m_BottomFace->m_Nodes[n]);
			}
		}
		else if(organInfo.m_InsideState == CONTAINER_INSIDE_FIXED)
		{
			for(int n = 0 ; n < 3; n++)
			{
				organInfo.m_TopFace->m_Nodes[n]->m_CurrPosition = m_ContainerRegion.GetWorldPos(organInfo.m_TopFaceCoords[n]);
				organInfo.m_BottomFace->m_Nodes[n]->m_CurrPosition = m_ContainerRegion.GetWorldPos(organInfo.m_BottomFaceCoords[n]);
			}
		}
		OrganItor++;
	}
	*/
}
//===========================================================================================================
GFPhysVector3 MisCTool_PluginContainer::GetPluginForceFeedBack()
{
	return GFPhysVector3(0,0,0);
}
//===========================================================================================================
void MisCTool_PluginContainer::RigidSoftCollisionsSolved(const GFPhysAlignedVectorObj<GFPhysSoftRigidContact> & RSContactConstraints)
{
	
	 
}
//==============================================================================================
void MisCTool_PluginContainer::PhysicsSimulationStart(int currStep , int TotalStep , float dt)
{
	m_ContainerRegion.UpdateToWorldSpace();

	/*
	ITraining * itraining = m_ToolObject->GetOwnerTraining();
	MisNewTraining * pNewTraining = dynamic_cast<MisNewTraining*>(itraining);
	bool canLetOut = false;
	if(pNewTraining)
	{
		Ogre::Vector3 zdir = GPVec3ToOgre(m_ContainerRegion.m_ZAxisWorld);
		if(zdir.dotProduct(pNewTraining->GetSceneGravityDir()) < 0)
			canLetOut = true;
	}
	*/

	/*
	std::map<MisMedicOrgan_Ordinary*,OrganInfo>::iterator OrganItor = m_OrganMap.begin();
	while (OrganItor != m_OrganMap.end())
	{

		MisMedicOrgan_Ordinary *pOrgan = OrganItor->first;

		OrganInfo & organInfo = OrganItor->second;

		if(canLetOut && organInfo.m_InsideState == CONTAINER_INSIDE_FIXED)
			organInfo.m_InsideState = CONTAINER_INSIDE_FREE;

		std::list<InsideFaceInfo> & faces = organInfo.m_FacesInside;
		
		GFPhysSoftBody *pSoftBody = pOrgan->m_physbody;

		int totalFaceNum = pSoftBody->GetNumFace();

		int insideNum = faces.size();

		m_OrganRatio = 100 * insideNum / totalFaceNum;
		//if((float)insideNum / (float)totalFaceNum > 0.9)
		if(m_OrganRatio > 90)
		{
			pOrgan->m_IsInContainer = true;
			organInfo.m_CanFix = true;
		}
		else
			pOrgan->m_IsInContainer = false;

		OrganItor++;
	}*/
}
//==============================================================================================
void MisCTool_PluginContainer::PhysicsSimulationEnd(int currStep , int TotalStep , float dt)
{
	m_OrganMap.clear();

	for (int f = 0; f < (int)m_ToolObject->m_ToolColliedFaces.size(); f++)
	{
		const ToolCollidedFace & datacd = m_ToolObject->m_ToolColliedFaces[f];

		if (datacd.m_collideRigid != m_ContainerRegion.m_pAttachRigid)
			continue;
		 
		MisMedicOrgan_Ordinary * pOrgan = dynamic_cast<MisMedicOrgan_Ordinary*>(m_ToolObject->GetOwnerTraining()->GetOrgan((GFPhysSoftBody*)datacd.m_collideSoft));

		if (pOrgan)
		{
			std::map<MisMedicOrgan_Ordinary*, OrganInfo>::iterator itor = m_OrganMap.find(pOrgan);

			int PartID = datacd.m_collideFace->m_Nodes[0]->m_SepPartID;

			if (itor != m_OrganMap.end())
			{
				OrganInfo & organInfo = itor->second;
				organInfo.m_SubPart.insert(PartID);
			}
			else
			{
				OrganInfo organInfo(pOrgan);
				organInfo.m_SubPart.insert(PartID);
				m_OrganMap[pOrgan] = organInfo;
			}
		}
	}
	
	//check every organ's part
	
	/*ITraining * currtrain = m_ToolObject->GetOwnerTraining();

	for(size_t f = 0 ; f < m_ToolObject->m_ToolColliedFaces.size() ; f++)
	{
		const ToolCollidedFace & datacd = m_ToolObject->m_ToolColliedFaces[f];

		if(datacd.m_collideRigid != m_ContainerRegion.m_pAttachRigid)
			continue;

		GFPhysSoftBody * softbody = (GFPhysSoftBody*)datacd.m_collideSoft;

		MisMedicOrgan_Ordinary * pOrgan = dynamic_cast<MisMedicOrgan_Ordinary*>(currtrain->GetOrgan(softbody));

		if(!pOrgan)
			continue;

		GFPhysSoftBodyFace *pFace = datacd.m_collideFace;
		bool inside = false;
		for(int n = 0 ; n < 3 ; n++)
		{
			GFPhysSoftBodyNode *pNode = pFace->m_Nodes[n];
			Ogre::Vector3 coord =  m_ContainerRegion.GetLocalCoord(pNode);
			float height = m_ContainerRegion.m_LocalMin.GetZ() - coord.z;
			if(height < 0.1 && height > 0 &&
				(coord.x > m_ContainerRegion.m_LocalMin.m_x && coord.x < m_ContainerRegion.m_LocalMax.m_x) &&
				(coord.y > m_ContainerRegion.m_LocalMin.m_y && coord.y < m_ContainerRegion.m_LocalMax.m_y))
			{
				inside = true;
				break;
			}
			else
			{
// 				inside = false;
// 				break;
			}
		}
		if(inside)
		{
			pFace->DisableCollideWithRigid();

			//m_InsideFacesInfo.push_back();
			std::map<MisMedicOrgan_Ordinary*,OrganInfo>::iterator itor = m_OrganMap.find(pOrgan);
			if(itor != m_OrganMap.end())
			{
				OrganInfo & organInfo = itor->second;
				std::list<InsideFaceInfo> & faces = organInfo.m_FacesInside;
				faces.push_back(InsideFaceInfo(pFace , pOrgan->m_OrganID));
			}
			else
			{
				OrganInfo organInfo(pOrgan);
				organInfo.m_FacesInside.push_back(InsideFaceInfo(pFace , pOrgan->m_OrganID));
				m_OrganMap[pOrgan] = organInfo;
			}
		}
	}*/

}
//==============================================================================================
void MisCTool_PluginContainer::OneFrameUpdateStarted(float timeelapsed)
{
	//m_painting.Update(timeelapsed);
}
//==============================================================================================
void MisCTool_PluginContainer::OnSoftBodyFaceBeDeleted(GFPhysSoftBody *sb , GFPhysSoftBodyFace *face)
{

}
//===============================================================================================
void MisCTool_PluginContainer::OnSoftBodyFaceBeAdded(GFPhysSoftBody *sb , GFPhysSoftBodyFace *face)
{
	
}
//==============================================================================================
