#ifndef _DEFERREDSCENEMANAGER_
#define _DEFERREDSCENEMANAGER_
#include "OgreSceneManager.h"
#include "OgreRenderOperation.h"
#include "Singleton.h"
//this is custom scene manager for post rend
class DeferredRendSceneManager : public Ogre::SceneManager
{
public:
	enum DeferredRendStage
	{
		Stage_None = 0,
		Stage_Depth = 1,
		Stage_SolidLit = 2,
		Stage_TransparentLit = 3,
		Stage_CustomFirst = 4,
		Stage_CustomMiddle = 5,
		Stage_CustomFinal = 6,
	};

	class DeferedSceneRendListener
	{
	public:
		virtual void BeforeRendVisibleObjects(DeferredRendSceneManager * sceneMgr , DeferredRendSceneManager::DeferredRendStage currentStage) = 0;

	};
	/** Standard Constructor. */
	DeferredRendSceneManager(const Ogre::String& name);
	
	/// @copydoc SceneManager::getTypeName
	const Ogre::String& getTypeName(void) const;

	/** @copydoc SceneManager::_renderVisibleObjects */
	virtual void _renderVisibleObjects(void);

	/** @copydoc SceneManager::renderBasicQueueGroupObjects */
	virtual void renderBasicQueueGroupObjects(Ogre::RenderQueueGroup* pGroup, 
		Ogre::QueuedRenderableCollection::OrganisationMode om);

	/** @copydoc SceneManager::renderBasicQueueGroupObjects */
	virtual const Ogre::Pass* _setPass(const Ogre::Pass* pass, 
		bool evenIfSuppressed = false, bool shadowDerivation = true);

	virtual void manualRender(Ogre::RenderOperation* rend, Ogre::Pass* pass, Ogre::Viewport* vp, 
            const Ogre::Matrix4& worldMatrix, const Ogre::Matrix4& viewMatrix, const Ogre::Matrix4& projMatrix, 
            bool doBeginEndFrame = false) ;

	virtual void manualRender(Ogre::Renderable* rend, const Ogre::Pass* pass, Ogre::Viewport* vp, 
			const Ogre::Matrix4& viewMatrix, const Ogre::Matrix4& projMatrix, bool doBeginEndFrame = false, bool lightScissoringClipping = true, 
			bool doLightIteration = true, const Ogre::LightList* manualLightList = 0);

	void SetDepthRendMaterial(const Ogre::String & depthMatName);

	void SetCurrentRendStage(DeferredRendSceneManager::DeferredRendStage pass);

	DeferredRendStage m_CurrentStage;
	Ogre::Pass * m_DepthPass;

	DeferedSceneRendListener * m_Listener;
};

/// Factory for OctreeSceneManager
class DeferredRendSceneManagerFactory : public Ogre::SceneManagerFactory , public CSingleT<DeferredRendSceneManagerFactory>
{
protected:
	void initMetaData(void) const;
public:
	DeferredRendSceneManagerFactory() {}
	~DeferredRendSceneManagerFactory() {}
	/// Factory type name
	static const Ogre::String FACTORY_TYPE_NAME;
	Ogre::SceneManager* createInstance(const Ogre::String& instanceName);
	void destroyInstance(Ogre::SceneManager* instance);
};
#endif