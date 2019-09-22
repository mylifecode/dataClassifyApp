#include "MisCTool_PluginClamp.h"
#include "math/GoPhysTransformUtil.h"
#include "collision/NarrowPhase/GoPhysPrimitiveTest.h"
#include "VeinConnectObject.h"
#include "MisNewTraining.h"
#include "stdafx.h"
#include "InputSystem.h"
#include "Inception.h"
#include "MXEventsDump.h"
#include "MXEvent.h"
#include "EffectManager.h"
#include "ScreenEffect.h"
#include "HelperLogics.h"
#include "Instruments/Tool.h"

#include "NullSchemeHandler.h"
#include "Collision/CollisionDispatch/GoPhysSoftRigidCollision.h"
#include "TrainUtils.h"
#include <QDebug>
#include "CustomCollision.h"
#include "MisCTool_TubeClamp.h"
#include "SutureThreadV2.h"
//===========================================================================================================
class SoftFaceContactClampRegCB : public GFPhysNodeOverlapCallback
{
public:
	SoftFaceContactClampRegCB()
	{
		m_SoftFacesToCheck.reserve(20);
	}

	void ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData)
	{
		 GFPhysSoftBodyFace * face = (GFPhysSoftBodyFace*)UserData;
		 m_SoftFacesToCheck.push_back(face);
	}

	std::vector<GFPhysSoftBodyFace*> m_SoftFacesToCheck;
};
//===========================================================================================================
GFPhysVector3 MisCTool_PluginClamp::ToolClampRegion::GetLocalClampNormal()
{
	return m_Axis0Local.Cross(m_Axis1Local).Normalized() * m_normalSign;
}
//===========================================================================================================
GFPhysVector3 MisCTool_PluginClamp::ToolClampRegion::GetLocalClampCoordOrigin()
{
	return m_LocalOrigin;
}

//===========================================================================================================
void MisCTool_PluginClamp::ToolClampRegion::CalcRegionWorldAABB(float margin , GFPhysVector3 & aabbmin , GFPhysVector3 & aabbmax)
{

	GFPhysVector3 quad00 = m_OriginWorld + m_axis0Min * m_Axis0World + m_axis1Min * m_Axis1World;
	GFPhysVector3 quad01 = m_OriginWorld + m_axis0Max * m_Axis0World + m_axis1Min * m_Axis1World;
	GFPhysVector3 quad11 = m_OriginWorld + m_axis0Max * m_Axis0World + m_axis1Max * m_Axis1World;
	GFPhysVector3 quad10 = m_OriginWorld + m_axis0Min * m_Axis0World + m_axis1Max * m_Axis1World;

    aabbmax = aabbmin = quad00;
    aabbmin.SetMin(quad10);
    aabbmin.SetMin(quad11);
    aabbmin.SetMin(quad01);

    aabbmax.SetMax(quad10);
    aabbmax.SetMax(quad11);
    aabbmax.SetMax(quad01);

    aabbmin -= GFPhysVector3(margin , margin , margin);
    aabbmax += GFPhysVector3(margin , margin , margin);
}
//===========================================================================================================
class ClampVeinConnTestCallBack : public GFPhysNodeOverlapCallback
{
public:
	ClampVeinConnTestCallBack(MisCTool_PluginClamp * clampPlugin)  : m_pClampPlugin(clampPlugin){}
	
	void ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData)
	{
		VeinCollideData * cdata = (VeinCollideData*)UserData;
		
		if(!cdata)
			return;

		VeinConnectObject *veinobj = cdata->m_HostObject;
		VeinConnectCluster & cluster = veinobj->m_clusters[cdata->m_ClusterIndex];
		
		if(cdata->m_InContact)	//检测是否已经与此器械碰撞
		{
			//GFPhysRigidBody * collidebodies[3];

			//int toolpartNum = m_pClampPlugin->m_ToolObject->GetCollideVeinObjectBody(collidebodies);
	
			//bool contactWithThisTool = false;
			//for(int i = 0 ; i < toolpartNum ; i++)
			//{
				//if(cdata->m_contactRigid == collidebodies[i])
				//{
				//	contactWithThisTool = true;
				//	break;
				//}
			//}

			//if(contactWithThisTool)
			//{
				CheckContactConn(cdata);
			//}
		}
		else
		{
			CheckNonContactConn(cdata);
		}
	}

	void CheckContactConn(VeinCollideData * veinCollideData)
	{
		VeinConnectObject *veinobj = veinCollideData->m_HostObject;
		VeinConnectCluster & cluster = veinobj->m_clusters[veinCollideData->m_ClusterIndex];


        VeinConnectPair &pair1 = cluster.m_pair[0];
        VeinConnectPair &pair2 = cluster.m_pair[1];

		GFPhysVector3 connVertrices[3];
		connVertrices[0] = veinCollideData->m_contactinWorld;
		connVertrices[1] = OgreToGPVec3(pair1.m_CurrPointPosA);
		connVertrices[2] = OgreToGPVec3(pair2.m_CurrPointPosA);

		GFPhysVector3 ptInRegion;
		GFPhysVector3 ptInFace;

		bool isContact = m_pClampPlugin->IsVeinConnContactClampRegion(connVertrices , ptInRegion , ptInFace , 0.0);

		if(isContact)
		{
			MisCTool_PluginClamp::VeinConnClamped connClamped(veinobj->m_OrganID , veinCollideData->m_ClusterIndex , veinCollideData->m_PairIndex);
			connClamped.m_contactPt = GPVec3ToOgre(ptInRegion);
			m_VeinConnClamp.push_back(connClamped);
		}
		else
		{
			connVertrices[1] = OgreToGPVec3(pair1.m_CurrPointPosB);
			connVertrices[2] = OgreToGPVec3(pair2.m_CurrPointPosB);
			isContact = m_pClampPlugin->IsVeinConnContactClampRegion(connVertrices , ptInRegion , ptInFace , 0.0);
			if(isContact)
			{
				MisCTool_PluginClamp::VeinConnClamped connClamped(veinobj->m_OrganID , veinCollideData->m_ClusterIndex , veinCollideData->m_PairIndex);
				connClamped.m_contactPt = GPVec3ToOgre(ptInRegion);
				m_VeinConnClamp.push_back(connClamped);
			}
		}
	}
	
	void CheckNonContactConn(VeinCollideData * veinCollideData)
	{
		VeinConnectObject *veinobj = veinCollideData->m_HostObject;
		VeinConnectCluster & cluster = veinobj->m_clusters[veinCollideData->m_ClusterIndex];

        VeinConnectPair &pair1 = cluster.m_pair[0];
        VeinConnectPair &pair2 = cluster.m_pair[1];

		GFPhysVector3 connVertrices[3];
		connVertrices[0] = OgreToGPVec3(pair1.m_CurrPointPosA);
		connVertrices[1] = OgreToGPVec3(pair1.m_CurrPointPosB);
		connVertrices[2] = OgreToGPVec3(pair2.m_CurrPointPosA);

		GFPhysVector3 ptInRegion;
		GFPhysVector3 ptInFace;

		bool isContact = m_pClampPlugin->IsVeinConnContactClampRegion(connVertrices , ptInRegion , ptInFace , 0);

		if(isContact)
		{
			MisCTool_PluginClamp::VeinConnClamped connClamped(veinobj->m_OrganID , veinCollideData->m_ClusterIndex , veinCollideData->m_PairIndex);
			connClamped.m_contactPt = GPVec3ToOgre(ptInRegion);
			m_VeinConnClamp.push_back(connClamped);
		}
		else
		{
			connVertrices[0] = OgreToGPVec3(pair1.m_CurrPointPosB);
			connVertrices[1] = OgreToGPVec3(pair2.m_CurrPointPosA);
			connVertrices[2] = OgreToGPVec3(pair2.m_CurrPointPosB);
			isContact = m_pClampPlugin->IsVeinConnContactClampRegion(connVertrices , ptInRegion , ptInFace , 0);
			if(isContact)
			{
				MisCTool_PluginClamp::VeinConnClamped connClamped(veinobj->m_OrganID , veinCollideData->m_ClusterIndex , veinCollideData->m_PairIndex);
				connClamped.m_contactPt = GPVec3ToOgre(ptInRegion);
				m_VeinConnClamp.push_back(connClamped);
			}
		}
	}


	MisCTool_PluginClamp *m_pClampPlugin;
	std::vector<MisCTool_PluginClamp::VeinConnClamped> m_VeinConnClamp;
};
//===========================================================================================================
void MisCTool_PluginClamp::ToolClampRegion::UpdateWorldData()
{
	const GFPhysTransform & worldtrans = m_AttachRigid->GetWorldTransform();
	const GFPhysMatrix3 & worldRotate = worldtrans.GetBasis();
	
	//record the old center position
	m_OriginWorldPrev = m_OriginWorld;

	//update world axis and normal etc
	m_Axis0World  = worldRotate*m_Axis0Local;
	m_Axis1World  = worldRotate*m_Axis1Local;
	
	m_OriginWorld = worldtrans*m_LocalOrigin;
	m_ClampNormalWorld = m_Axis0World.Cross(m_Axis1World).Normalized() * m_normalSign;
}
//===========================================================================================================
int  MisCTool_PluginClamp::TestFaceContactClampReg(GFPhysSoftBodyFace * face, const float threshold, GFPhysVector3 & ResultPtReg, GFPhysVector3 & ResultPtFace, float & contactDepth)
{
	int RegionInContact = -1;
	
	//triangle vertex pos and face normal
	GFPhysVector3 triVertsWorld[3];
	triVertsWorld[0] = face->m_Nodes[0]->m_CurrPosition;
	triVertsWorld[1] = face->m_Nodes[1]->m_CurrPosition;
	triVertsWorld[2] = face->m_Nodes[2]->m_CurrPosition;
	GFPhysVector3 faceNormal = (triVertsWorld[1]-triVertsWorld[0]).Cross(triVertsWorld[2]-triVertsWorld[0]).Normalized();

	GFPhysTransform transReg;
	GFPhysTransform transTri;

	transReg.SetIdentity();
	transTri.SetIdentity();

	//test every pair of clamp region represent by triangle pair
	bool isInMiddleReg = false;
	
	GFPhysVector3 closetPointReg;
	
	GFPhysVector3 closetPointTri;

	for(size_t t = 0 ; t < m_ClampSpaceCells.size() ; t++)
	{
		ClampCellData & clampcell = m_ClampSpaceCells[t];

		//
		float fTriVertLoc[3][3];

		GFPhysVector3 trilocalMin(FLT_MAX , FLT_MAX , FLT_MAX);
		
		GFPhysVector3 trilocalMax(-FLT_MAX , -FLT_MAX , -FLT_MAX);
		
		for(int c = 0 ; c < 3 ; c++)
		{
            GFPhysVector3 triLocalVert = clampcell.m_InvTrans(triVertsWorld[c]);
			
			fTriVertLoc[c][0] = triLocalVert.x();
			fTriVertLoc[c][1] = triLocalVert.y();
			fTriVertLoc[c][2] = triLocalVert.z();

			trilocalMin.SetMin(triLocalVert);
            trilocalMax.SetMax(triLocalVert);
		}
	
        if (TestAabbAgainstAabb2(clampcell.m_localmin, clampcell.m_localmax, trilocalMin, trilocalMax) == false)
		{
			clampcell.m_AABBTriOverlap = false;
			continue;
		}
		
#if(0)//further check if need
		if(TriangleBoxIntersect(m_ClampSpaceCells[t].m_localmin , m_ClampSpaceCells[t].m_localmax , fTriVertLoc) == false)
		{
			clampcell.m_AABBTriOverlap = false;
			continue;
		}
#endif
		clampcell.m_AABBTriOverlap = true;
	
		float closetDist = GetConvexsClosetPoint(clampcell.m_CellVertsWorldSpace , 6 , 0 ,
				                                 triVertsWorld , 3 , 0,
				                                 transReg , transTri ,
				                                 closetPointReg , closetPointTri , 
				                                 0.1f);

        if (closetDist <= threshold)//consider triangle in contact with region
		{
		   contactDepth = closetDist;//record contact dist
		   isInMiddleReg = true;
		   break;
		}
	}

	//if intersect middle region check which region belong this face
	if(isInMiddleReg)
	{
		float n0 = faceNormal.Dot(m_ClampReg[0].m_ClampNormalWorld);
		float n1 = faceNormal.Dot(m_ClampReg[1].m_ClampNormalWorld);
		if(n0 <= 0)
		{
		   RegionInContact = 0;
		}
		else
		{
		   RegionInContact = 1;
		}

		ResultPtReg  = closetPointReg;
		ResultPtFace = closetPointTri;
		return RegionInContact;
	}

	//if not intersect middle Reg we do addition check to the "extend" region
	float expandMargin = m_ClampRegThickNess;
	float n0 = faceNormal.Dot(m_ClampReg[0].m_ClampNormalWorld);
	float n1 = faceNormal.Dot(m_ClampReg[1].m_ClampNormalWorld);
	
	GFPhysVector3 ExtRegVert[6];
	int ExtRegToCheck = -1;

	if(n0 < -0.3f)//check region 0 's expand region
	{
	   ExtRegToCheck = 0;
	}
	else if(n1 < -0.3f)//check region 1 's expand region
	{
	   ExtRegToCheck = 1;
	}
	else
	{
	   return -1;//this face not clamped
	}

	bool isInExtReg = false;

	ToolClampRegion & extReg = m_ClampReg[ExtRegToCheck];

	for(size_t t = 0 ; t < m_ClampSpaceCells.size() ; t++)
	{
		const ClampCellData & clampcell = m_ClampSpaceCells[t];

		if(clampcell.m_AABBTriOverlap)//use previous result to quick cull out
		{
		   ExtRegVert[0] = clampcell.m_CellVertsWorldSpace[ExtRegToCheck*3];
		   ExtRegVert[1] = clampcell.m_CellVertsWorldSpace[ExtRegToCheck*3+1];
		   ExtRegVert[2] = clampcell.m_CellVertsWorldSpace[ExtRegToCheck*3+2];

		   ExtRegVert[3] = ExtRegVert[0] - extReg.m_ClampNormalWorld*expandMargin;
		   ExtRegVert[4] = ExtRegVert[1] - extReg.m_ClampNormalWorld*expandMargin;
		   ExtRegVert[5] = ExtRegVert[2] - extReg.m_ClampNormalWorld*expandMargin;

		   float closetDist = GetConvexsClosetPoint(ExtRegVert , 6 , 0 ,
				                                    triVertsWorld , 3 , 0,
				                                    transReg , transTri ,
				                                    closetPointReg , closetPointTri , 
				                                    0.1f);

           if (closetDist <= threshold)//consider triangle in contact with region
			{
			   contactDepth = closetDist;
			   isInExtReg = true;
			   break;
			}
		}
	}

	if(isInExtReg)
	{
		ResultPtReg  = closetPointReg;
		ResultPtFace = closetPointTri;
		return ExtRegToCheck;
	}
	else
	{
		return -1;
	}
}
//========================================================================================
bool MisCTool_PluginClamp::TestNodeContactClampReg(GFPhysSoftBodyNode * node)
{
    GFPhysTransform transReg;
    GFPhysTransform transTri;

    transReg.SetIdentity();
    transTri.SetIdentity();

    GFPhysVector3 closetPointReg;

    GFPhysVector3 closetPointTri;

    GFPhysVector3 triVertsWorld[1];

    triVertsWorld[0] = node->m_CurrPosition;

    for (size_t t = 0; t < m_ClampSpaceCells.size(); t++)
    {
        ClampCellData & clampcell = m_ClampSpaceCells[t];

        float closetDist = GetConvexsClosetPoint(clampcell.m_CellVertsWorldSpace, 6, 0,
            triVertsWorld, 1, 0,
            transReg, transTri,
            closetPointReg, closetPointTri,
            0.1f);
        if (closetDist < 0.1f)
        {
            return true;
        }
    }
    return false;
}
//========================================================================================

bool  MisCTool_PluginClamp::IsVeinConnContactClampRegion(GFPhysVector3 connVertrices[] ,
														GFPhysVector3 & ResultPtInClampReg , 
														GFPhysVector3 & ResultPtInFace , 
														float seperateDist /* = 0 */)
{

	GFPhysTransform transReg;
	GFPhysTransform transTri;

	transReg.SetIdentity();
	transTri.SetIdentity();

	GFPhysVector3 ClampRegionVert[6];

	bool isFound = false;

	//test every pair of clamp region represent by triangle pair
	for(size_t c = 0 ; c < m_ClampSpaceCells.size() ; c++)
	{
		ClampRegionVert[0] = m_ClampSpaceCells[c].m_CellVertsWorldSpace[0];
		ClampRegionVert[1] = m_ClampSpaceCells[c].m_CellVertsWorldSpace[1];
		ClampRegionVert[2] = m_ClampSpaceCells[c].m_CellVertsWorldSpace[2];

		ClampRegionVert[3] = m_ClampSpaceCells[c].m_CellVertsWorldSpace[3];
		ClampRegionVert[4] = m_ClampSpaceCells[c].m_CellVertsWorldSpace[4];
		ClampRegionVert[5] = m_ClampSpaceCells[c].m_CellVertsWorldSpace[5];


		GFPhysVector3 closetPointReg;
		GFPhysVector3 closetPointTri;

		float closetDist = GetConvexsClosetPoint(ClampRegionVert , 6 , 0 ,
			                                     connVertrices , 3 , 0,
			                                     transReg , transTri ,
			                                     closetPointReg , closetPointTri , 
			                                     seperateDist+0.1f);

		if(closetDist <= seperateDist)//0)//consider triangle in contact with region
		{
			isFound = true;
			ResultPtInClampReg = closetPointReg;
			ResultPtInFace  = closetPointTri;
		}

		if(isFound)
			return isFound;
	}
	return isFound;
}
//========================================================================================
MisCTool_PluginClamp::MisCTool_PluginClamp(CTool * tool , float checkClampStartShaft) : MisMedicCToolPluginInterface(tool)
{
	m_IsOpenAngleEnough = false;
	m_ClampStateStage = MisCTool_PluginClamp::CSS_Released;
	m_InVeinClamp = false;
    m_IsThreadClamp  = false;
	m_CanClampMultiBody = true;
	m_clampCheckAngleStart = checkClampStartShaft;//6.0f;//change by tool
	PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(this ,0);
	
	//m_OrganInClamp      = NULL;
	m_pClampedRope = NULL;
	m_pClampedRopeV2 = NULL;

	//m_OrganIgnored		= NULL;
	m_minShaftRangeUpperValue = FLT_MAX;
	m_minShaftRangeLowerValue = 1.0f;
	
	m_ShowClampRegion = false;//bacon -->true
	m_lastVeinforceFeedBack = GFPhysVector3(0,0,0);
	m_MoveIncrementInPointTo = 0;
	m_ClampMoveSpeed = 0.0f;
	m_MoveDistAfterClamped = 0.0f;

	m_ClampRegThickNess = 0.1f;

	m_MaxReleasingTime = 0.05f;
	//for debug, show the axis,clamp region 
	m_manual = 0;
	m_CanClampConnect = true;
    m_CanClampThread = true;
    m_CanClampLargeFace = true;
	m_CheckClampForAllOrgans = false;
	//m_TetraOrgan = 0;
    m_MaxPerSistentShaftAside = -100.0f;
	if(m_ShowClampRegion)
	   DrawClampAxisAndRegion();

	m_NormalSolveSoftDist = 0.0f;

	m_TubeClampProcessor = new MisCTool_TubeClamp((MisNewTraining*)m_ToolObject->GetOwnerTraining() , this);

}
//====================ReleaseClampedOrgans====================================================================
MisCTool_PluginClamp::~MisCTool_PluginClamp()
{
	ReleaseClampedOrgans();
	
	ReleaseClampedVeinConn();    

	if (m_pClampedRope)
	{
		ReleaseClampedRope();
	}
	if (m_pClampedRopeV2)
	{
		ReleaseClampedRopeV2();
	}

	m_TubeClampProcessor->ReleaseClampedSegment();

	if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
	   PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(this);

	if(m_ShowClampRegion)
	{
		m_manual->detachFromParent();
		MXOgreWrapper::Get()->GetDefaultSceneManger()->destroyManualObject(m_manual);
	}
	delete m_TubeClampProcessor;
}
void MisCTool_PluginClamp::SetMaxReleasingTime(float time)
{
    m_MaxReleasingTime = time;
}
//========================================================================================
void MisCTool_PluginClamp::OnOrganBeRemovedFromWorld(MisMedicOrganInterface * organif)
{
	if(organif == 0)
	   return;

	if(organif->GetCreateInfo().m_objTopologyType == DOT_VEINCONNECT)
	{
	   ReleaseClampedVeinConn();//temple release all veinconnect Object if one is destroyed need modify
	}
	else
	{
		for(size_t c = 0 ; c < m_ClampedOrgans.size() ; c++)
		{
			if(organif == m_ClampedOrgans[c]->m_organ)
			{
			   ReleaseClampedOrgans();

			   delete m_ClampedOrgans[c];//删除该抓取记录 因为该器官即将被删除

			   m_ClampedOrgans.erase(m_ClampedOrgans.begin()+c);
			   break;
			}
		}
	}
}
//========================================================================================
void MisCTool_PluginClamp::OnCustomSimObjBeRemovedFromWorld(MisCustomSimObj * simobj)
{
	if(simobj == m_pClampedRope)
	{
       ReleaseClampedRope();
	   m_pClampedRope = NULL;
	}
	if (simobj == m_pClampedRopeV2)
	{
		ReleaseClampedRopeV2();
		m_pClampedRopeV2 = NULL;
	}
}
//========================================================================================
void MisCTool_PluginClamp::InternalFreeClampedFaces(OrganBeClamped & organclamp)
{
	uint32 toolcat = (m_ToolObject->GetToolSide() == ITool::TSD_LEFT ? MMRC_LeftTool : MMRC_RightTool);

	//retrive clamped face's collision with rigid
	for (size_t f = 0; f < organclamp.m_ClampedFaces.size(); f++)
	{
		organclamp.m_ClampedFaces[f].m_PhysFace->m_RSCollisionMask |= toolcat;
	}

	//unmask clamped node
	std::map<GFPhysSoftBodyNode*, ClampedNodeData>::iterator itor = organclamp.m_ClampedNodes.begin();

	while (itor != organclamp.m_ClampedNodes.end())
	{
		GFPhysSoftBodyNode * clampNode = itor->first;

		if(itor->second.m_IsInClamp)//node clamped  by this tool
		{
		   clampNode->m_StateFlag &= (~EMMP_ClampByTool);
		   clampNode->m_StateFlag &= (~GPSESF_ATTACHED);
		}
		itor++;
	}

	//Real sor = organclamp.m_organ->m_physbody->GetVolumeSolveSor();
	//PhysicsWrapper::GetSingleTon().m_dynamicsWorld->ChangeVolumeElementSor(organclamp.m_TetrasInClampReg, sor);

	organclamp.m_IsReleased = true;//标记为已经释放即可，后面的face数组不清除，应为一些切割可能会用到
	//organclamp.m_ClampedNodes.clear();
	//organclamp.m_ClampedFaces.clear();
	///organclamp.m_EdgeClampedNodes.clear();
	//organclamp.m_TetrasInClampReg.clear();
}
//========================================================================================
void MisCTool_PluginClamp::ReleaseClampedOrgans()
{
	m_IgnorCoolDownTime = 0.0f;
	
	m_OrgansIgnored.clear();
	
	for(size_t c = 0 ; c < m_ClampedOrgans.size() ; c++)
	{
		InternalFreeClampedFaces(*m_ClampedOrgans[c]);

		MisMedicOrgan_Ordinary * organ = m_ClampedOrgans[c]->m_organ;

		//organ->m_IsGrasped = false;
		if(m_ToolObject->GetToolSide() == ITool::TSD_LEFT)
		   organ->SetClampedByLTool(false);
		else
		   organ->SetClampedByRTool(false);

		organ->m_ClampInstrumentType = "";
		
		SetIgnored(organ);
	}
	
	//m_ClampedOrgans.clear();//不clear一些Cut操作可能会用到。下次抓取时在clear

	m_ToolObject->SetMinShaftAside(0.0f);
	
	m_ClampStateStage = MisCTool_PluginClamp::CSS_Released;
}
//========================================================================================
void MisCTool_PluginClamp::ClearClampedVeinConn()
{
	m_ClampVeinConn.clear();
}
//========================================================================================
void MisCTool_PluginClamp::ReleaseClampedVeinConn()
{
	m_InVeinClamp = false;

	ITraining *currtrain = m_ToolObject->GetOwnerTraining();

	if(currtrain != NULL)
	{
		GFPhysRigidBody * collidebodies[3];

		int toolpartNum = m_ToolObject->GetCollideVeinObjectBody(collidebodies);

		for(size_t c = 0 ; c < (int)m_ClampVeinConn.size() ; c++)
		{
			VeinConnClamped & connClamped = m_ClampVeinConn[c];

			VeinConnectObject * pVeinobj = currtrain->GetVeinConnect(connClamped.m_VeinOrganId);

			if(pVeinobj != NULL)
			{
				for(int r = 0; r < toolpartNum; r++)//release all connect pair hooked by tool rigid
					pVeinobj->ReleaseConnectPairWithRigid(connClamped.m_ClusterId, collidebodies[r]);
			}
		}

		m_lastVeinforceFeedBack = GFPhysVector3(0,0,0);

	    ClearClampedVeinConn();
	}

}
//========================================================================================
bool MisCTool_PluginClamp::isInClampState()
{
	return (m_ClampStateStage == MisCTool_PluginClamp::CSS_Clamped);
}
//========================================================================================
void MisCTool_PluginClamp::GetOrgansBeClamped(std::vector<MisMedicOrgan_Ordinary *> & organsclamped)
{
	for (int c = 0; c < (int)m_ClampedOrgans.size(); c++)
	{
		if (false == m_ClampedOrgans[c]->m_IsReleased)
			organsclamped.push_back(m_ClampedOrgans[c]->m_organ);
	}
}
void MisCTool_PluginClamp::GetVeinConnectBeClamped(std::vector<VeinConnClamped> & connectclamped)
{
	connectclamped = m_ClampVeinConn;
}

int  MisCTool_PluginClamp::GetNumOrgansBeClamped()
{
	return (int)m_ClampedOrgans.size();
}

int MisCTool_PluginClamp::GetNumVeinConnectPairsBeClamped()
{
	return (int)m_ClampVeinConn.size();
}
//========================================================================================
void MisCTool_PluginClamp::SetIgnored(MisMedicOrgan_Ordinary *organ)
{
	//m_OrganIgnored = organ;
	m_OrgansIgnored.insert(organ);
}
//========================================================================================
void MisCTool_PluginClamp::ClearIgnored()
{
	m_OrgansIgnored.clear();
}
//========================================================================================
void MisCTool_PluginClamp::SetClampRegion(NewTrainToolConvexData & convexRigid ,//GFPhysRigidBody * attachRigid,
										  const GFPhysVector3 & center,
										  const GFPhysVector3 & axis0,
										  const GFPhysVector3 & axis1,
										  float size0,
										  float size1,
										  ClampRegSide regSide,
										  Real  normalSign)
{
	GFPhysVector3 invOffset(0,0,0);
	GFPhysMatrix3 invRot;
	invRot.SetIdentity();

	if(convexRigid.m_CompoundShape == 0 && convexRigid.m_CollideShapesData.size() > 0)//tool may refit convex vertex , compound shape not involve since they are in child transform
	{
	   if(convexRigid.m_CollideShapesData[0].m_ShapeType == 1)
	   {
          invOffset = -(OgreToGPVec3(convexRigid.m_CollideShapesData[0].m_boxcenter));
          
		  GFPhysQuaternion quat = (OgreToGPQuaternion(convexRigid.m_CollideShapesData[0].m_boxrotate));
          
		  invRot.SetRotation(quat.Inverse());
	   }
	}


	//GFPhysVector3 TestNormal = axis0.Cross(axis1).Normalized();
	m_ClampReg[regSide].m_LocalOrigin = (invRot * (center + invOffset));//center;
	m_ClampReg[regSide].m_Axis0Local  = (invRot * axis0);//axis0;
	m_ClampReg[regSide].m_Axis1Local  = (invRot * axis1);//axis1;
	m_ClampReg[regSide].m_HalfExt0 = size0;
	m_ClampReg[regSide].m_HalfExt1 = size1;
	m_ClampReg[regSide].m_AttachRigid = convexRigid.m_rigidbody;
	m_ClampReg[regSide].m_axis0Min = -size0;
	m_ClampReg[regSide].m_axis0Max = size0;
	m_ClampReg[regSide].m_axis1Min = -size1;
	m_ClampReg[regSide].m_axis1Max = size1;
	m_ClampReg[regSide].m_normalSign = normalSign;
	m_ClampReg[regSide].m_IsRect = true;
	m_ClampReg[regSide].m_RegSide = regSide;
	m_IsRect = true;
	//m_ClampReg[regSide].UpdateToWorldSpace();
	//UpdateClampRegionWorldAABB();
}
//========================================================================================
void MisCTool_PluginClamp::SetClampRegion(NewTrainToolConvexData & convexRigid, 
										  const GFPhysVector3 & center, 
										  const GFPhysVector3 & axis0, 
										  const GFPhysVector3 & axis1, 
										  Ogre::Vector2 triVertices[] , 
										  int numVertices, 
										  ClampRegSide regSide,
										  Real  normalSign)
{
	GFPhysVector3 invOffset(0,0,0);
	GFPhysMatrix3 invRot;
	invRot.SetIdentity();

	if(convexRigid.m_CompoundShape == 0 && convexRigid.m_CollideShapesData.size() > 0)//tool may refit convex vertex , compound shape not involve since they are in child transform
	{
		if(convexRigid.m_CollideShapesData[0].m_ShapeType == 1)
		{
			invOffset = -(OgreToGPVec3(convexRigid.m_CollideShapesData[0].m_boxcenter));

			GFPhysQuaternion quat = (OgreToGPQuaternion(convexRigid.m_CollideShapesData[0].m_boxrotate));

			invRot.SetRotation(quat.Inverse());
		}
	}

	m_ClampReg[regSide].m_AttachRigid = convexRigid.m_rigidbody;
	m_ClampReg[regSide].m_LocalOrigin = (invRot * (center + invOffset));//center;
	m_ClampReg[regSide].m_Axis0Local  = (invRot * axis0);//axis0;
	m_ClampReg[regSide].m_Axis1Local  = (invRot * axis1);//axis1;
	m_ClampReg[regSide].m_normalSign  = normalSign;
	m_ClampReg[regSide].m_IsRect = false;
	m_ClampReg[regSide].m_RegSide = regSide;
	m_IsRect = false;

	//add triangle vertex represent clamp region
	for(int i = 0 ; i < numVertices ; ++i)
	{
		Ogre::Vector2 coord = triVertices[i];
		m_ClampReg[regSide].m_triVertices.push_back(coord);
		m_ClampReg[regSide].m_axis0Min = min(m_ClampReg[regSide].m_axis0Min, coord.x);
		m_ClampReg[regSide].m_axis0Max = max(m_ClampReg[regSide].m_axis0Max, coord.x);
		m_ClampReg[regSide].m_axis1Min = min(m_ClampReg[regSide].m_axis1Min, coord.y);
		m_ClampReg[regSide].m_axis1Max = max(m_ClampReg[regSide].m_axis1Max, coord.y);
	}
	//
	//m_ClampReg[regSide].UpdateToWorldSpace();
	//UpdateClampRegionWorldAABB();
}
//========================================================================================
/*void MisCTool_PluginClamp::AddTriClampRegion(Ogre::Vector2 triVertices[] , int numVertices, int regionIndex)
{
	for(int i = 0 ; i < numVertices ; ++i)
	{
		m_ClampReg[regionIndex].m_triVertices.push_back(triVertices[i]);
		m_ClampReg[regionIndex].m_axis0Min = min(m_ClampReg[regionIndex].m_axis0Min ,triVertices[i].x);
		m_ClampReg[regionIndex].m_axis0Max = max(m_ClampReg[regionIndex].m_axis0Max ,triVertices[i].x);
		m_ClampReg[regionIndex].m_axis1Min = min(m_ClampReg[regionIndex].m_axis1Min ,triVertices[i].y);
		m_ClampReg[regionIndex].m_axis1Max = max(m_ClampReg[regionIndex].m_axis1Max ,triVertices[i].y);
	}
}
*/
//===========================================================================================================
void MisCTool_PluginClamp::CalculateVeinConnInClampRegions(
														  VeinConnectObject * veinobj)
{
	float softMargin = 0 ;


	GFPhysVector3 regRegMin = m_WorldClampRegMin;
	GFPhysVector3 regRegMax = m_WorldClampRegMax;

	ClampVeinConnTestCallBack callback(this);
	veinobj->GetCollideTree().TraverseTreeAgainstAABB(&callback , regRegMin , regRegMax);
	
	//ClearClampedVeinConn();

	//m_ClampVeinConn = callback.m_VeinConnClamp;

	for(size_t i = 0 ; i < callback.m_VeinConnClamp.size() ;i++)
		m_ClampVeinConn.push_back(callback.m_VeinConnClamp[i]);

}
//===========================================================================================================
void MisCTool_PluginClamp::PrepareSolveConstraint(Real Stiffness,Real TimeStep)
{
	for (int c = 0; c < (int)m_ClampedOrgans.size(); c++)
	{
		OrganBeClamped * clampedOrgan = m_ClampedOrgans[c];
		
		if (clampedOrgan->m_IsReleased == true)//this may redundunt since we relase organ at all
			continue;

		std::map<GFPhysSoftBodyNode*, MisCTool_PluginClamp::ClampedNodeData>::iterator itor = clampedOrgan->m_ClampedNodes.begin();
		while (itor != clampedOrgan->m_ClampedNodes.end())
		{
			MisCTool_PluginClamp::ClampedNodeData & clampNode = itor->second;
			clampNode.m_DragForces = GFPhysVector3(0, 0, 0);
			itor++;
		}

		for (size_t f = 0; f < clampedOrgan->m_ClampedFaces.size(); f++)
		{
			clampedOrgan->m_ClampedFaces[f].m_DragForce = Ogre::Vector3(0, 0, 0);
			clampedOrgan->m_ClampedFaces[f].m_FrictForce = Ogre::Vector3(0, 0, 0);
		}
	}

	for (size_t s = 0; s < m_ClampedRopeSegments.size(); s++)
	{
		m_ClampedRopeSegments[s].m_DragForce = GFPhysVector3(0, 0, 0);
	}

	m_TubeClampProcessor->PrepareSolve(TimeStep);
}
//===========================================================================================================
void MisCTool_PluginClamp::SolveConstraint(Real Stiffniss,Real TimeStep)
{
	uint32 opposeToolcat = 0;

	//如果一个面同时被左右手夹住,左手的面就要不参见解算
	if (m_ToolObject->GetToolSide() == ITool::TSD_LEFT)
		opposeToolcat = MMRC_RightTool;

	m_TubeClampProcessor->Solve(TimeStep , 0.08f);

    //solve clamp rope impulse
    if (m_IsThreadClamp)
	{
		if (m_pClampedRope)
		{
			const MisCTool_PluginClamp::ToolClampRegion & BelongRegionLeft = GetClampRegion(0);
			const MisCTool_PluginClamp::ToolClampRegion & BelongRegionRight = GetClampRegion(1);
			for (size_t s = 0; s < m_ClampedRopeSegments.size(); s++)
			{
				ClampedRopeSegData & ropeSegClamp = m_ClampedRopeSegments[s];

				int localindex = -1;
				SutureRopeNode & node = m_pClampedRope->GetThreadNodeGlobalRef(ropeSegClamp.m_SegmentGlobalIndex, localindex);
				if (-1 == localindex)
				{
					continue;
				}
				GFPhysVector3 ClampRopeForce(0, 0, 0);

				for (int i = 0; i < 2; i++)
				{
					SutureRopeNode & tnode = m_pClampedRope->GetThreadNodeRef(localindex + i);
					GFPhysVector3 srcpos = tnode.m_CurrPosition;

					GFPhysVector3 destPosleft = BelongRegionLeft.m_OriginWorld
						+ ropeSegClamp.m_Coord0[i] * BelongRegionLeft.m_Axis0World
						+ ropeSegClamp.m_Coord1[i] * BelongRegionLeft.m_Axis1World;

					if (tnode.IsStickInNeedle() == false)
					{
						tnode.m_CurrPosition += destPosleft - srcpos;
						for (int r = 0; r < 2; r++)
						{
							float CoordN = (tnode.m_CurrPosition - GetClampRegion(r).m_OriginWorld).Dot(GetClampRegion(r).m_ClampNormalWorld);
							float radiusinf = m_pClampedRope->GetRendRadius()*0.5f;
							if (CoordN < radiusinf)
							{
								GFPhysVector3 normalCorrect = GetClampRegion(r).m_ClampNormalWorld*(-CoordN + radiusinf);
								tnode.m_CurrPosition += normalCorrect;
								ClampRopeForce += normalCorrect;
							}
						}
					}


				}
				ropeSegClamp.m_DragForce += ClampRopeForce;
			}
		}

		if (m_pClampedRopeV2)
		{
			const MisCTool_PluginClamp::ToolClampRegion & BelongRegionLeft = GetClampRegion(0);
			const MisCTool_PluginClamp::ToolClampRegion & BelongRegionRight = GetClampRegion(1);
			for (size_t s = 0; s < m_ClampedRopeSegments.size(); s++)
			{
				ClampedRopeSegData & ropeSegClamp = m_ClampedRopeSegments[s];

				int localindex = -1;
				SutureThreadNodeV2 & node = m_pClampedRopeV2->GetThreadNodeGlobalRef(ropeSegClamp.m_SegmentGlobalIndex, localindex);
				if (-1 == localindex)
				{
					continue;
				}
				GFPhysVector3 ClampRopeForce(0, 0, 0);

				for (int i = 0; i < 2; i++)
				{
					GFPhysSoftTubeNode & tnode = m_pClampedRopeV2->GetThreadNodeRef(localindex + i);
					GFPhysVector3 srcpos = tnode.m_CurrPosition;

					GFPhysVector3 destPosleft = BelongRegionLeft.m_OriginWorld
						+ ropeSegClamp.m_Coord0[i] * BelongRegionLeft.m_Axis0World
						+ ropeSegClamp.m_Coord1[i] * BelongRegionLeft.m_Axis1World;

					if (true/*m_pClampedRopeV2->GetTubeWireSegment(localindex).IsAttached() == false*/)
					{
						tnode.m_CurrPosition += destPosleft - srcpos;
						for (int r = 0; r < 2; r++)
						{
							float CoordN = (tnode.m_CurrPosition - GetClampRegion(r).m_OriginWorld).Dot(GetClampRegion(r).m_ClampNormalWorld);
							float radiusinf = m_pClampedRopeV2->GetRendRadius()*0.5f;
							if (CoordN < radiusinf)
							{
								GFPhysVector3 normalCorrect = GetClampRegion(r).m_ClampNormalWorld*(-CoordN + radiusinf);
								tnode.m_CurrPosition += normalCorrect;
								ClampRopeForce += normalCorrect;
							}
						}
					}
				}


				ropeSegClamp.m_DragForce += ClampRopeForce;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	if(m_ClampStateStage == MisCTool_PluginClamp::CSS_InRelease || m_ClampStateStage == MisCTool_PluginClamp::CSS_Clamped)
	{
		bool needsolveTan = true;
		
		if(m_ClampStateStage == MisCTool_PluginClamp::CSS_InRelease)
		{
           needsolveTan = false;
		}

		for (int c = 0; c < (int)m_ClampedOrgans.size(); c++)
		{
			OrganBeClamped * clampedOrgan = m_ClampedOrgans[c];

			if (clampedOrgan->m_IsReleased == true)//this may redundunt since we relase organ at all
				continue;

			for (size_t f = 0; f < clampedOrgan->m_ClampedFaces.size(); f++)
			{
				SoftBodyFaceClamped & faceclamp = clampedOrgan->m_ClampedFaces[f];

				MisCTool_PluginClamp::ToolClampRegion & clampRegion = m_ClampReg[faceclamp.m_Part];

				//如果一个面同时被左右手夹住,左手的面就不参见解算
				if (opposeToolcat && (faceclamp.m_PhysFace->m_RSCollisionMask & opposeToolcat) == 0)
					continue;

				Ogre::Vector3 normalClampForce(0, 0, 0);
				Ogre::Vector3 tangentClampForce(0, 0, 0);

				GFPhysSoftBodyNode * tetraNode = 0;

				if (faceclamp.m_PhysFace && (faceclamp.m_PhysFace->m_IsMemFreed == false))
				{
					GFPhysSoftBodyTetrahedron * tetra = faceclamp.m_PhysFace->m_GenFace->m_ShareTetrahedrons[0].m_Hosttetra;

					for (int n = 0; n < 4; n++)
					{
						if (tetra->m_TetraNodes[n] != faceclamp.m_PhysFace->m_Nodes[0]
							&& tetra->m_TetraNodes[n] != faceclamp.m_PhysFace->m_Nodes[1]
							&& tetra->m_TetraNodes[n] != faceclamp.m_PhysFace->m_Nodes[2])
							tetraNode = tetra->m_TetraNodes[n];
					}
				}

				for (int n = 0; n < 3; n++)
				{
					GFPhysSoftBodyNode * physNode = faceclamp.m_PhysFace->m_Nodes[n];

					if (physNode == 0 || physNode->m_InvM < GP_EPSILON)//exclude static node
						continue;

					for (int r = 0; r < 2; r++)
					{
						float CoordN = (physNode->m_CurrPosition - m_ClampReg[r].m_OriginWorld).Dot(m_ClampReg[r].m_ClampNormalWorld);
						
						CoordN += m_NormalSolveSoftDist;

						if (CoordN < 0)
						{
							if (r != faceclamp.m_Part)
							{
								GFPhysVector3 normalCorrect = m_ClampReg[r].m_ClampNormalWorld*(-CoordN);
								physNode->m_CurrPosition += normalCorrect;
								normalClampForce += GPVec3ToOgre(normalCorrect);
							}
							else
							{
								GFPhysVector3 normalCorrect = m_ClampReg[r].m_ClampNormalWorld*(-CoordN) * faceclamp.m_NormalStiff[n];
								physNode->m_CurrPosition += normalCorrect;
								normalClampForce += GPVec3ToOgre(normalCorrect);
							}
						}
					}
					if (needsolveTan)
					{
						//GFPhysVector3 TanCorrect = (tangImpluse * 1.0f);
						//physNode->m_CurrPosition += TanCorrect;
						//tangentClampForce += GPVec3ToOgre(TanCorrect);
						float tan0 = (physNode->m_CurrPosition - clampRegion.m_OriginWorld).Dot(clampRegion.m_Axis0World);
						float tan1 = (physNode->m_CurrPosition - clampRegion.m_OriginWorld).Dot(clampRegion.m_Axis1World);

						float dt0 = faceclamp.m_NodeTanDist[n].x - tan0;
						float dt1 = faceclamp.m_NodeTanDist[n].y - tan1;

						float tanCorrStiff = faceclamp.m_TanStiff[n];// (m_ClampCluster.m_ClampMode == 1 ? 0.99f : 0.9f);
						GFPhysVector3 tanCorrect = (clampRegion.m_Axis0World * dt0 + clampRegion.m_Axis1World * dt1)*tanCorrStiff;
						physNode->m_CurrPosition += tanCorrect;

						tangentClampForce += GPVec3ToOgre(tanCorrect);
					}
				}

				if (faceclamp.m_NormalStiff[0] > 0.999f 
				 && faceclamp.m_NormalStiff[1] > 0.999f
				 && faceclamp.m_NormalStiff[2] > 0.999f
				 && tetraNode)
				{
					for (int r = 0; r < 2; r++)
					{
						float CoordN = (tetraNode->m_CurrPosition - m_ClampReg[r].m_OriginWorld).Dot(m_ClampReg[r].m_ClampNormalWorld);

						CoordN += m_NormalSolveSoftDist;

						if (CoordN < 0)
						{
							GFPhysVector3 normalCorrect = m_ClampReg[r].m_ClampNormalWorld*(-CoordN);
							tetraNode->m_CurrPosition += normalCorrect;
						}
					}
				}
				faceclamp.m_DragForce += (normalClampForce + tangentClampForce);
				faceclamp.m_FrictForce = tangentClampForce;
			}

			//edge
			
			std::map<GFPhysSoftBodyNode*, ClampedNodeData>::iterator itor = clampedOrgan->m_EdgeClampedNodes.begin();
			while (itor != clampedOrgan->m_EdgeClampedNodes.end())
			{
				GFPhysSoftBodyNode * physNode = itor->first;
				ClampedNodeData & nodeData = itor->second;
				for (int r = 0; r < 2; r++)
				{
					float CoordN = (physNode->m_CurrPosition - m_ClampReg[r].m_OriginWorld).Dot(m_ClampReg[r].m_ClampNormalWorld);
					if (CoordN < 0)
					{
						GFPhysVector3 normalCorrect = m_ClampReg[r].m_ClampNormalWorld*(-CoordN);
						physNode->m_CurrPosition += normalCorrect;
					}
				}
				MisCTool_PluginClamp::ToolClampRegion & clampRegion = m_ClampReg[0];

				float tan0 = (physNode->m_CurrPosition - clampRegion.m_OriginWorld).Dot(clampRegion.m_Axis0World);
				float tan1 = (physNode->m_CurrPosition - clampRegion.m_OriginWorld).Dot(clampRegion.m_Axis1World);

				float dt0 = nodeData.m_Tan0 - tan0;
				float dt1 = nodeData.m_Tan1 - tan1;

				GFPhysVector3 tanCorrect = (clampRegion.m_Axis0World * dt0 + clampRegion.m_Axis1World * dt1);
				physNode->m_CurrPosition += tanCorrect;

				itor++;
			}
		}
	}	
}
//===========================================================================================================
GFPhysVector3 MisCTool_PluginClamp::GetPluginForceFeedBack()
{
	if((m_ClampStateStage == MisCTool_PluginClamp::CSS_Clamped) && m_ClampedOrgans.size() > 0 )//bacon add m_OrganInClamp
	{	
		GFPhysVector3 DragForces(0,0,0);

		float GraspArea = 0;

		for (int c = 0; c < (int)m_ClampedOrgans.size(); c++)
		{
			OrganBeClamped * organClamped = m_ClampedOrgans[c];

			float dragrate = organClamped->m_organ->GetForceFeedBackRation();

			for (size_t f = 0; f < organClamped->m_ClampedFaces.size(); f++)
			{
				GFPhysSoftBodyFace * face = organClamped->m_ClampedFaces[f].m_PhysFace;
				
				DragForces += OgreToGPVec3(organClamped->m_ClampedFaces[f].m_DragForce) * dragrate;
			}

			return -DragForces + CalcVeinClampedForceFeedBack();
		}
	}
	else if(m_InVeinClamp)
	{
		return CalcVeinClampedForceFeedBack();
	}
	else if (m_IsThreadClamp && m_pClampedRope)
    {
        return CalcRopeClampedForceFeedBack();
    }
    else
    {
        return GFPhysVector3(0, 0, 0);
    }
    return GFPhysVector3(0, 0, 0);
}
//===========================================================================================================
GFPhysVector3 MisCTool_PluginClamp::CalcRopeClampedForceFeedBack()
{    
    if (m_pClampedRope->m_islock)//绳子可抗拉不抗压
    {
		GFPhysVector3 DragForces(0, 0, 0);
		Real deformlength = m_pClampedRope->GetTotalLen(true);
		Real undeformlength = m_pClampedRope->GetTotalLen(false);

		if (deformlength > undeformlength)
		{			
			int k = -1;
			SutureRopeNode & node = m_pClampedRope->GetThreadNodeGlobalRef(m_pClampedRope->m_ClampSegIndexVector[0], k);
			if (-1 == k)
			{
				return GFPhysVector3(0, 0, 0);
			}

			GFPhysVector3 n0, n1;
			m_pClampedRope->GetThreadSegmentNodePos(n0, n1, k);

			GFPhysVector3 end0, end1;
			m_pClampedRope->GetThreadSegmentNodePos(end0, end1, m_pClampedRope->GetNumThreadNodes() - 2);

			GFPhysVector3 tan0 = (end0 - n0).Normalized();

			return (deformlength - undeformlength)*tan0;
		}
		else
			return GFPhysVector3(0, 0, 0);
    }
	else
	{
		GFPhysVector3 force(0, 0, 0);
		const std::vector<ClampedRopeSegData>& ropesegs = GetRopeSegmentsBeClamped();
		for (int i = 0; i < ropesegs.size();i++)
		{
			force += ropesegs[i].m_DragForce;
		}
		return GFPhysVector3(0, 0, 0);
	}        	
}
//===========================================================================================================
GFPhysVector3 MisCTool_PluginClamp::CalcVeinClampedForceFeedBack()
{
	GFPhysVector3 force(0,0,0);

	if(m_InVeinClamp)
	{
		ITraining *currtrain = m_ToolObject->GetOwnerTraining();

		GFPhysRigidBody * collidebodies[3];

		int toolpartNum = m_ToolObject->GetCollideVeinObjectBody(collidebodies);

		GFPhysVector3 offsetDir = axis1World;

		offsetDir.Normalize();

		for(size_t c = 0 ; c < m_ClampVeinConn.size(); c++)
		{
			VeinConnClamped & connClamped = m_ClampVeinConn[c];

			VeinConnectObject * pVeinobj = currtrain->GetVeinConnect(connClamped.m_VeinOrganId);

			if(pVeinobj != NULL)
			{
				const VeinConnectPair & connpair = pVeinobj->GetConnectPair(connClamped.m_ClusterId , connClamped.m_PairId);

				if(connpair.m_Valid && connpair.m_BVNode != NULL)
				{
					VeinCollideData * cdata = (VeinCollideData *)connpair.m_BVNode->m_UserData;
					
					if(cdata->m_InHookState && ( cdata->m_contactRigid == collidebodies[0] || cdata->m_contactRigid == collidebodies[1]))
					{
						GFPhysVector3 temp = connpair.GetHookPointForce(offsetDir , false);
						force += temp;
					}
				}
			}
		}
	}

	//copy from ElectricHook.cpp  CElectricHook::CalcVeinHookForceFeedBack()
	float lastForce = m_lastVeinforceFeedBack.Length();

	float currMagnitude = force.Length();

	//prevent force increase suddenly
	float MaxIncPercent = 0.15f;
	if((currMagnitude - lastForce) > (currMagnitude*MaxIncPercent) )
	{
		if(currMagnitude > GP_EPSILON)
			force = force * ((lastForce + currMagnitude*MaxIncPercent) / currMagnitude);
	}

	//prevent force dir change suddenly
	GFPhysVector3 currdir = force.Normalized();

	GFPhysVector3 lastdir = m_lastVeinforceFeedBack.Normalized();

	float currmag = force.Length();

	if(currdir.Dot(lastdir) < 0.99f)
	{
		currdir = (9.0f*lastdir + currdir) / 10.0f;
		currdir.Normalize();
		force = currdir*currmag;
	}

	//prevent total force magnitude too large
	float MaxForce = 1.2f;
	float hookforcemag = force.Length();
	if(hookforcemag > MaxForce)
		force = force * (MaxForce / hookforcemag);

	m_lastVeinforceFeedBack = force;//LeftForceFeedBack += Ogre::Vector3(hookveinForce.x() , hookveinForce.y() , hookveinForce.z());

	return 8 * force;
}
//===========================================================================================================
void MisCTool_PluginClamp::onRSContactsBuildBegin(ConvexSoftFaceCollidePair * collidePairs , int NumCollidePair)
{
	
}
//===========================================================================================================
void MisCTool_PluginClamp::RigidSoftCollisionsSolved(const GFPhysAlignedVectorObj<GFPhysSoftRigidContact> & RSContactConstraints)
{
	
	 
}
//========================================================================================
void MisCTool_PluginClamp::onRSFaceContactsSolveEnded(const GFPhysAlignedVectorObj<GFPhysSoftFaceRigidContact> & RSContactConstraints)
{
	///m_FaceCollidedWithPart[ClampReg_Left] = 0;
	///m_FaceCollidedWithPart[ClampReg_Right] = 0;
	m_OrgansCollidedRightPart.clear();
	m_OrgansCollidedLeftPart.clear();

	for(size_t c = 0 ; c < RSContactConstraints.size() ; c++)
	{
        const GFPhysSoftFaceRigidContact & srContact = RSContactConstraints[c];
		
		float contactForce = srContact.GetNormalImpluse(0) 
			               + srContact.GetNormalImpluse(1)
			               + srContact.GetNormalImpluse(2);

		int PartSide = m_ToolObject->GetRigidBodyPart(srContact.m_Rigid);
		
		if (PartSide == 0 && contactForce > 0)
		{
		    MisMedicOrganInterface * organ = static_cast<MisMedicOrganInterface*>(srContact.m_SoftBody->GetUserPointer());
            if(organ)
		    {
               m_OrgansCollidedLeftPart.insert(organ);
		    }
		}
		else if (PartSide == 1 && contactForce > 0)
		{
			MisMedicOrganInterface * organ = static_cast<MisMedicOrganInterface*>(srContact.m_SoftBody->GetUserPointer());
			if(organ)
			{
			   m_OrgansCollidedRightPart.insert(organ);
			}
		}
	}
}
//========================================================================================
void MisCTool_PluginClamp::UpdateClampRegions()
{
	m_ClampSpaceCells.clear();
	
	GFPhysAlignedVectorObj<GFPhysVector3> Reg0TriVerts;
	GFPhysAlignedVectorObj<GFPhysVector3> Reg1TriVerts;

	for(int r = 0 ; r < 2 ; r++)
	{
		ToolClampRegion & reg = m_ClampReg[r];
	    reg.UpdateWorldData();

		if(m_IsRect)
		{
			GFPhysVector3 v00 = reg.m_OriginWorld + reg.m_Axis0World*reg.m_axis0Min + reg.m_Axis1World*reg.m_axis1Min;
			GFPhysVector3 v10 = reg.m_OriginWorld + reg.m_Axis0World*reg.m_axis0Max + reg.m_Axis1World*reg.m_axis1Min;
			GFPhysVector3 v11 = reg.m_OriginWorld + reg.m_Axis0World*reg.m_axis0Max + reg.m_Axis1World*reg.m_axis1Max;
			GFPhysVector3 v01 = reg.m_OriginWorld + reg.m_Axis0World*reg.m_axis0Min + reg.m_Axis1World*reg.m_axis1Max;

			if(r == 0)
			{
				Reg0TriVerts.push_back(v00);
				Reg0TriVerts.push_back(v10);
				Reg0TriVerts.push_back(v11);

				Reg0TriVerts.push_back(v00);
				Reg0TriVerts.push_back(v11);
				Reg0TriVerts.push_back(v01);
			}
			else
			{
				Reg1TriVerts.push_back(v00);
				Reg1TriVerts.push_back(v10);
				Reg1TriVerts.push_back(v11);

				Reg1TriVerts.push_back(v00);
				Reg1TriVerts.push_back(v11);
				Reg1TriVerts.push_back(v01);
			}
		}
		else
		{
			for(size_t c = 0 ; c < reg.m_triVertices.size() ; c++)
			{
				GFPhysVector3 physPos = reg.m_Axis0World * reg.m_triVertices[c].x + reg.m_Axis1World * reg.m_triVertices[c].y;
				physPos = physPos + reg.m_OriginWorld;
				if(r == 0)
				   Reg0TriVerts.push_back(physPos);
				else
				   Reg1TriVerts.push_back(physPos);
			}
		}
	}

	//build every cell
	int cellNum = (Reg0TriVerts.size() <= Reg1TriVerts.size() ? Reg0TriVerts.size() : Reg1TriVerts.size());
	cellNum /= 3;
	for(int c = 0 ; c < cellNum ; c++)
	{
        ClampCellData celldata;
	
		celldata.m_CellVertsWorldSpace[0] = Reg0TriVerts[c*3];
		celldata.m_CellVertsWorldSpace[1] = Reg0TriVerts[c*3+1];
		celldata.m_CellVertsWorldSpace[2] = Reg0TriVerts[c*3+2];

		celldata.m_CellVertsWorldSpace[3] = Reg1TriVerts[c*3];
		celldata.m_CellVertsWorldSpace[4] = Reg1TriVerts[c*3+1];
		celldata.m_CellVertsWorldSpace[5] = Reg1TriVerts[c*3+2];
		
		
		GFPhysVector3 center;
		GFPhysMatrix3 rotMat;
		GFPhysVector3 halfextend;

		bool succed = CalConvexHullBestFitFrame(celldata.m_CellVertsWorldSpace , 6 , center , rotMat , halfextend);
		if(succed)
		{
           celldata.m_localmin = -halfextend;
           
		   celldata.m_localmax = halfextend;
		   
		   celldata.m_transform = GFPhysTransform(rotMat , center);
	      
		   GFPhysTransform invTrans = celldata.m_transform.Inverse();
		   
		   //transform back
		   celldata.m_CellVertsReg0[0] = invTrans(celldata.m_CellVertsWorldSpace[0]);
		   celldata.m_CellVertsReg0[1] = invTrans(celldata.m_CellVertsWorldSpace[1]);
		   celldata.m_CellVertsReg0[2] = invTrans(celldata.m_CellVertsWorldSpace[2]);

		   celldata.m_CellVertsReg1[0] = invTrans(celldata.m_CellVertsWorldSpace[3]);
		   celldata.m_CellVertsReg1[1] = invTrans(celldata.m_CellVertsWorldSpace[4]);
		   celldata.m_CellVertsReg1[2] = invTrans(celldata.m_CellVertsWorldSpace[5]);

		   //
		   GFPhysVector3 testmin(FLT_MAX , FLT_MAX , FLT_MAX);
		   GFPhysVector3 testmax(-FLT_MAX , -FLT_MAX , -FLT_MAX);

		   for(int t = 0 ; t < 3 ; t++)
		   {
			   testmin.SetMin(celldata.m_CellVertsReg0[t]);
			   testmax.SetMax(celldata.m_CellVertsReg0[t]);

			   testmin.SetMin(celldata.m_CellVertsReg1[t]);
			   testmax.SetMax(celldata.m_CellVertsReg1[t]);
		   }

		   //
		}
		else
		{
           celldata.m_transform.SetIdentity();
		  
		   celldata.m_CellVertsReg0[0] = celldata.m_CellVertsWorldSpace[0];
		   celldata.m_CellVertsReg0[1] = celldata.m_CellVertsWorldSpace[1];
		   celldata.m_CellVertsReg0[2] = celldata.m_CellVertsWorldSpace[2];

		   celldata.m_CellVertsReg1[0] = celldata.m_CellVertsWorldSpace[3];
		   celldata.m_CellVertsReg1[1] = celldata.m_CellVertsWorldSpace[4];
		   celldata.m_CellVertsReg1[2] = celldata.m_CellVertsWorldSpace[5];

		   celldata.m_localmin = celldata.m_localmax = celldata.m_CellVertsWorldSpace[0];
		   
		   for(int t = 1 ; t < 6 ; t++)
		   {
              celldata.m_localmin.SetMin(celldata.m_CellVertsWorldSpace[t]);
			  celldata.m_localmax.SetMax(celldata.m_CellVertsWorldSpace[t]);
		   }
		}

		//save inverse trans for further use
		celldata.m_InvTrans = celldata.m_transform.Inverse();

		//add expand region thickness in to account
		GFPhysVector3 clampThick(m_ClampRegThickNess , m_ClampRegThickNess , m_ClampRegThickNess);
		celldata.m_localmin -= clampThick;
		celldata.m_localmax += clampThick;

		//
		m_ClampSpaceCells.push_back(celldata);
	}

	//update world aabb
	m_WorldClampRegMin = GFPhysVector3(GP_INFINITY  , GP_INFINITY  , GP_INFINITY);

	m_WorldClampRegMax = GFPhysVector3(-GP_INFINITY , -GP_INFINITY , -GP_INFINITY);

	for(size_t c = 0 ; c < m_ClampSpaceCells.size() ; c++)
	{
        ClampCellData & cellData = m_ClampSpaceCells[c];

	    //cell world aabb box
		GetWorldAABB(cellData.m_transform ,
			         cellData.m_localmin ,
			         cellData.m_localmax , 
			         cellData.m_worldmin,
			         cellData.m_worldmax);

		//total region world aabb
		m_WorldClampRegMin.SetMin(cellData.m_worldmin);
		m_WorldClampRegMax.SetMax(cellData.m_worldmax);
	}
}
//===========================================================================================================
void MisCTool_PluginClamp::GetClampSpaceWorldAABB(GFPhysVector3 & regMin, GFPhysVector3 & regMax)
{
	regMin = m_WorldClampRegMin;
	regMax = m_WorldClampRegMax;
}
//========================================================================================
void MisCTool_PluginClamp::PhysicsSimulationStart(int currStep , int TotalStep , float dt)
{
	UpdateClampRegions();
}
//========================================================================================
bool MisCTool_PluginClamp::CheckVeinObjBeClamped()
{
	if(!m_CanClampConnect)
	    return false;

	float shaftTool = m_ToolObject->GetShaftAside();
	
	if(shaftTool > 5.0f)
		return false;

	if(shaftTool <= 2.0f)
	{
		if(!m_InVeinClamp)
		{
			ITraining * currtrain = m_ToolObject->GetOwnerTraining();
			
			//than check
			if(m_ClampVeinConn.size() >0)
				m_InVeinClamp = true;
			else
				return false;

			GFPhysRigidBody * collidebodies[3];

			int toolpartNum = m_ToolObject->GetCollideVeinObjectBody(collidebodies);

			for(size_t c = 0 ; c < m_ClampVeinConn.size(); c++)
			{
				VeinConnClamped & connClamped = m_ClampVeinConn[c];
				
				VeinConnectObject * pVeinobj = currtrain->GetVeinConnect(connClamped.m_VeinOrganId);
				
				if(pVeinobj != NULL)
				{
					const VeinConnectPair & connpair = pVeinobj->GetConnectPair(connClamped.m_ClusterId , connClamped.m_PairId);

					if(connpair.m_Valid && connpair.m_BVNode != NULL)
					{
						VeinCollideData * cdata = (VeinCollideData *)connpair.m_BVNode->m_UserData;

						if(pVeinobj->SetHookInfo(connClamped.m_ClusterId , connClamped.m_PairId))
						{
							if(cdata->m_InContact) {
								pVeinobj->SetConnectPairClampByRigid(connClamped.m_ClusterId, connClamped.m_PairId, cdata->m_contactRigid, OgreToGPVec3(connClamped.m_contactPt));//cdata->m_contactinWorld);
							} else if(toolpartNum > 0) {
								pVeinobj->SetConnectPairClampByRigid(connClamped.m_ClusterId, connClamped.m_PairId, collidebodies[0], OgreToGPVec3(connClamped.m_contactPt));
							}
						}
					}
				}
			}
			return true;
		} 
		else
		{
			bool inClamp = false;

			GFPhysRigidBody * collidebodies[3];

			int toolpartNum = m_ToolObject->GetCollideVeinObjectBody(collidebodies);

			for(size_t c = 0 ; c < m_ClampVeinConn.size(); c++)
			{
				VeinConnClamped & connClamped = m_ClampVeinConn[c];

				ITraining * currtrain = m_ToolObject->GetOwnerTraining();

				VeinConnectObject * pVeinobj = currtrain->GetVeinConnect(connClamped.m_VeinOrganId);

				if(pVeinobj != NULL)
				{
					const VeinConnectPair & connpair = pVeinobj->GetConnectPair(connClamped.m_ClusterId , connClamped.m_PairId);

					if(connpair.m_Valid && connpair.m_BVNode != NULL)
					{
						VeinCollideData * cdata = (VeinCollideData *)connpair.m_BVNode->m_UserData;

						if(cdata->m_InHookState) {
							if(cdata->m_contactRigid == collidebodies[0] || cdata->m_contactRigid == collidebodies[1]) {
								inClamp = true;
								break;
							}
						}
					}
				}
			}
			if(inClamp)
				return true;
			else
			{
				m_InVeinClamp = false;
				ClearClampedVeinConn();
				return false;
			}
		}
	}
	else if(shaftTool > 2.0f)
	{
		if(m_InVeinClamp)
		{
			ReleaseClampedVeinConn();
			//m_InVeinClamp = false;
		}
		else
		{
		   //first detect veinconnect in clamp region
		   ITraining * currtrain = m_ToolObject->GetOwnerTraining();

		   ClearClampedVeinConn();

		   std::vector<VeinConnectObject*>  veinobjs = currtrain->GetVeinConnectObjects();

		   for (size_t v = 0; v < veinobjs.size(); v++)
		   {
			 CalculateVeinConnInClampRegions(veinobjs[v]);
		   }

		}
		return false;
	}
	return false;
}

//==================================================================================
int MisCTool_PluginClamp::DisconnectClampedVeinConnectPairs()
{
	ITraining * currtrain = m_ToolObject->GetOwnerTraining();

	std::vector<VeinConnClamped> ConnectPairToDisconnects;

	ConnectPairToDisconnects = m_ClampVeinConn;

	ReleaseClampedVeinConn();

	for (int c = 0; c < (int)ConnectPairToDisconnects.size(); c++)
	{
		VeinConnClamped & connClamped = ConnectPairToDisconnects[c];

		VeinConnectObject * pVeinobj = currtrain->GetVeinConnect(connClamped.m_VeinOrganId);

		pVeinobj->DisconnectPair(connClamped.m_ClusterId);
	}

	return (int)ConnectPairToDisconnects.size();
}
//==================================================================================
bool MisCTool_PluginClamp::IsEdgeBeClamped(GFPhysSoftBodyEdge * edge)
{
	GFPhysVector3 edgeVertsWorld[2];
	edgeVertsWorld[0] = edge->m_Nodes[0]->m_CurrPosition;
	edgeVertsWorld[1] = edge->m_Nodes[1]->m_CurrPosition;

	//further check clamp cell
	if (IsLineSegAABBOverLap(m_WorldClampRegMin, 
		                     m_WorldClampRegMax,
							 edgeVertsWorld[0],
							 edgeVertsWorld[1]))
	{
		GFPhysTransform transReg;
		GFPhysTransform transEdge;

		transReg.SetIdentity();
		transEdge.SetIdentity();

		//test every pair of clamp region represent by triangle pair
		bool isInMiddleReg = false;

		GFPhysVector3 closetPointReg;

		GFPhysVector3 closetPointEdge;

		for (size_t t = 0; t < m_ClampSpaceCells.size(); t++)
		{
			ClampCellData & clampcell = m_ClampSpaceCells[t];
            
			if (IsLineSegAABBOverLap(clampcell.m_worldmin,
				                     clampcell.m_worldmax,
									 edgeVertsWorld[0],
									 edgeVertsWorld[1]))
			{
				float closetDist = GetConvexsClosetPoint(clampcell.m_CellVertsWorldSpace, 6, 0,
					                                     edgeVertsWorld, 2, 0,
					                                     transReg, transEdge,
														 closetPointReg, closetPointEdge,
					                                     0.1f);

				if (closetDist <= 0)//consider triangle in contact with region
				{
					return true;
				}
			}
		}
	}
	return false;
}
//========================================================================================================================
void MisCTool_PluginClamp::SelectOrgansToClamp(std::set<MisMedicOrgan_Ordinary*> & organSet,
	                                           bool allowUnTouchClamp)
{
	//if we have at least 1 face in each part clamp
	//we consider it as clamp state
	bool  clampSucced = false; 
	float minShaftToSet = 0.5f;

	GFPhysVector3 regMin = m_WorldClampRegMin;//use local variable else will miss 16 lignement thus crash in TraverseFaceTreeAgainstAABB
	GFPhysVector3 regMax = m_WorldClampRegMax;

	std::set<MisMedicOrgan_Ordinary *>::iterator organItor;
	
	float maxArea = 0.0f;
	
	//check all candidate organs can be clamp
	for(organItor = organSet.begin() ; organItor != organSet.end() ;  organItor++)
	{
		MisMedicOrgan_Ordinary * pCurrOrgan = (*organItor);

		if(allowUnTouchClamp == false && 
		(m_OrgansCollidedLeftPart.find(pCurrOrgan) == m_OrgansCollidedLeftPart.end() || m_OrgansCollidedRightPart.find(pCurrOrgan) == m_OrgansCollidedRightPart.end()))
		   continue;

		if (pCurrOrgan->CanBeGrasp() == false)
			continue;

		if(pCurrOrgan && (m_OrgansIgnored.find(pCurrOrgan) == m_OrgansIgnored.end()))
		{
			OrganBeClamped * clampedOrganData = new OrganBeClamped(pCurrOrgan);

			GFPhysSoftBody * pCurrSoftBody = pCurrOrgan->m_physbody;

			SoftFaceContactClampRegCB callback;
	
			pCurrSoftBody->GetSoftBodyShape().TraverseFaceTreeAgainstAABB(&callback , regMin , regMax);
			
			float currTotalArea = 0.0f;
			
			int NumFaceContactReg[2] = { 0 , 0 };//int FaceInClampReg[2] = { 0 , 0 };

			int ValidFrictionFaceCount[2] = { 0 , 0 };//int PressedForceCount[2] = {0 , 0};

			float validClampArea = 0.0f;
		
			//check potential faces be clamped
            for(size_t f = 0 ; f < callback.m_SoftFacesToCheck.size() ; f++)
			{
                GFPhysSoftBodyFace * physFace = callback.m_SoftFacesToCheck[f];
                
				GFPhysVector3 contactPtOnFace;
				
				GFPhysVector3 contactPtOnReg;
				
				float contactDepth;

				int FaceContactRegion = TestFaceContactClampReg(physFace ,0.0f, contactPtOnReg , contactPtOnFace , contactDepth);

				if(FaceContactRegion >= 0 && (physFace->m_CollideFlag & GPSECD_RIGID))
				{
					float facePtWeights[3];

					//calculate region contact point in face's weights
					GFPhysVector3 cPoint = ClosestPtPointTriangle(contactPtOnReg ,
						                                          physFace->m_Nodes[0]->m_CurrPosition, 
						                                          physFace->m_Nodes[1]->m_CurrPosition, 
						                                          physFace->m_Nodes[2]->m_CurrPosition);

					CalcBaryCentric(physFace->m_Nodes[0]->m_CurrPosition, 
						            physFace->m_Nodes[1]->m_CurrPosition, 
						            physFace->m_Nodes[2]->m_CurrPosition,
						            cPoint,
						            facePtWeights[0],
						            facePtWeights[1],
						            facePtWeights[2]);

					MisCTool_PluginClamp::SoftBodyFaceClamped clampface(pCurrOrgan , physFace , FaceContactRegion , facePtWeights , contactDepth);
                    
					GFPhysVector3 clampRegNorm = m_ClampReg[FaceContactRegion].m_ClampNormalWorld;
					
					GFPhysVector3 physFaceNorm = (physFace->m_Nodes[1]->m_CurrPosition-physFace->m_Nodes[0]->m_CurrPosition).Cross(physFace->m_Nodes[2]->m_CurrPosition-physFace->m_Nodes[0]->m_CurrPosition).Normalized();
					
					NumFaceContactReg[FaceContactRegion]++;

					currTotalArea += physFace->m_CurrAreaMult2;

					//face angle with clamp normal small then some threshold be see as really un-slipped(enough friction) face
                    if(clampRegNorm.Dot(physFaceNorm) < -0.5f)
					{
					   ValidFrictionFaceCount[FaceContactRegion]++;
					   validClampArea += physFace->m_CurrAreaMult2;
					}
					clampedOrganData->m_ClampedFaces.push_back(clampface);
				}	
			} 

			bool organCanBeClamp = false;

			if(NumFaceContactReg[0] > 0 && NumFaceContactReg[1] > 0) 
			{
				if(ValidFrictionFaceCount[0] > 0 && ValidFrictionFaceCount[1] > 0)
				{
					organCanBeClamp = true;
				} 
				clampedOrganData->m_ClampMode = 0;
			}

			
			if (organCanBeClamp == false && m_ToolObject->m_ToolColliedFaces.size() > 0)
            {
				if (allowUnTouchClamp == false && m_CanClampLargeFace && m_MaxPerSistentShaftAside > 10.0f)
                {
					clampedOrganData->m_ClampedFaces.clear();
                                       
                    for (int f = 0; f < (int)m_ToolObject->m_ToolColliedFaces.size(); f++)
                    {                        
                        const ToolCollidedFace & datacd = m_ToolObject->m_ToolColliedFaces[f];
						
						if(datacd.m_collideSoft != pCurrOrgan->m_physbody)
							continue;
						
						if (datacd.m_IsBackFace == true)
							continue;

						if (datacd.m_depth >= 0.01f)
							continue;

						int contactPart = -1;
						
						if (datacd.m_collideRigid == m_ClampReg[0].m_AttachRigid)
							contactPart = 0;
						
						else if (datacd.m_collideRigid == m_ClampReg[1].m_AttachRigid)
							contactPart = 1;
						
						if (contactPart < 0)
							continue;

						if (datacd.m_collideFace->m_FaceNormal.Dot(GetClampRegNormal(contactPart)) > 0.2f)
							continue;

						if (datacd.m_FaceContactImpluse[0] <= 0.5 && datacd.m_FaceContactImpluse[1] <= 0.5 && datacd.m_FaceContactImpluse[2] <= 0.5)
							continue;

                        GFPhysSoftBodyFace * physFace = datacd.m_collideFace;

						//GFPhysSoftBodyTetrahedron * tetra = physFace->m_GenFace->m_ShareTetrahedrons[0].m_Hosttetra;

						//Real currVol = tetra->CalculateTetraSignedVolume();
						//if (fabsf((currVol - tetra->m_RestSignedVolume) / tetra->m_RestSignedVolume) < 0.5f)
						//	continue;

						MisCTool_PluginClamp::SoftBodyFaceClamped clampface(pCurrOrgan, physFace, contactPart, datacd.m_collideWeights, datacd.m_depth);
						clampface.m_AngleToClampNorm = 1 - GPClamped(fabsf(clampface.m_PhysFace->m_FaceNormal.Dot(GetClampRegNormal(contactPart))), 0.0f, 1.0f);
						clampface.m_CollideNorm = GPVec3ToOgre(datacd.m_CollideNormal);
						clampedOrganData->m_ClampedFaces.push_back(clampface);
                    }    
                   
					if (clampedOrganData->m_ClampedFaces.size() > 0)//模式1选取面积最小的
                    {
						sort(clampedOrganData->m_ClampedFaces.begin(), clampedOrganData->m_ClampedFaces.end());
						clampedOrganData->m_ClampMode = 1;
						organCanBeClamp = true;
                    }
                }
            }

			if(!organCanBeClamp)//not clamped this organ erase clamp faces off this organ
			{
				delete clampedOrganData;
				clampedOrganData = 0;
			}
			else
			{
				m_ClampedOrgans.push_back(clampedOrganData);
			}
		}
	}
}
//===============================================================================================================
bool MisCTool_PluginClamp::CheckOrganBeClampedV2()
{
	std::set<MisMedicOrgan_Ordinary *> OrganToCheck;

	MisNewTraining * currtrain = dynamic_cast<MisNewTraining *>(m_ToolObject->GetOwnerTraining());

	for(int f = 0 ; f < (int)m_ToolObject->m_ToolColliedFaces.size() ; f++)
	{
		const ToolCollidedFace & datacd = m_ToolObject->m_ToolColliedFaces[f];
		GFPhysSoftBody * softbody = (GFPhysSoftBody*)datacd.m_collideSoft;
		MisMedicOrgan_Ordinary * organ = dynamic_cast<MisMedicOrgan_Ordinary*>(currtrain->GetOrgan(softbody));
		if (organ->CanBeGrasp())
		    OrganToCheck.insert(organ);
	}
	//remove old data in last clamp first!
	for (int c = 0; c < (int)m_ClampedOrgans.size(); c++)
	{
		delete m_ClampedOrgans[c];
	}
	m_ClampedOrgans.clear();
	//

	SelectOrgansToClamp(OrganToCheck , false);
	
	//
	if (m_CheckClampForAllOrgans)
	{
		if (m_ClampedOrgans.size() > 0)
		{
			std::vector<MisMedicOrganInterface*> furtherogans;

			((MisNewTraining*)m_ToolObject->GetOwnerTraining())->GetAllOrgan(furtherogans);

			std::set<MisMedicOrgan_Ordinary*> additionOrgans;
			for (int c = 0; c < (int)furtherogans.size(); c++)
			{
				MisMedicOrgan_Ordinary * organ = dynamic_cast<MisMedicOrgan_Ordinary*>(furtherogans[c]);

				if (organ)// && m_ClampedOrgans.find(organ) == m_ClampedOrgans.end())
				{
					bool addTest = true;
					for (int t = 0; t < (int)m_ClampedOrgans.size(); t++)
					{
						if (m_ClampedOrgans[t]->m_organ == organ)
						{
							addTest = false;
							break;
						}
					}
					if (addTest)
					    additionOrgans.insert(organ);
				}
			}
			SelectOrgansToClamp(additionOrgans , true);
		}
	}
	
	//filter organ that we do not want in special train
	if(m_ClampedOrgans.size() > 0)
	{
		std::vector<MisMedicOrgan_Ordinary*> organsToFilter;
		for (int c = 0; c < (int)m_ClampedOrgans.size(); c++)
		{
			organsToFilter.push_back(m_ClampedOrgans[c]->m_organ);
		}

		MisMedicOrgan_Ordinary * filterOutOrgan = currtrain->FilterClampedOrgan(organsToFilter);
		if (filterOutOrgan)
		{
			for (int c = 0; c < (int)m_ClampedOrgans.size(); c++)
			{
				OrganBeClamped * clampedOrgan = m_ClampedOrgans[c];
				if (clampedOrgan->m_organ == filterOutOrgan)
				{
					m_ClampedOrgans.erase(m_ClampedOrgans.begin() + c);
					delete clampedOrgan;
					break;
				}
			}
		}
	}

	//final organs be clamped
	if(m_ClampedOrgans.size() > 0)
	{

	   float shaftTool = m_ToolObject->GetShaftAside();

	   float minShaftToSet = 0.5f;

	   m_ClampStateStage = MisCTool_PluginClamp::CSS_Clamped;

	   m_IsOpenAngleEnough = false;

	   minShaftToSet = 0.25f;// shaftTool - 1.0f;

	   minShaftToSet = GPClamped(minShaftToSet , m_minShaftRangeLowerValue , m_minShaftRangeUpperValue);

	   m_ToolObject->SetMinShaftAside(minShaftToSet);

	   m_ClampCenterAtClamped = (m_ClampReg[0].m_OriginWorld + m_ClampReg[1].m_OriginWorld) * 0.5;

	   m_MoveDistAfterClamped = 0.0f;

	   for(size_t c = 0 ; c < m_ClampedOrgans.size() ; c++)
	   {
		   m_ClampedOrgans[c]->m_ClampedNodes.clear();

		   MisMedicOrgan_Ordinary * organclamp = m_ClampedOrgans[c]->m_organ;

		   organclamp->m_FaceMoveIncrementInClamp = 0.f;

		   organclamp->m_FaceMoveSpeedInClamp = 0.f;

		   //organclamp->m_IsGrasped = true;
		   if(m_ToolObject->GetToolSide() == ITool::TSD_LEFT)
			   organclamp->SetClampedByLTool(true);
		   else
			   organclamp->SetClampedByRTool(true);

		   organclamp->m_ClampInstrumentType = m_ToolObject->GetType();

		   ProcessFacesAfterClamped(*m_ClampedOrgans[c]);
		   CollectTetrasInClampRegion(*m_ClampedOrgans[c]);
	   }
	   m_ReleaseClampShaft = shaftTool+0.1f;

	   if(m_ReleaseClampShaft < 2)
		  m_ReleaseClampShaft = 2;//temple need refractory
	}
	else
	{
	   m_ClampStateStage = MisCTool_PluginClamp::CSS_Released;
	}

	//send clamp message to owner train
	if (m_ClampedOrgans.size() > 0)
	{
		MisNewTraining * pNewTraining = dynamic_cast<MisNewTraining*>(m_ToolObject->GetOwnerTraining());

		std::vector<Ogre::Vector2> texCoords;

		std::vector<Ogre::Vector2> unused;

		for (int c = 0; c < (int)m_ClampedOrgans.size(); c++)
		{
			std::vector<SoftBodyFaceClamped> & clampFaces = m_ClampedOrgans[c]->m_ClampedFaces;

			MisMedicOrgan_Ordinary * currOrgan = m_ClampedOrgans[c]->m_organ;

			texCoords.clear();

			for (size_t f = 0; f < clampFaces.size(); f++)
			{
				SoftBodyFaceClamped & face = clampFaces[f];
				Ogre::Vector2 textureCoord = currOrgan->GetTextureCoord(face.m_PhysFace, face.m_ContactPtWeights);
				texCoords.push_back(textureCoord);
			}

			pNewTraining->receiveCheckPointList(MisNewTraining::OCPT_Clamp, texCoords, unused, m_ToolObject, currOrgan);
		}
	}


	return (m_ClampStateStage == MisCTool_PluginClamp::CSS_Clamped);
}
//========================================================================================
void MisCTool_PluginClamp::ProcessFacesAfterClamped(OrganBeClamped & clampOrgan)
{
	uint32 toolcat = (m_ToolObject->GetToolSide() == ITool::TSD_LEFT ? MMRC_LeftTool : MMRC_RightTool);
	


	//calculate clamped node's tangent and normal coordinate
	GFPhysVector3 ClampRegNormals[2];
	ClampRegNormals[0] = GetClampRegNormal(0);
	ClampRegNormals[1] = GetClampRegNormal(1);

	//calculate compress direction
	std::vector<GFPhysSoftBodyFace*> faceInRegL;

	std::vector<GFPhysSoftBodyFace*> faceInRegR;

	for (size_t c = 0; c < clampOrgan.m_ClampedFaces.size(); c++)
	{
		if (clampOrgan.m_ClampedFaces[c].m_Part == 0)
			faceInRegR.push_back(clampOrgan.m_ClampedFaces[c].m_PhysFace);

		else if (clampOrgan.m_ClampedFaces[c].m_Part == 1)
			faceInRegL.push_back(clampOrgan.m_ClampedFaces[c].m_PhysFace);
	}

	//m_ClampReg[1] left region
	//m_ClampReg[0] right region
	GFPhysVector3 clampDirection = (m_ClampReg[1].m_ClampNormalWorld - m_ClampReg[0].m_ClampNormalWorld).Normalized();
	clampOrgan.m_ClampDirInMaterialSpace = TransFormWorldClipDirToLocalDir(clampDirection, faceInRegL, faceInRegR);

	//correct face to be inside the clamp plane
	GFPhysVector3 tanPart[2][2];
	GetClampRegTangent(0, tanPart[0][0], tanPart[0][1]);
	GetClampRegTangent(1, tanPart[1][0], tanPart[1][1]);

	int   minValidFaceIndex = -1;
	float minValidDist = FLT_MAX;
	int   minValidEdgeIndex = -1;

	int   minInvalidFace = -1;
	float minInvalidDist = FLT_MAX;
	int   minInvalidEdge = -1;
	for (size_t f = 0; f < clampOrgan.m_ClampedFaces.size(); f++)
	{
		 SoftBodyFaceClamped & clampFace = clampOrgan.m_ClampedFaces[f];

		 const MisCTool_PluginClamp::ToolClampRegion & BelongRegion = m_ClampReg[clampFace.m_Part];

		 for (int n = 0; n < 3; n++)
		 {
			  GFPhysVector3 nodePos = clampFace.m_PhysFace->m_Nodes[n]->m_CurrPosition;
			  float CoordN0  = (nodePos - m_ClampReg[0].m_OriginWorld).Dot(ClampRegNormals[0]);
			  float CoordN1  = (nodePos - m_ClampReg[1].m_OriginWorld).Dot(ClampRegNormals[1]);
			
			  float tan0 = (nodePos - m_ClampReg[clampFace.m_Part].m_OriginWorld).Dot(tanPart[clampFace.m_Part][0]);
			  float tan1 = (nodePos - m_ClampReg[clampFace.m_Part].m_OriginWorld).Dot(tanPart[clampFace.m_Part][1]);
			  clampFace.m_NodeTanDist[n] = Ogre::Vector2(tan0, tan1);
			  clampFace.m_NodeNormDist[n][0] = CoordN0;
			  clampFace.m_NodeNormDist[n][1] = CoordN1;

			  if (CoordN0 > -0.08f && CoordN1  > -0.08f)
			  {
				  clampFace.m_NormalStiff[n] = 1.0f;
				  clampFace.m_TanStiff[n] = 0.3f;
			  }
			  else
			  {
				  clampFace.m_NormalStiff[n] = 0.0f;
				  clampFace.m_TanStiff[n] = 0.0f;
			  }
		 }
		 GFPhysVector3 fNode[3];
		 fNode[0] = GFPhysVector3(clampFace.m_NodeTanDist[0].x, clampFace.m_NodeTanDist[0].y, 0);
		 fNode[1] = GFPhysVector3(clampFace.m_NodeTanDist[1].x, clampFace.m_NodeTanDist[1].y, 0);
		 fNode[2] = GFPhysVector3(clampFace.m_NodeTanDist[2].x, clampFace.m_NodeTanDist[2].y, 0);
		
		 //check 3 valid edge of this face
		 GFPhysSoftBodyFace * physFace = clampFace.m_PhysFace;

		 for (int e = 0; e < 3; e++)
		 {
			  if (clampFace.m_NormalStiff[e] > 0 && clampFace.m_NormalStiff[(e + 1) % 3] > 0)//valid check closet distance
			  {
				  GFPhysVector3 cp = CloasetPtToSegment(GFPhysVector3(0, 0, 0), fNode[e], fNode[(e + 1) % 3]);
				  float dist = cp.Length();
				  if (dist < minValidDist)
				  {
					  minValidDist = dist;
					  minValidFaceIndex = f;
					  minValidEdgeIndex = e;
				  }
			  }
			  else//invalid edge we save min 
			  {
				  float avgDist = fabsf(clampFace.m_NodeNormDist[e][0]) + fabsf(clampFace.m_NodeNormDist[e][1])
					            + fabsf(clampFace.m_NodeNormDist[(e + 1) % 3][0]) + fabsf(clampFace.m_NodeNormDist[(e + 1) % 3][1]);

				  if (avgDist < minInvalidDist)
				  {
					  minInvalidDist = avgDist;
					  minInvalidFace = f;
					  minInvalidEdge = e; 
				  }
			  }
		 }
	}

	SoftBodyFaceClamped * minClampFace = 0;
	
	int minEdgeId = -1;

	if (minValidEdgeIndex >= 0 && minValidFaceIndex >= 0)
	{
		minClampFace = &clampOrgan.m_ClampedFaces[minValidFaceIndex];
		minEdgeId = minValidEdgeIndex;
		clampOrgan.m_MainClampFaceIndex = minValidFaceIndex;
	}
	else if (minInvalidEdge >= 0 && minInvalidFace >= 0)//we don't have an valid edge use the min dist invalid one
	{
		minClampFace = &clampOrgan.m_ClampedFaces[minInvalidFace];
		minEdgeId = minInvalidEdge;
		clampOrgan.m_MainClampFaceIndex = minInvalidFace;
	}

	if (minClampFace)
	{
		int e = minEdgeId;
		int t = (minEdgeId + 1) % 3;
		GFPhysVector3 eNode[2];
		eNode[0] = GFPhysVector3(minClampFace->m_NodeTanDist[e].x, minClampFace->m_NodeTanDist[e].y, 0);
		eNode[1] = GFPhysVector3(minClampFace->m_NodeTanDist[t].x, minClampFace->m_NodeTanDist[t].y, 0);

		GFPhysVector3 cp = -CloasetPtToSegment(GFPhysVector3(0, 0, 0), eNode[0], eNode[1]).Normalized();
		Ogre::Vector2 moveDir(cp.m_x , cp.m_y);

		minClampFace->m_TanStiff[e] = minClampFace->m_TanStiff[t] = 0.99f;
		minClampFace->m_NormalStiff[e] = minClampFace->m_NormalStiff[t] = 1.0f;//ensure normal stiffness = 1.0f this is important

		if (clampOrgan.m_ClampMode == 1 || minValidFaceIndex < 0)
		{
			minClampFace->m_NodeTanDist[e] += moveDir*0.15f;
			minClampFace->m_NodeTanDist[t] += moveDir*0.15f;
		}
	}
	else
	{
		clampOrgan.m_ClampedFaces.clear();
	}

	//process node belong clamped faces
	for (int f = 0; f < (int)clampOrgan.m_ClampedFaces.size(); f++)
	{
		 GFPhysSoftBodyFace * face = clampOrgan.m_ClampedFaces[f].m_PhysFace;

		 face->m_RSCollisionMask &= (~toolcat);

		 int RegionClampFace = clampOrgan.m_ClampedFaces[f].m_Part;

		 for (int n = 0; n < 3; n++)
		 {
			GFPhysSoftBodyNode * clampedNode = face->m_Nodes[n];

			//exclude static node
			if (clampedNode->m_InvM > FLT_EPSILON)
			{
				std::map<GFPhysSoftBodyNode*, ClampedNodeData>::iterator nitor = clampOrgan.m_ClampedNodes.find(clampedNode);
				if (nitor == clampOrgan.m_ClampedNodes.end())//not exist in node map
				{
					MisCTool_PluginClamp::ClampedNodeData clampNodeData;
					clampNodeData.m_RegionIndex = RegionClampFace;
					clampNodeData.m_NodesInClamp = clampedNode;
					clampNodeData.m_FaceRefCount = 1;

					//this node is already clamped by other tool yet
					if ((clampedNode->m_StateFlag & EMMP_ClampByTool) != 0)
						clampNodeData.m_IsInClamp = false;
					else
						clampNodeData.m_IsInClamp = true;

					if (clampNodeData.m_IsInClamp == true)
					{
						clampedNode->m_StateFlag |= EMMP_ClampByTool;//bacon omit
						clampedNode->m_StateFlag |= GPSESF_ATTACHED;
						//clampedNode->m_RSCollisionMask &= (~toolcat);
					}
					clampOrgan.m_ClampedNodes.insert(std::make_pair(clampedNode, clampNodeData));
				}
				else//increase reference count
				{
					nitor->second.m_FaceRefCount++;
				}
			}
		 }
	}
	//
	/*
	clampOrgan.m_EdgeClampedNodes.clear();
	GFPhysSoftBodyEdge * edge = clampOrgan.m_organ->m_physbody->GetEdgeList();
	while (edge)
	{
		if (IsEdgeBeClamped(edge))
		{
			for (int n = 0; n < 2; n++)
			{
				GFPhysSoftBodyNode * node = edge->m_Nodes[n];

				if (clampOrgan.m_ClampedNodes.find(node) == clampOrgan.m_ClampedNodes.end()
				 && node->m_InvM > FLT_EPSILON)
				{
					ClampedNodeData cdata;
					cdata.m_Tan0 = (node->m_CurrPosition - m_ClampReg[0].m_OriginWorld).Dot(tanPart[0][0]);
					cdata.m_Tan1 = (node->m_CurrPosition - m_ClampReg[0].m_OriginWorld).Dot(tanPart[0][1]);
					clampOrgan.m_EdgeClampedNodes.insert(std::make_pair(node, cdata));
				}
			}
		}
		edge = edge->m_Next;
	}*/
	
}
//========================================================================================
void MisCTool_PluginClamp::PhysicsSimulationEnd(int currStep , int TotalStep , float dt)
{
	uint32 toolcat = (m_ToolObject->GetToolSide() == ITool::TSD_LEFT ? MMRC_LeftTool : MMRC_RightTool);

	//update clamp region normal , axis , region vertex to world space
	//m_ClampReg[0].UpdateToWorldSpace();
	//m_ClampReg[1].UpdateToWorldSpace();
	
	//clamp region's world aabb
	//UpdateClampRegionWorldAABB();

	UpdateMoveInfo(dt);

	float shaftTool = m_ToolObject->GetShaftAside();
	
	if(m_ClampStateStage == MisCTool_PluginClamp::CSS_Clamped)
	{
		bool prepareUnClamp = false;
		if(shaftTool > m_ReleaseClampShaft)//if we need release drag points
		{
		    prepareUnClamp = true;
		}
		//else if(m_ClampedOrgans.size() > 0 && CalculateValidClampedFacesNum(dt) <= 0)//check whether clamped face all slip out clamp region
		//{
			//prepareUnClamp= true;
		//}
		if(m_ClampedOrgans.size() > 0)
		{
			for(int i = 0 ; i < 2 ; i++)
			{
				float moveDist = (m_ClampReg[i].m_OriginWorld - m_ClampReg[i].m_OriginWorldPrev).Length();
				if(moveDist > 10.0f*dt)
				   prepareUnClamp = false;
			}
		}
		if(prepareUnClamp && m_ClampedOrgans.size() > 0)//prepare release
		{
		   m_ClampStateStage  = MisCTool_PluginClamp::CSS_InRelease;
		   m_TimeSinceRelease = 0;
		}
		else if(m_ClampedOrgans.size() > 0)//update min shaft angle
		{   //
			for(int r = 0 ; r < 2 ; r++)
			{
				MisCTool_PluginClamp::ToolClampRegion & clampRegion = m_ClampReg[r];
				
				GFPhysVector3 RegNormal = GetClampRegNormal(r);
				
				int FaceContactNum = 0;

				for (int c = 0; c < (int)m_ClampedOrgans.size(); c++)
				{
					OrganBeClamped * clampOrgan = m_ClampedOrgans[c];
					
					for (int f = 0; f < (int)clampOrgan->m_ClampedFaces.size(); f++)
					{
						if (clampOrgan->m_ClampedFaces[f].m_Part == r)
						{
							GFPhysSoftBodyFace * face = clampOrgan->m_ClampedFaces[f].m_PhysFace;
							for (int n = 0; n < 3; n++)
							{
								GFPhysSoftBodyNode * FaceNode = face->m_Nodes[n];

								float CoordN = (FaceNode->m_CurrPosition - clampRegion.m_OriginWorld).Dot(RegNormal);

								if (CoordN < 0)// && face->m_FaceNormal.Dot(RegNormal) < -0.88f)
								{
									FaceContactNum++;
								}
							}
						}
					}
				}
				if(FaceContactNum == 0)
				{
					//float minshaft = m_ToolObject->GetMinShaftAside()*0.8f;
					//minshaft = GPClamped(minshaft , m_minShaftRangeLowerValue , m_minShaftRangeUpperValue);
					//m_ToolObject->SetMinShaftAside(minshaft);
					//break;
				}
			}
		}
	}
	else
	{
		if (m_IsOpenAngleEnough)
		{
			if (shaftTool <= m_clampCheckAngleStart)//to do! 需要根据策略选择被抓的物体
			{
				if (!CheckOrganBeClampedV2())
				{
					CheckRopeBeClamped();
					CheckRopeBeClampedV2();
				}
			}

			//check  tube
			if (shaftTool <= 4.5f)//temp give 4.5 shaft
			{
				std::vector<ACTubeShapeObject*>  tubes;
				MisNewTraining * train = (MisNewTraining*)m_ToolObject->GetOwnerTraining();
				train->GetAllTubes(tubes);
				bool hasClampedSomeTube = m_TubeClampProcessor->CheckTubeBeClamped(tubes);
				if (hasClampedSomeTube)
				{
					m_ToolObject->SetMinShaftAside(4.0f);
					m_IsOpenAngleEnough = false;
				}
			}

			//if (shaftTool <= 3)
			//{
			//	m_IsOpenAngleEnough = false;
			//}
		}
	}
	CheckVeinObjBeClamped();

	int needReleaseNum = 0;
	
	for (int c = 0; c < (int)m_ClampedOrgans.size(); c++)
	{
		OrganBeClamped * clampOrgan = m_ClampedOrgans[c];
		int faceSlipped = 0;
		for (int f = 0; f < clampOrgan->m_ClampedFaces.size(); f++)
		{
			SoftBodyFaceClamped & cface = clampOrgan->m_ClampedFaces[f];
			if (cface.m_FrictForce.length() > 0.3f)
			{
				faceSlipped++;
			}
		}

		if ((float)faceSlipped / (float)clampOrgan->m_ClampedFaces.size() > 0.5f)
			needReleaseNum++;
		/*Ogre::Vector3 frictForce(0, 0, 0);
		for (int c = 0; c < (int)clampOrgan->m_ClampedFaces.size(); c++)
		{
			frictForce += clampOrgan->m_ClampedFaces[c].m_FrictForce;
		}
		if (clampOrgan->m_ClampMode == 0 &&
			clampOrgan->m_ClampedNodes.size() > 0 &&
			frictForce.length() / clampOrgan->m_ClampedNodes.size() > 10000.4f)
		{
			needReleaseNum++;
		}
		else if (clampOrgan->m_ClampMode == 1 && (frictForce.length() / 3.0f > 10000.4f))
		{
			needReleaseNum++;
		}*/
	}

	if (needReleaseNum >= (int)m_ClampedOrgans.size() && needReleaseNum > 0)
	{
	    /*for (int c = 0; c < (int)m_ClampedOrgans.size(); c++)
		{
			OrganBeClamped * clampOrgan = m_ClampedOrgans[c];
			
			if (clampOrgan->m_ClampedFaces.size() > 0)
			{
			    Ogre::Vector2 TexCenter(0, 0);
				for (int f = 0; f < (int)clampOrgan->m_ClampedFaces.size(); f++)
			    {
					GFPhysSoftBodyFace * physFace = clampOrgan->m_ClampedFaces[f].m_PhysFace;
			       TexCenter.x += (physFace->m_TexCoordU[0] + physFace->m_TexCoordU[1] + physFace->m_TexCoordU[2])*0.33333f;
			       TexCenter.y += (physFace->m_TexCoordV[0] + physFace->m_TexCoordV[1] + physFace->m_TexCoordV[2])*0.33333f;
			    }
				TexCenter /= clampOrgan->m_ClampedFaces.size();

			    Ogre::TexturePtr TexBleed = Ogre::TextureManager::getSingleton().load("partcl_bleed_new.dds", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
				for (int c = 0; c < (int)m_ClampedOrgans.size(); c++)
			    {
			        m_ClampedOrgans[c]->m_organ->ApplyEffect_Bleeding(TexCenter, 0, 1.0, TexBleed);
			    }
			}
		}*/
		ReleaseClampedOrgans();
	}

	if (false && m_pClampedRope && m_pClampedRope->m_move)
	{
		ReleaseClampedRope();
		m_pClampedRope->m_move = false;
	}
}
//=========================================================================================
void MisCTool_PluginClamp::OneFrameUpdateStarted(float timeelapsed)
{
	for (int c = 0; c < (int)m_ClampedOrgans.size(); c++)
	{
		OrganBeClamped * clampOrgan = m_ClampedOrgans[c];
		for (size_t f = 0; f < clampOrgan->m_ClampedFaces.size(); f++)
		{
			clampOrgan->m_ClampedFaces[f].m_DragForce = Ogre::Vector3(0, 0, 0);
			clampOrgan->m_ClampedFaces[f].m_FrictForce = Ogre::Vector3(0, 0, 0);
		}
	}

	float shaftAside = m_ToolObject->GetShaftAside();
    
    if (shaftAside > m_clampCheckAngleStart && m_IsThreadClamp)
    {
		if (m_pClampedRope)
		{
			ReleaseClampedRope();
		}
		if (m_pClampedRopeV2)
		{
			ReleaseClampedRopeV2();
		}
    }

    if (shaftAside > 2.0f && m_InVeinClamp)
    {
        ReleaseClampedVeinConn();
    }

	if (shaftAside > 4.5f)
	{
		m_TubeClampProcessor->ReleaseClampedSegment();

		if (m_ClampedOrgans.size() == 0)//also none organ clamped
		    m_ToolObject->SetMinShaftAside(0.0f);
	}

    if (shaftAside > m_clampCheckAngleStart && m_ClampStateStage == MisCTool_PluginClamp::CSS_Released)
	{
		m_IsOpenAngleEnough = true;
	}
	else if(m_ClampStateStage == MisCTool_PluginClamp::CSS_InRelease)
	{
		m_TimeSinceRelease += timeelapsed;
		if(m_TimeSinceRelease > m_MaxReleasingTime)// || shaftAside > m_clampCheckAngleStart + 2.0f)
		{
 		   ReleaseClampedOrgans();
		}
	}
	
	
	m_IgnorCoolDownTime += timeelapsed;
	if(m_IgnorCoolDownTime > 0.5f)
	{
       m_OrgansIgnored.clear();
	}
}
//========================================================================================
void MisCTool_PluginClamp::OneFrameUpdateEnded()
{
	if(m_ShowClampRegion) 
		DrawClampAxisAndRegion();
	//m_paintingtool.Update(0);
}
//========================================================================================
const MisCTool_PluginClamp::ToolClampRegion & MisCTool_PluginClamp::GetClampRegion(int index)
{
	return m_ClampReg[index];
}
//============================================================================================================
const MisCTool_PluginClamp::ToolClampRegion * MisCTool_PluginClamp::GetClampRegions() const
{
	return m_ClampReg;
}
//============================================================================================================
GFPhysVector3 MisCTool_PluginClamp::GetClampRegNormal(int RegIndex)
{
	return m_ClampReg[RegIndex].m_ClampNormalWorld;
}
//============================================================================================================
void MisCTool_PluginClamp::GetClampRegTangent(int RegIndex, GFPhysVector3 & tan0, GFPhysVector3 & tan1)
{
	tan0 = m_ClampReg[RegIndex].m_Axis0World;
	tan1 = m_ClampReg[RegIndex].m_Axis1World;
}
//============================================================================================================
/*void MisCTool_PluginClamp::UpdateClampRegionWorldAABB()
{
	m_WorldClampRegMin = GFPhysVector3(GP_INFINITY  , GP_INFINITY  , GP_INFINITY);
	
	m_WorldClampRegMax = GFPhysVector3(-GP_INFINITY , -GP_INFINITY , -GP_INFINITY);

	for(int i = 0 ; i < 2 ; i++)
	{	
		ToolClampRegion & regClamp = m_ClampReg[i];
		
		if(regClamp.m_RegSide != ClampReg_UnKnow)//valid
		{
			for(size_t c = 0 ; c < regClamp.m_WorldTriVerts.size() ; c++)
			{
				m_WorldClampRegMin.SetMin(regClamp.m_WorldTriVerts[c]);
				m_WorldClampRegMax.SetMax(regClamp.m_WorldTriVerts[c]);
			}
		}
	}
}*/
//============================================================================================================
class ConvexRegionCollideResult : public GFPhysDCDInterface::Result
{
public:
	ConvexRegionCollideResult()
	{
		m_HasPoint = false;
	}
	/*overridden*/
	void SetShapeIdentifiers(int partId0,int index0,int partId1,int index1)
	{}
	/*overridden*/
	void AddContactPoint(const GFPhysVector3& normalOnBInWorld,const GFPhysVector3& pointInWorld,Real depth)
	{
		m_Depth = depth;
		m_NormalOnB = normalOnBInWorld;
		m_PointOnB = pointInWorld;
		m_HasPoint = true;
	}
	bool m_HasPoint;
	float m_Depth;
	GFPhysVector3 m_NormalOnB;
	GFPhysVector3 m_PointOnB;
};
//==========================================================================================
void MisCTool_PluginClamp::CollectTetrasInClampRegion(OrganBeClamped & organclamped)//to do need optimize
{
	if (m_IsRect == false)//to do support triangle collect
		return;//
	
	organclamped.m_TetrasInClampReg.clear();

	GFPhysSoftBody * physbody = organclamped.m_organ->m_physbody;

	//clamp region vertx
	GFPhysVector3 RegConvexVert[8];
	for(int r = 0 ; r < 2 ; r++)
	{
		ToolClampRegion & clampReg = m_ClampReg[r];
		
		RegConvexVert[4 * r + 0] = clampReg.m_OriginWorld + clampReg.m_Axis0World*clampReg.m_axis0Min + clampReg.m_Axis1World*clampReg.m_axis1Min;
		RegConvexVert[4 * r + 1] = clampReg.m_OriginWorld + clampReg.m_Axis0World*clampReg.m_axis0Max + clampReg.m_Axis1World*clampReg.m_axis1Min;
		RegConvexVert[4 * r + 2] = clampReg.m_OriginWorld + clampReg.m_Axis0World*clampReg.m_axis0Max + clampReg.m_Axis1World*clampReg.m_axis1Max;
		RegConvexVert[4 * r + 3] = clampReg.m_OriginWorld + clampReg.m_Axis0World*clampReg.m_axis0Min + clampReg.m_Axis1World*clampReg.m_axis1Max;
	}
	
	//for debug
	


	GFPhysTransform transReg;
	GFPhysTransform transTetra;
	transReg.SetIdentity();
	transTetra.SetIdentity();
	
	//get region vertex aabb for quick rough exclude
	GFPhysVector3 regAabbMin; 
	GFPhysVector3 regAabbMax;
	regAabbMin = regAabbMax = RegConvexVert[0];
	for(int v = 1 ; v < 8 ; v++)
	{	
		regAabbMin.SetMin(RegConvexVert[v]);
		regAabbMax.SetMax(RegConvexVert[v]);
	}
	
	//GFPhysSoftBodyTetrahedron * tetraSoft = physbody->GetTetrahedronList();
	//while(tetraSoft)
	for(size_t th = 0 ; th < physbody->GetNumTetrahedron() ; th++)
	{
		GFPhysSoftBodyTetrahedron * tetraSoft = physbody->GetTetrahedronAtIndex(th);

		GFPhysVector3 tetraNodePos[4];
		tetraNodePos[0] = tetraSoft->m_TetraNodes[0]->m_CurrPosition;
		tetraNodePos[1] = tetraSoft->m_TetraNodes[1]->m_CurrPosition;
		tetraNodePos[2] = tetraSoft->m_TetraNodes[2]->m_CurrPosition;
		tetraNodePos[3] = tetraSoft->m_TetraNodes[3]->m_CurrPosition;

		GFPhysVector3 tetraBoxMin = tetraNodePos[0];
		GFPhysVector3 tetraBoxMax = tetraBoxMin;

		tetraBoxMin.SetMin(tetraNodePos[1]);
		tetraBoxMin.SetMin(tetraNodePos[2]);
		tetraBoxMin.SetMin(tetraNodePos[3]);

		tetraBoxMax.SetMax(tetraNodePos[1]);
		tetraBoxMax.SetMax(tetraNodePos[2]);
		tetraBoxMax.SetMax(tetraNodePos[3]);

		//whether region and tetrahedron intersect
		if(TestAabbAgainstAabb2(regAabbMin, regAabbMax,tetraBoxMin, tetraBoxMax))//further check
		{
			GFPhysVector3 closetPointReg,closetPointTri;
			float closetDist = GetConvexsClosetPoint(RegConvexVert , 8 , 0.01 ,
				                                     tetraNodePos, 4, 0.01,
													 transReg , transTetra ,
													 closetPointReg , closetPointTri , 
													 0.1f);
			if(closetDist < 0)
			{
				organclamped.m_TetrasInClampReg.push_back(tetraSoft);
			}
		}
	}
	//PhysicsWrapper::GetSingleTon().m_dynamicsWorld->ChangeVolumeElementSor(organclamped.m_TetrasInClampReg, 0.0f);
	
}
//========================================================================================================
void MisCTool_PluginClamp::GetFacesBeClamped(GFPhysVectorObj<GFPhysSoftBodyFace*> & resultFaces , MisMedicOrgan_Ordinary * organ)
{
	for (int c = 0; c < (int)m_ClampedOrgans.size(); c++)
	{
		if (m_ClampedOrgans[c]->m_organ == organ)
		{
			for (size_t f = 0; f < m_ClampedOrgans[c]->m_ClampedFaces.size(); f++)
			{
				if (m_ClampedOrgans[c]->m_ClampedFaces[f].m_Organ == organ)
					resultFaces.push_back(m_ClampedOrgans[c]->m_ClampedFaces[f].m_PhysFace);
			}
		}
	}
	
}
//===============================================================================================
void MisCTool_PluginClamp::OnSoftBodyNodesBeDeleted(GFPhysSoftBody * sb , const GFPhysAlignedVectorObj<GFPhysSoftBodyNode*> & nodes)
{
	for (int c = 0; c < (int)m_ClampedOrgans.size(); c++)
	{
		if (m_ClampedOrgans[c]->m_organ->m_physbody == sb)
		{
			for (size_t n = 0; n < nodes.size(); n++)
			{
				m_ClampedOrgans[c]->m_ClampedNodes.erase(nodes[n]);
				m_ClampedOrgans[c]->m_EdgeClampedNodes.erase(nodes[n]);
			}
		}
	}
}
//===============================================================================================
void MisCTool_PluginClamp::OnSoftBodyFaceBeDeleted(GFPhysSoftBody * sb, GFPhysSoftBodyFace *face)
{
	uint32 toolcat = (m_ToolObject->GetToolSide() == ITool::TSD_LEFT ? MMRC_LeftTool : MMRC_RightTool);

	for (int c = 0; c < (int)m_ClampedOrgans.size(); c++)
	{
		if (m_ClampedOrgans[c]->m_organ->m_physbody == sb)
		{
			OrganBeClamped * DstOrgan = m_ClampedOrgans[c];

			for (int f = 0; f < (int)DstOrgan->m_ClampedFaces.size(); f++)
			{
				if (DstOrgan->m_ClampedFaces[f].m_PhysFace == face)
				{
					for (int n = 0; n < 3; n++)
					{
						GFPhysSoftBodyNode * clampNode = face->m_Nodes[n];

						std::map<GFPhysSoftBodyNode*, ClampedNodeData>::iterator nitor = DstOrgan->m_ClampedNodes.find(clampNode);

						if (nitor != DstOrgan->m_ClampedNodes.end())
						{
							ClampedNodeData & clampNodeData = nitor->second;

							clampNodeData.m_FaceRefCount--;

							if (clampNodeData.m_FaceRefCount <= 0 && clampNodeData.m_IsInClamp)
							{
								clampNode->m_StateFlag &= (~EMMP_ClampByTool);
								clampNode->m_StateFlag &= (~GPSESF_ATTACHED);
								//clampNode->m_RSCollisionMask  |= toolcat;
								clampNodeData.m_IsInClamp = false;
							}
						}
					}
					DstOrgan->m_ClampedFaces.erase(DstOrgan->m_ClampedFaces.begin() + f);
					break;
				}
			}
		}
	}
}
//===============================================================================================
void MisCTool_PluginClamp::OnSoftBodyFaceBeAdded(GFPhysSoftBody * sb, GFPhysSoftBodyFace *face)
{
	
}
//===============================================================================================

#define RigionBorder 0.09f

void MisCTool_PluginClamp::CollectSubFaceInClampRegionUV(GFPhysSoftBodyFace * face, int nodeIndex, float& U, float& V, float regionScale)
{
	GFPhysSoftBodyNode* nod0 = face->m_Nodes[nodeIndex];
	GFPhysVector3 pos = nod0->m_CurrPosition;
	GFPhysVector3 centDir = centerWorld - pos;
	U = centDir.Dot(axis0World);
	V = centDir.Dot(axis1World);
	Ogre::String type = m_ToolObject->GetType();
	U = U / (regionScale*halfExt0) * 0.5f + 0.5f;
	V = V / (regionScale*halfExt1) * 0.5f + 0.5f;
}

void MisCTool_PluginClamp::CollectPos()
{
	ToolClampRegion& clampReg_0 = m_ClampReg[0];

	ToolClampRegion& clampReg_1 = m_ClampReg[1];

	
	axis0World = clampReg_0.m_Axis0World;
	axis1World = clampReg_0.m_Axis1World;// +clampReg_1.m_Axis1World * 0.5f);
	if(m_IsRect)
	{
		halfExt0 = clampReg_0.m_HalfExt0;
		halfExt1 = clampReg_0.m_HalfExt1;
		centerWorld = (clampReg_0.m_OriginWorld + clampReg_1.m_OriginWorld) * 0.5f;
	}
	else 
	{
		//halfExt0 = FLT_MAX;
		//halfExt1 = FLT_MAX;
		//for(int i = 0; i < 2;i++)
		//{
			//float axis0 = max(abs(m_ClampReg[i].m_axis0Min),abs(m_ClampReg[i].m_axis0Max));
			//float axis1 = max(abs(m_ClampReg[i].m_axis1Min),abs(m_ClampReg[i].m_axis1Max));
			//if(axis0 < halfExt0)
			//	halfExt0 = axis0;
			//if(axis1 < halfExt1)
			//	halfExt1 = axis1;
		//}

	    float cx = (m_ClampReg[0].m_axis0Min + m_ClampReg[0].m_axis0Max)*0.5f;
		float cy = (m_ClampReg[0].m_axis1Min + m_ClampReg[0].m_axis1Max)*0.5f;

		centerWorld = clampReg_0.m_OriginWorld + axis0World * cx + axis1World * cy;
		halfExt0 = (m_ClampReg[0].m_axis0Max - m_ClampReg[0].m_axis0Min)*0.5f;
		halfExt1 = (m_ClampReg[0].m_axis1Max - m_ClampReg[0].m_axis1Min)*0.5f;
	}
}

void MisCTool_PluginClamp::UpdateMoveInfo(float dt)
{
	//假设已调用UpdateToWorldSpace
	ToolClampRegion& clampReg0 = m_ClampReg[0];
//	clampReg0.UpdateToWorldSpace();

	ToolClampRegion& clampReg1 = m_ClampReg[1];
//	clampReg1.UpdateToWorldSpace();
	
	GFPhysVector3 clampPointTo = clampReg0.m_Axis1World + clampReg1.m_Axis1World;
	clampPointTo.Normalized();

	GFPhysVector3 currPos = clampReg0.m_OriginWorld + clampReg1.m_OriginWorld;
	currPos *= 0.5;

	m_MoveIncrementInPointTo = (currPos - m_LastClampCenter).Dot(m_LastClampPointTo);

	m_ClampMoveSpeed = currPos.Distance(m_LastClampCenter) / dt;

	if((m_ClampStateStage == MisCTool_PluginClamp::CSS_Clamped) && m_ClampedOrgans.size() > 0)
	{
		for (int c = 0; c < (int)m_ClampedOrgans.size(); c++)
		{
			MisMedicOrgan_Ordinary * DstOrgan = m_ClampedOrgans[c]->m_organ;
			
			DstOrgan->m_FaceMoveIncrementInClamp = m_MoveIncrementInPointTo;

			DstOrgan->m_FaceMoveSpeedInClamp = m_ClampMoveSpeed;

			m_MoveDistAfterClamped = currPos.Distance(m_ClampCenterAtClamped);
		}
	}
	
	m_LastClampCenter = currPos;
	m_LastClampPointTo = clampPointTo;

    if (m_CanClampLargeFace)
        CollectClampCandidateInfo();

}

void MisCTool_PluginClamp::CollectClampCandidateInfo()
{
    MisNewTraining * currtrain = dynamic_cast<MisNewTraining *>(m_ToolObject->GetOwnerTraining());

	int numleftcollide = 0;
	
	int numrigthcollide = 0;
    
	for (size_t f = 0; f < m_ToolObject->m_ToolColliedFaces.size(); f++)
    {
        const ToolCollidedFace & datacd = m_ToolObject->m_ToolColliedFaces[f];
        /*GFPhysSoftBody * softbody = (GFPhysSoftBody*)datacd.m_collideSoft;
        MisMedicOrgan_Ordinary * pCurrOrgan = dynamic_cast<MisMedicOrgan_Ordinary*>(currtrain->GetOrgan(softbody));

        if (m_OrgansCollidedLeftPart.find(pCurrOrgan) == m_OrgansCollidedLeftPart.end() || m_OrgansCollidedRightPart.find(pCurrOrgan) == m_OrgansCollidedRightPart.end())
        {
            m_MaxPerSistentShaftAside = -100.0f;
        }*/ 
		int toolside = m_ToolObject->GetRigidBodyPart(datacd.m_collideRigid);
		
		if (toolside == 0)
			numleftcollide++;
		
		else if (toolside == 1)
			numrigthcollide++; 
    }

	if (numleftcollide > 0 && numrigthcollide > 0)
	{
		Real Shaft = m_ToolObject->GetShaftAside();
		if (Shaft > m_MaxPerSistentShaftAside)
		{
			m_MaxPerSistentShaftAside = Shaft;
		}
	}   
	else
	{
		m_MaxPerSistentShaftAside = -100.0f;
	}
}

bool MisCTool_PluginClamp::IsCloseToClampFace(GFPhysSoftBodyNode * node, Real threshold)
{
#if 0
    GFPhysVector3 pos = centerWorld;

    GFPhysVector3 normal = m_ClampReg[0].m_ClampNormalWorld;

    Real dis;
    if (GPDistanceFromPointToSurface(node->m_CurrPosition, pos, normal, dis))
    {
        if (dis < threshold)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
#else
    
    return TestNodeContactClampReg(node);
#endif

}

void MisCTool_PluginClamp::GetFaceVertices2DPos(const GFPhysSoftBodyFace& face , Ogre::Vector2 faceVertices[3])
{
	for(int p  = 0 ; p < 3; p++)
	{
		GFPhysVector3 dir = face.m_Nodes[p]->m_CurrPosition - centerWorld;
		faceVertices[p].x = dir.Dot(axis0World);
		faceVertices[p].y = dir.Dot(axis1World);
	}
}

void MisCTool_PluginClamp::GetClampCoordSys(GFPhysVector3 &origin,GFPhysVector3 &right,GFPhysVector3 &up)
{
	origin = centerWorld;
	right = axis0World;
	up = axis1World;
}


void MisCTool_PluginClamp::DrawClampAxisAndRegion()
{
	if(m_manual == 0)
	{
		m_manual = MXOgreWrapper::Get()->GetDefaultSceneManger()->createManualObject();
		m_manual->setDynamic(true);
		m_manual->begin("clampdebug",Ogre::RenderOperation::OT_LINE_LIST);
		m_manual->position(0,0,0);
		m_manual->colour(0,1,1);
		m_manual->position(1,1,1);
		m_manual->colour(1,1,1);
		m_manual->end();
		MXOgreWrapper::Get()->GetDefaultSceneManger()->getRootSceneNode()->attachObject(m_manual);
		//clamp region
		m_manual->begin("clampdebug");
		m_manual->position(0,0,0);
		m_manual->colour(0,0.5,0.5);
		m_manual->position(1,1,0);
		m_manual->colour(0,0.5,0.5);
		m_manual->position(1,0,0);
		m_manual->colour(0,0.5,0.5);
		m_manual->end();
		//bounding
		m_manual->begin("clampdebug",Ogre::RenderOperation::OT_LINE_STRIP);
		m_manual->position(0,0,0);
		m_manual->colour(0,0.5,0.5);
		m_manual->position(0,0,0);
		m_manual->colour(0,0.5,0.5);
		m_manual->position(0,0,0);
		m_manual->colour(0,0.5,0.5);
		m_manual->end(); 
	}
	else
	{
		//axis
		m_manual->beginUpdate(0);
		for(int r = 0 ; r < 2 ; r++)
		{
			Ogre::Vector3 center = GPVec3ToOgre(m_ClampReg[r].m_OriginWorld);
			Ogre::Vector3 axis0 = GPVec3ToOgre(m_ClampReg[r].m_Axis0World);
			Ogre::Vector3 axis1 = GPVec3ToOgre(m_ClampReg[r].m_Axis1World);
			Ogre::Vector3 normal = GPVec3ToOgre(GetClampRegNormal(r));
			/*m_manual->position(center);
			m_manual->colour(1,0,0);
			m_manual->position(center + axis0 * 2);
			m_manual->colour(1,0,0);
			m_manual->position(center);
			m_manual->colour(0,1,0);
			m_manual->position(center + axis1 * 2);
			m_manual->colour(0,1,0);
			m_manual->position(center);
			m_manual->colour(0,0,1);
			m_manual->position(center + normal * 2);
			m_manual->colour(1,1,1);//bacon test color(1,1,1)*/

			m_manual->position(GPVec3ToOgre(m_ToolObject->m_CutBladeLeft.m_LinePointsWorld[0]));
			m_manual->colour(1,0,0);
			m_manual->position(GPVec3ToOgre(m_ToolObject->m_CutBladeLeft.m_LinePointsWorld[1]));
			m_manual->colour(1,0,0);

			m_manual->position(GPVec3ToOgre(m_ToolObject->m_CutBladeRight.m_LinePointsWorld[0]));
			m_manual->colour(0,1,0);
			m_manual->position(GPVec3ToOgre(m_ToolObject->m_CutBladeRight.m_LinePointsWorld[1]));
			m_manual->colour(0,1,0);
		}
		m_manual->end();
		//clamp region
		m_manual->beginUpdate(1);
		for(int r = 0; r < 2; r++)
		{
			Ogre::Vector3 center = GPVec3ToOgre(m_ClampReg[r].m_OriginWorld);
			Ogre::Vector3 axis0 = GPVec3ToOgre(m_ClampReg[r].m_Axis0World);
			Ogre::Vector3 axis1 = GPVec3ToOgre(m_ClampReg[r].m_Axis1World);
			Ogre::Vector3 normal = GPVec3ToOgre(GetClampRegNormal(r));
			std::vector<Ogre::Vector2> & vertices = m_ClampReg[r].m_triVertices;
			for(size_t i = 0; i < vertices.size(); i+=3)
			{
				m_manual->position(center + axis0 * vertices[i].x + axis1 * vertices[i].y + normal * 0.02);
				m_manual->colour(0,0.5,0.5,0.5);
				m_manual->position(center + axis0 * vertices[i+1].x + axis1 * vertices[i+1].y + normal * 0.02);
				m_manual->colour(0,0.5,0.5,0.5);
				m_manual->position(center + axis0 * vertices[i+2].x + axis1 * vertices[i+2].y + normal * 0.02);
				m_manual->colour(0,0.5,0.5,0.5);
			}
		}
		m_manual->end();
		//bounding
		m_manual->beginUpdate(2);
		for(int r = 0; r < 2; r++)
		{
			Ogre::Vector3 center = GPVec3ToOgre(m_ClampReg[r].m_OriginWorld);
			Ogre::Vector3 axis0 = GPVec3ToOgre(m_ClampReg[r].m_Axis0World);
			Ogre::Vector3 axis1 = GPVec3ToOgre(m_ClampReg[r].m_Axis1World);	
			Ogre::Vector3 normal = GPVec3ToOgre(GetClampRegNormal(r));
			m_manual->position(center + axis0 * m_ClampReg[r].m_axis0Min + axis1 * m_ClampReg[r].m_axis1Min);
			m_manual->colour(0,0.5,0.5,0.5);
			m_manual->position(center + axis0 * m_ClampReg[r].m_axis0Min + axis1 * m_ClampReg[r].m_axis1Max);
			m_manual->colour(0,0.5,0.5,0.5);
			m_manual->position(center + axis0 * m_ClampReg[r].m_axis0Max + axis1 * m_ClampReg[r].m_axis1Max);
			m_manual->colour(0,0.5,0.5,0.5);
			m_manual->position(center + axis0 * m_ClampReg[r].m_axis0Max + axis1 * m_ClampReg[r].m_axis1Min);
			m_manual->colour(0,0.5,0.5,0.5);
			m_manual->position(center + axis0 * m_ClampReg[r].m_axis0Min + axis1 * m_ClampReg[r].m_axis1Min);
			m_manual->colour(0,0.5,0.5,0.5);

			m_manual->position(center + axis0 * m_ClampReg[r].m_axis0Min + axis1 * m_ClampReg[r].m_axis1Min + normal * 0.16);
			m_manual->colour(0,0.5,0.5,0.5);
			m_manual->position(center + axis0 * m_ClampReg[r].m_axis0Min + axis1 * m_ClampReg[r].m_axis1Max + normal * 0.16);
			m_manual->colour(0,0.5,0.5,0.5);
			m_manual->position(center + axis0 * m_ClampReg[r].m_axis0Max + axis1 * m_ClampReg[r].m_axis1Max + normal * 0.16);
			m_manual->colour(0,0.5,0.5,0.5);
			m_manual->position(center + axis0 * m_ClampReg[r].m_axis0Max + axis1 * m_ClampReg[r].m_axis1Min + normal * 0.16);
			m_manual->colour(0,0.5,0.5,0.5);
			m_manual->position(center + axis0 * m_ClampReg[r].m_axis0Min + axis1 * m_ClampReg[r].m_axis1Min + normal * 0.16);
			m_manual->colour(0,0.5,0.5,0.5);


		}
		m_manual->end();
	}
}
//==================================================================================================
GFPhysVector3 MisCTool_PluginClamp::TransFormWorldClipDirToLocalDir(const GFPhysVector3 & worldDir,
																    std::vector<GFPhysSoftBodyFace*> & faceInRegL ,
																    std::vector<GFPhysSoftBodyFace*> & faceInRegR)
{
	float weights[3] = {0.3333f , 0.3333f , 0.3333f};

	float maxDot = -FLT_MAX;

	GFPhysSoftBodyFace * faceSel0 = 0;
	GFPhysSoftBodyFace * faceSel1 = 0;

	for(size_t l = 0 ; l < faceInRegL.size() ; l++)
	{
		GFPhysSoftBodyFace * pface = faceInRegL[l];
		GFPhysVector3 lcenter = pface->GetMassCenter(weights);

		//test every face in other Side
		for(size_t r = 0 ; r < faceInRegR.size() ; r++)
		{
			GFPhysSoftBodyFace * tFace = faceInRegR[r];

			GFPhysVector3 rcenter = tFace->GetMassCenter(weights);

			GFPhysVector3 nDirection = (rcenter - lcenter).Normalized();

			float angle = nDirection.Dot(worldDir);

			if(angle > maxDot)
			{
				maxDot = angle;
				faceSel0 = pface;
				faceSel1 = tFace;
			}
		}
	}

	if(faceSel0 && faceSel1)
	{
		//calculate material coordinate
		GFPhysVector3 center = faceSel0->GetMassCenter(weights);

		GFPhysVector3 e0 = (faceSel1->m_Nodes[0]->m_CurrPosition - center).Normalized();
		GFPhysVector3 e1 = (faceSel1->m_Nodes[1]->m_CurrPosition - center).Normalized();
		GFPhysVector3 e2 = (faceSel1->m_Nodes[2]->m_CurrPosition - center).Normalized();

		Ogre::Matrix3 coordinatematrix;
		coordinatematrix.SetColumn(0, GPVec3ToOgre(e0));
		coordinatematrix.SetColumn(1, GPVec3ToOgre(e1));
		coordinatematrix.SetColumn(2, GPVec3ToOgre(e2));

		Ogre::Matrix3 invcoordmat;

		bool succed = coordinatematrix.Inverse(invcoordmat);

		if(succed == true)//since e0,e1,e2 is linear independent the matrix must can be inverse
		{
			Ogre::Vector3 LocalVec = invcoordmat * GPVec3ToOgre(worldDir.Normalized());

			GFPhysVector3 undeformedcenter = (faceSel0->m_Nodes[0]->m_UnDeformedPos
											 +faceSel0->m_Nodes[1]->m_UnDeformedPos
											 +faceSel0->m_Nodes[2]->m_UnDeformedPos)*0.3333f;

			GFPhysVector3 u0 = (faceSel1->m_Nodes[0]->m_UnDeformedPos - undeformedcenter).Normalized();
			GFPhysVector3 u1 = (faceSel1->m_Nodes[1]->m_UnDeformedPos - undeformedcenter).Normalized();
			GFPhysVector3 u2 = (faceSel1->m_Nodes[2]->m_UnDeformedPos - undeformedcenter).Normalized();

			return (LocalVec.x*u0 + LocalVec.y*u1 + LocalVec.z*u2);
		}
	}
	return worldDir;
}
//===========================================================================================================
bool MisCTool_PluginClamp::IsThreadContactClampRegion(SutureThread* thread,  int segIndex , GFPhysVector3 & ResultPtInClampReg , GFPhysVector3 & ResultPtInTri , float seperateDist /*= 0*/ )
{

    GFPhysVector3 VertsWorld[2];

    SutureRopeNode & n0 = thread->GetThreadNodeRef(segIndex);
    SutureRopeNode & n1 = thread->GetThreadNodeRef(segIndex+1);
    
    VertsWorld[0] = n0.m_CurrPosition;
    VertsWorld[1] = n1.m_CurrPosition;


    GFPhysTransform transReg;
    GFPhysTransform transTri;

    transReg.SetIdentity();
    transTri.SetIdentity();

    GFPhysVector3 ClampRegionVert[6];

    bool isFound = false;

    //test every pair of clamp region represent by triangle pair
    for(size_t c = 0 ; c < m_ClampSpaceCells.size() ; c++)
    {
        ClampRegionVert[0] = m_ClampSpaceCells[c].m_CellVertsWorldSpace[0];
        ClampRegionVert[1] = m_ClampSpaceCells[c].m_CellVertsWorldSpace[1];
        ClampRegionVert[2] = m_ClampSpaceCells[c].m_CellVertsWorldSpace[2];

        ClampRegionVert[3] = m_ClampSpaceCells[c].m_CellVertsWorldSpace[3];
        ClampRegionVert[4] = m_ClampSpaceCells[c].m_CellVertsWorldSpace[4];
        ClampRegionVert[5] = m_ClampSpaceCells[c].m_CellVertsWorldSpace[5];


        GFPhysVector3 closetPointReg;
        GFPhysVector3 closetPointTri;

        float closetDist = GetConvexsClosetPoint(ClampRegionVert , 6 , 0 ,
            VertsWorld , 2 , 0,
            transReg , transTri ,
            closetPointReg , closetPointTri , 
            seperateDist + 0.1f);

        if(closetDist <= seperateDist)//0)//consider triangle in contact with region
        {
            isFound = true;
            ResultPtInClampReg = closetPointReg;
            ResultPtInTri = closetPointTri;
        }

        if(isFound)
            return isFound;
    }
    return isFound;    
}
//===========================================================================================================
bool MisCTool_PluginClamp::IsThreadContactClampRegion(SutureThreadV2* thread, int segIndex, GFPhysVector3 & ResultPtInClampReg, GFPhysVector3 & ResultPtInTri, float seperateDist /*= 0*/)
{

	GFPhysVector3 VertsWorld[2];

	GFPhysSoftTubeNode & n0 = thread->GetThreadNodeRef(segIndex);
	GFPhysSoftTubeNode & n1 = thread->GetThreadNodeRef(segIndex + 1);

	VertsWorld[0] = n0.m_CurrPosition;
	VertsWorld[1] = n1.m_CurrPosition;


	GFPhysTransform transReg;
	GFPhysTransform transTri;

	transReg.SetIdentity();
	transTri.SetIdentity();

	GFPhysVector3 ClampRegionVert[6];

	bool isFound = false;

	//test every pair of clamp region represent by triangle pair
	for (size_t c = 0; c < m_ClampSpaceCells.size(); c++)
	{
		ClampRegionVert[0] = m_ClampSpaceCells[c].m_CellVertsWorldSpace[0];
		ClampRegionVert[1] = m_ClampSpaceCells[c].m_CellVertsWorldSpace[1];
		ClampRegionVert[2] = m_ClampSpaceCells[c].m_CellVertsWorldSpace[2];

		ClampRegionVert[3] = m_ClampSpaceCells[c].m_CellVertsWorldSpace[3];
		ClampRegionVert[4] = m_ClampSpaceCells[c].m_CellVertsWorldSpace[4];
		ClampRegionVert[5] = m_ClampSpaceCells[c].m_CellVertsWorldSpace[5];


		GFPhysVector3 closetPointReg;
		GFPhysVector3 closetPointTri;

		float closetDist = GetConvexsClosetPoint(ClampRegionVert, 6, 0,
			VertsWorld, 2, 0,
			transReg, transTri,
			closetPointReg, closetPointTri,
			seperateDist + 0.1f);

		if (closetDist <= seperateDist)//0)//consider triangle in contact with region
		{
			isFound = true;
			ResultPtInClampReg = closetPointReg;
			ResultPtInTri = closetPointTri;
		}

		if (isFound)
			return isFound;
	}
	return isFound;
}

//===========================================================================================================
class ClampThreadSegmentTestCallBack : public GFPhysNodeOverlapCallback
{
public:
	ClampThreadSegmentTestCallBack(MisCTool_PluginClamp::ToolClampRegion * clampRegions , 
								   MisCTool_PluginClamp * clampPlugin , 
								   SutureThread * ropObject)
	{
		m_ThreadObject = ropObject;
		m_ClampPlugin = clampPlugin;
		m_IsRect = true;
	}

	void ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData)
	{
		int segmentIndex = (int)UserData;

		GFPhysVector3 closetPtSeg, closetPtRegion;		
				
        bool beclamped = m_ClampPlugin->IsThreadContactClampRegion(m_ThreadObject, segmentIndex, closetPtRegion, closetPtSeg, 0.05f);
		if(beclamped)
		{
			MisCTool_PluginClamp::ClampedRopeSegData clampSeg;
			
            clampSeg.m_SegmentIndex = segmentIndex;
            clampSeg.m_SegmentGlobalIndex = m_ThreadObject->GetThreadNodeRef(segmentIndex).m_GlobalId;
			
			GFPhysVector3 n0 , n1 ;
			m_ThreadObject->GetThreadSegmentNodePos(n0 , n1 , segmentIndex);
			float dist = (closetPtSeg - n0).Length();
			float length = (n1 - n0).Length();

			clampSeg.m_Weight = (length > FLT_EPSILON ? GPClamped(dist / length , 0.0f , 1.0f) : 0);

			m_SegmentBeClamped.push_back(clampSeg);
		}
	}

	MisCTool_PluginClamp * m_ClampPlugin;

	SutureThread * m_ThreadObject;

	std::vector<MisCTool_PluginClamp::ClampedRopeSegData> m_SegmentBeClamped;

	bool m_IsRect;

};

class ClampThreadSegmentTestCallBackV2 : public GFPhysNodeOverlapCallback
{
public:
	ClampThreadSegmentTestCallBackV2(MisCTool_PluginClamp::ToolClampRegion * clampRegions,
		MisCTool_PluginClamp * clampPlugin,
		SutureThreadV2 * ropObject)
	{
		m_ThreadObject = ropObject;
		m_ClampPlugin = clampPlugin;
		m_IsRect = true;
	}

	void ProcessOverlappedNode(int subPart, int triangleIndex, void * UserData)
	{
		int segmentIndex = (int)UserData;

		GFPhysVector3 closetPtSeg, closetPtRegion;

		bool beclamped = m_ClampPlugin->IsThreadContactClampRegion(m_ThreadObject, segmentIndex, closetPtRegion, closetPtSeg, 0.05f);
		if (beclamped)
		{
			MisCTool_PluginClamp::ClampedRopeSegData clampSeg;

			clampSeg.m_SegmentIndex = segmentIndex;
			clampSeg.m_SegmentGlobalIndex = m_ThreadObject->GetThreadNodeRefReal(segmentIndex).m_GlobalId;
			
			GFPhysVector3 n0, n1;
			
			n0 = m_ThreadObject->GetTubeWireSegment(segmentIndex).m_Node0->m_CurrPosition;
			n1 = m_ThreadObject->GetTubeWireSegment(segmentIndex).m_Node1->m_CurrPosition;

			float dist = (closetPtSeg - n0).Length();
			float length = (n1 - n0).Length();

			clampSeg.m_Weight = (length > FLT_EPSILON ? GPClamped(dist / length, 0.0f, 1.0f) : 0);

			m_SegmentBeClamped.push_back(clampSeg);
		}
	}

	MisCTool_PluginClamp * m_ClampPlugin;

	SutureThreadV2 * m_ThreadObject;

	std::vector<MisCTool_PluginClamp::ClampedRopeSegData> m_SegmentBeClamped;

	bool m_IsRect;

};
//===========================================================================================================
void MisCTool_PluginClamp::CalculateThreadSegmentsInClampRegions(Real NormalSpan , SutureThread * threadRope)
{
	if(threadRope == 0)
	   return;

	GFPhysVector3 aabbMin[2];
	GFPhysVector3 aabbMax[2];
	for(int r = 0 ; r < 2 ; r++)
	{
		m_ClampReg[r].CalcRegionWorldAABB( NormalSpan, aabbMin[r] , aabbMax[r]);
	}
	aabbMin[0].SetMin(aabbMin[1]);
	aabbMax[0].SetMax(aabbMax[1]);

	//traverse against tree of soft body
	ClampThreadSegmentTestCallBack callback(m_ClampReg , this , threadRope);
	callback.m_IsRect = m_IsRect;
	//GFPhysDBVTree tree = threadRope->GetSegmentBVTree();
    threadRope->GetSegmentBVTree().TraverseTreeAgainstAABB(&callback , aabbMin[0] , aabbMax[0]);
	
	for(size_t c = 0 ; c < callback.m_SegmentBeClamped.size() ; c++)
	{
		m_ClampedRopeSegments.push_back(callback.m_SegmentBeClamped[c]);
	}		
}
void MisCTool_PluginClamp::CalculateThreadSegmentsInClampRegions(Real NormalSpan, SutureThreadV2 * threadRope)
{
	if (threadRope == 0)
		return;

	GFPhysVector3 aabbMin[2];
	GFPhysVector3 aabbMax[2];
	for (int r = 0; r < 2; r++)
	{
		m_ClampReg[r].CalcRegionWorldAABB(NormalSpan, aabbMin[r], aabbMax[r]);
	}
	aabbMin[0].SetMin(aabbMin[1]);
	aabbMax[0].SetMax(aabbMax[1]);

	//traverse against tree of soft body
	ClampThreadSegmentTestCallBackV2 callback(m_ClampReg, this, threadRope);
	callback.m_IsRect = m_IsRect;
	//GFPhysDBVTree tree = threadRope->GetSegmentBVTree();
	threadRope->GetSegmentBVTree().TraverseTreeAgainstAABB(&callback, aabbMin[0], aabbMax[0]);

	for (size_t c = 0; c < callback.m_SegmentBeClamped.size(); c++)
	{
		m_ClampedRopeSegments.push_back(callback.m_SegmentBeClamped[c]);
		threadRope->SetSegmentBeClamped(callback.m_SegmentBeClamped[c].m_SegmentIndex);
	}

	

}

//for test
//========================================================================================
void MisCTool_PluginClamp::ReleaseClampedRope()
{    
    ITraining *currtrain = m_ToolObject->GetOwnerTraining();
    if(currtrain != NULL)
    {
        if (m_IsThreadClamp)
        {
            //train callback
            m_IsThreadClamp = false;

            MisNewTraining * pMisNewTraining = dynamic_cast<MisNewTraining*>(currtrain);
            if (pMisNewTraining)
            {
                pMisNewTraining->OnThreadReleaseByTool(m_ClampedRopeSegments[0].m_SegmentGlobalIndex,m_ToolObject);
            }
            //////////////////////////////////////////////////////////////////////////
            std::set<int>::iterator it;
            for(it = m_vRopeNodeClampedIndexList.begin(); it != m_vRopeNodeClampedIndexList.end(); it++) 
            {

                int index = -1;;
                SutureRopeNode& node = m_pClampedRope->GetThreadNodeGlobalRef(*it, index);
				if (-1 == index)
                    continue;

				node.SetSolverInvMassScaleForSelfCollision(1.0f);
				node.SetSolverInvMassScale(1.0f);//test
				node.m_StretchDampingX = 0.01f;
				node.m_BendDampingX = 0.01f;

				if (node.IsAttached())
				{
					node.MarkAsAttached(false);
					node.SetCanCollideRigid(true);
				}
            }
            //////////////////////////////////////////////////////////////////////////
            m_vRopeNodeClampedIndexList.clear();
            
            int index = -1;
			for (int i = 0; i < (int)m_ClampedRopeSegments.size(); i++)
            {
                SutureRopeNode & node = m_pClampedRope->GetThreadNodeGlobalRef(m_ClampedRopeSegments[i].m_SegmentGlobalIndex, index);
				if (-1 == index)
                {
                    continue;
                }
                
                for (vector<int>::iterator it = m_pClampedRope->m_ClampSegIndexVector.begin(); it != m_pClampedRope->m_ClampSegIndexVector.end();)
                {
                    if (*it == node.m_GlobalId)
                    {
                        it = m_pClampedRope->m_ClampSegIndexVector.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                }
            }

            m_ClampedRopeSegments.clear();           
            m_ToolObject->SetMinShaftAside(0.0f);
            m_ClampStateStage = MisCTool_PluginClamp::CSS_Released;
			m_pClampedRope = NULL;
        }
    }    
}
//========================================================================================
void MisCTool_PluginClamp::ReleaseClampedRopeV2()
{
	ITraining *currtrain = m_ToolObject->GetOwnerTraining();
	if (currtrain != NULL)
	{
		if (m_IsThreadClamp)
		{
			//train callback
			m_IsThreadClamp = false;

			MisNewTraining * pMisNewTraining = dynamic_cast<MisNewTraining*>(currtrain);
			if (pMisNewTraining)
			{
				pMisNewTraining->OnThreadReleaseByTool(m_ClampedRopeSegments[0].m_SegmentGlobalIndex, m_ToolObject);
			}

			for (int c = 0; c < (int)m_ClampedRopeSegments.size(); c++)
			    m_pClampedRopeV2->SetSegmentBeReleased(m_ClampedRopeSegments[c].m_SegmentIndex);

			//////////////////////////////////////////////////////////////////////////
			std::set<int>::iterator it;
			for (it = m_vRopeNodeClampedIndexList.begin(); it != m_vRopeNodeClampedIndexList.end(); it++)
			{

				int index = -1;;
				SutureThreadNodeV2& node = m_pClampedRopeV2->GetThreadNodeGlobalRef(*it, index);
				if (-1 == index)
					continue;

				//node.SetSolverInvMassScaleForSelfCollision(1.0f);

				GFPhysSoftTubeSegment & nodeNext = m_pClampedRopeV2->GetTubeWireSegment(index);

				if (nodeNext.IsAttached())
				{
					nodeNext.MarkAsAttached(false);
					nodeNext.SetCanCollideRigid(true);
				}
			}
			//////////////////////////////////////////////////////////////////////////
			m_vRopeNodeClampedIndexList.clear();

			int index = -1;
			for (int i = 0; i < (int)m_ClampedRopeSegments.size(); i++)
			{
				SutureThreadNodeV2 & node = m_pClampedRopeV2->GetThreadNodeGlobalRef(m_ClampedRopeSegments[i].m_SegmentGlobalIndex, index);
				if (-1 == index)
				{
					continue;
				}

				for (vector<int>::iterator it = m_pClampedRopeV2->m_ClampSegIndexVector.begin(); it != m_pClampedRopeV2->m_ClampSegIndexVector.end();)
				{
					if (*it == node.m_GlobalId)
					{
						it = m_pClampedRopeV2->m_ClampSegIndexVector.erase(it);
					}
					else
					{
						++it;
					}
				}
			}

			m_ClampedRopeSegments.clear();
			m_ToolObject->SetMinShaftAside(0.0f);
			m_ClampStateStage = MisCTool_PluginClamp::CSS_Released;
			m_pClampedRopeV2 = NULL;
		}
	}
}
//========================================================================================
bool MisCTool_PluginClamp::CheckRopeBeClamped()
{
	if (!m_CanClampThread || m_IsThreadClamp)
		return false;

	Real shaftTool = m_ToolObject->GetShaftAside();

    if (true)
    {
        //check which rope can be clamp

        std::map<SutureThread*,std::pair<int,int>> threadBeClamp;        

        for (size_t f = 0 ; f < m_ToolObject->m_ToolCollidedSutureThreads.size() ; f++)
        {
            const ToolCollideSutureThreadSegment & datacd = m_ToolObject->m_ToolCollidedSutureThreads[f];

            SutureThread * threadObject = datacd.m_collideSutureThread;
            GFPhysCollideObject * rigidbody = datacd.m_collideRigid; 
            GFPhysVector3 dircollide = datacd.m_NormalOnRigid;

            int k = m_ToolObject->GetRigidBodyPart(rigidbody); 
            if (k == 0)//LEFT
            {
                Real dotleft = dircollide.Dot(GetClampRegNormal(1));
                if (fabsf(dotleft) > 0.9f)
                {
                    threadBeClamp[threadObject].first++;
                }
            }
            else if ( k == 1)//RIGHT
            {
                Real dotright = dircollide.Dot(GetClampRegNormal(0));
                if (fabsf(dotright) > 0.9f)
                {
                    threadBeClamp[threadObject].second++;
                }
            }
            if (threadBeClamp[threadObject].first > 0 && threadBeClamp[threadObject].second > 0 )
            {
                CalculateThreadSegmentsInClampRegions(0.1f , threadObject);

                if(m_ClampedRopeSegments.size() > 0)
                {
                    Real minShaftToSet = 0.50f;
                    minShaftToSet = shaftTool - 1.0f;

                    minShaftToSet = GPClamped(minShaftToSet, m_minShaftRangeLowerValue, m_minShaftRangeUpperValue);

                    //m_ToolObject->SetMinShaftAside(minShaftToSet);

                    m_IsThreadClamp = true;
                    m_pClampedRope = threadObject;
					m_IsOpenAngleEnough = false;
                    m_ClampStateStage = MisCTool_PluginClamp::CSS_Clamped;                    
                    
                    //train callback
                    ITraining *currtrain = m_ToolObject->GetOwnerTraining();
                    MisNewTraining * pMisNewTraining = dynamic_cast<MisNewTraining*>(currtrain);
                    if (pMisNewTraining)
                    {
                        pMisNewTraining->OnThreadClampByTool(m_ClampedRopeSegments[0].m_SegmentGlobalIndex,m_ToolObject);
                    }

                    //m_pClampedRope->m_bAttached = true;
                    m_ReleaseClampShaft = shaftTool + 0.1f;
                    if(m_ReleaseClampShaft < 2)
                       m_ReleaseClampShaft = 2;//temple need refractory

                    //threadObject->SetClampedSegment(m_ClampedRopeSegments);

                    for(size_t s = 0 ; s < m_ClampedRopeSegments.size() ; s++)
                    {
                        GFPhysVector3 pos[2];
#if 0
                        int ClampSegIndex = -1;
                        SutureRopeNode & node = threadObject->GetThreadNodeGlobalRef(m_ClampedRopeSegments[s].m_SegmentGlobalIndex, ClampSegIndex);
                        if (ClampSegIndex == -1)
                        {
                            continue;
                        }
                        threadObject->GetThreadSegmentNodePos(pos[0], pos[1], ClampSegIndex);
#else
                        int ClampSegIndex = m_ClampedRopeSegments[s].m_SegmentIndex;
                        threadObject->GetThreadSegmentNodePos(pos[0], pos[1], ClampSegIndex);
#endif

                        SutureRopeNode & node = threadObject->GetThreadNodeRef(ClampSegIndex);

						node.SetSolverInvMassScaleForSelfCollision(0.0f);
						node.SetSolverInvMassScale(0.0f);//test
						node.m_StretchDampingX = 0.0f;
						node.m_BendDampingX = 0.0f;

                        threadObject->m_ClampSegIndexVector.push_back(node.m_GlobalId);

                        for (int i = 0; i < 2; i++)
                        {
                            GFPhysVector3 collidePos = pos[i];
							m_ClampedRopeSegments[s].m_Coord0[i] = (collidePos - m_ClampReg[0].m_OriginWorld).Dot(m_ClampReg[0].m_Axis0World);
							m_ClampedRopeSegments[s].m_Coord1[i] = (collidePos - m_ClampReg[0].m_OriginWorld).Dot(m_ClampReg[0].m_Axis1World);
                        }

                        int InfluenceSegIndex[3];
                        InfluenceSegIndex[0] = ClampSegIndex;
                        InfluenceSegIndex[1] = ClampSegIndex + 1;
                        InfluenceSegIndex[2] = ClampSegIndex - 1;

                        for(int t = 0 ; t < 3; t++)
                        {
                            if(InfluenceSegIndex[t] >= 1 && InfluenceSegIndex[t] < m_pClampedRope->GetNumThreadNodes())
                            {
                                if (m_vRopeNodeClampedIndexList.find(InfluenceSegIndex[t]) == m_vRopeNodeClampedIndexList.end())
                                {
                                    SutureRopeNode & nodeNext = threadObject->GetThreadNodeRef(InfluenceSegIndex[t]);
									nodeNext.MarkAsAttached(true);
									nodeNext.SetCanCollideRigid(false);
                                    m_vRopeNodeClampedIndexList.insert(nodeNext.m_GlobalId);
                                }
                            }
                        }
                    }
                    return m_IsThreadClamp;
                }
            }
        }
    }
	return m_IsThreadClamp;
}
//========================================================================================
bool MisCTool_PluginClamp::CheckRopeBeClampedV2()
{
	if (!m_CanClampThread || m_IsThreadClamp)
		return false;

	Real shaftTool = m_ToolObject->GetShaftAside();

	if (true)
	{
		//check which rope can be clamp

		std::map<SutureThreadV2*, std::pair<int, int>> threadBeClamp;

		for (size_t f = 0; f < m_ToolObject->m_ToolCollidedSutureThreadsV2.size(); f++)
		{
			const ToolCollideSutureThreadSegmentV2 & datacd = m_ToolObject->m_ToolCollidedSutureThreadsV2[f];

			SutureThreadV2 * threadObject = datacd.m_collideSutureThread;
			GFPhysCollideObject * rigidbody = datacd.m_collideRigid;
			GFPhysVector3 dircollide = datacd.m_NormalOnRigid;

			int k = m_ToolObject->GetRigidBodyPart(rigidbody);
			if (k == 0)//LEFT
			{
				Real dotleft = dircollide.Dot(GetClampRegNormal(1));
				if (fabsf(dotleft) > 0.9f)
				{
					threadBeClamp[threadObject].first++;
				}
			}
			else if (k == 1)//RIGHT
			{
				Real dotright = dircollide.Dot(GetClampRegNormal(0));
				if (fabsf(dotright) > 0.9f)
				{
					threadBeClamp[threadObject].second++;
				}
			}
			if (threadBeClamp[threadObject].first > 0 && threadBeClamp[threadObject].second > 0)
			{
				CalculateThreadSegmentsInClampRegions(0.1f, threadObject);

				if (m_ClampedRopeSegments.size() > 0)
				{
					Real minShaftToSet = 0.50f;
					minShaftToSet = shaftTool - 1.0f;

					minShaftToSet = GPClamped(minShaftToSet, m_minShaftRangeLowerValue, m_minShaftRangeUpperValue);

					m_ToolObject->SetMinShaftAside(minShaftToSet);

					m_IsThreadClamp = true;
					m_pClampedRopeV2 = threadObject;
					m_IsOpenAngleEnough = false;
					m_ClampStateStage = MisCTool_PluginClamp::CSS_Clamped;

					//train callback
					ITraining *currtrain = m_ToolObject->GetOwnerTraining();
					MisNewTraining * pMisNewTraining = dynamic_cast<MisNewTraining*>(currtrain);
					if (pMisNewTraining)
					{
						pMisNewTraining->OnThreadClampByTool(m_ClampedRopeSegments[0].m_SegmentGlobalIndex, m_ToolObject);
					}

					//m_pClampedRope->m_bAttached = true;
					m_ReleaseClampShaft = shaftTool + 0.1f;
					if (m_ReleaseClampShaft < 2)
						m_ReleaseClampShaft = 2;//temple need refractory

					//threadObject->SetClampedSegment(m_ClampedRopeSegments);

					for (size_t s = 0; s < m_ClampedRopeSegments.size(); s++)
					{
						GFPhysVector3 pos[2];
#if 0
						int ClampSegIndex = -1;
						SutureRopeNode & node = threadObject->GetThreadNodeGlobalRef(m_ClampedRopeSegments[s].m_SegmentGlobalIndex, ClampSegIndex);
						if (ClampSegIndex == -1)
						{
							continue;
						}						
						pos[0] = threadObject->GetTubeWireSegment(ClampSegIndex).m_Node0->m_CurrPosition;
						pos[1] = threadObject->GetTubeWireSegment(ClampSegIndex).m_Node1->m_CurrPosition;
#else
						int ClampSegIndex = m_ClampedRopeSegments[s].m_SegmentIndex;
						
						pos[0] = threadObject->GetTubeWireSegment(ClampSegIndex).m_Node0->m_CurrPosition;
						pos[1] = threadObject->GetTubeWireSegment(ClampSegIndex).m_Node1->m_CurrPosition;
#endif

						SutureThreadNodeV2 & node = threadObject->GetThreadNodeRefReal(ClampSegIndex);

						//node.SetSolverInvMassScaleForSelfCollision(0.0f);

						threadObject->m_ClampSegIndexVector.push_back(node.m_GlobalId);

						for (int i = 0; i < 2; i++)
						{
							GFPhysVector3 collidePos = pos[i];
							m_ClampedRopeSegments[s].m_Coord0[i] = (collidePos - m_ClampReg[0].m_OriginWorld).Dot(m_ClampReg[0].m_Axis0World);
							m_ClampedRopeSegments[s].m_Coord1[i] = (collidePos - m_ClampReg[0].m_OriginWorld).Dot(m_ClampReg[0].m_Axis1World);
						}

						int InfluenceSegIndex[3];
						InfluenceSegIndex[0] = ClampSegIndex;
						InfluenceSegIndex[1] = ClampSegIndex + 1;
						InfluenceSegIndex[2] = ClampSegIndex - 1;

						for (int t = 0; t < 3; t++)
						{
							if (InfluenceSegIndex[t] >= 1 && InfluenceSegIndex[t] < m_pClampedRopeV2->GetNumThreadNodes())
							{
								if (m_vRopeNodeClampedIndexList.find(InfluenceSegIndex[t]) == m_vRopeNodeClampedIndexList.end())
								{
									GFPhysSoftTubeSegment & nodeNext = threadObject->GetTubeWireSegment(InfluenceSegIndex[t]);
									nodeNext.MarkAsAttached(true);
									nodeNext.SetCanCollideRigid(false);

									SutureThreadNodeV2 & node = threadObject->GetThreadNodeRefReal(InfluenceSegIndex[t]);
									m_vRopeNodeClampedIndexList.insert(node.m_GlobalId);
								}
							}
						}
					}
					return m_IsThreadClamp;
				}
			}
		}
	}
	return m_IsThreadClamp;
}
