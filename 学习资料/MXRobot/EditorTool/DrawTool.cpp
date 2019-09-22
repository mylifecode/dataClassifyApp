#include "stdafx.h"
#include "PhysicsWrapper.h"
#include "MXOgreWrapper.h"
#include "MXOgreGraphic.h"
#include "../NewTrain/MisMedicOrganOrdinary.h"

#include "DrawTool.h"

using namespace EditorTool;

VisualObject::VisualObject()
: m_pManual(NULL) ,
m_Color(Ogre::ColourValue::White)
{
	m_pManual = MXOgreWrapper::Get()->GetDefaultSceneManger()->createManualObject();
	MXOgreWrapper::Get()->GetDefaultSceneManger()->getRootSceneNode()->attachObject(m_pManual);
	m_pManual->setDynamic(true);
}

VisualObject::~VisualObject()
{
	if(m_pManual)
	{
		m_pManual->detachFromParent();
		MXOgreWrapper::Get()->GetDefaultSceneManger()->destroyManualObject(m_pManual);
		m_pManual = NULL;
	}
}

bool VisualObject::IsVisible()
{
	if(m_pManual)
		return m_pManual->isVisible();
	else
		return false;
}

int EditorObject::sObjectCount = 0;
EditorObject::EditorObject()
: m_ObjectId(sObjectCount++) , 
m_Position(Ogre::Vector3::ZERO)
{
	std::stringstream ss;
	ss << m_ObjectId;
	m_Name = std::string("EditorObject") +  ss.str();
}

EditorObject::EditorObject(const std::string & name)
: m_ObjectId(sObjectCount++) ,
m_Name(name) ,
m_Position(Ogre::Vector3::ZERO)
{

}

EditorObject::~EditorObject()
{

}
//==================================================================
Doodle::Doodle(int width , int height , const Ogre::String & name)
: m_Width(width) , 
m_Height(height) , 
m_IsShowEdge(false)
{
	CreateSceneMgrAndCamera(name);
	
	CreateTexture(name);

	CreateMaterial(name);


}

void Doodle::Paint(const Ogre::Vector2 & pos , bool erase /* = false */)
{
	if(m_PaintingResultTexPtr.isNull())
		return;

	if(m_pPaintingResultTexRT)
	{
		//Ogre::Vector3 QuadCenter(pos.x - 0.5f , 0.5f - pos.y , 0);

		float centerX = pos.x - 0.5f;

		float centerY = 0.5f - pos.y;

		m_pDoodleResultManual->clear();
		if(erase)
			m_pDoodleResultManual->begin("Editor/EraserMat");
		else
			m_pDoodleResultManual->begin("Editor/BrushMat");
	
		float radius = m_BrushSize / 2.0f;

		float left = centerX - radius;
		float right = centerX + radius;
		float top = centerY + radius;
		float bottom = centerY - radius;

		m_pDoodleResultManual->position(Ogre::Vector3(left, top, -1.0));
		m_pDoodleResultManual->textureCoord(0, 0);
		m_pDoodleResultManual->colour(m_BrushColor);

		m_pDoodleResultManual->position(Ogre::Vector3(right, top, -1.0));
		m_pDoodleResultManual->textureCoord(1, 0);
		m_pDoodleResultManual->colour(m_BrushColor);

		m_pDoodleResultManual->position(Ogre::Vector3(left, bottom, -1.0));
		m_pDoodleResultManual->textureCoord(0, 1);
		m_pDoodleResultManual->colour(m_BrushColor);

		m_pDoodleResultManual->position(Ogre::Vector3(right , bottom, -1.0));
		m_pDoodleResultManual->textureCoord(1, 1);
		m_pDoodleResultManual->colour(m_BrushColor);

		//using indices
		m_pDoodleResultManual->index(0);
		m_pDoodleResultManual->index(2);
		m_pDoodleResultManual->index(1);
		m_pDoodleResultManual->index(1);
		m_pDoodleResultManual->index(2);
		m_pDoodleResultManual->index(3);

		m_pDoodleResultManual->end();

		m_pPaintingResultTexRT->update();
	}
}

void Doodle::Erase(const Ogre::Vector2 & pos)
{
	static Ogre::ColourValue clearColor = Ogre::ColourValue(0,0,0,0);
	Ogre::ColourValue bak = m_BrushColor;
	m_BrushColor = clearColor;
	Paint(pos , true);
	m_BrushColor = bak;
}

void Doodle::BrushPos(const Ogre::Vector2 & pos)
{
	if(m_BrushPosTexPtr.isNull())
		return;

	if(m_pBrushPosTexRT)
	{
		//Ogre::Vector3 QuadCenter(pos.x - 0.5f , 0.5f - pos.y , 0);

		float centerX = pos.x - 0.5f;

		float centerY = 0.5f - pos.y;

		m_pBrushPosManual->clear();

		m_pBrushPosManual->begin("Editor/BrushMat");

		float radius = m_BrushSize / 2.0f;

		float left = centerX - radius;
		float right = centerX + radius;
		float top = centerY + radius;
		float bottom = centerY - radius;

		m_pBrushPosManual->position(Ogre::Vector3(left, top, -1.0));
		m_pBrushPosManual->textureCoord(0, 0);
		m_pBrushPosManual->colour(m_BrushColor);

		m_pBrushPosManual->position(Ogre::Vector3(right, top, -1.0));
		m_pBrushPosManual->textureCoord(1, 0);
		m_pBrushPosManual->colour(m_BrushColor);

		m_pBrushPosManual->position(Ogre::Vector3(left, bottom, -1.0));
		m_pBrushPosManual->textureCoord(0, 1);
		m_pBrushPosManual->colour(m_BrushColor);

		m_pBrushPosManual->position(Ogre::Vector3(right , bottom, -1.0));
		m_pBrushPosManual->textureCoord(1, 1);
		m_pBrushPosManual->colour(m_BrushColor);

		//using indices
		m_pBrushPosManual->index(0);
		m_pBrushPosManual->index(2);
		m_pBrushPosManual->index(1);
		m_pBrushPosManual->index(1);
		m_pBrushPosManual->index(2);
		m_pBrushPosManual->index(3);

		m_pBrushPosManual->end();

		m_pBrushPosTexRT->update();

		//m_pBrushPosTexRT->writeContentsToFile("c:\\test.png");
	}
}

void Doodle::ClearBrushTrack()
{
	static Ogre::ColourValue clearColor = Ogre::ColourValue(0,0,0,0);
	CleanRenderTexture(m_BrushPosTexPtr , clearColor);
}

void Doodle::ClearDrawingBoard()
{
	static Ogre::ColourValue clearColor = Ogre::ColourValue(0,0,0,0);
	CleanRenderTexture(m_PaintingResultTexPtr , clearColor);
}

void Doodle::SaveResultToFile(Ogre::String & fileName)
{
	m_pPaintingResultTexRT->writeContentsToFile(fileName);
}

void Doodle::LoadImage(Ogre::String & fileName)
{
	Ogre::MaterialPtr material   = Ogre::MaterialManager::getSingleton().getByName("Editor/ClearWithImage");

	Ogre::TexturePtr  Srctextureptr = Ogre::TextureManager::getSingleton().load(fileName , 
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME , 
		Ogre::TEX_TYPE_2D , 0 , 1.0f , true ,Ogre::PF_A8R8G8B8);
	

	ApplyTextureToMaterial(material , Srctextureptr , "SrcImgMap");
	//ApplyTextureToMaterial(material , imageTexture , "SrcImgMap");

	Ogre::RenderTarget * rttTex = m_pPaintingResultTexRT;

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

			Ogre::ManualObject * QuadObj = DstScenemgr->createManualObject("ImageObj");

			DstScenemgr->getRootSceneNode()->attachObject(QuadObj);

			QuadObj->begin(material->getName(), Ogre::RenderOperation::OT_TRIANGLE_LIST);

			QuadObj->position(Ogre::Vector3(-0.5f, 0.5f, -1.0));
			QuadObj->textureCoord(0, 0);

			QuadObj->position(Ogre::Vector3(0.5f, 0.5f, -1.0));
			QuadObj->textureCoord(1, 0);

			QuadObj->position(Ogre::Vector3(-0.5f, -0.5f, -1.0));
			QuadObj->textureCoord(0, 1);

			QuadObj->position(Ogre::Vector3(0.5f, -0.5f, -1.0));
			QuadObj->textureCoord(1, 1);

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

void Doodle::SetShowEdge(bool showEdge)
{
	 m_IsShowEdge = showEdge;
	 float value = m_IsShowEdge ? 2.0f : 0.0f;
	 try
	 {
		 Ogre::Pass * pass = m_MatPtr->getTechnique(0)->getPass(0);
		 Ogre::GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();
		 params->setNamedConstant("IsShowEdge", value);
	 }
	 catch(Ogre::Exception &ex)
	 {
	 }

}

void Doodle::CreateSceneMgrAndCamera(const Ogre::String & name)
{
	//result scene mgr
	bool hasTest = Ogre::Root::getSingleton().hasSceneManager(name + "_DoodleResultSceneMgr");
	if(!hasTest)
	{
		m_pPaintingResultSceneMgr = Ogre::Root::getSingleton().createSceneManager(Ogre::ST_GENERIC, name + "_DoodleResultSceneMgr");
	}
	else
		m_pPaintingResultSceneMgr = Ogre::Root::getSingleton().getSceneManager(name + "_DoodleResultSceneMgr");

	//brush scene mgr
	hasTest = Ogre::Root::getSingleton().hasSceneManager(name + "_DoodleBrushSceneMgr");
	if(!hasTest)
	{
		m_pBrushPosSceneMgr = Ogre::Root::getSingleton().createSceneManager(Ogre::ST_GENERIC, name + "_DoodleBrushSceneMgr");
	}
	else
		m_pBrushPosSceneMgr = Ogre::Root::getSingleton().getSceneManager(name + "_DoodleBrushSceneMgr");
	
	//manualobject
	m_pDoodleResultManual = m_pPaintingResultSceneMgr->createManualObject("DoodleReusltManual");
	m_pPaintingResultSceneMgr->getRootSceneNode()->attachObject(m_pDoodleResultManual);
	m_pDoodleResultManual->setDynamic(true);

	m_pBrushPosManual = m_pBrushPosSceneMgr->createManualObject("BrushPosManual");
	m_pBrushPosSceneMgr->getRootSceneNode()->attachObject(m_pBrushPosManual);
	m_pBrushPosManual->setDynamic(true);

	//camera
	hasTest = m_pPaintingResultSceneMgr->hasCamera("DoodleResultCamera");
	if (hasTest == false)
	{
		m_pDoodleResultCamera  = m_pPaintingResultSceneMgr->createCamera("DoodleResultCamera");
	}
	else
	{
		m_pDoodleResultCamera  = m_pPaintingResultSceneMgr->getCamera("DoodleResultCamera");
	}

	hasTest = m_pBrushPosSceneMgr->hasCamera("BrushPosCamera");
	if (hasTest == false)
	{
		m_pBrushPosCamera  = m_pBrushPosSceneMgr->createCamera("BrushPosCamera");
	}
	else
	{
		m_pBrushPosCamera  = m_pBrushPosSceneMgr->getCamera("BrushPosCamera");
	}
}

void Doodle::CreateTexture(const Ogre::String & name)
{
	m_PaintingResultTexPtr = Ogre::TextureManager::getSingleton().createManual(
		name + "_DoodleResultTex" , 
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME , 
		Ogre::TEX_TYPE_2D , 
		m_Width , 
		m_Height  , 
		0 ,
		Ogre::PF_A8R8G8B8 , //Ogre::PF_FLOAT16_RGBA , 
		Ogre::TU_RENDERTARGET);

	m_PaintingResultTexPtr->setFSAA(0,"");

	m_BrushPosTexPtr = Ogre::TextureManager::getSingleton().createManual(
		name + "_DoodleBrushTex" , 
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME , 
		Ogre::TEX_TYPE_2D , 
		m_Width , 
		m_Height  , 
		0 ,
		Ogre::PF_A8R8G8B8 , //Ogre::PF_FLOAT16_RGBA , 
		Ogre::TU_RENDERTARGET);

	m_BrushPosTexPtr->setFSAA(0 ,"");

	m_pPaintingResultTexRT = m_PaintingResultTexPtr->getBuffer()->getRenderTarget();
	m_pPaintingResultTexRT->addViewport(m_pDoodleResultCamera);
	m_pPaintingResultTexRT->setAutoUpdated(false);
	

	m_pBrushPosTexRT = m_BrushPosTexPtr->getBuffer()->getRenderTarget();
	m_pBrushPosTexRT->addViewport(m_pBrushPosCamera);
	m_pBrushPosTexRT->setAutoUpdated(false);

	InitRenderTexture(m_PaintingResultTexPtr , Ogre::ColourValue(0,0,0,0));
	InitRenderTexture(m_BrushPosTexPtr , Ogre::ColourValue(0,0,0,0));
}

void Doodle::CreateMaterial(const Ogre::String & name)
{
	//Tex todo
	Ogre::MaterialPtr basicMatPtr = Ogre::MaterialManager::getSingleton().getByName("Editor/DoodleMat");
	m_MatPtr = basicMatPtr->clone(name + "_DoodleMat");
	ApplyTextureToMaterial(m_MatPtr , m_PaintingResultTexPtr , "DoodleResult");
	ApplyTextureToMaterial(m_MatPtr , m_BrushPosTexPtr , "BrushPos");

}

void Doodle::InitRenderTexture(Ogre::TexturePtr texture ,const Ogre::ColourValue & color)
{
	Ogre::RenderTarget * rttTex = texture->getBuffer()->getRenderTarget();
	
	if(rttTex)
	{
		rttTex->setAutoUpdated(false);

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

void Doodle::CleanRenderTexture(Ogre::TexturePtr texture , const Ogre::ColourValue & color)
{
	Ogre::RenderTarget * rttTex = texture->getBuffer()->getRenderTarget();
	if(rttTex)
	{
		rttTex->setAutoUpdated(false);
		for(size_t v = 0 ; v < rttTex->getNumViewports() ; v++)
		{
			Ogre::Viewport* vp = rttTex->getViewport(v);
			vp->clear(Ogre::FBT_COLOUR, color);
			rttTex->update();
		}
	}
}
//==================================================================
CoordAxis::CoordAxis()
: m_Size(1.f)
{
}

void CoordAxis::Draw()
{
	
}




OrganShell::OrganShell() 
: m_pOrgan(NULL) , 
m_Thickness(0.0f)
{
	m_MatPtr.setNull();
}

OrganShell::~OrganShell()
{
	m_pOrgan = NULL;
}

void OrganShell::Update(float dt , Ogre::Camera *pCamera)
{
	Draw();
}

// void OrganShell::RespondToInput(const InputState & state)
// {
// 	if(state.Action == IA_MOUSE_MOVE_NOBUTTON)
// 	{
// 		
// 	}
// }

void OrganShell::Draw()
{
	if(m_pOrgan)
	{
		m_pManual->clear();
		m_pManual->begin(m_MatPtr->getName());

		for(size_t v = 0 ; v  < m_pOrgan->m_OrganRendNodes.size() ; v++)
		{
			m_pManual->position(m_pOrgan->m_OrganRendNodes[v].m_CurrPos + m_pOrgan->m_OrganRendNodes[v].m_Normal * m_Thickness);
			m_pManual->normal(m_pOrgan->m_OrganRendNodes[v].m_Normal);
			m_pManual->colour(m_pOrgan->m_OrganRendNodes[v].m_Color);
			m_pManual->textureCoord(m_pOrgan->m_OrganRendNodes[v].m_TextureCoord);
		}

		for (size_t f = 0 ; f < m_pOrgan->m_OriginFaces.size(); f++)
		{
			int vid[3];
			vid[0] = m_pOrgan->m_OriginFaces[f].vi[0];
			vid[1] = m_pOrgan->m_OriginFaces[f].vi[1];
			vid[2] = m_pOrgan->m_OriginFaces[f].vi[2];
			if(m_pOrgan->m_OriginFaces[f].m_physface && m_pOrgan->m_OriginFaces[f].m_NeedRend)
				m_pManual->triangle(vid[0] , vid[1] , vid[2]);
		}
		m_pManual->end();
	}
}

//=============================================================================================
ViewDetectionForEditor::ViewDetectionForEditor()
: m_ViewDetection(m_Position , 0.95 , true)
{
}

bool ViewDetectionForEditor::Update(float dt , Ogre::Camera *pCamera)
{
	bool result = m_ViewDetection.Update(dt , pCamera);
	int colorIndex = result ? 1 : 0;
	m_ViewDetection.Draw(pCamera , colorIndex);
	return result;
}

void ViewDetectionForEditor::SetPosition(const Ogre::Vector3 & pos)
{
	m_Position = pos;
	m_ViewDetection.SetPosition(m_Position);
}

void ViewDetectionForEditor::Translate(const Ogre::Vector3 & translation)
{
	m_Position += translation;
	m_ViewDetection.SetPosition(m_Position);
}

//==================================================================

OrganPainter::OrganPainter(int width /* = 512 */, int height /* = 512 */)
: m_pOrgan(NULL) , 
m_pOrganShell(NULL) , 
m_pDoodle(NULL) ,
m_Width(width) ,
m_Height(height) ,
m_IsActive(true) , 
m_IsShowEdge(false)
{

}

OrganPainter::~OrganPainter()
{

}

void OrganPainter::SetOrgan(MisMedicOrgan_Ordinary *pOrgan)
{
	m_pOrgan = pOrgan;
	m_pOrganShell = new OrganShell;
	m_pDoodle = new Doodle(m_Width , m_Height , pOrgan->getName());
	
	m_pOrganShell->SetOrgan(pOrgan);
	m_pOrganShell->SetThickness(0.01f);

	m_pOrganShell->SetMaterial(m_pDoodle->GetDoodleMatPtr());

	//temp
	m_pDoodle->SetBrushColor(Ogre::ColourValue::Blue);
	m_pDoodle->SetBrushSize(0.01f);
}

void OrganPainter::Update(float dt , Ogre::Camera * pCamera)
{
	m_pOrganShell->Update(dt , pCamera);
}


void OrganPainter::RespondToInput(Ogre::Camera *pCamera ,const InputState & state)
{
	m_pDoodle->ClearBrushTrack();
	
	if(!m_IsActive)
		return;

	Ogre::Ray mouseRay;
	pCamera->getCameraToViewportRay(state.MousePosX , state.MousePosY ,&mouseRay);  
	RayCastResult result = PickPointOnOrgan(m_pOrgan , mouseRay);
	if(result.m_pFace)
	{
		if(state.Action == IA_MOUSE_MOVE_NOBUTTON)
		{
			m_pDoodle->BrushPos(result.m_Uv);
		}
		else if(state.Action == IA_MOUSE_MOVE_LEFTBUTTON)
		{
			m_pDoodle->BrushPos(result.m_Uv);
			m_pDoodle->Paint(result.m_Uv);
		}
		else if(state.Action == IA_MOUSE_MOVE_RIGHTBUTTON)
		{
			m_pDoodle->BrushPos(result.m_Uv);
			m_pDoodle->Erase(result.m_Uv);
		}
	}

 }

void OrganPainter::SetBrushColor(const Ogre::ColourValue & color)
{
	m_pDoodle->SetBrushColor(color);
}

void OrganPainter::SetBrushSize(float size)
{
	m_pDoodle->SetBrushSize(size);
}

void OrganPainter::SaveImageToFile()
{
	m_pDoodle->SaveResultToFile("D:\\OrganMark\\" + m_pOrgan->getName() + ".png");
}

void OrganPainter::LoadImage(Ogre::String & imageName)
{
	m_pDoodle->LoadImage(imageName);
}

void OrganPainter::ShowEdge(bool showEdge)
{
	m_IsShowEdge  = showEdge;
	m_pDoodle->SetShowEdge(m_IsShowEdge);
}


//==================================================================

OrganRender::OrganRender()
: m_pFirstStageManual(NULL) , 
m_pSecondStageManual(NULL)
{

}

void OrganRender::Initialize(int texWidth , int texHeight , MisMedicOrgan_Ordinary * pOrgan)
{
	Ogre::String name = pOrgan->getName();

	//SceneMgr
	bool hasTest = Ogre::Root::getSingleton().hasSceneManager(name + "_FirstStageSceneMgr");
	if(!hasTest)
	{
		m_FirstStageSceneMgr = Ogre::Root::getSingleton().createSceneManager(Ogre::ST_GENERIC, name + "_FirstStageSceneMgr");
	}
	else
		m_FirstStageSceneMgr = Ogre::Root::getSingleton().getSceneManager(name + "_FirstStageSceneMgr");

	//Tex todo
	Ogre::MaterialPtr firstMatPtr = Ogre::MaterialManager::getSingleton().getByName("");
	m_FirstStageMatPtr->clone("FirstStage_" + name);

	Ogre::MaterialPtr secondMatPtr = Ogre::MaterialManager::getSingleton().getByName("");
	m_SecondStageMatPtr->clone("SecondStage_" + name);

	m_DiffuseTexPtr = Ogre::TextureManager::getSingleton().createManual(
		"DiffuseTex_" + name , 
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME , 
		Ogre::TEX_TYPE_2D , 
		texWidth , 
		texHeight  , 
		0 ,
		Ogre::PF_A8R8G8B8 , 
		Ogre::TU_RENDERTARGET);

	//unused now
	m_SecondTargetTexPtr = Ogre::TextureManager::getSingleton().createManual(
		"SecondTex_" + name , 
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME , 
		Ogre::TEX_TYPE_2D , 
		texWidth , 
		texHeight  , 
		0 ,
		Ogre::PF_A8R8G8B8 , 
		Ogre::TU_RENDERTARGET);

	m_DiffuseTexRT = m_DiffuseTexPtr->getBuffer()->getRenderTarget();
	m_DiffuseTexRT->addViewport(m_pCamera);

	m_DiffuseTexRT->setAutoUpdated(false);



}
