#include "DeferredRendSceneManager.h"
//-----------------------------------------------------------------------
DeferredRendSceneManager::DeferredRendSceneManager(const Ogre::String& name) : SceneManager(name)
{
	m_CurrentStage = DeferredRendSceneManager::Stage_None;
	m_DepthPass = 0;
	m_Listener = 0;
	SetDepthRendMaterial("linear_depth");
}
//-----------------------------------------------------------------------
const Ogre::String& DeferredRendSceneManager::getTypeName(void) const
{
	return DeferredRendSceneManagerFactory::FACTORY_TYPE_NAME;
}
//-----------------------------------------------------------------------
const Ogre::Pass* DeferredRendSceneManager::_setPass(const Ogre::Pass* pass, bool evenIfSuppressed, bool shadowDerivation)
{
    //draw depth pass
	if(m_CurrentStage == DeferredRendSceneManager::Stage_Depth)
	{
		//use depth pass
		if(m_DepthPass)
		{
		   return Ogre::SceneManager::_setPass(m_DepthPass,  evenIfSuppressed, shadowDerivation);
		}
		else
		{
		   OGRE_EXCEPT(Ogre::Exception::ERR_ITEM_NOT_FOUND , "No Depth Rend Pass", "DeferredRendSceneManager::_setPass");
		}
	}
	else//use the origin pass
	{
	    return Ogre::SceneManager::_setPass(pass,  evenIfSuppressed, shadowDerivation);
	}
}
//---------------------------------------------------------------------
void DeferredRendSceneManager::manualRender(Ogre::RenderOperation* rend, 
								Ogre::Pass* pass, Ogre::Viewport* vp, const Ogre::Matrix4& worldMatrix, 
								const Ogre::Matrix4& viewMatrix, const Ogre::Matrix4& projMatrix, 
								bool doBeginEndFrame) 
{
	

	_setPass(pass);
	// Do we need to update GPU program parameters?
	if (pass->isProgrammable())
	{
		updateGpuProgramParameters(pass);
	}
	mDestRenderSystem->_render(*rend);
}
//---------------------------------------------------------------------
void DeferredRendSceneManager::manualRender(Ogre::Renderable* rend, const Ogre::Pass* pass, Ogre::Viewport* vp,
								const Ogre::Matrix4& viewMatrix, 
								const Ogre::Matrix4& projMatrix,bool doBeginEndFrame,
								bool lightScissoringClipping, bool doLightIteration, const Ogre::LightList* manualLightList)
{
	_setPass(pass);
	renderSingleObject(rend, pass, lightScissoringClipping, doLightIteration, manualLightList);
}
//-----------------------------------------------------------------------------
void DeferredRendSceneManager::_renderVisibleObjects(void)
{
	if(m_Listener)
	   m_Listener->BeforeRendVisibleObjects(this , m_CurrentStage);
	
	Ogre::SceneManager::_renderVisibleObjects();
}
//-----------------------------------------------------------------------
void DeferredRendSceneManager::renderBasicQueueGroupObjects(Ogre::RenderQueueGroup* pGroup, 
												Ogre::QueuedRenderableCollection::OrganisationMode om)
{
	if(m_CurrentStage == DeferredRendSceneManager::Stage_None)
	   return;

	else if (m_CurrentStage >= DeferredRendSceneManager::Stage_CustomFirst)
	   return Ogre::SceneManager::renderBasicQueueGroupObjects(pGroup,om);
	
	// Basic render loop
	// Iterate through priorities
	Ogre::RenderQueueGroup::PriorityMapIterator groupIt = pGroup->getIterator();

	while (groupIt.hasMoreElements())
	{
		Ogre::RenderPriorityGroup* pPriorityGrp = groupIt.getNext();

		// Sort the queue first
		pPriorityGrp->sort(mCameraInProgress);

		// only rend solids in depth and solid light pass
		if(m_CurrentStage == DeferredRendSceneManager::Stage_Depth
		 ||m_CurrentStage == DeferredRendSceneManager::Stage_SolidLit
		 )
		{
		    renderObjects(pPriorityGrp->getSolidsBasic(), om, true, true);
		}
		else if(m_CurrentStage == DeferredRendSceneManager::Stage_TransparentLit)
		{
			// Do unsorted transparent
			renderObjects(pPriorityGrp->getTransparentsUnsorted(), om, true, true);
			
			// Do transparent (always descending)
			renderObjects(pPriorityGrp->getTransparents(), 	Ogre::QueuedRenderableCollection::OM_SORT_DESCENDING, true, true);
		}
	}// for each priority
}
//-----------------------------------------------------------------------
void DeferredRendSceneManager::SetDepthRendMaterial(const Ogre::String & depthMatName)
{
	Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().getByName(depthMatName);
	if (mat.isNull())
	{
		OGRE_EXCEPT(Ogre::Exception::ERR_ITEM_NOT_FOUND , "Cannot locate material called '" + depthMatName + "'", "SceneManager::setShadowTextureCasterMaterial");
	}
	mat->load();
	if (!mat->getBestTechnique())
	{
		// unsupported
		m_DepthPass = 0;
	}
	else
	{
		m_DepthPass = mat->getBestTechnique()->getPass(0);
	}
}
//-----------------------------------------------------------------------
void DeferredRendSceneManager::SetCurrentRendStage(DeferredRendSceneManager::DeferredRendStage stage)
{
	m_CurrentStage = stage;
}

//-----------------------------------------------------------------------
const Ogre::String DeferredRendSceneManagerFactory::FACTORY_TYPE_NAME = "DeferredRendSceneManager";
//-----------------------------------------------------------------------
void DeferredRendSceneManagerFactory::initMetaData(void) const
{
	mMetaData.typeName = FACTORY_TYPE_NAME;
	mMetaData.description = "Scene Purpose for  Deferred Rend";
	mMetaData.sceneTypeMask = (Ogre::ST_INTERIOR << 1);
	mMetaData.worldGeometrySupported = false;
}
//-----------------------------------------------------------------------
Ogre::SceneManager* DeferredRendSceneManagerFactory::createInstance(const Ogre::String& instanceName)
{
	return OGRE_NEW DeferredRendSceneManager(instanceName);
}
//-----------------------------------------------------------------------
void DeferredRendSceneManagerFactory::destroyInstance(Ogre::SceneManager* instance)
{
	OGRE_DELETE instance;
}
