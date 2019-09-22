#include "SYSealAndCut.h"
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
#include "Instruments/MisCTool_PluginClamp.h"
#include <stack>
#include <vector>
#include "MisMedicOrganAttachment.h"
#include "CustomConstraint.h"
#include "Instruments/ElectricHook.h"
#include "Instruments/GraspingForceps.h"
#include "MisMedicObjectUnion.h"
#include "MXOgreGraphic.h"
#include "SYScoreTableManager.h"
void NewTrainingHandleEvent11(MxEvent * pEvent, ITraining * pTraining);

//0th as the first
static void FindKthNodeIndex(const std::vector<int> &sortedNodes , int kth , int needNum,std::vector<int> &results)
{
	int current_num = 0;
	for(size_t n = kth; n < sortedNodes.size();n++)
	{
		if(current_num < needNum)
		{		
			results.push_back(sortedNodes[n]);
			current_num++;
		}
		else 
			break;
	}
}

static void FindInverseKthIndex(const std::vector<int> &sortedNodes , int kth , int needNum,std::vector<int> &results)
{
	int current_num = 0;
	int begin = sortedNodes.size() - 1 - kth;
	for(int n = begin; n > 0 ;n--)
	{
		if(current_num < needNum)
		{		
			results.push_back(sortedNodes[n]);
			current_num++;
		}
		else 
			break;
	}
}

//面的一点在此范围内即可
static void CollectFacesWithV_V1(MisMedicOrgan_Ordinary *pOrgan , double minV ,double maxV , std::vector<GFPhysSoftBodyFace *> &results)
{
	results.clear();
	for(size_t f = 0 ; f < pOrgan->m_OriginFaces.size() ; f++)
	{
		MMO_Face &face = pOrgan->m_OriginFaces[f];
		for(int k = 0; k < 3; k++)
		{
			Ogre::Vector2 vertTexCoord = face.GetTextureCoord(k);
			if (vertTexCoord.y > minV && vertTexCoord.y < maxV)
			{
				results.push_back(face.m_physface);
				break;
			}
		}
	}
}

//面的三点在此范围内才符合
static void CollectFacesWithV_V2(MisMedicOrgan_Ordinary *pOrgan , double minV ,double maxV , std::vector<GFPhysSoftBodyFace *> &results)
{
	results.clear();
	for(size_t f = 0 ; f < pOrgan->m_OriginFaces.size() ; f++)
	{
		MMO_Face &face = pOrgan->m_OriginFaces[f];
		int num = 0;
		for(int k = 0; k < 3; k++)
		{
			Ogre::Vector2 vertTexCoord = face.GetTextureCoord(k);
			if (vertTexCoord.y > minV && vertTexCoord.y < maxV)
				num++;
		}
		if(num == 3)
			results.push_back(face.m_physface);
	}
}

int SYSealAndCut::ObjectOperationInfo::time = 0;

SYSealAndCut::SYSealAndCut(void)
{
	m_bRunning = false;
	m_bFinish = false;
	m_bPerfectFinish = true;
	m_bAllFail = false;
	m_pFlesh = NULL;
	m_pFleshObjectInfo = NULL;
	m_bBreakVessel = false;
	//TitanicClipInfo::s_clipEmptyCount = 0;
}

SYSealAndCut::~SYSealAndCut(void)
{
	for(std::map<MisMedicOrgan_Ordinary*,ObjectOperationInfo*>::iterator itr = m_organToObjectInfoMap.begin();itr != m_organToObjectInfoMap.end();++itr)
	{
		delete itr->second;
	}
	m_organToObjectInfoMap.clear();
	//m_NodesBeGrasped.clear();

	for(size_t c = 0 ; c < m_constraints.size(); c++)
	{
		if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
			PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(m_constraints[c]);
		delete m_constraints[c];
	}
	m_constraints.clear();


	ITool * pLeftTool,*pRightTool;
	pLeftTool = m_pToolsMgr->GetLeftTool();
	pRightTool = m_pToolsMgr->GetRightTool();
	if(pLeftTool && pLeftTool->GetType() == TT_GRASPING_FORCEPS)
	{
		CGraspingForceps * pTool = (CGraspingForceps*)pLeftTool;
		pTool->ReleaseClampedOrgans();
	}
	if(pRightTool && pRightTool->GetType() == TT_GRASPING_FORCEPS)
	{
		CGraspingForceps * pTool = (CGraspingForceps*)pRightTool;
		pTool->ReleaseClampedOrgans();
	}
	
	m_manual->detachFromParent();
	MXOgreWrapper::Get()->GetDefaultSceneManger()->destroyManualObject(m_manual);
}

//======================================================================================================================
bool SYSealAndCut::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{
	bool result = MisNewTraining::Initialize(pTrainingConfig , pToolConfig);

	Ogre::SceneManager * pSMG =  MXOgre_SCENEMANAGER;
	CLightMgr::Instance()->GetLight()->setPosition(Ogre::Vector3(0.0f, 0.0f, 0.40f));
	CLightMgr::Instance()->GetLight()->setDirection(Ogre::Vector3(0.0f, 0, -1));
		
	m_manual =MXOgreWrapper::Get()->GetDefaultSceneManger()->createManualObject();
	MXOgreWrapper::Get()->GetDefaultSceneManger()->getRootSceneNode()->attachObject(m_manual);
	Init();

	PhysicsWrapper::GetSingleTon().m_dynamicsWorld->SoftCollisionHashGridSize(0.8f);
	//register train event 
	CMXEventsDump::Instance()->RegisterHandleEventsFunc(NewTrainingHandleEvent11, this);

	return result;
}

void SYSealAndCut::AttachNodesCenterToBody(const std::vector<GFPhysSoftBodyNode*> & nodes , GFPhysSoftBody * attacbody)
{
	GFPhysVector3 centerPos(0,0,0);

	for(size_t n = 0 ; n < nodes.size(); n++)
	{
		GFPhysVector3 nodepos = nodes[n]->m_CurrPosition;

		float minDist = FLT_MAX;

		GFPhysSoftBodyTetrahedron * minTetra = 0;

		GFPhysVector3 closetPointInSoft;

		//GFPhysSoftBodyTetrahedron * tetra = attacbody->GetTetrahedronList();

		//while(tetra)
		for(size_t th = 0 ; th < attacbody->GetNumTetrahedron() ; th++)
		{
			GFPhysSoftBodyTetrahedron * tetra = attacbody->GetTetrahedronAtIndex(th);

			GFPhysVector3 closetPoint = ClosetPtPointTetrahedron(nodepos, 
				tetra->m_TetraNodes[0]->m_CurrPosition,
				tetra->m_TetraNodes[1]->m_CurrPosition,
				tetra->m_TetraNodes[2]->m_CurrPosition,
				tetra->m_TetraNodes[3]->m_CurrPosition);

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
			bool  gettedf = GetPointBarycentricCoordinate(minTetra->m_TetraNodes[0]->m_CurrPosition,
				minTetra->m_TetraNodes[1]->m_CurrPosition,
				minTetra->m_TetraNodes[2]->m_CurrPosition,
				minTetra->m_TetraNodes[3]->m_CurrPosition,
				closetPointInSoft,
				weights);
			if(gettedf)
			{
				TetrahedronAttachConstraint * cs = new TetrahedronAttachConstraint(nodes[n] , 
					minTetra,//->m_TetraNodes,
					weights);

				cs->SetStiffness(0.99f);
				PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(cs);
				m_constraints.push_back(cs);
			}
		}
	}
}

void SYSealAndCut::Init()
{
	MisMedicOrgan_Ordinary * pOrganFlesh = NULL;
	std::vector<MisMedicOrgan_Ordinary*> VesslesMidSize;

	DynObjMap::iterator organItr = m_DynObjMap.begin();
	while(organItr != m_DynObjMap.end())
	{
		MisMedicOrgan_Ordinary * organ = dynamic_cast<MisMedicOrgan_Ordinary*>(organItr->second);
		if(organ)
		{
			ObjectOperationInfo * pInfo = new ObjectOperationInfo;
			
			VesslesMidSize.push_back(organ);
			
			pInfo->state = ObjectOperationInfo::WaitOperate;
			
			if(organ->m_OrganID == 22 || organ->m_OrganID == 24)
				pInfo->type = ObjectOperationInfo::Small;
			else if(organ->m_OrganID == 23 || organ->m_OrganID == 26)
			{
				pInfo->type = ObjectOperationInfo::Ordinary;
				if(organ->m_OrganID == 26)
					pInfo->scale_factor = 2.22;
			}
			else if(organ->m_OrganID == 25)
			{
				pInfo->type = ObjectOperationInfo::Big;
				pInfo->scale_factor = 2.4;
			}
			else if(organ->m_OrganID == 27)
			{
				pInfo->type =  ObjectOperationInfo::Unknow;
				pInfo->state = ObjectOperationInfo::NotOperate;		//无需操作的对象
				m_pDizuo2 = organ;
				VesslesMidSize.erase(VesslesMidSize.begin() + VesslesMidSize.size() - 1);
			}
			else if(organ->m_OrganID == 28)
			{
				pInfo->type =  ObjectOperationInfo::Unknow;
				pInfo->state = ObjectOperationInfo::NotOperate;		//无需操作的对象
				VesslesMidSize.erase(VesslesMidSize.begin() + VesslesMidSize.size() - 1);
			}
			else
			{
				pInfo->state = ObjectOperationInfo::NotOperate;		//无需操作的对象
				pInfo->type = ObjectOperationInfo::Unknow;			//非血管--肉
				pOrganFlesh = organ;
				VesslesMidSize.erase(VesslesMidSize.begin() + VesslesMidSize.size() - 1);		//delete pDistOrgan
				m_pFlesh = organ;
				m_pFleshObjectInfo = pInfo;
			}
			pInfo->organ = organ;
			m_organToObjectInfoMap.insert(make_pair(organ,pInfo));

			//organ->m_MaxSoftNodeMove  = 0.4f;
			//organ->m_MaxSoftNodeSpeed = 12.0f;
			
			//find the connected node
			if(pInfo->type != ObjectOperationInfo::Unknow)
			{

				std::vector<int>& fixedPoints = organ->GetCreateInfo().m_FixPointsIndex;
				GFPhysSoftBodyNode * pNode = NULL;

				std::vector<int> sorted_node_indices;
				int total_num = 0;
				organ->GetSortedNodeIndex(sorted_node_indices,total_num);
				assert(sorted_node_indices.size() == total_num);
				pInfo->topNodeIndex = sorted_node_indices[total_num - 1];
				pInfo->bottomNodeIndex = sorted_node_indices[0];
				pInfo->centerNodeIndex = sorted_node_indices[(int)(0.5 * total_num)];

				pInfo->upperClipNodeIndex = sorted_node_indices[(int)(0.6875 * total_num)];
				pInfo->lowerClipNodeIndex = sorted_node_indices[(int)(0.3125 * total_num)];


				FindKthNodeIndex(sorted_node_indices,0,10,pInfo->bottomConnectedNodeIndices);
				FindInverseKthIndex(sorted_node_indices,0,25,pInfo->topConnectedNodeIndices);

				GFPhysSoftBodyNode *p_top_node = organ->m_physbody->GetNode(pInfo->topNodeIndex);
				GFPhysSoftBodyNode *p_bottom_node = organ->m_physbody->GetNode(pInfo->bottomNodeIndex);

				pInfo->init_length = p_top_node->m_CurrPosition.Distance(p_bottom_node->m_CurrPosition);
				
				if(pInfo->type != ObjectOperationInfo::Small)
				{
					TitanicClipInfo upper_clip;
					CollectFacesWithV_V2(organ,0.25,0.4,upper_clip.m_facesSatisfied);
					organ->m_titanicClipInfos.push_back(upper_clip);

					TitanicClipInfo lower_clip;
					CollectFacesWithV_V2(organ,0.62,0.75,lower_clip.m_facesSatisfied);
					organ->m_titanicClipInfos.push_back(lower_clip);

					//TestForUV(organ,0.25,0.4);
					//TestForUV(organ,0.62,0.75);
				}
				if(pInfo->type == ObjectOperationInfo::Small)
				{
					ElecCutInfo cut_info;
					CollectFacesWithV_V1(organ,0.4375,0.5625,cut_info.m_facesSatisfied);
					organ->m_elecCutInfos.push_back(cut_info);

					//TestForUV2(organ,0.4375,0.5625);
				}

			}
		}
		++organItr;
	}
	
	//血管上下表面和肉连接
	if (VesslesMidSize.size())
	{
		std::vector<GFPhysSoftBodyNode*> TopSurfNodes;

		std::vector<GFPhysSoftBodyNode*> BottomSurfNodes;

		for (std::size_t o = 0; o < VesslesMidSize.size(); ++o)
		{
			GFPhysSoftBodyNode * pNode = NULL;
			
			OrganToObjectInfoMap::const_iterator itor = m_organToObjectInfoMap.find(VesslesMidSize[o]);
			
			//connect top nodes
			if(itor != m_organToObjectInfoMap.end())
			{
				ObjectOperationInfo *p_info = itor->second;
				const std::vector<int> & top_nodes = p_info->topConnectedNodeIndices;
				for(size_t i = 0 ; i < top_nodes.size(); i++)
				{
					pNode = VesslesMidSize[o]->m_physbody->GetNode(top_nodes[i]);
					if(pNode)
					   TopSurfNodes.push_back(pNode);
				}
			}

			//connect bottom fixed nodes & make fixed nodes not fix
			std::vector<int> & fixedPoints = VesslesMidSize[o]->GetCreateInfo().m_FixPointsIndex;
			for (std::size_t p = 0; p < fixedPoints.size(); ++p)
			{
				GFPhysSoftBodyNode * pNode = VesslesMidSize[o]->m_physbody->GetNode(fixedPoints[p]);
				if (pNode)
				{
					float curY = pNode->m_CurrPosition.GetY();
					pNode->m_CurrPosition.SetY(curY + 1.15);
				}
				BottomSurfNodes.push_back(pNode);
				pNode->SetMass(0.1f);
			}
		}

		if (TopSurfNodes.size() && pOrganFlesh)
			AttachNodesCenterToBody(TopSurfNodes, pOrganFlesh->m_physbody);

		if (BottomSurfNodes.size() && m_pDizuo2)
			AttachNodesCenterToBody(BottomSurfNodes, m_pDizuo2->m_physbody);
	}

	//测量原始长度 -- 需要改进不要用固定索引
	for (std::size_t o = 0; o < VesslesMidSize.size(); ++o)
	{
		GFPhysSoftBodyNode * pTopNode = VesslesMidSize[o]->m_physbody->GetNode(20);	//
		
		GFPhysSoftBodyNode * pNode = VesslesMidSize[o]->m_physbody->GetNodeList();
		
		while(pNode)
		{
			float dis = pTopNode->m_CurrPosition.Distance(pNode->m_CurrPosition);
			
			PhysNode_Data & nodeData = VesslesMidSize[o]->GetPhysNodeData(pNode);
			
			if(nodeData.m_HasError == false)
			   nodeData.m_Dist = (int)(dis * 1000);
			
			pNode = pNode->m_Next;
		}

		//set bleed effect
		MisMedicOrgan_Ordinary *pOrgan = VesslesMidSize[o];
		if(pOrgan->m_OrganID == 22 || pOrgan->m_OrganID == 24) //small
			VesslesMidSize[o]->setVesselBleedEffectTempalteName(PT_BLEED_SMALLVESSEL);
		else if(pOrgan->m_OrganID == 23 || pOrgan->m_OrganID == 26)	
			VesslesMidSize[o]->setVesselBleedEffectTempalteName(PT_BLEED_02);
		else if(pOrgan->m_OrganID == 25)
			VesslesMidSize[o]->setVesselBleedEffectTempalteName(PT_BLEED_BIGVESSEL);

	}

}

void SYSealAndCut::InternalSimulateStart(int currStep , int TotalStep , Real dt)
{
	MisNewTraining::InternalSimulateStart( currStep ,  TotalStep ,  dt);

	/*m_NodesBeGrasped.clear();
	CTool * leftTool  = dynamic_cast<CTool*>(m_pToolsMgr->GetLeftTool());
	CTool * rightTool = dynamic_cast<CTool*>(m_pToolsMgr->GetRightTool());
	if(leftTool)
	{
		leftTool->GetSoftNodeBeingGrasped(m_NodesBeGrasped);
	}
	if(rightTool)
	{
		rightTool->GetSoftNodeBeingGrasped(m_NodesBeGrasped);
	}
	*/
}

GFPhysVector3 SYSealAndCut::GetTrainingForceFeedBack(ITool* tool)
{
	return GFPhysVector3(0,0,0);

	GoPhys::GFPhysVector3 result = GoPhys::GFPhysVector3(0,0,0);
	CTool* pTool = dynamic_cast<CTool*>(tool);

	GFPhysRigidBody * part[3];
	part[0] = pTool->m_lefttoolpartconvex.m_rigidbody;
	part[1]  = pTool->m_righttoolpartconvex.m_rigidbody;
	part[2]  = pTool->m_centertoolpartconvex.m_rigidbody;

	float minZ = 1000;
	for (int i = 0; i < 3; ++i)
	{
		if (part[i])
		{
			GFPhysVector3 aabbMin;
			GFPhysVector3 aabbMax;
			part[i]->GetAabb(aabbMin, aabbMax);
			float dirZ = aabbMin.GetZ();
			if(dirZ  < minZ)	
				minZ = dirZ;
			//dirZ = max(dirZ,-1.0f);
			if (minZ < 2.2)
			{
				result = GFPhysVector3(0,0, 1);
			}
		}
	}
	return result * 1.2f;

}

MisCTool_PluginClamp* SYSealAndCut::GetToolPlugin(CTool* tool)
{
	for(size_t p = 0 ; p <tool->m_ToolPlugins.size() ; p++)
	{
		MisCTool_PluginClamp * clampPlugin = dynamic_cast<MisCTool_PluginClamp*>(tool->m_ToolPlugins[p]);		
		if(clampPlugin && clampPlugin->isInClampState())
			return clampPlugin;
	}
	return NULL;
}

bool SYSealAndCut::Update( float dt )
{
	bool result = MisNewTraining::Update(dt);
	UpdateStateAndMaterial();
	if(m_bAllFail)
	{
		//CTipMgr::Instance()->ShowTip("AllFail");
	}
	else if(m_bPerfectFinish && m_bFinish)
	{
		//CTipMgr::Instance()->ShowTip("TrainingPerfectFinish");
	}
	else if(m_bFinish)
	{
		//CTipMgr::Instance()->ShowTip("TrainingFinish");
	}
	else
	{
		m_bAllFail = true;
		bool flag = true;
		for(OrganToObjectInfoMap::iterator itr = m_organToObjectInfoMap.begin();itr != m_organToObjectInfoMap.end();++itr)
		{
			if(itr->second->state == ObjectOperationInfo::WaitOperate)
			{
				m_bAllFail = false;
				flag = false;
				break;
			}
			else if (itr->second->state == ObjectOperationInfo::FailOperate 
				  || itr->second->state == ObjectOperationInfo::FailOperateNotClipBeforeCut
				  || itr->second->state == ObjectOperationInfo::FaileOperateCutNotBetweenClip)
			{
				m_bPerfectFinish = false;
			}
			else if(itr->second->state == ObjectOperationInfo::OkOperate)
			{
				m_bAllFail = false;
			}
		}
		m_bFinish = flag;

		if(m_bAllFail)
		{
			TrainingFatalError();
		}
		else if(m_bFinish)
		{
			TrainingFinish();
		}
	}

	CTool * pLeftTool,*pRightTool;
	pLeftTool = (CTool*)m_pToolsMgr->GetLeftTool();
	pRightTool = (CTool*)m_pToolsMgr->GetRightTool();
	
	if(pLeftTool)
	{
		if(pLeftTool->GetType() == TT_GRASPING_FORCEPS)
		{
			MisCTool_PluginClamp * toolPlugin = NULL;
			toolPlugin = GetToolPlugin(pLeftTool);
			if(toolPlugin)
			{
				//MisMedicOrgan_Ordinary * leftClampedObject = toolPlugin->GetOrganBeClamped();
				std::vector<MisMedicOrgan_Ordinary *> organsClamped;
				toolPlugin->GetOrgansBeClamped(organsClamped);
                if(organsClamped.size() > 0)
				   updateClampedObject(organsClamped[0],toolPlugin);
			}
		}
		else
		{
			MakeSmoke(pLeftTool);
		}
	}

	if(pRightTool)
	{
		if(pRightTool->GetType() == TT_GRASPING_FORCEPS)
		{
			MisCTool_PluginClamp * toolPlugin = NULL;
			toolPlugin = GetToolPlugin(pRightTool);
			if(toolPlugin)
			{
				//MisMedicOrgan_Ordinary * rightClampedObject = toolPlugin->GetOrganBeClamped();
				std::vector<MisMedicOrgan_Ordinary *> organsClamped;
				toolPlugin->GetOrgansBeClamped(organsClamped);
				if(organsClamped.size() > 0)
				   updateClampedObject(organsClamped[0],toolPlugin);
			}
		}
		else
		{
			MakeSmoke(pRightTool);
		}
	}
	//DrawDebugPoints();
	
	return result;
}

void SYSealAndCut::UpdateStateAndMaterial()
{
	bool is_show_raise_tip = true;
	bool is_remanent = false;
	bool is_show_clip_mark = false;
	bool is_show_cut_mark = false;
	bool is_one_being_operated = false;

	for(OrganToObjectInfoMap::iterator itor = m_organToObjectInfoMap.begin();itor != m_organToObjectInfoMap.end();++itor)
	{
		MisMedicOrgan_Ordinary *p_organ = itor->first;
		ObjectOperationInfo *p_info = itor->second;
		
		if(p_info->type == ObjectOperationInfo::Unknow)
			continue;

		if(p_info->state != ObjectOperationInfo::WaitOperate)
			continue;


		is_remanent = true;

		GFPhysSoftBodyNode *p_top_node = p_organ->m_physbody->GetNode(p_info->topNodeIndex);
		GFPhysSoftBodyNode *p_bottom_node = p_organ->m_physbody->GetNode(p_info->bottomNodeIndex);
		GFPhysSoftBodyNode *p_center_node = p_organ->m_physbody->GetNode(p_info->centerNodeIndex);

		double current_length = p_top_node->m_CurrPosition.Distance(p_bottom_node->m_CurrPosition);
		

		if((current_length / p_info->init_length) > p_info->scale_factor)
		{
			BreakVessel(p_organ,p_center_node);
			CTipMgr::Instance()->ShowTip("BreakVesse2");
			p_info->state = ObjectOperationInfo::FailOperate;
			p_info->m_HasBeTeared = true;
			SetMaterialWithoutMark(p_organ);
		}

		if(p_info->state != ObjectOperationInfo::WaitOperate)
			continue;

		//更新材质
		bool is_show_mark = false;

		if((current_length / p_info->init_length) > p_info->scale_factor_for_mark)
			is_show_mark = true;
		
		if(p_info->is_show_mark != is_show_mark)
		{
			p_info->is_show_mark = is_show_mark;
			ChangeMaterial(p_organ,p_info);
		}

//================= new add
		if(p_info->is_operating == true)
			is_one_being_operated = true;

		if(p_info->is_show_mark == true)
		{
			m_bRunning = true;
			if(p_info->type == ObjectOperationInfo::Small)
				is_show_cut_mark = true;
			else 
				is_show_clip_mark = true;

			is_show_raise_tip = false;
			p_info->m_HasMarkBeSeen = true;
		}
//=================
	}

	if(is_one_being_operated)
		return;
	else
	{
		if(is_show_clip_mark && is_show_cut_mark)
			CTipMgr::Instance()->ShowTip("ClipAndCutTip");
		else if(is_show_clip_mark)
			CTipMgr::Instance()->ShowTip("ClipTip");
		else if(is_show_cut_mark)
			CTipMgr::Instance()->ShowTip("CutTip");
		else if(is_show_raise_tip && is_remanent && m_bRunning)
				CTipMgr::Instance()->ShowTip("RaiseTip");
	}

}

void SYSealAndCut::updateClampedObject(MisMedicOrgan_Ordinary * pOrgan,MisCTool_PluginClamp * pPluginClamp)
{
	if(pOrgan == NULL || pOrgan == m_pDizuo2)
		return ;
	if(pOrgan == m_pFlesh)
	{
		for(OrganToObjectInfoMap::iterator itr = m_organToObjectInfoMap.begin();itr != m_organToObjectInfoMap.end();++itr)
		{
			MisMedicOrgan_Ordinary * organ = itr->first;

			if(organ != m_pFlesh && itr->second->state == ObjectOperationInfo::WaitOperate)
			{
				ObjectOperationInfo *p_info = itr->second;

				//GFPhysSoftBodyNode * pTopNode = organ->m_physbody->GetNode(itr->second->upNodeIndex);
				//GFPhysSoftBodyNode * pMidNode = organ->m_physbody->GetNode(itr->second->midNodeIndex);

				GFPhysSoftBodyNode *p_top_node = organ->m_physbody->GetNode(p_info->topNodeIndex);
				GFPhysSoftBodyNode *p_bottom_node = organ->m_physbody->GetNode(p_info->bottomNodeIndex);
				GFPhysSoftBodyNode *p_center_node = organ->m_physbody->GetNode(p_info->centerNodeIndex);

				double current_length = p_top_node->m_CurrPosition.Distance(p_bottom_node->m_CurrPosition);
				if((current_length / p_info->init_length) > p_info->scale_factor)
				{
					BreakVessel(itr->first,p_center_node);
					CTipMgr::Instance()->ShowTip("BreakVesse2");
					p_info->state = ObjectOperationInfo::FailOperate;
				}
/*
				
				PhysNode_Data & nodeData = organ->GetPhysNodeData(pMidNode);

				float initLength = ((int)nodeData.m_Dist) * 0.001f;
				
				float curTopLength = pTopNode->m_CurrPosition.Distance(pMidNode->m_CurrPosition);
				
				if(abs(curTopLength - initLength) > 1.0f)
				{
					BreakVessel(itr->first,pMidNode);
					CTipMgr::Instance()->ShowTip("BreakVesse2");
					itr->second->state = ObjectOperationInfo::FailOperate;
				}
	*/
			}
		}
	}
	else 
	{
		OrganToObjectInfoMap::iterator itr = m_organToObjectInfoMap.find(pOrgan);
		ObjectOperationInfo * pInfo = itr->second;
//		GFPhysSoftBodyNode * pTopNode = pOrgan->m_physbody->GetNode(pInfo->upNodeIndex);
		GFPhysSoftBodyNode * p_top_node = pOrgan->m_physbody->GetNode(pInfo->topNodeIndex);
		GFPhysSoftBodyNode * p_bottom_node = pOrgan->m_physbody->GetNode(pInfo->bottomNodeIndex);

		//std::vector<GFPhysSoftBodyFace*>& faceInClamp = pPluginClamp->m_SoftFaceInClamp;
		//for(std::set<GFPhysSoftBodyFace*>::iterator itrFace = faceInClamp.begin();itrFace != faceInClamp.end();++itrFace)
		for (int t = 0; t < (int)pPluginClamp->m_ClampedOrgans.size(); t++)
		{
			 MisCTool_PluginClamp::OrganBeClamped * organClamped = pPluginClamp->m_ClampedOrgans[t];

			 if (organClamped->m_organ == pOrgan)
			 {
				 for (size_t c = 0; c < organClamped->m_ClampedFaces.size(); c++)
				 {
					 GFPhysSoftBodyFace * clampedFace = organClamped->m_ClampedFaces[c].m_PhysFace;

					 GFPhysSoftBodyNode * pNode = clampedFace->m_Nodes[0];

					 double current_length = p_top_node->m_CurrPosition.Distance(pNode->m_CurrPosition) + p_bottom_node->m_CurrPosition.Distance(pNode->m_CurrPosition);

					 if ((current_length / pInfo->init_length) > pInfo->scale_factor)
					 {
						 BreakVessel(pOrgan, pNode);
						 CTipMgr::Instance()->ShowTip("BreakVessel");
						 pInfo->state = ObjectOperationInfo::FailOperate;
						 break;
					 }
				 }
			 }
		}
	}
}

void SYSealAndCut::BreakVessel(MisMedicOrgan_Ordinary * pOrgan,GFPhysSoftBodyNode * attachedNode)
{
	std::set<GFPhysSoftBodyTetrahedron*> setTetras;
	//GFPhysSoftBodyTetrahedron * tetra = pOrgan->m_physbody->GetTetrahedronList();
	GFPhysVectorObj<GFPhysSoftBodyTetrahedron*> vecTearOffTetra;
	//while(tetra)
	for(size_t th = 0 ; th < pOrgan->m_physbody->GetNumTetrahedron() ; th++)
	{
		GFPhysSoftBodyTetrahedron * tetra = pOrgan->m_physbody->GetTetrahedronAtIndex(th);

		/*for (int i = 0; i < 4; ++i)
		{
			if(tetra->m_TetraNodes[i] == attachedNode)
			{
				vecTearOffTetra.push_back(tetra);
				/ *GFPhysVectorObj<GFPhysSoftBodyTetrahedron*> tempResult;
				BFSTearOffPointTetrahedron(pOrgan,tetra,4,tempResult);
				for (int p = 0; p != tempResult.size(); ++p)
				{
					setTetras.insert(tempResult[p]);
				}* /
				break;
			}
		}*/
		bool flag = true;
		for(int i = 0;i<4;++i)
		{
			if(tetra->m_TetraNodes[i]->m_CurrPosition.Distance(attachedNode->m_CurrPosition) > 0.50f)
			{
				flag = false;
				break;
			}
		}
		if(flag)
			vecTearOffTetra.push_back(tetra);
		//tetra = tetra->m_Next;
	}
	/*if (setTetras.empty())
	{
		return;
	}*/
	for (std::set<GFPhysSoftBodyTetrahedron*>::iterator iter = setTetras.begin(); iter != setTetras.end(); ++iter)
	{
		vecTearOffTetra.push_back(*iter);
	}
	pOrgan->EliminateTetras(vecTearOffTetra);

	m_bBreakVessel = true;
}

void SYSealAndCut::BFSTearOffPointTetrahedron(MisMedicOrgan_Ordinary * pOrgan,GFPhysSoftBodyTetrahedron* tetra, int level, GFPhysVectorObj<GFPhysSoftBodyTetrahedron*>& result)
{
	if (tetra == NULL || level <= 0)
	{
		return;
	}
	std::stack<std::pair<GFPhysSoftBodyTetrahedron*, int>> QueueTetras;
	QueueTetras.push(std::pair<GFPhysSoftBodyTetrahedron*, int>(tetra, 0));

	//遍历血管的所有四面体 并将其标记为0,0表示为没有被访问过
	//GFPhysSoftBodyTetrahedron * headtetra = pOrgan->m_physbody->GetTetrahedronList();
	//while(headtetra)
	for(size_t th = 0 ; th < pOrgan->m_physbody->GetNumTetrahedron() ; th++)
	{
		GFPhysSoftBodyTetrahedron * headtetra = pOrgan->m_physbody->GetTetrahedronAtIndex(th);
		headtetra->m_TempData = (void*)0;
		//headtetra = headtetra->m_Next;
	}

	while(QueueTetras.size() > 0)
	{	
		GFPhysSoftBodyTetrahedron * QTetra = QueueTetras.top().first;
		int nlevel = QueueTetras.top().second;
		QueueTetras.pop();
		result.push_back(QTetra);
		if (nlevel >= level)
		{
			return;
		}
		for(int nb = 0 ; nb < 4 ; nb++)
		{
			GFPhysGeneralizedFace * genFace = QTetra->m_TetraFaces[nb];

			if(genFace && genFace->m_ShareTetrahedrons.size() > 1)
			{
				GFPhysSoftBodyTetrahedron * t0 = genFace->m_ShareTetrahedrons[0].m_Hosttetra;

				GFPhysSoftBodyTetrahedron * t1 = genFace->m_ShareTetrahedrons[1].m_Hosttetra;

				GFPhysSoftBodyTetrahedron * NBTetra = (t0 == QTetra ? t1 : t0);

				if (NBTetra->m_TempData == (void*)0)
				{
					NBTetra->m_TempData = (void*)1;
					QueueTetras.push(std::pair<GFPhysSoftBodyTetrahedron*, int>(NBTetra, nlevel+1));
				}
			}
		}
	}
}

void SYSealAndCut::MakeSmoke(CTool * pTool)
{
	if(!pTool)
		return;
	CElectricHook * pElecHook = dynamic_cast<CElectricHook*>(pTool);
	if(pElecHook)
	{
		std::vector<Ogre::Vector3> burnPos;

		if(pElecHook->GetToolSide() == ITool::TSD_LEFT)
		{
			const GFPhysSoftBodyFace * pFace = pElecHook->GetDistCollideFace();
			if((pElecHook->GetElectricLeftPad()/* || pElecHook->GetElectricRightPad()*/) && pFace)
			{
				for(std::size_t n = 0;n<3;n++)
				{
					const GFPhysSoftBodyNode * pNode = pFace->m_Nodes[n];
					Ogre::Vector3 pos(pNode->m_CurrPosition.GetX(),pNode->m_CurrPosition.GetY(),pNode->m_CurrPosition.GetZ());
					burnPos.push_back(pos);
				}
			}
		}
		else
		{
			const GFPhysSoftBodyFace * pFace = pElecHook->GetDistCollideFace();
			if((pElecHook->GetElectricLeftPad() /* || pElecHook->GetElectricRightPad()*/)  && pFace)
			{
				for(std::size_t n = 0;n<3;n++)
				{
					const GFPhysSoftBodyNode * pNode = pFace->m_Nodes[n];
					Ogre::Vector3 pos(pNode->m_CurrPosition.GetX(),pNode->m_CurrPosition.GetY(),pNode->m_CurrPosition.GetZ());
					burnPos.push_back(pos);
				}
			}
		}
		pElecHook->OnVeinConnectBurned(burnPos);
	}
}

void SYSealAndCut::DealWithElcCutObject(MisMedicOrgan_Ordinary * pOrgan)
{
	if(pOrgan == NULL)
	   return ;

	OrganToObjectInfoMap::iterator itr = m_organToObjectInfoMap.find(pOrgan);
	if(itr == m_organToObjectInfoMap.end())
	   return ;

	ObjectOperationInfo * pObjectInfo = itr->second;
	if(pObjectInfo->state ==  ObjectOperationInfo::WaitOperate)
	{
		//记录一次操作
		m_eachtime_organ_id.push_back(pOrgan->m_OrganID);

		std::vector<Ogre::Vector2> reginInfo;
		pOrgan->TestLinkingArea(0,reginInfo);
		if(reginInfo.size() > 1)
		{
			/*if(pObjectInfo->type == ObjectOperationInfo::Small)		//电切成功
			{
				pOrgan->setVesselBleedEffectTempalteName("Effect/BleedAfterSmallVesselElecCut");
				pOrgan->stopVesselBleedEffect();
				if(ElecCutInfo::s_cutSucceed == true)
				{				
					pObjectInfo->state = ObjectOperationInfo::OkOperate;
					CTipMgr::Instance()->ShowTip("ElectricCutOk");
					CScoreMgr::Instance()->Grade("scoreSmall");			//电段得分
				}
				else	
				{
					pObjectInfo->state = ObjectOperationInfo::FailOperate;
					CTipMgr::Instance()->ShowTip("ElectricCutInWrongPos");
				}


			}
			else*/
			{
				if (pObjectInfo->validClipNum < 2)//两端并没被夹闭
				{
					pOrgan->setVesselBleedEffectTempalteName("Effect/BleedAfterBigVesselElecCut");
					pOrgan->stopVesselBleedEffect();
					
					pObjectInfo->state = ObjectOperationInfo::FailOperate;
					pObjectInfo->m_CutInfo = ObjectOperationInfo::NotClippedBeforeCut;
					
					CTipMgr::Instance()->ShowTip("ScissorCutFail1");
				}
				else 
				{
					bool isCutBetweenClip = CheckCutBetweenClip(pOrgan);
					if (isCutBetweenClip)
					{
						pOrgan->setVesselBleedEffectTempalteName(PT_BLEED_SMALLVESSEL);
						pOrgan->stopVesselBleedEffect();

						pObjectInfo->state = ObjectOperationInfo::OkOperate;
						CTipMgr::Instance()->ShowTip("ElectricCutOk");
						//CScoreMgr::Instance()->Grade("scoreCut");				//剪断得分
						
						SetMaterialWithoutMark(pOrgan);

						pObjectInfo->operate_indices.push_back(m_eachtime_organ_id.size() - 1);
						//if(CheckIfConsistent(pObjectInfo))						//连贯动作得分
						//   CScoreMgr::Instance()->Grade("scoreSequence");

						pObjectInfo->m_CutInfo = ObjectOperationInfo::CutGood;
					}
					else 
					{
						pOrgan->setVesselBleedEffectTempalteName("Effect/BleedAfterBigVesselElecCut");
						pOrgan->stopVesselBleedEffect();
						
						pObjectInfo->state = ObjectOperationInfo::FailOperate;
						CTipMgr::Instance()->ShowTip("ElectricCutFail");

						pObjectInfo->m_CutInfo = ObjectOperationInfo::CutInOneSideOfClip;
					}
				}
			}
			SetMaterialWithoutMark(pOrgan);
		}
	}
}

void SYSealAndCut::DealWithScissorCutObject(MisMedicOrgan_Ordinary * pOrgan)
{
	if(pOrgan == NULL)
		return;
	OrganToObjectInfoMap::iterator itr = m_organToObjectInfoMap.find(pOrgan);
	if(itr == m_organToObjectInfoMap.end())
		return ;

	ObjectOperationInfo * pObjectInfo = itr->second;
	if(pObjectInfo->state == ObjectOperationInfo::WaitOperate)
	{
		//记录一次操作
		m_eachtime_organ_id.push_back(pOrgan->m_OrganID);

		if(pObjectInfo->type == ObjectOperationInfo::Small)
		{
			pObjectInfo->state = ObjectOperationInfo::FailOperate;
			CTipMgr::Instance()->ShowTip("ScissorCutFail");

			SetMaterialWithoutMark(pOrgan);
		}
		else
		{
			std::vector<Ogre::Vector2> reginInfo;
			
			pOrgan->TestLinkingArea(0, reginInfo);


			if (reginInfo.size() > 1)
			{
				pObjectInfo->m_IsCutOffed = true;
			}
			int numAttachment = pOrgan->GetAttachmentCount(MOAType_TiantumClip);
			if(numAttachment < 2)
			{
				pObjectInfo->state = ObjectOperationInfo::FailOperateNotClipBeforeCut;//未夹闭再剪断
				CTipMgr::Instance()->ShowTip("ScissorCutFail1");
			}
			else
			{
				vector<GFPhysSoftBodyFace*> newCrossFaces;
				
				pOrgan->GetLastTimeCutCrossFaces(newCrossFaces);

				if(newCrossFaces.size())
				{
					float cutY,minAttachmentY,maxAttachmentY;
					cutY = newCrossFaces[0]->m_Nodes[0]->m_CurrPosition.GetY();
					minAttachmentY = maxAttachmentY = 0.0f;

					std::size_t attachmentIndex = 0;
					bool bFirst = true;
					
					//计算钛架的最小和最大Y坐标
					while(attachmentIndex < pOrgan->m_OrganAttachments.size())
					{
						if(pOrgan->m_OrganAttachments[attachmentIndex]->m_type == MOAType_TiantumClip)
						{
							MisMedicTitaniumClampV2 * attachment = static_cast<MisMedicTitaniumClampV2*>(pOrgan->m_OrganAttachments[attachmentIndex]);
							
							GFPhysSoftBodyFace * attachFace = attachment->getRelativeFace();

							if (attachFace)//some clip may be detached from face when on cut see "MisMedicTitaniumClampV2::RefindAttachFace"
							{
								float y = attachment->getRelativeFace()->m_Nodes[0]->m_CurrPosition.GetY();
								if (bFirst)
								{
									minAttachmentY = maxAttachmentY = y;
									bFirst = false;
								}
								else
								{
									if (y > maxAttachmentY)
										maxAttachmentY = y;
									else if (y < minAttachmentY)
										minAttachmentY = y;
								}
							}
						}
						++attachmentIndex;
					}
					if(bFirst)
						return;

					//检查面是否在两个钛架之间
					if(cutY > minAttachmentY && cutY < maxAttachmentY)
					{
						if(reginInfo.size() > 1)//血管分成了两部分
						{
							SetMaterialWithoutMark(pOrgan);
							pObjectInfo->state = ObjectOperationInfo::OkOperate;
							pObjectInfo->is_operating = false;
							
							CTipMgr::Instance()->ShowTip("ScissorCutOk");
							
							//CScoreMgr::Instance()->Grade("scoreCut");				//剪断得分
							
							
							//
							//int index = pObjectInfo->clipNum;
							//int vaild_index = pObjectInfo->validClipNum;
							//if(vaild_index !=2 )
							//	throw "cut operate error!";
							//valid is replace by valid_index
							//pObjectInfo->operateTime[vaild_index] = ObjectOperationInfo::time++;
							//++pObjectInfo->clipNum;

							pObjectInfo->operate_indices.push_back(m_eachtime_organ_id.size() - 1);
							
							//if(pObjectInfo->operateTime[1] - pObjectInfo->operateTime[0] == 1 &&
							//	pObjectInfo->operateTime[2] - pObjectInfo->operateTime[1] == 1)
							//	CScoreMgr::Instance()->Grade("scoreSequence");		//连贯动作得分

							if (CheckIfConsistent(pObjectInfo))
							{//连贯动作得分
								//CScoreMgr::Instance()->Grade("scoreSequence");
							}
							pOrgan->setVesselBleedEffectTempalteName(PT_BLEED_SMALLVESSEL);
							pOrgan->stopVesselBleedEffect();

						}
						else
						{		
							CTipMgr::Instance()->ShowTip("ScissorCutContinue");
						}
					}
					else
					{
						pObjectInfo->state = ObjectOperationInfo::FaileOperateCutNotBetweenClip;
						pObjectInfo->is_operating = false;
						
						CTipMgr::Instance()->ShowTip("ScissorCutFail2");
						SetMaterialWithoutMark(pOrgan);
					}
				}
			}
		}
	}
}

void SYSealAndCut::DealWithClipObject(MisMedicOrgan_Ordinary * pOrgan)
{
	if (pOrgan == NULL)
	{
		TitanicClipInfo::s_clipEmptyCount++;
		return;
	}
		
	OrganToObjectInfoMap::iterator itr = m_organToObjectInfoMap.find(pOrgan);
	if(itr == m_organToObjectInfoMap.end())
	   return;
	ObjectOperationInfo * pObjectInfo = itr->second;
	if(pObjectInfo->state == ObjectOperationInfo::WaitOperate)
	{
		//记录此次操作的器官ID
		m_eachtime_organ_id.push_back(pOrgan->m_OrganID);

		if( pObjectInfo->type != ObjectOperationInfo::Small)
		{
			pObjectInfo->is_operating = true;

			pObjectInfo->operate_indices.push_back(m_eachtime_organ_id.size() - 1);
			
			pObjectInfo->clipNum++;

			//int lastValidClipNum = pObjectInfo->validClipNum;
			
			//计算两端是否上了钛架
			pObjectInfo->numClipPartFin = 0;
			for (int c = 0; c < pOrgan->m_titanicClipInfos.size(); c++)
			{
				if (pOrgan->m_titanicClipInfos[c].m_IsClip)
				{
					pObjectInfo->numClipPartFin++;
				}
			}

			if (TitanicClipInfo::s_clipInValidReg)//本次夹子在任意一段的有效区域内
			{
				pObjectInfo->validClipNum++;

				if (pObjectInfo->numClipPartFin == 1)
				{
					CTipMgr::Instance()->ShowTip("Clip1");
				}
				else if (pObjectInfo->numClipPartFin == 2)
				{
					CTipMgr::Instance()->ShowTip("Clip2");
				}
			}
			else//夹在无效范围内
			{
				pObjectInfo->invalidClipNum++;
				CTipMgr::Instance()->ShowTip("ClipFail");
			}

			if (pObjectInfo->is_show_mark == false)
			{
				pObjectInfo->m_IsMarkAlwaysBeSeen = false;//操作过程中标记不处于暴露状态
			}
		}
		else
		{
			CTipMgr::Instance()->ShowTip("ClipError");
		}
	}
}

void SYSealAndCut::DrawDebugPoints()
{
	int offset = 0;
	m_manual->setDynamic(true);
	m_manual->clear();
	m_manual->begin("VeinEditor/Cube");
	OrganToObjectInfoMap::const_iterator itor = m_organToObjectInfoMap.begin();
	for(; itor != m_organToObjectInfoMap.end(); ++itor)
	{
		MisMedicOrgan_Ordinary *p_organ = itor->first;
		ObjectOperationInfo *p_info = itor->second;
		if(p_info->type !=  ObjectOperationInfo::Unknow)
		{
			int top_node_index = p_info->topNodeIndex;
			int center_node_index = p_info->centerNodeIndex;
			int bottom_node_index = p_info->bottomNodeIndex;

			int upperIndex = p_info->upperClipNodeIndex;
			int lowerIndex = p_info->lowerClipNodeIndex;

			DrawOnePoint(p_organ->m_physbody->GetNode(top_node_index)->m_CurrPosition,Ogre::ColourValue::Red,0.05,offset);
			DrawOnePoint(p_organ->m_physbody->GetNode(center_node_index)->m_CurrPosition,Ogre::ColourValue::Green,0.05,offset);
			DrawOnePoint(p_organ->m_physbody->GetNode(bottom_node_index)->m_CurrPosition,Ogre::ColourValue::Blue,0.05,offset);
			DrawOnePoint(p_organ->m_physbody->GetNode(upperIndex)->m_CurrPosition,Ogre::ColourValue(1,1,0,1),0.05,offset);
			DrawOnePoint(p_organ->m_physbody->GetNode(lowerIndex)->m_CurrPosition,Ogre::ColourValue(0,1,1,1),0.05,offset);
/*
			const std::vector<int> & bottom_nodes = p_info->bottomConnectedNodeIndices;
			for(int i = 0 ; i < bottom_nodes.size() ; i++)
				DrawOnePoint(p_organ->m_physbody->GetNode(bottom_nodes[i])->m_CurrPosition,Ogre::ColourValue::White,0.05,offset);	

			const std::vector<int> & top_nodes = p_info->topConnectedNodeIndices;
			for(int i = 0 ; i < top_nodes.size() ; i++)
				DrawOnePoint(p_organ->m_physbody->GetNode(top_nodes[i])->m_CurrPosition,Ogre::ColourValue::Black,0.05,offset);	
*/
			}
	}
	m_manual->end();

	//draw face
	if(m_test_faces.size() == 0)
		return;
	m_manual->begin("BaseWhiteNoLighting");
	for(int f = 0; f< m_test_faces.size();f++)
	{
		GFPhysSoftBodyFace * p_face = m_test_faces[f];
		if(p_face)
		{
			m_manual->position(GPVec3ToOgre(p_face->m_Nodes[0]->m_CurrPosition));
			m_manual->colour(1,1,1,1);
			m_manual->position(GPVec3ToOgre(p_face->m_Nodes[1]->m_CurrPosition));
			m_manual->colour(1,1,1,1);
			m_manual->position(GPVec3ToOgre(p_face->m_Nodes[2]->m_CurrPosition));
			m_manual->colour(1,1,1,1);
		}
	}
	m_manual->end();
}

void SYSealAndCut::DrawOnePoint(const GFPhysVector3& gvposition , Ogre::ColourValue color , float size , int &offset)
{
	Ogre::Vector3 position = GPVec3ToOgre(gvposition);
		
	m_manual->position( position + Ogre::Vector3(-size , -size , -size) );   //0
	m_manual->colour(color);

	m_manual->position( position + Ogre::Vector3(size , -size , -size) );    //1
	m_manual->colour(color);

	m_manual->position( position + Ogre::Vector3(size , -size , size) );    //2
	m_manual->colour(color);

	m_manual->position( position + Ogre::Vector3(-size , -size , size) );    //3
	m_manual->colour(color);

	m_manual->position( position + Ogre::Vector3(-size , size , -size) );    //4
	m_manual->colour(color);

	m_manual->position( position + Ogre::Vector3(size , size , -size) );    //5
	m_manual->colour(color);

	m_manual->position( position + Ogre::Vector3(size , size , size) );    //6
	m_manual->colour(color);

	m_manual->position( position + Ogre::Vector3(-size , size , size));    //7
	m_manual->colour(color);

	//index
	m_manual->triangle(0 + offset, 2 + offset, 1 + offset);
	m_manual->triangle(0 + offset, 2+ offset, 3+ offset);
	m_manual->triangle(3+ offset, 4+ offset, 0+ offset);
	m_manual->triangle(3+ offset, 7+ offset, 4+ offset);
	m_manual->triangle(4+ offset, 7+ offset, 6+ offset);
	m_manual->triangle(4+ offset, 6+ offset, 5+ offset);
	m_manual->triangle(5+ offset, 2+ offset, 1+ offset);
	m_manual->triangle(5+ offset, 6+ offset, 2+ offset);
	m_manual->triangle(0+ offset, 4+ offset, 1+ offset);
	m_manual->triangle(5+ offset, 1+ offset, 4+ offset);
	m_manual->triangle(3+ offset, 6+ offset, 7+ offset);
	m_manual->triangle(3+ offset, 2+ offset, 6+ offset);

	offset += 8;

}


bool SYSealAndCut::CheckIfConsistent(ObjectOperationInfo *pInfo)
{
	std::vector<int> & indices = pInfo->operate_indices;
	const std::vector<int> & organ_ids = m_eachtime_organ_id;
	assert(indices.size() >= 3);
	
	int szIndiecs = indices.size();
	for (int i = 0; i < szIndiecs - 1; i++)
	{
		int begin_index = indices[i];
		int end_index = indices[i+1];
		int organ_id = organ_ids[begin_index];
		for(int current_index = begin_index + 1; current_index <= end_index;current_index++)
		{
			if(organ_ids[current_index] != organ_id)
				return false;
		}
	}
	return true;

}

void SYSealAndCut::TestForUV(MisMedicOrgan_Ordinary *pOrgan ,double minV , double maxV)
{
	//std::ofstream outfile("d:\\testforuv.txt" , ios::out | ios::app );
	//outfile << minV << "," << maxV << "\n";
	for(size_t f = 0 ; f < pOrgan->m_OriginFaces.size() ; f++)
	{
		MMO_Face &face = pOrgan->m_OriginFaces[f];
		int num = 0;
		for(int k = 0; k < 3; k++)
		{	
			Ogre::Vector2 vertTexCoord = face.GetTextureCoord(k);
			if (vertTexCoord.y >= minV && vertTexCoord.y <= maxV)
			{
				num++;
			}
		}
		if(num == 3)
		{
			//outfile << face.m_TextureCoord[k].y << "\n";
			m_test_faces.push_back(face.m_physface);
		}
	}
	//outfile << "end\n";
	//outfile.close();
}

void SYSealAndCut::TestForUV2(MisMedicOrgan_Ordinary *pOrgan ,double minV , double maxV)
{
	for(size_t f = 0 ; f < pOrgan->m_OriginFaces.size() ; f++)
	{
		MMO_Face &face = pOrgan->m_OriginFaces[f];
		for(int k = 0; k < 3; k++)
		{	
			Ogre::Vector2 vertTexCoord = face.GetTextureCoord(k);
			if (vertTexCoord.y >= minV && vertTexCoord.y <= maxV)
			{
				m_test_faces.push_back(face.m_physface);
				break;
			}
		}
	}

}

void SYSealAndCut::SetMaterialWithoutMark(MisMedicOrgan_Ordinary *pOrgan)
{
	pOrgan->setMaterialName("NewTraining11/VesselWithoutMark");
}

void SYSealAndCut::ChangeMaterial(MisMedicOrgan_Ordinary *pOrgan , ObjectOperationInfo * pObjectInfo)
{
	if(pObjectInfo->is_show_mark)
	{
		if(pObjectInfo->type == ObjectOperationInfo::Small)
			pOrgan->SetOrdinaryMatrial("NewTraining11/VesselWithoutClip");
		else
			pOrgan->SetOrdinaryMatrial("NewTraining11/VesselWithClip");
	}
	else
		pOrgan->SetOrdinaryMatrial("NewTraining11/VesselWithoutMark");
}
void SYSealAndCut::GetTiantumClipRange(MisMedicOrgan_Ordinary *pOrgan , float &miny , float &maxy)
{
	int num_titanium_attachment = pOrgan->GetAttachmentCount(MOAType_TiantumClip);

	bool is_first = true;
	//float miny = 0, maxy = 0;
	for(int t = 0 ; t < pOrgan->m_OrganAttachments.size() ; t++)
	{
		if(pOrgan->m_OrganAttachments[t]->m_type == MOAType_TiantumClip)
		{
			MisMedicTitaniumClampV2 * attachment = static_cast<MisMedicTitaniumClampV2*>(pOrgan->m_OrganAttachments[t]);
			float y = attachment->getRelativeFace()->m_Nodes[0]->m_CurrPosition.GetY();
			if(is_first)
			{
				miny = miny = y;
				is_first = false;
			}
			else
			{
				if(y > maxy)
					maxy = y;
				if(y < miny)
					miny = y;
			}
		}
	}
}

bool SYSealAndCut::CheckCutBetweenClip(MisMedicOrgan_Ordinary *pOrgan)
{
	float miny = -FLT_MAX;
	float maxy = FLT_MAX;
	GetTiantumClipRange(pOrgan,miny,maxy);
	GFPhysSoftBodyFace *p_cut_face = pOrgan->m_lastEleCutFace;
	for(int n = 0 ; n < 3 ; n++)
	{
		//GFPhysSoftBodyNode *p_node;
		float y = p_cut_face->m_Nodes[n]->m_CurrPosition.m_y;
		if(y < maxy && y > miny)
			return true;
	}
	return false;
}

void SYSealAndCut::OnSaveTrainingReport()
{
	/*
	if (!m_bTrainingIlluminated)
		return;

	int clipTotalNum = 0;
	int clipValidNum = 0;
	int scissorCutFinish = 0;
	int scissorCutWait = 0;

	int elcCutFinish = 0;
	int elcCutWait = 0;
	
	int opConsistent = 0;

	DynObjMap::iterator organItr = m_DynObjMap.begin();
	while (organItr != m_DynObjMap.end())
	{
		MisMedicOrgan_Ordinary * organ = dynamic_cast<MisMedicOrgan_Ordinary*>(organItr->second);
		if (organ)
		{
			OrganToObjectInfoMap::iterator itr = m_organToObjectInfoMap.find(organ);
			if (itr == m_organToObjectInfoMap.end())
				continue;

			ObjectOperationInfo * pObjectInfo = itr->second;

			if (organ->m_OrganID == 22 || organ->m_OrganID == 24)
			{
				//pInfo->type = ObjectOperationInfo::Small;
				if (pObjectInfo->state == ObjectOperationInfo::State::OkOperate)
				{
					elcCutFinish++;
					if (CheckIfConsistent(pObjectInfo))
						opConsistent++;
				}

				else if (pObjectInfo->state == ObjectOperationInfo::State::WaitOperate)
				{
					elcCutWait++;
				}
			}
				
			else if (organ->m_OrganID == 23 || organ->m_OrganID == 26)
			{
				//pInfo->type = ObjectOperationInfo::Ordinary;
				clipTotalNum += pObjectInfo->clipNum;
				clipValidNum += pObjectInfo->validClipNum;
				
				if (pObjectInfo->state == ObjectOperationInfo::State::OkOperate)
				{
					scissorCutFinish++;
					if (CheckIfConsistent(pObjectInfo))
						opConsistent++;
				}

				else if (pObjectInfo->state == ObjectOperationInfo::State::WaitOperate)
				{
					scissorCutWait++;
				}
				
				
			}

			else if (organ->m_OrganID == 25)
			{
				//pInfo->type = ObjectOperationInfo::Big;
				
				clipTotalNum += pObjectInfo->clipNum;
				clipValidNum += pObjectInfo->validClipNum;
				
				if (pObjectInfo->state == ObjectOperationInfo::State::OkOperate)
				{
					scissorCutFinish++;
					if (CheckIfConsistent(pObjectInfo))
						opConsistent++;

				}
				else if (pObjectInfo->state == ObjectOperationInfo::State::WaitOperate)
				{
					scissorCutWait++;
				}
				
			}

		}

		organItr++;
	}

	clipTotalNum += TitanicClipInfo::s_clipEmptyCount;

	COnLineGradeMgr* onLineGradeMgr = COnLineGradeMgr::Instance();

	onLineGradeMgr->SendGrade("ClipCut_" + Ogre::StringConverter::toString(scissorCutFinish));

	if (clipValidNum >= 6)
		onLineGradeMgr->SendGrade("ClipNumValid_6");
	else if (clipValidNum >= 3)
		onLineGradeMgr->SendGrade("ClipNumValid_3~6");
	else if (clipValidNum >= 1)
		onLineGradeMgr->SendGrade("ClipNumValid_1~2");
	else
		onLineGradeMgr->SendGrade("ClipNumValid_0");

	if (scissorCutFinish == 3)
		onLineGradeMgr->SendGrade("ScissorCut_OK");
	else if (scissorCutWait > 0)
		onLineGradeMgr->SendGrade("ScissorCut_Miss");
	else
		onLineGradeMgr->SendGrade("ScissorCut_Fail");

	onLineGradeMgr->SendGrade("ElecCut_" + Ogre::StringConverter::toString(elcCutFinish));

	if (clipValidNum  >= 6)
	{
		if (clipTotalNum <= 7 )
			onLineGradeMgr->SendGrade("ClipNumTotal_OK");
		else
			onLineGradeMgr->SendGrade("ClipNumTotal_Exceed");
		
	}

	else
	{
		int clipInvalid = clipTotalNum - clipValidNum ;
		if (clipInvalid >= 2)
			onLineGradeMgr->SendGrade("ClipDrop");
	}

	if (clipTotalNum >= 4 && clipValidNum / clipTotalNum > 0.8)
		onLineGradeMgr->SendGrade("Operator_Steady");
		
	if (m_bBreakVessel)
		onLineGradeMgr->SendGrade("Operator_BreakConnect");

	float leftToolSpeed = m_pToolsMgr->GetLeftToolMovedSpeed();
	float rightTooSpeed = m_pToolsMgr->GetRightToolMovedSpeed();
	float ToolSpeed = std::max(leftToolSpeed, rightTooSpeed);

	if (scissorCutWait < 3 || elcCutWait < 2)
	{
		if (ToolSpeed > 0.0)
		{
			if (ToolSpeed <= 5.0)
				onLineGradeMgr->SendGrade("MachineHandle_Normal");
			else if (ToolSpeed >  5.0 && ToolSpeed <= 10.0)
				onLineGradeMgr->SendGrade("MachineHandle_Fast");
			else
				onLineGradeMgr->SendGrade("MachineHandle_TooFast");
		}
	}
	
	
	if (m_bFinish && !m_bAllFail)
	{
		if (!m_bBreakVessel)
			onLineGradeMgr->SendGrade("Operator_NoMiss");

		float usedtime = GetElapsedTime();

		if (usedtime < 120)
			onLineGradeMgr->SendGrade("TwoHands_Cooperation");

		if (usedtime < 180)
			onLineGradeMgr->SendGrade("Finished_In3M");
		else if (usedtime < 360)
			onLineGradeMgr->SendGrade("Finished_In6M");
		else
			onLineGradeMgr->SendGrade("Finished_Out6M");
	}
	else
	{
		onLineGradeMgr->SendGrade("Finished_Out6M");
	}*/
    int  numMarkBeExplored = 0;//暴露待夹闭位置

	int  numOrganClipIsEnough = 0;//是否间隔一定距离施放两个钛夹

	int  numOrganClipPartFinished = 0;//施夹位置是否正确
	
	int  numOrganHasInValidClip = 0;
	
	int  numClipUsed = 0;//总共使用的钛架数量

	int  numOrganBeCutoffed = 0;

	int  numOrganCutPosError = 0;

	int  numOrganAlwaysShowMark = 0;

	int numVesselBeTeared = 0;
    OrganToObjectInfoMap::iterator itor = m_organToObjectInfoMap.begin();
    
	while (itor != m_organToObjectInfoMap.end())
	{
		ObjectOperationInfo * info = itor->second;

		if (info->type != ObjectOperationInfo::Type::Unknow)
		{
			MisMedicOrgan_Ordinary * organ = itor->first;
			if (info->m_HasMarkBeSeen)
				numMarkBeExplored++;

			if (info->invalidClipNum + info->validClipNum >= 2)
				numOrganClipIsEnough++;

			if (info->invalidClipNum > 0)
				numOrganHasInValidClip++;

			if (info->numClipPartFin == 2)
				numOrganClipPartFinished++;

			if (info->m_HasBeTeared)
				numVesselBeTeared++;

			numClipUsed += (info->invalidClipNum + info->validClipNum);

			if (info->m_IsCutOffed)
				numOrganBeCutoffed++;

			if (info->state == ObjectOperationInfo::FailOperateNotClipBeforeCut
				|| info->state == ObjectOperationInfo::FaileOperateCutNotBetweenClip)
			{
				numOrganCutPosError++;
			}

			if (info->m_IsMarkAlwaysBeSeen && (info->validClipNum + info->invalidClipNum) > 0)
				numOrganAlwaysShowMark++;
		}
		itor++;
	}

	//是否暴露待夹闭位置
	if (numMarkBeExplored == 2)
		AddScoreItemDetail("0180101800" , 0);
	else if (numMarkBeExplored == 1)
		AddScoreItemDetail("0180101801", 0);
	else
		AddScoreItemDetail("0180101809", 0);


	//是否间隔一定距离施放两个钛夹
	if (numOrganClipIsEnough == 2)
		AddScoreItemDetail("0180204410", 0);
	else if (numOrganClipIsEnough == 1)
		AddScoreItemDetail("0180204411", 0);
	else
		AddScoreItemDetail("0180204419", 0);

	//施夹位置是否正确
	if (numOrganClipPartFinished == 2)
		AddScoreItemDetail("0180302210", 0);
	else if (numOrganClipPartFinished == 1)
		AddScoreItemDetail("0180302211", 0);
	else
		AddScoreItemDetail("0180302212", 0);

	//是否一次夹闭成功
	if (numOrganClipPartFinished == 2)
	{
		if (numOrganHasInValidClip == 0)
			AddScoreItemDetail("0180400110", 0);
		else
			AddScoreItemDetail("0180400111", 0);
	}

	//注意钛夹使用数量
	if (numClipUsed <= 4 && numOrganClipIsEnough > 0)
	{
		if (numOrganHasInValidClip == 0)
			AddScoreItemDetail("0180504300", 0);
		else
			AddScoreItemDetail("0180504301", 0);
	}

	//剪断数量
	if (numOrganBeCutoffed == 2)
		AddScoreItemDetail("0180603900", 0);
	else if (numOrganBeCutoffed == 1)
		AddScoreItemDetail("0180603901", 0);
	else
		AddScoreItemDetail("0180603909", 0);

	//剪断位置是否正确
	if (numOrganCutPosError == 1)
		AddScoreItemDetail("0180702211", 0);
	else if (numOrganCutPosError == 2)
		AddScoreItemDetail("0180702218", 0);
	else if (numOrganCutPosError == 0 && numOrganBeCutoffed == 2)
		AddScoreItemDetail("0180702210", 0);
		

	//是否始终暴露标记
	if (numOrganClipIsEnough > 0)
	{
		if (numOrganAlwaysShowMark == 2)
			AddScoreItemDetail("0180803500", 0);
		else
			AddScoreItemDetail("0180803508", 0);
	}
	if (numVesselBeTeared == 0)
		AddScoreItemDetail("0180904510", 0);
	else
		AddScoreItemDetail("0180904511", 0);

	if (numOrganHasInValidClip == 0 && numOrganClipIsEnough == 2 && numOrganCutPosError == 0)
	{
		AddScoreItemDetail("0181000300", 0);
	}
	else
	{
		AddScoreItemDetail("0181000301", 0);
	}

	float leftToolSpeed = m_pToolsMgr->GetLeftToolMovedSpeed();
	float rightToolSpeed = m_pToolsMgr->GetRightToolMovedSpeed();

	if (m_pToolsMgr->GetLeftToolMovedDistance() > 10 || m_pToolsMgr->GetRightToolMovedDistance() > 10)
	{
		if (leftToolSpeed > 10 || rightToolSpeed > 10)
		{
			AddScoreItemDetail("0181100802", 0);//移动速度过快，有安全隐患
		}
		else if (leftToolSpeed < 5 && rightToolSpeed < 5)
		{
			AddScoreItemDetail("0181100800", 0);//移动平稳流畅
		}
		else
		{
			AddScoreItemDetail("0181100801", 0);//移动速度较快
		}
	}
	//

	if (numOrganClipPartFinished >= 2 && numOrganBeCutoffed >= 2)
	{
		int TimeUsed = GetElapsedTime();
		if (TimeUsed < 60)
			AddScoreItemDetail("0181200500", 0);//2分钟内完成所有操作
		else if (TimeUsed < 120)
			AddScoreItemDetail("0181200501", 0);//在2分钟~3分钟内完成所有操作
		else
			AddScoreItemDetail("0181200502", 0);//完成所有规定操作时超过了3分钟
	}
	__super::OnSaveTrainingReport();
}


SYScoreTable* SYSealAndCut::GetScoreTable()
{
	return SYScoreTableManager::GetInstance()->GetTable("01101801");
}

void NewTrainingHandleEvent11(MxEvent * pEvent, ITraining * pTraining)
{
	if (!pEvent || !pTraining)
		return;
	MisMedicOrgan_Ordinary * pOrgan = NULL;

	SYSealAndCut * pNewTraining = (SYSealAndCut*)pTraining;
	if(pNewTraining->IsFinished())
		return;

	MxToolEvent * pToolEvent = (MxToolEvent*)pEvent;
	pOrgan = static_cast<MisMedicOrgan_Ordinary*>(pToolEvent->GetOrgan());

	switch(pEvent->m_enmEventType)
	{
	case MxEvent::MXET_Cut:
		pNewTraining->DealWithScissorCutObject(pOrgan);
		break;
	case MxEvent::MXET_AddHemoClip:
		pNewTraining->DealWithClipObject(pOrgan);
		break;
	}
}

