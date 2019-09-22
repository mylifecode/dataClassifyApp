#ifndef __COLLISIONTOOLS__
#define __COLLISIONTOOLS__

#include "Ogre.h"
#include "Singleton.h"

class CCollisionTools:public CSingleT<CCollisionTools>
{
public:
	enum QueryFlags
	{
		WATER_MASK = 1<<7,
		ENTITY_MASK  = 1<<8,
	};
public:
	CCollisionTools();
	~CCollisionTools();

	bool collidesWithEntity(const Ogre::Vector3& fromPoint, const Ogre::Vector3& toPoint, const float collisionRadius = 2.5f, const float rayHeightLevel = 0.0f, const Ogre::uint32 queryMask = 0xFFFFFFFF);

	bool raycastFromPoint(const Ogre::Vector3 &point, const Ogre::Vector3 &normal, Ogre::Vector3 &result,Ogre::ulong &target,float &closest_distance, const Ogre::uint32 queryMask = 0xFFFFFFFF);

	bool raycast(const Ogre::Ray &ray, Ogre::Vector3 &result,Ogre::ulong &target,float &closest_distance, const Ogre::uint32 queryMask = 0xFFFFFFFF);
	
	bool raycastFromPoint(const Ogre::Vector3 &point, const Ogre::Vector3 &normal, const Ogre::String colliObjName, Ogre::Vector3 &result,Ogre::Entity* target,float &closest_distance, const Ogre::uint32 queryMask = 0xFFFFFFFF);
	
	bool raycast(const Ogre::Ray &ray, Ogre::String colliObjName, Ogre::Vector3 &result, Ogre::Entity* entity,float &closest_distance, const Ogre::uint32 queryMask = 0xFFFFFFFF);
	

	bool CheckPointInSphere(Ogre::Real radius,Ogre::Vector3 vecCenter,Ogre::Vector3 vecPoint);

	inline Ogre::String GetCollosionEntityName() { return m_strEntityName; }

private:
		void GetMeshInformation(const Ogre::MeshPtr mesh,
		size_t &vertex_count,
		Ogre::Vector3* &vertices,
		size_t &index_count,
		unsigned long* &indices,
		const Ogre::Vector3 &position,
		const Ogre::Quaternion &orient,
		const Ogre::Vector3 &scale);

public:
	Ogre::RaySceneQuery *mRaySceneQuery;
	Ogre::SceneManager *mSceneMgr;

private:
	Ogre::String m_strEntityName;

public:
	// create the ray to test
	Ogre::Ray ray;

};
#endif