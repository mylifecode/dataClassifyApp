#include "stdafx.h"
#include "ShadowMap.h"
#include "MisNewTraining.h"
#include "TrainingMgr.h"
static const int  NUM_MATS = 31;
static string g_materials[NUM_MATS] ={
			"zigongjingtai",
			"zigongfumo",
			"xueguan",
			"biaopi",
			"qiuti",
			"fubi",
			"diaphragm",
			"Intestinesm",
			"large_intestine",
			"sig_diaphragm",
			"dangchanginlanwei2",
			"Standardmaterial",
			"ga_diaphragm",
			"stomach_new",
			"Mat_XiaoWangMo",
			"21-Default",
			"21-Default1",
			"ElectrocoagulationTrain/dizuo",
			"ElectrocoagulationTrain/biaopi",
			"ElectrocoagulationTrain/flesh",
			"06-Default",
			"SecondTietu",
			"NewTrain/biaopi",
			"NBT_02",
			"NBT_01",
			"Material#51",
			"NewZigongjingtai",
			"NewZigong",
			"rou",
			"envolop_body" ,
			"MisMedical/lanwei_DomeActive_B"

};

//==================================================================================================================================
bool Vector3Equal(const Ogre::Vector3& l,const Ogre::Vector3& r)
{
	if ( FLT_EPSILON < abs(l.x - r.x))
		return false;
	if ( FLT_EPSILON < abs(l.y - r.y))
		return false;
	if ( FLT_EPSILON < abs(l.z - r.z))
		return false;
	return true;
}

CShadowMap::ShadowRendListener::ShadowRendListener():m_ShadowMap(0)
{

}

CShadowMap::ShadowRendListener::~ShadowRendListener()
{
	if(m_ShadowMap)
	{
	   m_ShadowMap->RemoveListener(this);
	   m_ShadowMap = 0;
	}
}

void CShadowMap::ShadowRendListener::SetShadowMap(CShadowMap * shadowmap)
{
	m_ShadowMap = shadowmap;
	if(m_ShadowMap)
	   m_ShadowMap->AddListener(this);
}

CShadowMap::CShadowMap(void)
{
	m_DepthRendTargrt = NULL;
	m_bShadow = false;
	m_Camera_Shadow = NULL;
	m_sceneMgr_Shadow = NULL;

	m_bDirty = true;
	m_IsDepthRangeSetted = false;
	m_ShadowLightDir = Ogre::Vector3::NEGATIVE_UNIT_Z;
	m_ShadowLightPos = Ogre::Vector3(0.0f , 0 , -0.1);
}
//==================================================================================================================================
CShadowMap::~CShadowMap()
{
	destoryShadowMap();
}
//==================================================================================================================================
void CShadowMap::AddListener(CShadowMap::ShadowRendListener * listener)
{
	for(size_t c = 0 ; c < m_listener.size() ; c++)
	{
		if(m_listener[c] == listener)
		   return;
	}
	m_listener.push_back(listener);
}
//==================================================================================================================================
void CShadowMap::RemoveListener(CShadowMap::ShadowRendListener * listener)
{
	for(size_t c = 0 ; c < m_listener.size() ; c++)
	{
		if(m_listener[c] == listener)
		{
			m_listener.erase(m_listener.begin()+c);
			return;
		}
	}
}
//==================================================================================================================================
void CShadowMap::destoryShadowMap()
{
	//remove schema listener
	Ogre::MaterialManager::getSingleton().removeListener(this);

	//remove rend target before remove depth map
	if(m_DepthRendTargrt)
	{
	   m_DepthRendTargrt->removeListener(this);

	   Ogre::Root::getSingleton().getRenderSystem()->detachRenderTarget(m_DepthRendTargrt->getName());

	   m_DepthRendTargrt = 0;
	}

	//remove depth texture
	if(m_DepthMap.isNull() == false)
	{
		Ogre::TextureManager::getSingleton().remove(m_DepthMap->getName());
		m_DepthMap.setNull();
	}

	//remove all listener
	for(size_t c = 0 ; c < m_listener.size() ; c++)
	{
		m_listener[c]->m_ShadowMap = 0;
	}
	m_listener.clear();

	m_Camera_Shadow = 0;
	m_sceneMgr_Shadow = 0;
	m_bShadow = false;
	m_IsDepthRangeSetted = false;
}
//==================================================================================================================================
void CShadowMap::updateShadowCamera(Ogre::Light* light  , const Ogre::Vector3 & hintUpDir)
{
	if((light == 0) || (m_Camera_Shadow == 0))
	   return;

	Ogre::Node *  lightparentnode = light->getParentNode();
	Ogre::Vector3 lightcamerapos = m_ShadowLightPos;//light->getPosition() ;
	Ogre::Vector3 lightcameradir = m_ShadowLightDir;//light->getDirection();
	lightcameradir.normalise();

	if (lightparentnode)
	{
		// Ok, update with SceneNode we're attached to
		const Ogre::Quaternion& parentOrientation = lightparentnode->_getDerivedOrientation();
		const Ogre::Vector3& parentPosition = lightparentnode->_getDerivedPosition();
		lightcameradir = parentOrientation * lightcameradir;
		lightcamerapos = (parentOrientation * lightcamerapos) + parentPosition;
	}
	//if( !Vector3Equal(m_Camera_Shadow->getPosition(), lightcamerapos) ||  !Vector3Equal(m_Camera_Shadow->getDirection() , lightcameradir))
	//{
	Ogre::Vector3 rightVec = lightcameradir.crossProduct(hintUpDir);
	Ogre::Vector3 DstupVec = rightVec.crossProduct(lightcameradir);
	DstupVec.normalise();


	m_Camera_Shadow->setPosition(lightcamerapos);
	
	m_Camera_Shadow->setDirection(lightcameradir);
	
	Ogre::Vector3 SrcUpVec = m_Camera_Shadow->getUp();

	Ogre::Quaternion rotUp = SrcUpVec.getRotationTo(DstupVec);

	Ogre::Quaternion srcort = m_Camera_Shadow->getOrientation();

	m_Camera_Shadow->setOrientation(rotUp * srcort);

	m_bDirty = true;//mark is dirty
	//}
}
//==================================================================================================================================
void CShadowMap::initConfig(Ogre::SceneManager* sceneManager , Ogre::Radian camFov , float camAspectRate)
{
	SY_ASSERT(m_DepthMap.isNull() == false && "shadow map must be create before init config");
 	
	//set scene manager to current scene
	m_sceneMgr_Shadow = sceneManager;

	//create shadow camera in current scene
	if(!m_Camera_Shadow)
	{
		Ogre::Radian shadowLightFov = camFov * 1.2f * camAspectRate;//

		m_Camera_Shadow = m_sceneMgr_Shadow->createCamera("lightCamera");
		m_Camera_Shadow->setNearClipDistance(0.1f);
		m_Camera_Shadow->setFarClipDistance(200.0);
		//m_lightCamera->setProjectionType(Ogre::PT_ORTHOGRAPHIC);
		//m_lightCamera->setOrthoWindow(40 , 40);
		m_Camera_Shadow->setAspectRatio(1.0f);//);
		m_Camera_Shadow->setFOVy(shadowLightFov);//Ogre::Degree(80.0));//Ogre::Degree(90.0));
		m_bDirty = true;
	}	

	Ogre::String depthTechSchema = "SM_DrawDepth";

	//bind shadow camera to shadow render target
	if (m_DepthRendTargrt)
	{
		Ogre::Viewport * pV = m_DepthRendTargrt->addViewport( m_Camera_Shadow ); 
		pV->setClearEveryFrame(true);                        //设置每一帧都清理当前后台缓冲
		pV->setBackgroundColour(Ogre::ColourValue(1.0f));       //设置当前后台缓冲背景
		pV->setOverlaysEnabled(false); 
		pV->setMaterialScheme(depthTechSchema);
		m_DepthRendTargrt->addListener(this);
	}

	//
	Ogre::MaterialManager::getSingleton().addListener( this , depthTechSchema);
}
//==================================================================================================================================
void CShadowMap::createShadowTexture(int width, int height)
{
	if(m_DepthMap.isNull())
	{
		m_DepthMap = Ogre::TextureManager::getSingleton().createManual("EyeFrontDepthTexture",
																		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
																		Ogre::TEX_TYPE_2D,
																		width,	height,	
																		0,
																		Ogre::PF_FLOAT32_R,
																		Ogre::TU_RENDERTARGET);

		if(m_DepthMap.isNull() == false)
		{
		   m_DepthRendTargrt = m_DepthMap->getBuffer()->getRenderTarget();
		  
		   m_DepthRendTargrt->setAutoUpdated(false);//manually update
		  
		   SY_ASSERT(m_DepthRendTargrt != 0);
		}
		else
		{
			m_bShadow = false;
		}
	}
}
//==================================================================================================================================
void CShadowMap::GetDepthRange(float & dmin , float & dmax)
{
	if(!m_IsDepthRangeSetted)
	{
		Ogre::Matrix4 ShadowLightProjM4 = m_Camera_Shadow->getProjectionMatrixWithRSDepth();
		
		float  NearDist = m_Camera_Shadow->getNearClipDistance();
		
		float  FarDist  = m_Camera_Shadow->getFarClipDistance();
		
		m_DepthMin = 0;//(ShadowLightProjM4*Ogre::Vector4(0,0,-NearDist,1)).z;
		
		m_DepthMax = FarDist;//(ShadowLightProjM4*Ogre::Vector4(0,0,-FarDist,1)).z;
		
		m_IsDepthRangeSetted = true;
	}

	dmin = m_DepthMin;
	dmax = m_DepthMax;
}
//==================================================================================================================================
void CShadowMap::_update(Ogre::Light* light , const Ogre::Vector3 & hintUpDir)
{
	return;//temp since scene mgr is changed need do for shadow tool
	if(!m_bShadow)
		return;

	MisNewTraining * hostTrain = dynamic_cast<MisNewTraining*>(CTrainingMgr::Instance()->GetCurTraining());
	if(hostTrain == 0)
	   return;

	
	//update shadow camera via the light position and direction
	updateShadowCamera(light , hintUpDir);
	
	m_DepthRendTargrt->update(true);//放到外面以保證不會因為未更新矩陣丟失鉗子
	
	if (m_bDirty)//這樣可以減少不必要的Matrix更新操作
	{
		float  NearDist , FarDist;

		GetDepthRange(NearDist , FarDist);

		float  depthRange = FarDist - NearDist;
		
		Ogre::Vector4 shadowDepthRange = Ogre::Vector4(NearDist , FarDist, depthRange,1.0f / depthRange);

		// set the projection matrix we need
		static const Ogre::Matrix4 CLIP_SPACE_TO_IMAGE_SPACE(0.5,    0,    0,  0.5,
															 0,   -0.5,    0,  0.5,
															 0,      0,    1,    0,
															 0,      0,    0,    1);

		Ogre::Matrix4 ShadowLightViewM4 = hostTrain->m_pLargeCamera->getViewMatrix(true);
		
		Ogre::Matrix4 ShadowLightProjM4 = hostTrain->m_pLargeCamera->getProjectionMatrixWithRSDepth();

		Ogre::Matrix4 ShadowTexViewProjM4 = CLIP_SPACE_TO_IMAGE_SPACE*ShadowLightProjM4*ShadowLightViewM4;

		setMatrixToShader("default_template", "TexViewProjM4", VERTEX_PROGRAME,  ShadowTexViewProjM4);
		SetVector4ToShader("default_template", "shadowDepthRange", FRAGMENT_PROGRAME,  shadowDepthRange);

		setMatrixToShader("defaultStatic_template", "TexViewProjM4", VERTEX_PROGRAME,  ShadowTexViewProjM4);
		SetVector4ToShader("defaultStatic_template", "shadowDepthRange", FRAGMENT_PROGRAME,  shadowDepthRange);

		int RevMatNum = NUM_MATS;
		for (int i = 0; i < RevMatNum;i++)
		{
			setMatrixToShader(g_materials[i], "TexViewProjM4", VERTEX_PROGRAME,  ShadowTexViewProjM4);
			SetVector4ToShader(g_materials[i], "shadowDepthRange", FRAGMENT_PROGRAME,  shadowDepthRange);
		}

		std::vector<Ogre::String> TrainmatRecShadow;
		hostTrain->GetAllMaterialReceiveShadow(TrainmatRecShadow);
		for(size_t c = 0 ; c < TrainmatRecShadow.size() ; c++)
		{
			setMatrixToShader(TrainmatRecShadow[c], "TexViewProjM4", VERTEX_PROGRAME,  ShadowTexViewProjM4);
			SetVector4ToShader(TrainmatRecShadow[c], "shadowDepthRange", FRAGMENT_PROGRAME,  shadowDepthRange);
		}
		m_bDirty = false;
	}
}
//==============================================================================================================================================================
void CShadowMap::setMatrixToShader(Ogre::String materialName, Ogre::String paraName, CG_PROGRAME_TYPE type, Ogre::Matrix4 matrix, int technique, int pass)
{
	Ogre::GpuProgramParametersSharedPtr ShaderPtr = GetShaderParamterPtr(materialName, type, technique , pass);

	if(ShaderPtr.isNull() == false)
	{
	   if (ShaderPtr->_findNamedConstantDefinition(paraName, false))
	   {
		   ShaderPtr->setNamedConstant(paraName, matrix);
	   }
	}
}
//==============================================================================================================================================================
void CShadowMap::SetVector4ToShader(Ogre::String materialName, Ogre::String paraName, CG_PROGRAME_TYPE type, Ogre::Vector4 vec4, int technique, int pass)
{
	Ogre::GpuProgramParametersSharedPtr ShaderPtr = GetShaderParamterPtr(materialName, type, technique , pass);

	if(ShaderPtr.isNull() == false)
	{
		if (ShaderPtr->_findNamedConstantDefinition(paraName, false))
		{
			ShaderPtr->setNamedConstant(paraName, vec4);
		}
	}
}
//==============================================================================================================================================================
void CShadowMap::preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
{
	//Ogre::SceneManager::MovableObjectIterator it = m_sceneMgr_Shadow->getMovableObjectIterator("Entity");//m_pSceneMgr管理的实体列表，“SceneNode”是遍历所有的场景结点   
	//while(it.hasMoreElements())//遍历整个列表   
	//{ 
	//	Ogre::MovableObject * object = it.getNext();
	//	Ogre::String _name = object->getName();
	//	if(_name.find("T-") != Ogre::String::npos || _name.find("Clip") != Ogre::String::npos)
	//		object->setVisible(true);
	//	else
	//		object->setVisible(false);
	//}

	//it = m_sceneMgr_Shadow->getMovableObjectIterator("DynamicStripObj");//m_pSceneMgr管理的实体列表，“SceneNode”是遍历所有的场景结点   
	//while(it.hasMoreElements())//遍历整个列表   
	//{ 
	//	Ogre::MovableObject * object = it.getNext();
	//	object->setVisible(false);
	//}
	for(size_t c = 0 ; c < m_listener.size() ; c++)
	{
		m_listener[c]->preRenderShadowDepth();
	}
	std::vector<Ogre::String> particles;
	particles.push_back("smoke");
	particles.push_back("spark");
	for(size_t i = 0 ; i < particles.size(); i++)
	{
		Ogre::String parname = particles[i];
		if(m_sceneMgr_Shadow->hasParticleSystem(parname))
		{
			Ogre::ParticleSystem * psys = m_sceneMgr_Shadow->getParticleSystem(parname);
			psys->setVisible(false);
		}
	}
}

void CShadowMap::postRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
{
	//if(m_bShadow)
	//{
		//writeDepthTexture();
	//	Ogre::SceneManager::MovableObjectIterator it = m_sceneMgr_Shadow->getMovableObjectIterator("Entity");//m_pSceneMgr管理的实体列表，“SceneNode”是遍历所有的场景结点   
	//	while(it.hasMoreElements())//遍历整个列表   
	//	{ 
	//		Ogre::MovableObject * object = it.getNext();
	//		object->setVisible(true);
	//	}

	//	it = m_sceneMgr_Shadow->getMovableObjectIterator("DynamicStripObj");//m_pSceneMgr管理的实体列表，“SceneNode”是遍历所有的场景结点   
	//	while(it.hasMoreElements())//遍历整个列表   
	//	{ 
	//		Ogre::MovableObject * object = it.getNext();
	//		object->setVisible(true);
	//	}
	//}
	for(size_t c = 0 ; c < m_listener.size() ; c++)
	{
		m_listener[c]->postRenderShadowDepth();
	}
	std::vector<Ogre::String> particles;
	particles.push_back("smoke");
	particles.push_back("spark");
	for(size_t i = 0 ; i < particles.size(); i++)
	{
		Ogre::String parname = particles[i];
		if(m_sceneMgr_Shadow->hasParticleSystem(parname))
		{
			Ogre::ParticleSystem * psys = m_sceneMgr_Shadow->getParticleSystem(parname);
			psys->setVisible(true);
		}
	}
}
//===============================================================================================
Ogre::Technique* CShadowMap::handleSchemeNotFound(unsigned short schemeIndex, 
												  const Ogre::String& schemeName, 
												  Ogre::Material* originalMaterial, 
												  unsigned short lodIndex, 
												  const Ogre::Renderable* rend)
{

	Ogre::String matName = originalMaterial->getName();

	//check particle
	if(matName == "SmokeParticle" || matName == "MatEffect/Spark")
	   return NULL;

	
	if(matName == "MisMedical/DefaultCutFace" || matName == "waterColumn")
	   return NULL;

    if (matName == "MisMedic/VeinConnCalot" || matName == "MisMedic/VeinConnBottom" || matName == "MisMedic/VeinConnKidney")
	   return NULL;
	
	if(originalMaterial->getNumTechniques() > 1)//临时处理Ogre对某些Material会 无线创建technique的bug，后续请继续检查原因
	   return NULL;
	
	//create shadow depth scheme for this material
	Ogre::Technique * gBufferTech = originalMaterial->createTechnique();
	gBufferTech->removeAllPasses();
	gBufferTech->setSchemeName(schemeName);

	Ogre::Pass * newPass = gBufferTech->createPass();
	newPass->setVertexProgram("dynamicdepth_vp") ;
	Ogre::GpuProgramParametersSharedPtr gpuParams = newPass->getVertexProgramParameters();
	try
	{
		float NearDist;

		float FarDist;
		
		GetDepthRange(NearDist , FarDist);

		float  depthRange = FarDist - NearDist;

		Ogre::Vector4 shadowDepthRange = Ogre::Vector4(NearDist , FarDist, depthRange,1.0f / depthRange);
		gpuParams->setNamedAutoConstant("worldViewProj",  Ogre::GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
		gpuParams->setNamedConstant("depthRange",  shadowDepthRange);
	}
	catch(...)
	{}
	newPass->setFragmentProgram("dynamicdepth_fp") ;
	
	return gBufferTech;
}

void CShadowMap::writeDepthTexture()
{
	if(m_DepthRendTargrt)
	{
		m_DepthRendTargrt->writeContentsToFile("c:\\depthMap_test.bmp");
	}
}
