#include "stdafx.h"
#include "BasicNewTraining_Level13.h"
#include "MisMedicOrganOrdinary.h"

#include "VeinConnectObject.h"
#include "TextureBloodEffect.h"
#include "MisMedicEffectRender.h"
#include "MXEventsDump.h"
#include "MXToolEvent.h"
#include "OgreAxisAlignedBox.h"
#include "Instruments/Tool.h"
#include "Inception.h"
#include "LightMgr.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "Instruments/MisCTool_PluginClamp.h"
#include "MisMedicOrganAttachment.h"
#include "CustomConstraint.h"
#include "Instruments/ElectricHook.h"
#include "Instruments/GraspingForceps.h"
#include "MisMedicObjectUnion.h"
#include "MXOgreGraphic.h"
#include "CustomCollision.h"

//判断两条线段p1p2、q1q2是否相交
bool Intersect2D(const Ogre::Vector2 & p1,const Ogre::Vector2 & p2,const Ogre::Vector2 & q1,const Ogre::Vector2 & q2,Ogre::Vector2 & intersectPoint)
{
	Ogre::Vector2 u = p2 - p1;
	Ogre::Vector2 v = q2 - q1;
	Ogre::Vector2 w = p1 - q1;
	float d = u.perpendicular().dotProduct(v);
	if(fabs(d) <= FLT_EPSILON)
	{
		return false;
	}
	else
	{
		float sI = v.perpendicular().dotProduct(w) / d;
		if(sI < 0 || sI > 1)
			return false;

		float tI = u.perpendicular().dotProduct(w) / d;
		if(tI < 0 || tI > 1)
			return false;
		intersectPoint = p1 + sI * u;
	}
	return true;
}

bool ApplyTextureToMaterial(const Ogre::String & materialName,const Ogre::String& srcTextureName,const Ogre::String& distTextureNameAlias)
{
	Ogre::MaterialPtr material = Ogre::MaterialManager::getSingletonPtr()->getByName(materialName);
	if(material.isNull())
		return false;
	Ogre::Technique * tec = material->getTechnique(0);
	if(tec && tec->getNumPasses())
	{
		Ogre::Pass * pass = tec->getPass(0);
		if(pass)
		{
			for(int i = 0;i<pass->getNumTextureUnitStates();++i)
			{
				Ogre::TextureUnitState * pState = pass->getTextureUnitState(i);
				if(pState->getTextureNameAlias() == distTextureNameAlias)
				{
					pState->setTextureName(srcTextureName);
					return true;
				}
			}
		}
	}
	return false;
}

CBasicNewTraining_Level13::CBasicNewTraining_Level13(void)
{
	m_numShowedVessel = 0;
	m_bFinish = false;
	CTipMgr::Instance()->SetQueeu(false);	
	m_lastPunctureOrganTime = 0;
	m_lastPunctureVesselTime = 0;
	m_lastElecOrganTime = 0;
	m_lastElecVesselTime = 0;
	m_deltaTime = 500;
	m_bOffVessel = false;
	m_trainbegintime = 0;
	m_bElecOff = false;
}

CBasicNewTraining_Level13::~CBasicNewTraining_Level13(void)
{
	m_organNodesMap.clear();
	for(std::size_t i = 0;i<m_vecVesselInfo.size();++i)
	{
		delete m_vecVesselInfo[i];
	}
	m_vecVesselInfo.clear();
}


bool CBasicNewTraining_Level13::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{
	bool bRet = MisNewTraining::Initialize(pTrainingConfig,pToolConfig);
	if(!bRet)
		return false;
	init();
	return bRet;
}

void AttachNodesCenterToBody(const std::vector<GFPhysSoftBodyNode*> & nodes , GFPhysSoftBody * attacbody)
{
	GFPhysVector3 centerPos(0,0,0);

	for(size_t n = 0 ; n < nodes.size(); n++)
	{
		GFPhysVector3 nodepos = nodes[n]->m_UnDeformedPos;

		float minDist = FLT_MAX;

		GFPhysSoftBodyTetrahedron * minTetra = 0;

		GFPhysVector3 closetPointInSoft;

		//GFPhysSoftBodyTetrahedron * tetra = attacbody->GetTetrahedronList();

		//while(tetra)
		for(size_t th = 0 ; th < attacbody->GetNumTetrahedron() ; th++)
		{
			GFPhysSoftBodyTetrahedron * tetra = attacbody->GetTetrahedronAtIndex(th);

			GFPhysVector3 closetPoint = ClosetPtPointTetrahedron(nodepos, ///一个四面体与一个点最近的距离点
				tetra->m_TetraNodes[0]->m_UnDeformedPos,
				tetra->m_TetraNodes[1]->m_UnDeformedPos,
				tetra->m_TetraNodes[2]->m_UnDeformedPos,
				tetra->m_TetraNodes[3]->m_UnDeformedPos);

			float dist = (closetPoint-nodepos).Length();

			if(dist < minDist)
			{
				minDist = dist;
				closetPointInSoft = closetPoint;
				minTetra = tetra;
			}

			//tetra = tetra->m_Next;
		}
		if(minDist < FLT_MAX)
		{
			float weights[4];
			bool  gettedf = GetPointBarycentricCoordinate(  minTetra->m_TetraNodes[0]->m_UnDeformedPos,///计算一点在四面体的重心权重
				minTetra->m_TetraNodes[1]->m_UnDeformedPos,
				minTetra->m_TetraNodes[2]->m_UnDeformedPos,
				minTetra->m_TetraNodes[3]->m_UnDeformedPos,
				closetPointInSoft,
				weights);
			if(gettedf)
			{
				TetrahedronAttachConstraint * cs = new TetrahedronAttachConstraint(nodes[n] , ///距离约束，点随着四面体动而动
					minTetra,//->m_TetraNodes,
					weights);

				cs->SetStiffness(0.99f);
				PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(cs);
			}
		}
	}
}

bool GetSoftBodyNodesFormIndeces(const GFPhysSoftBody * pSoftBody,const std::vector<int>& indeces,std::vector<GFPhysSoftBodyNode*>& nodes)
{
	GFPhysSoftBody * pBody = const_cast<GFPhysSoftBody*>(pSoftBody);
	if(pBody == NULL || indeces.size() == 0)
		return false;

	std::size_t initSize = nodes.size();
	GFPhysSoftBodyNode * pNode = NULL;
	for(std::size_t i = 0;i<indeces.size();++i)
	{
		pNode = pBody->GetNode(indeces[i]);
		if(pNode)
			nodes.push_back(pNode);
	}

	return nodes.size() > initSize;
}
	
	 
void CBasicNewTraining_Level13::init()
{
// 	read nodes index form file
// 	std::ifstream fin;
// 	fin.open("../Config/MultiPortConfig/Basic/NBT_Training_13/nodesInfo.data");
// 	
// 	m_organNodesMap.clear();
// 	if(fin.is_open())
// 	{
// 		while(!fin.eof())
// 		{
// 			char buffer[1024] = {0};
// 			fin.getline(buffer,sizeof(buffer));
// 			if(strlen(buffer) < 4)
// 				continue;
// 			std::string str(buffer);
// 			std::istringstream strStream(str);
// 
// 			int organId = 0;
// 			int nodeIndex = 0;
// 
// 			strStream>>organId;
// 			if(organId < 100 || organId > 110)
// 				MXASSERT(0 && "read organ id error!");
// 
// 			while(1)
// 			{
// 				strStream.get();		//read a char
// 				if(strStream.eof())
// 					break;
// 				strStream>>nodeIndex;
// 				m_organNodesMap[organId].push_back(nodeIndex);
// 			}
// 		}
// 		
// 		fin.close();
// 	}
// 	else
// 		MXASSERT(0 && "open file faile!!!");

	//
	DynObjMap::iterator organItr = m_DynObjMap.begin();
	OrganNodesMap::iterator organNodesItr = m_organNodesMap.end();
	MisMedicOrgan_Ordinary * organs[3];	

	while(organItr != m_DynObjMap.end())
	{
		MisMedicOrgan_Ordinary * pOrgan = dynamic_cast<MisMedicOrgan_Ordinary*>(organItr->second);
		if(pOrgan)
		{
			if(pOrgan->m_OrganID == 102)	//bottomVesselLeft
			{
				//find organ node
				organs[2] = pOrgan;
				m_allOrgans.insert(pOrgan);
			}
			else if(pOrgan->m_OrganID == 103)	//bottomVesselRight
			{
				//find organ node
				m_allOrgans.insert(pOrgan);
			}
			else if(pOrgan->m_OrganID == 104)	//topVesselLeft
			{
				//find organ node
				organs[0] = pOrgan;
				m_allOrgans.insert(pOrgan);
			}
			else if(pOrgan->m_OrganID == 105)	//topVesselCenter
			{
				organs[1] = pOrgan;
				m_allOrgans.insert(pOrgan);
			}
			else if(pOrgan->m_OrganID == 106)	//topVesselRight
			{
				//find organ node
				m_allOrgans.insert(pOrgan);
			}

		}
		++organItr;
	}


	//create vessel info
	int nodeIndces[6] = {94,30,		//top VesselLeft
						30,21,		//top VesselCenter
						45,109};	//bottm VesselLeft
	float radius[3] = {0.35,0.222,0.157};
	for(int i = 0;i<3;++i)
	{
		GFPhysSoftBodyNode * pNode1 = organs[i]->m_physbody->GetNode(nodeIndces[i * 2]);
		GFPhysSoftBodyNode * pNode2 = organs[i]->m_physbody->GetNode(nodeIndces[i * 2 + 1]);
		Ogre::Vector3 endPoint1 = GPVec3ToOgre(pNode1->m_CurrPosition);
		Ogre::Vector3 endPoint2 = GPVec3ToOgre(pNode2->m_CurrPosition);

		VesselInfo * pInfo = new VesselInfo(organs[i],endPoint1,endPoint2,(Position)i,radius[i]);
		m_vecVesselInfo.push_back(pInfo);
		//ApplyTextureToMaterial(organs[i]->getMaterialName(),"training13_mark.png","BaseMap");
	}

	//set pair info
	std::vector<VeinConnectObject*> connectobjets = GetVeinConnectObjects();
	for(std::size_t i = 0;i<m_vecVesselInfo.size();++i)
	{
		for(std::size_t v = 0;v < connectobjets.size();++v)
		{
			m_vecVesselInfo[i]->SetPairInfo(connectobjets[v],m_pLargeCamera);
		}
	}
	for(std::size_t v = 0;v < connectobjets.size();++v)
	{
		connectobjets[v]->m_CanBlood = false;//temp disable blood
	}

}

void CBasicNewTraining_Level13::OnHandleEvent(MxEvent* pEvent)
{
	if(m_bFinish)
		return;
	const VeinConnectPair * pPair = NULL;
	MxToolEvent * pToolEvent = (MxToolEvent *)pEvent;
	MisMedicOrgan_Ordinary * pOrgan = NULL;
	float curTime;

	switch(pEvent->m_enmEventType)
	{
	case MxEvent::MXET_VeinConnectBreak:
	case MxEvent::MXET_VeinConnectBurned:
		pPair = static_cast<const VeinConnectPair*>(pToolEvent->m_pUserPoint);

		int ratios[3];
		for(std::size_t i = 0;i < m_vecVesselInfo.size();++i)
		{
			ratios[i] = 100 - m_vecVesselInfo[i]->GetCurRatio();		//100减去覆盖率就是完成率	
			bool bAlreadyShow = m_vecVesselInfo[i]->Showed();

			if(m_vecVesselInfo[i]->ErasePair(pPair))
			{
				ratios[i] = 100 - m_vecVesselInfo[i]->GetCurRatio();	//获取新的完成率
#define NewMode
#ifdef	NewMode
				if(bAlreadyShow)
					continue;

				const char* operateItemName = nullptr;
				switch(m_vecVesselInfo[i]->Pos())
				{
				case TopLeft:
					operateItemName = "ShowLeftVessel";
					break;
				case TopCenter:
					operateItemName = "ShowRightVessel";
					break;
				case BottomLeft:
					operateItemName = "ShowUpVessel";
					break;
				default:
					throw std::string("error");
				}

				MxOperateItem* operateItem = nullptr;
				AddOperateItem(operateItemName, ratios[i], false, AM_ReplaceAll, &operateItem);	//默认满分
				if(ratios[i] < 100)
					operateItem->ScaleScoreValueOfLastScoreItem(ratios[i] / 100.f,1.f);				//如果完全显示则不进行分数缩放，为满分
				else
				{
					++m_numShowedVessel;
					if(m_numShowedVessel == 3)
					{
						TrainingFinish();
						m_bFinish = true;
					}
				}
#else

				if(bAlreadyShow == false && ratios[i] == 100)
				{
					++m_numShowedVessel;
					switch(m_vecVesselInfo[i]->Pos())
					{
					case TopLeft:	//上面左侧血管--屏幕左侧血管	0
						CScoreMgr::Instance()->Grade("ShowLeftVessel");
						break;
					case TopCenter:	//上面右侧血管--屏幕右侧血管	1
						CScoreMgr::Instance()->Grade("ShowRightVessel");
						break;
					case BottomLeft://底部左侧血管--屏幕上侧血管	2
						CScoreMgr::Instance()->Grade("ShowUpVessel");
						break;
					}

					if(m_numShowedVessel == 3)
					{
						TrainingFinish();
						m_bFinish = true;
					}
				}
#endif
			}
		}

		CTipMgr::Instance()->ShowTip("OperateRatio",ratios[0],ratios[1],ratios[2]);
		break;
	case MxEvent::MXET_PunctureSurface:
		pOrgan = static_cast<MisMedicOrgan_Ordinary * >(pToolEvent->m_pUserPoint);
		curTime = GetTickCount();
		if(pOrgan->m_OrganID == 100 || pOrgan->m_OrganID == 101)		//戳伤器官
		{
			if(curTime - m_lastPunctureOrganTime < m_deltaTime)
				return;
			m_lastPunctureOrganTime = curTime;
			CTipMgr::Instance()->ShowTip("PunctureOrgan");
			CScoreMgr::Instance()->Grade("PunctureOrgan",true);
			AddOperateItem("PunctureOrgan", 1.f, true, AM_MergeValue);
		}
		else															//戳伤血管
		{
			if(curTime - m_lastPunctureVesselTime < m_deltaTime)
				return;
			m_lastPunctureVesselTime = curTime;
			CTipMgr::Instance()->ShowTip("PunctureVessel");
			CScoreMgr::Instance()->Grade("PunctureVessel",true);
			AddOperateItem("PunctureVessel", 1.f, true, AM_MergeValue);
		}
		break;
	case MxEvent::MXET_ElecCoagStart:
	case MxEvent::MXET_ElecCutStart:
		pOrgan = dynamic_cast<MisMedicOrgan_Ordinary * >(pToolEvent->GetOrgan());
		curTime = GetTickCount();
		if(pOrgan->m_OrganID == 100 || pOrgan->m_OrganID == 101)		//电到器官
		{
			if(curTime - m_lastElecOrganTime < m_deltaTime)
				return;
			m_lastElecOrganTime = curTime;
			CTipMgr::Instance()->ShowTip("ElecOrgan");
			CScoreMgr::Instance()->Grade("ElecOrgan",true);
			AddOperateItem("ElecOrgan", 1.f, true, AM_MergeValue);
		}
		else															//电到血管
		{
			if(curTime - m_lastElecVesselTime < m_deltaTime)
				return;
			m_lastElecVesselTime = curTime;
			CTipMgr::Instance()->ShowTip("ElecVessel");
			CScoreMgr::Instance()->Grade("ElecVessel",true);
			AddOperateItem("ElecVessel", 1.f, true, AM_MergeValue);
		}
		break;
	}
}

void CBasicNewTraining_Level13::OnOrganCutByTool(MisMedicOrganInterface * pOrgan , bool iselectriccut)
{
	if(!m_bFinish)
	{
		std::set<MisMedicOrgan_Ordinary*>::iterator itr = m_allOrgans.find((MisMedicOrgan_Ordinary*)pOrgan);
		if(itr != m_allOrgans.end())
		{
			if ((*itr)->m_OrganID >= 102)
			{
				if ((*itr)->GetNumSubParts() > 1)
				{
					m_bOffVessel = true;
				}
			}
			TrainingFatalError();
		}
		m_bFinish = true;
	}
}

bool CBasicNewTraining_Level13::Update(float dt)
{
	MisNewTraining::Update(dt);
	if(!m_bTrainingRunning)
	{
		//CTipMgr::Instance()->ShowTip("TrainingTimeout");
		m_bFinish = true;
	}
	
	if (!m_bElecOff)
	{
		ITool* lefttool = m_pToolsMgr->GetLeftTool();
		if (lefttool)
		{
			if (lefttool->GetElectricLeftPad() || lefttool->GetElectricRightPad())
			{
				m_bElecOff = true;
			}
		}
		ITool* righttool = m_pToolsMgr->GetRightTool();
		if (righttool)
		{
			if (righttool->GetElectricLeftPad() || righttool->GetElectricRightPad())
			{
				m_bElecOff = true;
			}
		}
	}
	
// 	m_drawObject.Clear();
//   	int i = 0;
//   	//for(int i = 0;i<3;++i)
//   	{
//   		GFPhysSoftBodyNode * pNode1 = organs[i]->m_physbody->GetNode(nodeIndces[i * 2]);
//   		GFPhysSoftBodyNode * pNode2 = organs[i]->m_physbody->GetNode(nodeIndces[i * 2 + 1]);
//   		Ogre::Vector3 endPoint1 = GPVec3ToOgre(pNode1->m_CurrPosition);
//   		Ogre::Vector3 endPoint2 = GPVec3ToOgre(pNode2->m_CurrPosition);
//   		m_drawObject.AddDynamicPoint(endPoint1,0.15,Ogre::ColourValue(1,0,0));
//   		m_drawObject.AddDynamicPoint(endPoint2,0.15,Ogre::ColourValue(1,0,0));
//   
//   	}
//   	
//   	//for(std::size_t i = 0;i < m_vecVesselInfo.size();++i)
//   	{
//   		const std::set<const VeinConnectPair*>& pairs = m_vecVesselInfo[i]->GetPairs();
//   		for(std::set<const VeinConnectPair*>::const_iterator itr = pairs.begin();itr != pairs.end();++itr)
//   		{
//   			const VeinConnectPair& pair = **itr;
//   			GFPhysSoftBodyFace * faceA = pair.m_faceA;
//   			GFPhysVector3 point1 = faceA->m_Nodes[0]->m_CurrPosition*pair.m_weightsA[0]
//   			+faceA->m_Nodes[1]->m_CurrPosition*pair.m_weightsA[1]
//   			+faceA->m_Nodes[2]->m_CurrPosition*pair.m_weightsA[2];
//   
//   			GFPhysSoftBodyFace * faceB = pair.m_faceB;
//   			GFPhysVector3 point2 = faceB->m_Nodes[0]->m_CurrPosition*pair.m_weightsB[0]
//   			+faceB->m_Nodes[1]->m_CurrPosition*pair.m_weightsB[1]
//   			+faceB->m_Nodes[2]->m_CurrPosition*pair.m_weightsB[2];
//   
//   			Ogre::Vector3 endPoint1 = GPVec3ToOgre(point1);
//   			Ogre::Vector3 endPoint2 = GPVec3ToOgre(point2);
//   			m_drawObject.AddDynamicLine(endPoint1,endPoint2,Ogre::ColourValue(1.f,0,0));
//   		}
//   	}
//   	m_drawObject.Update();


 /*	m_drawObject.Clear();
	Ogre::Vector3 lightPos = CLightMgr::Instance()->GetLight()->getPosition();
	Ogre::Vector3 lightPos1 = CLightMgr::Instance()->GetLight()->getDerivedPosition();
	Ogre::Vector3 lightDir1 = CLightMgr::Instance()->GetLight()->getDerivedDirection();

	//m_drawObject.DrawPoint(lightPos,1.f,Ogre::ColourValue(1.f,0.f,0.f));
	m_drawObject.AddDynamicPoint(lightPos1,0.52);
	Ogre::Vector3 newPos = lightPos1 + lightDir1 * 5;
	m_drawObject.AddDynamicPoint(newPos,.25f,Ogre::ColourValue(1.f,0.f,0.f));
	m_drawObject.AddDynamicLine(lightPos1,newPos);

  	m_drawObject.Update();*/
	return true;
}


void CBasicNewTraining_Level13::VesselInfo::SetPairInfo(const VeinConnectObject * pVeinConnectObject,Ogre::Camera * pCamera)
{
	assert(pVeinConnectObject);
	//static TrainingCommon::DrawObject drawObject;

	//Ogre::Plane plane(Ogre::Vector3(1,1,0),Ogre::Vector3(0,0,0),Ogre::Vector3(1,0,0));	//z轴向上
	//Ogre::Vector3 cameraDirection = pCamera->getDirection();
	Ogre::Vector3 cameraDirection = pCamera->getDerivedDirection();
	Ogre::Plane plane(cameraDirection,1);

	for(std::size_t c = 0 ;c < pVeinConnectObject->m_clusters.size();++c)///肉筋膜
	{
		const VeinConnectCluster  & cluster = pVeinConnectObject->m_clusters[c];

        for(std::size_t p = 0;p < 2; ++p)
        {
            const VeinConnectPair & pair = cluster.m_pair[p];///一根筋

			GFPhysSoftBodyFace * faceA = pair.m_faceA;
			GFPhysVector3 point1 = faceA->m_Nodes[0]->m_CurrPosition*pair.m_weightsA[0]
								+faceA->m_Nodes[1]->m_CurrPosition*pair.m_weightsA[1]
								+faceA->m_Nodes[2]->m_CurrPosition*pair.m_weightsA[2];

			GFPhysSoftBodyFace * faceB = pair.m_faceB;
			GFPhysVector3 point2 = faceB->m_Nodes[0]->m_CurrPosition*pair.m_weightsB[0]
								+faceB->m_Nodes[1]->m_CurrPosition*pair.m_weightsB[1]
								+faceB->m_Nodes[2]->m_CurrPosition*pair.m_weightsB[2];

			Ogre::Vector3 endPoint1 = GPVec3ToOgre(point1);
			Ogre::Vector3 endPoint2 = GPVec3ToOgre(point2);

			endPoint1 = plane.projectVector(endPoint1);
			endPoint2 = plane.projectVector(endPoint2);
			//drawObject.DrawLine(endPoint1,endPoint2);

			Ogre::Vector3 endPoint3 = plane.projectVector(this->endPoint1);
			Ogre::Vector3 endPoint4 = plane.projectVector(this->endPoint2);
			//drawObject.DrawLine(endPoint3,endPoint4,Ogre::ColourValue(1,0,0));

			Ogre::Vector2 p1(endPoint1.x,endPoint1.y);
			Ogre::Vector2 p2(endPoint2.x,endPoint2.y);
			Ogre::Vector2 q1(endPoint3.x,endPoint3.y);
			Ogre::Vector2 q2(endPoint4.x,endPoint4.y);
			Ogre::Vector2 intersectPoint;
			if(pair.m_Valid && pair.m_BVNode)
			{
 				if(Intersect2D(p1,p2,q1,q2,intersectPoint)) //
 					pairs.insert(&pair);
 				else
				{
					float distance,s,t;
					GFPhysVector3 p1,p2;
                    distance = GPClosestPtSegmentSegment(OgreToGPVec3(this->endPoint1),///两线段的最短距离向量
													   OgreToGPVec3(this->endPoint2),
													   point1,
													   point2,
													   s,///返回点在线段的权重
													   t,
													   p1,///距离最近的点
													   p2);
					if(distance < radius)
					{
						pairs.insert(&pair);
					}
				}
				//drawObject.DrawPoint(Ogre::Vector3(intersectPoint.x,intersectPoint.y,0),0.01,Ogre::ColourValue(1,1,0));
			}
		}
	}
	initNumPair = pairs.size();
}	

bool CBasicNewTraining_Level13::VesselInfo::ErasePair(const VeinConnectPair * pPair)
{
	std::set<const VeinConnectPair*>::iterator itr = pairs.find(pPair);
	if(itr == pairs.end())
		return false;

	pairs.erase(pPair);
	std::size_t curNumPair = pairs.size();
	float tempRatio = (float)curNumPair / (float)initNumPair;
	if(tempRatio <= 0.001f)
	{
		//pOrgan->setMaterialName("MisTraining13/vesselMarked");
		ApplyTextureToMaterial(pOrgan->getMaterialName(),"training13_mark.png","BaseMap");
		bShowed = true;
		curRatio = 0;
	}
	else
	{
		curRatio = tempRatio * 100;
		if(curRatio == 0)
			curRatio = 1;
	}
	return true;
}

bool CBasicNewTraining_Level13::VesselInfo::Showed()
{
	return bShowed;
}

void CBasicNewTraining_Level13::OnSaveTrainingReport()
{
	float usedtime = GetElapsedTime();

	float ratio = 0;
	int vesselcount = m_vecVesselInfo.size();
	for (int i = 0; i < vesselcount; ++i)
	{
		Ogre::String itemname = "vesselRatio" + Ogre::StringConverter::toString(i);
		ratio = 100 - m_vecVesselInfo[i]->GetCurRatio();
		ratio /= 100.0f;
		if (ratio == 1.0)
			COnLineGradeMgr::Instance()->SendGrade(itemname + "complete", m_trainbegintime, usedtime);
		else if (ratio >= 0.8)
			COnLineGradeMgr::Instance()->SendGrade(itemname + "In08", m_trainbegintime, usedtime);
		else if (ratio >= 0.6)
			COnLineGradeMgr::Instance()->SendGrade(itemname + "In06", m_trainbegintime, usedtime);
		else if (ratio > 0)
			COnLineGradeMgr::Instance()->SendGrade(itemname + "In00", m_trainbegintime, usedtime);
		else
			COnLineGradeMgr::Instance()->SendGrade(itemname + "Out00", m_trainbegintime, usedtime);
	}

	if (m_bElecOff)
		COnLineGradeMgr::Instance()->SendGrade("electricPeel", m_trainbegintime, usedtime);
	else
		COnLineGradeMgr::Instance()->SendGrade("mechanicalPeel", m_trainbegintime, usedtime);

	float allElecTime	  = m_pToolsMgr->GetTotalElectricTime();
	float validElecTime = m_pToolsMgr->GetValidElectricTime();
	float eleceffecient = validElecTime / allElecTime;
	if (eleceffecient >= 0.7)
		COnLineGradeMgr::Instance()->SendGrade("mostElecInOrgan", m_trainbegintime, usedtime);
	else if (eleceffecient > 0)
		COnLineGradeMgr::Instance()->SendGrade("leastElecInOrgan", m_trainbegintime, usedtime);

	float keepelectime = m_pToolsMgr->GetMaxKeeppingElectricTime();
	if ( keepelectime >= 5.0)
		COnLineGradeMgr::Instance()->SendGrade("onceElecTimeLong", m_trainbegintime, usedtime);
	else if (keepelectime > 0)
		COnLineGradeMgr::Instance()->SendGrade("onceElecTimeValid", m_trainbegintime, usedtime);

	if (m_bOffVessel)
		COnLineGradeMgr::Instance()->SendGrade("vessel_breakoff", m_trainbegintime, usedtime);
	else
		COnLineGradeMgr::Instance()->SendGrade("vessel_integrity", m_trainbegintime, usedtime);

	ITool* lefttool = m_pToolsMgr->GetLeftTool();
	ITool* righttool = m_pToolsMgr->GetRightTool();
	if (lefttool || righttool)
	{
		float toolspeed, leftdist, rightdist, lefttime, righttime;
		leftdist = m_pToolsMgr->GetLeftToolMovedDistance();
		rightdist = m_pToolsMgr->GetRightToolMovedDistance();
		lefttime = m_pToolsMgr->GetLeftToolMovedTime();
		righttime = m_pToolsMgr->GetRightToolMovedTime();
		toolspeed = (leftdist + rightdist) / (lefttime + righttime);
		if (toolspeed <= 5.0)
			COnLineGradeMgr::Instance()->SendGrade("MachineHandle_Normal", m_trainbegintime, usedtime);
		else if (toolspeed <= 10.0)
			COnLineGradeMgr::Instance()->SendGrade("MachineHandle_Fast", m_trainbegintime, usedtime);
		else
			COnLineGradeMgr::Instance()->SendGrade("MachineHandle_TooFast", m_trainbegintime, usedtime);
	}

	float trainetotaltime = usedtime - m_trainbegintime;
	if (m_numShowedVessel == 3)
	{
		if (trainetotaltime <= 180)
		{
			COnLineGradeMgr::Instance()->SendGrade("Finished_In3M", m_trainbegintime, usedtime);
			COnLineGradeMgr::Instance()->SendGrade("cooperateWell", m_trainbegintime, usedtime);
		}
		else if (trainetotaltime <= 300)
			COnLineGradeMgr::Instance()->SendGrade("Finished_In5M", m_trainbegintime, usedtime);
	}
	else
	{
		COnLineGradeMgr::Instance()->SendGrade("Finished_Out5M", m_trainbegintime, usedtime);
	}

	__super::OnSaveTrainingReport();
}