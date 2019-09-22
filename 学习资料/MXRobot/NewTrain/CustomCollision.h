#ifndef _CUSTOMCOLLISION_
#define _CUSTOMCOLLISION_
#include "collision/GoPhysCollisionlib.h"
#include "Dynamic/GoPhysDynamicLib.h"
#include "Ogre.h"
#include "MXOgreWrapper.h"
#define  SEGMENT_COLLIDE 0
using namespace GoPhys;



class VeinConnectObject;
class VeinConnectObjectV2;
class VeinConnectPairV2;
class VeinRigidCollisionListener;
class VeinCollideData : public GFPhysDCDInterface::Result
{
public:
#if SEGMENT_COLLIDE
    VeinCollideData(const GFPhysVector3 & pA , const GFPhysVector3 & pB , int ClusterIndex , int PairIndex);
#else
    VeinCollideData(const GFPhysVector3 & pA0, const GFPhysVector3 & pB0, const GFPhysVector3 & pA1, const GFPhysVector3 & pB1, int ClusterIndex, int PairIndex);
#endif	
	GFPhysVector3 CalculatStickPointInWorld();

	//update state
	void Update(float dt);

	//mark not contact
	//void SetContactOff();

	void SetHookAndContactOff();

	/*overridden*/
	void SetShapeIdentifiers(int partId0,int index0,int partId1,int index1)
	{}
	/*overridden*/
	void AddContactPoint(const GFPhysVector3& normalOnBInWorld,const GFPhysVector3& pointInWorld,Real depth);

	void AddHookPoint(const GFPhysVector3& pointInWorld , const GFPhysVector3 & localHookOffset, const GFPhysVector3 & localHookDir);
#if SEGMENT_COLLIDE
	GFPhysVector3 m_PointA;//segment points
	
	GFPhysVector3 m_PointB;
#else 
    GFPhysVector3 m_PointA0, m_PointA1;//segment points

    GFPhysVector3 m_PointB0, m_PointB1;
#endif
	int m_ClusterIndex;//cluster in vein connect object
	
	int m_PairIndex;//pair index in cluster

	bool m_InContact;//whether is contact with instructment

	//bool m_WillBeDissected;	

	bool m_CanContact;

	int  m_InHookState;

	VeinConnectObject * m_HostObject;

	GFPhysCollideObject * m_contactRigid;//contacted instrucment
	
	VeinRigidCollisionListener * m_collisionlistener;

	GFPhysVector3 m_contactinRigidLocal;//point in contact instrucment local space

	GFPhysVector3 m_contactinWorld;

	GFPhysVector3 m_LocalHookOffset;
	GFPhysVector3 m_WorldHookOffset;

	GFPhysVector3 m_LocalHookDir;
	GFPhysVector3 m_WorldHookDir;
protected:
	float m_ContactRecovredTime;
};

class VeinTreeCollideConvexCallBack : public GFPhysNodeOverlapCallback
{
public:
	VeinTreeCollideConvexCallBack();

	void ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData);

	GFPhysCollideObject * m_Convex;

	VeinConnectObject * m_VeinConnectObj;

	VeinRigidCollisionListener * m_collisionlistener;

	GFPhysVector3 m_Convexaabbmin,m_Convexaabbmax;

};


class VeinTreeCollideSphereCallBack : public GFPhysNodeOverlapCallback
{
public:
	VeinTreeCollideSphereCallBack();

	void ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData);

	GFPhysCollideObject * m_Convex;

	GFPhysVector3 m_WorldCenter;

	GFPhysVector3 m_WorldSegmentPointA;

	GFPhysVector3 m_WorldSegmentPointB;
	
	GFPhysVector3 m_HookLocalOffset;

	GFPhysVector3 m_HookLocalDir;

	GFPhysVector3 m_WorldHookDir;

	float m_radius;

	VeinConnectObject * m_VeinConnectObj;

	GFPhysVector3 m_Radiusaabbmin,m_Radiusaabbmax;

	int m_currHookCount;

};

bool TriangleIntersectWithMargin(GFPhysVector3 triangleA[3] , GFPhysVector3 triangleB[3] , float margin);

//veinv2========================================================================================================
class VeinCollideDataV2 : public GFPhysDCDInterface::Result
{
public:
	VeinCollideDataV2(int clusterIndex , int pairIndex);

	/*overridden*/
	void SetShapeIdentifiers(int partId0,int index0,int partId1,int index1) {}
	/*overridden*/
	void AddContactPoint(const GFPhysVector3& normalOnBInWorld,const GFPhysVector3& pointInWorld,Real depth) {}

// 	void Update(float dt);
// 
// 	void SetContactOff();
// 
// 	void SetHookOff();
// 
// 	void AddHookPoint(const GFPhysVector3& pointInWorld , const GFPhysVector3 & localHookOffset, const GFPhysVector3 & localHookDir);

	GFPhysCollideObject * m_ContactRigid;//contacted instrucment

	VeinConnectObjectV2 * m_pHostVeinObj;

	VeinConnectPairV2 * m_pPair;

	int m_ClusterIndex;

	int m_PairIndex;

	bool m_IsInContact;
private:


};

class VeinV2TreeCollideConvexCallBack : public GFPhysNodeOverlapCallback
{
public:
	VeinV2TreeCollideConvexCallBack();

	void ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData);

	GFPhysCollideObject * m_Convex;

	//VeinConnectObjectV2 * m_VeinConnectObj;

	GFPhysVector3 m_ConvexAabbMin,m_ConvexAabbMax;

};


class ViewDetection
{
public:
	//ViewDetection() {};
	ViewDetection(const Ogre::Vector3 & pos  = Ogre::Vector3::ZERO, float minCos = 0.95 ,  bool isDebug = false);
	~ViewDetection();
	bool Update(float dt , Ogre::Camera * pCamera);
	float GetAimingTime() { return m_AccumulatedTime;}
	void Clear() { m_AccumulatedTime = 0.f;}
	
	//void SetDetectDir(bool detect) { m_IsDetectDir = detect;}
	void SetPosition(float x , float y , float z) { m_Position.x = x ; m_Position.y = y ; m_Position.z = z ;  }
	void SetPosition(const Ogre::Vector3 & pos) { m_Position = pos; }
	void SetFaceTo(const Ogre::Vector3 & faceto) { m_FaceTo = faceto; }
	void SetDetectDist(const float dist) {m_DetectDist = dist;}
	void SetMinCos(const float minCos) { m_MinCos = minCos;}

	void DetectDist(bool detect) {m_IsDetectDist = detect;}
	void DetectDir(bool detect) {m_IsDetectDir = detect;}

	const Ogre::Vector3 & GetPosition() {return m_Position;}
	const Ogre::Vector3 & GetFaceTo() {return m_FaceTo;}
	float GetDetectDist() {return m_DetectDist;}
	float GetMinCos() {return m_MinCos;}

	bool IsDetectDir() { return m_IsDetectDir;}
	bool IsDetectDist() { return m_IsDetectDist;}
	
	void SetValid(bool IsValid) { m_IsValid = IsValid;}
	bool IsValid() const { return m_IsValid; }

	//debug
	bool m_IsDebug;
	Ogre::ManualObject *m_pManual;
	void Draw(Ogre::Camera * pCamera , int colorIndex);

private:
	float m_AccumulatedTime;
	bool m_LastResult;
	bool m_IsDetectDir;
	bool m_IsDetectDist;
	Ogre::Vector3 m_Position;
	Ogre::Vector3 m_FaceTo;
	float m_DetectDist;
	float m_MinCos;
	bool m_IsValid;


};

#endif