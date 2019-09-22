#ifndef _GEOMETRYINSTANCEMASNAGER_
#define _GEOMETRYINSTANCEMASNAGER_

class GeometryInstanceManager {
public:
	GeometryInstanceManager(int useInstance_, const char* name_, const char* meshName_, const char* materialName_);
	~GeometryInstanceManager();
	int addInstance();
	void delInstance(int id);
	void updateInstance(int id, Ogre::Vector3 scale, Ogre::Vector3 position, Ogre::Quaternion q);
protected:
	int useInstance;
	char name[100];
	char meshName[100];
	char materialName[100];
	int max;
	int count;
	void** instances;
	Ogre::InstanceManager* pInstanceMgr;
};

#endif