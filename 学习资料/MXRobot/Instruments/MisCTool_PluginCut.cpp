#include "MisCTool_PluginCut.h"
#include "Tool.h"
#include "MisMedicOrganInterface.h"
#include "MisMedicOrganOrdinary.h"
#include "VeinConnectObject.h"
#include "CustomCollision.h"
#include "MisMedicThreadRope.h"
#include "MXEvent.h"
#include "MXEventsDump.h"
#include "ITraining.h"
#include "Collision/CollisionDispatch/GoPhysSoftRigidCollision.h"

class OrganFaceClamped
{
public:
	OrganFaceClamped(GFPhysSoftBodyFace * ClampFace , float AngleBlade) : m_Face(ClampFace) , m_AngleBlade(AngleBlade)
	{
		m_IsInRange = false;
	}
	bool operator < (const OrganFaceClamped & rths)
	{
		if((m_IsInRange && rths.m_IsInRange) || (m_IsInRange==false && rths.m_IsInRange==false))
			return m_AngleBlade < rths.m_AngleBlade;
		else
			return m_IsInRange;
	}
	GFPhysSoftBodyFace * m_Face;
	float m_AngleBlade;
	bool  m_IsInRange;
};

class ThreadContactBlade
{
public:

	ThreadContactBlade(MisMedicThreadRope * rthread) : m_thread(rthread),m_LeftBladeContactCount(0),m_RightBladeContactCount(0)
	{}

	MisMedicThreadRope * m_thread;
	
	int m_LeftBladeContactCount;
	
	int m_RightBladeContactCount;
};

class OrganContactBlade
{
public:
	OrganContactBlade()
	{
		m_collideobj = 0;
	}

	void PushLeftClampFace(GFPhysSoftBodyFace * face , float angleblade)
	{
		m_LeftClampedFace.push_back(OrganFaceClamped(face , angleblade));
	}

	void PushRightClampFace(GFPhysSoftBodyFace * face , float angleblade)
	{
		m_RightClampedFace.push_back(OrganFaceClamped(face , angleblade));
	}
	GFPhysCollideObject * m_collideobj;

	std::vector<OrganFaceClamped> m_LeftClampedFace;

	std::vector<OrganFaceClamped> m_RightClampedFace;
};
//==================================================================================================
class VeinConnTestCutCallBack : public GFPhysNodeOverlapCallback
{
public:
	VeinConnTestCutCallBack(MisCTool_PluginCut * cutPlugin)  : m_pCutPlugin(cutPlugin){}

	void ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData)
	{
		VeinCollideData * cdata = (VeinCollideData*)UserData;

		if(!cdata)
			return;

		VeinConnectObject *veinobj = cdata->m_HostObject;
		VeinConnectCluster & cluster = veinobj->m_clusters[cdata->m_ClusterIndex];

		bool isStick = cdata->m_InContact || cdata->m_InHookState;
		TestVeinConn(cdata , isStick);

	}

	void TestVeinConn(VeinCollideData * veinCollideData, bool isStick)
	{
		VeinConnectObject *veinobj = veinCollideData->m_HostObject;
		VeinConnectCluster & cluster = veinobj->m_clusters[veinCollideData->m_ClusterIndex];


        VeinConnectPair &pair1 = cluster.m_pair[0];
        VeinConnectPair &pair2 = cluster.m_pair[1];

		//
		GFPhysVector3 cutTri[2][3];

		cutTri[0][0] =  m_pCutPlugin->m_ToolObject->m_CutBladeRight.m_LinePointsWorld[0];
		cutTri[0][1] =  m_pCutPlugin->m_ToolObject->m_CutBladeRight.m_LinePointsWorld[1];
		cutTri[0][2] =  m_pCutPlugin->m_ToolObject->m_CutBladeLeft.m_LinePointsWorld[1];

		cutTri[1][0] =  m_pCutPlugin->m_ToolObject->m_CutBladeLeft.m_LinePointsWorld[0];
		cutTri[1][1] =  m_pCutPlugin->m_ToolObject->m_CutBladeRight.m_LinePointsWorld[0];
		cutTri[1][2] =  m_pCutPlugin->m_ToolObject->m_CutBladeLeft.m_LinePointsWorld[1];

		//
		GFPhysVector3 connVertrices[2][3];
		if(isStick)
		{
			connVertrices[0][0] = veinCollideData->m_contactinWorld;
			connVertrices[0][1] = OgreToGPVec3(pair1.m_CurrPointPosA);
			connVertrices[0][2] = OgreToGPVec3(pair2.m_CurrPointPosA);

			connVertrices[1][0] = veinCollideData->m_contactinWorld;
			connVertrices[1][1] = OgreToGPVec3(pair1.m_CurrPointPosB);
			connVertrices[1][2] = OgreToGPVec3(pair2.m_CurrPointPosB);
		} else {
			connVertrices[0][0] = OgreToGPVec3(pair1.m_CurrPointPosA);
			connVertrices[0][1] = OgreToGPVec3(pair2.m_CurrPointPosA);
			connVertrices[0][2] = OgreToGPVec3(pair1.m_CurrPointPosB);

			connVertrices[1][0] = OgreToGPVec3(pair2.m_CurrPointPosA);
			connVertrices[1][1] = OgreToGPVec3(pair1.m_CurrPointPosB);
			connVertrices[1][2] = OgreToGPVec3(pair2.m_CurrPointPosB);
		}

		bool isInCutRegion = false;

		GFPhysVector3 result[2];
		for(int cutRegion = 0 ; cutRegion < 2; cutRegion++)
		{
			for(int connTri = 0 ; connTri < 2 ; connTri++)
			{
				isInCutRegion = TriangleIntersect(cutTri[cutRegion] , connVertrices[connTri] , result);
				if(isInCutRegion) break;
			}
			if(isInCutRegion) break;
		}

		if(isInCutRegion)
		{
			MisCTool_PluginCut::VeinConnInCutRegion connInCutRegion(veinobj, veinCollideData->m_ClusterIndex , veinCollideData->m_PairIndex);
			m_VeinConnsInCutRegion.push_back(connInCutRegion);
		}
	}


	MisCTool_PluginCut *m_pCutPlugin;
	std::vector<MisCTool_PluginCut::VeinConnInCutRegion> m_VeinConnsInCutRegion;
};
//==================================================================================================
static bool IsClampedEnough(OrganContactBlade & organcut , GFPhysVector3 leftbladepoint[2] , GFPhysVector3 rightbladepoint[2])
{
	GFPhysVector3 CliptriVerts[2][3];
	GFPhysVector3 Dir0 = rightbladepoint[0]-leftbladepoint[0];
	Dir0.Normalize();

	GFPhysVector3 Dir1 = rightbladepoint[1]-leftbladepoint[1];
	Dir1.Normalize();

	CliptriVerts[0][0] = leftbladepoint[0]-Dir0*0.5f;
	CliptriVerts[0][1] = leftbladepoint[1]-Dir1*0.5f;
	CliptriVerts[0][2] = rightbladepoint[0]+Dir0*0.5f;

	CliptriVerts[1][0] = leftbladepoint[1]-Dir1*0.5f;
	CliptriVerts[1][1] = rightbladepoint[0]+Dir0*0.5f;
	CliptriVerts[1][2] = rightbladepoint[1]+Dir1*0.5f;

	float LeftLenSquare  = (leftbladepoint[1]-leftbladepoint[0]).Dot(leftbladepoint[1]-leftbladepoint[0]);

	float RightLenSquare = (rightbladepoint[1]-rightbladepoint[0]).Dot(rightbladepoint[1]-rightbladepoint[0]);

	float leftmint = FLT_MAX;

	float leftmaxt = -FLT_MAX;

	for(size_t f = 0 ; f < organcut.m_LeftClampedFace.size() ; f++)
	{
		GFPhysVector3 ResultPoint[2];

		GFPhysVector3 FaceVerts[3];
		GFPhysSoftBodyFace * face = organcut.m_LeftClampedFace[f].m_Face;
		FaceVerts[0] = face->m_Nodes[0]->m_CurrPosition;
		FaceVerts[1] = face->m_Nodes[1]->m_CurrPosition;
		FaceVerts[2] = face->m_Nodes[2]->m_CurrPosition;

		for(int t = 0 ; t < 2 ; t++)
		{
			bool intersect = TriangleIntersect(CliptriVerts[t] , FaceVerts , ResultPoint);

			if(intersect)
			{
				float t0 = (ResultPoint[0]-leftbladepoint[0]).Dot(leftbladepoint[1]-leftbladepoint[0]) / LeftLenSquare;
				float t1 = (ResultPoint[1]-leftbladepoint[0]).Dot(leftbladepoint[1]-leftbladepoint[0]) / LeftLenSquare;
				if(t0 < leftmint)
					leftmint = t0;
				if(t0 > leftmaxt)
					leftmaxt = t0;

				if(t1 < leftmint)
					leftmint = t1;
				if(t1 > leftmaxt)
					leftmaxt = t1;

				organcut.m_LeftClampedFace[f].m_IsInRange = true;
			}
		}

	}


	float rightmint = FLT_MAX;

	float rightmaxt = -FLT_MAX;

	for(size_t f = 0 ; f < organcut.m_RightClampedFace.size() ; f++)
	{
		GFPhysVector3 ResultPoint[2];

		GFPhysVector3 FaceVerts[3];
		GFPhysSoftBodyFace * face = organcut.m_RightClampedFace[f].m_Face;
		FaceVerts[0] = face->m_Nodes[0]->m_CurrPosition;
		FaceVerts[1] = face->m_Nodes[1]->m_CurrPosition;
		FaceVerts[2] = face->m_Nodes[2]->m_CurrPosition;

		for(int t = 0 ; t < 2 ; t++)
		{
			bool intersect = TriangleIntersect(CliptriVerts[t] , FaceVerts , ResultPoint);

			if(intersect)
			{
				float t0 = (ResultPoint[0]-rightbladepoint[0]).Dot(rightbladepoint[1]-rightbladepoint[0]) / RightLenSquare;
				float t1 = (ResultPoint[1]-rightbladepoint[0]).Dot(rightbladepoint[1]-rightbladepoint[0]) / RightLenSquare;
				if(t0 < rightmint)
					rightmint = t0;
				if(t0 > rightmaxt)
					rightmaxt = t0;

				if(t1 < rightmint)
					rightmint = t1;
				if(t1 > rightmaxt)
					rightmaxt = t1;

				organcut.m_RightClampedFace[f].m_IsInRange = true;
			}
		}
	}

	if(/*leftmint < 0.1f && */leftmaxt > 0.1f || /*rightmint < 0.1f &&*/ rightmaxt > 0.1f)
		return true;
	else
		return false;
}

MisCTool_PluginCut::MisCTool_PluginCut(CTool * tool, Real checkCutStartShaft)
: MisMedicCToolPluginInterface(tool)
{
    m_CutBeginCheckShaft = checkCutStartShaft;
	m_CanPerformCutOperaion = false;
	m_TimeElapsedSinceLastClip = FLT_MAX;
	m_MaxShaftSinceLastClip = 0;
	m_IsClampingConnect = false;
	m_IsClampingOrgans = false;
	m_ClampingClockTimes = 0;
}

MisCTool_PluginCut::~MisCTool_PluginCut()
{

}

//===========================================================================================================
void MisCTool_PluginCut::onRSContactsBuildBegin(ConvexSoftFaceCollidePair * collidePairs , int NumCollidePair)
{
	GFPhysRigidBody * rightRB = m_ToolObject->m_righttoolpartconvex.m_rigidbody;
	
	GFPhysRigidBody * leftRB = m_ToolObject->m_lefttoolpartconvex.m_rigidbody;

	GFPhysVector3 rightClampNormal = m_ToolObject->m_CutBladeRight.m_CuttDirectionWord;
	GFPhysVector3 leftClampNormal  = m_ToolObject->m_CutBladeLeft.m_CuttDirectionWord;
	if(m_TimeElapsedSinceLastClip < 0.5f)
	{
		for(size_t c = 0 ; c < NumCollidePair ; c++)
		{
			ConvexSoftFaceCollidePair & cdpair = collidePairs[c];

			if(cdpair.m_ConvexObj == leftRB && leftClampNormal.Dot(cdpair.m_normalOnFaceInWorld) < 0)
			{
				cdpair.m_stateTag &= (~ConvexSoftFaceCollidePair::CFP_ISCOLLIDED);
			}
			else if(cdpair.m_ConvexObj == rightRB && rightClampNormal.Dot(cdpair.m_normalOnFaceInWorld) < 0)
			{
				cdpair.m_stateTag &= (~ConvexSoftFaceCollidePair::CFP_ISCOLLIDED);
			}
		}
	}
}
//==================================================================================================
void MisCTool_PluginCut::PhysicsSimulationEnd(int currStep , int TotalStep , float dt)
{
	if (m_CanPerformCutOperaion == false)
		return;

    float toolshaft = m_ToolObject->GetShaftAside();

	ITraining * currtrain = m_ToolObject->GetOwnerTraining();

	if(toolshaft <= m_CutBeginCheckShaft)//begin check
	{
	    //first check cut organ
		int numOrganBeCutted = 0;
		 
		int numConnectPairsCutted = 0;

	    std::vector<MisMedicOrgan_Ordinary*> organsGrasped;

		m_ToolObject->GetGraspedOrgans(organsGrasped);
		  
		//todo 加入判断抓取的面是否和剪面向量垂直
		if (m_IsClampingOrgans)
		{
			if (m_ClampingClockTimes >= 5)
			{
				for (int c = 0; c < (int)organsGrasped.size(); c++)
				{
					if (organsGrasped[c]->CanBeCut())
					{
						m_ToolObject->ReleaseClampedOrgans();

						organsGrasped[c]->CutOrganByTool(m_ToolObject);

						numOrganBeCutted++;

						m_IsClampingOrgans = false;

						m_ClampingClockTimes = 0;
					}
				}
			}
			m_ClampingClockTimes++;
		}
	  
		//try cut connect pairs
		//first disconnect clamped pairs
		if (m_IsClampingConnect)
		{
			numConnectPairsCutted = m_ToolObject->DisconnectClampedVeinConnectPairs();

			m_IsClampingConnect = false;
		}
		//further check pairs in region
		/*ClearVeinConnInCutRegion();
		
		std::vector<VeinConnectObject*> veinobjs = currtrain->GetVeinConnectObjects();
		
		for (int v = 0; v < veinobjs.size(); v++)
		{
			TestVeinConnInCutReions(veinobjs[v]);
		}

		for (int c = 0; c < m_VeinConnInCutRegion.size(); c++)
		{
			VeinConnInCutRegion & conn = m_VeinConnInCutRegion[c];
			conn.m_pVeinObj->DisconnectPair(conn.m_ClusterId, conn.m_PairId, 2);
			numConnectPairsCutted++;
		}*/

		if (numOrganBeCutted > 0 || (numConnectPairsCutted > 0 && m_IsClampingOrgans == false))
		{
			m_CanPerformCutOperaion = false;

			m_LastClipShaft = m_MaxShaftSinceLastClip = toolshaft;

			m_TimeElapsedSinceLastClip = 0;

			return;
		}


	   //final try cut thread
	   std::vector<ThreadContactBlade> ThreadToCut;
	   
	   GFPhysRigidBody * leftpart  = m_ToolObject->m_lefttoolpartconvex.m_rigidbody;

	   GFPhysRigidBody * rightpart = m_ToolObject->m_righttoolpartconvex.m_rigidbody;
	  
	   for(size_t c = 0 ; c < m_ToolObject->m_ToolCollidedThreads.size() ; c++)
	   {
		   const ToolCollideThreadSegment & collideseg = m_ToolObject->m_ToolCollidedThreads[c];
		   
		   int tIndex = -1;

		   for(size_t t = 0 ; t < ThreadToCut.size() ; t++)
		   {
				if(ThreadToCut[t].m_thread == collideseg.m_collideThread)
				{
					tIndex = t;
					break;
				}
		   }
		   if(tIndex < 0)
		   {
		      ThreadToCut.push_back(ThreadContactBlade(collideseg.m_collideThread));
			  tIndex = (int)ThreadToCut.size()-1;
		   }
		  
		   if(collideseg.m_collideRigid == leftpart)
		      ThreadToCut[tIndex].m_LeftBladeContactCount++;
		  
		   if(collideseg.m_collideRigid == rightpart)
			  ThreadToCut[tIndex].m_RightBladeContactCount++;

		   if(ThreadToCut[tIndex].m_LeftBladeContactCount > 0 && ThreadToCut[tIndex].m_RightBladeContactCount > 0)
		   {
			   bool cutted = collideseg.m_collideThread->CutThreadByTool(m_ToolObject);
			  
			   if(cutted == true)
			   {
				   break;
			   }
		   }
	   } 

	   
	   //check cut vein connect
	   //if(toolshaft == 0)
	  // {
	   
		   /*
		   for(int c = 0 ; c < m_VeinConnInCutRegion.size() ; c++)
		   {
			   VeinConnInCutRegion & conn = m_VeinConnInCutRegion[c];
			   conn.m_pVeinObj->DisconnectPair(conn.m_ClusterId , conn.m_PairId , 2);
		   }
		   if(m_VeinConnInCutRegion.size() > 0)
		   {
			   m_CanPerformCutOperaion = false;

			   m_LastClipShaft = m_MaxShaftSinceLastClip = toolshaft;

			   m_TimeElapsedSinceLastClip = 0;
		   }
		   ClearVeinConnInCutRegion();
		   */
	  // } else {
		   /*ClearVeinConnInCutRegion();
		   ITraining * currtrain = m_ToolObject->GetOwnerTraining();
		   std::vector<VeinConnectObject*>  veinobjs = currtrain->GetVeinConnectObjects();
		   for(int v = 0 ; v < veinobjs.size() ; v++)
		   {
				TestVeinConnInCutReions(veinobjs[v]);
		   }*/
	  // }
	  


	}
}
//==================================================================================================
void MisCTool_PluginCut::OneFrameUpdateStarted(float timeelapsed)
{
	if(m_CanPerformCutOperaion == false)
	{
		m_TimeElapsedSinceLastClip += timeelapsed;

		float toolshaft = m_ToolObject->GetShaftAside();

		if(toolshaft > m_MaxShaftSinceLastClip)
		   m_MaxShaftSinceLastClip = toolshaft;

		
		if (m_TimeElapsedSinceLastClip > 1.0f && toolshaft < m_MaxShaftSinceLastClip-1.0f && m_MaxShaftSinceLastClip > m_CutBeginCheckShaft + 1.0f)
		{
			m_CanPerformCutOperaion = true;
		}

		//器械合拢后重置
		if (toolshaft <= 0)
		{
			m_MaxShaftSinceLastClip = 0;
			m_CanPerformCutOperaion = false;
		}
	}
}
//==================================================================================================
// void MisCTool_PluginCut::OneFrameUpdateEnded()
// {
// 	
// }

void MisCTool_PluginCut::TestVeinConnInCutReions(VeinConnectObject *veinobj)
{
	GFPhysVector3 aabbMin;
	GFPhysVector3 aabbMax;

	aabbMin.SetMin(m_ToolObject->m_CutBladeLeft.m_LinePointsWorld[0]);
	aabbMin.SetMin(m_ToolObject->m_CutBladeLeft.m_LinePointsWorld[1]);
	aabbMin.SetMin(m_ToolObject->m_CutBladeRight.m_LinePointsWorld[0]);
	aabbMin.SetMin(m_ToolObject->m_CutBladeRight.m_LinePointsWorld[1]);

	aabbMax.SetMax(m_ToolObject->m_CutBladeLeft.m_LinePointsWorld[0]);
	aabbMax.SetMax(m_ToolObject->m_CutBladeLeft.m_LinePointsWorld[1]);
	aabbMax.SetMax(m_ToolObject->m_CutBladeRight.m_LinePointsWorld[0]);
	aabbMax.SetMax(m_ToolObject->m_CutBladeRight.m_LinePointsWorld[1]);

	VeinConnTestCutCallBack callback(this);
	veinobj->GetCollideTree().TraverseTreeAgainstAABB(&callback , aabbMin , aabbMax);

	for(int i = 0 ; i < callback.m_VeinConnsInCutRegion.size() ;i++)
		m_VeinConnInCutRegion.push_back(callback.m_VeinConnsInCutRegion[i]);
}
