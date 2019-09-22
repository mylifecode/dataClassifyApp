#pragma once

#include "Ogre.h"
#include "collision\BroadPhase\GoPhysDynBVTree.h"
#include <vector>
#include "OgreVector3.h"

using namespace GoPhys;

struct Geoinfo
{
    Geoinfo(Ogre::String name,
            GFPhysDBVTree* tree)
    {
        objName = name;
        CollisionTree = tree;
    }
    Ogre::String objName;
    GFPhysDBVTree* CollisionTree;
};

class CollisionTest
{
public:
	CollisionTest( void );
	CollisionTest(Ogre::SceneManager* pSceneManager );
	~CollisionTest( void );
	void AddStaticEntity(Ogre::String name, Ogre::Entity* pEntity );
	void RemoveStaticEntity( Ogre::Entity* pEntity );
	const Ogre::String RayTest( const Ogre::Ray& ray, const Ogre::Real fDistance );
	//const Ogre::String SphereTest(const Ogre::Sphere& sphere);

private:    
    std::vector<Geoinfo*> m_objects; 
}; 