/**Author:zx**/
#pragma once
#include "IMXDefine.h"
#include "Singleton.h"
#include "OgreMaxScene.hpp"


using namespace::std;

class CXMLContentManager;
class CXMLWrapperGlobalTraining;
class CXMLWrapperToolConfig;
class CXMLWrapperTraining;
class CTrainingMgr;
class CXMLWrapperTool;
class ITool;
class ITraining;
class MisRobotInput;
class CXMLWrapperParticles;
class CXMLWrapperParticle;

typedef ITraining* (*REGISTER)();


class CResourceManager : public CSingleT<CResourceManager>
{
public:
	CResourceManager();
	~CResourceManager();

	bool LoadTraining(CTrainingMgr * pTrainingManager); // load all training, zx

	bool LoadResource(Ogre::vector<Ogre::String>::type & addResloacte); // load ogre resource, zx

	bool m_bOgreResourceLoaded;
	bool m_bClassRegistered;
	bool m_bXMLConfigsLoaded;
	bool m_bChangeTool;//ÊÇ·ñ»»Æ÷Ðµ



    void EnableReloadConfigFile(int controlMode ){  m_bXMLConfigsLoaded = false; m_controlMode = controlMode;}// reload config file
    inline int GetControlModeType(){ return m_controlMode;}//mode 1:singlePort ---mode 2:multiPort
	inline CXMLWrapperGlobalTraining * GetGlobalTraining() const { return m_pGlobalTraining; }
	inline CXMLWrapperToolConfig * GetToolConfig() const { return m_pToolConfig; } 
    //inline CXMLWrapperHardwareConfig * GetHardwareConfig() const { return m_pHardwareConfig; } 
	inline CXMLWrapperParticles * GetParticles() const { return m_pParticlesConfig; }
	CXMLWrapperTraining * GetTrainingConfigByName(const Ogre::String & strTrainingName) const;

	const Ogre::String LoadDefaultTraining(CTrainingMgr * pTrainingManager);
	bool LoadTraining(const Ogre::String strTrainingName, CXMLWrapperToolConfig * pToolConfig, CTrainingMgr * pTrainingManager); // load one training by name in config, zx
	bool LoadTraining(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig, CTrainingMgr * pTrainingManager); // load one training by config, zx

	ITool * GetOneTool(const Ogre::String & strType, const bool bReuse, const Ogre::String & strSubType = ""); // get one tool by tool's type and subtype, zx
	ITraining * CreateTrainingByTypeAndName(const Ogre::String & strType, const Ogre::String & strName); // create one training by training's name and type, zx
	void RemoveToolInPool(ITool * pTool); // tools in pool, for reuse, zx

	CXMLWrapperParticle * GetParticleConfigByNameAndType(const Ogre::String & strParticleName, const Ogre::String & strParticleType); // get one particle effect by name and type in config file, zx


	template<typename T>
	struct Register
	{
		Register(const std::string& trainingName,ITraining* (*generator)())
		{
			CResourceManager::Instance()->RegisterTraining(trainingName,generator);
		}
	};

	void RegisterTraining(const std::string& trainingName,ITraining* (*generator)());

private:
	bool LoadXMLConfigs(); // load all xml config files, zx
	ITool * CreateToolInstance(CXMLWrapperTool * pToolConfig); // create one tool's instance by config, zx

	CXMLContentManager * m_pXMLContentManager;

	CXMLWrapperGlobalTraining * m_pGlobalTraining;
	CXMLWrapperToolConfig * m_pToolConfig;
	
	CXMLWrapperParticles * m_pParticlesConfig;

	std::vector<ITool *> m_vectToolsPool;

	OgreMax::OgreMaxScene * m_pOms; // only for ogre scene loading use, zx
	
	int m_controlMode;// single port  or  multi port  

	std::map<std::string,REGISTER> m_trainingGeneratorMap;
};