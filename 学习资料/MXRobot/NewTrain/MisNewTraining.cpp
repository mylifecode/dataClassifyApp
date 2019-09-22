#include "MisNewTraining.h"
#include "Instruments/tool.h"
#include "MisMedicOrganOrdinary.h"
#include "VeinConnectObject.h"
#include "InputSystem.h"
#include "Instruments/ElectricHook.h"
#include "Instruments/DissectingForceps.h"
#include "Instruments/Scissors.h"
#include "EffectManager.h"
#include "DynamicObjectRenderable.h"
#include "XMLWrapperOrgan.h"
#include "XMLWrapperAdhere.h"
#include "XMLWrapperAdhesion.h"
#include "XMLWrapperTraining.h"
#include "XMLWrapperOrganTranslation.h"
#include "XMLWrapperWaterPool.h"
#include "XMLWrapperViewDetection.h"
#include "XMLWrapperOperateItem.h"
#include "TrainScoreSystem.h"
#include "MXEventsDump.h"
#include "MXToolEvent.h"
#include "customconstraint.h"
#include "MisMedicRigidPrimtive.h"
#include "MisToolCollideDataConfig.h"
#include "MisMedicObjectUnion.h"
#include "MisMedicAdhesion.h"
#include "MisMedicObjLink_Approach.h"
#include "MisMedicThreadRope.h"
#include "MisMedicSimpleLooper.h"
#include "MisMedicTubeBody.h"
#include "SutureThreadV2.h"
//#include "NewVeinConnectObject.h"
//#include "VeinConnectObjectV2.h"
#include "MisMedicOrganAttachment.h"
#include "MXGlobalConfig.h"
#include "SYTrainingReport.h"
#include "WaterPool.h"
#include "WaterManager.h"
#include "CustomCollision.h"
#include "Instruments/HarmonicScalpel.h"
#include "EditCamera.h"
#include "../include/ScreenEffect.h"
#include "../include/ToolSpenetrateMgr.h"
#include "ACTubeShapeObject.h"
//#include <QDebug>
#include "SYUserData.h"
#include "SYScoreTable.h"
#include <QSet>

static QString GetOrganNameByType(TEDOT type);

void NewTrainingHandleEvent(MxEvent * pEvent, ITraining * pTraining)
{
	if (!pEvent || !pTraining)
		return;

	MisNewTraining * pNewTraining = dynamic_cast<MisNewTraining *> (pTraining);

	if(pNewTraining)
	{
		TrainScoreSystem * scoreSys = pNewTraining->GetScoreSystem();
		if(scoreSys)
		{
			scoreSys->ProcessTrainEvent(pEvent);
		}
	}
}

//=============================================================================================
MisNewTraining::MisNewTraining(void)
:m_ScoreSys(NULL)
{
	//Ogre::Root::getSingleton().addFrameListener(this);
	m_IsNewTrainMode = true;
	m_IsFinalMovieShowed = false;
	m_CameraAngle = 0;
	CTipMgr::Instance()->SetQueeu(true);
	MisToolCollideDataConfigMgr::Create();
	

	m_SceneGravityDir = Ogre::Vector3(0 , 0 , -1.0f);

	m_NeedExplore = false;
	m_ExploreResult = false;
	m_pLocationDetection = NULL;
	m_DetectCameraIntersect = false;
	m_HasCameraTouchSomething = false;
	m_AutoGenOrganID = EDOT_ORGAN_LIMIT + 1000;
	m_QuitType = TPWBT_TRAIN_NORMAL;
	CScoreMgr::Instance()->OnTrainCreated();
	CTipMgr::Instance()->OnTrainCreated();
    TitanicClipInfo::s_clipEmptyCount = 0;

	srand(GetTickCount());
}
//=============================================================================================
MisNewTraining::~MisNewTraining(void)
{
	CScoreMgr::Instance()->OnTrainDestroyed();
	CTipMgr::Instance()->OnTrainDestroyed();

	PhysicsWrapper::GetSingleTon().m_dynamicsWorld->SetInternalSimulateListener(0);

	PhysicsWrapper::GetSingleTon().m_dynamicsWorld->SetCustomActionListener(0);

	PhysicsWrapper::GetSingleTon().m_dynamicsWorld->SetDynamicWorldEventListener(0);

	GFPhysGlobalConfig::GetGlobalConfig().SetRSContactListener(0);

	GFPhysGlobalConfig::GetGlobalConfig().SetRigidContactListener(0);

	//Destroy config manager
	MisToolCollideDataConfigMgr::Destroy();
	
	//
	CTipMgr::Instance()->SetQueeu(false);

	if(m_ScoreSys)
	{
		m_ScoreSys->SetTrainRunning(false);
		delete m_ScoreSys;
		m_ScoreSys = NULL;
	}

	//remove adhesions 
	//for(size_t c = 0 ; c < m_ObjAdhersions.size(); c++)
	//{
	//	delete m_ObjAdhersions[c];
	//}
	//m_ObjAdhersions.clear();

    for (int i = 0; i != m_ObjAdhersions.size(); ++i)
    {
        if (m_ObjAdhersions[i])
        {
            delete m_ObjAdhersions[i];
            m_ObjAdhersions[i] = NULL;
        }
    }
    m_ObjAdhersions.clear();

	//remove envelop
	for(size_t e = 0 ; e < m_ObjEnvelops.size(); e++)
	{
		delete m_ObjEnvelops[e];
	}
	m_ObjEnvelops.clear();

	//remove waterpool
	for(size_t w = 0 ; w < m_WaterPools.size(); w++)
	{
		delete m_WaterPools[w];
	}
	m_WaterPools.clear();

	for(size_t v = 0 ; v < m_ExploreDetections.size() ; v++)
	{
		delete m_ExploreDetections[v];
	}
	m_ExploreDetections.clear();
	if(m_pLocationDetection)
	{
		delete m_pLocationDetection;
		m_pLocationDetection = NULL;	
	}

	//remove rope
	std::vector<MisMedicThreadRope *> ThreadToRemove = m_ThreadRopes;
	for(size_t c = 0 ; c < ThreadToRemove.size() ; c++)
	{
		RemoveThreadRopeFromWorld(ThreadToRemove[c]);
	}
	ThreadToRemove.clear();
	m_ThreadRopes.clear();
    
    //remove needle
    std::vector<SutureNeedle *> NeedleToRemove = m_SutureNeedles;
    for(size_t c = 0 ; c < NeedleToRemove.size() ; c++)
    {
        RemoveNeedleFromWorld(NeedleToRemove[c]);
    }
    NeedleToRemove.clear();
    m_SutureNeedles.clear();

    //remove tube
    for(size_t w = 0 ; w < m_TubeBodies.size(); w++)
    {
        delete m_TubeBodies[w];
    }
    m_TubeBodies.clear();

	//remove all objects etc(soft mesh , soft tube , connect , rigid body)
	//connect first  no need remove this 
	std::vector<MisMedicOrganInterface*> OrgansToRemove;
	DynObjMap::iterator itor = m_DynObjMap.begin();
	while(itor != m_DynObjMap.end())
	{
		MisMedicOrganInterface * oif = itor->second;
		if(oif)
		{
		   OrgansToRemove.push_back(oif);
		}
		itor++;
	}
	

	for(size_t i = 0 ; i < OrgansToRemove.size() ;i++)
	{
		RemoveOrganFromWorld(OrgansToRemove[i]);
	}

	OrgansToRemove.clear();

	//fake organ
	DynObjMap::iterator fakeOrganItor = m_DynObjMapForNonOrgan.begin();
	while(fakeOrganItor != m_DynObjMapForNonOrgan.end())
	{
		MisMedicOrganInterface * oif = fakeOrganItor->second;
		if(oif)
		{
			OrgansToRemove.push_back(oif);
		}
		fakeOrganItor++;
	}

	for(size_t i = 0 ; i < OrgansToRemove.size() ;i++)
	{
		RemoveFakeOrganFromWorld(OrgansToRemove[i]);
	}

	OrgansToRemove.clear();

	m_DynObjMapForNonOrgan.clear();
	
	m_DynObjMap.clear();

    EditorCamera * editcam = EditorCamera::GetGlobalEditorCamera();
    if (editcam)
    {
        EditorCamera::DelEditorCamera();
    }

	/*for(size_t i = 0 ; i < m_connectobjets.size() ;i++)
	{
		m_DynObjMap.erase(m_connectobjets[i]->m_OrganID);
		m_connectobjets[i]->RemoveFromWorld();
		delete m_connectobjets[i];
	}
	m_connectobjets.clear();

	DynObjMap::iterator itor = m_DynObjMap.begin();
	while(itor != m_DynObjMap.end())
	{
		MisMedicOrganInterface * oif = itor->second;
		if(oif)
		{
		   oif->RemoveFromWorld();
		   delete oif;
		}
		itor = m_DynObjMap.erase(itor);
	}
	*/

	/*

	//remove dynamic objects
	for(size_t i = 0 ; i < m_dynamicObjects.size() ;i++)
	{
		m_DynObjMap.erase(m_dynamicObjects[i]->m_OrganID);
		m_dynamicObjects[i]->RemoveFromWorld();
		delete m_dynamicObjects[i];
	}
	m_dynamicObjects.clear();


	//remove dynamic tube
	for(size_t i = 0 ; i < m_softtubes.size() ;i++)
	{
		m_DynObjMap.erase(m_softtubes[i]->m_OrganID);
		m_softtubes[i]->RemoveFromWorld();
		delete m_softtubes[i];
	}
	m_softtubes.clear();
	*/

	//remove listener and destroy GPSDK
	PhysicsWrapper::GetSingleTon().Terminate();

	RemoveAllTimer();
}

//=============================================================================================
std::vector<VeinConnectObject*> MisNewTraining::GetVeinConnectObjects()
{
	std::vector<VeinConnectObject*> connectObjects;
	
	DynObjMap::iterator itor = m_DynObjMap.begin();
	
	while(itor != m_DynObjMap.end())
	{
		VeinConnectObject * veinobj = dynamic_cast<VeinConnectObject*>(itor->second);
		
		if(veinobj)
		   connectObjects.push_back(veinobj);
		
		itor++;
	}

	return connectObjects;
}

//=============================================================================================
VeinConnectObject  *  MisNewTraining::GetVeinConnect(int connectType)
{
	DynObjMap::iterator itor = m_DynObjMap.find(connectType);
	if(itor != m_DynObjMap.end())
	{
		MisMedicOrganInterface * organif = itor->second;
		VeinConnectObject * veinobj = dynamic_cast<VeinConnectObject*>(organif);
		return veinobj;
	}
	return 0;
}
//=============================================================================================
MisMedicOrganInterface* MisNewTraining::GetOrgan(DynamicObjType type)
{
	MisMedicOrganInterface * pOrgan = NULL;

	for(DynObjMap::iterator itr = m_DynObjMap.begin();itr != m_DynObjMap.end();++itr)
	{
		if(itr->second->m_OrganType == type)
		{
			pOrgan = itr->second;
			break;
		}
	}

	return pOrgan;
}

MisMedicOrganInterface * MisNewTraining::GetOrgan(int id)
{
	DynObjMap::iterator itor = m_DynObjMap.find(id);
	
	if(itor == m_DynObjMap.end())
		return 0;
	else
		return itor->second;
}

MisMedicOrganInterface * MisNewTraining::GetOrgan(GFPhysSoftBody * body)
{	
	DynObjMap::iterator itor = m_DynObjMap.begin();
	while(itor != m_DynObjMap.end())
	{
		MisMedicOrganInterface * organif = itor->second;
		if(organif)
		{
			if(organif->GetCreateInfo().m_objTopologyType == DOT_VOLMESH || organif->GetCreateInfo().m_objTopologyType == DOT_MEMBRANE)
			{
				MisMedicOrgan_Ordinary * organor = static_cast<MisMedicOrgan_Ordinary*>(organif);
				if(organor->m_physbody == body)
				  return organif;
			}
			
		}
		itor++;
	}
	return 0;
}
//=============================================================================================
void MisNewTraining::GetAllOrgan(std::vector<MisMedicOrganInterface*>& ogans)
{
	DynObjMap::iterator itor = m_DynObjMap.begin();
	while(itor != m_DynObjMap.end())
	{
		MisMedicOrganInterface * organif = itor->second;
		if(organif)
		{
			ogans.push_back(organif);
		}
		itor++;
	}
}
//========================================================================================================
void MisNewTraining::GetAllTubes(std::vector<ACTubeShapeObject*> & tubes)
{
	DynObjMap::iterator itor = m_DynObjMap.begin();
	
	while (itor != m_DynObjMap.end())
	{
		ACTubeShapeObject * tubeObj = dynamic_cast<ACTubeShapeObject*>(itor->second);
		if (tubeObj)
		{
			tubes.push_back(tubeObj);
		}
		itor++;
	}
}
//=============================================================================================
void MisNewTraining::FacesBeAdded(const GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & faces , MisMedicOrgan_Ordinary * organ)
{
	GFPhysSoftBody * sb = organ->m_physbody;

	for(size_t c = 0 ; c < faces.size() ; c++)
	{
		GFPhysSoftBodyFace * face = faces[c];
		if(m_pToolsMgr)
		{
			ITool * toolArray[2] = {0,0};
			toolArray[0] = m_pToolsMgr->GetLeftTool();
			toolArray[1] = m_pToolsMgr->GetRightTool();
			for(int t = 0 ; t < 2 ; t++)
			{
				if(toolArray[t])
				   toolArray[t]->OnSoftBodyFaceBeAdded(sb , face);
			}
		}
	}
}
//=============================================================================================
void MisNewTraining::TetrasBeRemoved(const GFPhysAlignedVectorObj<GFPhysSoftBodyTetrahedron*> & tetras , MisMedicOrgan_Ordinary * organ)
{
	for(size_t c = 0 ; c < m_ObjAdhersions.size() ; c++)
	{	
		if(m_ObjAdhersions[c]->m_ConnectOrganA == organ || m_ObjAdhersions[c]->m_ConnectOrganB == organ)
		{
			m_ObjAdhersions[c]->OnTetrasRemoved(tetras);
		}
	}
}
//=============================================================================================
void MisNewTraining::RefreshVeinconnectOnFacesBeModified(const GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & modifiedfaces, MisMedicOrgan_Ordinary * organ)
{
	std::set<MMO_Face::VeinClusterInfo> veinconnectChanged;

    for (size_t c = 0, nc = modifiedfaces.size(); c <nc; c++)
	{
        MMO_Face& mmoface = organ->GetMMOFace(modifiedfaces[c]);

		if (!mmoface.m_HasError)
		{
            for (int i = 0, ni = mmoface.m_VeinInfoVector.size(); i<ni; i++)
			{
                const MMO_Face::VeinInfo info = mmoface.m_VeinInfoVector[i];
                if (info.valid)
				{
                    MMO_Face::VeinClusterInfo clusterinfo = { info.veinobj, info.clusterId };
                    veinconnectChanged.insert(clusterinfo);
				}
			}
		}
	}

	for (std::set<MMO_Face::VeinClusterInfo>::const_iterator iter = veinconnectChanged.begin(); iter != veinconnectChanged.end(); iter++)
	{
		VeinConnectObject * veinobj = iter->veinobj;

		VeinConnectPair & MajorPair = veinobj->m_clusters[iter->clusterId].m_pair[0];
		VeinConnectPair & AttachPair = veinobj->m_clusters[iter->clusterId].m_pair[1];
		if (MajorPair.m_Valid || AttachPair.m_Valid)
		{
			OnVeinconnectChanged(veinobj, iter->clusterId);
		}
	}
}
//=============================================================================================
void MisNewTraining::FacesBeRemoved(const GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & faces,  GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & createdfaces, MisMedicOrgan_Ordinary * organ)
{
	GFPhysSoftBody * sb = organ->m_physbody;
	
	GFPhysSoftBodyShape * hostshape = &sb->GetSoftBodyShape();		
	//
    for (size_t c = 0, nc = faces.size(); c <nc; c++)
    {
        GFPhysSoftBodyFace * face = faces[c];

        for (size_t m = 0, nm = m_ObjAdhersions.size(); m <nm; m++)
        {
            m_ObjAdhersions[m]->OnFaceRemoved(face);
        }

        for (size_t e = 0, ne = m_ObjEnvelops.size(); e <ne; e++)
        {
            m_ObjEnvelops[e]->OnFaceRemoved(face, hostshape);
        }

        std::map<MisMedicOrgan_Ordinary*, MisMedicAdhesionCluster*>::iterator adhesionItor = m_Adhesions.begin();
        while (adhesionItor != m_Adhesions.end())
        {
            MisMedicAdhesionCluster *pAdhesionCluster = adhesionItor->second;
            pAdhesionCluster->OnFaceRemoved(face, hostshape);
            ++adhesionItor;
        }

        //notify tool object
        if (m_pToolsMgr)
        {
            ITool * toolArray[2] = { 0, 0 };
            toolArray[0] = m_pToolsMgr->GetLeftTool();
            toolArray[1] = m_pToolsMgr->GetRightTool();
            for (int t = 0; t < 2; t++)
            {
                if (toolArray[t])
                    toolArray[t]->OnSoftBodyFaceBeDeleted(sb, face);
            }

			std::vector<ITool*> fixTools;

			m_pToolsMgr->GetFixTools(fixTools);

			for (int c = 0; c < fixTools.size(); c++)
			{
				fixTools[c]->OnSoftBodyFaceBeDeleted(sb, face);
			}
        }
	}
}
//===============================================================================================
void MisNewTraining::FacesBeModified(const GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & faces,  GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & createdfaces, MisMedicOrgan_Ordinary * organ)
{
	//temp process remove pair
	GFPhysSoftBody * sb = organ->m_physbody;

	GFPhysSoftBodyShape * hostshape = &sb->GetSoftBodyShape();

    for (size_t c = 0, nc = faces.size(); c <nc; c++)
	{
		GFPhysSoftBodyFace * face = faces[c];

        for (size_t m = 0, nm = m_ObjAdhersions.size(); m <nm; m++)
		{
			m_ObjAdhersions[m]->OnFaceRemoved(face);
		}

        for (size_t e = 0, ne = m_ObjEnvelops.size(); e <ne; e++)
		{
			m_ObjEnvelops[e]->OnFaceRemoved(face, hostshape);
		}

		std::map<MisMedicOrgan_Ordinary*, MisMedicAdhesionCluster*>::iterator adhesionItor = m_Adhesions.begin();
		while (adhesionItor != m_Adhesions.end())
		{
			MisMedicAdhesionCluster *pAdhesionCluster = adhesionItor->second;
			pAdhesionCluster->OnFaceRemoved(face, hostshape);
			++adhesionItor;
		}
	}
}
//=============================================================================================
void MisNewTraining::NodesBeRemoved(const GFPhysAlignedVectorObj<GFPhysSoftBodyNode*> & nodes , MisMedicOrgan_Ordinary * organ)
{
    for (size_t c = 0, nc = m_ObjAdhersions.size(); c <nc; c++)
	{	
		if(m_ObjAdhersions[c]->m_ConnectOrganA == organ || m_ObjAdhersions[c]->m_ConnectOrganB == organ)
		{
			m_ObjAdhersions[c]->OnNodesRemoved(nodes);
		}
		//
	}

    for (size_t c = 0, nc = m_ShadowNodesLinkage.size(); c <nc; c++)
    {
        m_ShadowNodesLinkage[c].OnNodesRemoved(nodes, organ);
    }
	//notify tool object
	if(m_pToolsMgr)
	{
		ITool * toolArray[2] = {0,0};
		toolArray[0] = m_pToolsMgr->GetLeftTool();
		toolArray[1] = m_pToolsMgr->GetRightTool();
		for(int t = 0 ;t < 2 ; t++)
		{
			if(toolArray[t])
				toolArray[t]->OnSoftBodyNodesBeDeleted(organ->m_physbody , nodes);
		}
	}
}

class TetraTreeOverLapCallBack : public GFPhysNodeOverlapCallback
{
public:

    TetraTreeOverLapCallBack(GFPhysVectorObj<GFPhysSoftBodyTetrahedron*> & ovelaptetra)
        : m_OverlappedTetra(ovelaptetra)
    {

    }

	bool PointInPositivePlane(const GFPhysVector3 &p,  const GFPhysVector3 &a,  const GFPhysVector3 &b,  const GFPhysVector3& c)
	{
		return (p-a).Dot((b-a).Cross(c-a)) >= 0.0f; // [AP AB AC] < 0
	}

    void ProcessOverlappedNode(int subPart , int triangleIndex , void * UserData)
    {
        GFPhysSoftBodyTetrahedron * tetra = (GFPhysSoftBodyTetrahedron*)UserData;

		/*GFPhysSoftBodyNode * tetraNodeA = tetra->m_TetraNodes[0];
		GFPhysSoftBodyNode * tetraNodeB = tetra->m_TetraNodes[1]; 
		GFPhysSoftBodyNode * tetraNodeC = tetra->m_TetraNodes[2]; 
		GFPhysSoftBodyNode * tetraNodeD = tetra->m_TetraNodes[3]; 


		bool result0 = PointInPositivePlane(m_PointPos , tetraNodeA->m_UnDeformedPos , tetraNodeB->m_UnDeformedPos ,  tetraNodeC->m_UnDeformedPos);
		bool result1 = PointInPositivePlane(m_PointPos , tetraNodeA->m_UnDeformedPos , tetraNodeD->m_UnDeformedPos ,  tetraNodeB->m_UnDeformedPos);
		bool result2 = PointInPositivePlane(m_PointPos , tetraNodeB->m_UnDeformedPos , tetraNodeD->m_UnDeformedPos ,  tetraNodeC->m_UnDeformedPos);
		bool result3 = PointInPositivePlane(m_PointPos , tetraNodeC->m_UnDeformedPos , tetraNodeD->m_UnDeformedPos ,  tetraNodeA->m_UnDeformedPos);
	
		if(result0 == true && result1 == true && result2 == true && result3 == true)//for negative volume oriented tetra
		{
			m_OverlappedTetra.push_back(tetra);
		}
		else if(result0 == false && result1 == false && result2 == false && result3 == false)//for negative volume oriented tetra
		{
			m_OverlappedTetra.push_back(tetra);
		}*/
		m_OverlappedTetra.push_back(tetra);
    }

    GFPhysVectorObj<GFPhysSoftBodyTetrahedron*> & m_OverlappedTetra;
};

void MisNewTraining::UpdateNodeToTetraLinks(const std::map<GFPhysSoftBodyTetrahedron *, std::vector<GFPhysSoftBodyTetrahedron *>> & DelToCreateMap,
											const GFPhysAlignedVectorObj<GFPhysSoftBodyTetrahedron*> createdtetras ,
											MisMedicOrgan_Ordinary * organ)
{   
    for(size_t c = 0 ; c < m_ObjAdhersions.size() ; c++)
    {
		MisMedicObjectAdhersion * linkadh = dynamic_cast<MisMedicObjectAdhersion*>(m_ObjAdhersions[c]);

		if(linkadh == 0)
		   continue;

		if(linkadh->m_ConnectOrganA == organ || linkadh->m_ConnectOrganB == organ)
		{
			std::vector<TetrahedronAttachConstraint> & links = linkadh->m_NodeToTetraLinks;

			for (size_t c = 0; c < links.size(); c++)
			{
				if (links[c].m_IsValid == false || links[c].m_AttachNode == 0)
					continue;

				std::map<GFPhysSoftBodyTetrahedron *, std::vector<GFPhysSoftBodyTetrahedron *>>::const_iterator itor = DelToCreateMap.find(links[c].m_Tetra);
					
				if (itor != DelToCreateMap.end())//the link's tetra has been removed
				{
					//try bind to it's splatted tetrahedrons
					for (size_t t = 0; t < itor->second.size(); t++)
					{
						GFPhysSoftBodyTetrahedron * newTetra = itor->second[t];
						
						float TetraWeights[4] = { -1.0f, -1.0f, -1.0f, -1.0f };
						bool gettedf = GetPointBarycentricCoordinate(newTetra->m_TetraNodes[0]->m_UnDeformedPos,
								                                     newTetra->m_TetraNodes[1]->m_UnDeformedPos,
								                                     newTetra->m_TetraNodes[2]->m_UnDeformedPos,
								                                     newTetra->m_TetraNodes[3]->m_UnDeformedPos,
								                                     links[c].m_AttachNode->m_UnDeformedPos,
								                                     TetraWeights);
						
						if (gettedf && TetraWeights[0] >= 0 && TetraWeights[1] >= 0
								    && TetraWeights[2] >= 0 && TetraWeights[3] >= 0
								    && TetraWeights[0] <= 1 && TetraWeights[1] <= 1
								    && TetraWeights[2] <= 1 && TetraWeights[3] <= 1)
						{
							links[c].ReBuild(links[c].m_AttachNode, newTetra, TetraWeights);
							newTetra->m_StateFlag |= GPSESF_CONNECTED;
							break;
						}
					}
				}
			}
		}
    }
}
//=============================================================================================
void MisNewTraining::InternalSimulateStart(int currStep , int TotalStep , Real dt)
{
	if(m_pToolsMgr->GetLeftTool())
	{
	   m_pToolsMgr->GetLeftTool()->InternalSimulationStart(currStep ,  TotalStep ,  dt);// UpdateIntepolatedConvex((float)(currStep+1) / (float)(TotalStep));
	}

	if(m_pToolsMgr->GetRightTool())
	{
	   m_pToolsMgr->GetRightTool()->InternalSimulationStart(currStep ,  TotalStep ,  dt);// UpdateIntepolatedConvex((float)(currStep+1) / (float)(TotalStep));
	}

	for(size_t c = 0 ; c < m_ThreadRopes.size() ; c++)
	{
		m_ThreadRopes[c]->BeginSimulateThreadPhysics(dt);
	}

    for(size_t c = 0 ; c < m_SutureNeedles.size() ; c++)
    {
        m_SutureNeedles[c]->InternalSimulationStart(currStep ,  TotalStep ,  dt);
    }

	for (size_t c = 0; c < m_SutureNeedlesV2.size(); c++)
	{
		m_SutureNeedlesV2[c]->InternalSimulationStart(currStep, TotalStep, dt);
	}
    for(size_t c = 0 ; c < m_TubeBodies.size() ; c++)
    {
        m_TubeBodies[c]->SimulateTubePhysics(dt);
    }

	DynObjMap::iterator ditor = m_DynObjMap.begin();
	while (ditor != m_DynObjMap.end())
	{
		ditor->second->InternalSimulateStart(currStep, TotalStep, dt);
		ditor++;
	}
}
//======================================================================================================================
void MisNewTraining::InternalSimulateEnd(int currStep , int TotalStep , Real dt)
{
	if(m_pToolsMgr->GetLeftTool())
	{
		m_pToolsMgr->GetLeftTool()->InternalSimulationEnd(currStep ,  TotalStep ,  dt);// UpdateIntepolatedConvex((float)(currStep+1) / (float)(TotalStep));
	}

	if(m_pToolsMgr->GetRightTool())
	{
		m_pToolsMgr->GetRightTool()->InternalSimulationEnd(currStep ,  TotalStep ,  dt);// UpdateIntepolatedConvex((float)(currStep+1) / (float)(TotalStep));
	}

	for(size_t c = 0 ; c < m_ThreadRopes.size() ; c++)
	{
		m_ThreadRopes[c]->EndSimuolateThreadPhysics(dt);
	}

    for(size_t c = 0 ; c < m_SutureNeedles.size() ; c++)
    {
        m_SutureNeedles[c]->InternalSimulationEnd(currStep ,  TotalStep ,  dt);
    }

	for (size_t c = 0; c < m_SutureNeedlesV2.size(); c++)
	{
		m_SutureNeedlesV2[c]->InternalSimulationEnd(currStep, TotalStep, dt);
	}

	DynObjMap::iterator ditor = m_DynObjMap.begin();
	while(ditor != m_DynObjMap.end())
	{
		ditor->second->InternalSimulateEnd( currStep ,  TotalStep ,  dt);
		ditor++;
	}
}
//======================================================================================================================
void MisNewTraining::PerformCustomCollision(GFPhysDiscreteDynamicsWorld * dynworld)
{
	CTool * leftTool = (CTool*)(m_pToolsMgr->GetLeftTool());
	CTool * rightTool = (CTool*)(m_pToolsMgr->GetRightTool());

	std::vector<CTool *> Tools;
	Tools.push_back(leftTool);
	Tools.push_back(rightTool);

	std::vector<VeinConnectObject*> ConnectObjects = GetVeinConnectObjects();

	//std::vector<VeinConnectObjectV2*> veinObjV2s = GetVeinConnectObjectV2s();

	for(size_t t = 0 ;t < Tools.size() ; t++)
	{
		CTool * tool = Tools[t];

		if(tool)
		{
			

			for(size_t v = 0 ; v < ConnectObjects.size() ; v++)
			{
				VeinConnectObject * veinobj = ConnectObjects[v];
				
				CElectricHook * electricTool = dynamic_cast<CElectricHook*>(tool);

				if(electricTool)
				{
					CElectricHook::VeinHookShapeData WorldHookData = electricTool->GetWorldHookShapeData();
					
					CElectricHook::VeinHookShapeData LocalHookData = electricTool->GetLocalHookShapeData();

					GFPhysRigidBody * HookPart = electricTool->GetHookRigidBodyPart();
					
					int NumStripHooked = veinobj->GetNumPairRigidHooked(HookPart);

					if (NumStripHooked < 3)
					{
						veinobj->ClearHookedCount();
						veinobj->TryHookStrips(WorldHookData.m_HookLinePoints[0], WorldHookData.m_HookLinePoints[1], LocalHookData.m_HookSupportOffsetVec, LocalHookData.m_HookProbDir, LocalHookData.m_HookLineRadius, HookPart);
					}
					else
					{
						int i = 0;
						int j = i + 2;
					}
				}
				else
				{//veinconnect collision instrument
                    GFPhysRigidBody * collidebodies[3];

                    int toolpartNum = tool->GetCollideVeinObjectBody(collidebodies);

                    for (int t = 0; t < toolpartNum; t++)
                    {
                        if (collidebodies[t])
                        {
                            //NewTrainToolConvexData::CollideShapeData & ldata = tool->m_righttoolpartconvex.m_CollideShapesData[0];
                            //GFPhysVector3 center = collidebodies[t]->GetWorldTransform() * OgreToGPVec3(ldata.m_boxcenter);
                            //veinobj->painting.PushBackPoint(CustomPoint(&center, Ogre::ColourValue::Red, 0.1));


                            veinobj->TestCollisionWithBody(collidebodies[t], tool);
                            //////////////////////////////////////////////////////////////////////////
                            //GFPhysVector3 aabbmin, aabbmax;

                            //GFPhysCollideShape* shape = (GFPhysCollideShape*)(collidebodies[t]->GetCollisionShape());
                            //shape->GetAabb(collidebodies[t]->GetWorldTransform(), aabbmin, aabbmax);

                            //veinobj->painting.PushBackPoint(CustomPoint(&aabbmin, Ogre::ColourValue::Red, 0.1));
                            //veinobj->painting.PushBackPoint(CustomPoint(&aabbmax, Ogre::ColourValue::Red, 0.1));

                            //////////////////////////////////////////////////////////////////////////
                        }
                    }
				}
				veinobj->RecalculateHookPointWorldPos();
			}

			/*for(size_t v = 0 ; v < m_veinObjV2s.size() ; v++)
			{
				VeinConnectObjectV2 * veinObjV2 = m_veinObjV2s[v];
				
				CElectricHook * electricTool = dynamic_cast<CElectricHook*>(tool);
				if(!electricTool)
				{
					for(int t = 0 ; t < toolpartNum ; t++)
					{
						if(collidebodies[t])
							veinObjV2->TestCollisionWithRigidBody(collidebodies[t]);
					}
				}
			}*/
		}
	}
}
//======================================================================================================================
void MisNewTraining::GetTempPosConstraints(GFPhysVectorObj<GFPhysPositionConstraint*>)
{

}
void MisNewTraining::ReadCustomDataFile(const Ogre::String & customDataFile)
{

}
//======================================================================================================================
bool MisNewTraining::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{
	MisToolCollideDataConfigMgr::Instance()->ReadFromXML("..\\Data\\InstrumentCollideData.xml");

	//initialize simulate engine first train initialize will rely on it
	PhysicsWrapper::GetSingleTon().InitializePhysicsWorld(pTrainingConfig->m_SolverItertorNum , pTrainingConfig->m_SolverThreadNum,pTrainingConfig->m_StrainItertorNum);

	//register listeners
	PhysicsWrapper::GetSingleTon().m_dynamicsWorld->SetInternalSimulateListener(this);

	PhysicsWrapper::GetSingleTon().m_dynamicsWorld->SetCustomActionListener(this);

	PhysicsWrapper::GetSingleTon().m_dynamicsWorld->SetDynamicWorldEventListener(this);  
	
	GFPhysGlobalConfig::GetGlobalConfig().SetRSContactListener(this);
	
	GFPhysGlobalConfig::GetGlobalConfig().SetRigidContactListener(this);


	if(pTrainingConfig->m_flag_SimulationFreqency)
	{
		float simfreq = pTrainingConfig->m_SimulationFreqency;
		PhysicsWrapper::GetSingleTon().SetSimulationFrequency(simfreq);
	}

	//load train
	if(pTrainingConfig->m_flag_CustomDataFile)
	{
		ReadCustomDataFile(pTrainingConfig->m_CustomDataFile);
	}
	bool result =  CBasicTraining::Initialize(pTrainingConfig , pToolConfig);
	
	CreateTrainingScene(pTrainingConfig);
	
	//score system
	m_ScoreSys = new TrainScoreSystem(this);
	m_ScoreSys->SetTrainRunning(true);

	//CMXEventsDump::Instance()->RegisterHandleEventsFunc(0 , this);//default no event handle function
    m_SutureNodeNum = pTrainingConfig->m_ThreadNodeNum;
    m_SutureRsLength = pTrainingConfig->m_ThreadRSLEN;
    m_NeedleSkeletonFile = pTrainingConfig->m_NeedleSkeleton;

	m_camPivotPos = m_pLargeCamera->getParentNode()->getPosition();

    m_preCameraPos = m_pLargeCamera->getDerivedPosition();
    m_disCameraMove = 0.0f;
    m_timeCameraMove = 0.0f;
    m_CameraSpeed = 0.0f;

	return result;
}
//=================================================================================================
void MisNewTraining::SetEditorMode(bool set)
{
	CBasicTraining::SetEditorMode(set);
	if(set == true)
	{
		//PhysicsWrapper::GetSingleTon().SetActive(false);
		DynObjMap::iterator itor = m_DynObjMap.begin();
		while(itor != m_DynObjMap.end())
		{
			MisMedicOrganInterface * oif = itor->second;
			
			MisMedicOrgan_Ordinary * organmesh = dynamic_cast<MisMedicOrgan_Ordinary*>(oif);

			if(organmesh)//reset all soft body position to reset position
			{
				GFPhysSoftBody * sb = organmesh->m_physbody;
				sb->SetGravity(GFPhysVector3(0,0,0));
				GFPhysSoftBodyNode * node = sb->GetNodeList();
				while(node)
				{
					node->m_Velocity = GFPhysVector3(0,0,0);
					node = node->m_Next;
				}
			}
			itor++;
		}
	}
}
//=================================================================================================
void MisNewTraining::UpdateForceFeedBack()
{
	Ogre::Vector3 LeftContactForce , RightContactForce , LeftDragForce , RightDragForce;

	float ForceFeedAmplify = 100.0f;

	if(MxGlobalConfig::Instance()->EnabledForceFeedback())
	{
		//set force feed back to phantom
		ITool * toolLeft = m_pToolsMgr->GetLeftTool();
		if(toolLeft)
		{
			float stickForcePercent = 0;
			toolLeft->GetForceFeedBack(LeftContactForce , LeftDragForce);
			InputSystem::GetInstance(DEVICETYPE_LEFT)->SetForceFeedBack_New(true, LeftContactForce*ForceFeedAmplify, LeftDragForce*ForceFeedAmplify);
		}
		else
			InputSystem::GetInstance(DEVICETYPE_LEFT)->SetForceFeedBack_New(false, Ogre::Vector3::ZERO, Ogre::Vector3::ZERO);

		ITool * toolRight = m_pToolsMgr->GetRightTool();
		if(toolRight)
		{
			float stickForcePercent = 0;
			toolRight->GetForceFeedBack(RightContactForce, RightDragForce);
			InputSystem::GetInstance(DEVICETYPE_RIGHT)->SetForceFeedBack_New(true, RightContactForce*ForceFeedAmplify, RightDragForce*ForceFeedAmplify);
		}
		else
			InputSystem::GetInstance(DEVICETYPE_RIGHT)->SetForceFeedBack_New(false, Ogre::Vector3::ZERO, Ogre::Vector3::ZERO);
	}
	//draw debug force
#ifdef DRAW_DEBUGFORCE
	m_ForceFeedBackDebugDraw->clear();
	m_ForceFeedBackDebugDraw->begin("forcedraw" , Ogre::RenderOperation::OT_LINE_LIST);

	if(toolLeft)
	{
		Ogre::Vector3 LeftStartPos = toolLeft->GetLeftNode()->_getDerivedPosition();
		Ogre::Vector3 LeftEndPos = LeftStartPos+LeftForceFeedBack*2.0f;

		m_ForceFeedBackDebugDraw->position(LeftStartPos.x, LeftStartPos.y, LeftStartPos.z);
		m_ForceFeedBackDebugDraw->colour(Ogre::ColourValue(0.0f,1.0f,0.0f,1.0f));

		m_ForceFeedBackDebugDraw->position(LeftEndPos.x, LeftEndPos.y, LeftEndPos.z);
		m_ForceFeedBackDebugDraw->colour(Ogre::ColourValue(0.0f,1.0f,0.0f,1.0f));

	}

	if(toolRight)
	{
		Ogre::Vector3 RightStartPos = toolRight->GetLeftNode()->_getDerivedPosition();
		Ogre::Vector3 RightEndPos = RightStartPos+RightForceFeedBack*2.0f;

		m_ForceFeedBackDebugDraw->position(RightStartPos.x, RightStartPos.y, RightStartPos.z);
		m_ForceFeedBackDebugDraw->colour(Ogre::ColourValue(0.0f,1.0f,0.0f,1.0f));

		m_ForceFeedBackDebugDraw->position(RightEndPos.x, RightEndPos.y, RightEndPos.z);
		m_ForceFeedBackDebugDraw->colour(Ogre::ColourValue(0.0f,1.0f,0.0f,1.0f));
	}
	m_ForceFeedBackDebugDraw->end();

#endif
}
//========================================================================================
bool MisNewTraining::BeginRendOneFrame(float timeelpsed)//frameStarted(const Ogre::FrameEvent& evt)
{
	//
	DynObjMap::iterator itor = m_DynObjMap.begin();
	while(itor != m_DynObjMap.end())
	{
		MisMedicOrganInterface * organif = itor->second;
		organif->RefreshDirtyData();
		itor++;
	}

	EffectManager::Instance()->UpdateCamera(m_pOms);
	
	EffectManager::Instance()->BeginRendOneFrame(timeelpsed);

	//average connected node's position first
	//for(size_t e = 0 ; e < m_ObjEnvelops.size() ; e++)
	//	m_ObjEnvelops[e]->FrameStarted();

	//call "PreUpdateScene" organ's may calculate their normal tangent etc and 
	//other data need for rend
	DynObjMap::iterator objitor = m_DynObjMap.begin();
	while(objitor != m_DynObjMap.end())
	{
		MisMedicOrganInterface * oif = objitor->second;
		if(oif)
		{
			oif->PreUpdateScene(timeelpsed , m_pLargeCamera);
		}
		objitor++;
	}

	//average connected node's normal
	//to do group organ will get more cache-friend result
	for (size_t c = 0; c < m_ShadowNodesLinkage.size(); c++)
	{
		ShadowNodeForLinkage & oneLink = m_ShadowNodesLinkage[c];

		oneLink.m_AvgPosition = m_ShadowNodesLinkage[c].m_AvgNormal = Ogre::Vector3(0, 0, 0);
		oneLink.m_NumNodes = 0;
		bool hasStaticNode = false;

		std::set<ShadowNodeForLinkage::NodeBeLinked>::iterator itor = oneLink.m_LinkedNodes.begin();
		while (itor != oneLink.m_LinkedNodes.end())
		{
			PhysNode_Data & physData = itor->m_Organ->m_PhysNodeData[itor->m_PhysNodeDataId];

			if (physData.m_PhysNode->m_InvM > GP_EPSILON && hasStaticNode == false)
			{
				oneLink.m_AvgPosition += GPVec3ToOgre(physData.m_PhysNode->m_CurrPosition);
			}
			else
			{
				oneLink.m_AvgPosition = GPVec3ToOgre(physData.m_PhysNode->m_CurrPosition);
				oneLink.m_NumNodes = 1;
				hasStaticNode = true;
			}

			oneLink.m_AvgNormal += physData.m_AvgNormal;
			itor++;
		}
		if (hasStaticNode == false)
		    oneLink.m_AvgPosition /= ((float)oneLink.m_LinkedNodes.size());
		oneLink.m_AvgNormal.normalise();
	}


	DynObjMap::iterator fakeOrganItor = m_DynObjMapForNonOrgan.begin();
	while(fakeOrganItor != m_DynObjMapForNonOrgan.end())
	{
		MisMedicOrganInterface * oif = fakeOrganItor->second;
		if(oif)
		{
			oif->PreUpdateScene(timeelpsed , m_pLargeCamera);
		}
		fakeOrganItor++;
	}

	
	//call update Scene
	objitor = m_DynObjMap.begin();
	while(objitor != m_DynObjMap.end())
	{
		MisMedicOrganInterface * oif = objitor->second;
		if(oif)
		{
			oif->UpdateScene(timeelpsed , m_pLargeCamera);
		}
		objitor++;
	}

	fakeOrganItor = m_DynObjMapForNonOrgan.begin();
	while(fakeOrganItor != m_DynObjMapForNonOrgan.end())
	{
		MisMedicOrganInterface * oif = fakeOrganItor->second;
		if(oif)
		{
			oif->UpdateScene(timeelpsed , m_pLargeCamera);
		}
		fakeOrganItor++;
	}

    if (m_StaticDomeMeshPtr.isNull() == false)
    {
        m_StaticDynDomeUnion.UpdateStaticVertexByDynamic(m_StaticDomeMeshPtr);
    }

	for (size_t p = 0, np = m_WaterPools.size(); p < np; p++)
		m_WaterPools[p]->Update(timeelpsed);

	for (size_t s = 0, ns = m_SutureNeedles.size(); s < ns; s++)
		m_SutureNeedles[s]->UpdateMesh();

	for (size_t s = 0, ns = m_SutureNeedlesV2.size(); s < ns; s++)
		m_SutureNeedlesV2[s]->UpdateMesh();

	return true;
}

/*void MisNewTraining::NotifySoftBodyFaceDeleted(const std::set<GFPhysSoftBodyFace*> & facedelete)
{
	std::vector<VeinConnectObject*> veinconnects = GetVeinConnectObjects();
	for(size_t v = 0 ; v < veinconnects.size() ; v++)
	{
		veinconnects[v]->RemoveConnectPairWithFace(facedelete);
	}
}*/
//======================================================================================================
void MisNewTraining::ScisscorCutOrgan(CScissors * scissor , MisMedicOrgan_Ordinary * organToCut)
{
	//organToCut->CutOrganByTool(scissor);
}
//======================================================================================================
bool MisNewTraining::Update(float dt)
{
	DynObjMap::iterator itor = m_DynObjMap.begin();
	while(itor != m_DynObjMap.end())
	{
		MisMedicOrganInterface * organif = itor->second;
		organif->RefreshDirtyData();
		itor++;
	}
	

	//添加通电被影响的对象给tool
	if(m_pToolsMgr)
	{
		CTool * leftTool  = dynamic_cast<CTool*>(m_pToolsMgr->GetLeftTool());
		CTool * rightTool = dynamic_cast<CTool*>(m_pToolsMgr->GetRightTool());
		CTool * pTool = NULL;
		bool canAdded = false;

		if(leftTool && 
			leftTool->HasElectricAttribute() && 
			(leftTool->GetElectricLeftPad() || leftTool->GetElectricRightPad()))
		{
			canAdded = true;
		}
		else
			leftTool = NULL;

		if(rightTool && 
			rightTool->HasElectricAttribute() &&
			(rightTool->GetElectricLeftPad() || rightTool->GetElectricRightPad()))
		{
			canAdded = true;
		}
		else
			rightTool = NULL;

		if(canAdded)
		{
			MisMedicOrganInterface * pOrganInterface = NULL;
			//get hemoclip
			std::vector<MisMedicOrganAttachment*> attachments;
			for(DynObjMap::iterator itr = m_DynObjMap.begin();itr != m_DynObjMap.end();++itr)
			{
				pOrganInterface = itr->second;
				pOrganInterface->GetAttachment(MOAType_TiantumClip,attachments);
			}
			
			std::vector<ToolElectricCheckObject> checkedObjects;
			for(std::size_t i = 0;i < attachments.size();++i)
			{
				checkedObjects.push_back(ToolElectricCheckObject(attachments[i],ToolElectricCheckObject::OT_HemoClip,1.5f));
			}

			//get bileDuct
			pOrganInterface = GetOrgan(EDOT_COMMON_BILE_DUCT);
			if(pOrganInterface)
			{
				checkedObjects.push_back(ToolElectricCheckObject(pOrganInterface,ToolElectricCheckObject::OT_OrdinaryOrgan,0.5f));
			}

            //get Uterus
            pOrganInterface = GetOrgan(EODT_UTERUS);
            if(pOrganInterface)
            {
                checkedObjects.push_back(ToolElectricCheckObject(pOrganInterface,ToolElectricCheckObject::OT_OrdinaryOrgan,0.5f));
            }

// 			pOrganInterface = GetOrgan(EDOT_CYSTIC_DUCT);
// 			if(pOrganInterface)
// 			{
// 				checkedObjects.push_back(ToolElectricCheckObject(pOrganInterface,ToolElectricCheckObject::OT_OrdinaryOrgan,0.5f));
// 			}

			//add checked object
			if(leftTool)
			{
				for(size_t i = 0;i < checkedObjects.size();++i)
					leftTool->AddCheckObjectForToolElectric(checkedObjects[i]);
			}

			if(rightTool)
			{
				for(size_t i = 0;i < checkedObjects.size();++i)
					rightTool->AddCheckObjectForToolElectric(checkedObjects[i]);
			}
		}
	}

	bool result = CBasicTraining::Update(dt);
	

	CTipMgr::Instance()->Update(dt);
	
	//update physics 
	if(m_pToolsMgr->GetLeftTool())
	   m_pToolsMgr->GetLeftTool()->onFrameUpdateStarted(dt);

	if(m_pToolsMgr->GetRightTool())
	   m_pToolsMgr->GetRightTool()->onFrameUpdateStarted(dt);

	int PhysicsStepCount = PhysicsWrapper::GetSingleTon().UpdateWorld(dt);

	if(m_pToolsMgr->GetLeftTool())
	   m_pToolsMgr->GetLeftTool()->onFrameUpdateEnded();

	if(m_pToolsMgr->GetRightTool())
	   m_pToolsMgr->GetRightTool()->onFrameUpdateEnded();


	//burn hook
	//////////////////////////////////////////////////////////////////////////
	CTool * lefTool = (CTool*)(m_pToolsMgr->GetLeftTool());
	if (lefTool)
	{
		BurnHookedAndContactedConnect(lefTool,dt);
	}	
	CTool * rightTool = (CTool*)(m_pToolsMgr->GetRightTool());
	if (rightTool)
	{
		BurnHookedAndContactedConnect(rightTool,dt);
	}	
	//////////////////////////////////////////////////////////////////////////

	if(PhysicsStepCount > 0)
	   UpdateForceFeedBack();//force feed

	DetectExplore(dt);

	DetectLocation(dt);

	bool isCameraTouched = DetectLineSegmentIntersectOrgan();

	if (isCameraTouched)
		m_HasCameraTouchSomething = true;
	//update time of all timer
	for (auto itr = m_timers.begin(); itr != m_timers.end();){
		Timer* timer = *itr;
		timer->curTime += dt;

		if (timer->curTime >= timer->totalTime){
			OnTimerTimeout(timer->id, timer->curTime , timer->userData);
			delete timer;
			itr = m_timers.erase(itr);
		}
		else
			++itr;
	}

	//

	return result;
}
//======================================================================================================
bool MisNewTraining::Terminate()
{
	return CBasicTraining::Terminate();
}
//==========================================================================================================
void MisNewTraining::BuildObjectAdhesion(CXMLWrapperTraining * pTrainingConfig)
{
	for(size_t c = 0 ; c < pTrainingConfig->m_AdhereObject.size() ; c++)
	{
		CXMLWrapperAdhere * adhereconfig = pTrainingConfig->m_AdhereObject[c];

		if(adhereconfig->m_AdhereType == "Approach")
		{
			MisMedicOrgan_Ordinary * organA = dynamic_cast<MisMedicOrgan_Ordinary*>(GetOrgan(adhereconfig->m_Object1ID));

			MisMedicOrgan_Ordinary * organB = dynamic_cast<MisMedicOrgan_Ordinary*>(GetOrgan(adhereconfig->m_Object2ID));

			MisMedicObjLink_Approach * approachlink = new MisMedicObjLink_Approach(adhereconfig->m_HideLinkFace);
			
			approachlink->LinkTwoOrgans(*organA , *organB , adhereconfig->m_Range , adhereconfig->m_Ratio);

            m_ObjAdhersions.push_back(approachlink);

			//create shadow nodes for linkage
			GFPhysAlignedVectorObj<PhysNode_Data> & NodeDataA = approachlink->m_ConnectOrganA->m_PhysNodeData;
			GFPhysAlignedVectorObj<PhysNode_Data> & NodeDataB = approachlink->m_ConnectOrganB->m_PhysNodeData;

			for (size_t t = 0; t < approachlink->m_NodeToNodeLinks.size(); t++)
			{
				if (approachlink->m_NodeToNodeLinks[t].m_IsValid)
				{
					GFPhysSoftBodyNode * nodeA = approachlink->m_NodeToNodeLinks[t].m_NodeInA;
					GFPhysSoftBodyNode * nodeB = approachlink->m_NodeToNodeLinks[t].m_NodeInB;

					int indexA = (int)nodeA->m_UserPointer;
					int indexB = (int)nodeB->m_UserPointer;

					bool ValidA = (indexA >= 0 && indexA < (int)NodeDataA.size());
					bool ValidB = (indexB >= 0 && indexB < (int)NodeDataB.size());

					if (ValidA && ValidB)
					{
						int sa = NodeDataA[indexA].m_ShadowNodeIndex;
							
						int sb = NodeDataB[indexB].m_ShadowNodeIndex;

						if (sa < 0 && sb < 0)
						{
							ShadowNodeForLinkage shadowNodes;
							shadowNodes.m_LinkedNodes.insert(ShadowNodeForLinkage::NodeBeLinked(approachlink->m_ConnectOrganA, indexA));
							shadowNodes.m_LinkedNodes.insert(ShadowNodeForLinkage::NodeBeLinked(approachlink->m_ConnectOrganB, indexB));

							NodeDataA[indexA].m_ShadowNodeIndex = NodeDataB[indexB].m_ShadowNodeIndex = (int)m_ShadowNodesLinkage.size();
							m_ShadowNodesLinkage.push_back(shadowNodes);
						}
						else if (sa < 0)
						{
							m_ShadowNodesLinkage[sb].m_LinkedNodes.insert(ShadowNodeForLinkage::NodeBeLinked(approachlink->m_ConnectOrganA, indexA));
                   			NodeDataA[indexA].m_ShadowNodeIndex = sb;
						}
						else if (sb < 0)
						{
							m_ShadowNodesLinkage[sa].m_LinkedNodes.insert(ShadowNodeForLinkage::NodeBeLinked(approachlink->m_ConnectOrganB, indexB));
							NodeDataB[indexB].m_ShadowNodeIndex = sa;
						}
						else if (sa != sb)//merge B -> A
						{
							std::set<ShadowNodeForLinkage::NodeBeLinked>::iterator itor = m_ShadowNodesLinkage[sb].m_LinkedNodes.begin();
  
							while (itor != m_ShadowNodesLinkage[sb].m_LinkedNodes.end())
							{
								MisMedicOrgan_Ordinary * oragnBeMerge = itor->m_Organ;
								
								int dataIndexBeMerge = itor->m_PhysNodeDataId;
								
								m_ShadowNodesLinkage[sa].m_LinkedNodes.insert(ShadowNodeForLinkage::NodeBeLinked(oragnBeMerge, dataIndexBeMerge));

								oragnBeMerge->m_PhysNodeData[dataIndexBeMerge].m_ShadowNodeIndex = sa;

								itor++;
							}
							m_ShadowNodesLinkage[sb].m_LinkedNodes.clear();
						}
					}
				}
			}

		}
		else
		{
			MisMedicObjectAdhersion * adhersion = new MisMedicObjectAdhersion();
			adhersion->BuildObjectAdhesion(adhereconfig , this);
			m_ObjAdhersions.push_back(adhersion);
		}	
	}
}
//======================================================================================================
void MisNewTraining::BuildAdhesionClusters(CXMLWrapperTraining * pTrainingConfig)
{
	for(size_t a = 0 ; a < pTrainingConfig->m_AdhesionClusters.size() ; a++)
	{
		CXMLWrapperAdhesionCluster * pAdhersionClusterConfig = pTrainingConfig->m_AdhesionClusters[a];

		std::set<int> connectOrgansID;

		if(pAdhersionClusterConfig->m_flag_OrganIDs)
		{
			Ogre::vector<Ogre::String>::type  organIDStrs = Ogre::StringUtil::split(pAdhersionClusterConfig->m_OrganIDs , ",");
			for(size_t f = 0 ; f < organIDStrs.size() ; f++)
			{
				Ogre::String & str = organIDStrs[f];
				int organID = Ogre::StringConverter::parseInt(str);
				connectOrgansID.insert(organID);
			}
		}
		MisMedicOrgan_Ordinary *pAdhesionOrgan = dynamic_cast<MisMedicOrgan_Ordinary*>(GetOrgan(pAdhersionClusterConfig->m_AdhesionID));
		if(pAdhesionOrgan) 
		{
			MisMedicAdhesionCluster *pAdhesionCluster = new MisMedicAdhesionCluster(pAdhesionOrgan , pAdhersionClusterConfig );
			pAdhesionCluster->BuildAdhesionCluster(pAdhersionClusterConfig , this , connectOrgansID);
		}
	}
}
//======================================================================================================
void MisNewTraining::FindAdhesionClustersConnections(CXMLWrapperTraining * pTrainingConfig)
{
	for(size_t a = 0 ; a < pTrainingConfig->m_AdhesionClusters.size() ; a++)
	{
		CXMLWrapperAdhesionCluster * pAdhersionClusterConfig = pTrainingConfig->m_AdhesionClusters[a];

		std::set<int> connectOrgansID;

		if(pAdhersionClusterConfig->m_flag_OrganIDs)
		{
			Ogre::vector<Ogre::String>::type  organIDStrs = Ogre::StringUtil::split(pAdhersionClusterConfig->m_OrganIDs , ",");
			for(size_t f = 0 ; f < organIDStrs.size() ; f++)
			{
				Ogre::String & str = organIDStrs[f];
				int organID = Ogre::StringConverter::parseInt(str);
				connectOrgansID.insert(organID);
			}
		}

		MisMedicOrgan_Ordinary *pAdhesionOrgan = dynamic_cast<MisMedicOrgan_Ordinary*>(GetOrgan(pAdhersionClusterConfig->m_AdhesionID));
		if(pAdhesionOrgan) 
		{
			std::map<MisMedicOrgan_Ordinary* , MisMedicAdhesionCluster*>::iterator AdhesionItor = m_Adhesions.find(pAdhesionOrgan);
			if(AdhesionItor == m_Adhesions.end())
			{
				MisMedicAdhesionCluster *pAdhesionCluster = new MisMedicAdhesionCluster(pAdhesionOrgan , pAdhersionClusterConfig);
				bool result = pAdhesionCluster->FindAllConnection(pAdhersionClusterConfig , this , connectOrgansID);
				if(result)
					m_Adhesions[pAdhesionOrgan] = pAdhesionCluster;
				else
					delete pAdhesionCluster;
			}
			else
			{
				//todo
			}
		}
	}

	std::map<MisMedicOrgan_Ordinary* , MisMedicAdhesionCluster*>::iterator AdhesionItor = m_Adhesions.begin();
	while (AdhesionItor != m_Adhesions.end())
	{
		MisMedicOrgan_Ordinary *pAdhesion = AdhesionItor->first;
		MisMedicAdhesionCluster *pAdhesionCluster = AdhesionItor->second;
		if(pAdhesionCluster->IsAdhesionScale())
		{
			GFPhysVector3 scaleDir;
			if(pAdhesionCluster->IsAdhesionAutoScale())
			{
				int regionNum = pAdhesionCluster->ComputeConnectionRegionV2();
				if(regionNum >= 2)
				{
					scaleDir =pAdhesionCluster->GetScaleDir();
				}
			}
			else
				scaleDir =pAdhesionCluster->GetScaleDir();

			pAdhesion->ScaleSerializerNodeByDir(pAdhesionCluster->GetAdhesionScaleFactor(),scaleDir);
		}
	
		AdhesionItor++;
	}
}
//======================================================================================================
void MisNewTraining::BuildMergedObjectConstructInFo(CXMLWrapperTraining * pTrainingConfig, std::vector<MisMedicDynObjConstructInfo> & objectBorn, std::set<int> & objectBeMerged)
{
	for(size_t c = 0 ; c < pTrainingConfig->m_AdhereObject.size() ; c++)
	{
		CXMLWrapperAdhere * adhereconfig = pTrainingConfig->m_AdhereObject[c];

		if( adhereconfig->m_AdhereType == "merge")
		{
			Ogre::String mergestr = adhereconfig->m_MergedObjectIDS;

			Ogre::vector<Ogre::String>::type idstrs = Ogre::StringUtil::split(mergestr, ",");

			MisMedicDynObjConstructInfo newobject;
			
			for (int t = 0; t < (int)idstrs.size(); t++)
			{
				 int objID = Ogre::StringConverter::parseInt(idstrs[t]);
				
				 objectBeMerged.insert(objID);

				 for (int i = 0; i < (int)pTrainingConfig->m_DynamicScene.size(); i++)
				 {
					 CXMLWrapperOrgan * organconfig = pTrainingConfig->m_DynamicScene[i];

					 if (organconfig != 0 && organconfig->m_Type == objID)
					 {
						std::string mmsfile = organconfig->m_PhysicFile;
						std::string layername = organconfig->m_LayerName;
						if (t == 0)//read first object as config
						{
							newobject.ReadParameter(organconfig);
						}
						newobject.m_MergedObjMMSFileNames.push_back(mmsfile);
						
						newobject.m_MergedObjIDS.push_back(objID);
						
						newobject.m_MergedObjLayerNames.push_back(layername);

						newobject.m_MergedObjLayerMaterials.push_back(organconfig->m_MaterialName);
						break;
					 }
				 }
			}

			if (newobject.m_objTopologyType == DOT_VOLMESH)
			    objectBorn.push_back(newobject);
		}
	}
}
//======================================================================================================
void MisNewTraining::TranslateOrgans(CXMLWrapperTraining * pTrainingConfig)
{
	for(size_t c = 0 ; c < pTrainingConfig->m_OrganTranslations.size() ; c++)
	{
		CXMLWrapperOrganTranslation * translation = pTrainingConfig->m_OrganTranslations[c];
		MisMedicOrganInterface *oif = GetOrgan(translation->m_OrganID);
		MisMedicOrgan_Ordinary *organ = dynamic_cast<MisMedicOrgan_Ordinary*>(oif);
		if(organ)
		{
			organ->TranslateUndeformedPosition(OgreToGPVec3(translation->m_Offset));
			organ->CreatePoseRigidForce();
		}
	}
}
//======================================================================================================
void MisNewTraining::AddWaterPools(CXMLWrapperTraining * pTrainingConfig)
{
	for(size_t c = 0 ; c < pTrainingConfig->m_WaterPools.size() ; c++)
	{
		CXMLWrapperWaterPool * pWaterPoolConfig = pTrainingConfig->m_WaterPools[c];

		WaterPool * pWaterPool = new WaterPool(this ,pWaterPoolConfig->m_MeshWidth , 
																				pWaterPoolConfig->m_MeshHeight ,
																				pWaterPoolConfig->m_ActualWidth ,
																				pWaterPoolConfig->m_ActualHeight ,
																				pWaterPoolConfig->m_Origin ,
																				pWaterPoolConfig->m_Normal,
																				pWaterPoolConfig->m_MaxHeight,
																				pWaterPoolConfig->m_CenterAndStageHeight,
																				pWaterPoolConfig->m_IsCenterChangeBySuction,
																				pWaterPoolConfig->m_MeshName);
		pWaterPool->SetCanSoakOrgans(pWaterPoolConfig->m_IsSoakOrgans);

		if(pWaterPoolConfig->m_flag_RejectOrganIDs)
		{
			std::set<int> rejectOrganIds;
			Ogre::vector<Ogre::String>::type  organIDStrs = Ogre::StringUtil::split(pWaterPoolConfig->m_RejectOrganIDs , ",");
			for(size_t f = 0 ; f < organIDStrs.size() ; f++)
			{
				Ogre::String & str = organIDStrs[f];
				int organID = Ogre::StringConverter::parseInt(str);
				rejectOrganIds.insert(organID);
			}
			pWaterPool->SetRejectOrgan(rejectOrganIds);
		}

		m_WaterPools.push_back(pWaterPool);
	}

	//
	DynObjMap::iterator itor = m_DynObjMap.begin();
	while(itor != m_DynObjMap.end())
	{
		MisMedicOrganInterface *oif = itor->second;
		for(size_t p = 0 ; p < m_WaterPools.size() ; p++)
		{
			if(!m_WaterPools[p]->IsReject(oif->m_OrganID))
				oif->AddOrganActionListener(m_WaterPools[p]);
		}
		itor++;
	}
}
//======================================================================================================
void MisNewTraining::AddViewDetections(CXMLWrapperTraining *pTrainingConfig)
{
	for(size_t v = 0 ; v < pTrainingConfig->m_ViewDetections.size() ; v++)
	{
		CXMLWrapperViewDetection * pViewDetectionConfig = pTrainingConfig->m_ViewDetections[v];
	
		if(pViewDetectionConfig->m_Type == "Explore")
		{
			ViewDetection *pViewDetection = new ViewDetection(pViewDetectionConfig->m_Position, pViewDetectionConfig->m_MinCos , pViewDetectionConfig->m_IsDebug);

			if(pViewDetection)
			{
				pViewDetection->DetectDist(pViewDetectionConfig->m_IsDetectDist);
				pViewDetection->SetDetectDist(pViewDetectionConfig->m_DetectDist);
				m_ExploreDetections.push_back(pViewDetection);
			}
		}
		else if(pViewDetectionConfig->m_Type == "Location")
		{
			if(m_pLocationDetection)
				delete m_pLocationDetection;

			m_pLocationDetection = new ViewDetection(pViewDetectionConfig->m_Position , pViewDetectionConfig->m_MinCos  , pViewDetectionConfig->m_IsDebug);
			m_pLocationDetection->DetectDist(pViewDetectionConfig->m_IsDetectDist);
			m_pLocationDetection->SetDetectDist(pViewDetectionConfig->m_DetectDist);
		}
	}

	if(m_ExploreDetections.size() != 0)
		m_NeedExplore = true;
}
//======================================================================================================
void MisNewTraining::BuildVeinConnectionConstraint()
{
	DynObjMap::iterator itor = m_DynObjMap.begin();
	while(itor != m_DynObjMap.end())
	{
		MisMedicOrganInterface *oif = itor->second;
		if(oif->GetCreateInfo().m_objTopologyType == DOT_VEINCONNECT)
		{
			VeinConnectObject *veinobj = dynamic_cast<VeinConnectObject*>(oif);
			if(veinobj)
				veinobj->BuildConnectConstraint(m_DynObjMap);
		}
		itor++;
	}
}
//======================================================================================================
bool MisNewTraining::DetectExplore(float dt)
{
	if(m_NeedExplore)
	{
		int exploreNum = 0;
		std::vector<ViewDetection*>::iterator itor = m_ExploreDetections.begin();
		while(itor != m_ExploreDetections.end())
		{
			ViewDetection * pVd = *itor;

			if(pVd->IsValid())
			{
				bool result = pVd->Update(dt , m_pLargeCamera);

				if(pVd->m_IsDebug)
				{
					int colorIndex = result ? 1 : 0;
					pVd->Draw(m_pLargeCamera , colorIndex);
				}

				if(result && !pVd->m_IsDebug)
					pVd->SetValid(false);
			}
			else
				exploreNum++;

			++itor;
		}
		if(exploreNum == m_ExploreDetections.size())
		{
			m_NeedExplore = false;
			m_ExploreResult = true;
		}
	}
	return m_ExploreResult;
}
//======================================================================================================
bool MisNewTraining::DetectLocation(float dt)
{
	if(m_pLocationDetection)
	{
		bool locateResult = m_pLocationDetection->Update(dt , m_pLargeCamera);

		if(m_pLocationDetection->m_IsDebug)
		{
			int colorIndex = locateResult ? 1 : 0;
			m_pLocationDetection->Draw(m_pLargeCamera , colorIndex);
		}

		return locateResult;
	}

	return false;
}

//======================================================================================================
void MisNewTraining::GetAllMaterialReceiveShadow(std::vector<Ogre::String> & matRecShadow)
{
	DynObjMap::iterator itor = m_DynObjMap.begin();

	while(itor != m_DynObjMap.end())
	{
		MisMedicOrgan_Ordinary * organ = dynamic_cast<MisMedicOrgan_Ordinary *>(itor->second);
		if(organ != 0)
		{
			matRecShadow.push_back(organ->getMaterialName());
		}
		itor++;
	}
}
//======================================================================================================
void MisNewTraining::CreateTrainingScene(CXMLWrapperTraining * pTrainingConfig)
{
	if(pTrainingConfig->m_flag_GravityDir)
	{
	   m_SceneGravityDir = pTrainingConfig->m_GravityDir;
	}
	
	std::vector<MisMedicDynObjConstructInfo> DstObjectToConstruct;
	
	std::set<int> objectBeUnioned;
	
	BuildMergedObjectConstructInFo(pTrainingConfig , DstObjectToConstruct , objectBeUnioned);

	for (int i = 0; i < (int)pTrainingConfig->m_DynamicScene.size(); i++)
	{
		CXMLWrapperOrgan * organconfig = pTrainingConfig->m_DynamicScene[i];
		
		if(organconfig != 0 && objectBeUnioned.find(organconfig->m_Type) == objectBeUnioned.end())//not unioned object
		{
			//read config to construct struct
			MisMedicDynObjConstructInfo cs;
			cs.ReadParameter(organconfig);//(pTrainingConfig , i);
			if(organconfig->m_CanAutoCreate)
				DstObjectToConstruct.push_back(cs);
			else
				m_reservedConstructInfos.push_back(cs);
		}
	}

	for(auto& cs : m_reservedConstructInfos){
		if(cs.m_HasCustomGravityDir == false)
			cs.m_CustomGravityDir = m_SceneGravityDir;
	}

	//MisMedicOrganOrdinary的只读取文件
	std::vector<int> connectConfigIndex;
	for(size_t c = 0 ; c < DstObjectToConstruct.size(); c++)
	{
		MisMedicDynObjConstructInfo & cs = DstObjectToConstruct[c];
		if(cs.m_HasCustomGravityDir == false)
		   cs.m_CustomGravityDir = m_SceneGravityDir;

		if(cs.m_objTopologyType == DOT_VOLMESH || cs.m_objTopologyType == DOT_MEMBRANE)
		{  
			MisMedicOrgan_Ordinary * organobject = new MisMedicOrgan_Ordinary(cs.m_OrganType ,cs.m_OrganId, this);
			if(organobject)
			{
				organobject->ReadOrganObjectFile(cs);

				m_DynObjMap.insert(std::make_pair(organobject->m_OrganID , organobject));//m_dynamicObjects.push_back(organif);
			}
		}
		else if(cs.m_objTopologyType == DOT_TUBE)
		{
			MisMedicOrganInterface * organif = LoadOrganism(cs, this);
			if(organif)
				m_DynObjMap.insert(std::make_pair(organif->m_OrganID , organif));//m_dynamicObjects.push_back(organif);
		}
		else if (cs.m_objTopologyType == DOT_UNDEF)
		{
			MisMedicOrganInterface * organif = LoadOrganism(cs, this);
			if(organif)
			{
			   organif->m_OrganID = m_AutoGenOrganID++;
			   m_DynObjMap.insert(std::make_pair(organif->m_OrganID , organif));//m_dynamicObjects.push_back(organif);
			}
		}
		else if(cs.m_objTopologyType == DOT_VEINCONNECT)
		{
			connectConfigIndex.push_back(c);
		}
	}

	OnAllOrdinaryOrganReadObjectFile(pTrainingConfig);

	//MisMedicOrganOrdinary真正创建
	for(size_t c = 0 ; c < DstObjectToConstruct.size(); c++)
	{
		MisMedicDynObjConstructInfo & cs = DstObjectToConstruct[c];
        if(cs.m_objTopologyType == DOT_VOLMESH || cs.m_objTopologyType == DOT_MEMBRANE)
        {
            DynObjMap::iterator OrdinaryOrganItor = m_DynObjMap.find(cs.m_OrganId);

            if(OrdinaryOrganItor != m_DynObjMap.end())
            {
                MisMedicOrganInterface * organif  = OrdinaryOrganItor->second;
                organif->Create(cs);
            }
        }
	}
	BuildOrgansVolumeTextureCoord();
	OnAllOrdinaryOrganCreated(pTrainingConfig);
    
	BuildObjectAdhesion(pTrainingConfig);
	TranslateOrgans(pTrainingConfig);

	//BuildAdhesionClusters(pTrainingConfig);
	
	//final build connect
	for(size_t c = 0 ; c < connectConfigIndex.size() ; c++)
	{
		MisMedicDynObjConstructInfo & cs = DstObjectToConstruct[connectConfigIndex[c]];

		MisMedicOrganInterface * organif = LoadOrganism(cs, this);

		if(cs.m_objTopologyType == DOT_VEINCONNECT)
		{
			//m_connectobjets.push_back(dynamic_cast<VeinConnectObject*>(organif));
			m_DynObjMap.insert(std::make_pair(organif->m_OrganID , organif));
		}
		else if(cs.m_objTopologyType == DOT_NEWVEINCONNECT)
		{
			m_DynObjMap.insert(std::make_pair(organif->m_OrganID , organif));
		}
		else if(cs.m_objTopologyType == DOT_VEINCONNECTV2)
		{
			//m_veinObjV2s.push_back(dynamic_cast<VeinConnectObjectV2*>(organif));
			m_DynObjMap.insert(std::make_pair(organif->m_OrganID , organif));
		}
	}


	//call "postload"
	DynObjMap::iterator itor = m_DynObjMap.begin();
	while(itor != m_DynObjMap.end())
	{
		itor->second->PostLoadScene();
		itor++;
	}

	AddWaterPools(pTrainingConfig);
	
	AddViewDetections(pTrainingConfig);

	WaterManager * pWaterMgr = EffectManager::Instance()->GetWaterManager();
	if(pWaterMgr)
	{
		pWaterMgr->SetCurrTraining(this);
		pWaterMgr->SetGravity(m_SceneGravityDir);
	}
}
//================================================================================================
void MisNewTraining::OnAllOrdinaryOrganReadObjectFile(CXMLWrapperTraining * pTrainingConfig)
{
//=============================//=============================//=============================
	FindAdhesionClustersConnections(pTrainingConfig);
	
//=============================//=============================//=============================
}
//================================================================================================

void MisNewTraining::OnAllOrdinaryOrganCreated(CXMLWrapperTraining * pTrainingConfig)
{
	std::map<MisMedicOrgan_Ordinary* , MisMedicAdhesionCluster*>::iterator AdhesionItor = m_Adhesions.begin();
	while (AdhesionItor != m_Adhesions.end())
	{
		MisMedicOrgan_Ordinary *pAdhesion = AdhesionItor->first;
		MisMedicAdhesionCluster *pAdhesionCluster = AdhesionItor->second;
		pAdhesionCluster->BuildAllConnection();
		AdhesionItor++;
	}
}
//================================================================================================
MisMedicOrganInterface * MisNewTraining::LoadOrganism(MisMedicDynObjConstructInfo & cs, MisNewTraining *pTrain)//(CXMLWrapperTraining * pTrainingConfig, int i)
{ 
	if (cs.m_objTopologyType == DOT_UNDEF)	// this should be rigid
	{
		Ogre::String strEntityName = Ogre::String(cs.m_name);//(organconfig->m_Name) 
			+ "$" + Ogre::StringConverter::toString(m_pOms->s_nLoadCount);  
		m_pOms->s_nLoadCount++;
		
		MisMedicRigidPrimtive * rigidShape = new MisMedicRigidPrimtive(DOT_UNDEF, this);
		
		rigidShape->Create(cs, m_pOms->GetSceneManager() , strEntityName);

		return rigidShape;
 	}
	else if(cs.m_objTopologyType == DOT_VOLMESH || cs.m_objTopologyType == DOT_MEMBRANE)
	{
		MisMedicOrgan_Ordinary * organobject = new MisMedicOrgan_Ordinary(cs.m_OrganType ,cs.m_OrganId, this);
		organobject->Create(cs);
		return organobject;
	}
	
	else if(cs.m_objTopologyType == DOT_VEINCONNECT)
	{
		VeinConnectObject * veinobj = new VeinConnectObject(this);
		veinobj->m_OrganID = cs.m_OrganType;
		veinobj->Create(cs , m_DynObjMap);
		return veinobj;
	}
	/*else if(cs.m_objTopologyType == DOT_NEWVEINCONNECT)
	{
		NewVeinConnectObject *newveinobj = new NewVeinConnectObject(this);
		newveinobj->m_OrganID = cs.m_OrganType;
		newveinobj->Create(cs,m_DynObjMap);
		return newveinobj;
	}*/
	/*else if(cs.m_objTopologyType == DOT_VEINCONNECTV2)
	{
		VeinConnectObjectV2 * veinobj = new VeinConnectObjectV2(this);
		veinobj->m_OrganID = cs.m_OrganType;
		veinobj->Create(cs , m_DynObjMap);
		return veinobj;
	}*/

	return 0;
}
//================================================================================================
void MisNewTraining::RemoveOrganFromWorld(MisMedicOrganInterface * organ)
{
	if(organ)
	{
	   int OrganID = organ->m_OrganID;
	   DynObjMap::iterator orgnitor = m_DynObjMap.find(OrganID);
	   if(orgnitor != m_DynObjMap.end() && orgnitor->second == organ)//organ in this train
	   {
		  m_DynObjMap.erase(orgnitor);

		  //call listeners
		  if(m_pToolsMgr)
		  {
			 CTool * leftTool  = dynamic_cast<CTool*>(m_pToolsMgr->GetLeftTool());
			 
			 CTool * rightTool = dynamic_cast<CTool*>(m_pToolsMgr->GetRightTool());
			 
			 if(leftTool)
			    leftTool->OnOrganBeRemoved(organ);

			 if(rightTool)
				rightTool->OnOrganBeRemoved(organ);
		  }
		  //now do real remove and delete
		  organ->RemoveFromWorld();
		  delete organ;
	   }
	}
}
//==========================================================================================
void MisNewTraining::RemoveOrganFromWorld(int organID)
{
	DynObjMap::iterator orgnitor = m_DynObjMap.find(organID);
	if (orgnitor != m_DynObjMap.end())
	{
		RemoveOrganFromWorld(orgnitor->second);
	}

}
//=================================================================================================
void MisNewTraining::BuildOrgansVolumeTextureCoord()
{
	DynObjMap::iterator itor = m_DynObjMap.begin();
	while (itor != m_DynObjMap.end())
	{
		MisMedicOrgan_Ordinary * organ = dynamic_cast<MisMedicOrgan_Ordinary*>(itor->second);

		if (organ && organ->GetCreateInfo().m_objTopologyType == DOT_VOLMESH)
		{
			organ->BuildTetrahedronNodeTextureCoord(GFPhysVector3(0, 0, 0), false);
		}

		itor++;
	}
}
//================================================================================================
MisMedicOrganInterface *MisNewTraining::AddFakeOrgan(MisMedicDynObjConstructInfo & cs)
{
	cs.m_OrganId = m_DynObjMapForNonOrgan.size();	
	MisMedicOrganInterface *pOrgan = LoadOrganism(cs,this);
	if(pOrgan)
	{
		m_DynObjMapForNonOrgan[pOrgan->m_OrganID] = pOrgan;
		return pOrgan;
	}
	return NULL;
}
//=========================================================================================================
MisMedicOrganInterface * MisNewTraining::ManuallyCreateOrgan(MisMedicDynObjConstructInfo & cs)
{
	MisMedicOrganInterface * pOrgan = LoadOrganism(cs, this);

	if (cs.m_objTopologyType == DOT_UNDEF)
	{
		pOrgan->m_OrganID = m_AutoGenOrganID++;
	}

	m_DynObjMap.insert(std::make_pair(pOrgan->m_OrganID, pOrgan));

	return pOrgan;
}
//================================================================================================
void MisNewTraining::RemoveFakeOrganFromWorld(MisMedicOrganInterface * pOrgan)
{
	if(pOrgan)
	{
		int OrganID = pOrgan->m_OrganID;
		DynObjMap::iterator orgnitor = m_DynObjMapForNonOrgan.find(OrganID);
		if(orgnitor != m_DynObjMapForNonOrgan.end() && orgnitor->second == pOrgan)//organ in this train
		{
			m_DynObjMapForNonOrgan.erase(orgnitor);

			//call listeners
			if(m_pToolsMgr)
			{
				CTool * leftTool  = dynamic_cast<CTool*>(m_pToolsMgr->GetLeftTool());

				CTool * rightTool = dynamic_cast<CTool*>(m_pToolsMgr->GetRightTool());

				if(leftTool)
					leftTool->OnOrganBeRemoved(pOrgan);

				if(rightTool)
					rightTool->OnOrganBeRemoved(pOrgan);
			}
			//now do real remove and delete
			pOrgan->RemoveFromWorld();
			delete pOrgan;
			pOrgan = NULL;
		}
	}
}
//================================================================================================
MisMedicThreadRope * MisNewTraining::CreateRopeThread(Ogre::SceneManager * scenemgr)
{
	MisMedicThreadRope * rope = new MisMedicThreadRope(scenemgr , this);
	m_ThreadRopes.push_back(rope);

	return rope;
}
MisMedicThreadRope * MisNewTraining::CreateSimpleLooper(Ogre::SceneManager * scenemgr)
{
	MisMedicThreadRope * rope = new MisMedicSimpleLooper(scenemgr , this);
	m_ThreadRopes.push_back(rope);

	return rope;
}
//================================================================================================
SutureNeedle * MisNewTraining::CreateNeedle(Ogre::SceneManager * scenemgr, int threadnodenum, Real restlen,const Ogre::String & needleskeleton)
{
    SutureNeedle * needle = new SutureNeedle();
    needle->CreateSutureNeedle(this, threadnodenum, restlen, needleskeleton);
    m_SutureNeedles.push_back(needle);

    return needle;
}

//================================================================================================
SutureNeedleV2 * MisNewTraining::CreateNeedleV2(Ogre::SceneManager * scenemgr, int threadnodenum, Real restlen, const Ogre::String & needleskeleton)
{
	SutureNeedleV2 * needle = new SutureNeedleV2();
	needle->CreateSutureNeedleV2(this, threadnodenum, restlen, needleskeleton);
	m_SutureNeedlesV2.push_back(needle);

	return needle;
}
//================================================================================================
MisMedicTubeBody * MisNewTraining::CreateTubeThread(Ogre::SceneManager * scenemgr)
{
    MisMedicTubeBody * tube = new MisMedicTubeBody(scenemgr , this);
    m_TubeBodies.push_back(tube);

    return tube;
}
//=====================================================================================================
ACTubeShapeObject * MisNewTraining::CreateSoftTube(int ObjID, 
	                                               const GFPhysVector3 & center, 
												   float ringRadius, 
												   float radius)
{
	if (m_DynObjMap.find(ObjID) != m_DynObjMap.end())
	{
		MessageBox(0, _T("redundantID!!"), _T("redundantID!!"), 0);
		return 0;
	}
	ACTubeShapeObject * softTube = new ACTubeShapeObject(0, this);
	softTube->CreateToturs(MXOgre_SCENEMANAGER, center, GFPhysVector3(0, 1, 0), ringRadius);// 0.5f);
	m_DynObjMap.insert(std::make_pair(ObjID, softTube));
	return softTube;
}
//================================================================================================
bool MisNewTraining::RemoveThreadRopeFromWorld(MisMedicThreadRope * rope)
{
	for(size_t c = 0 ;c < m_ThreadRopes.size() ; c++)
	{
		if(m_ThreadRopes[c] == rope)
		{
			//remove from list
			m_ThreadRopes.erase(m_ThreadRopes.begin()+c);

			//call listeners
			if(m_pToolsMgr)
			{
				CTool * leftTool  = dynamic_cast<CTool*>(m_pToolsMgr->GetLeftTool());

				CTool * rightTool = dynamic_cast<CTool*>(m_pToolsMgr->GetRightTool());

				if(leftTool)
				   leftTool->OnCustomSimObjBeRemovedFromWorld(rope);

				if(rightTool)
				   rightTool->OnCustomSimObjBeRemovedFromWorld(rope);
			}

			//now do real remove and delete
			delete rope;
			return true;
		}
	}
	return false;
}
//================================================================================================
bool MisNewTraining::OnRemoveSutureThreadFromWorld(SutureThread * rope)
{
	if(m_pToolsMgr)
	{		
		CTool * leftTool  = dynamic_cast<CTool*>(m_pToolsMgr->GetLeftTool());

		CTool * rightTool = dynamic_cast<CTool*>(m_pToolsMgr->GetRightTool());

		if(leftTool)
		   leftTool->OnCustomSimObjBeRemovedFromWorld(rope);

		if(rightTool)
		   rightTool->OnCustomSimObjBeRemovedFromWorld(rope);
		return true;
	}
	return false;
}

bool MisNewTraining::OnRemoveSutureThreadFromWorld(SutureThreadV2 * rope)
{
	if (m_pToolsMgr)
	{
		CTool * leftTool = dynamic_cast<CTool*>(m_pToolsMgr->GetLeftTool());

		CTool * rightTool = dynamic_cast<CTool*>(m_pToolsMgr->GetRightTool());

		if (leftTool)
			leftTool->OnCustomSimObjBeRemovedFromWorld(rope);

		if (rightTool)
			rightTool->OnCustomSimObjBeRemovedFromWorld(rope);
		return true;
	}
	return false;
}

//================================================================================================
bool MisNewTraining::RemoveNeedleFromWorld(SutureNeedle * needle)
{
    for(size_t c = 0 ;c < m_SutureNeedles.size() ; c++)
    {
        if(m_SutureNeedles[c] == needle)
        {
			OnRemoveSutureThreadFromWorld(needle->GetSutureThread());
            //remove from list
            m_SutureNeedles.erase(m_SutureNeedles.begin()+c);

			//call listeners
			if(m_pToolsMgr)
			{
				CTool * leftTool  = dynamic_cast<CTool*>(m_pToolsMgr->GetLeftTool());

				CTool * rightTool = dynamic_cast<CTool*>(m_pToolsMgr->GetRightTool());

				if(leftTool)
				   leftTool->OnRigidBodyBeRemoved(needle->GetPhysicBody());

				if(rightTool)
				   rightTool->OnRigidBodyBeRemoved(needle->GetPhysicBody());
			}

            //now do real remove and delete
            delete needle;
            return true;
        }
    }
    return false;
}

bool MisNewTraining::RemoveNeedleFromWorld(SutureNeedleV2 * needle)
{
	for (size_t c = 0; c < m_SutureNeedlesV2.size(); c++)
	{
		if (m_SutureNeedlesV2[c] == needle)
		{
			OnRemoveSutureThreadFromWorld(needle->GetSutureThread());
			//remove from list
			m_SutureNeedlesV2.erase(m_SutureNeedlesV2.begin() + c);

			//call listeners
			if (m_pToolsMgr)
			{
				CTool * leftTool = dynamic_cast<CTool*>(m_pToolsMgr->GetLeftTool());

				CTool * rightTool = dynamic_cast<CTool*>(m_pToolsMgr->GetRightTool());

				if (leftTool)
					leftTool->OnRigidBodyBeRemoved(needle->GetPhysicBody());

				if (rightTool)
					rightTool->OnRigidBodyBeRemoved(needle->GetPhysicBody());
			}

			//now do real remove and delete
			delete needle;
			return true;
		}
	}
	return false;
}

//================================================================================================
bool MisNewTraining::RemoveTubeBodyFromWorld(MisMedicTubeBody * tube)
{
    for(size_t c = 0 ;c < m_TubeBodies.size() ; c++)
    {
        if(m_TubeBodies[c] == tube)
        {
            //remove from list
            m_TubeBodies.erase(m_TubeBodies.begin()+c);

            //now do real remove and delete
            delete tube;
            return true;
        }
    }
    return false;
}
//==============================================================================
bool MisNewTraining::HasOperateItemConfig()
{
	if(m_pTrainingConfig && (m_pTrainingConfig->m_OperateItems.size() || m_pTrainingConfig->m_CommonOperateItems.size()))
		return true;
	else
		return false;
}
//==============================================================================
SYScoreTable* MisNewTraining::GetScoreTable()
{
	return nullptr;
}
//==============================================================================
void MisNewTraining::LoadConfig(CXMLWrapperTraining * pTrainingConfig)
{
	CBasicTraining::LoadConfig(pTrainingConfig);
	LoadOperateItem(pTrainingConfig);
	AddDefaultScoreItemDetail();
}

//==============================================================================
void MisNewTraining::LoadOperateItem(CXMLWrapperTraining * pTrainingConfig)
{
	for(std::size_t i = 0;i < pTrainingConfig->m_OperateItems.size();++i)
	{
		CXMLWrapperOperateItem * operateItem = pTrainingConfig->m_OperateItems[i];
		m_operateItemMap[operateItem->m_Name] = operateItem;
	}
}
//==============================================================================
void MisNewTraining::LoadDynamicData(CXMLWrapperTraining * pTrainingConfig)
{
	//skip put it in initialize
}

TrainScoreSystem *  MisNewTraining::GetScoreSystem()
{
	return m_ScoreSys;
}
DynObjMap MisNewTraining::GetOrganObjectMap()
{
	return m_DynObjMap;
}
//==============================================================================
void MisNewTraining::OnRemoveRigidBody(GFPhysRigidBody * rb)
{
	DynObjMap::iterator itor = m_DynObjMap.begin();
	while(itor != m_DynObjMap.end())
	{
		MisMedicOrganInterface * organif = itor->second;
		if(organif)
		   organif->NotifyRigidBodyRemovedFromWorld(rb);
		itor++;
	}

}
//==============================================================================
void MisNewTraining::OnRemoveSoftBody(GFPhysSoftBody * sb)
{

}
//==============================================================================
void MisNewTraining::OnRemovePositionConstraint(GFPhysPositionConstraint * cs)
{

}
//==============================================================================
void MisNewTraining::OnRemoveJoint(GFPhysJointConstraint * cs)
{

}
//========================================================================================================================
int MisNewTraining::onBeginCheckCollide(GFPhysCollideObject * softobj , GFPhysCollideObject * rigidobj )
{
	int rscollisionAlgo = 0;

	std::vector<CTool*> ToolsInUse;
	ToolsInUse.push_back((CTool*)m_pToolsMgr->GetLeftTool());
	ToolsInUse.push_back((CTool*)m_pToolsMgr->GetRightTool());
	
	for(size_t t = 0 ; t < ToolsInUse.size() ; t++)
	{
		CTool * tool = ToolsInUse[t];
		if(tool && (tool->GetRigidBodyPart(rigidobj) >= 0))
		{
		   int cdtyoe = tool->onBeginCheckCollide(softobj , rigidobj );
		   if(cdtyoe == 1)
		      rscollisionAlgo = 1;
		}
	}

	return rscollisionAlgo;
}
//========================================================================================================================
/*
void MisNewTraining::onFaceConvexCollided( GFPhysCollideObject * rigidobj , 
									 GFPhysCollideObject * softobj ,
									 GFPhysSoftBodyFace * facecollide,
									 const GFPhysVector3 &   CdpointOnFace,
									 const GFPhysVector3 &   CdnormalOnFace,
									 float depth,
									 float weights[3],
									 int   contactmode
									 )
{
	std::vector<CTool*> ToolsInUse;
	ToolsInUse.push_back((CTool*)m_pToolsMgr->GetLeftTool());
	ToolsInUse.push_back((CTool*)m_pToolsMgr->GetRightTool());

	for(size_t t = 0 ; t < ToolsInUse.size() ; t++)
	{
		CTool * tool = ToolsInUse[t];
		if(tool && (tool->GetRigidBodyPart(rigidobj) >= 0))
		   tool->onFaceConvexCollided( rigidobj , 
									   softobj ,
									   facecollide,
									   CdpointOnFace,
									   CdnormalOnFace,
									   depth,
									   weights,
									   contactmode
									  );
	}
}
*/
//========================================================================================================================
void MisNewTraining::onEndCheckCollide(GFPhysCollideObject * softobj , GFPhysCollideObject * rigidobj, const GFPhysAlignedVectorObj<GFPhysRSManifoldPoint> & contactPoints)
{
	if(contactPoints.size() > 0)
	{
		int i =0;
		int j = i+1;
	}
	std::vector<CTool*> ToolsInUse;
	ToolsInUse.push_back((CTool*)m_pToolsMgr->GetLeftTool());
	ToolsInUse.push_back((CTool*)m_pToolsMgr->GetRightTool());

	for(size_t t = 0 ; t < ToolsInUse.size() ; t++)
	{
		CTool * tool = ToolsInUse[t];
		if(tool && (tool->GetRigidBodyPart(rigidobj) >= 0))
		   tool->onEndCheckCollide( softobj , rigidobj , contactPoints);
	}
}
void MisNewTraining::onRSContactsBuildBegin(ConvexSoftFaceCollidePair * collidePairs , int NumCollidePair)
{
	std::vector<CTool*> ToolsInUse;
	ToolsInUse.push_back((CTool*)m_pToolsMgr->GetLeftTool());
	ToolsInUse.push_back((CTool*)m_pToolsMgr->GetRightTool());

	for(size_t t = 0 ; t < ToolsInUse.size() ; t++)
	{
		CTool * tool = ToolsInUse[t];
		if(tool)
			tool->onRSContactsBuildBegin(collidePairs , NumCollidePair);
	}
}
//========================================================================================================================
void MisNewTraining::onRSContactsBuildFinish( GFPhysAlignedVectorObj<GFPhysSoftRigidContact> & RSContactConstraints)
{
	std::vector<CTool*> ToolsInUse;
	ToolsInUse.push_back((CTool*)m_pToolsMgr->GetLeftTool());
	ToolsInUse.push_back((CTool*)m_pToolsMgr->GetRightTool());

	for(size_t t = 0 ; t < ToolsInUse.size() ; t++)
	{
		CTool * tool = ToolsInUse[t];
		if(tool)
		   tool->onRSContactsBuildFinish(RSContactConstraints);
	}
}
//========================================================================================================================
void MisNewTraining::onRSContactsSolveEnded(const GFPhysAlignedVectorObj<GFPhysSoftRigidContact> & RSContactConstraints)
{
	std::vector<CTool*> ToolsInUse;
	ToolsInUse.push_back((CTool*)m_pToolsMgr->GetLeftTool());
	ToolsInUse.push_back((CTool*)m_pToolsMgr->GetRightTool());

	for(size_t t = 0 ; t < ToolsInUse.size() ; t++)
	{
		CTool * tool = ToolsInUse[t];
		if(tool)
		   tool->onRSContactsSolveEnded(RSContactConstraints);
	}
}
//========================================================================================================================
void MisNewTraining::onRSFaceContactsBuildFinish(GFPhysAlignedVectorObj<GFPhysSoftFaceRigidContact> & RSContactConstraints)
{
	std::vector<CTool*> ToolsInUse;
	ToolsInUse.push_back((CTool*)m_pToolsMgr->GetLeftTool());
	ToolsInUse.push_back((CTool*)m_pToolsMgr->GetRightTool());

	for(size_t t = 0 ; t < ToolsInUse.size() ; t++)
	{
		CTool * tool = ToolsInUse[t];
		if(tool)
		   tool->onRSFaceContactsBuildFinish(RSContactConstraints);
	}
	//////////////////////////////////////////////////////////////////////////
	for (int i = 0, ni = m_SutureNeedles.size(); i < ni; i++)
	{

		//RSContactConstraints.clear();//只删除跟此针和被针拉远的器官有关的碰撞
		bool noexist = false;

		for (int j = 0, nj = RSContactConstraints.size(); j < nj; j++)
		{
			if (m_SutureNeedles[i]->m_BForceSeparate
				&& RSContactConstraints[j].m_SoftBody == m_SutureNeedles[i]->m_SeparateOrgan->m_physbody
				&& RSContactConstraints[j].m_Rigid == m_SutureNeedles[i]->GetPhysicBody())
			{
				noexist = true;
				RSContactConstraints.remove(j);
			}
		}

		if (!noexist)
		{
			m_SutureNeedles[i]->m_BForceSeparate = false;
			m_SutureNeedles[i]->m_SeparateOrgan = NULL;
		}
	}
}
//===========================================================================================================================
void MisNewTraining::OnSSContactBuildFinish(const GFPhysAlignedVectorObj<GFPhysSSContactPoint> & ssContact, int numSSContact)
{

}
//========================================================================================================================
void MisNewTraining::onRSFaceContactsSolveEnded(const GFPhysAlignedVectorObj<GFPhysSoftFaceRigidContact> & RSContactConstraints)
{
	std::vector<CTool*> ToolsInUse;
	ToolsInUse.push_back((CTool*)m_pToolsMgr->GetLeftTool());
	ToolsInUse.push_back((CTool*)m_pToolsMgr->GetRightTool());

	for(size_t t = 0 ; t < ToolsInUse.size() ; t++)
	{
		CTool * tool = ToolsInUse[t];
		if(tool)
		   tool->onRSFaceContactsSolveEnded(RSContactConstraints);
	}
    //////////////////////////////////////////////////////////////////////////
    for(size_t c = 0 ; c < m_SutureNeedles.size() ; c++)
    {
        SutureNeedle * needle = m_SutureNeedles[c];
        if (needle->GetPhysicBody())
        {
            needle->CreateNeedleAnchor(RSContactConstraints);
        }
    }

	//////////////////////////////////////////////////////////////////////////
	for (size_t c = 0; c < m_SutureNeedlesV2.size(); c++)
	{
		SutureNeedleV2 * needle = m_SutureNeedlesV2[c];
		if (needle->GetPhysicBody())
		{
			needle->CreateNeedleAnchor(RSContactConstraints);
		}
	}
}
//========================================================================================================================
void MisNewTraining::OnCollisionStart(GFPhysCollideObject * objA , GFPhysCollideObject * objB , const GFPhysManifoldPoint * contactPoints , int NumContactPoints)
{
	GFPhysRigidBody * ra = GFPhysRigidBody::Upcast(objA);
	GFPhysRigidBody * rb = GFPhysRigidBody::Upcast(objB);
	for (size_t c = 0; c < m_SutureNeedlesV2.size(); c++)
	{
		SutureNeedleV2 * needle = m_SutureNeedlesV2[c];
		if (needle && (needle->GetPhysicBody() == ra || needle->GetPhysicBody() == rb))
		    needle->OnRigidBodyCollided(objA, objB , contactPoints, NumContactPoints);
	}
}
//========================================================================================================================
void MisNewTraining::OnCollisionKeep(GFPhysCollideObject * objA , GFPhysCollideObject * objB , const GFPhysManifoldPoint * contactPoints , int NumContactPoints)
{
	GFPhysRigidBody * ra = GFPhysRigidBody::Upcast(objA);
	GFPhysRigidBody * rb = GFPhysRigidBody::Upcast(objB);

	if(ra && rb)
	{
		std::vector<CTool*> ToolsInUse;
		ToolsInUse.push_back((CTool*)m_pToolsMgr->GetLeftTool());
		ToolsInUse.push_back((CTool*)m_pToolsMgr->GetRightTool());

		for(size_t t = 0 ; t < ToolsInUse.size() ; t++)
		{
			CTool * tool = ToolsInUse[t];
			if(tool && (tool->GetRigidBodyPart(ra) >= 0  || tool->GetRigidBodyPart(rb) >= 0))
			{
			   tool->OnRigidCollided(ra , rb , contactPoints , NumContactPoints);
			}
		}
	}
}
//========================================================================================================================
void MisNewTraining::OnCollisionEnd(GFPhysCollideObject * ra, GFPhysCollideObject * rb)
{
	for (size_t c = 0; c < m_SutureNeedlesV2.size(); c++)
	{
		SutureNeedleV2 * needle = m_SutureNeedlesV2[c];
		if (needle && (needle->GetPhysicBody() == ra || needle->GetPhysicBody() == rb))
			needle->OnRigidBodySeperate(ra, rb);
	}
}
//========================================================================================================================
void MisNewTraining::onThreadConvexCollided( GFPhysCollideObject * rigidobj , 
											  MisMedicThreadRope * rope ,
											  int SegIndex,
											  const GFPhysVector3 &   pointOnRigid,
											  const GFPhysVector3 &   normalOnRigid,
											  float depth,
											  float weights
											  )
{
	std::vector<CTool*> ToolsInUse;
	ToolsInUse.push_back((CTool*)m_pToolsMgr->GetLeftTool());
	ToolsInUse.push_back((CTool*)m_pToolsMgr->GetRightTool());

	for(size_t t = 0 ; t < ToolsInUse.size() ; t++)
	{
		CTool * tool = ToolsInUse[t];
		if(tool)
		   tool->OnThreadSegmentCollided(rigidobj , rope, SegIndex , pointOnRigid , normalOnRigid , weights , depth);//RSContactConstraints);
	}
}
//========================================================================================================================
//void MisNewTraining::triggerSeriousFault( int errorID )
//{
	//m_IsSeriousFault = true;
	//m_fLastTime = 2;

	//if (m_SeriousFaultID == -1)
	//{
	//	m_SeriousFaultID = errorID;
	//}

//}
//========================================================================================================================
void MisNewTraining::receiveCheckPointList(MisNewTraining::OperationCheckPointType type,const std::vector<Ogre::Vector2> & texCord, const std::vector<Ogre::Vector2> & TFUV , ITool * tool , MisMedicOrganInterface * oif)
{

}
//========================================================================================================================
//void MisNewTraining::triggerTrainPopupWidget(TrainPopupWidgetButtonActionType popupType, const std::string& title, const std::string& description, const std::string& leftButtonText, const std::string& rightButtongText)
//{
/*
#define condition 0

#if condition == 0			//只停止分数记录
	if(m_bIsTerminating == false)
	{
		Inception::Instance()->EmitTrainPopupWidgetInfo(popupType, title, description, leftButtonText, rightButtongText);
		CScoreMgr::Instance()->OnTrainEnd();
		m_bIsTerminating = true;
	}
#elif condition == 1		//停止tip提示和分数记录
	if(m_bTrainingRunning)
	{
		Inception::Instance()->EmitTrainPopupWidgetInfo(popupType, title, description, leftButtonText, rightButtongText);
		TrainingFinish();
	}
#else					//旧版本
	Inception::Instance()->EmitTrainPopupWidgetInfo(popupType, title, description, leftButtonText, rightButtongText);
#endif
	*/
//}
//========================================================================================================================
void MisNewTraining::TrainTimeOver()
{
	if (!m_bTrainingRunning)
		return;

	CTipMgr::Instance()->ShowTip("TimeOver");
	CTipMgr::Instance()->OnTrainEnd();

	CScoreMgr::Instance()->OnTrainEnd();

	m_bTrainingRunning = false;
}
//========================================================================================================================
void MisNewTraining::TrainingFinish()
{
	if (!m_bTrainingRunning)
		return;

	CTipMgr::Instance()->ShowTip("TrainingFinish");
	CTipMgr::Instance()->OnTrainEnd();

	CScoreMgr::Instance()->OnTrainEnd();

	m_bTrainingRunning = false;

	//triggerTrainPopupWidget(TPWBT_TRAIN_COMPLETELY, "", "完成训练", "退出训练", "重新开始");
	m_QuitType = TPWBT_TRAIN_COMPLETELY;
}
//==============================================================================================================================
void MisNewTraining::TrainingFatalError(Ogre::String errorTip)
{
	if (!m_bTrainingRunning)
		return;
	
	CTipMgr::Instance()->ShowTip(errorTip);
	CTipMgr::Instance()->OnTrainEnd();

	CScoreMgr::Instance()->OnTrainEnd();

	m_bTrainingRunning = false;

	//triggerTrainPopupWidget(TPWBT_TRAIN_FATALERROR,"","严重操作失误","退出训练","重新开始");
	m_QuitType = TPWBT_TRAIN_FATALERROR;
}
//========================================================================================================================
void MisNewTraining::TrainingErrorWithoutQuit(Ogre::String errorTip)
{
#if 1
    if (!m_bTrainingRunning)
        return;

    CTipMgr::Instance()->ShowTip(errorTip);

    //triggerTrainPopupWidget(TPWBT_TRAIN_ERRORWITHOUTQUIT, "", "严重操作失误", "退出训练", "继续训练");
	m_QuitType = TPWBT_TRAIN_ERRORWITHOUTQUIT;
#else
    QMessageBox::information(NULL, QString::fromLocal8Bit("严重操作失误"), QString::fromLocal8Bit("继续训练"), QMessageBox::Yes);
#endif
}
//========================================================================================================================
void MisNewTraining::AddBleedingRecords()
{
	if(m_pTrainingConfig->m_Type != "NewGallTrain")
		return;
	std::vector<MisMedicOrganInterface*> organs;
	GetAllOrgan(organs);
	Ogre::String unused("");
	for(size_t o = 0 ; o < organs.size() ; o++)
	{
		MisMedicOrganInterface * organ = organs[o];
		std::vector<BleedingRecord> & records = organ->m_BleedingRecords;
		for(size_t r = 0 ; r < records.size() ; r++)
		{
			BleedingRecord & record = records[r];
			QString organName = GetOrganNameByType((TEDOT)organ->m_OrganID);
			CScoreMgr::Instance()->AddBleedingRecords(record.m_IsStopped , record.m_IsAutoStopped , record.m_RemainderTimeAtBleed ,record.m_BleedingTime , organName.toLocal8Bit().constData());
		}
	}
}
//========================================================================================================================
void MisNewTraining::GetObjectLink_Approach(int objectId1,int objectId2,std::vector<MisMedicObjLink_Approach*> & adhersions)
{
	MisMedicOrgan_Ordinary* organ1 = NULL;
	MisMedicOrgan_Ordinary* organ2 = NULL;

	DynObjMap::iterator itr = m_DynObjMap.find(objectId1);
	if(itr != m_DynObjMap.end())
		organ1 = (MisMedicOrgan_Ordinary*)itr->second;
	itr = m_DynObjMap.find(objectId2);
	if(itr != m_DynObjMap.end())
		organ2 = (MisMedicOrgan_Ordinary*)itr->second;

	if(organ1 && organ2)
	{
		for(std::size_t i = 0;i < m_ObjAdhersions.size();++i)
		{
			MisMedicObjLink_Approach * curAdhersion = dynamic_cast<MisMedicObjLink_Approach*>(m_ObjAdhersions[i]);
			if(curAdhersion)
			{
			   if((curAdhersion->m_ConnectOrganA == organ1 && curAdhersion->m_ConnectOrganB == organ2) ||
				  (curAdhersion->m_ConnectOrganA == organ2 && curAdhersion->m_ConnectOrganB == organ1))
			   {
					adhersions.push_back(curAdhersion);
			   }
			}
		}
	}
}
//========================================================================================================================
static QString GetOrganNameByType(TEDOT type)
{
	switch(type)
	{
	case EDOT_BRAVERY_ARTERY:
		return QString::fromLocal8Bit("胆囊动脉");
		break;
	case EDOT_COMMON_BILE_DUCT:
		return QString::fromLocal8Bit("肝总管");
		break;
	case EDOT_CYSTIC_DUCT:
		return QString::fromLocal8Bit("胆囊管");
		break;
	case EDOT_GALLBLADDER:
		return QString::fromLocal8Bit("胆囊");
		break;
	case EDOT_HEPATIC_ARTERY:
		return QString::fromLocal8Bit("肝动脉");
		break;
	case EDOT_LIVER:
		return QString::fromLocal8Bit("肝");
		break;
	case EDOT_VEIN:
		return QString::fromLocal8Bit("胆囊三角");
		break;
	case EDOT_RENAL_ARTERY:
		return QString::fromLocal8Bit("底部三角");
		break;
	case EODT_VEINCONNECT:
		return QString::fromLocal8Bit("胆囊三角连接");
		break;
	case EODT_VEINBOTTOMCONNECT:
		return QString::fromLocal8Bit("胆囊床连接");
		break;
	case EDOT_RENAL_VEIN:
		return QString::fromLocal8Bit("");
		break;
	case EODT_URETER:
		return QString::fromLocal8Bit("");
		break;
	case EODT_UTERUS:
		return QString::fromLocal8Bit("");
		break;
	case EDOT_APPENDIX:
		return QString::fromLocal8Bit("");
		break;
	case EDOT_APPENDMENSTORY:
		return QString::fromLocal8Bit("");
		break;
	case EODT_PEITAI:
		return QString::fromLocal8Bit("");
		break;
	case EDOT_ORGAN_MAX:
		return QString::fromLocal8Bit("");
		break;
	case EDOT_HELPER_OBJECT0:
		return QString::fromLocal8Bit("");
		break;
	case EDOT_ORGAN_LIMIT:
		return QString::fromLocal8Bit("");
		break;
	default:
		return QString::fromLocal8Bit("组织");
		break;
	}
}
//========================================================================================================================
void MisNewTraining::OnSaveTrainingReport()
{
	__super::OnSaveTrainingReport();
	SYTrainingReport * pReport = SYTrainingReport::Instance();

	//set max bleed time
	float maxBloodTime = 0.f;
	MisMedicOrganInterface * pOrgan = NULL;
	for(DynObjMap::iterator itr = m_DynObjMap.begin();itr != m_DynObjMap.end();++itr)
	{
		pOrgan = itr->second;
		switch(pOrgan->m_OrganType)
		{
		case EDOT_GALLBLADDER:
		case EDOT_BRAVERY_ARTERY:
		case EDOT_COMMON_BILE_DUCT:
		case EDOT_VEIN:
		case EDOT_HEPATIC_ARTERY:
		case EDOT_CYSTIC_DUCT:
			continue;
		}
		
		float curOrganMaxTime = pOrgan->GetCurMaxBleedTime();
		if(maxBloodTime < curOrganMaxTime)
			maxBloodTime = curOrganMaxTime;
	}
	pReport->SetMaxBleedTime(maxBloodTime);

	//设置训练公有的操作项
	for(size_t i = 0;i < m_pTrainingConfig->m_CommonOperateItems.size();++i)
	{
		CXMLWrapperOperateItem * pOperateItem = m_pTrainingConfig->m_CommonOperateItems[i];
		const std::string& operateName = pOperateItem->m_Name;
		MxOperateItem item(pOperateItem->m_ID,operateName,pOperateItem->m_Description,-1);
		item.SetValueType(pOperateItem->m_ValueType);
		item.SetCategory(MxOperateItem::ConvertValueToCategory(pOperateItem->m_Category));

		if(operateName == "UsedTime")
		{
			//训练总用时
			//item.SetValue(Inception::Instance()->m_totalTime - Inception::Instance()->m_remainTime);
			item.SetValue(GetElapsedTime());
		}
		else if(operateName == "TotalElectricTime")
		{
			//器械通电总时间
			item.SetValue(m_pToolsMgr->GetTotalElectricTime());
			//qDebug() << "GetTotalElectricTime : " << m_pToolsMgr->GetTotalElectricTime();
		}
		else if(operateName == "InvalidElectricTime")
		{
			//无效通电时间
			item.SetValue(m_pToolsMgr->GetTotalElectricTime() - m_pToolsMgr->GetValidElectricTime());
		}
		else if(operateName == "ElectricEfficiency")
		{
			//通电效率
			float totalTime = m_pToolsMgr->GetTotalElectricTime();
			float validTime = m_pToolsMgr->GetValidElectricTime();
			float value = 0;
			if(totalTime > 0.0001)
				value = 100.f * validTime / totalTime;

			item.SetValue(value);
		}
		else if(operateName == "MaxKeeppingElectricTime")
		{
			//最长持续通电时间
			float time = m_pToolsMgr->GetMaxKeeppingElectricTime();
			item.SetValue(time);
			if(time > 1)
				item.AddOperateTime(m_pToolsMgr->GetMaxKeeppingElectricBeginTime());
			//qDebug() << "GetMaxKeeppingElectricTime : " << time;
		}
		else if(operateName == "ElectricTimeForHemoClip")
		{
			//通电对钛夹所影响的时间
			item.SetValue(m_pToolsMgr->GetElectricTimeForHemoClip());
		}
		else if(operateName == "ElectricTimeForOrdinaryOrgan")
		{
			//通电对胆囊管所影响的时间
			item.SetValue(m_pToolsMgr->GetElectricTimeForOrdinaryOrgan());
		}
		else if(operateName == "MaxBloodTime")
		{
			//最长流血时间
			item.SetValue(maxBloodTime);
		}
		else if(operateName == "NumberOfReleasedTitanicClip")
		{
			//施放钛夹个数
			item.SetValue(m_pToolsMgr->GetNumberOfReleasedTitanicClip());
		}
		else if(operateName == "BleedingVolume")
		{
			//流血量
			float bleedingVolume = 0.f;
			for(DynObjMap::iterator itr = m_DynObjMap.begin();itr != m_DynObjMap.end();++itr)
			{
				bleedingVolume += itr->second->GetBleedingVolume();
			}
			item.SetValue(bleedingVolume);
		}
		else if(operateName == "LigationTimes")
		{
			//套扎次数
			int n = 0;
			for(DynObjMap::iterator itr = m_DynObjMap.begin();itr != m_DynObjMap.end();++itr)
			{
				n += itr->second->GetAttachmentCount(MOAType_BindedRope);
			}
			item.SetValue(n);
		}//for tool
		else if(operateName == "LeftToolClosedTimes")
		{
			//左手器械夹闭次数
			item.SetValue(m_pToolsMgr->GetLeftToolClosedTimes());
		}
		else if(operateName == "RightToolClosedTimes")
		{
			//右手器械夹闭次数
			item.SetValue(m_pToolsMgr->GetRightToolClosedTimes());
		}
		else if(operateName == "LeftToolMovedDistance")
		{
			//左手器械移动距离
			item.SetValue(m_pToolsMgr->GetLeftToolMovedDistance());
		}
		else if(operateName == "RightToolMovedDistance")
		{
			//右手器械移动距离
			item.SetValue(m_pToolsMgr->GetRightToolMovedDistance());
		}
		else if(operateName == "LeftToolMovedSpeed")
		{
			//左手器械移动速度
			item.SetValue(m_pToolsMgr->GetLeftToolMovedSpeed());
		}
		else if(operateName == "RightToolMovedSpeed")
		{
			//右手器械移动速度
			item.SetValue(m_pToolsMgr->GetRightToolMovedSpeed());
		}
		else if(operateName == "ToolIsClosedInsertion")
		{
			//器械是否闭合插入
			item.SetValue(m_pToolsMgr->ToolIsClosedInsertion());
		}
		else if(operateName == "SuctionAndIrrigationTime")
		{
			//冲洗时间
			item.SetValue(m_pToolsMgr->GetToolSuctionTime() + m_pToolsMgr->GetToolIrrigationTime());
		}
		else if(operateName == "ToolInAndOutTimes")
		{
			//冲洗时间
			item.SetValue(m_pToolsMgr->GetToolInAndOutTimes());
		}
		else if(operateName == "IsClosedInSeparateTime")
		{
			//冲洗时间
			item.SetValue(m_pToolsMgr->ToolIsClosedInSeparateTime());
		}
		else
		{
			continue;
		}
		
		m_operateItems.push_back(item);
	}

	//添加未被触发的操作项
	for(std::map<std::string,CXMLWrapperOperateItem*>::iterator itr = m_operateItemMap.begin();itr != m_operateItemMap.end();++itr)
	{
		if(m_operateItemTimesMap.find(itr->first) == m_operateItemTimesMap.end())
		{
			CXMLWrapperOperateItem * pOperateItem = itr->second;
			MxOperateItem item(pOperateItem->m_ID,
								pOperateItem->m_Name,
								pOperateItem->m_Description,
								-1,
								MxOperateItem::ConvertValueToCategory(pOperateItem->m_Category));

			if(pOperateItem->m_flag_ValueType)
				item.SetValueType(pOperateItem->m_ValueType);
			else
				item.SetValueType(MxOperateItem::VT_NumberInt);

			m_operateItems.push_back(item);
		}
	}

	//save all operate item
	if(m_ScoreSys)
		m_ScoreSys->SaveOperateItems();
	pReport->AddOperateItems(m_operateItems);

	pReport->SetScoreItemDetail(GetScoreTable(),m_scoreItemDetails);
}
//========================================================================================================================
bool MisNewTraining::AddOperateItem(const std::string& operateItemName,bool setOperateTime /* = true */,AddMode addMode,MxOperateItem ** ppOperateItem)
{
	int value = INT_MAX;
	return AddOperateItem(operateItemName,value,setOperateTime,addMode,ppOperateItem);
}
//========================================================================================================================
bool MisNewTraining::AddOperateItem(const std::string& operateItemName,float value /* = 0 */,bool setOperateTime /* = true */,AddMode addMode ,MxOperateItem ** ppOperateItem)
{
	bool isOk = false;
	int operateItemIndex = -1;
	std::map<std::string,CXMLWrapperOperateItem*>::const_iterator itr = m_operateItemMap.find(operateItemName);

	if(itr != m_operateItemMap.end())
	{
		isOk = true;	
		const CXMLWrapperOperateItem * pOperateItem = itr->second;
		float elapsedTime = GetElapsedTime();
		MxOperateItem::ItemCategory category = MxOperateItem::ConvertValueToCategory(pOperateItem->m_Category);

		//使用配置中的value值
		if(value == INT_MAX)
			value = pOperateItem->m_Value;

		//对于带分数的操作项
		if(pOperateItem->m_flag_Score)
		{
			//考虑有效次数 、 最小得分间隔（要保证有效需要修改上层Inception中的变量的值为float，并且定时器更新的时间间隔不为1s）
			//TODO
// 			if(times > validTime || interval < pOperateItem->m_MinScoreInterval)
// 			{
// 				isOk = false;
// 				goto END;
// 			}

			float score = pOperateItem->m_Score;
			float timeScore = 0.f;

			//计算时间分
			if(pOperateItem->m_flag_TimeScore)
			{
				timeScore = pOperateItem->m_TimeScore;
				if(pOperateItem->m_flag_TimeLimit1 && pOperateItem->m_flag_TimeLimit2)
				{
					float minTime = pOperateItem->m_TimeLimit1;
					float maxTime = pOperateItem->m_TimeLimit2;

					if(elapsedTime >= maxTime)
						timeScore = 0.f;
					else if(elapsedTime > minTime)
					{
						float temp = timeScore;
						temp -= timeScore * (elapsedTime - minTime) / (maxTime - minTime) * pOperateItem->m_FalloffFactor;
						if(temp < 0)
							temp = 0.f;
						if(temp < timeScore)
							timeScore = temp;
					}
				}
			}

			bool canAddNewOperateItem = true;;

			if(addMode & AM_MergeAll)
			{
				for(std::size_t i = 0;i < m_operateItems.size();++i)
				{
					if(m_operateItems[i].GetName() == operateItemName)
					{
						//1 set value
						if(addMode & AM_MergeValue)
							m_operateItems[i].SetValue(m_operateItems[i].GetValue() + value);
						else
							m_operateItems[i].SetValue(value);

						//2 set score item
						m_operateItems[i].AddScoreItem(score,timeScore,elapsedTime);
						if(addMode & AM_MergeScoreItem)
							m_operateItems[i].MergeAllScoreItem(setOperateTime);

						//3 set operate time
						if(setOperateTime)
							m_operateItems[i].AddOperateTime(elapsedTime);
						if(addMode & AM_MergeTime)
							m_operateItems[i].MergeOperateTime(setOperateTime);

						canAddNewOperateItem = false;
						operateItemIndex = i;
						break;
					}
				}
			}
			else if(addMode == AM_ReplaceOnlyValue || addMode == AM_ReplaceAll)
			{
				for(std::size_t i = 0;i < m_operateItems.size();++i)
				{
					if(m_operateItems[i].GetName() == operateItemName)
					{
						//1 replace value
						m_operateItems[i].SetValue(value);
						//2 replace socre item
						m_operateItems[i].ClearScoreItems();
						m_operateItems[i].AddScoreItem(score,timeScore,elapsedTime);
						//3 replace operate time
						m_operateItems[i].ClearOperateTime();
						if(setOperateTime)
							m_operateItems[i].AddOperateTime(elapsedTime);

						canAddNewOperateItem = false;
						operateItemIndex = i;
						break;
					}
				}
			}
			else
			{//default：AM_Add
				canAddNewOperateItem = true;
			}

			if(canAddNewOperateItem)
			{
				if(!setOperateTime)
					elapsedTime = -1;
				MxOperateItem item(pOperateItem->m_ID,operateItemName,pOperateItem->m_Description,elapsedTime,category,operateItemName,pOperateItem->m_Description,score,timeScore);

				item.SetValue(value);
				if(pOperateItem->m_flag_ValueType)
					item.SetValueType(pOperateItem->m_ValueType);
				else
					item.SetValueType(MxOperateItem::VT_NumberInt);

				m_operateItems.push_back(item);
				operateItemIndex = m_operateItems.size() - 1;
			}
		}
		else
		{//对于普通的操作项
			bool canAddNewOperateItem = true;
			if(addMode & AM_MergeAll)
			{
				for(size_t i = 0;i < m_operateItems.size();++i)
				{
					if(m_operateItems[i].GetName() == operateItemName)
					{
						//1 set value
						if(addMode & AM_MergeValue)
							m_operateItems[i].SetValue(m_operateItems[i].GetValue() + value);
						else
							m_operateItems[i].SetValue(value);

						//2 set operate time
						if(setOperateTime)
							m_operateItems[i].AddOperateTime(elapsedTime);
						if(addMode & AM_MergeTime)
							m_operateItems[i].MergeOperateTime(setOperateTime);

						canAddNewOperateItem = false;
						operateItemIndex = i;
						break;
					}
				}
			}
			else if(addMode == AM_ReplaceOnlyValue)
			{
				for(size_t i = 0;i < m_operateItems.size();++i)
				{
					if(m_operateItems[i].GetName() == operateItemName)
					{
						//replace value only
						m_operateItems[i].SetValue(value);
						if(setOperateTime)
							m_operateItems[i].AddOperateTime(elapsedTime);

						canAddNewOperateItem = false;
						operateItemIndex = i;
						break;
					}
				}
			}
			else if(addMode == AM_ReplaceAll)
			{
				for(size_t i = 0;i < m_operateItems.size();++i)
				{
					if(m_operateItems[i].GetName() == operateItemName)
					{
						//replace value
						m_operateItems[i].SetValue(value);

						m_operateItems[i].ClearOperateTime();
						if(setOperateTime)
							m_operateItems[i].AddOperateTime(elapsedTime);

						canAddNewOperateItem = false;
						operateItemIndex = i;
						break;
					}
				}
			}
			else
			{//default：AM_Add
				canAddNewOperateItem = true;	
			}

			if(canAddNewOperateItem)
			{
				if(!setOperateTime)
					elapsedTime = -1.f;

				MxOperateItem item(pOperateItem->m_ID,operateItemName,pOperateItem->m_Description,elapsedTime,category);

				item.SetValue(value);
				if(pOperateItem->m_flag_ValueType)
					item.SetValueType(pOperateItem->m_ValueType);
				else
					item.SetValueType(MxOperateItem::VT_NumberInt);

				m_operateItems.push_back(item);
				operateItemIndex = m_operateItems.size() - 1;
			}
		}

		//统计操作项的触发次数
		std::map<std::string,int>::iterator itr = m_operateItemTimesMap.find(operateItemName);
		if(itr == m_operateItemTimesMap.end())
			m_operateItemTimesMap.insert(std::make_pair(operateItemName,1));
		else
			++(itr->second);

		//返回当前的操作项
		if(ppOperateItem && operateItemIndex != -1)
			*ppOperateItem = &m_operateItems[operateItemIndex];
	}

	if(!isOk && ppOperateItem)
		*ppOperateItem = NULL;

//END:
	return isOk;
}
//========================================================================================================================
MxOperateItem* MisNewTraining::GetLastOperateItem(const std::string& operateItemName)
{
	MxOperateItem * pOperateItem = NULL;

	for(int i = m_operateItems.size() - 1;i >= 0;--i)
	{
		if(m_operateItems[i].GetName() == operateItemName)
		{
			pOperateItem = &m_operateItems[i];
			break;
		}
	}

	return pOperateItem;
}
//========================================================================================================================
MxOperateItem* MisNewTraining::GetOperateItem(const std::string& operateItemName)
{
	MxOperateItem* pOperateItem = NULL;
	
	for(size_t i = 0;i < m_operateItems.size();++i)
	{
		if(m_operateItems[i].GetName() == operateItemName)
		{
			pOperateItem = &m_operateItems[i];
			break;
		}
	}

	return pOperateItem;
}
//========================================================================================================================
bool MisNewTraining::HasOperateItem(const std::string& operateItemName)
{
	for(std::size_t i = 0 ;i < m_operateItems.size();++i)
	{
		if(m_operateItems[i].GetName() == operateItemName)
			return true;
	}

	return false;
}
//========================================================================================================================
void MisNewTraining::RemoveLastOperateItem(const std::string& operateItemName)
{
	for(int i = m_operateItems.size() - 1;i >= 0;--i)
	{
		if(m_operateItems[i].GetName() == operateItemName)
		{
			m_operateItems.erase(m_operateItems.begin() + i);

// 			std::map<std::string,int>::iterator itr = m_operateItemTimesMap.find(operateItemName);
// 			if(itr == m_operateItemTimesMap.end())
// 				m_operateItemTimesMap.insert(std::make_pair(operateItemName,1));
// 			else
// 				--(itr->second);
			break;
		}
	}
}

void MisNewTraining::AddDefaultScoreItemDetail()
{
	SYScoreTable* scoreTable = GetScoreTable();
	if(scoreTable == nullptr)
		return;

	QString scoreTableCode = scoreTable->GetCode();
	QString scoreCode;
	const QSet<SYScorePointDetailData*>& spddSet = scoreTable->GetAllScorePointDetailData();
	for(auto spdd : spddSet){
		//代码"9"表示未操作的评分细节
		if(spdd->Code() == "9"){
			scoreCode = scoreTable->GetScoreCode(spdd);
			if(scoreCode.size() == 0)
				throw "code size error";
			m_scoreItemDetails.push_back(SYScoreItemDetail(scoreTableCode, scoreCode));
		}
	}
}

void MisNewTraining::AddScoreItemDetail(const QString& scoreCode, int time)
{
	if(scoreCode.size() != SYScoreTable::ScoreCodeLength)
		throw "score code length error";

	//替换掉默认编码，即未操作时的编码(最后一位为9)
	bool replaced = false;
	QString codePrefix = scoreCode.left(SYScoreTable::ScoreCodeLength - 1);		//最后一位为评分点详细编码
	for(auto& scoreItemDetail : m_scoreItemDetails){
		const QString& preScoreCode = scoreItemDetail.GetScoreCode();
		if(preScoreCode.right(1) == "9" && preScoreCode.startsWith(codePrefix)){
			scoreItemDetail.SetScoreCode(scoreCode);
			scoreItemDetail.SetTime(time);
			replaced = true;
			break;
		}
	}

	if ((!replaced)&& GetScoreTable())
		m_scoreItemDetails.push_back(SYScoreItemDetail(GetScoreTable()->GetScoreTableCode(), scoreCode, time));
}

void MisNewTraining::RemoveScoreItemDetail(const QString& scoreCode)
{
	for(auto itr = m_scoreItemDetails.begin(); itr != m_scoreItemDetails.end(); ++itr){
		if(itr->GetScoreCode() == scoreCode){
			m_scoreItemDetails.erase(itr);
			break;
		}
	}
}

//========================================================================================================================
void MisNewTraining::ShowTip(const std::string& tip)
{
	CTipMgr::Instance()->ShowTip(tip);
}
//========================================================================================================================
void MisNewTraining::SetNextTip(const std::string& tip,float waitTime)
{
	CTipMgr::Instance()->SetNextTip(tip,waitTime);
}
//========================================================================================================================
void MisNewTraining::onSutureThreadConvexCollided(const GFPhysAlignedVectorObj<TRCollidePair> & TRCollidePairs,SutureThread * suturerope )
{
    std::vector<CTool*> ToolsInUse;
    ToolsInUse.push_back((CTool*)m_pToolsMgr->GetLeftTool());
    ToolsInUse.push_back((CTool*)m_pToolsMgr->GetRightTool());

    for(size_t t = 0 ; t < ToolsInUse.size() ; t++)
    {
        CTool * tool = ToolsInUse[t];
        if(tool)
        {
            for(size_t t = 0  ; t < TRCollidePairs.size() ; t++)
            {
                const TRCollidePair & trPair = TRCollidePairs[t];                
                tool->OnSutureThreadSegmentCollided(trPair.m_Rigid , 
                    suturerope ,
                    trPair.m_Segment,
                    trPair.m_RigidWorldPoint,
                    trPair.m_NormalOnRigid,
                    trPair.m_SegWeight,
                    trPair.m_Depth);
            }
        }
    }
}
//========================================================================================================================
void MisNewTraining::onSutureThreadConvexCollided(const GFPhysAlignedVectorObj<STVRGCollidePair> & TRCollidePairs,SutureThreadV2 * suturerope)
{
	std::vector<CTool*> ToolsInUse;
	ToolsInUse.push_back((CTool*)m_pToolsMgr->GetLeftTool());
	ToolsInUse.push_back((CTool*)m_pToolsMgr->GetRightTool());

	for (size_t t = 0; t < ToolsInUse.size(); t++)
	{
		CTool * tool = ToolsInUse[t];
		if (tool)
		{
			for (size_t t = 0; t < TRCollidePairs.size(); t++)
			{
				const STVRGCollidePair & trPair = TRCollidePairs[t];
				tool->OnSutureThreadSegmentCollided(trPair.m_Rigid,
					suturerope,
					trPair.m_Segment,
					trPair.m_RigidWorldPoint,
					trPair.m_NormalOnRigid,
					trPair.m_SegWeight,
					trPair.m_Depth);
			}
		}
	}
}
//========================================================================================================================
MisMedicOrgan_Ordinary * MisNewTraining::FilterClampedOrgan(std::vector<MisMedicOrgan_Ordinary *> & organs)
{
    return 0;
}

//====================================================================================
static class RayIntersectMesCallBack : public GFPhysTriangleProcessor
{
public:
	RayIntersectMesCallBack(const GFPhysVector3 & source, const GFPhysVector3 & target)
	{
		m_raySource = source;
		m_rayTarget = target;
		m_closetCastWeightInRay = FLT_MAX;
		m_CastSucced = false;
	}
	void ProcessTriangle(GFPhysVector3* triangleVerts, int partId, int triangleIndex, void * UserData)
	{
		Real  Rayweight;
		Real  triangleWeight[3];
		GFPhysVector3  intersectpt;

		bool intersect = LineIntersectTriangle(triangleVerts[0],
			triangleVerts[1],
			triangleVerts[2],
			m_raySource,
			m_rayTarget,
			Rayweight,
			intersectpt,
			triangleWeight);

		if (intersect && Rayweight >= 0 && Rayweight <= 1 && Rayweight < m_closetCastWeightInRay)
		{
			m_closetCastWeightInRay = Rayweight;
			m_ResultPoint = m_raySource + (m_rayTarget - m_raySource) * Rayweight;
			m_CastSucced = true;
		}
	}

	GFPhysVector3 m_raySource;
	GFPhysVector3 m_rayTarget;
	GFPhysVector3 m_ResultPoint;
	float m_closetCastWeightInRay;
	bool  m_CastSucced;
};
//========================================================================================================================
bool MisNewTraining::DetectLineSegmentIntersectOrgan(MisMedicOrganInterface * insecOrgan, GFPhysSoftBodyFace * insecface)
{
	if (m_DetectCameraIntersect == false)
		return false;

	Ogre::Vector3 lineSeg0 = m_camPivotPos;
	Ogre::Vector3 lineSeg1 = m_pLargeCamera->getParentNode()->getPosition();

	Ogre::Vector3 lineSegDir = (lineSeg1 - lineSeg0).normalisedCopy();
	lineSeg1 += lineSegDir * (m_pLargeCamera->getNearClipDistance() + 0.05f);
	
	DynObjMap::iterator itor = m_DynObjMap.begin();
	for (; itor != m_DynObjMap.end(); ++itor)
	{
		MisMedicOrganInterface * organif = itor->second;
	
		Ogre::Real dist;
		
		MisMedicOrgan_Ordinary* ordiOrgan = dynamic_cast<MisMedicOrgan_Ordinary*>(organif);
		
		MisMedicRigidPrimtive * rigidMesh = dynamic_cast<MisMedicRigidPrimtive*>(organif);

		if (ordiOrgan)
		{
			GoPhys::GFPhysSoftBodyFace * bodyface = ordiOrgan->GetRayIntersectFace(lineSeg0, lineSeg1, dist);
			if (bodyface)
			{
                CScreenEffect::Instance()->ShowImage("PicOverlay/BlurOrganCamera");
			    return true;
			}
		}
		else if (rigidMesh)
		{
			GFPhysBvhTriMeshShape * triMesh = dynamic_cast<GFPhysBvhTriMeshShape*>(rigidMesh->m_body->GetCollisionShape());

			RayIntersectMesCallBack detector(OgreToGPVec3(lineSeg0), OgreToGPVec3(lineSeg1));

			triMesh->CastRay(&detector, OgreToGPVec3(lineSeg0), OgreToGPVec3(lineSeg1));

			if (detector.m_CastSucced)
			{
				CScreenEffect::Instance()->ShowImage("PicOverlay/BlurOrganCamera");
				return true;
			}
		}
	}
	CScreenEffect::Instance()->HideImage("PicOverlay/BlurOrganCamera");
	return false;
}
//========================================================================================================================
void MisNewTraining::CalcCameraSpeed( Real dt,bool isFixed)
{
    if (!isFixed)
    {
        Ogre::Vector3 campos = m_pLargeCamera->getDerivedPosition();
        if (fabs(campos.x - m_preCameraPos.x) > 0.001f || fabs(campos.y - m_preCameraPos.y) > 0.001f || fabs(campos.z - m_preCameraPos.z) > 0.001f)
        {
            Real dismov = campos.distance(m_preCameraPos);
            m_disCameraMove += dismov;
            m_timeCameraMove += dt;
        }
        m_preCameraPos = campos;
        if (m_timeCameraMove > GP_EPSILON)
        {
            m_CameraSpeed = m_disCameraMove / m_timeCameraMove;
        }        
    }
}
float MisNewTraining::GetCameraSpecialAngle()
{
	return m_CameraAngle;
}

void MisNewTraining::SetCameraSpecialAngle(float angle)
{
	m_CameraAngle = angle;
}
//========================================================================================================================
void MisNewTraining::OnVeinconnectChanged(VeinConnectObject * veinobj, int clusterID)
{
    veinobj->m_clusters[clusterID].OnAttachFaceChanged();
}
//========================================================================================================================
/*
void MisNewTraining::RemoveClustbyID(VeinConnectObject * veinobj, int clusterID)
{
    VeinConnectPair & MajorPair = veinobj->m_clusters[clusterID].m_pair[0];
    VeinConnectPair & AttachPair = veinobj->m_clusters[clusterID].m_pair[1];

    if (MajorPair.m_Valid || AttachPair.m_Valid)
    {
        MisMedicOrgan_Ordinary * organs[2];
        organs[0] = dynamic_cast<MisMedicOrgan_Ordinary*>(m_DynObjMap.find(veinobj->m_clusters[clusterID].m_ObjAID)->second);
        organs[1] = dynamic_cast<MisMedicOrgan_Ordinary*>(m_DynObjMap.find(veinobj->m_clusters[clusterID].m_ObjBID)->second);

        int faceid[4];
        faceid[0] = MajorPair.m_MMo_faceAID;
        faceid[1] = MajorPair.m_MMo_faceBID;
        faceid[2] = AttachPair.m_MMo_faceAID;
        faceid[3] = AttachPair.m_MMo_faceBID;

        for (int i = 0; i < 4; i++)
        {
            if (organs[i % 2] != 0)
            {
                MisMedicOrgan_Ordinary * organ = organs[i % 2];

                if (faceid[i] > 0 && faceid[i] < (int)organ->m_OriginFaces.size())
                {
                    MMO_Face& mmoface = organ->m_OriginFaces[faceid[i]];                    

                    std::vector<MMO_Face::VeinInfo>::iterator iter = mmoface.m_VeinInfoVector.begin();

                    while (iter != mmoface.m_VeinInfoVector.end())
                    {
                        if (iter->valid && iter->veinobj == veinobj && iter->clusterId == clusterID)
                        {
                            //iter->valid = false;
                            iter = mmoface.m_VeinInfoVector.erase(iter);                            
                        }
                        else
                        {
                            iter++;
                        }
                    }
                }
            }
        }
        veinobj->removeClustbyIDinternal(clusterID);
    }
}
*/
//========================================================================================================================
void MisNewTraining::BurnHookedAndContactedConnect(CTool* tool,Real dt)
{
	std::vector<VeinConnectObject*> ConnectObjects = GetVeinConnectObjects();

	std::vector<Ogre::Vector3>  burnpos;

	if (tool)
	{
		bool IsElecButton = (tool->m_bElectricLeftPad || tool->m_bElectricRightPad);

		bool IsElecCut = tool->m_bElectricLeftPad;

		float BurnRate = IsElecCut ? dt : dt * 0.3f;

		if (IsElecButton)
		{
			std::vector<VeinConnectPair*> burnPairs;
			
			for (size_t v = 0, nv = ConnectObjects.size(); v < nv; v++)
			{
				VeinConnectObject * veinobj = ConnectObjects[v];

				int burncount = tool->TryBurnConnectStrips(veinobj, BurnRate, burnpos, burnPairs);

				if (burnPairs.size() != 0)
				{
					OnVeinConnectCuttingByElecTool(burnPairs);
				}

				if (burncount > 0)
				{
					MxEvent *  pEvent = CMXEventsDump::Instance()->CreateEventNew(MxEvent::MEXT_Coaulation_DeleteLine, tool, veinobj);
					CMXEventsDump::Instance()->PushEvent(pEvent);
				}
			}
		}

		//
		for (size_t v = 0, nv = ConnectObjects.size(); v < nv; v++)
		{
			VeinConnectObject * veinobj = ConnectObjects[v];
			if (veinobj->m_Disconnected)
			{
				MxEvent *  pEvent = CMXEventsDump::Instance()->CreateEventNew(MxEvent::MEXT_Coaulation_DeleteLine, tool, veinobj);
				CMXEventsDump::Instance()->PushEvent(pEvent);
			}
		}
	}
}

void MisNewTraining::UpdateTrocarTransform()
{
	Ogre::Node * pTrocarLeftTool = 0;
	Ogre::Node * pTrocarRightTool = 0;

	if (MXOgreWrapper::Get()->GetDefaultSceneManger()->hasSceneNode("trocar_righttool$1"))
	{
		pTrocarRightTool = MXOgreWrapper::Get()->GetDefaultSceneManger()->getSceneNode("trocar_righttool$1");

		if (pTrocarRightTool)
		{
			pTrocarRightTool->setPosition(InputSystem::GetInstance(DEVICETYPE_RIGHT)->GetPivotPosition());
			pTrocarRightTool->setOrientation(InputSystem::GetInstance(DEVICETYPE_RIGHT)->GetOrientation_OGRE(E_EULER_ANGLES_YXZ));
		}
	}

	if (MXOgreWrapper::Get()->GetDefaultSceneManger()->hasSceneNode("trocar_lefttool$1"))
	{
		pTrocarLeftTool = MXOgreWrapper::Get()->GetDefaultSceneManger()->getSceneNode("trocar_lefttool$1");

		if (pTrocarLeftTool)
		{
			pTrocarLeftTool->setPosition(InputSystem::GetInstance(DEVICETYPE_LEFT)->GetPivotPosition());
			pTrocarLeftTool->setOrientation(InputSystem::GetInstance(DEVICETYPE_LEFT)->GetOrientation_OGRE(E_EULER_ANGLES_YXZ));
		}
	}

}
int MisNewTraining::StartTimer(float time,void* userData)
{
	static int localId = 1;
	Timer* timer = new Timer;
	timer->id = localId;
	timer->curTime = 0.f;
	timer->totalTime = time;
	timer->userData = userData;

	m_timers.push_back(timer);
	return localId++;
}

void MisNewTraining::RemoveAllTimer()
{
	for(auto timer : m_timers)
		delete timer;

	m_timers.clear();
}

void MisNewTraining::RemoveTimer(int id)
{
	for(auto itr = m_timers.begin(); itr != m_timers.end(); ++itr){
		if((*itr)->id == id){
			delete *itr;
			m_timers.erase(itr);
			break;
		}
	}
}
