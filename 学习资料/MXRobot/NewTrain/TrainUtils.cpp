#include "TrainUtils.h"
#include "OgreVector3.h"
#include "PhysicsWrapper.h"
#include "collision\NarrowPhase\GoPhysPrimitiveTest.h"
#include "collision\CollisionShapes\GoPhysTriangleShape.h"
#include "MXOgreGraphic.h"
using namespace GoPhys;

class AabbSoftFaceClosetCallBack : public GFPhysNodeOverlapCallback
{
public:
    AabbSoftFaceClosetCallBack(const GFPhysVector3 & aabbmin,
                               const GFPhysVector3 & aabbmax,
                               GFPhysRigidBody* Rigid,
                               Real Threshold
                               )
    {
        m_AabbMin = aabbmin;
        m_AabbMax = aabbmax;
        m_Rigid = Rigid;
        m_Threshold = Threshold;
        m_closetDist = FLT_MAX;
    }

    void ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData)
    {
        GFPhysSoftBodyFace * face = (GFPhysSoftBodyFace*)UserData;   
        Real AABBdist = ClosetDistAaabb(m_AabbMin,m_AabbMax,face->m_AabbMin,face->m_AabbMax);
        if(AABBdist <= m_closetDist)
        {
            GFPhysTriangleShape softface= GFPhysTriangleShape(
                face->m_Nodes[0]->m_CurrPosition,
                face->m_Nodes[1]->m_CurrPosition,
                face->m_Nodes[2]->m_CurrPosition);
            GFPhysConvexCDShape * rigidconvex = dynamic_cast<GFPhysConvexCDShape *>(m_Rigid->GetCollisionShape());
            GFPhysTransform transR;
            transR = m_Rigid->GetWorldTransform();
            GFPhysTransform transS;
            transS.SetIdentity();
            Real realDist = GetConvexsClosetPoint(&softface,rigidconvex,transS,transR,m_ClosetPoints[0],m_ClosetPoints[1],m_Threshold);
            
            if(realDist <= m_closetDist)
            {
                m_closetDist = realDist;
                m_Face = face;
            }            
        }        
    }
    float m_closetDist;
    GFPhysVector3 m_ClosetPoints[2];
    GFPhysVector3 m_AabbMin;
    GFPhysVector3 m_AabbMax;
    GFPhysRigidBody * m_Rigid; 
    GFPhysSoftBodyFace * m_Face;
    Real m_Threshold;
    
};

//====================================================================================================
bool GetRigidSoftClosestDist(GFPhysRigidBody* Rigid ,
                             GFPhysSoftBody* Soft , 
                             const Real& Threshold ,
                             GFPhysVector3 ClosetPoints[2] ,//0 is on rigid,1 is on soft
                             GFPhysSoftBodyFace & ClosetFace,  
                             Real & ClosestDist)
{
    GFPhysVector3 rigidaabbMin,rigidaabbMax,rigidenlargeaabbMin,rigidenlargeaabbMax;
    Rigid->GetAabb(rigidaabbMin,rigidaabbMax);
    rigidenlargeaabbMin = rigidaabbMin - GFPhysVector3(Threshold,Threshold,Threshold) * 0.5;
    rigidenlargeaabbMax = rigidaabbMax + GFPhysVector3(Threshold,Threshold,Threshold) * 0.5;

    AabbSoftFaceClosetCallBack closetCallBack(rigidaabbMin,rigidaabbMax,Rigid,Threshold);
    GFPhysVectorObj<GFPhysDBVTree*> bvtrees = Soft->GetSoftBodyShape().GetFaceBVTrees();
    for(size_t t = 0 ; t < bvtrees.size() ; t++)
    {
        bvtrees[t]->TraverseTreeAgainstAABB(&closetCallBack , rigidenlargeaabbMin , rigidenlargeaabbMax);
    }
    if(closetCallBack.m_closetDist < Threshold )
    {
        ClosestDist = closetCallBack.m_closetDist;
        ClosetPoints[0] = closetCallBack.m_ClosetPoints[0];
        ClosetPoints[1] = closetCallBack.m_ClosetPoints[1];
        
        ClosetFace= * closetCallBack.m_Face;        
        return true;
    }
    else
        return false;   
}



//aabb ->rigidcube ,GetConvexsClosetPoint(rigidcube,Rigid)
bool  GetRigidAabbClosestDist(GFPhysRigidBody* Rigid ,
                              const GFPhysVector3& aabbmin,
                              const GFPhysVector3& aabbmax,
                              const GFPhysVector3& translation,
                              const GFPhysQuaternion& rotation,
                              const Real& Threshold ,
                              GFPhysVector3 ClosetPoints[2] ,
                              Real & ClosestDist)
{
    GFPhysTransform transR = Rigid->GetWorldTransform();
	GFPhysConvexCDShape * rigidconvex = dynamic_cast<GFPhysConvexCDShape *>(Rigid->GetCollisionShape());

   
	GFPhysTransform transaabb;
	GFPhysVector3 localCenter = (aabbmin + aabbmax) * 0.5f;
	GFPhysVector3 halfExtend  = (aabbmax - aabbmin) * 0.5f;
    transaabb.SetOrigin(translation + QuatRotate(rotation , localCenter));
    transaabb.SetRotation(rotation);
	GFPhysBoxShape boxshape(halfExtend);
	boxshape.SetMargin(0);

    Real dis = Threshold;
    ClosestDist = GetConvexsClosetPoint(&boxshape,rigidconvex,transaabb,transR,ClosetPoints[0],ClosetPoints[1],dis);

    if (ClosestDist <= Threshold)
    {
        return true;
    }
    else
    {
        return false;
    }
}

class FaceQuadCallBack : public GFPhysNodeOverlapCallback
{
public:
	FaceQuadCallBack(const GFPhysVector3 quadVert[4] , 
		             GFPhysVectorObj<GFPhysSoftBodyFace*> & IntersectFaces) : m_IntersectFaces(IntersectFaces)
	{
		m_TriVertA[0] = quadVert[0];
		m_TriVertA[1] = quadVert[1];
		m_TriVertA[2] = quadVert[2];

		m_TriVertB[0] = quadVert[2];
		m_TriVertB[1] = quadVert[1];
		m_TriVertB[2] = quadVert[3];
	} 

	void ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData)
	{
		GFPhysSoftBodyFace * face = (GFPhysSoftBodyFace*)UserData;
		
		GFPhysVector3 FaceVerts[3];
		FaceVerts[0] = face->m_Nodes[0]->m_CurrPosition;
		FaceVerts[1] = face->m_Nodes[1]->m_CurrPosition;
		FaceVerts[2] = face->m_Nodes[2]->m_CurrPosition;

		for(int t = 0 ; t < 2 ; t++)//test intersect with quad
		{
			GFPhysVector3 ResultPoint[2];

			bool intersect = TriangleIntersect(m_TriVertA , FaceVerts , ResultPoint);

			if(intersect == false)
			{
			   intersect = TriangleIntersect(m_TriVertB , FaceVerts , ResultPoint);
			}

			if(intersect)
			{
			   m_IntersectFaces.push_back(face);
			}
		}
	}

	void ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA , GFPhysDBVNode * dynNodeB)
	{

	}

	GFPhysVector3 m_TriVertA[3];
	GFPhysVector3 m_TriVertB[3];
	GFPhysVectorObj<GFPhysSoftBodyFace*> & m_IntersectFaces;
};

void GetSoftFaceIntersectQuad(const GFPhysVector3 quadVerts[4] ,
							  GFPhysSoftBody * sb ,
							  GFPhysVectorObj<GFPhysSoftBodyFace*> & IntersectFaces)
{
	GFPhysVector3 aabbMin = quadVerts[0];
	GFPhysVector3 aabbMax = aabbMin;
	
	aabbMin.SetMin(quadVerts[1]);
	aabbMin.SetMin(quadVerts[2]);
	aabbMin.SetMin(quadVerts[3]);

	aabbMax.SetMax(quadVerts[1]);
	aabbMax.SetMax(quadVerts[2]);
	aabbMax.SetMax(quadVerts[3]);

	FaceQuadCallBack faceCallBack(quadVerts , IntersectFaces);
	
	sb->GetSoftBodyShape().TraverseFaceTreeAgainstAABB( &faceCallBack , aabbMin , aabbMax);
}

void GetWorldAABB(const GFPhysTransform & worldTrans ,
				  const GFPhysVector3 & localmin ,
				  const GFPhysVector3 & localmax , 
				  GFPhysVector3 & worldmin,
				  GFPhysVector3 & worldmax)
{
	GFPhysVector3 localHalfExtents = Real(0.5)*(localmax - localmin);

	GFPhysVector3 localCenter = Real(0.5)*(localmax + localmin);

	GFPhysMatrix3 abs_b = worldTrans.GetBasis().Absolute();  

	GFPhysPoint3 center = worldTrans(localCenter);

	GFPhysVector3 extent = GFPhysVector3(abs_b[0].Dot(localHalfExtents),
		                                 abs_b[1].Dot(localHalfExtents),
		                                 abs_b[2].Dot(localHalfExtents));
	worldmin = center - extent;
	worldmax = center + extent;
}

bool CalConvexHullBestFitFrame(GFPhysVector3 * hullVerts , 
							   int numHullVerts,
							   GFPhysVector3 & center , 
							   GFPhysMatrix3 & rotMat ,
							   GFPhysVector3 & extend)
{
	GFPhysMatrix3 covMat;
	covMat.SetZero();

	GFPhysVector3 mean(0,0,0);
	for(size_t c = 0  ; c < numHullVerts ; c++)
	{
		mean += hullVerts[c];
	}
	mean = mean / float(numHullVerts);

	for(size_t c = 0  ; c < numHullVerts ; c++)
	{
		GFPhysVector3 q = hullVerts[c]-mean;
		covMat[0][0] += q.m_x * q.m_x;
		covMat[0][1] += q.m_x * q.m_y;
		covMat[0][2] += q.m_x * q.m_z;

		covMat[1][1] += q.m_y * q.m_y;
		covMat[1][2] += q.m_y * q.m_z;

		covMat[2][2] += q.m_z * q.m_z;
	}

	covMat[1][0] = covMat[0][1];
	covMat[2][0] = covMat[0][2];

	covMat.Scale(1.0f / float(numHullVerts));

	//calculate eigen value of covariant matrix
	GFPhysMatrix3 eigenVectors;
	GFPhysVector3 eigenValue;
	CalcEigenVectorB (covMat , eigenVectors , eigenValue);

	GFPhysVector3 vecx = eigenVectors.GetColumn(0).Normalized();
	GFPhysVector3 vecy = eigenVectors.GetColumn(1).Normalized();
	GFPhysVector3 vecz = eigenVectors.GetColumn(2).Normalized();

	float xycrs = vecx.Dot(vecy);
	float xzcrs = vecx.Dot(vecz);
	float yzcrs = vecy.Dot(vecz);

	if(xycrs < 0.0001f && xzcrs < 0.0001f && yzcrs < 0.0001f)
	{
		// ObbAixs[0] = vecx;
		// ObbAixs[1] = vecy;
		// ObbAixs[2] = vecz;
		center = mean;
		rotMat.SetColumn(0 , vecx);
		rotMat.SetColumn(1 , vecy);
		rotMat.SetColumn(2 , vecz);

		//bound
		GFPhysMatrix3 invRot = rotMat.Inverse();
		GFPhysVector3 minvec(FLT_MAX , FLT_MAX , FLT_MAX);
		GFPhysVector3 maxvec(-FLT_MAX , -FLT_MAX , -FLT_MAX);

		extend = GFPhysVector3(0,0,0);
		for(size_t c = 0 ; c < numHullVerts ; c++)
		{
			GFPhysVector3 localPos = invRot * (hullVerts[c]-mean);
			minvec.SetMin(localPos);
			maxvec.SetMax(localPos);
		}

		//new center
		GFPhysVector3 localCenter = (minvec+maxvec)*0.5f;
		center = rotMat * localCenter + mean;
		extend = (maxvec-minvec)*0.5f;
		return true;
	}
	else//need process this case
	{
		return false;
	}
}
bool CalConvexHullBestFitFrame(const GFPhysAlignedVectorObj<GFPhysVector3> & hullVerts , 
							   GFPhysVector3 & center , 
							   GFPhysMatrix3 & rotMat ,
							   GFPhysVector3 & extend)//, GFPhysVector3 & ObbExt , GFPhysVector3 ObbAixs[3])
{
	GFPhysMatrix3 covMat;
	covMat.SetZero();

	GFPhysVector3 mean(0,0,0);
	for(size_t c = 0  ; c < hullVerts.size() ; c++)
	{
		mean += hullVerts[c];
	}
	mean = mean / float(hullVerts.size());

	for(size_t c = 0  ; c < hullVerts.size() ; c++)
	{
		GFPhysVector3 q = hullVerts[c]-mean;
		covMat[0][0] += q.m_x * q.m_x;
		covMat[0][1] += q.m_x * q.m_y;
		covMat[0][2] += q.m_x * q.m_z;

		covMat[1][1] += q.m_y * q.m_y;
		covMat[1][2] += q.m_y * q.m_z;

		covMat[2][2] += q.m_z * q.m_z;
	}

	covMat[1][0] = covMat[0][1];
	covMat[2][0] = covMat[0][2];

	covMat.Scale(1.0f / float(hullVerts.size()));

	//calculate eigen value of covariant matrix
	GFPhysMatrix3 eigenVectors;
	GFPhysVector3 eigenValue;
	CalcEigenVectorB (covMat , eigenVectors , eigenValue);

	GFPhysVector3 vecx = eigenVectors.GetColumn(0).Normalized();
	GFPhysVector3 vecy = eigenVectors.GetColumn(1).Normalized();
	GFPhysVector3 vecz = eigenVectors.GetColumn(2).Normalized();

	float xycrs = vecx.Dot(vecy);
	float xzcrs = vecx.Dot(vecz);
	float yzcrs = vecy.Dot(vecz);

	if(xycrs < 0.0001f && xzcrs < 0.0001f && yzcrs < 0.0001f)
	{
		// ObbAixs[0] = vecx;
		// ObbAixs[1] = vecy;
		// ObbAixs[2] = vecz;
		center = mean;
		rotMat.SetColumn(0 , vecx);
		rotMat.SetColumn(1 , vecy);
		rotMat.SetColumn(2 , vecz);

		//bound
		GFPhysMatrix3 invRot = rotMat.Inverse();
		GFPhysVector3 minvec(FLT_MAX , FLT_MAX , FLT_MAX);
		GFPhysVector3 maxvec(-FLT_MAX , -FLT_MAX , -FLT_MAX);

		extend = GFPhysVector3(0,0,0);
		for(size_t c = 0 ; c < hullVerts.size() ; c++)
		{
			GFPhysVector3 localPos = invRot * (hullVerts[c]-mean);
			minvec.SetMin(localPos);
			maxvec.SetMax(localPos);
		}

		//new center
		GFPhysVector3 localCenter = (minvec+maxvec)*0.5f;
		center = rotMat * localCenter + mean;
		extend = (maxvec-minvec)*0.5f;
		return true;
	}
	else//need process this case
	{
		return false;
	}
}

Real CalcAngleBetweenLineAndFace( const GFPhysVector3 & lineDir , const GFPhysVector3 & faceNormal )
{
    GFPhysVector3 lineDircopy = lineDir.Normalized();
    GFPhysVector3 faceNormalcopy = faceNormal.Normalized();

    Real PI = Real( 4.0 * atanf( 1.0f ) );
    Real rad = acosf(lineDircopy.Dot(faceNormalcopy))* 180.0f / PI;
    if ( rad > 0.0f && rad <= 90.0f)
    {
        return 90.0f - rad;
    }
    if ( rad > 90.0f && rad <= 180.0f)
    {
        return rad - 90.0f;        
    }
}