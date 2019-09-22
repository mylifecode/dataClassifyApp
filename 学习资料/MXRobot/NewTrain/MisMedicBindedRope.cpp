#include "MisMedicBindedRope.h"
#include "Math/GoPhysTransformUtil.h"
#include "Topology/GoPhysSoftBodyRestShapeModify.h"
#include "MXOgreGraphic.h"
#include "MisMedicThreadRope.h"
#include "MisMedicOrganOrdinary.h"

class PointFaceDistInRangeCallback : public GFPhysNodeOverlapCallback
{
public:
	struct PointClosetToFace
	{
	public:

		PointClosetToFace(const GFPhysVector3 & nodePos)
		{
			m_PointPos = nodePos;
			m_ClosetFaces = 0;
			m_MinDist = FLT_MAX;
		}
		
		void SetDistFace(GFPhysSoftBodyFace * face , const GFPhysVector3 & posInFace , float dist)
		{
			if(dist < m_MinDist)
			{
				m_ClosetFaces = face;
				m_PosInFace = posInFace;
				m_MinDist = dist;
			}
		}
		
		GFPhysVector3 m_PointPos;
		GFPhysSoftBodyFace * m_ClosetFaces;
		GFPhysVector3 m_PosInFace;
		float m_MinDist;
	};

	PointFaceDistInRangeCallback(GFPhysAlignedVectorObj<GFPhysVector3> & threadPoints , float threshold)
	{
		m_ThresDist = threshold;
		
		for(size_t c = 0 ; c < threadPoints.size(); c++)
		{
			PointClosetToFace facept(threadPoints[c]);

			m_threadPoints.push_back(facept);
		}
	}
	
	virtual ~PointFaceDistInRangeCallback()
	{

	}

	virtual void ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData)
	{

	}

	virtual void ProcessOverlappedNodes(GFPhysDBVNode * dynNodes , GFPhysAABBNode * staticnode)
	{

	}

	virtual void ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA , GFPhysDBVNode * dynNodeB)
	{
		int NodeIndex = (int)dynNodeA->m_UserData;
		
		GFPhysSoftBodyFace * softFace = (GFPhysSoftBodyFace*)dynNodeB->m_UserData;
		
		GFPhysVector3 NodePos = m_threadPoints[NodeIndex].m_PointPos;

		GFPhysVector3 closetPoint = ClosestPtPointTriangle(NodePos, 
														   softFace->m_Nodes[0]->m_CurrPosition, 
														   softFace->m_Nodes[1]->m_CurrPosition, 
														   softFace->m_Nodes[2]->m_CurrPosition);
		float dist = (closetPoint - NodePos).Length();
		
		if(dist < m_ThresDist)
		{
			m_threadPoints[NodeIndex].SetDistFace(softFace , closetPoint , dist);
		}
	}

	GFPhysAlignedVectorObj<PointClosetToFace> m_threadPoints;
	
	float m_ThresDist;
};

class RaysSurfaceCallback : public GFPhysNodeOverlapCallback
{
public:
	RaysSurfaceCallback(const GFPhysAlignedVectorObj<GFPhysVector3> & samplePoints , const GFPhysVector3 & rayStart) 
	{
		m_RayStart = rayStart;
		m_SamplePoints = samplePoints;

		MisMedicBindedRope::ThreadBindPoint nullPoint;
		nullPoint.m_Temp = FLT_MAX;

		for(size_t c = 0 ; c < m_SamplePoints.size() ; c++)
		{
			m_ResultBindPoint.push_back(nullPoint);
		}
	}

	virtual void ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData)
	{
		GFPhysSoftBodyFace * face = (GFPhysSoftBodyFace*)UserData;
		//brute force need optimize
		for(size_t c = 0 ; c < m_SamplePoints.size() ; c++)
		{
			Real Rayweight; 
			GFPhysVector3 intersectpt;
			Real triangleWeight[3];

			bool hitted = LineIntersectTriangle(face->m_Nodes[0]->m_CurrPosition ,
											    face->m_Nodes[1]->m_CurrPosition , 
											    face->m_Nodes[2]->m_CurrPosition ,
											    m_RayStart , 
											    m_SamplePoints[c] , 
											    Rayweight , 
											    intersectpt , 
											    triangleWeight);
			
			if(hitted && Rayweight > 0 && Rayweight < 1)
			{
				if((m_ResultBindPoint[c].m_AttachFace == 0) || (Rayweight < m_ResultBindPoint[c].m_Temp))
				{
					m_ResultBindPoint[c].m_AttachFace = face;

					m_ResultBindPoint[c].m_Weights[0] = triangleWeight[0];
					
					m_ResultBindPoint[c].m_Weights[1] = triangleWeight[1];
					
					m_ResultBindPoint[c].m_Weights[2] = triangleWeight[2];

					m_ResultBindPoint[c].m_Temp = Rayweight;
				}
			}
		}
	}

	GFPhysAlignedVectorObj<MisMedicBindedRope::ThreadBindPoint> m_ResultBindPoint;
	GFPhysAlignedVectorObj<GFPhysVector3> m_SamplePoints;
	GFPhysVector3 m_RayStart;
};


MisMedicBindedRope::FaceIntersectPoint::FaceIntersectPoint(GFPhysSoftBodyFace * face)
{
	m_PointCount = 0;
	m_Face = face;
	m_LinkedNext = -1;
}
void MisMedicBindedRope::FaceIntersectPoint::AddEdgePoint(GFPhysSoftBodyNode * epoint0 , GFPhysSoftBodyNode * epoint1 ,float weight0)
{
	if(m_PointCount < 2)
	{
		m_s[m_PointCount][0] = epoint0;
		m_s[m_PointCount][1] = epoint1;
		m_Weight[m_PointCount] = weight0;
		GFPhysVector3 temp = epoint0->m_CurrPosition*(1-weight0) + epoint1->m_CurrPosition*weight0;
		m_Position[m_PointCount] = Ogre::Vector3(temp.x() , temp.y() , temp.z());
		m_PointCount++;
	}
}

MisMedicBindedRope::ThreadBindPoint::ThreadBindPoint()
{
	m_AttachFace = 0;
	//m_Valid = false;
}
MisMedicBindedRope::ThreadBindPoint::ThreadBindPoint(const Ogre::Vector3 & position , GFPhysSoftBodyFace * face)
{
	m_AttachFace = face;
	//m_Valid = true;
	CalcBaryCentric(face->m_Nodes[0]->m_CurrPosition,
					face->m_Nodes[1]->m_CurrPosition,
					face->m_Nodes[2]->m_CurrPosition,
					OgreToGPVec3(position),
					m_Weights[0],
					m_Weights[1],
					m_Weights[2]);

	m_UndeformedPos = GPVec3ToOgre(face->m_Nodes[0]->m_UnDeformedPos*m_Weights[0]
								  +face->m_Nodes[1]->m_UnDeformedPos*m_Weights[1]
								  +face->m_Nodes[2]->m_UnDeformedPos*m_Weights[2]);

	m_ShrinkUndefPos = m_UndeformedPos;
}
void MisMedicBindedRope::ThreadBindPoint::ReAttach(GFPhysSoftBodyFace * face , float weights[3])
{
	m_AttachFace = face;
	//m_Valid = true;

	m_Weights[0] = weights[0];
	m_Weights[1] = weights[1];
	m_Weights[2] = weights[2];

	m_UndeformedPos = GPVec3ToOgre(face->m_Nodes[0]->m_UnDeformedPos*m_Weights[0]
								  +face->m_Nodes[1]->m_UnDeformedPos*m_Weights[1]
								  +face->m_Nodes[2]->m_UnDeformedPos*m_Weights[2]);
}

Ogre::Vector3 MisMedicBindedRope::ThreadBindPoint::GetPassPointPosition()const
{
	GFPhysVector3 temp = m_AttachFace->m_Nodes[0]->m_CurrPosition * m_Weights[0]+
						 m_AttachFace->m_Nodes[1]->m_CurrPosition * m_Weights[1]+
						 m_AttachFace->m_Nodes[2]->m_CurrPosition * m_Weights[2];

	return Ogre::Vector3(temp.x() , temp.y() , temp.z());
}

Ogre::Vector3 MisMedicBindedRope::ThreadBindPoint::GetPassPointUndeformedPosition() const
{
	/*GFPhysVector3 temp = m_AttachFace->m_Nodes[0]->m_UnDeformedPos * m_Weights[0]+
		m_AttachFace->m_Nodes[1]->m_UnDeformedPos * m_Weights[1]+
		m_AttachFace->m_Nodes[2]->m_UnDeformedPos * m_Weights[2];

	return Ogre::Vector3(temp.x() , temp.y() , temp.z());
	*/
	return m_UndeformedPos;
}

//==============================================================
MisMedicBindedRope::MisMedicBindedRope(Ogre::SceneManager * sceneMgr) 
: m_BindedOrgan(0) , 
	m_KnotNode(NULL),
	m_KnotEnitity(NULL),
	//m_pFaceWithKnot(NULL),
	m_ConnectState(MisMedicBindedRope::CS_UNCONNECTED)
{
	m_type = MOAType_BindedRope;

	static int s_BindRopeId = 0;
	s_BindRopeId++;
	
	m_BoundRopeID = s_BindRopeId;

	Ogre::String strRopeName = "BindRopeObject" + Ogre::StringConverter::toString(s_BindRopeId);
	m_RendObject.CreateRendPart(strRopeName , sceneMgr);
	m_NeedRend = true;
	m_UserData = 0;
}
//==============================================================
MisMedicBindedRope::~MisMedicBindedRope()
{
	if(m_KnotEnitity)
	{
		m_KnotEnitity->detachFromParent();
		MXOgreWrapper::Get()->GetDefaultSceneManger()->destroyEntity(m_KnotEnitity);
		m_KnotEnitity = NULL;
	}
	if(m_KnotNode)
	{
		MXOgreWrapper::Get()->GetDefaultSceneManger()->getRootSceneNode()->removeChild(m_KnotNode);
		MXOgreWrapper::Get()->GetDefaultSceneManger()->destroySceneNode(m_KnotNode);
		m_KnotNode = NULL;
	}
}
struct ConnectedSegment
{
	ConnectedSegment(int headface , int tailface,
					 int headpoint , int tailpoint)
	{
		m_HeadFace  = headface;
		m_TailFace  = tailface;
		m_HeadPoint = headpoint;
		m_TailPoint = tailpoint;
	}
	int m_HeadFace;
	int m_TailFace;
	int m_HeadPoint;
	int m_TailPoint;
};
bool MisMedicBindedRope::TryBindThread(MisMedicOrgan_Ordinary & organ , 
									   GFPhysAlignedVectorObj<GFPhysVector3> & threadPoints , 
									   const GFPhysVector3 & loopFixPoint , 
									   float MaxDeviate)
{

	if(m_BindedOrgan)
	   return true;

	m_ThreadBindPoint.clear();

	float TolerantDist = MaxDeviate;

	GFPhysDBVTree ThreadNodesTree;

	GFPhysSoftBodyShape & softShape = organ.m_physbody->GetSoftBodyShape();

	int NumNode = (int)threadPoints.size();

	if(NumNode < 20)
	   return false;

	//start bind 1/2 half of the loop thread
	int MainLoopStart = NumNode / 4;

	int MainLoopEnd   = NumNode-NumNode / 4;

	GFPhysVector3 MainLoopSkin(TolerantDist , TolerantDist , TolerantDist);

	for(int c = MainLoopStart ; c <= MainLoopEnd ; c++)//add all node to bv tree with margin = tolerant
	{
		GFPhysVector3 NodePos = threadPoints[c];
		GFPhysDBVNode * bvNode = ThreadNodesTree.InsertAABBNode(NodePos-MainLoopSkin , NodePos+MainLoopSkin);
		bvNode->m_UserData = (void*)c;
	}

	//get all closet point to the node
	GFPhysVectorObj<GFPhysDBVTree*> bvTrees = softShape.GetFaceBVTrees();
	PointFaceDistInRangeCallback collideCallBack(threadPoints , TolerantDist);
	for(size_t t = 0 ; t < bvTrees.size() ; t++)
	{
		GFPhysDBVTree * bvTree = bvTrees[t];
		ThreadNodesTree.CollideWithDBVTree(*bvTree , &collideCallBack);
	}

	bool AllFindClosetPoint = true;
	
	//save all loop thread node's closet point in face if exists
	int closetStartIndex = MainLoopStart;

	int closetEndIndex;

	for(int c = MainLoopStart ; c <= MainLoopEnd ; c++)
	{
		PointFaceDistInRangeCallback::PointClosetToFace pointFace = collideCallBack.m_threadPoints[c];
		if(pointFace.m_ClosetFaces == 0)
		{
			AllFindClosetPoint = false;//m_ThreadBindPoint.clear();//return false;
			break;
		}
		else
		{
			GFPhysSoftBodyFace * face = pointFace.m_ClosetFaces;
			Ogre::Vector3 PosInFace = Ogre::Vector3(pointFace.m_PosInFace.m_x , pointFace.m_PosInFace.m_y , pointFace.m_PosInFace.m_z);
			m_ThreadBindPoint.push_back(ThreadBindPoint(PosInFace , face));
			Ogre::Vector3 posPass = m_ThreadBindPoint[m_ThreadBindPoint.size()-1].GetPassPointPosition();
			Ogre::Vector3 temp = posPass;
		}
	}

	if(AllFindClosetPoint)
	{
		GFPhysVector3 vs = OgreToGPVec3(m_ThreadBindPoint[0].GetPassPointPosition());
		
		GFPhysVector3 ve = OgreToGPVec3(m_ThreadBindPoint[m_ThreadBindPoint.size()-1].GetPassPointPosition());
		
		float DistES = (vs - ve).Length();

		if(DistES > FLT_EPSILON)
		{

#if(1)
			GFPhysVector3 raysMin = loopFixPoint;
			
			GFPhysVector3 raysMax = loopFixPoint;

			GFPhysAlignedVectorObj<GFPhysVector3> samplePoints;
			
			int numSamplePt = (int)threadPoints.size() - (MainLoopEnd-MainLoopStart);

			GPClamp(numSamplePt , 6 , 20);

			for(int c = 1 ; c < numSamplePt ; c++)
			{
				float w = ((float)c / (float)numSamplePt);

				GFPhysVector3 samplePos = ve + (vs - ve)*w;

				//samplePos = samplePos + (samplePos-loopFixPoint).Normalized()*0.2f;//expand a little
				
				samplePoints.push_back(samplePos);
				
				raysMin.SetMin(samplePos);
				raysMax.SetMax(samplePos);
			}
			RaysSurfaceCallback rayscb(samplePoints , loopFixPoint);

			softShape.TraverseFaceTreeAgainstAABB(&rayscb , raysMin , raysMax);

            for(size_t c = 0 ; c < rayscb.m_ResultBindPoint.size() ; c++)
			{
				MisMedicBindedRope::ThreadBindPoint srcBind = rayscb.m_ResultBindPoint[c];

				if(srcBind.m_AttachFace)
				{
				   srcBind.ReAttach(srcBind.m_AttachFace , srcBind.m_Weights);
				   srcBind.m_ShrinkUndefPos = srcBind.m_OriginUndeformPos = srcBind.m_UndeformedPos;
				   m_ThreadBindPoint.push_back(srcBind);
				}
			}
			m_ThreadBindPoint.push_back(m_ThreadBindPoint[0]);

#else
			GFPhysVector3 dirES = (ve - vs) / DistES;

			GFPhysVector3 direction = (loopFixPoint-vs) - dirES * (loopFixPoint - vs).Dot(dirES);
			direction.Normalize();

			GFPhysVector3 newCenter = (ve + vs)*0.5f + direction*DistES*0.5f;

			GFPhysAlignedVectorObj<GFPhysVector3> RemainTPoints;

			int segment = (MainLoopStart-1 > 0 ? MainLoopStart-1 : 1);

			for(int n = 1 ; n <= segment ; n++)
			{
				RemainTPoints.push_back(ve + (newCenter-ve)*((float)n / (float)segment));
			}

			for(int n = 1 ; n <= segment ; n++)
			{
				RemainTPoints.push_back(newCenter + (vs-newCenter)*((float)n / (float)segment));
			}

			TolerantDist = DistES;

			ThreadNodesTree.Clear();
			
			for(size_t c = 0 ; c < RemainTPoints.size() ; c++)
			{
				GFPhysVector3 NodePos = RemainTPoints[c];

				GFPhysVector3 span(TolerantDist , TolerantDist , TolerantDist);

				GFPhysDBVNode * bvNode = ThreadNodesTree.InsertAABBNode(NodePos-span , NodePos+span);

				bvNode->m_UserData = (void*)c;
			}
		
			GFPhysVectorObj<GFPhysDBVTree*> bvTrees = softShape.GetFaceBVTrees();

			PointFaceDistInRangeCallback RemainClosetCallBack(RemainTPoints , TolerantDist);

			for(size_t t = 0 ; t < bvTrees.size() ; t++)
			{
				GFPhysDBVTree * bvTree = bvTrees[t];
				ThreadNodesTree.CollideWithDBVTree(*bvTree , &RemainClosetCallBack);
			}

			//
			for(size_t c = 0 ; c < RemainTPoints.size() ; c++)
			{
				PointFaceDistInRangeCallback::PointClosetToFace pointFace = RemainClosetCallBack.m_threadPoints[c];
				
				GFPhysSoftBodyFace * face = pointFace.m_ClosetFaces;
				
				if(face == 0)
				{
				   AllFindClosetPoint = false;
				   break;
				}
				
				Ogre::Vector3 PosInFace = Ogre::Vector3(pointFace.m_PosInFace.m_x , pointFace.m_PosInFace.m_y , pointFace.m_PosInFace.m_z);
				
				m_ThreadBindPoint.push_back(ThreadBindPoint(PosInFace , face));

			}
#endif
		}

	}
	
	if(AllFindClosetPoint)
	{
		m_BindedOrgan = &organ;
		BuildBindRopeSegments();
		CalcBindCircleSegLen();
		return true;
	}
	else
	{
		m_ThreadBindPoint.clear();
		return false;
	}
}
void MisMedicBindedRope::SelectInterSectLoopTetras(GFPhysVectorObj<GFPhysSoftBodyTetrahedron*> & intersectTetras)
{
	GFPhysVector3 boxmin(GP_INFINITY, GP_INFINITY, GP_INFINITY);
	GFPhysVector3 boxmax(-GP_INFINITY, -GP_INFINITY, -GP_INFINITY);

	GFPhysVector3 bindPos[100];
	int numBindPos = (int)m_ThreadBindPoint.size();

	for (int t = 0; t < (int)m_ThreadBindPoint.size(); t++)
	{
		bindPos[t] = OgreToGPVec3(m_ThreadBindPoint[t].m_UndeformedPos);

		boxmin.SetMin(bindPos[t]);
		
		boxmax.SetMax(bindPos[t]);
	}

	if (m_BindedOrgan)
	{
		for (int c = 0; c < m_BindedOrgan->m_physbody->GetNumTetrahedron(); c++)
		{
			GFPhysSoftBodyTetrahedron * tetra = m_BindedOrgan->m_physbody->GetTetrahedronAtIndex(c);
			
			GFPhysVector3 tetramin(GP_INFINITY, GP_INFINITY, GP_INFINITY);
			
			GFPhysVector3 tetramax(-GP_INFINITY, -GP_INFINITY, -GP_INFINITY);

			GFPhysVector3 tetraVerts[4];

			for (int n = 0; n < 4; n++)
			{
				tetraVerts[n] = tetra->m_TetraNodes[n]->m_UnDeformedPos;
				tetramin.SetMin(tetraVerts[n]);
				tetramax.SetMax(tetraVerts[n]);
			}
			
			if (TestAabbAgainstAabb2(boxmin, boxmax, tetramin, tetramax))
			{
				//further check
				GFPhysTransform identiytrans;
				identiytrans.SetIdentity();

				GFPhysVector3 closetPointA, closetPointB;

				Real dist = GetConvexsClosetPoint(bindPos , numBindPos , 0.01f,
					                              tetraVerts , 4 , 0.01f,
										          identiytrans , identiytrans,
					                              closetPointA , closetPointB,
					                              0.1f);
				if (dist <= 0.1f)
				{
					intersectTetras.push_back(tetra);
				}
			}
		}
	}
}
//==============================================================
void MisMedicBindedRope::TightenBindedOrgan(float tightpercent)
{
	if(m_BindedOrgan)
	{
		float DstSegLen[200];

		GFPhysVector3 loopshrinkpos[200];

		GFPhysSoftBodyFace * PointsInFace[200];

		float weight[200][3];

		int bindPointCount = m_ThreadBindPoint.size()-1;//last one is coincide first one

		for (int t = 0; t < bindPointCount; t++)
		{
#if(1)
			PointsInFace[t]  = m_ThreadBindPoint[t].m_AttachFace;
			
			weight[t][0] = m_ThreadBindPoint[t].m_Weights[0];
			weight[t][1] = m_ThreadBindPoint[t].m_Weights[1];
			weight[t][2] = m_ThreadBindPoint[t].m_Weights[2];
#else
			PointsInFace[2*t]   = m_ThreadBindPoint[t].m_AttachFace;
			PointsInFace[2*t+1] = m_ThreadBindPoint[t+1].m_AttachFace;

			weight[2*t][0] = m_ThreadBindPoint[t].m_Weights[0];
			weight[2*t][1] = m_ThreadBindPoint[t].m_Weights[1];
			weight[2*t][2] = m_ThreadBindPoint[t].m_Weights[2];

			weight[2*t+1][0] = m_ThreadBindPoint[t+1].m_Weights[0];
			weight[2*t+1][1] = m_ThreadBindPoint[t+1].m_Weights[1];
			weight[2*t+1][2] = m_ThreadBindPoint[t+1].m_Weights[2];
#endif
			m_ThreadBindPoint[t].m_EdgeLen *= tightpercent;
			DstSegLen[t] = m_ThreadBindPoint[t].m_EdgeLen;
		}

		//shrink loop first
		int itorstep = 0;
		
		while (itorstep <= 8)
		{
			for (int c = 0; c < bindPointCount; c++)
			{
				 ThreadBindPoint & bNode0 = m_ThreadBindPoint[c];

				 ThreadBindPoint & bNode1 = m_ThreadBindPoint[(c + 1) % bindPointCount];

				 Real RestLen = m_ThreadBindPoint[c].m_EdgeLen;

				 GFPhysVector3 Diff = OgreToGPVec3(bNode0.m_ShrinkUndefPos) - OgreToGPVec3(bNode1.m_ShrinkUndefPos);

				 Real length = Diff.Length();

				 Real softness = 1.0f;

				 if (length > 0)
				 {
					 GFPhysVector3 Correct = Diff *(length - RestLen) / length;

					 bNode0.m_ShrinkUndefPos -= GPVec3ToOgre(Correct*0.5f * softness);
 
					 bNode1.m_ShrinkUndefPos += GPVec3ToOgre(Correct*0.5f * softness);
				 }
			}
			itorstep++;
		}

		for (int t = 0; t < bindPointCount; t++)
		{
			loopshrinkpos[t] = OgreToGPVec3(m_ThreadBindPoint[t].m_ShrinkUndefPos);
		}
#if(0)
		GoPhysSoftBodyRestShapeModify modify;
		modify.TightenPointPoint(PhysicsWrapper::GetSingleTon().m_dynamicsWorld,
			                     m_BindedOrgan->m_physbody,  
			                     PointsInFace,
			                     weight,
								 loopshrinkpos,
								 bindPointCount);
#else
		GFPhysVectorObj<GFPhysSoftBodyTetrahedron*> intersectTetras;
		
		SelectInterSectLoopTetras(intersectTetras);
		
		GoPhysSoftBodyRestShapeModify modify;
		
		modify.SoftShrinkInMaterialSpace(PhysicsWrapper::GetSingleTon().m_dynamicsWorld,
			                             m_BindedOrgan->m_physbody, GFPhysVector3(0, 0, 1),
			                             intersectTetras , 0.98, 0.1f );

		//modify.SoftShrinkInMaterialSpace(PhysicsWrapper::GetSingleTon().m_dynamicsWorld,
			                            // m_BindedOrgan->m_physbody, GFPhysVector3(0, 0, 1),
			                            // intersectTetras, 0.95, 0.2f);
#endif

		//refresh
		for(size_t t = 0 ; t < m_ThreadBindPoint.size() ; t++)
		{
			m_ThreadBindPoint[t].ReAttach(m_ThreadBindPoint[t].m_AttachFace , m_ThreadBindPoint[t].m_Weights);
			m_ThreadBindPoint[t].m_OriginUndeformPos = m_ThreadBindPoint[t].m_UndeformedPos;
		}

		if(m_KnotPoint.m_AttachFace)
		{
			m_KnotPoint.ReAttach(m_KnotPoint.m_AttachFace , m_KnotPoint.m_Weights);
			m_KnotPoint.m_OriginUndeformPos = m_KnotPoint.m_UndeformedPos;
			m_KnotPoint.m_ShrinkUndefPos = m_KnotPoint.m_UndeformedPos;
		}
		
	}
}
bool MisMedicBindedRope::GetClosetBindedPoint(const GFPhysVector3 & srcPos , MisMedicBindedRope::ThreadBindPoint & bindpoint)
{
	int PointIndex = -1;
	float minDist = FLT_MAX;
	for(size_t c = 0 ; c < m_ThreadBindPoint.size() ; c++)
	{
		MisMedicBindedRope::ThreadBindPoint point = m_ThreadBindPoint[c];
		Ogre::Vector3 temp = point.GetPassPointPosition();

		float dist = (GFPhysVector3(temp.x , temp.y , temp.z)-srcPos).Length();
		if(dist < minDist)
		{
			PointIndex = c;
			minDist = dist;
		}
	}

	if(PointIndex >= 0 && PointIndex < (int)m_ThreadBindPoint.size())
	{
	   bindpoint = m_ThreadBindPoint[PointIndex];
	   return true;
	}
	else
	   return false;
}
//========================================================================================================
float MisMedicBindedRope::GetBindTotalLength(bool materialSpace)
{
	float BindPartLen = 0;

	for(int s = 0 ; s < m_ThreadBindPoint.size()-1 ; s++)
	{
		if(materialSpace)
		{
			Ogre::Vector3 Pos0 = m_ThreadBindPoint[s].m_ShrinkUndefPos;// GetPassPointUndeformedPosition();
			Ogre::Vector3 Pos1 = m_ThreadBindPoint[s + 1].m_ShrinkUndefPos;//GetPassPointUndeformedPosition();
			BindPartLen += (Pos0-Pos1).length();
		}
		else
		{
			Ogre::Vector3 Pos0 = m_ThreadBindPoint[s].GetPassPointPosition();
			Ogre::Vector3 Pos1 = m_ThreadBindPoint[s+1].GetPassPointPosition();
			BindPartLen += (Pos0-Pos1).length();
		}
	}
	return BindPartLen;
}
//========================================================================================================
void MisMedicBindedRope::Update(float deltatime)
{
	if(m_NeedRend == false)
	   return;
	
	/*
	bool needRebudilSeg = false;

	for(int s = 0 ; s < m_ThreadBindPoint.size()-1 ; s++)
	{
		ThreadBindPoint & p0 = m_ThreadBindPoint[s];
		ThreadBindPoint & p1 = m_ThreadBindPoint[s+1];

		GFPhysVector3 t0 = OgreToGPVec3(p0.GetPassPointUndeformedPosition());
		GFPhysVector3 t1 = OgreToGPVec3(p1.GetPassPointUndeformedPosition());
		float undeformLen = (t0-t1).Length();
		
		t0 = OgreToGPVec3(p0.GetPassPointPosition());
		t1 = OgreToGPVec3(p1.GetPassPointPosition());
		float deformlen = (t0-t1).Length();

		if(deformlen / undeformLen > 10.0f)
		{
			p0.m_Valid = p1.m_Valid = false;
			needRebudilSeg = true;
		}
	}
	if(needRebudilSeg)
	{
	   BuildBindRopeSegments();
	}
	*/

	for(size_t s = 0 ; s < m_BindSegments.size() ; s++)
	{
		int SegStartIndex = (m_BindSegments[s] >> 16);

		int SegEndIndex   = (m_BindSegments[s] & 0xFFFF);

		for(int c = SegStartIndex ; c <= SegEndIndex ; c++)
		{
			GFPhysSoftBodyFace * physFace = m_ThreadBindPoint[c].m_AttachFace;
			
			GFPhysVector3 normal = (physFace->m_Nodes[0]->m_Normal * m_ThreadBindPoint[c].m_Weights[0]
				                  + physFace->m_Nodes[1]->m_Normal * m_ThreadBindPoint[c].m_Weights[1]
				                  + physFace->m_Nodes[2]->m_Normal * m_ThreadBindPoint[c].m_Weights[2]).Normalized();
			
			m_SegmentRendPos[s][c - SegStartIndex] = m_ThreadBindPoint[c].GetPassPointPosition() + GPVec3ToOgre(physFace->m_FaceNormal) * 0.06f;
		}
		//smooth
		for (int c = SegStartIndex+1; c <= SegEndIndex-1; c++)
		{
			GFPhysSoftBodyFace * physFace = m_ThreadBindPoint[c].m_AttachFace;

			m_SegmentRendPos[s][c - SegStartIndex] = (m_SegmentRendPos[s][c - SegStartIndex - 1] + m_SegmentRendPos[s][c - SegStartIndex + 1])*0.5f;
		}
	}
	
	m_RendObject.UpdateRendSegments(m_SegmentRendPos , 0.03f);

	if(m_KnotNode)
	{
		GFPhysSoftBodyFace * FaceWithKnot = m_KnotPoint.m_AttachFace;
		
		float * FaceWeightsForKnot = m_KnotPoint.m_Weights;

		GFPhysVector3 position = FaceWithKnot->m_Nodes[0]->m_CurrPosition * FaceWeightsForKnot[0] +
							     FaceWithKnot->m_Nodes[1]->m_CurrPosition * FaceWeightsForKnot[1] +
								 FaceWithKnot->m_Nodes[2]->m_CurrPosition * FaceWeightsForKnot[2];

		m_KnotNode->setPosition(GPVec3ToOgre(position - FaceWithKnot->m_FaceNormal * 0.02));

		if(m_ConnectState == MisMedicBindedRope::CS_UNCONNECTED)
		{
		   m_KnotNode->setDirection(GPVec3ToOgre(-FaceWithKnot->m_FaceNormal) , Ogre::Node::TransformSpace::TS_WORLD);
		}
	}
	
}
//========================================================================================================
class PointSoftFaceClosetCallBack : public GFPhysNodeOverlapCallback
{
public:
	PointSoftFaceClosetCallBack(const GFPhysVector3 & point)
	{
		m_PointPos = point;
		closetDist = FLT_MAX;
		m_ClosetFace = 0;
	}

	void ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData)
	{
		GFPhysSoftBodyFace * face = (GFPhysSoftBodyFace*)UserData;
		
		GFPhysVector3 closetPt = ClosestPtPointTriangle(m_PointPos , 
													    face->m_Nodes[0]->m_UnDeformedPos , 
													    face->m_Nodes[1]->m_UnDeformedPos , 
													    face->m_Nodes[2]->m_UnDeformedPos);

		float dist = (closetPt-m_PointPos).Length();
		
		if(dist < closetDist)
		{
			closetDist = dist;
			m_ResultClosetPt = closetPt;
			m_ClosetFace = face;
		}

	}
	float closetDist;

	GFPhysSoftBodyFace * m_ClosetFace;

	GFPhysVector3 m_PointPos;

	GFPhysVector3 m_ResultClosetPt;
};

GFPhysSoftBodyFace * ClosetPointToSoftBodyFaces(const GFPhysVector3 & UndeformPos , 
												GFPhysSoftBody * sb ,
												float threhold ,
												float weights[3]
												)
{
	GFPhysVector3 margin(threhold , threhold , threhold);
	
	PointSoftFaceClosetCallBack closetCallBack(UndeformPos);

	GFPhysVectorObj<GFPhysDBVTree*> bvtrees = sb->GetSoftBodyShape().GetFaceBVTrees(false);//TraverseFaceTreeAgainstAABB(&closetCallBack , UndeformPos-margin , UndeformPos+margin);

	for(size_t t = 0 ; t < bvtrees.size() ; t++)
	{
		bvtrees[t]->TraverseTreeAgainstAABB(&closetCallBack , UndeformPos-margin , UndeformPos+margin);
	}

	if(closetCallBack.m_ClosetFace != 0 )
	{
		CalcBaryCentric(closetCallBack.m_ClosetFace->m_Nodes[0]->m_UnDeformedPos ,
						closetCallBack.m_ClosetFace->m_Nodes[1]->m_UnDeformedPos ,
						closetCallBack.m_ClosetFace->m_Nodes[2]->m_UnDeformedPos , 
						closetCallBack.m_ResultClosetPt ,
						weights[0] , 
						weights[1] , 
						weights[2]);
		
		return closetCallBack.m_ClosetFace;
	}
	else
		return 0;
}
//===================================================================================================
void MisMedicBindedRope::OnFaceSplittedByCut(std::vector<FaceSplitByCut> & facesSplitted ,GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & facesDeleted, MisMedicOrgan_Ordinary * organ)
{
	//for(size_t b = 0 ; b < m_ThreadBindPoint.size() ; b++)
	

	//some face may be delete or split so reattach rope some point may become invalid
	/*
	for(size_t b = 0 ; b < m_ThreadBindPoint.size() ; b++)
	{
		if(m_ThreadBindPoint[b].m_Valid == true)
		{
			float attachWeights[3];

			GFPhysSoftBodyFace * newAttachFace = ClosetPointToSoftBodyFaces(OgreToGPVec3(m_ThreadBindPoint[b].m_UndeformedPos) , 
																			organ->m_physbody ,
																			0.1f ,
																			attachWeights);

			if(newAttachFace)
			{
				m_ThreadBindPoint[b].ReAttach(newAttachFace , attachWeights);
			}
			else
			{
				m_ThreadBindPoint[b].m_Valid = false;
			}
		}
	}
	*/
}
//===================================================================================================
void MisMedicBindedRope::OnCutByToolFinish()
{
	std::vector<ThreadBindPoint>::iterator itor = m_ThreadBindPoint.begin();

	while(itor != m_ThreadBindPoint.end())
	{
		ThreadBindPoint & bindPoint = (*itor);

		float attachWeights[3];

		GFPhysSoftBodyFace * newAttachFace = ClosetPointToSoftBodyFaces(OgreToGPVec3(bindPoint.m_OriginUndeformPos) , 
			m_BindedOrgan->m_physbody ,
			0.1f ,
			attachWeights);

		if(newAttachFace)
		{
			bindPoint.ReAttach(newAttachFace , attachWeights);
			itor++;
		}
		else
		{
			itor = m_ThreadBindPoint.erase(itor);
		}
	}
	if(m_KnotNode)
	{
		float attachWeights[3];

		GFPhysSoftBodyFace * newAttachFace = ClosetPointToSoftBodyFaces(OgreToGPVec3(m_KnotPoint.m_OriginUndeformPos) , 
			m_BindedOrgan->m_physbody ,
			0.1f ,
			attachWeights);

		if(newAttachFace)
		{
			m_KnotPoint.ReAttach(newAttachFace , attachWeights);
		}
		else
		{
			Ogre::SceneManager * sceneMgr = MXOgreWrapper::Get()->GetDefaultSceneManger();

			if(sceneMgr)
			{
				if(m_KnotEnitity)
				{
					m_KnotEnitity->detachFromParent();
					sceneMgr->destroyEntity(m_KnotEnitity);
					m_KnotEnitity = NULL;
				}
				sceneMgr->getRootSceneNode()->removeChild(m_KnotNode);
				sceneMgr->destroySceneNode(m_KnotNode);
			}

			m_KnotNode = NULL;
		}

	}

	BuildBindRopeSegments();
}
void MisMedicBindedRope::OnRemoveTetrahedron(GFPhysSoftBodyTetrahedron * TetraRemved)
{
	//
	/*
	bool needrebuildSegment = false;

	for(int b = 0 ; b < (int)m_ThreadBindPoint.size()-1; b++)
	{
		ThreadBindPoint & b0 = m_ThreadBindPoint[b];

		ThreadBindPoint & b1 = m_ThreadBindPoint[b+1];

		if(b0.m_Valid && b1.m_Valid)
		{
			GFPhysVector3 pos0 = OgreToGPVec3(b0.GetPassPointPosition());
			
			GFPhysVector3 pos1 = OgreToGPVec3(b1.GetPassPointPosition());

			//GFPhysVector3 directon = (pos1-pos0).Normalize();
			
			GFPhysVector3 TetraVerts[4];
			TetraVerts[0] = TetraRemved->m_TetraNodes[0]->m_CurrPosition;
			TetraVerts[1] = TetraRemved->m_TetraNodes[1]->m_CurrPosition;
			TetraVerts[2] = TetraRemved->m_TetraNodes[2]->m_CurrPosition;
			TetraVerts[3] = TetraRemved->m_TetraNodes[3]->m_CurrPosition;

			bool instersect = LineSegmentTetraIntersect(pos0 , pos1 , TetraVerts);
			if(instersect)
			{
			   b0.m_Valid = b1.m_Valid = false;
			   needrebuildSegment = true;
			}
		}
	}
	//@@ step2 build segments of the bind rope
	if(needrebuildSegment)
	   BuildBindRopeSegments();
   */
}
//==============================================================================================================
/*
void MisMedicBindedRope::OnRemoveCuttedTetrahedron(const GFPhysVectorObj<GFPhysSoftBodyTetrahedron*> & removedtetras)
{
	for(int b = 0 ; b < (int)m_ThreadBindPoint.size()-1; b++)
	{
		ThreadBindPoint & b0 = m_ThreadBindPoint[b];

		ThreadBindPoint & b1 = m_ThreadBindPoint[b+1];

		if(b0.m_Valid && b1.m_Valid)
		{
			GFPhysVector3 pos0 = OgreToGPVec3(b0.GetPassPointPosition());
			GFPhysVector3 pos1 = OgreToGPVec3(b1.GetPassPointPosition());

			GFPhysVector3 directon = (pos1-pos0).Normalize();
			//pos1 += directon*0.02f;
			//pos0 -= directon*0.02f;
			for(size_t t = 0 ; t < removedtetras.size() ; t++)
			{
				GFPhysSoftBodyTetrahedron * TetraRemved = removedtetras[t];
				
				GFPhysVector3 TetraVerts[4];
				TetraVerts[0] = TetraRemved->m_TetraNodes[0]->m_CurrPosition;
				TetraVerts[1] = TetraRemved->m_TetraNodes[1]->m_CurrPosition;
				TetraVerts[2] = TetraRemved->m_TetraNodes[2]->m_CurrPosition;
				TetraVerts[3] = TetraRemved->m_TetraNodes[3]->m_CurrPosition;

				bool instersect = LineSegmentTetraIntersect(pos0 , pos1 , TetraVerts);
				if(instersect)
				{
					b0.m_Valid = b1.m_Valid = false;
				}
			}
		}
	}
	//@@ step2 build segments of the bind rope
	BuildBindRopeSegments();
}
*/
//==============================================================================================================
void MisMedicBindedRope::BuildBindRopeSegments()
{
	m_BindSegments.clear();

	int NumPoint = m_ThreadBindPoint.size();

	int c = 0;

	while(c < NumPoint)
	{
		GFPhysSoftBodyNode * InitNode = m_ThreadBindPoint[c].m_AttachFace->m_Nodes[0];
			
		PhysNode_Data & InitPhysData = m_BindedOrgan->GetPhysNodeData(InitNode);
			
		if(InitPhysData.m_HasError == true)
		   c++;
		else
		{
		   int InitPartID = InitNode->m_SepPartID; //InitPhysData.m_SepPartID;

		   int SegStartIndex = c++;//record segment start index

		   while(c < NumPoint)
		   {
			   GFPhysSoftBodyNode * CurrNode = m_ThreadBindPoint[c].m_AttachFace->m_Nodes[0];
					
			   //PhysNode_Data & CurrphysData = m_BindedOrgan->GetPhysNodeData(CurrNode);
					
			   int CurrPartID = CurrNode->m_SepPartID;// CurrphysData.m_SepPartID;

			   if((CurrPartID != InitPartID))// || (m_ThreadBindPoint[c].m_Valid == false))
			   {
				  break;
			   }
			   else
			   {
				  c++;
			   }
		   }

		   int SegEndIndex = c-1;
			
		   if(SegEndIndex-SegStartIndex > 0)
		   {
			  m_BindSegments.push_back((SegStartIndex << 16) | (SegEndIndex & 0xFFFF));
		   }
	   }
	}

	m_SegmentRendPos.clear();

	m_SegmentRendPos.resize(m_BindSegments.size());

	for(size_t s = 0 ; s < m_BindSegments.size() ; s++)
	{
		int SegStartIndex = (m_BindSegments[s] >> 16);

		int SegEndIndex   = (m_BindSegments[s] & 0xFFFF);

		m_SegmentRendPos[s].resize(SegEndIndex-SegStartIndex+1);
	}
}
//===============================================================================================================
void MisMedicBindedRope::CalcBindCircleSegLen()
{
	for (size_t c = 0; c < m_ThreadBindPoint.size(); c++)
	{
		ThreadBindPoint & p0 = m_ThreadBindPoint[c];
		ThreadBindPoint & p1 = m_ThreadBindPoint[(c + 1) % m_ThreadBindPoint.size()];
		p0.m_EdgeLen = (p0.GetPassPointUndeformedPosition() - p1.GetPassPointUndeformedPosition()).length();
	}
}
//===========================================================================================================
int MisMedicBindedRope::GetNumBindPoints()
{
	return m_ThreadBindPoint.size();
}
//===========================================================================================================
const MisMedicBindedRope::ThreadBindPoint & MisMedicBindedRope::GetBindPoint(int index)
{
	return m_ThreadBindPoint[index];
}

void MisMedicBindedRope::GetBindPoints(std::vector<ThreadBindPoint>& bindPoints)
{
	for(std::size_t p = 0;p < m_ThreadBindPoint.size();++p)
		bindPoints.push_back(m_ThreadBindPoint[p]);
}

bool  MisMedicBindedRope::GetApproximateLoopPlane_MaterialSpace(GFPhysVector3 & planeNormal , GFPhysVector3 & planePoint)
{
	int NumPoint = m_ThreadBindPoint.size();
	if(NumPoint > 3)
	{
		int SampleIndex0 = (NumPoint / 3)-1;
		int SampleIndex1 = 2*(NumPoint / 3)-1;
		int SampleIndex2 = NumPoint-1;

		GFPhysVector3 samplePoint0 = OgreToGPVec3(m_ThreadBindPoint[SampleIndex0].m_UndeformedPos);
		GFPhysVector3 samplePoint1 = OgreToGPVec3(m_ThreadBindPoint[SampleIndex1].m_UndeformedPos);
		GFPhysVector3 samplePoint2 = OgreToGPVec3(m_ThreadBindPoint[SampleIndex2].m_UndeformedPos);

		planeNormal = (samplePoint1-samplePoint0).Cross(samplePoint2-samplePoint0);
		planeNormal.Normalize();

		planePoint = samplePoint0;
		return true;
	}
	else
		return false;
}

void MisMedicBindedRope::CreateKnotNode(Ogre::SceneManager * sceneMgr , GFPhysSoftBodyFace *pAttachedFace , float weights[3])
{
	m_KnotNode = sceneMgr->getRootSceneNode()->createChildSceneNode("Knot" + Ogre::StringConverter::toString(m_BoundRopeID));
	m_KnotEnitity = sceneMgr->createEntity("shengjie"  + Ogre::StringConverter::toString(m_BoundRopeID) , "shengjie.mesh");
	Ogre::MaterialPtr matr = Ogre::MaterialManager::getSingleton().getByName("RopeKnot");
	m_KnotEnitity->setMaterial(matr);
	m_KnotNode->attachObject(m_KnotEnitity);
	m_KnotNode->setScale(2.2,2.2,2.2);

	//m_pFaceWithKnot = pAttachedFace;
	//std::copy(weights ,  weights + 3 , m_FaceWeightsForKnot);

	GFPhysVector3 position = pAttachedFace->m_Nodes[0]->m_CurrPosition * weights[0] +
							 pAttachedFace->m_Nodes[1]->m_CurrPosition * weights[1] +
						     pAttachedFace->m_Nodes[2]->m_CurrPosition * weights[2];

	m_KnotNode->setPosition(GPVec3ToOgre(position));

	m_KnotPoint.m_AttachFace = pAttachedFace;
	m_KnotPoint.m_Weights[0] = weights[0];
	m_KnotPoint.m_Weights[1] = weights[1];
	m_KnotPoint.m_Weights[2] = weights[2];
	m_KnotPoint.m_UndeformedPos = GPVec3ToOgre(pAttachedFace->m_Nodes[0]->m_UnDeformedPos * weights[0] +
											   pAttachedFace->m_Nodes[1]->m_UnDeformedPos * weights[1] +
											   pAttachedFace->m_Nodes[2]->m_UnDeformedPos * weights[2]);
	m_KnotPoint.m_OriginUndeformPos = m_KnotPoint.m_UndeformedPos;

	m_ConnectState = MisMedicBindedRope::CS_CONNECTED;
}

void MisMedicBindedRope::SetKnotNodeDir(const Ogre::Vector3 & dir)
{
	if(m_KnotNode)
		m_KnotNode->setDirection(dir , Ogre::Node::TransformSpace::TS_WORLD);
}

MisMedicOrgan_Ordinary* MisMedicBindedRope::GetBindedOrgan()
{
	return m_BindedOrgan;
}