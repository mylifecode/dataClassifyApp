#ifndef _MISMEDICEFFECTRENDER_
#define _MISMEDICEFFECTRENDER_
#include "Ogre.h"
#include "MXOgreGraphic.h"
class TextureBloodTrackEffect;
class TextureWaterTrackEffect;
class ITool;
struct MMO_Node;
struct MMO_Face;
class MisMedicOrganRender;

//#include "MisMedicOrganRender.h"

#define     MARKNUM 8
#define		GENERATE_MESH_TEXTURE 0

class MisMedicEffectRender: Ogre::RenderSystem::Listener
{
public:
	MisMedicEffectRender();

	~MisMedicEffectRender();

	void SetDynBldPar_Tex(const Ogre::String & dynBldParTex);

	void Create(const std::string & OrganMaterialName , int efftexWidth, int efftexHeight, Ogre::String name);

	void Destory();

	void InitRenderTextureWithTexture(Ogre::TexturePtr texture , Ogre::String imagename);

	void InitRenderTexture(Ogre::TexturePtr texture , Ogre::ColourValue color);
	void CleanRenderTexture(Ogre::TexturePtr texture , Ogre::ColourValue color);

	//Ogre::TexturePtr GetComposedEffectTexture();

	void clearMarkQuadObject();

	void MarkTexBackgClearEveryFrame(bool autoclear);
	
	void MarkTextureFlush();

	void MarkEffectTexture(Ogre::Vector2 screencoord, float radius, std::string matename);

	void ApplySoakEffect(MisMedicOrganRender* pRenderObj, std::string matename);

	//void ComposeEffectTexture(const Ogre::ColourValue & bleedcolor , float umin = 0 , float umax = 1, float vmin = 0, float vmax = 1);

	void ApplyHeat(Ogre::Vector2 texturecoord , float radius , float value , Ogre::TexturePtr cogBrandTex);

	void ApplyBleeding(Ogre::Vector2 texturecoord, float radius, float value, Ogre::TexturePtr bleedingPointTex);

	void ApplyBleedings(std::vector<Ogre::Vector2> texturecoords, 
		                float radius, 
						float value, 
						Ogre::TexturePtr bleedingPointTex);

	void ApplyCongulate_type_2(const std::vector<Ogre::Vector2> & texturecoords ,const std::vector<Ogre::Vector2> & TFUV , ITool * tool);

	void RendBleedPoints(std::vector<Ogre::Vector2> & point ,
						 std::vector<float> & quantity ,
						 float radius);

	bool RendLiquidParticles(TextureBloodTrackEffect & bloodsys , TextureWaterTrackEffect & waterSys , float dt);
	//__/__
	bool RendWaterParticles(TextureWaterTrackEffect & waterSys , float dt);

	void RendVeinConnectBlood(TextureBloodTrackEffect & bloodsys , float dt);
	
	//void RendBloodEffusionEffect(TextureBloodTrackEffect & bloodsys, float deltavalue);

	void ApplyMeshTexture(const std::vector<Ogre::Vector2> & texturecoords);

	void Initialize();

	void OnDeviceLost();

	void OnDeviceReset();

	void BloodBurnSpread(MisMedicOrganRender* pManualObj, float crossDir);

	//void BurnMarkSpread(MisMedicOrganRender* pManualObj);

	void SetBloodBleedTexture(const Ogre::String & bleedtex);

// 	static void ApplyTextureToMaterial(Ogre::MaterialPtr mat , Ogre::TexturePtr tex , const Ogre::String & unitName);

	//overriden
	void eventOccurred(const Ogre::String& eventName, 
					   const Ogre::NameValuePairList* parameters);

	std::string m_OrganMaterialName;

	bool m_BeCreated;
	std::vector<Ogre::MaterialPtr> m_BuildSectionMats;

	//Ogre::TexturePtr  m_MixAlphaTexturePtr;//ruoyu added

	Ogre::MaterialPtr m_DynamicWaterTexMaterialPtr;//器官表面水流渲染脚本(纹理)
	
	
	Ogre::TexturePtr  m_DynamicBloodTexturePtr;
	Ogre::Viewport *  m_pDynamicBloodTexViewPort;

	Ogre::TexturePtr  m_MarkTexturePtr;///把该纹理付给肉的材质的第二个texture_unit  ApplyTextureToMaterial
	Ogre::Viewport * m_pMarkViewPort;

	//Ogre::MaterialPtr m_ComposedEffectMaterialPtr;
	//Ogre::TexturePtr  m_ComposedEffectTexturePtr;
	//Ogre::Viewport *  m_pComposedTexViewPort;

	//Ogre::TexturePtr m_LatestCoagulationTexPtr;	//最新一次电凝
	Ogre::TexturePtr  m_QuantityTexturePtr;//R G B A R-white value G- value B-heat value  A-BloodValue
	Ogre::TexturePtr  m_QuantityTexturePtr1;
	Ogre::TexturePtr  m_QuantityTexturePtr2;
	//int m_QuantityTextureNum;
	Ogre::TexturePtr  m_DefaultHeatBrandTexPtr;//R G B A R-white value G- value B-heat value  A-BloodValue
	Ogre::String	m_bHeatBrandName;
	//Ogre::MaterialPtr m_LatestCoagulationMaterialPtr;
	//Ogre::MaterialPtr m_JustForClampCoagulationMaterialPtr;
	Ogre::MaterialPtr m_HeatOrganFaceMaterial;
	//Ogre::MaterialPtr m_AlphaPaintMaterial;
	Ogre::MaterialPtr m_BleedingPointMaterial;
	//Ogre::MaterialPtr m_CopyMaterial;
	Ogre::MaterialPtr m_GravityMaterial;
	//Ogre::MaterialPtr m_BurnSpreadMaterial;


	Ogre::SceneManager * m_pQuantityTexSceneMgr;
	Ogre::Camera * m_pQuantityTexCamera;
	Ogre::Viewport * m_pQuantityTexViewPort1;
	Ogre::Viewport * m_pQuantityTexViewPort2;
	Ogre::ManualObject * m_BuildQuadObj;

    Ogre::ManualObject * m_MarkQuadObj;

	Ogre::MaterialPtr m_DynBloodBuildMatPtr;
	Ogre::MaterialPtr m_BleedBuildMatPtr;
	Ogre::MaterialPtr m_VeinBloodBuildMatPtr;

	Ogre::MaterialPtr m_WaterWashBloodMatPtr;


	//Ogre::SceneManager * m_pLatestCoagulationTexSceneMgr;
	//Ogre::Camera * m_pLatestCoagulationTexCamera;
	//Ogre::Viewport * m_pLatestCoagulationTexViewPort;

	//Ogre::TexturePtr  m_BurnWhiteTexturePtr;
	//Ogre::Viewport *  m_pBurnWhiteTexViewPort;

	Ogre::Image m_CacheQuantityTexImage;

	Ogre::Image m_CacheBurnWhiteTexImage;

	float m_deviateScale;

	//Ogre::String m_BurnWhiteSrcImageName;

	float m_TimeSinceLastBloodParUpdate;

	float m_TimeSinceLastVeinBloodUpdate;
#if GENERATE_MESH_TEXTURE
	Ogre::TexturePtr	m_meshTexture;
	Ogre::Viewport *	m_TexViewPort;
#endif

};

#endif