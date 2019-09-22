
#include "Ogre.h"
#include "MXOgreWrapper.h"
#include "GeometryInstanceManager.h"

GeometryInstanceManager::GeometryInstanceManager(int useInstance_, const char* name_, const char* meshName_, const char* materialName_) {
	useInstance = useInstance_;
	strcpy(meshName, meshName_);
	strcpy(materialName, materialName_);
	strcpy(name, name_);
	max = 1000;
	count = 0;

	if (useInstance) {
		instances = (void**)malloc(max*sizeof(Ogre::InstancedEntity*));
		pInstanceMgr = MXOgreWrapper::Get()->GetDefaultSceneManger()->createInstanceManager(
			name, meshName,
			Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, Ogre::InstanceManager::HWInstancingBasic,
			max, Ogre::IM_USEALL);

	}
	else {
		instances = (void**)malloc(max*sizeof(Ogre::SceneNode*));
	}
	
	
	for (int i = 0; i < max; i++) instances[i] = NULL;

}

GeometryInstanceManager::~GeometryInstanceManager() {
	for (int i = 0; i < max; i++) {
		if (instances[i]) delInstance(i);
	}
	free(instances);
	MXOgreWrapper::Get()->GetDefaultSceneManger()->destroyInstanceManager(pInstanceMgr);
	
}

int GeometryInstanceManager::addInstance() {
	int i;
	char currentName[200];

	for (i = 0; i < max; i++) {
		if (instances[i] == NULL) break;
	}

	if (useInstance) {
		instances[i] = pInstanceMgr->createInstancedEntity(materialName);
	}
	else {
		sprintf(currentName, "%s%d", name, i);
		Ogre::SceneManager * pSceneManager = MXOgreWrapper::Get()->GetDefaultSceneManger();
		bool flag = pSceneManager->hasEntity(currentName);
		Ogre::Entity* LightEnt = NULL;
		if (flag)
		{
			LightEnt = pSceneManager->getEntity(currentName);
		}
		else
		{
			LightEnt = pSceneManager->createEntity(currentName, meshName);
		}

		LightEnt->setMaterialName(materialName);
		Ogre::SceneNode* lightMeshNode = pSceneManager->getRootSceneNode()->createChildSceneNode();
		lightMeshNode->attachObject(LightEnt);
		lightMeshNode->setPosition(0.0, 0.0, 0.0);
		//lightMeshNode->setOrientation(quatDir);
		lightMeshNode->setScale(0.15, 0.15, 0.15);
		instances[i]=lightMeshNode;
	}
	
	return i;
}

void GeometryInstanceManager::delInstance(int id) {
	Ogre::InstancedEntity* currentInstance;
	
	if (useInstance) {
		currentInstance = (Ogre::InstancedEntity*)instances[id];
		MXOgreWrapper::Get()->GetDefaultSceneManger()->destroyInstancedEntity(currentInstance);
	}
	else {

	}
	instances[id] = NULL;

}

void GeometryInstanceManager::updateInstance(int id, Ogre::Vector3 scale, Ogre::Vector3 position, Ogre::Quaternion q) {
	Ogre::InstancedEntity* currentInstance;
	Ogre::SceneNode* currentNode;
	
	if (useInstance) {
		currentInstance = (Ogre::InstancedEntity*)instances[id];
		currentInstance->setScale(scale);
		currentInstance->setPosition(position);
		currentInstance->setOrientation(q);
	}
	else {
		currentNode = (Ogre::SceneNode*)instances[id];
		currentNode->setScale(scale);
		currentNode->setPosition(position);
		currentNode->setOrientation(q);
	}
	
}

