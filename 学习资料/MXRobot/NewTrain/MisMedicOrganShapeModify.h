#ifndef _MISMEDICORGANSHAPEMODIFY_
#define _MISMEDICORGANSHAPEMODIFY_
#include "dynamic/GoPhysDynamicLib.h"
#include <set>
using namespace GoPhys;
class MisMedicOrgan_Ordinary;

class SphereCutSurface : public GFPhysISoftBodyCutSurface
{
public:
	/*class AroundSoftNode
	{
	public:
		AroundSoftNode()
		{
			m_CenterNode = false;
			m_LieInCutPlane = false;
			m_InSideCutPlane = false;
		}
		bool m_CenterNode;
		bool m_LieInCutPlane;
		bool m_InSideCutPlane;
	};*/
	SphereCutSurface(MisMedicOrgan_Ordinary * organ , 
					 const GFPhysVector3 & centerPos ,
					 float radius , 
					 float threshold , 
					 bool  inDeformeSpace = true);
	
	//GFPhysHashMap<GFPhysHashNode , AroundSoftNode> m_NodesAround;

	virtual bool IsIntersectByCutPlane(GFPhysSoftBodyTetrahedron * tetra ,
											const GFPhysVector3 & NodeVert0 , 
											const GFPhysVector3 & NodeVert1 , 
											const GFPhysVector3 & NodeVert2 ,
											const GFPhysVector3 & NodeVert3 , 
											bool & needCheckDecom);

	virtual bool GetEdgeCutPoint(GFPhysSoftBodyNode * pNode0 , GFPhysSoftBodyNode * pNode1 ,const GFPhysVector3 & node0 , const GFPhysVector3 & node1 , GFPhysVector3 & clipPoint , Real & cutWeight);

	virtual Real NodeDist(GFPhysSoftBodyNode * pNode , const GFPhysVector3 & node);

	void ExtractInnerTetras(std::vector<GFPhysSoftBodyTetrahedron *> & TissueToCheck);

	float m_CutRadius;
	float m_CutThresHold;

	//GFPhysSoftBodyNode * m_CenterNode;
	GFPhysVector3 m_CenterPos;
	bool m_IndeformSpace;
};

class EnvolopeSpaceCutFace : public GFPhysISoftBodyCutSurface
{
public:
	class EnvolopePlane
	{
	public:
		EnvolopePlane()
		{
			m_IsCapped = true;
		}
		GFPhysVector3 m_Point;
		GFPhysVector3 m_Normal;
		bool m_IsCapped;
	};

	EnvolopeSpaceCutFace();

	virtual void addEnvolopePlane(const GFPhysVector3 & point , const GFPhysVector3 & normal);

	virtual void addEnvolopePlane(const GFPhysVector3 & point0 , const GFPhysVector3 & point1 , const GFPhysVector3 & point2 , bool iscapped);

	virtual bool IsIntersectByCutPlane( GFPhysSoftBodyTetrahedron * tetra , 
										const GFPhysVector3 & NodeVert0 ,
										const GFPhysVector3 & NodeVert1 , 
										const GFPhysVector3 & NodeVert2 , 
										const GFPhysVector3 & NodeVert3 ,
										bool & needCheckDecom);

	virtual bool GetEdgeCutPoint(GFPhysSoftBodyNode * pNode0 , GFPhysSoftBodyNode * pNode1 ,const GFPhysVector3 & node0 , const GFPhysVector3 & node1 , GFPhysVector3 & clipPoint , Real & cutWeight);

	virtual Real NodeDist(GFPhysSoftBodyNode * pNode , const GFPhysVector3 & node);


	bool IsInSideEnvelope(const GFPhysVector3 & pos);
	GFPhysAlignedVectorObj<EnvolopePlane> m_Planes;

	GFPhysAlignedVectorObj<float> m_PlaneDist0;
	GFPhysAlignedVectorObj<float> m_PlaneDist1;

};


class CutQuadWithThickNess : public EnvolopeSpaceCutFace
{
public:
	CutQuadWithThickNess(const GFPhysVector3 & u0 , const GFPhysVector3 & u1 , const GFPhysVector3 & d0 , const GFPhysVector3 & d1 , float thickness);

	bool IsIntersectByCutPlane( GFPhysSoftBodyTetrahedron * tetra , 
													  const GFPhysVector3 & NodeVert0 ,
													  const GFPhysVector3 & NodeVert1 , 
													  const GFPhysVector3 & NodeVert2 , 
													  const GFPhysVector3 & NodeVert3 ,
													  bool & needCheckDecom);
	GFPhysVector3 m_CutMin;
	GFPhysVector3 m_CutMax;

	GFPhysVector3 m_convexPos[8];

	GFPhysVector3 m_UpperPlanePt;
	GFPhysVector3 m_UpperPlaneNormal;

	GFPhysVector3 m_DownPlanePt;
	GFPhysVector3 m_DownPlaneNormal;

	std::set<GFPhysSoftBodyTetrahedron*> m_IntersectTetras;
};


class CutQuadWithThickNess2 : public GFPhysPresetCutPolygonSurface
{
public:
	

	CutQuadWithThickNess2(const GFPhysVector3 & u0 , const GFPhysVector3 & u1 , const GFPhysVector3 & d0 , const GFPhysVector3 & d1 , float thickness);
};
#endif