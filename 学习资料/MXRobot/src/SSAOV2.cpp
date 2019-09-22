#include "stdafx.h"
#include "SSAOV2.h"
#include "MisNewTraining.h"
#include "TrainingMgr.h"
#include "OgreDepthBuffer.h"
#define KERNELSIZE 8

void Generate8X8NoiseTexture()
{
	// Create noise texture
	Ogre::TexturePtr noiseTex = Ogre::TextureManager::getSingleton().createManual("ssaoNoiseTex" ,
																	Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
																	Ogre::TEX_TYPE_2D,
																	8 ,	8,	
																	0,
																	Ogre::PF_A8R8G8B8,  //Ogre::PF_A8R8G8B8,//
																	Ogre::TU_DEFAULT,
																	0);

	const Ogre::PixelBox & pBox = noiseTex->getBuffer()-> lock(Ogre::Image::Box(0 , 0 , 8 , 8) , Ogre::HardwareBuffer::HBL_NORMAL);

	unsigned int * data = (unsigned int*)pBox.data;

	for ( unsigned int i = 0; i < 64; i++ )
	{
		float sample[4];
		sample[0] = ( (float) rand() / (float) RAND_MAX ) * 255.0f; // R
		sample[1] = ( (float) rand() / (float) RAND_MAX ) * 255.0f; // G
		sample[2] = ( (float) rand() / (float) RAND_MAX ) * 255.0f; // B
		sample[3] = ( (float) rand() / (float) RAND_MAX ) * 255.0f; // A

		unsigned int color = (unsigned int) sample[0] | ( (unsigned int) sample[1] << 8 ) | ( (unsigned int) sample[2] << 16 ) | ( (unsigned int) sample[3] << 24 );
		data[i] = color;
	}
	noiseTex->getBuffer()->unlock();

	Ogre::Image teximage;
	noiseTex->convertToImage(teximage);
	teximage.save("c:\\ssapNoise8.bmp");


	
}
//==================================================================================================================================
CSSAO::SSAORenderListener::SSAORenderListener():m_SSAO(0)
{

}

CSSAO::SSAORenderListener::~SSAORenderListener()
{
	if(m_SSAO)
	{
	   m_SSAO->RemoveListener(this);
	   m_SSAO = 0;
	}
}

void CSSAO::SSAORenderListener::SetSSAO(CSSAO * ssao)
{
	m_SSAO = ssao;
	if(m_SSAO)
	   m_SSAO->AddListener(this);
}

CSSAO::CSSAO(void)
{
	//m_SceneTarget = NULL;
	//m_DepthTarget = NULL;
	m_OcclusionNoBlurTarget = NULL;
	m_OcclusionFinalTarget = NULL;

	m_bShadow = false;
	//m_pCurrCamera = NULL;
	//m_pSceneMgr = NULL;

	GenerateKernel_SSAO(KERNELSIZE);
	//Generate8X8NoiseTexture();
}
//==================================================================================================================================
CSSAO::~CSSAO()
{
	destorySSAO();
}
//==================================================================================================================================
void CSSAO::GenerateKernel_SSAO(uint kernelSize)
{
	for ( unsigned int i = 0; i < kernelSize; i++ )
	{
		float z = Ogre::Math::RangeRandom(-1.0 , 1.0);
		float t = ( 0.5f * ( Ogre::Math::RangeRandom(-1.0 , 1.0) + 1.0f ) ) * 2.0f * Ogre::Math::PI;

		float r = sqrtf( 1.0f - z * z );
		float x = r * cos(t);
		float y = r * sin(t);

		float length = Ogre::Math::Sqrt( x * x + y * y + z * z );

		x /= length;
		y /= length;
		z /= length;

		float rLength = 0.5f * (Ogre::Math::RangeRandom(-1.0 , 1.0) + 1.0f );

		x *= rLength;
		y *= rLength;
		z *= rLength;

		float4 vec;

		vec.a = x;
		vec.b = y;
		vec.c = z;
		vec.d = 0.0f;

		m_kernel[i] = vec;
	}

#if(1)
	float guassWeight[7];

	float halfKernalWid = 3;//see shader blurx
	
	float sigma = halfKernalWid / 3; // make 3 kernal cover half ranget

	int c = 0;
	for (float i = -3 ; i <= 3; i++)
	{
		float Weight = expf(-powf(i, 2) / (2 * powf(sigma, 2)));//shader will normalize weight no need take const factor into acount

		guassWeight[c++] = Weight;
	}
    
	int i = 0;
	int j = i+1;
#endif

	if (kernelSize == 8)
	{
		m_kernel[0] = float4(0.01305719, 0.5872321, -0.119337, 0);
		m_kernel[1] = float4(0.3230782, 0.02207272, -0.4188725, 0);
		m_kernel[2] = float4(-0.310725, -0.191367, 0.05613686, 0);
		m_kernel[3] = float4(-0.4796457, 0.09398766, -0.5802653, 0);
		m_kernel[4] = float4(0.1399992, -0.3357702, 0.5596789, 0);
		m_kernel[5] = float4(-0.2484578, 0.2555322, 0.3489439, 0);
		m_kernel[6] = float4(0.1871898, -0.702764, -0.2317479, 0);
		m_kernel[7] = float4(0.8849149, 0.2842076, 0.368524, 0);
	}

	else if (kernelSize == 14)
	{
		m_kernel[0] = float4(0.4010039, 0.8899381, -0.01751772, 0);
		m_kernel[1] = float4(0.1617837, 0.1338552, -0.3530486, 0);
		m_kernel[2] = float4(-0.2305296, -0.1900085, 0.5025396, 0);
		m_kernel[3] = float4(-0.6256684, 0.1241661, 0.1163932, 0);
		m_kernel[4] = float4(0.3820786, -0.3241398, 0.4112825, 0);
		m_kernel[5] = float4(-0.08829653, 0.1649759, 0.1395879, 0);
		m_kernel[6] = float4(0.1891677, -0.1283755, -0.09873557, 0);
		m_kernel[7] = float4(0.1986142, 0.1767239, 0.4380491, 0);
		m_kernel[8] = float4(-0.3294966, 0.02684341, -0.4021836, 0);
		m_kernel[9] = float4(-0.01956503, -0.3108062, -0.410663, 0);
		m_kernel[10] = float4(-0.3215499, 0.6832048, -0.3433446, 0);
		m_kernel[11] = float4(0.7026125, 0.1648249, 0.02250625, 0);
		m_kernel[12] = float4(0.03704464, -0.939131, 0.1358765, 0);
		m_kernel[13] = float4(-0.6984446, -0.6003422, -0.04016943, 0);
	}

	else if (kernelSize == 26)
	{
		m_kernel[0] = float4(0.2196607, 0.9032637, 0.2254677, 0);
		m_kernel[1] = float4(0.05916681, 0.2201506, -0.1430302, 0);
		m_kernel[2] = float4(-0.4152246, 0.1320857, 0.7036734, 0);
		m_kernel[3] = float4(-0.3790807, 0.1454145, 0.100605, 0);
		m_kernel[4] = float4(0.3149606, -0.1294581, 0.7044517, 0);
		m_kernel[5] = float4(-0.1108412, 0.2162839, 0.1336278, 0);
		m_kernel[6] = float4(0.658012, -0.4395972, -0.2919373, 0);
		m_kernel[7] = float4(0.5377914, 0.3112189, 0.426864, 0);
		m_kernel[8] = float4(-0.2752537, 0.07625949, -0.1273409, 0);
		m_kernel[9] = float4(-0.1915639, -0.4973421, -0.3129629, 0);
		m_kernel[10] = float4(-0.2634767, 0.5277923, -0.1107446, 0);
		m_kernel[11] = float4(0.8242752, 0.02434147, 0.06049098, 0);
		m_kernel[12] = float4(0.06262707, -0.2128643, -0.03671562, 0);
		m_kernel[13] = float4(-0.1795662, -0.3543862, 0.07924347, 0);
		m_kernel[14] = float4(0.06039629, 0.24629, 0.4501176, 0);
		m_kernel[15] = float4(-0.7786345, -0.3814852, -0.2391262, 0);
		m_kernel[16] = float4(0.2792919, 0.2487278, -0.05185341, 0);
		m_kernel[17] = float4(0.1841383, 0.1696993, -0.8936281, 0);
		m_kernel[18] = float4(-0.3479781, 0.4725766, -0.719685, 0);
		m_kernel[19] = float4(-0.1365018, -0.2513416, 0.470937, 0);
		m_kernel[20] = float4(0.1280388, -0.563242, 0.3419276, 0);
		m_kernel[21] = float4(-0.4800232, -0.1899473, 0.2398808, 0);
		m_kernel[22] = float4(0.6389147, 0.1191014, -0.5271206, 0);
		m_kernel[23] = float4(0.1932822, -0.3692099, -0.6060588, 0);
		m_kernel[24] = float4(-0.3465451, -0.1654651, -0.6746758, 0);
		m_kernel[25] = float4(0.2448421, -0.1610962, 0.1289366, 0);
	};
}
//==================================================================================================================================
void CSSAO::AddListener(CSSAO::SSAORenderListener * listener)
{
	for(size_t c = 0 ; c < m_listener.size() ; c++)
	{
		if(m_listener[c] == listener)
		   return;
	}
	m_listener.push_back(listener);
}
//==================================================================================================================================
void CSSAO::RemoveListener(CSSAO::SSAORenderListener * listener)
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
void CSSAO::Initialize(Ogre::RenderWindow * renderWindow , DeferredRendFrameWork * framwork)
{
	m_bShadow = true;	
	
	m_CurrWindowWid = renderWindow->getWidth();
	m_CurrWindowHei = renderWindow->getHeight();

	m_OcclusionNoBlurTarget = new DeferedRendTarget();
	m_OcclusionFinalTarget = new DeferedRendTarget();

	m_OcclusionNoBlurTarget->CreateRendeTarget("SAOOcclusionTexture_NoBlur" , m_CurrWindowWid , m_CurrWindowHei , 1.0f , Ogre::PF_A8R8G8B8 , 0);
	m_OcclusionFinalTarget->CreateRendeTarget("SAOOcclusionTexture" , m_CurrWindowWid , m_CurrWindowHei , 1.0f , Ogre::PF_A8R8G8B8 , 0);
	

	// this is the camera you're using
	Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().load("SSAO/CrytekSAO" , Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME).dynamicCast<Ogre::Material>();

	// get the pass
	Ogre::Pass * pass = mat->getTechnique(0)->getPass(0);

	// get the pixel shader parameters
	//Ogre::GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();
	//if (params->_findNamedConstantDefinition("LightCorrection"))
		//params->_get("ProjTexMat", CLIP_SPACE_TO_IMAGE_SPACE * cam->getProjectionMatrixWithRSDepth());
	m_AOLightCorrect = Ogre::Vector4(0.2 , 1.0f , 0 , 0);
}
//=============================================================================================================
void CSSAO::MainRenderWindowSizeChanged(int WinWidth , int WinHeight)
{
	m_OcclusionNoBlurTarget->ResizeRendTarget(WinWidth , WinHeight);
	m_OcclusionFinalTarget->ResizeRendTarget(WinWidth , WinHeight);	
}
//=============================================================================================================
void CSSAO::destorySSAO()
{
	//remove all listener
	for(size_t c = 0 ; c < m_listener.size() ; c++)
	{
		m_listener[c]->m_SSAO = 0;
	}
	m_listener.clear();

	if(m_OcclusionNoBlurTarget)
	{
	   delete m_OcclusionNoBlurTarget;
	   m_OcclusionNoBlurTarget = 0;
	}

	if(m_OcclusionFinalTarget)
	{
	   delete m_OcclusionFinalTarget;
	   m_OcclusionFinalTarget = 0;
	}
	
	m_bShadow = false;
}
void CSSAO::SetAOLightCorrect(const Ogre::Vector4 & correctParam)
{
	m_AOLightCorrect = correctParam;
}
//==================================================================================================================================
void CSSAO::_updateShaderParameter(Ogre::Camera *cam)
{
	// this is the camera you're using
	Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().load("SSAO/CrytekSAO" , Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME).dynamicCast<Ogre::Material>();
	
	// get the pass
	Ogre::Pass * pass = mat->getTechnique(0)->getPass(0);

	// get the pixel shader parameters
	Ogre::GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();

	static const Ogre::Matrix4 CLIP_SPACE_TO_IMAGE_SPACE(
		0.5,    0,    0,  0.5,
		0,   -0.5,    0,  0.5,
		0,      0,    1,    0,
		0,      0,    0,    1);

	if (params->_findNamedConstantDefinition("ProjTexMat"))
		params->setNamedConstant("ProjTexMat", CLIP_SPACE_TO_IMAGE_SPACE * cam->getProjectionMatrixWithRSDepth());

	if(params->_findNamedConstantDefinition("kernel"))
	   params->setNamedConstant("kernel", (float*)m_kernel , KERNELSIZE);

	Ogre::Vector4 KernelSize = Ogre::Vector4( KERNELSIZE , 1 , 1 , 1 );
	if(params->_findNamedConstantDefinition("KernelSize"))
	   params->setNamedConstant("KernelSize", KernelSize);

	if (params->_findNamedConstantDefinition("LightCorrection"))
	    params->setNamedConstant("LightCorrection", m_AOLightCorrect);

}
//==================================================================================================================================
void CSSAO::SceneOpaqueStageFinish(Ogre::RenderWindow * renderWindow , DeferredRendFrameWork * framwork)
{
	if(!m_bShadow)
 		return;

	MisNewTraining * hostTrain = dynamic_cast<MisNewTraining*>(CTrainingMgr::Instance()->GetCurTraining());
	if(hostTrain == 0)
	   return;
	
	//to do deal with rend windows changed, ogre has bug in release render target so not deal currently
	int currWidth  = renderWindow->getWidth();
	int currheight = renderWindow->getHeight();
	if(currWidth != m_CurrWindowWid || currheight != m_CurrWindowHei)
	{	
	   m_CurrWindowWid = currWidth;
	   m_CurrWindowHei = currheight;
	   MainRenderWindowSizeChanged(m_CurrWindowWid , m_CurrWindowHei);
	}
	
	//
	_updateShaderParameter(hostTrain->m_pLargeCamera);
	
	//rend occlusion rend target
	m_OcclusionNoBlurTarget->DrawScreenQuad("SSAO/CrytekSAO");
	
	//blur at X direction
	m_OcclusionFinalTarget->DrawScreenQuad("SSAO/BlurXSry");
	
	//blur at Y direction
	m_OcclusionNoBlurTarget->DrawScreenQuad("SSAO/BlurYSry");
	
	//writeOcclusionTexture();
}

void CSSAO::SceneAllStageFinish(Ogre::RenderWindow * renderWindow  , DeferedRendTarget * finalsceneTarget , DeferredRendFrameWork * framwork)
{

}

void CSSAO::writeOcclusionTexture()
{
	if(m_OcclusionNoBlurTarget)
	{
		m_OcclusionNoBlurTarget->m_RendTarget->writeContentsToFile("c:\\SAOOcclusionNoBlur.bmp");
	}
}

void CSSAO::writeFinalOcclusionTexture()
{
	if(m_OcclusionFinalTarget)
	{
	   m_OcclusionFinalTarget->m_RendTarget->writeContentsToFile("c:\\SAOOcclusionFinal.bmp");
	}
}