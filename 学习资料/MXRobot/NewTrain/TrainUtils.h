#ifndef _TRAINUTILS_
#define _TRAINUTILS_
#include "Dynamic\PhysicBody\GoPhysRigidBody.h"
#include "Dynamic\PhysicBody\GoPhysSoftBody.h"
#include "Math\GoPhysVector3.h"
using namespace GoPhys;


bool  GetRigidSoftClosestDist(GFPhysRigidBody* Rigid ,
                              GFPhysSoftBody* Soft , 
                              const Real& Threshold ,
                              GFPhysVector3 ClosetPoints[2] ,
                              GFPhysSoftBodyFace & ClosetFace,
                              Real & ClosestDist);

bool  GetRigidAabbClosestDist(GFPhysRigidBody* Rigid ,
                              const GFPhysVector3& aabbmin,
                              const GFPhysVector3& aabbmax,
                              const GFPhysVector3& translation,
                              const GFPhysQuaternion& rotation,
                              const Real& Threshold ,
                              GFPhysVector3 ClosetPoints[2] ,
                              Real & ClosestDist);

void GetSoftFaceIntersectQuad(const GFPhysVector3 quadVerts[4] , 
							  GFPhysSoftBody * sb ,
							  GFPhysVectorObj<GFPhysSoftBodyFace*> & IntersectFaces);

bool CalConvexHullBestFitFrame(GFPhysVector3 * hullVerts , 
							   int numHullVerts,
							   GFPhysVector3 & center , 
							   GFPhysMatrix3 & rotMat ,
							   GFPhysVector3 & extend);

bool CalConvexHullBestFitFrame(const GFPhysAlignedVectorObj<GFPhysVector3> & hullVerts , 
							   GFPhysVector3 & center , 
							   GFPhysMatrix3 & rotMat ,
							   GFPhysVector3 & extend);//GFPhysVector3 & ObbExt , GFPhysVector3 ObbAixs[3]);

void GetWorldAABB(const GFPhysTransform & worldTrans ,
				  const GFPhysVector3 & localmin ,
				  const GFPhysVector3 & localmax , 
				  GFPhysVector3 & worldmin,
				  GFPhysVector3 & worldmax);

Real CalcAngleBetweenLineAndFace(const GFPhysVector3 & lineDir ,
                                 const GFPhysVector3 & faceNormal);

#endif