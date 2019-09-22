#include "MisMedicOrganShapeModify.h"
#include "MisMedicOrganOrdinary.h"
#include "Math/GoPhysSIMDMath.h"
SphereCutSurface::SphereCutSurface(MisMedicOrgan_Ordinary * organ , 
								   const GFPhysVector3 & centerPos , 
								   float radius ,
								   float threshold , 
								   bool  indeformspace)
{
	m_IndeformSpace = indeformspace;
	m_CutRadius = radius;
	m_CutThresHold = threshold;//m_CenterNode = centerNode;
	m_CenterPos = centerPos;

	//AroundSoftNode CenNodeData;
	//CenNodeData.m_CenterNode = true;
	//m_NodesAround.insert(GFPhysHashNode(m_CenterNode) , CenNodeData);

	//std::vector<GFPhysSoftBodyTetrahedron*> TetrasInrange;
	//organ->CollectTetrasAroundPoint(centerNode->m_UnDeformedPos ,  radius , TetrasInrange);

	/*
	for(size_t t = 0 ; t < TetrasAround.size() ; t++)
	{
		GFPhysSoftBodyTetrahedron * tetra = TetrasAround[t];
		if(tetra->m_TetraNodes[0] == centerNode
		 ||tetra->m_TetraNodes[1] == centerNode
		 ||tetra->m_TetraNodes[2] == centerNode
		 ||tetra->m_TetraNodes[3] == centerNode)
		{
			for(int n = 0 ; n < 4 ; n++)
			{
				GFPhysSoftBodyNode * nNode = tetra->m_TetraNodes[n];
				if((nNode != centerNode) && (m_NodesAround.find(nNode) == 0))
				{
					AroundSoftNode nData;
					float dist = (nNode->m_UnDeformedPos-m_CenterNode->m_UnDeformedPos).Length();
					if(dist < m_CutRadius + m_CutRadius*0.3f)
					   nData.m_LieInCutPlane = true;

					m_NodesAround.insert(GFPhysHashNode(nNode) , nData);
				}
			}
		}
	}
	*/
}

bool SphereCutSurface::IsIntersectByCutPlane(GFPhysSoftBodyTetrahedron * tetra ,
												  const GFPhysVector3 & NodeVert0 , 
												  const GFPhysVector3 & NodeVert1 , 
												  const GFPhysVector3 & NodeVert2 ,
												  const GFPhysVector3 & NodeVert3 , 
												  bool & needCheckDecom)
{
	needCheckDecom = true;
	return true;//temp need optimize
}

bool RaySphereIntersect(const GFPhysVector3 & p, const GFPhysVector3 & q, const GFPhysVector3 & center, float r, float &t, GFPhysVector3 & intersect)
{
	GFPhysVector3 d = q - p;
	float dlen = d.Length();
	if (dlen < GP_EPSILON)
		return false;
	d /= dlen;//normalize d
	GFPhysVector3 m = p - center;
	float b = m.Dot(d);
	float c = m.Dot(m) - r*r;
	if (c > 0 && b > 0)
		return false;
	float discr = b*b - c;
	if (discr < 0)
		return false;

	float t0 = -b - sqrtf(discr);
	float t1 = -b + sqrtf(discr);

	if (t0 >= 0 && t0 <= dlen)
	{
		t = t0;
	}
	else if (t1 >= 0 && t1 <= dlen)
	{
		t = t1;
	}
	else
		return false;
	
	intersect = p + d*t;
	
	t /= dlen;//transform to weight
	return true;
}
bool SphereCutSurface::GetEdgeCutPoint(GFPhysSoftBodyNode * pNode0 , GFPhysSoftBodyNode * pNode1 , const GFPhysVector3 & node0 , const GFPhysVector3 & node1 , GFPhysVector3 & clipPoint , Real & cutWeight)
{
	Real dist0 = ((m_IndeformSpace == true ? pNode0->m_CurrPosition : pNode0->m_UnDeformedPos) - m_CenterPos).Length();
	Real dist1 = ((m_IndeformSpace == true ? pNode1->m_CurrPosition : pNode1->m_UnDeformedPos) - m_CenterPos).Length();
		
	 if(dist0 <= m_CutRadius && dist1 <= m_CutRadius)
	    return false;
	 else if(dist0 >= m_CutRadius && dist1 >= m_CutRadius)
		return false;
	 else
	 {
		if(dist0 < dist1)
		   cutWeight = (m_CutRadius-dist0) / (dist1-dist0);
		else
		   cutWeight = (dist0-m_CutRadius) / (dist0-dist1);

		GPClamp(cutWeight , 0.0f , 1.0f);

		clipPoint = (m_IndeformSpace == true ? pNode0->m_CurrPosition : pNode0->m_UnDeformedPos) * (1 - cutWeight) + (m_IndeformSpace == true ? pNode1->m_CurrPosition : pNode1->m_UnDeformedPos) * cutWeight;

		bool intersect = RaySphereIntersect((m_IndeformSpace == true ? pNode0->m_CurrPosition : pNode0->m_UnDeformedPos),
			                                (m_IndeformSpace == true ? pNode1->m_CurrPosition : pNode1->m_UnDeformedPos),
			                                 m_CenterPos, m_CutRadius, cutWeight, clipPoint);

		return intersect;
		//return true;
	 }

	 return false;

}

Real SphereCutSurface::NodeDist(GFPhysSoftBodyNode * pNode , const GFPhysVector3 & node)
{
	Real dist = ((m_IndeformSpace == true ? pNode->m_CurrPosition : pNode->m_UnDeformedPos) - m_CenterPos).Length();

	if(fabsf(dist-m_CutRadius) < m_CutRadius*0.001f)//mark as lie in surface
	   return 0;
	else if(dist < m_CutRadius)
	   return 1;
	else
	   return -1;
}

void SphereCutSurface::ExtractInnerTetras(std::vector<GFPhysSoftBodyTetrahedron *> & TissueToCheck)
{
	for(size_t c = 0 ; c < TissueToCheck.size() ; c++)
	{
		GFPhysSoftBodyTetrahedron * tetra = TissueToCheck[c];

		int numInnerVert  = 0;

		int numOutterVert = 0;
		for(int n = 0 ; n < 4 ; n++)
		{
			GFPhysSoftBodyNode * tNode = tetra->m_TetraNodes[n];
			GFPhysVector3 unUsed;

			Real dist = SphereCutSurface::NodeDist(tNode , unUsed);

			if(dist > 0)
			{
			   numInnerVert++;
			}
			else if(dist < 0)
			{
			   numOutterVert++;
			}
		}

		if(numInnerVert == 0 || numOutterVert > 0)
		{
		   TissueToCheck[c] = 0;//unmark
		}
	}
}

EnvolopeSpaceCutFace::EnvolopeSpaceCutFace()
{

}

void EnvolopeSpaceCutFace::addEnvolopePlane(const GFPhysVector3 & point , const GFPhysVector3 & normal)
{
	EnvolopePlane planeEnv;
	planeEnv.m_Point  = point;
	planeEnv.m_Normal = normal;

	m_Planes.push_back(planeEnv);
	m_PlaneDist0.resize(m_Planes.size());
	m_PlaneDist1.resize(m_Planes.size());
}
void EnvolopeSpaceCutFace::addEnvolopePlane(const GFPhysVector3 & point0, const GFPhysVector3 & point1, const GFPhysVector3 & point2, bool iscapped)
{
	EnvolopePlane planeEnv;
	planeEnv.m_Point  = point0;
	planeEnv.m_Normal = (point1-point0).Cross(point2-point0).Normalized();
	planeEnv.m_IsCapped = iscapped;

	m_Planes.push_back(planeEnv);
	m_PlaneDist0.resize(m_Planes.size());
	m_PlaneDist1.resize(m_Planes.size());
}

bool EnvolopeSpaceCutFace::IsInSideEnvelope(const GFPhysVector3 & pos)
{
	bool isinside = true;

	GFPhysAlignedVectorObj<EnvolopePlane> CutPlanes;
	for (int c = 0; c < (int)m_Planes.size(); c++)
	{
		 //if (m_Planes[c].m_IsCapped)
		 {
			 CutPlanes.push_back(m_Planes[c]);
		 }
	}

	for (int c = 0; c < (int)CutPlanes.size(); c++)
	{
		m_PlaneDist0[c] = (pos - CutPlanes[c].m_Point).Dot(CutPlanes[c].m_Normal);

		 if(m_PlaneDist0[c] <= 0)//out
		 {
		    isinside = false;
		 }
	}
	return isinside;
}
bool EnvolopeSpaceCutFace::IsIntersectByCutPlane( GFPhysSoftBodyTetrahedron * tetra , 
												 const GFPhysVector3 & NodeVert0 ,
												 const GFPhysVector3 & NodeVert1 , 
												 const GFPhysVector3 & NodeVert2 , 
												 const GFPhysVector3 & NodeVert3 ,
												 bool & needCheckDecom)
{
	needCheckDecom = true;
	return true;
}

bool EnvolopeSpaceCutFace::GetEdgeCutPoint(GFPhysSoftBodyNode * pNode0 , GFPhysSoftBodyNode * pNode1 ,const GFPhysVector3 & node0 , const GFPhysVector3 & node1 , GFPhysVector3 & clipPoint , Real & cutWeight)
{
	GFPhysVector3 pos0 = pNode0->m_CurrPosition;
	GFPhysVector3 pos1 = pNode1->m_CurrPosition;

	bool point0InSide = true;
	bool point1InSide = true;

	GFPhysAlignedVectorObj<EnvolopePlane> CutPlanes;
	for (int c = 0; c < (int)m_Planes.size(); c++)
	{
		//if (m_Planes[c].m_IsCapped)
		{
			CutPlanes.push_back(m_Planes[c]);
		}
	}

	for (int c = 0; c < (int)CutPlanes.size(); c++)
	{
		 m_PlaneDist0[c] = (pos0 - CutPlanes[c].m_Point).Dot(CutPlanes[c].m_Normal);

		 m_PlaneDist1[c] = (pos1 - CutPlanes[c].m_Point).Dot(CutPlanes[c].m_Normal);

		 if (m_PlaneDist0[c] <= 0)//out
		 {
			 point0InSide = false;
		 }
		 if (m_PlaneDist1[c] <= 0)//out
		 {
			 point1InSide = false;
		 }
	}

	if(point0InSide == point1InSide)//not intersect 
	   return false;

	cutWeight = FLT_MAX;
	
	if(point0InSide)
	{
		for (int c = 0; c < (int)CutPlanes.size(); c++)
		{
			float w = 0;
			
			w = m_PlaneDist0[c] / (m_PlaneDist0[c]-m_PlaneDist1[c]);
			
			if(w >= 0 && w < cutWeight)
			{
			   cutWeight = w;
			}
		}

		if(cutWeight > 1)
		{
			int i = 0;
			int j = i+1;
		}
		GPClamp(cutWeight , 0.0f , 1.0f);
		
		clipPoint = pos0 * (1-cutWeight) + pos1 * cutWeight;
		//cutWeight = 0.5f;
		return true;
	}
	else if(point1InSide)
	{
		for (int c = 0; c < (int)CutPlanes.size(); c++)
		{
			float w = 0;

			w = m_PlaneDist1[c] / (m_PlaneDist1[c]-m_PlaneDist0[c]);

			if(w >= 0 && w < cutWeight)
			{
			   cutWeight = w;
			}
		}
		//
		if(cutWeight > 1)
		{
			int i = 0;
			int j = i+1;
		}
		GPClamp(cutWeight , 0.0f , 1.0f);
		//cutWeight = 0.5f;
		clipPoint = pos1 * (1-cutWeight) + pos0 * cutWeight;

		cutWeight = 1-cutWeight;
		return true;
	}
	else
	    return false;
}

Real EnvolopeSpaceCutFace::NodeDist(GFPhysSoftBodyNode * pNode , const GFPhysVector3 & node)
{
	GFPhysVector3 nPos = pNode->m_CurrPosition;

	float minPosDist = FLT_MAX;
	
	float minNegDist = 0;
	
	bool isOutSide = false;
	for (size_t c = 0 ; c < m_Planes.size() ; c++)
	{
		float dist = (nPos - m_Planes[c].m_Point).Dot(m_Planes[c].m_Normal);

		if(dist >= 0)//inside this plane
		{
		   if(dist < minPosDist)
			  minPosDist = dist;
		}
		else//out side this plane
		{
		   if(dist < minNegDist)
		   {
			  minNegDist = dist;
			  isOutSide = true;
		   }
		}
	}

	if(isOutSide == true)//none negative point total inside
	{
	   return minPosDist;
	}
	else//out side
	{
	   return minNegDist;
	}
}

CutQuadWithThickNess::CutQuadWithThickNess(const GFPhysVector3 &  u0 , const GFPhysVector3 &  u1 , const GFPhysVector3 &  d0 , const GFPhysVector3 &  d1 , float thickness)
{
	GFPhysVector3 thickDir = (d0-u0).Cross(u1-u0).Normalized();
	m_convexPos[0] = u0 - thickDir*thickness;
	m_convexPos[1] = u1 - thickDir*thickness;
	m_convexPos[2] = u0 + thickDir*thickness;
	m_convexPos[3] = u1 + thickDir*thickness;

	m_convexPos[4] = d0 - thickDir*thickness;
	m_convexPos[5] = d1 - thickDir*thickness;
	m_convexPos[6] = d0 + thickDir*thickness;
	m_convexPos[7] = d1 + thickDir*thickness;

	addEnvolopePlane(m_convexPos[0] , m_convexPos[1] , m_convexPos[2] , false);//because semiinfinite so top plane uncapped
	addEnvolopePlane(m_convexPos[4], m_convexPos[6], m_convexPos[5], false);//bottom plane uncapped

	addEnvolopePlane(m_convexPos[4] , m_convexPos[0] , m_convexPos[2], true);
	addEnvolopePlane(m_convexPos[3], m_convexPos[1], m_convexPos[5], true);

	addEnvolopePlane(m_convexPos[2], m_convexPos[3], m_convexPos[6], true);
	addEnvolopePlane(m_convexPos[1], m_convexPos[0], m_convexPos[5], true);

	m_CutMin = m_CutMax = m_convexPos[0];
	
	for(int c = 1 ; c < 8 ; c++)
	{
		GPSIMDVec3SelfSetMin(m_CutMin , m_convexPos[c]);
		GPSIMDVec3SelfSetMax(m_CutMax , m_convexPos[c]);
	}
	m_CutMin -= GFPhysVector3(0.1f , 0.1f , 0.1f);
	m_CutMax += GFPhysVector3(0.1f , 0.1f , 0.1f);


	m_UpperPlanePt = m_convexPos[0];
	m_UpperPlaneNormal = (m_convexPos[1]-m_convexPos[0]).Cross(m_convexPos[2]-m_convexPos[0]).Normalized();
	

	m_DownPlanePt = m_convexPos[4];
	m_DownPlaneNormal = (m_convexPos[6]-m_convexPos[4]).Cross(m_convexPos[5]-m_convexPos[4]).Normalized();
}
//=======================================================================================================
bool CutQuadWithThickNess::IsIntersectByCutPlane(GFPhysSoftBodyTetrahedron * tetra,
	const GFPhysVector3 & NodeVert0,
	const GFPhysVector3 & NodeVert1,
	const GFPhysVector3 & NodeVert2,
	const GFPhysVector3 & NodeVert3,
	bool & needCheckDecom)

{
#if(1)
	needCheckDecom = true;

	GFPhysVector3 tetraMin, tetraMax;

	GFPhysVector3 tetraVertex[4];
	tetraVertex[0] = tetra->m_TetraNodes[0]->m_CurrPosition;
	tetraVertex[1] = tetra->m_TetraNodes[1]->m_CurrPosition;
	tetraVertex[2] = tetra->m_TetraNodes[2]->m_CurrPosition;
	tetraVertex[3] = tetra->m_TetraNodes[3]->m_CurrPosition;

	tetraMin = tetraMax = tetraVertex[0];

	GPSIMDVec3SelfSetMin(tetraMin, tetraVertex[1]);
	GPSIMDVec3SelfSetMax(tetraMax, tetraVertex[1]);

	GPSIMDVec3SelfSetMin(tetraMin, tetraVertex[2]);
	GPSIMDVec3SelfSetMax(tetraMax, tetraVertex[2]);

	GPSIMDVec3SelfSetMin(tetraMin, tetraVertex[3]);
	GPSIMDVec3SelfSetMax(tetraMax, tetraVertex[3]);

	bool aabbOverlap = TestAabbAgainstAabb2(tetraMin, tetraMax, m_CutMin, m_CutMax);

	if (aabbOverlap)
	{
		GFPhysVector3 closetPointA;

		GFPhysVector3 closetPointB;

		GFPhysTransform IdentityTrans;
		IdentityTrans.SetIdentity();

		Real depth = GetConvexsClosetPoint(m_convexPos, 8, 0,
			tetraVertex, 4, 0,
			IdentityTrans,
			IdentityTrans,
			closetPointA,
			closetPointB,
			0.5f);
		if (depth < 0)//overlap
		{
			m_IntersectTetras.insert(tetra);
		}
		if (depth < 0.001f)
			return true;
	}
	return false;
#else
	needCheckDecom = true;

	GFPhysVector3 tetraNodePos[4];
	tetraNodePos[0] = NodeVert0;
	tetraNodePos[1] = NodeVert1;
	tetraNodePos[2] = NodeVert2;
	tetraNodePos[3] = NodeVert3;
	bool isNodeInSide[4] = { true, true, true, true };

	for (int n = 0; n < 4; n++)
	{
		for (int c = 0; c < (int)m_Planes.size(); c++)
		{
			Real dist = (tetraNodePos[n] - m_Planes[c].m_Point).Dot(m_Planes[c].m_Normal);

			if (dist <= 0)//out
			{
				isNodeInSide[n] = false;
				break;
			}
		}
	}
	if (isNodeInSide[0] || isNodeInSide[1] || isNodeInSide[2] || isNodeInSide[3])
	{
		m_IntersectTetras.insert(tetra);
		return true;
    }
	else
		return false;
#endif
}
//=======================================================================================================
CutQuadWithThickNess2::CutQuadWithThickNess2(const GFPhysVector3 & u0 , const GFPhysVector3 & u1 , const GFPhysVector3 & d0 , const GFPhysVector3 & d1 , float thickness)
{
	GFPhysVector3 convexPos[8];

	GFPhysVector3 thickDir = (d0-u0).Cross(u1-u0).Normalized();
	convexPos[0] = u0 - thickDir*thickness;
	convexPos[1] = u1 - thickDir*thickness;
	convexPos[2] = u0 + thickDir*thickness;
	convexPos[3] = u1 + thickDir*thickness;

	convexPos[4] = d0 - thickDir*thickness;
	convexPos[5] = d1 - thickDir*thickness;
	convexPos[6] = d0 + thickDir*thickness;
	convexPos[7] = d1 + thickDir*thickness;

	m_Triangles.push_back(GFPhysPresetCutPolygonSurface::ClipTriangle(convexPos[2] , convexPos[3] , convexPos[6]));
	m_Triangles.push_back(GFPhysPresetCutPolygonSurface::ClipTriangle(convexPos[6] , convexPos[3] , convexPos[7]));	

	m_Triangles.push_back(GFPhysPresetCutPolygonSurface::ClipTriangle(convexPos[0] , convexPos[2] , convexPos[4]));
	m_Triangles.push_back(GFPhysPresetCutPolygonSurface::ClipTriangle(convexPos[4] , convexPos[2] , convexPos[6]));	

	m_Triangles.push_back(GFPhysPresetCutPolygonSurface::ClipTriangle(convexPos[1] , convexPos[0] , convexPos[5]));
	m_Triangles.push_back(GFPhysPresetCutPolygonSurface::ClipTriangle(convexPos[5] , convexPos[0] , convexPos[4]));	

}