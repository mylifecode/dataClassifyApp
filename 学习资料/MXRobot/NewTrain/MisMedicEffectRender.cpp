#include "MisMedicEffectRender.h"
#include "OrganBloodMotionSimulator.h"
#include "TextureBloodEffect.h"
#include "TextureWaterEffect.h"//__/__
#include "ITool.h"
#include "OgreMaterial.h"
#include "MXOgreGraphic.h"

MisMedicEffectRender::MisMedicEffectRender()
{
	//m_BurnWhiteSrcImageName = "DefaultBurnTex.dds";
	Ogre::Root::getSingleton().getRenderSystem()->addListener(this);
	m_deviateScale = 1.0f;
	m_TimeSinceLastBloodParUpdate = 0;
	m_TimeSinceLastVeinBloodUpdate = 0;
	m_BeCreated = false;
	m_BuildQuadObj = 0;
	m_pMarkViewPort = NULL;
}

MisMedicEffectRender::~MisMedicEffectRender()
{
	Ogre::Root::getSingleton().getRenderSystem()->removeListener(this);
}	

void MisMedicEffectRender::OnDeviceLost()
{
	//if(m_QuantityTexturePtr.isNull() == false)
	//{
	//	Ogre::HardwarePixelBufferSharedPtr pixelptr = m_QuantityTexturePtr->getBuffer();// convertToImage(m_CacheQuantityTexImage);
	//}
	
	//if(m_BurnWhiteTexturePtr.isNull() == false)
	 //  m_BurnWhiteTexturePtr->convertToImage(m_CacheBurnWhiteTexImage);
}	

void MisMedicEffectRender::OnDeviceReset()
{
	//if(m_PermantEffectTexture.isNull() == false)
		//m_PermantEffectTexture->loadImage(m_CachePermantImage);
	Initialize();//temple fix
	
	//if(m_BurnWhiteTexturePtr.isNull() == false && m_CacheBurnWhiteTexImage.getSize() > 0)
	 //  m_BurnWhiteTexturePtr->loadImage(m_CacheBurnWhiteTexImage);
	
	//if(m_QuantityTexturePtr.isNull() == false &&  m_CacheQuantityTexImage.getSize() > 0)
	//   m_QuantityTexturePtr->loadImage(m_CacheQuantityTexImage);
}

void MisMedicEffectRender::SetBloodBleedTexture(const Ogre::String & bleedtex)
{
	//Ogre::TexturePtr texbleed = Ogre::TextureManager::getSingleton().load(bleedtex , Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	//ApplyTextureToMaterial(m_ComposedEffectMaterialPtr , texbleed , "BloodBleedMap");
}
// void MisMedicEffectRender::ApplyTextureToMaterial(Ogre::MaterialPtr mat , Ogre::TexturePtr tex , const Ogre::String & unitName)
// {	
// 	if(mat.isNull() == false && tex.isNull() == false)
// 	{
// 		Ogre::Technique * tech = mat->getTechnique(0);
// 
// 		if(tech->getNumPasses() > 0)
// 		{
// 			Ogre::Pass * pass = tech->getPass(0);
// 			for (int  t = 0; t < pass->getNumTextureUnitStates() ; t++)
// 			{
// 				Ogre::TextureUnitState * texunit = pass->getTextureUnitState(t);
// 				if(texunit->getTextureNameAlias() == unitName )
// 				   texunit->setTextureName(tex->getName());
// 			}
// 		}
// 	}
// }

void MisMedicEffectRender::eventOccurred(const Ogre::String& eventName, 
								  const Ogre::NameValuePairList* parameters)
{
	if(eventName== "DeviceLost")
	{
		OnDeviceLost();
	}
	else if(eventName== "DeviceRestored")
	{
		OnDeviceReset();
	}
}

void MisMedicEffectRender::Initialize()
{	
	if (m_QuantityTexturePtr1.isNull() == false)
		InitRenderTexture(m_QuantityTexturePtr1, Ogre::ColourValue(0.0f, 0.0f, 0.0f, 0.0f));

	if (m_QuantityTexturePtr2.isNull() == false)
		InitRenderTexture(m_QuantityTexturePtr2, Ogre::ColourValue(0.0f, 0.0f, 0.0f, 0.0f));

	//if(m_BurnWhiteTexturePtr.isNull() == false)
	////{
	//	InitRenderTextureWithTexture(m_BurnWhiteTexturePtr , m_BurnWhiteSrcImageName);
	//}

	if(m_DynamicBloodTexturePtr.isNull() == false)
	{
	   Ogre::ColourValue clearColor(0,0,0,0);
	   InitRenderTexture(m_DynamicBloodTexturePtr , clearColor);
	   //dynamic effect need clear frame
	   m_pDynamicBloodTexViewPort->setBackgroundColour(clearColor);
	   m_pDynamicBloodTexViewPort->setClearEveryFrame(true);
	}

		//if(m_MixAlphaTexturePtr.isNull() == false)
	//{
	 //  InitRenderTexture(m_MixAlphaTexturePtr , Ogre::ColourValue(0,0,0,0));
	//}
#if GENERATE_MESH_TEXTURE
	if(m_meshTexture.isNull() == false)
	{
		InitRenderTexture(m_meshTexture , Ogre::ColourValue(0,0,0,0));
	}
#endif

}
void MisMedicEffectRender::Destory()
{
	if(m_BeCreated == false)
	   return;

	m_BeCreated = false;
	
	//destroy all texture
	m_QuantityTexturePtr.setNull();

	if (m_QuantityTexturePtr1.isNull() == false)
	{
		Ogre::TextureManager::getSingleton().remove(m_QuantityTexturePtr1->getHandle());
		m_QuantityTexturePtr1.setNull();
	}

	if (m_QuantityTexturePtr2.isNull() == false)
	{
		Ogre::TextureManager::getSingleton().remove(m_QuantityTexturePtr2->getHandle());
		m_QuantityTexturePtr2.setNull();
	}

#if GENERATE_MESH_TEXTURE
	if(m_meshTexture.isNull() == false)
	{
		Ogre::TextureManager::getSingleton().remove(m_meshTexture->getHandle());
		m_meshTexture.setNull();
	}

#endif

	//if(m_ComposedEffectTexturePtr.isNull() == false)
	//{
	//	Ogre::TextureManager::getSingleton().remove(m_ComposedEffectTexturePtr->getHandle());
	//	m_ComposedEffectTexturePtr.setNull();
	//}

	if(m_DynamicBloodTexturePtr.isNull() == false)
	{
		Ogre::TextureManager::getSingleton().remove(m_DynamicBloodTexturePtr->getHandle());
		m_DynamicBloodTexturePtr.setNull();
	}

//	if(m_BurnWhiteTexturePtr.isNull() == false)
	//{
	//	Ogre::TextureManager::getSingleton().remove(m_BurnWhiteTexturePtr->getHandle());
	//	m_BurnWhiteTexturePtr.setNull();
	//}

	//destroy all scene manager
	if(m_pQuantityTexSceneMgr)
	{
	   if(m_BuildQuadObj)
	   {
		  m_pQuantityTexSceneMgr->destroyManualObject(m_BuildQuadObj);
		  m_BuildQuadObj = 0;
	   }
	   if (m_MarkQuadObj)
	   {
		   m_pQuantityTexSceneMgr->destroyManualObject(m_MarkQuadObj);
		   m_MarkQuadObj = 0;
	   }	   

	   Ogre::Root::getSingleton().destroySceneManager(m_pQuantityTexSceneMgr);
	   m_pQuantityTexSceneMgr = 0;
	}
	m_DefaultHeatBrandTexPtr = Ogre::TexturePtr();//R G B A R-white value G- value B-heat value  A-BloodValue

	//destroy all clone material
	if(m_HeatOrganFaceMaterial.isNull() == false)
	{
		Ogre::MaterialManager::getSingleton().remove(m_HeatOrganFaceMaterial->getHandle());
		m_HeatOrganFaceMaterial.setNull();

	}

	if (m_GravityMaterial.isNull() == false)
	{
		Ogre::MaterialManager::getSingleton().remove(m_GravityMaterial->getHandle());
		m_GravityMaterial.setNull();
	}

	//if (m_BurnSpreadMaterial.isNull() == false)
	//{
	//	Ogre::MaterialManager::getSingleton().remove(m_BurnSpreadMaterial->getHandle());
		//m_BurnSpreadMaterial.setNull();
	//}

	if (m_BleedingPointMaterial.isNull() == false)
	{
		Ogre::MaterialManager::getSingleton().remove(m_BleedingPointMaterial->getHandle());
		m_BleedingPointMaterial.setNull();
	}
	
	//if(m_ComposedEffectMaterialPtr.isNull() == false)
	//{
	//	Ogre::MaterialManager::getSingleton().remove(m_ComposedEffectMaterialPtr->getHandle());
	//	m_ComposedEffectMaterialPtr.setNull();
	//}
}


void MisMedicEffectRender::Create(const std::string & OrganMaterialName, int efftexWidth, int efftexHeight, Ogre::String name)
{
	if(m_BeCreated == true)
		return;

	m_BeCreated = true;
	
	m_OrganMaterialName = OrganMaterialName;

	Ogre::RenderTarget * rttTex = 0;
	
	//heat,blood,etc texture in 4 channel
	Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName("OrganSurfEffect/Congulation");

	m_HeatOrganFaceMaterial = material->clone("QuantityMaterial_" + name);
	

	material = Ogre::MaterialManager::getSingleton().getByName("OrganSurfEffect/MixTexAlphaPaint");
	//m_AlphaPaintMaterial = material->clone("MixTexAlphaPaintMat_"+name);

	material = Ogre::MaterialManager::getSingleton().getByName("OrganSurfEffect/BleedingPoint");
	m_BleedingPointMaterial = material->clone("BleedingPointMat_" + name);

	//material = Ogre::MaterialManager::getSingleton().getByName("OrganSurfEffect/Copy");
	//m_CopyMaterial = material->clone("CopyMat_" + name);

	material = Ogre::MaterialManager::getSingleton().getByName("OrganSurfEffect/Gravity");
	m_GravityMaterial = material->clone("GravityMat_" + name);

	if (efftexWidth >= 2048)//现在dx11 下vertex program 传uniform参数似乎有问题 不知道是不是Ogre1.9的bug，暂时分开两个shader
	{
		m_GravityMaterial->getTechnique(0)->getPass(0)->setVertexProgram("OrganSurfGravity_VP_2048");
	}
	else
	{
		m_GravityMaterial->getTechnique(0)->getPass(0)->setVertexProgram("OrganSurfGravity_VP_1024");

	}

	//material = Ogre::MaterialManager::getSingleton().getByName("OrganSurfEffect/SimBurnSpread");
	//m_BurnSpreadMaterial = material->clone("BurnSpreadMat_" + name);


	m_QuantityTexturePtr1 = Ogre::TextureManager::getSingleton().createManual("QuantityTex_1"+name ,
																			  Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME , 
																			  Ogre::TEX_TYPE_2D , 
																			  efftexWidth , 
																			  efftexHeight , 
																			  0 , 
																			 Ogre::PF_A8B8G8R8 , 
																			 Ogre::TU_RENDERTARGET);

	m_QuantityTexturePtr2 = Ogre::TextureManager::getSingleton().createManual("QuantityTex2_" + name,
		                                                                      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		                                                                      Ogre::TEX_TYPE_2D,
		                                                                      efftexWidth,
		                                                                      efftexHeight,
		                                                                       0,
																			   Ogre::PF_A8B8G8R8,
		                                                                      Ogre::TU_RENDERTARGET);

	m_QuantityTexturePtr = m_QuantityTexturePtr1;
	//m_QuantityTextureNum = 1;

	m_DefaultHeatBrandTexPtr = Ogre::TextureManager::getSingleton().load("cogBrandDefault.tga", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	m_bHeatBrandName = "UteriaBurn.tga";

	//ApplyTextureToMaterial(m_HeatOrganFaceMaterial , m_QuantityTexturePtr , "HeatChannelMap");


	bool hasTest = Ogre::Root::getSingleton().hasSceneManager("QuantityTexSceneMgr"+name);
	if (hasTest == false)
	{
		m_pQuantityTexSceneMgr = Ogre::Root::getSingleton().createSceneManager(Ogre::ST_GENERIC, "QuantityTexSceneMgr"+name);
	}
	else
	{
		m_pQuantityTexSceneMgr = Ogre::Root::getSingleton().getSceneManager("QuantityTexSceneMgr"+name);
	}

	hasTest = m_pQuantityTexSceneMgr->hasCamera("EffectCamera");
	if (hasTest == false)
	{
		m_pQuantityTexCamera  = m_pQuantityTexSceneMgr->createCamera("EffectCamera");
	}
	else
	{
		m_pQuantityTexCamera  = m_pQuantityTexSceneMgr->getCamera("EffectCamera");
	}

	rttTex = m_QuantityTexturePtr1->getBuffer()->getRenderTarget();
	m_pQuantityTexViewPort1 = rttTex->addViewport(m_pQuantityTexCamera);

	rttTex = m_QuantityTexturePtr2->getBuffer()->getRenderTarget();
	m_pQuantityTexViewPort2 = rttTex->addViewport(m_pQuantityTexCamera);


	m_MarkTexturePtr = Ogre::TextureManager::getSingleton().createManual("MarkTexture_" + name,
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		Ogre::TEX_TYPE_2D,
		efftexWidth,
		efftexHeight,
		0,
		Ogre::PF_A8R8G8B8,
		Ogre::TU_RENDERTARGET);
	rttTex = m_MarkTexturePtr->getBuffer()->getRenderTarget();
	rttTex->setAutoUpdated(false);
	 
	m_pMarkViewPort = rttTex->addViewport(m_pQuantityTexCamera);
	m_pMarkViewPort->setClearEveryFrame(true);
	m_pMarkViewPort->setBackgroundColour(Ogre::ColourValue(0,0,0,0));


	
	

	/*composed effect texture*/
	/*m_ComposedEffectTexturePtr = Ogre::TextureManager::getSingleton().createManual("ComposedTex_"+name ,
										Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME , 
										Ogre::TEX_TYPE_2D , 
										efftexWidth , 
										efftexHeight , 
										0 , 
										Ogre::PF_A8R8G8B8 , 
										Ogre::TU_RENDERTARGET);
	rttTex = m_ComposedEffectTexturePtr->getBuffer()->getRenderTarget();

	m_pComposedTexViewPort = rttTex->addViewport(m_pQuantityTexCamera);*/

	//dynamic water
	/*m_DynamicWaterTexturePtr = Ogre::TextureManager::getSingleton().createManual("DynamicWaterTex_"+name ,
										Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME , 
										Ogre::TEX_TYPE_2D , 
										efftexWidth , 
										efftexHeight , 
										0 , 
										Ogre::PF_A8R8G8B8 , 
										Ogre::TU_RENDERTARGET);
	rttTex = m_DynamicWaterTexturePtr->getBuffer()->getRenderTarget();*/

	//m_pDynamicWaterTexViewPort = rttTex->addViewport(m_pQuantityTexCamera);

	//Composed water
	/*m_ComposedNormalTexturePtr = Ogre::TextureManager::getSingleton().createManual("ComposedWaterTex_"+name ,
										Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME , 
										Ogre::TEX_TYPE_2D , 
										efftexWidth , 
										efftexHeight , 
										0 , 
										Ogre::PF_A8R8G8B8 , 
										Ogre::TU_RENDERTARGET);
	rttTex = m_ComposedNormalTexturePtr->getBuffer()->getRenderTarget();

	rttTex->addViewport(m_pQuantityTexCamera);*/

	//
	m_DynamicBloodTexturePtr = Ogre::TextureManager::getSingleton().createManual("DynamicBloodTex_"+name ,
										Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME , 
										Ogre::TEX_TYPE_2D , 
										efftexWidth , 
										efftexHeight , 
										0 , 
										Ogre::PF_A8R8G8B8 , 
										Ogre::TU_RENDERTARGET);
	rttTex = m_DynamicBloodTexturePtr->getBuffer()->getRenderTarget();

	rttTex->setAutoUpdated(false);

	m_pDynamicBloodTexViewPort = rttTex->addViewport(m_pQuantityTexCamera);
	

	//
	/*m_BurnWhiteTexturePtr = Ogre::TextureManager::getSingleton().createManual("BurnWhiteTex_"+name ,
																			   Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME , 
																			   Ogre::TEX_TYPE_2D , 
																			   efftexWidth , 
																			   efftexHeight , 
																			   0 , 
																			   Ogre::PF_A8R8G8B8 , 
																			   Ogre::TU_RENDERTARGET);
	rttTex = m_BurnWhiteTexturePtr->getBuffer()->getRenderTarget();

	m_pBurnWhiteTexViewPort = rttTex->addViewport(m_pQuantityTexCamera);*/


#if GENERATE_MESH_TEXTURE
	m_meshTexture = Ogre::TextureManager::getSingleton().createManual("MeshUVTex"+name,
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME , 
		Ogre::TEX_TYPE_2D , 
		2048 , 
		2048 , 
		0 , 
		Ogre::PF_A8R8G8B8 , 
		Ogre::TU_RENDERTARGET);

	rttTex = m_meshTexture->getBuffer()->getRenderTarget();

	m_TexViewPort = rttTex->addViewport(m_pQuantityTexCamera);

#endif

	/*
	m_MixAlphaTexturePtr  = Ogre::TextureManager::getSingleton().createManual("MixAlphaTex_"+name ,
		                                                                      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME , 
		                                                                      Ogre::TEX_TYPE_2D , 
																			  512 , 
																			  512 , 
																			  0 , 
																			  Ogre::PF_A8R8G8B8 , 
																			  Ogre::TU_RENDERTARGET);
	rttTex = m_MixAlphaTexturePtr->getBuffer()->getRenderTarget();

	rttTex->addViewport(m_pQuantityTexCamera);*/

	//initialize texture content
	Initialize();

	//generate owner compose material
	//Ogre::MaterialPtr mat  = Ogre::MaterialManager::getSingleton().getByName("OrganSurfEffect/ComposeEffect");
	//m_ComposedEffectMaterialPtr = mat->clone("ComposeMat"+name);
	//ApplyTextureToMaterial(m_ComposedEffectMaterialPtr , m_QuantityTexturePtr , "HeatChannelMap");
	//ApplyTextureToMaterial(m_ComposedEffectMaterialPtr , m_DynamicBloodTexturePtr , "DynamicBloodMap");
	//ApplyTextureToMaterial(m_ComposedEffectMaterialPtr , m_BurnWhiteTexturePtr , "BurnWhiteChannelMap");
	//m_ComposedEffectMaterialPtr->load();//reload for prevent lag in first time


	static int countbuild = 0;

	//dynamic blood build material
	m_DynBloodBuildMatPtr = Ogre::MaterialManager::getSingleton().getByName("OrganSurfEffect/BloodDropStream");
	m_DynBloodBuildMatPtr = m_DynBloodBuildMatPtr->clone("DynBloodBuild" + Ogre::StringConverter::toString(countbuild++));
	m_DynBloodBuildMatPtr->load();

	//bleed material
	m_BleedBuildMatPtr = Ogre::MaterialManager::getSingleton().getByName("OrganSurfEffect/Bleed");
	m_BleedBuildMatPtr = m_BleedBuildMatPtr->clone("BleedBuild" + Ogre::StringConverter::toString(countbuild++));
	m_BleedBuildMatPtr->load();

	m_WaterWashBloodMatPtr = Ogre::MaterialManager::getSingleton().getByName("OrganSurfEffect/WaterWashBlood");
	m_WaterWashBloodMatPtr = m_WaterWashBloodMatPtr->clone("WashBlood" + Ogre::StringConverter::toString(countbuild++));
	m_WaterWashBloodMatPtr->load();

	ApplyTextureToMaterial(m_WaterWashBloodMatPtr , m_QuantityTexturePtr , "HeatChannelMap");

	//vein blood material
	m_VeinBloodBuildMatPtr = Ogre::MaterialManager::getSingleton().getByName("OrganSurfEffect/StripBleed");
	m_VeinBloodBuildMatPtr = m_VeinBloodBuildMatPtr->clone("VeinBloodBuild" + Ogre::StringConverter::toString(countbuild++));
	m_VeinBloodBuildMatPtr->load();

	//dynamic water normal material
	//m_DynamicWaterMaterialPtr = Ogre::MaterialManager::getSingleton().getByName("OrganSurfEffect/NormalEffect");
	//m_DynamicWaterMaterialPtr = m_DynamicWaterMaterialPtr->clone("DynWaterBuild" + Ogre::StringConverter::toString(countbuild++));
	//m_DynamicWaterMaterialPtr->load();

	m_DynamicWaterTexMaterialPtr = Ogre::MaterialManager::getSingleton().getByName("OrganSurfEffect/WaterSteamTex");
	m_DynamicWaterTexMaterialPtr = m_DynamicWaterTexMaterialPtr->clone("DynWaterTexBuild" + Ogre::StringConverter::toString(countbuild++));
	m_DynamicWaterTexMaterialPtr->load();

	//compose water normal material
	//m_ComposedNormalMaterialPtr = Ogre::MaterialManager::getSingleton().getByName("OrganSurfEffect/ComposeNormal");
	//m_ComposedNormalMaterialPtr = m_ComposedNormalMaterialPtr->clone("ComposeWaterBuild" + Ogre::StringConverter::toString(countbuild++));
	//m_ComposedNormalMaterialPtr->load();

	m_BuildSectionMats.clear();
	m_BuildSectionMats.push_back(m_HeatOrganFaceMaterial);
	//m_BuildSectionMats.push_back(m_ComposedEffectMaterialPtr);
	m_BuildSectionMats.push_back(m_DynBloodBuildMatPtr);
	m_BuildSectionMats.push_back(m_BleedBuildMatPtr);
	m_BuildSectionMats.push_back(m_VeinBloodBuildMatPtr);
	//m_BuildSectionMats.push_back(m_DynamicWaterMaterialPtr);
	//m_BuildSectionMats.push_back(m_ComposedNormalMaterialPtr);
	m_BuildSectionMats.push_back(m_WaterWashBloodMatPtr);
	m_BuildSectionMats.push_back(m_DynamicWaterTexMaterialPtr);
	m_BuildSectionMats.push_back(m_BleedingPointMaterial);


	m_BuildQuadObj = m_pQuantityTexSceneMgr->createManualObject();
	m_BuildQuadObj->setDynamic(true);

	for(size_t c = 0 ; c < m_BuildSectionMats.size() ; c++)
	{
		m_BuildQuadObj->begin(m_BuildSectionMats[c]->getName() , Ogre::RenderOperation::OT_TRIANGLE_LIST);

		m_BuildQuadObj->position(Ogre::Vector3::ZERO);
		m_BuildQuadObj->textureCoord(0, 0);
		m_BuildQuadObj->colour(Ogre::ColourValue::Black);

		m_BuildQuadObj->position(Ogre::Vector3::ZERO);
		m_BuildQuadObj->textureCoord(0, 0);
		m_BuildQuadObj->colour(Ogre::ColourValue::Black);
		
		m_BuildQuadObj->position(Ogre::Vector3::ZERO);
		m_BuildQuadObj->textureCoord(0, 0);
		m_BuildQuadObj->colour(Ogre::ColourValue::Black);

		//using indices
		m_BuildQuadObj->index(0);
		m_BuildQuadObj->index(2);
		m_BuildQuadObj->index(1);

		m_BuildQuadObj->end();
	}
	
	m_MarkQuadObj = m_pQuantityTexSceneMgr->createManualObject();
	m_MarkQuadObj->setDynamic(true);
	m_pQuantityTexSceneMgr->getRootSceneNode()->attachObject(m_MarkQuadObj);
}
//================================================================================================================================
void MisMedicEffectRender::SetDynBldPar_Tex(const Ogre::String & dynBldParTex)
{
	Ogre::TexturePtr DynBldTex = Ogre::TextureManager::getSingleton().load(dynBldParTex , Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	ApplyTextureToMaterial(m_DynBloodBuildMatPtr , DynBldTex , "BloodMap");

}
//================================================================================================================================
void MisMedicEffectRender::InitRenderTextureWithTexture(Ogre::TexturePtr texture , Ogre::String imagename)
{
	Ogre::MaterialPtr material   = Ogre::MaterialManager::getSingleton().getByName("OrganSurfEffect/ClearWithImage");

	Ogre::TexturePtr  Srctextureptr = Ogre::TextureManager::getSingleton().load(imagename , Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	
	ApplyTextureToMaterial(material , Srctextureptr , "SrcImgMap");
	//ApplyTextureToMaterial(material , imageTexture , "SrcImgMap");

	Ogre::RenderTarget * rttTex = texture->getBuffer()->getRenderTarget();

	if(rttTex)
	{
		rttTex->setAutoUpdated(false);//disable auto update for manually update

		for(size_t v = 0 ; v < rttTex->getNumViewports() ; v++)
		{
			Ogre::Viewport * Dstvp = rttTex->getViewport(v);

			Ogre::Camera * DstCam = Dstvp->getCamera();

			Ogre::SceneManager * DstScenemgr = DstCam->getSceneManager();

			DstCam->setProjectionType(Ogre::PT_ORTHOGRAPHIC);

			DstCam->setOrthoWindow(1 , 1);

			DstCam->setPosition(Ogre::Vector3::ZERO);

			DstCam->setNearClipDistance(0.001f);

			Dstvp->setOverlaysEnabled(false);

			Dstvp->setClearEveryFrame(true);

		
			Ogre::ManualObject * QuadObj = DstScenemgr->createManualObject("QuadObj");

			DstScenemgr->getRootSceneNode()->attachObject(QuadObj);

			QuadObj->begin(material->getName(), Ogre::RenderOperation::OT_TRIANGLE_LIST);

			QuadObj->position(Ogre::Vector3(0.5f, -0.5f, -1.0));
			QuadObj->textureCoord(1, 1);

			QuadObj->position(Ogre::Vector3(-0.5f, 0.5f, -1.0));
			QuadObj->textureCoord(0, 0);

			QuadObj->position(Ogre::Vector3(-0.5f, -0.5f, -1.0));
			QuadObj->textureCoord(0, 1);

			QuadObj->position(Ogre::Vector3(0.5f, 0.5f, -1.0));
			QuadObj->textureCoord(1, 0);

			//using indices
			QuadObj->index(0);
			QuadObj->index(1);
			QuadObj->index(2);
			QuadObj->index(0);
			QuadObj->index(3);
			QuadObj->index(1);
			QuadObj->end();

			rttTex->update();

			DstScenemgr->getRootSceneNode()->detachObject(QuadObj);
			DstScenemgr->destroyMovableObject(QuadObj);

			Dstvp->setClearEveryFrame(false);

			//rttTex->writeContentsToFile("c:\\rrttex.bmp");
		}
	}
}

void MisMedicEffectRender::CleanRenderTexture(Ogre::TexturePtr texture , Ogre::ColourValue color)
{
	Ogre::RenderTarget * rttTex = texture->getBuffer()->getRenderTarget();

	if(rttTex)
	{
		rttTex->setAutoUpdated(false);//disable auto update for manually update
		for(size_t v = 0 ; v < rttTex->getNumViewports() ; v++)
		{
			Ogre::Viewport* vp = rttTex->getViewport(v);
			vp->clear(Ogre::FBT_COLOUR, color);
			rttTex->update();
		}
// 		rttTex->setAutoUpdated(true);//disable auto update for manually update

	}
}

void MisMedicEffectRender::InitRenderTexture(Ogre::TexturePtr texture , Ogre::ColourValue color)
{
	Ogre::RenderTarget * rttTex = texture->getBuffer()->getRenderTarget();

	if(rttTex)
	{
		rttTex->setAutoUpdated(false);//disable auto update for manually update

		for(size_t v = 0 ; v < rttTex->getNumViewports() ; v++)
		{
			Ogre::Viewport * Dstvp = rttTex->getViewport(v);
			
			Ogre::Camera * DstCam = Dstvp->getCamera();

			Ogre::SceneManager * DstScenemgr = DstCam->getSceneManager();

			DstCam->setProjectionType(Ogre::PT_ORTHOGRAPHIC);

			DstCam->setOrthoWindow(1 , 1);

			DstCam->setPosition(Ogre::Vector3::ZERO);

			DstCam->setNearClipDistance(0.001f);

			Dstvp->setOverlaysEnabled(false);

			Dstvp->setClearEveryFrame(true);

			Dstvp->setBackgroundColour(color);

			Ogre::Matrix4 mat = DstCam->getProjectionMatrixRS();

			Ogre::ManualObject * QuadObj = DstScenemgr->createManualObject("QuadObj");

			DstScenemgr->getRootSceneNode()->attachObject(QuadObj);

			QuadObj->begin("OrganSurfEffect/Initialize", Ogre::RenderOperation::OT_TRIANGLE_LIST);

			QuadObj->position(Ogre::Vector3(-0.5f, 0.5f, -1.0));
			QuadObj->colour(color);

			QuadObj->position(Ogre::Vector3(0.5f, 0.5f, -1.0));
			QuadObj->colour(color);

			QuadObj->position(Ogre::Vector3(-0.5f, -0.5f, -1.0));
			QuadObj->colour(color);

			QuadObj->position(Ogre::Vector3(0.5f, -0.5f, -1.0));
			QuadObj->colour(color);

			//using indices
			QuadObj->index(0);
			QuadObj->index(2);
			QuadObj->index(1);
			QuadObj->index(1);
			QuadObj->index(2);
			QuadObj->index(3);
			QuadObj->end();

			rttTex->update();

			DstScenemgr->getRootSceneNode()->detachObject(QuadObj);
			DstScenemgr->destroyMovableObject(QuadObj);

			Dstvp->setClearEveryFrame(false);
		}
	}
}


//Ogre::TexturePtr MisMedicEffectRender::GetComposedEffectTexture()
//{
	//return m_ComposedEffectTexturePtr;
//}

void MisMedicEffectRender::clearMarkQuadObject()
{
	m_MarkQuadObj->clear();
}

void MisMedicEffectRender::MarkTexBackgClearEveryFrame(bool autoclear)
{
	if (m_pMarkViewPort)
	{
		m_pMarkViewPort->setClearEveryFrame(autoclear);
	}
}

void MisMedicEffectRender::MarkTextureFlush()
{
	Ogre::RenderTarget * rttTex = m_MarkTexturePtr->getBuffer()->getRenderTarget();
	if (rttTex)
	{
		rttTex->update();

		ApplyTextureToMaterial(m_OrganMaterialName, m_MarkTexturePtr, "MixTextureMap");
	}
}

void MisMedicEffectRender::MarkEffectTexture(Ogre::Vector2 screencoord, float radius, std::string matename)
{
	float umin = screencoord.x - radius - 0.01f;
	float umax = screencoord.x + radius + 0.01f;
	float vmin = screencoord.y - radius - 0.01f;
	float vmax = screencoord.y + radius + 0.01f;

	Ogre::RenderTarget * rttTex = m_MarkTexturePtr->getBuffer()->getRenderTarget();
	if (rttTex)
	{
		m_MarkQuadObj->begin(matename, Ogre::RenderOperation::OT_TRIANGLE_LIST);
		
		m_MarkQuadObj->position(Ogre::Vector3(umin - 0.5f, 0.5f - vmin, -1.0));
		m_MarkQuadObj->textureCoord(0, 0);//umin, vmin

		m_MarkQuadObj->position(Ogre::Vector3(umax - 0.5f, 0.5f - vmin, -1.0));
		m_MarkQuadObj->textureCoord(1, 0);//umax, vmin

		m_MarkQuadObj->position(Ogre::Vector3(umin - 0.5f, 0.5f - vmax, -1.0));
		m_MarkQuadObj->textureCoord(0, 1);//umin, vmax

		m_MarkQuadObj->position(Ogre::Vector3(umax - 0.5f, 0.5f - vmax, -1.0));
		m_MarkQuadObj->textureCoord(1, 1);//umax, vmax

		//using indices
		m_MarkQuadObj->index(0);
		m_MarkQuadObj->index(2);
		m_MarkQuadObj->index(1);
		m_MarkQuadObj->index(1);
		m_MarkQuadObj->index(2);
		m_MarkQuadObj->index(3);

		m_MarkQuadObj->end();

        //rttTex->writeContentsToFile("c://Marktest.bmp");
	}
}

void MisMedicEffectRender::ApplySoakEffect(MisMedicOrganRender* pRenderObj,std::string matename)
{
    Ogre::RenderTarget * rttTex = m_QuantityTexturePtr->getBuffer()->getRenderTarget();

	if (rttTex)
	{
		Ogre::SceneNode *oldnode = (Ogre::SceneNode *)pRenderObj->getParentNode();
		oldnode->detachObject(pRenderObj);

		int numlayer = pRenderObj->GetNumLayers();

		std::vector<std::string> originMatNames;
		std::vector<std::string> cutMatNames;
		originMatNames.reserve(numlayer);
		cutMatNames.reserve(numlayer);

		for (int c = 0; c < numlayer; c++)
		{
			Ogre::String oldMatName0 = pRenderObj->GetMaterialName(c, 0);
			Ogre::String oldMatName1 = pRenderObj->GetMaterialName(c, 1);
			originMatNames.push_back(oldMatName0);
			cutMatNames.push_back(oldMatName1);

			pRenderObj->setMaterialName(c , 0 , matename);
			pRenderObj->setMaterialName(c , 1 , matename);
		}
		pRenderObj->SetCutPartVisibility(true);

		m_pQuantityTexSceneMgr->getRootSceneNode()->attachObject(pRenderObj);


		rttTex->update();

		m_pQuantityTexSceneMgr->getRootSceneNode()->detachObject(pRenderObj);

		for (int c = 0; c < numlayer; c++)
		{
			pRenderObj->setMaterialName(c, 0, originMatNames[c]);
			pRenderObj->setMaterialName(c, 1, cutMatNames[c]);
		}
		pRenderObj->SetCutPartVisibility(true);

		oldnode->attachObject(pRenderObj);
	}
}

#define RANDCOUNT 1

void MisMedicEffectRender::ApplyHeat(Ogre::Vector2 texturecoord , float radius , float heatvalue ,  Ogre::TexturePtr cogBrandTex)
{
	if(m_QuantityTexturePtr.isNull())
	   return;

	//first update heat channel texture
//#if 0
 	Ogre::RenderTarget * rttTex = m_QuantityTexturePtr->getBuffer()->getRenderTarget();
//#endif
	//Ogre::RenderTarget * rttTex = m_ComposedEffectTexturePtr->getBuffer()->getRenderTarget();

	//int DstTexSize = m_QuantityTexturePtr->getWidth();
	
	if (cogBrandTex.isNull() == false)
	{
		if (m_bHeatBrandName != cogBrandTex->getName())
		{
			m_bHeatBrandName = cogBrandTex->getName();
			ApplyTextureToMaterial(m_HeatOrganFaceMaterial , cogBrandTex , "HeatBrand");
		}
	}

	if(rttTex)
	{
		int VFSAA = m_QuantityTexturePtr->getFSAA();
		m_QuantityTexturePtr->setFSAA(0,"");

		Ogre::Vector3 QuadCenterPos(texturecoord.x-0.5f , 0.5f-texturecoord.y , 0);

		Ogre::ManualObject * QuadObj = m_pQuantityTexSceneMgr->createManualObject("QuadObj");

		m_pQuantityTexSceneMgr->getRootSceneNode()->attachObject(QuadObj);

		const Ogre::String& sMaterialName = m_HeatOrganFaceMaterial->getName();
		QuadObj->begin(sMaterialName, Ogre::RenderOperation::OT_TRIANGLE_LIST);

		int startIndex = 0;

		float du = radius*0.5f*m_deviateScale*(float)(rand() % 5-2.5f) / 5.0f;

		float dv = radius*0.5f*m_deviateScale*(float)(rand() % 5-2.5f) / 5.0f;
	
		float cx = texturecoord.x;
		float cy = texturecoord.y;
		

		float minx = cx-radius-0.5f;
		float maxx = cx+radius-0.5f;

		float miny = 0.5f-(cy+radius);
		float maxy = 0.5f-(cy-radius);
		
		
		QuadObj->position(Ogre::Vector3(minx, maxy, -1.0));
		QuadObj->textureCoord(0, 0);
		QuadObj->colour(1 , 1 , 1 , heatvalue);

		QuadObj->position(Ogre::Vector3(maxx, maxy, -1.0));
		QuadObj->textureCoord(1, 0);
		QuadObj->colour(1 , 1 , 1 , heatvalue);

		QuadObj->position(Ogre::Vector3(minx, miny, -1.0));
		QuadObj->textureCoord(0, 1);
		QuadObj->colour(1 , 1 , 1 , heatvalue);

		QuadObj->position(Ogre::Vector3(maxx , miny, -1.0));
		QuadObj->textureCoord(1, 1);
		QuadObj->colour(1 , 1 , 1 , heatvalue);

		//using indices
		QuadObj->index(0+startIndex);
		QuadObj->index(2+startIndex);
		QuadObj->index(1+startIndex);
		QuadObj->index(1+startIndex);
		QuadObj->index(2+startIndex);
		QuadObj->index(3+startIndex);

		startIndex += 4;
		
		QuadObj->end();

		rttTex->update();

		m_pQuantityTexSceneMgr->getRootSceneNode()->detachObject(QuadObj);
		m_pQuantityTexSceneMgr->destroyMovableObject(QuadObj);

		m_QuantityTexturePtr->setFSAA(VFSAA,"");
	}

 	//ComposeEffectTexture(Ogre::ColourValue(1.0f , 1.0f ,1.0f ,1.0f) , 
		              //   texturecoord.x-radius-0.01f,
						// texturecoord.x+radius+0.01f,
						// texturecoord.y-radius-0.01f,
						// texturecoord.y+radius+0.01f);
}

void MisMedicEffectRender::BloodBurnSpread(MisMedicOrganRender* pRenderObj, float crossDir) {
	
	if (pRenderObj == 0)
		return;

	int renderOrgan = 1;
	Ogre::ManualObject * QuadObj;
	Ogre::SceneNode * oldnode;
	Ogre::String oldMatName0, oldMatName1;
	if (m_QuantityTexturePtr.isNull()) return;

	Ogre::TexturePtr QuantityTexturePtrThis, QuantityTexturePtrThat;

	if (m_QuantityTexturePtr.getPointer() == m_QuantityTexturePtr1.getPointer())
	{
		QuantityTexturePtrThis = m_QuantityTexturePtr1;
		QuantityTexturePtrThat = m_QuantityTexturePtr2;
	}
	else
	{
		QuantityTexturePtrThis = m_QuantityTexturePtr2;
		QuantityTexturePtrThat = m_QuantityTexturePtr1;
	}

   /*if (m_QuantityTextureNum==1) {
		m_QuantityTextureNum = 2; 
		QuantityTexturePtrThis = m_QuantityTexturePtr1;
		QuantityTexturePtrThat = m_QuantityTexturePtr2;
	}
	else {
		m_QuantityTextureNum = 1; 
		QuantityTexturePtrThis = m_QuantityTexturePtr2;
		QuantityTexturePtrThat = m_QuantityTexturePtr1;
	}*/

	// gravity
	Ogre::RenderTarget * rttTex = QuantityTexturePtrThat->getBuffer()->getRenderTarget();
	ApplyTextureToMaterial(m_GravityMaterial, QuantityTexturePtrThis, "LatestTexture");

	Ogre::GpuProgramParametersSharedPtr params = GetShaderParamterPtr(m_GravityMaterial->getName(), VERTEX_PROGRAME, 0, 0);
	if (params.isNull() == false)
		params->setNamedConstant("crossDir", crossDir);


	int VFSAA = QuantityTexturePtrThat->getFSAA();
	QuantityTexturePtrThat->setFSAA(0, "");
	
	int numlayer = pRenderObj->GetNumLayers();
	
	std::vector<std::string> originMatNames;
	
	std::vector<std::string> cutMatNames;
	
	originMatNames.reserve(numlayer);
	
	cutMatNames.reserve(numlayer);

	if (renderOrgan) {
		
		oldnode = (Ogre::SceneNode *)pRenderObj->getParentNode();
		oldnode->detachObject(pRenderObj);

		for (int c = 0; c < numlayer; c++)
		{
			oldMatName0 = pRenderObj->GetMaterialName(c , 0);
			oldMatName1 = pRenderObj->GetMaterialName(c , 1);
			
			originMatNames.push_back(oldMatName0);
			cutMatNames.push_back(oldMatName1);

			pRenderObj->setMaterialName(c , 0, m_GravityMaterial->getName());
			pRenderObj->setMaterialName(c , 1, m_GravityMaterial->getName());
		}
		pRenderObj->SetCutPartVisibility(false);
		m_pQuantityTexSceneMgr->getRootSceneNode()->attachObject(pRenderObj);
		//printf("%d\n", pManualObj->getSection(0)->;
	}
	else {
		//InitRenderTexture(m_QuantityTexturePtr2, Ogre::ColourValue(1.0f, 0.0f, 0.0f, 1.0f));
		/*
		QuadObj = m_pQuantityTexSceneMgr->createManualObject("QuadObj");

		m_pQuantityTexSceneMgr->getRootSceneNode()->attachObject(QuadObj);

		const Ogre::String& sMaterialName = m_GravityMaterial->getName();
		QuadObj->begin(sMaterialName, Ogre::RenderOperation::OT_TRIANGLE_LIST);


		QuadObj->position(Ogre::Vector3(0.5f, -0.5f, -1.0));
		QuadObj->textureCoord(1.0f, 0.0f);
		QuadObj->colour(1, 1, 1, 1.0);

		QuadObj->position(Ogre::Vector3(-0.5f, 0.5f, -1.0));
		QuadObj->textureCoord(0.0f, 1.0f);
		QuadObj->colour(1, 1, 1, 1.0);

		QuadObj->position(Ogre::Vector3(-0.5f, -0.5f, -1.0));
		QuadObj->textureCoord(0.0f, 0.0f);
		QuadObj->colour(1, 1, 1, 1.0);

		QuadObj->position(Ogre::Vector3(0.5f, 0.5f, -1.0));
		QuadObj->textureCoord(1.0f, 1.0f);
		QuadObj->colour(1, 1, 1, 1.0);

		//using indices
		QuadObj->index(0);
		QuadObj->index(2);
		QuadObj->index(1);
		QuadObj->index(0);
		QuadObj->index(1);
		QuadObj->index(3);

		QuadObj->end();
		*/
	}
	
	rttTex->update();

	if (renderOrgan) {
		m_pQuantityTexSceneMgr->getRootSceneNode()->detachObject(pRenderObj);

		for (int c = 0; c < numlayer; c++)
		{
			pRenderObj->setMaterialName(c, 0, originMatNames[c]);
			pRenderObj->setMaterialName(c, 1, cutMatNames[c]);
		}

		pRenderObj->SetCutPartVisibility(true);

		oldnode->attachObject(pRenderObj);
	}
	else {
		/*
		m_pQuantityTexSceneMgr->getRootSceneNode()->detachObject(QuadObj);
		m_pQuantityTexSceneMgr->destroyMovableObject(QuadObj);
		*/
	}
	
	QuantityTexturePtrThat->setFSAA(VFSAA, "");
	
	m_QuantityTexturePtr = QuantityTexturePtrThat;
}

/*
void MisMedicEffectRender::BurnMarkSpread(MisMedicOrganRender* pRenderObj)
{
	if (pRenderObj == 0)
		return;

	int renderOrgan = 1;
	Ogre::ManualObject * QuadObj;
	Ogre::SceneNode * oldnode;
	Ogre::String oldMatName0, oldMatName1;
	
	if (m_QuantityTexturePtr.isNull()) 
		return;

	Ogre::TexturePtr QuantityTexturePtrThis, QuantityTexturePtrThat;

	if (m_QuantityTexturePtr.getPointer() == m_QuantityTexturePtr1.getPointer())
	{
		QuantityTexturePtrThis = m_QuantityTexturePtr1;
		QuantityTexturePtrThat = m_QuantityTexturePtr2;
	}
	else 
	{
		QuantityTexturePtrThis = m_QuantityTexturePtr2;
		QuantityTexturePtrThat = m_QuantityTexturePtr1;
	}

	// gravity
	Ogre::RenderTarget * rttTex = QuantityTexturePtrThat->getBuffer()->getRenderTarget();
	ApplyTextureToMaterial(m_BurnSpreadMaterial, QuantityTexturePtrThis, "LatestTexture");

	int VFSAA = QuantityTexturePtrThat->getFSAA();
	QuantityTexturePtrThat->setFSAA(0, "");

	if (renderOrgan) {

		oldnode = (Ogre::SceneNode *)pRenderObj->getParentNode();
		oldnode->detachObject(pRenderObj);
		oldMatName0 = pRenderObj->GetMaterialName(0);
		oldMatName1 = pRenderObj->GetMaterialName(1);
		
		pRenderObj->setMaterialName(0, m_BurnSpreadMaterial->getName());
		pRenderObj->setMaterialName(1, m_BurnSpreadMaterial->getName());
		pRenderObj->SetCutPartVisibility(false);
		
		m_pQuantityTexSceneMgr->getRootSceneNode()->attachObject(pRenderObj);
	}

	rttTex->update();

	if (renderOrgan) {
		m_pQuantityTexSceneMgr->getRootSceneNode()->detachObject(pRenderObj);
		pRenderObj->setMaterialName(0, oldMatName0);
		pRenderObj->setMaterialName(1, oldMatName1);
		pRenderObj->SetCutPartVisibility(true);
		oldnode->attachObject(pRenderObj);
	}

	QuantityTexturePtrThat->setFSAA(VFSAA, "");

	m_QuantityTexturePtr = QuantityTexturePtrThat;
}
*/

void MisMedicEffectRender::ApplyBleedings(std::vector<Ogre::Vector2> texturecoords,
	                                      float radius,
										  float bloodvalue,
	                                      Ogre::TexturePtr bleedingPointTex)
{
	if (m_QuantityTexturePtr.isNull())
		return;

	Ogre::RenderTarget * rttTex = m_QuantityTexturePtr->getBuffer()->getRenderTarget();

	if (bleedingPointTex.isNull() == false)
	{
		ApplyTextureToMaterial(m_BleedingPointMaterial, bleedingPointTex, "BleedingAlpha");
	}

	if (rttTex)
	{
		int VFSAA = m_QuantityTexturePtr->getFSAA();
		m_QuantityTexturePtr->setFSAA(0, "");

		Ogre::ManualObject * QuadObj = m_pQuantityTexSceneMgr->createManualObject("QuadObj");

		m_pQuantityTexSceneMgr->getRootSceneNode()->attachObject(QuadObj);

		const Ogre::String& sMaterialName = m_BleedingPointMaterial->getName();
		QuadObj->begin(sMaterialName, Ogre::RenderOperation::OT_TRIANGLE_LIST);

		int startIndex = 0;

		for (int c = 0; c < texturecoords.size(); c++)
		{
			float cx = texturecoords[c].x;
			float cy = texturecoords[c].y;


			float minx = cx - radius - 0.5f;
			float maxx = cx + radius - 0.5f;

			float miny = 0.5f - (cy + radius);
			float maxy = 0.5f - (cy - radius);


			QuadObj->position(Ogre::Vector3(minx, maxy, -1.0));
			QuadObj->textureCoord(0, 0);
			QuadObj->colour(1, 1, 1, bloodvalue);

			QuadObj->position(Ogre::Vector3(maxx, maxy, -1.0));
			QuadObj->textureCoord(1, 0);
			QuadObj->colour(1, 1, 1, bloodvalue);

			QuadObj->position(Ogre::Vector3(minx, miny, -1.0));
			QuadObj->textureCoord(0, 1);
			QuadObj->colour(1, 1, 1, bloodvalue);

			QuadObj->position(Ogre::Vector3(maxx, miny, -1.0));
			QuadObj->textureCoord(1, 1);
			QuadObj->colour(1, 1, 1, bloodvalue);

			//using indices
			QuadObj->index(0 + startIndex);
			QuadObj->index(2 + startIndex);
			QuadObj->index(1 + startIndex);
			QuadObj->index(1 + startIndex);
			QuadObj->index(2 + startIndex);
			QuadObj->index(3 + startIndex);

			startIndex += 4;
		}
		QuadObj->end();

		rttTex->update();

		m_pQuantityTexSceneMgr->getRootSceneNode()->detachObject(QuadObj);
		m_pQuantityTexSceneMgr->destroyMovableObject(QuadObj);

		m_QuantityTexturePtr->setFSAA(VFSAA, "");
	}
}
void MisMedicEffectRender::ApplyBleeding(Ogre::Vector2 texturecoord, float radius, float heatvalue, Ogre::TexturePtr bleedingPointTex)
{
	if (m_QuantityTexturePtr.isNull())
		return;

	Ogre::RenderTarget * rttTex = m_QuantityTexturePtr->getBuffer()->getRenderTarget();
	
	if (bleedingPointTex.isNull() == false)
	{
		ApplyTextureToMaterial(m_BleedingPointMaterial, bleedingPointTex, "BleedingAlpha");
	
	}

	Ogre::GpuProgramParametersSharedPtr params = GetShaderParamterPtr(m_BleedingPointMaterial->getName(), FRAGMENT_PROGRAME, 0, 0);
	if (params.isNull() == false)
		params->setNamedConstant("opacity", heatvalue);

	if (rttTex)
	{
		int VFSAA = m_QuantityTexturePtr->getFSAA();
		m_QuantityTexturePtr->setFSAA(0, "");

		Ogre::Vector3 QuadCenterPos(texturecoord.x - 0.5f, 0.5f - texturecoord.y, 0);

		Ogre::ManualObject * QuadObj = m_pQuantityTexSceneMgr->createManualObject("QuadObj");

		m_pQuantityTexSceneMgr->getRootSceneNode()->attachObject(QuadObj);

		const Ogre::String& sMaterialName = m_BleedingPointMaterial->getName();
		QuadObj->begin(sMaterialName, Ogre::RenderOperation::OT_TRIANGLE_LIST);

		int startIndex = 0;

		float du = radius*0.5f*m_deviateScale*(float)(rand() % 5 - 2.5f) / 5.0f;

		float dv = radius*0.5f*m_deviateScale*(float)(rand() % 5 - 2.5f) / 5.0f;

		float cx = texturecoord.x;
		float cy = texturecoord.y;


		float minx = cx - radius - 0.5f;
		float maxx = cx + radius - 0.5f;

		float miny = 0.5f - (cy + radius);
		float maxy = 0.5f - (cy - radius);


		QuadObj->position(Ogre::Vector3(minx, maxy, -1.0));
		QuadObj->textureCoord(0, 0);
		QuadObj->colour(1, 1, 1, heatvalue);

		QuadObj->position(Ogre::Vector3(maxx, maxy, -1.0));
		QuadObj->textureCoord(1, 0);
		QuadObj->colour(1, 1, 1, heatvalue);

		QuadObj->position(Ogre::Vector3(minx, miny, -1.0));
		QuadObj->textureCoord(0, 1);
		QuadObj->colour(1, 1, 1, heatvalue);

		QuadObj->position(Ogre::Vector3(maxx, miny, -1.0));
		QuadObj->textureCoord(1, 1);
		QuadObj->colour(1, 1, 1, heatvalue);

		//using indices
		QuadObj->index(0 + startIndex);
		QuadObj->index(2 + startIndex);
		QuadObj->index(1 + startIndex);
		QuadObj->index(1 + startIndex);
		QuadObj->index(2 + startIndex);
		QuadObj->index(3 + startIndex);

		startIndex += 4;

		QuadObj->end();

		rttTex->update();

		m_pQuantityTexSceneMgr->getRootSceneNode()->detachObject(QuadObj);
		m_pQuantityTexSceneMgr->destroyMovableObject(QuadObj);

		m_QuantityTexturePtr->setFSAA(VFSAA, "");
	}
}

//====================================================================================================================================
void MisMedicEffectRender::ApplyCongulate_type_2(const std::vector<Ogre::Vector2> & texturecoords ,const std::vector<Ogre::Vector2> & TFUV, ITool * tool)
{
	if(m_QuantityTexturePtr.isNull())
		return;

	Ogre::RenderTarget * rttTex = m_QuantityTexturePtr->getBuffer()->getRenderTarget();
	
	int DstTexSize = m_QuantityTexturePtr->getWidth();

	if(rttTex)
	{
		int VFSAA = m_QuantityTexturePtr->getFSAA();
		m_QuantityTexturePtr->setFSAA(0,"");

		Ogre::ManualObject * QuadObj = m_pQuantityTexSceneMgr->createManualObject("QuadObj");

		m_pQuantityTexSceneMgr->getRootSceneNode()->attachObject(QuadObj);

		Ogre::TexturePtr tex = tool->GetToolBrandTexture();

		if (tex.isNull())
		{
			tex = m_DefaultHeatBrandTexPtr;
		}
	
		if (m_bHeatBrandName != tex->getName())
		{
			m_bHeatBrandName = tex->getName();
			ApplyTextureToMaterial(m_HeatOrganFaceMaterial , tex , "HeatBrand");
		}
		
		QuadObj->begin(m_HeatOrganFaceMaterial->getName(), Ogre::RenderOperation::OT_TRIANGLE_LIST);

		float umin = FLT_MAX;
		
		float umax = -FLT_MAX;
		
		float vmin = FLT_MAX;
		
		float vmax = -FLT_MAX;
		
		for(size_t quad = 0; quad < texturecoords.size() ; quad++)
		{
			Ogre::Vector2 texturecoord = texturecoords[quad];
			
			if(texturecoord.x < umin)
			   umin = texturecoord.x;

			if(texturecoord.x > umax)
			   umax = texturecoord.x;

			if(texturecoord.y < vmin)
			   vmin = texturecoord.y;

			if(texturecoord.y > vmax)
			   vmax = texturecoord.y;

			float cx = texturecoord.x - 0.5f;
			
			float cy = 0.5f - texturecoord.y;
			
			Ogre::Vector2 UV = TFUV[quad];

			QuadObj->position(Ogre::Vector3(cx, cy, -1.0));
			QuadObj->textureCoord(UV.x, UV.y);//(UV.x, UV.y + 0.15f);
			QuadObj->colour(1 , 1 , 1 , 0.025f);
			//using indices
		}
		for(size_t tr = 0; tr < texturecoords.size() ; tr+=3)
		{
			QuadObj->index(tr + 0);
			QuadObj->index(tr + 1);
			QuadObj->index(tr + 2);
			QuadObj->index(tr + 0);
			QuadObj->index(tr + 2);
			QuadObj->index(tr + 1);
		}

		QuadObj->end();

		rttTex->update();

		m_pQuantityTexSceneMgr->getRootSceneNode()->detachObject(QuadObj);
		m_pQuantityTexSceneMgr->destroyMovableObject(QuadObj);

		m_QuantityTexturePtr->setFSAA(VFSAA,"");
		//rttTex->writeContentsToFile("c:\\rrttex_type2.bmp");

		//ComposeEffectTexture(Ogre::ColourValue(1.0f , 1.0f ,1.0f ,1.0f),
			              //   umin - 0.01f,
			               //  umax + 0.01f,
			               //  vmin - 0.01f,
			               //  vmax + 0.01f);
	}
	
}
//=====================================================================================================================
void MisMedicEffectRender::RendBleedPoints(std::vector<Ogre::Vector2> & BleedPoints ,
										   std::vector<float> & quantity ,
										   float radius)
{
	if(BleedPoints.size() == 0)
		return;

	Ogre::RenderTarget * rttTex = m_QuantityTexturePtr->getBuffer()->getRenderTarget();
	
	if(rttTex)
	{
		Ogre::ManualObject * QuadObj = m_pQuantityTexSceneMgr->createManualObject("QuadObj");

		m_pQuantityTexSceneMgr->getRootSceneNode()->attachObject(QuadObj);

		QuadObj->clear();

		QuadObj->begin("OrganSurfEffect/Bleed", Ogre::RenderOperation::OT_TRIANGLE_LIST);

		int startIndex = 0;
		
		Ogre::ColourValue bleedcolor(1.0f , 1.0f , 1.0f , 0);//0.01f);

		float umin , umax , vmin , vmax;
		umin = vmin = FLT_MAX;
		umax = vmax = -FLT_MAX;
		for(size_t s = 0 ; s < BleedPoints.size() ; s++)
		{
			float bloodradius = radius;
			
			Ogre::Vector2 texcoord = BleedPoints[s];

			if(texcoord.x < umin)
			   umin = texcoord.x;

			if(texcoord.x > umax)
			   umax = texcoord.x;

			if(texcoord.y < vmin)
			   vmin = texcoord.y;

			if(texcoord.y > vmax)
			   vmax = texcoord.y;

			float bleedrotate = 0;

			float tu = texcoord.x;

			float tv = texcoord.y;

			tu = tu-0.5f;

			tv = 0.5-tv;

			Ogre::Quaternion rotation( Ogre::Radian(bleedrotate), Ogre::Vector3::UNIT_Z);

			Ogre::Vector3 quadCenter(tu,tv,-1.0);

			float realradius = bloodradius;

			bleedcolor.a = quantity[s];

			QuadObj->position(quadCenter+rotation*Ogre::Vector3(realradius, -realradius, 0));
			QuadObj->textureCoord(1, 1);
			QuadObj->colour(bleedcolor);

			QuadObj->position(quadCenter+rotation*Ogre::Vector3(-realradius, realradius, 0));
			QuadObj->textureCoord(0, 0);
			QuadObj->colour(bleedcolor);

			QuadObj->position(quadCenter+rotation*Ogre::Vector3(-realradius, -realradius, 0));
			QuadObj->textureCoord(0, 1);
			QuadObj->colour(bleedcolor);

			QuadObj->position(quadCenter+rotation*Ogre::Vector3(realradius, realradius, 0));
			QuadObj->textureCoord(1, 0);
			QuadObj->colour(bleedcolor);

			//using indices
			QuadObj->index(0+startIndex);
			QuadObj->index(1+startIndex);
			QuadObj->index(2+startIndex);

			QuadObj->index(0+startIndex);
			QuadObj->index(3+startIndex);
			QuadObj->index(1+startIndex);

			startIndex += 4;
		}
		QuadObj->end();

		rttTex->update();

		m_pQuantityTexSceneMgr->getRootSceneNode()->detachObject(QuadObj);
		m_pQuantityTexSceneMgr->destroyMovableObject(QuadObj);

		//ComposeEffectTexture(Ogre::ColourValue(1.0f , 1.0f ,1.0f , 1.0f),
			                 //umin - 0.01f,
			                 //umax + 0.01f,
			                // vmin - 0.01f,
			                // vmax + 0.01f);
	}

	
}
//==========================================================================================
void MisMedicEffectRender::RendVeinConnectBlood(TextureBloodTrackEffect & bloodsys , float dt)
{
	float UpdateInterval = 1.0f / 30.0f;

	bool  NeedReDraw = false;
	m_TimeSinceLastVeinBloodUpdate += dt;

	if(m_TimeSinceLastVeinBloodUpdate > UpdateInterval)
	{
		NeedReDraw = true;
		m_TimeSinceLastVeinBloodUpdate -= UpdateInterval * (int)(m_TimeSinceLastVeinBloodUpdate / UpdateInterval);
	}

	//second rend bleed effect
	bool hasbleedUpdated = false;

	if(m_QuantityTexturePtr.isNull())
	   return;

	Ogre::RenderTarget * rttTex = m_QuantityTexturePtr->getBuffer()->getRenderTarget();

	if(rttTex)
	{
	   m_pQuantityTexSceneMgr->getRootSceneNode()->attachObject(m_BuildQuadObj);
	   //
	   for(size_t c = 0 ; c < m_BuildSectionMats.size() ; c++)
	   {
		   m_BuildQuadObj->beginUpdate(c);
		   if(m_BuildSectionMats[c] == m_VeinBloodBuildMatPtr)
		   {
			  hasbleedUpdated = bloodsys.BuildVeinBlood(m_BuildQuadObj);
		   }
		   m_BuildQuadObj->end();
	   }

	   if(hasbleedUpdated && NeedReDraw)
		  rttTex->update();
	   
	   m_pQuantityTexSceneMgr->getRootSceneNode()->detachObject(m_BuildQuadObj);
	}
}
//====================================================================================================================================
bool MisMedicEffectRender::RendLiquidParticles(TextureBloodTrackEffect & bloodsys , TextureWaterTrackEffect & waterSys ,float dt)
{
	float UpdateInterval = 1.0f / 100.0f;

	bool  NeedReDraw = false;
	
	/*m_TimeSinceLastBloodParUpdate += dt;
	
	if(m_TimeSinceLastBloodParUpdate > UpdateInterval)
	{
	   NeedReDraw = true;
	   m_TimeSinceLastBloodParUpdate = 0;//UpdateInterval * (int)(m_TimeSinceLastBloodParUpdate / UpdateInterval);
	}*/
	
	NeedReDraw = true;
	//first rend dynamic blood effect
	if(m_DynamicBloodTexturePtr.isNull())
		return false;

	Ogre::RenderTarget * rttTex = m_DynamicBloodTexturePtr->getBuffer()->getRenderTarget();

	bool bNeedRenderBleed = false;
	if(rttTex)
	{
		m_pQuantityTexSceneMgr->getRootSceneNode()->attachObject(m_BuildQuadObj);
		
		bool bloodeRebuilded = false;
		bool waterRebuilded = false;
		//
		for(size_t c = 0 ; c < m_BuildSectionMats.size() ; c++)
		{
			m_BuildQuadObj->beginUpdate(c);
			if(m_BuildSectionMats[c] == m_DynBloodBuildMatPtr)
			{
			   bloodeRebuilded = bloodsys.BuildBloodStream(m_BuildQuadObj , bNeedRenderBleed);
			}
			if(m_BuildSectionMats[c] == m_DynamicWaterTexMaterialPtr)
			{
			   waterRebuilded = waterSys.BuildWaterStreamTex(m_BuildQuadObj);
			}
			m_BuildQuadObj->end();
		}

		bool rebuilded = bloodeRebuilded || waterRebuilded;

		if(rebuilded && NeedReDraw)
		   rttTex->update();
		
		m_pQuantityTexSceneMgr->getRootSceneNode()->detachObject(m_BuildQuadObj);
		//rttTex->writeContentsToFile("c:\\dynamicblood.bmp");
	}

	//second rend bleed effect
	bool hasbleedUpdated = false;

	if(m_QuantityTexturePtr.isNull())
		return false;

	rttTex = m_QuantityTexturePtr->getBuffer()->getRenderTarget();
	
	if(rttTex&&bNeedRenderBleed)
	{
		m_pQuantityTexSceneMgr->getRootSceneNode()->attachObject(m_BuildQuadObj);
		
		for(size_t c = 0 ; c < m_BuildSectionMats.size() ; c++)
		{
			m_BuildQuadObj->beginUpdate(c);
			if(m_BuildSectionMats[c] == m_BleedBuildMatPtr)
			{
			   hasbleedUpdated = bloodsys.BuildBloodStreamBleed(m_BuildQuadObj);
			}
			m_BuildQuadObj->end();
		}

		if(hasbleedUpdated && NeedReDraw)
		   rttTex->update();

		m_pQuantityTexSceneMgr->getRootSceneNode()->detachObject(m_BuildQuadObj);
	}

	//third step: 冲洗掉血迹
	bool hasWashUpdated = false;
	int nCount = 0;
	rttTex = m_QuantityTexturePtr->getBuffer()->getRenderTarget();
	if(rttTex)
	{
		m_pQuantityTexSceneMgr->getRootSceneNode()->attachObject(m_BuildQuadObj);
		
		for(size_t c = 0 ; c < m_BuildSectionMats.size() ; c++)
		{
			m_BuildQuadObj->beginUpdate(c);
			if(m_BuildSectionMats[c] == m_WaterWashBloodMatPtr)
			{
			   hasWashUpdated = waterSys.WaterWashBlood(m_BuildQuadObj);
			}
			m_BuildQuadObj->end();
		}

		if(hasWashUpdated)
		{
		   rttTex->update();
		   nCount++;
		}

		m_pQuantityTexSceneMgr->getRootSceneNode()->detachObject(m_BuildQuadObj);
	}
	
	return hasbleedUpdated || hasWashUpdated;
}

//__/__
#if(0)
bool MisMedicEffectRender::RendWaterParticles(TextureWaterTrackEffect & waterSys , float dt)
{
	/*
	if(m_DynamicWaterTexturePtr.isNull())
		return false;
	Ogre::RenderTarget * rttTex = m_DynamicWaterTexturePtr->getBuffer()->getRenderTarget();
	bool rebuilded = false;
	if(rttTex)
	{
		m_pQuantityTexSceneMgr->getRootSceneNode()->attachObject(m_BuildQuadObj);
		for(size_t c = 0 ; c < m_BuildSectionMats.size() ; c++)
		{
			m_BuildQuadObj->beginUpdate(c);
			if(m_BuildSectionMats[c] == m_DynamicWaterMaterialPtr)
			{
				rebuilded = waterSys.BuildWaterStream(m_BuildQuadObj);
			}
			m_BuildQuadObj->end();
		}
		if(rebuilded)
		   rttTex->update();
		
		m_pQuantityTexSceneMgr->getRootSceneNode()->detachObject(m_BuildQuadObj);
	}*/
	//绘制半透明水流效果
	bool rebuilded = false;

	Ogre::RenderTarget * rttTex = m_DynamicBloodTexturePtr->getBuffer()->getRenderTarget();
	if(rttTex)
	{
		m_pQuantityTexSceneMgr->getRootSceneNode()->attachObject(m_BuildQuadObj);
		for(size_t c = 0 ; c < m_BuildSectionMats.size() ; c++)
		{
			m_BuildQuadObj->beginUpdate(c);
			if(m_BuildSectionMats[c] == m_DynamicWaterTexMaterialPtr)
			{
				rebuilded = waterSys.BuildWaterStreamTex(m_BuildQuadObj);
			}
			m_BuildQuadObj->end();
		}

		if(rebuilded)
		   rttTex->update();
		
		m_pQuantityTexSceneMgr->getRootSceneNode()->detachObject(m_BuildQuadObj);
	}
	//冲掉血迹
	bool hasbleedUpdated = false;

	//if(rebuilded)
	//{
 //		ComposeNormalTexture(Ogre::ColourValue(1.0f , 1.0f ,1.0f ,1.0f));
	//}
	return hasbleedUpdated;
}
#endif

#if GENERATE_MESH_TEXTURE
void MisMedicEffectRender::ApplyMeshTexture(const std::vector<Ogre::Vector2> & texturecoords)
{
	if(m_meshTexture.isNull())
		return;

	Ogre::RenderTarget * rttTex = m_meshTexture->getBuffer()->getRenderTarget();

	Ogre::ManualObject * QuadObj = m_pQuantityTexSceneMgr->createManualObject("QuadObj");

	m_pQuantityTexSceneMgr->getRootSceneNode()->attachObject(QuadObj);

	QuadObj->begin(m_HeatOrganFaceMaterial->getName(), Ogre::RenderOperation::OT_TRIANGLE_LIST);

	for(size_t quad = 0; quad < texturecoords.size() ; quad++)
	{
		Ogre::Vector2 texturecoord = texturecoords[quad];
		float cx = texturecoord.x - 0.5f;
		float cy = 0.5f - texturecoord.y;
		Ogre::Vector2 UV = texturecoords[quad];

		QuadObj->position(Ogre::Vector3(cx, cy, -1.0));
// 		QuadObj->textureCoord(UV.x, UV.y);
		QuadObj->colour(1 , 1 , 1 , 1);
	}
	QuadObj->end();
	rttTex->update();
	m_pQuantityTexSceneMgr->getRootSceneNode()->detachObject(QuadObj);
	m_pQuantityTexSceneMgr->destroyMovableObject(QuadObj);
	rttTex->writeContentsToFile("c:\\line.bmp");
}
#endif
