#include "EditCamera.h"
#include "IMXDefine.h"
#include "MisNewTraining.h"
#include "TrainingMgr.h"

EditorCamera* gEditCamera = 0;

EditorCamera * EditorCamera::GetGlobalEditorCamera()
{
	return gEditCamera;
}
EditorCamera * EditorCamera::InitGlobalEditorCamera()
{
	if (gEditCamera)
	{
		gEditCamera->m_SceneCamera->getParentNode()->setOrientation(gEditCamera->m_CameParentOrientation);
		gEditCamera->m_SceneCamera->setOrientation(gEditCamera->m_CameOrientation);
		gEditCamera->m_SceneCamera->getParentNode()->setPosition(gEditCamera->m_CameParentPosition);
		gEditCamera->m_SceneCamera->setPosition(gEditCamera->m_CamePosition);
		gEditCamera->m_IsFreezed = false;
		gEditCamera->m_IsCtrlPressed = false;
	}
	else
	{
		gEditCamera = new EditorCamera();
		gEditCamera->m_CameParentOrientation = gEditCamera->m_SceneCamera->getParentNode()->getOrientation();
		gEditCamera->m_CameOrientation = gEditCamera->m_SceneCamera->getOrientation();
		gEditCamera->m_CameParentPosition = gEditCamera->m_SceneCamera->getParentNode()->getPosition();
		gEditCamera->m_CamePosition = gEditCamera->m_SceneCamera->getPosition();
	}
	return gEditCamera;
}
void EditorCamera::DelEditorCamera()
{
    if (gEditCamera)
    {
        gEditCamera = 0;
    }
}

EditorCamera::EditorCamera()
{
	OgreWidget * widget = MXOgreWrapper::Instance()->GetOgreWidgetByName(RENDER_WINDOW_LARGE);
	if(widget)
	{
		widget->AddListener(this);

		m_NewTrain = dynamic_cast<MisNewTraining*>(CTrainingMgr::Get()->GetCurTraining());
		m_SceneCamera = m_NewTrain->m_pLargeCamera;
		m_SelectOrgan = NULL;
		m_NewTrain->m_ineditormode = true;
		m_MousePosX = m_MousePosY = -1;
		m_OrganCenter = Ogre::Vector3::ZERO;

		m_SelectBoxNode = m_NewTrain->m_pOms->GetSceneManager()->getRootSceneNode()->createChildSceneNode("EditCameraNode");
		m_SelectBoxObject = m_NewTrain->m_pOms->GetSceneManager()->createManualObject("EditCameraBox");
		m_SelectBoxObject->setDynamic(true);
		m_SelectBoxObject->setVisible(true);
		if(m_SelectBoxObject)
		   m_SelectBoxNode->attachObject(m_SelectBoxObject);
	}
	m_IsFreezed = false;
	m_IsCtrlPressed = false;
}

EditorCamera::~EditorCamera()
{
// 	delete m_CameMenuWidget;
// 	delete m_EditCameraMenu;
// 	m_EditCameraMenu = 0;
	if (MXOgreWrapper::Get()->GetDefaultSceneManger())
	{
		m_SelectBoxObject->setVisible(false);
		m_SelectBoxObject->clear();
		m_SelectBoxNode->detachObject(m_SelectBoxObject);
		m_NewTrain->m_pOms->GetSceneManager()->destroyMovableObject(m_SelectBoxObject);
		m_NewTrain->m_pOms->GetSceneManager()->getRootSceneNode()->removeAndDestroyChild(m_SelectBoxNode->getName());
	}
	if (CTrainingMgr::Get()->GetCurTraining())
	{
		CTrainingMgr::Get()->GetCurTraining()->m_ineditormode = false;
	}
	OgreWidget * widget = MXOgreWrapper::Instance()->GetOgreWidgetByName(RENDER_WINDOW_LARGE);
	if (widget)
	{
		widget->RemoveListener(this);
	}
}

void EditorCamera::OnMouseReleased(char button , int x , int y)
{
	if (m_IsFreezed)
		return;
}

void EditorCamera::OnMousePressed(char button , int x , int y)
{
	if (m_IsFreezed)
		return;

	if((button & Qt::LeftButton) != 0)
	{
		//m_EditCameraMenu->clear();

		Ogre::Ray mouseRay;
		OgreWidget * widget = MXOgreWrapper::Instance()->GetOgreWidgetByName(RENDER_WINDOW_LARGE);
		Ogre::RenderWindow* rendwin = widget->getOgreRenderWindow();
		Ogre::Real tx = (Ogre::Real)x / (Ogre::Real) rendwin->getWidth();
		Ogre::Real ty = (Ogre::Real)y / (Ogre::Real) rendwin->getHeight();
		m_SceneCamera->getCameraToViewportRay(tx , ty ,&mouseRay);  

		std::map<Ogre::Real,MisMedicOrgan_Ordinary*> seleOrgans;
		std::vector<MisMedicOrganInterface*> trainOrgans;
		m_NewTrain->GetAllOrgan(trainOrgans);

		std::vector<MisMedicOrganInterface*>::iterator itor = trainOrgans.begin();
		for(; itor != trainOrgans.end(); ++itor)
		{
			Ogre::Real dist;
			MisMedicOrgan_Ordinary* ordiOrgan = dynamic_cast<MisMedicOrgan_Ordinary*>(*itor);
			if (ordiOrgan)
			{
				GoPhys::GFPhysSoftBodyFace * bodyface = ordiOrgan->GetRayIntersectFace(mouseRay.getOrigin(), mouseRay.getOrigin() + 1000.0f * mouseRay.getDirection(), dist);
				if (bodyface)
				{
					seleOrgans[dist] = ordiOrgan;
				}
			}
		}
		if (seleOrgans.size())
		{
			m_SelectOrgan = seleOrgans.begin()->second;
		} 
		else
		{
			m_SelectOrgan = 0;
		}
		SelectOrgan();
// 		if (seleOrgans.size())
// 		{
// 			QAction* action = new QAction("bodyCenter", m_EditCameraMenu);
// 			action->setData(1);
// 			m_EditCameraMenu->addAction(action);
// 			
// 			std::map<Ogre::Real,MisMedicOrgan_Ordinary*>::iterator mitor = seleOrgans.begin();
// 			for ( ; mitor != seleOrgans.end(); ++mitor )
// 			{
// 				action = new QAction(QString(mitor->second->getName().c_str()), m_EditCameraMenu);
// 				action->setData((uint)(mitor->second));
// 				m_EditCameraMenu->addAction(action);
// 			}
// 			m_EditCameraMenu->exec(QCursor::pos());
// 		}

	}
}

void EditorCamera::OnMouseMoved(char button , int x , int y)
{
	if (m_IsFreezed == true)
		return;

	int MouseDeltaX , MouseDeltaY;
	if(m_MousePosX < 0)
	{
		m_MousePosX = x;
		m_MousePosY = y;
		return;
	}
	else
	{
		MouseDeltaX = x-m_MousePosX;
		MouseDeltaY = y-m_MousePosY;
		m_MousePosX = x;
		m_MousePosY = y;
	}

	if((button & Qt::MidButton) != 0)
	{
		if(::GetKeyState(VK_MENU)&0x8000)
		{
			Ogre::Quaternion CamRotateX,CamRotateY;
			if(MouseDeltaX != 0)
				 CamRotateX.FromAngleAxis(Ogre::Radian(Ogre::Degree(MouseDeltaX*0.2f)) , m_SceneCamera->getParentNode()->getOrientation().zAxis());
			if(MouseDeltaY != 0)
				 CamRotateY.FromAngleAxis(Ogre::Radian(Ogre::Degree(-MouseDeltaY*0.2f)) , m_SceneCamera->getParentNode()->getOrientation().xAxis());

			Ogre::Quaternion OldParentOrit = m_SceneCamera->getParentNode()->getOrientation();
			Ogre::Quaternion RotateXY = CamRotateY*CamRotateX;
			Ogre::Quaternion CamRotate = RotateXY*OldParentOrit;
			m_SceneCamera->getParentNode()->setOrientation(CamRotate);

			Ogre::Vector3 CamSelfPos = m_SceneCamera->getPosition();
			Ogre::Vector3 ParentPos = m_SceneCamera->getParentNode()->getPosition();
			Ogre::Vector3 RealPos = ParentPos+CamSelfPos;

 			Ogre::Matrix4 backmat;
 			backmat.makeTrans(-1.0*m_OrganCenter);
 			Ogre::Matrix4 forwardmat;
 			forwardmat.makeTrans(m_OrganCenter);
 			Ogre::Vector4 NewPos = forwardmat*RotateXY*backmat*Ogre::Vector4(RealPos);
			m_SceneCamera->getParentNode()->setPosition(NewPos.x, NewPos.y, NewPos.z);
		}
		else
		{
			Ogre::Vector3 OldCamPos = m_SceneCamera->getParentNode()->getPosition();
			Ogre::Vector3 RightVec = m_SceneCamera->getParentNode()->getOrientation().xAxis();
			Ogre::Vector3 UpVec = m_SceneCamera->getParentNode()->getOrientation().zAxis();
			m_SceneCamera->getParentNode()->setPosition(OldCamPos + RightVec*(-MouseDeltaX)*0.05f + UpVec*(-MouseDeltaY)*0.05f);
		}
	}
}

void EditorCamera::OnWheelEvent(int delta)
{
	if (m_IsFreezed == true)
		return;

	Ogre::Vector3 direction = m_SceneCamera->getParentNode()->getOrientation().yAxis();
	Ogre::Vector3 position = m_SceneCamera->getParentNode()->getPosition();

	Ogre::Vector3 Newposition = position + direction.normalisedCopy() * delta * 0.01;
	m_SceneCamera->getParentNode()->setPosition(Newposition);
}

void EditorCamera::OnKeyPress(int whichButton)
{
	if (whichButton == Qt::Key_Space)
	{
		m_IsFreezed = !m_IsFreezed;
	}

	else if (whichButton == Qt::Key_Control)
	{
		m_IsCtrlPressed = true;
	}
}

void EditorCamera::OnKeyRelease(int whichButton)
{
	if (whichButton == Qt::Key_Control)
	{
		m_IsCtrlPressed = false;
	}
}
void EditorCamera::SelectOrgan()
{
	if ( !m_SelectOrgan )
	{
		m_OrganCenter = Ogre::Vector3::ZERO;
		m_SceneCamera->lookAt(0,0,0);
		m_SelectBoxObject->setVisible(false);
		m_SelectBoxObject->clear();
	} 
	else
	{
		GoPhys::GFPhysTransform IdentityTrans;
		IdentityTrans.SetIdentity();
		GoPhys::GFPhysVector3 aabbmin;
		GoPhys::GFPhysVector3 aabbmax;
		m_SelectOrgan->m_physbody->GetSoftBodyShape().GetAabb(IdentityTrans , aabbmin , aabbmax);
		Ogre::Vector3 organcenter;
		GoPhys::GFPhysVector3 gcenter = (aabbmax+aabbmin)/2.0;
		Ogre::Real organradius = (aabbmax-aabbmin).Length()/2.0;
		organcenter.x = gcenter.x();
		organcenter.y = gcenter.y();
		organcenter.z = gcenter.z();
		m_OrganCenter = organcenter;
		RendSelectBox(organcenter,Ogre::Vector3(((aabbmax-aabbmin)/2.0).x(),((aabbmax-aabbmin)/2.0).y(),((aabbmax-aabbmin)/2.0).z()));
	}
}

void EditorCamera::RendSelectBox(const Ogre::Vector3 & center , const Ogre::Vector3 & Extend)
{
	m_SelectBoxObject->setVisible(true);
	m_SelectBoxObject->clear();
	m_SelectBoxObject->begin("BaseWhiteNoLighting" , Ogre::RenderOperation::OT_LINE_LIST);

	Ogre::ColourValue boxColor = Ogre::ColourValue::White;
	m_SelectBoxObject->position(center+Ogre::Vector3(-Extend.x , Extend.y , -Extend.z));
	m_SelectBoxObject->colour(boxColor);

	m_SelectBoxObject->position(center+Ogre::Vector3( Extend.x , Extend.y , -Extend.z));
	m_SelectBoxObject->colour(boxColor);

	m_SelectBoxObject->position(center+Ogre::Vector3(-Extend.x , -Extend.y , -Extend.z));
	m_SelectBoxObject->colour(boxColor);

	m_SelectBoxObject->position(center+Ogre::Vector3(Extend.x , -Extend.y , -Extend.z));
	m_SelectBoxObject->colour(boxColor);

	m_SelectBoxObject->position(center+Ogre::Vector3(Extend.x , -Extend.y , -Extend.z));
	m_SelectBoxObject->colour(boxColor);

	m_SelectBoxObject->position(center+Ogre::Vector3(Extend.x , Extend.y , -Extend.z));
	m_SelectBoxObject->colour(boxColor);

	m_SelectBoxObject->position(center+Ogre::Vector3(-Extend.x , -Extend.y , -Extend.z));
	m_SelectBoxObject->colour(boxColor);

	m_SelectBoxObject->position(center+Ogre::Vector3(-Extend.x , Extend.y , -Extend.z));
	m_SelectBoxObject->colour(boxColor);

	m_SelectBoxObject->position(center+Ogre::Vector3(-Extend.x , Extend.y , -Extend.z));
	m_SelectBoxObject->colour(boxColor);

	m_SelectBoxObject->position(center+Ogre::Vector3( Extend.x , Extend.y , -Extend.z));
	m_SelectBoxObject->colour(boxColor);


	m_SelectBoxObject->position(center+Ogre::Vector3(-Extend.x , -Extend.y , Extend.z));
	m_SelectBoxObject->colour(boxColor);

	m_SelectBoxObject->position(center+Ogre::Vector3(Extend.x , -Extend.y , Extend.z));
	m_SelectBoxObject->colour(boxColor);

	m_SelectBoxObject->position(center+Ogre::Vector3(Extend.x , -Extend.y , Extend.z));
	m_SelectBoxObject->colour(boxColor);

	m_SelectBoxObject->position(center+Ogre::Vector3(Extend.x , Extend.y , Extend.z));
	m_SelectBoxObject->colour(boxColor);

	m_SelectBoxObject->position(center+Ogre::Vector3(-Extend.x , -Extend.y , Extend.z));
	m_SelectBoxObject->colour(boxColor);

	m_SelectBoxObject->position(center+Ogre::Vector3(-Extend.x , Extend.y , Extend.z));
	m_SelectBoxObject->colour(boxColor);

	m_SelectBoxObject->position(center+Ogre::Vector3(-Extend.x , Extend.y , Extend.z));
	m_SelectBoxObject->colour(boxColor);

	m_SelectBoxObject->position(center+Ogre::Vector3( Extend.x , Extend.y , Extend.z));
	m_SelectBoxObject->colour(boxColor);


	m_SelectBoxObject->position(center+Ogre::Vector3(-Extend.x , -Extend.y , -Extend.z));
	m_SelectBoxObject->colour(boxColor);

	m_SelectBoxObject->position(center+Ogre::Vector3(-Extend.x , -Extend.y , Extend.z));
	m_SelectBoxObject->colour(boxColor);

	m_SelectBoxObject->position(center+Ogre::Vector3(Extend.x , -Extend.y , -Extend.z));
	m_SelectBoxObject->colour(boxColor);

	m_SelectBoxObject->position(center+Ogre::Vector3(Extend.x , -Extend.y , Extend.z));
	m_SelectBoxObject->colour(boxColor);

	m_SelectBoxObject->position(center+Ogre::Vector3(Extend.x , Extend.y , -Extend.z));
	m_SelectBoxObject->colour(boxColor);

	m_SelectBoxObject->position(center+Ogre::Vector3(Extend.x , Extend.y , Extend.z));
	m_SelectBoxObject->colour(boxColor);

	m_SelectBoxObject->position(center+Ogre::Vector3(-Extend.x , Extend.y , -Extend.z));
	m_SelectBoxObject->colour(boxColor);

	m_SelectBoxObject->position(center+Ogre::Vector3(-Extend.x , Extend.y , Extend.z));
	m_SelectBoxObject->colour(boxColor);
	m_SelectBoxObject->end();
}
