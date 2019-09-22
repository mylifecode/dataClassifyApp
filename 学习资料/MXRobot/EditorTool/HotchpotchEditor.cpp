#include "PhysicsWrapper.h"
#include "BasicTraining.h"
#include "stdafx.h"
#include <direct.h> 
#include "ogrewidget.h"
#include "math/GoPhysTransformUtil.h"
#include "Math/GoPhysMathUtil.h"
#include"MXOgreGraphic.h"
#include"MXOgreWrapper.h"
#include "XMLWrapperTraining.h"
#include "../NewTrain/MisMedicOrganOrdinary.h"

#include "HotchpotchEditor.h"
#include "EditCamera.h"

extern EditorCamera* gEditCamera;
using namespace EditorTool;

static HotchpotchEditor *sEditorInstance = 0;

HotchpotchEditor* HotchpotchEditor::GetCurrentEditor()
{
	return sEditorInstance;
}

HotchpotchEditor::HotchpotchEditor() 
	: m_pHostTrain(NULL) 
{
	sEditorInstance = this;
	OgreWidget *  widget = MXOgreWrapper::Instance()->GetOgreWidgetByName(RENDER_WINDOW_LARGE);
	widget->AddListener(this);
	//widget->m_listener_for_training = this;
}

HotchpotchEditor::~HotchpotchEditor()
{
	//todo
	sEditorInstance = NULL;
	OgreWidget *  widget = MXOgreWrapper::Instance()->GetOgreWidgetByName(RENDER_WINDOW_LARGE);
	widget->RemoveListener(this);
	//widget->m_listener_for_training = NULL;
}


void HotchpotchEditor::Construct(Ogre::SceneManager * scenemgr , CBasicTraining * basic_train)
{
	m_pHostTrain = basic_train;
	m_pHostTrain->m_pHotchpotchEditor = this;
}

void HotchpotchEditor::OnMousePressed(char button , int mx , int my)
{
	if (gEditCamera->m_IsFreezed == false)
		return;
}

void HotchpotchEditor::OnMouseReleased(char button ,int x , int y)
{
	if (gEditCamera->m_IsFreezed == false)
		return;
}

void HotchpotchEditor::OnMouseMoved(char button ,int x , int y)
{
	if (gEditCamera->m_IsFreezed == false)
		return;
		Ogre::RenderWindow * rw =  MXOgreWrapper::Get()->GetRenderWindowByName(RENDER_WINDOW_LARGE);
		Ogre::Real tx = (Ogre::Real)x / (Ogre::Real) rw->getWidth();
		Ogre::Real ty = (Ogre::Real)y / (Ogre::Real) rw->getHeight();

		InputState state;
		state.MousePosX = tx;
		state.MousePosY = ty;

		bool lefton = button & Qt::LeftButton;
		bool righton = button & Qt::RightButton;

		if(lefton && !righton)
			state.Action = IA_MOUSE_MOVE_LEFTBUTTON;
		else if(!lefton && righton)
			state.Action = IA_MOUSE_MOVE_RIGHTBUTTON;
		else if(!lefton && !righton)
			state.Action = IA_MOUSE_MOVE_NOBUTTON;

 		for(size_t i = 0 ; i < m_pOrganPainters.size() ; i++)
		{
			m_pOrganPainters[i]->RespondToInput(m_pHostTrain->m_pLargeCamera , state);
		}
}

void HotchpotchEditor::OnWheelEvent(int delta)
{
	if (gEditCamera->m_IsFreezed == false)
		return;
}

void HotchpotchEditor::AddEventListener(HotchpotchEditorEventListener * listener)
{
	for(size_t i = 0; i < m_EditorListeners.size(); i++)
	{
		if(m_EditorListeners[i] == listener)
			return;
	}
	m_EditorListeners.push_back(listener);
}

void HotchpotchEditor::RemoveEventListener(HotchpotchEditorEventListener * listener)
{
	for(size_t i = 0; i < m_EditorListeners.size(); i++)
	{
		if(m_EditorListeners[i] == listener)
		{
			m_EditorListeners.erase(m_EditorListeners.begin()+i);
			return;
		}
	}
}

void HotchpotchEditor::Update(float dt, Ogre::Camera * pCamera)
{
	for(size_t i = 0 ; i < m_EditorObjs.size() ; i++)
	{
		m_EditorObjs[i]->Update(dt, pCamera);
	}

	for(size_t i = 0 ; i < m_pOrganPainters.size() ; i++)
	{
		m_pOrganPainters[i]->Update(dt , pCamera);
	}

	for(size_t i = 0 ; i < m_EditorObjs.size() ; i++)
	{
		std::vector<std::string> & msgs = m_EditorObjs[i]->m_Messages;
		
		msgs.clear();
	}
}

int HotchpotchEditor::AddViewDetectionObject()
{
	ViewDetectionForEditor * pObj = new ViewDetectionForEditor();
	m_ViewDetections.push_back(pObj);
	m_EditorObjs.push_back(pObj);
	return m_ViewDetections.size();
}

int HotchpotchEditor::AddOrganPainter(MisMedicOrgan_Ordinary *pOrgan)
{
	OrganPainter *pPainter = new OrganPainter(2048,2048);
	pPainter->SetOrgan(pOrgan);
	m_pOrganPainters.push_back(pPainter);
	return m_pOrganPainters.size();
}

bool HotchpotchEditor::ExportOrganToObjFile(MisMedicOrgan_Ordinary *pOrgan)
{
	std::string filename = "d://organobj//" + m_pHostTrain->m_pTrainingConfig->m_Name + "_" + pOrgan->getName() + ".obj";
	std::ofstream file(filename.c_str());
	if(!file)
		return false;

	file << "# " << pOrgan->getName() << std::endl;

	//vertex position
	for(size_t v = 0 ; v  < pOrgan->m_OrganRendNodes.size() ; v++)
	{
		MMO_Node & node = pOrgan->m_OrganRendNodes[v];
		GFPhysSoftBodyNode *pNode = node.m_PhysNode;
		file << "v " << pNode->m_UnDeformedPos.m_x << " "
			 << pNode->m_UnDeformedPos.m_y << " "
			 << pNode->m_UnDeformedPos.m_z << std::endl;
	}

	//texture coord
	for(size_t v = 0 ; v  < pOrgan->m_OrganRendNodes.size() ; v++)
	{
		MMO_Node & node = pOrgan->m_OrganRendNodes[v];
		file << "vt " << node.m_TextureCoord.x << " "
			<< node.m_TextureCoord.y << std::endl;
	}

	file << "g " << pOrgan->getName() << std::endl;

	for (size_t f = 0 ; f < pOrgan->m_OriginFaces.size(); f++)
	{
		int vid[3];
		vid[0] = pOrgan->m_OriginFaces[f].vi[0] + 1;
		vid[1] = pOrgan->m_OriginFaces[f].vi[1] + 1;
		vid[2] = pOrgan->m_OriginFaces[f].vi[2] + 1;
		if(pOrgan->m_OriginFaces[f].m_physface && pOrgan->m_OriginFaces[f].m_NeedRend)
		{
			file << "f " ;
			file << vid[0] << "/" << vid[0] << " ";
			file << vid[1] << "/" << vid[1] << " ";
			file << vid[2] << "/" << vid[2] << " ";
			file << std::endl;
		}
	}
	
	file.close();

	return true;
}

void HotchpotchEditor::SaveOrganPainterResult()
{
	for(size_t i = 0 ; i < m_pOrganPainters.size() ; i++)
	{
		m_pOrganPainters[i]->SaveImageToFile();
	}
}

void HotchpotchEditor::LoadImageIntoOrganPainter(Ogre::String & imageName)
{
	for(size_t i = 0 ; i < m_pOrganPainters.size() ; i++)
	{
		m_pOrganPainters[i]->LoadImage(imageName);
	}
}

void HotchpotchEditor::OrganPainterToggleShowEdge()
{
	for(size_t i = 0 ; i < m_pOrganPainters.size() ; i++)
	{
		bool showEdge = m_pOrganPainters[i]->IsShowEdge();
		m_pOrganPainters[i]->ShowEdge(!showEdge);
	}
}

void HotchpotchEditor::SetOrganPainterBrushColor(const Ogre::ColourValue & color)
{
	for(size_t i = 0 ; i < m_pOrganPainters.size() ; i++)
	{
		m_pOrganPainters[i]->SetBrushColor(color);
	}
}

void HotchpotchEditor::SetOrganPainterBrushSize(float size)
{
	for(size_t i = 0 ; i < m_pOrganPainters.size() ; i++)
	{
		m_pOrganPainters[i]->SetBrushSize(size);
	}
}