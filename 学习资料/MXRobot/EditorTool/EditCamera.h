#pragma once
#include <Ogre.h>
#include "collision/GoPhysCollisionlib.h"
#include "dynamic/PhysicBody/GoPhysSoftTube.h"
#include "math/GoPhysTransformUtil.h"
#include "../MXRobot/NewTrain/CustomCollision.h"
#include"UtilityForEditor.h"


class MisNewTraining;
class EditorCamera : public OgreWidgetEventListener
{
public:
	static EditorCamera * GetGlobalEditorCamera();

	static EditorCamera * InitGlobalEditorCamera();

	EditorCamera();

	virtual ~EditorCamera();

	void SetCurrentCamera(Ogre::Camera * pcamera);

    void OnMouseReleased(char button , int x , int y);
	 
    void OnMousePressed(char button , int x , int y);
	
	void OnMouseMoved(char button , int x , int y);
	
	void OnWheelEvent(int delta);

	void OnKeyPress(int whichButton);
	
	void OnKeyRelease(int whichButton);

	void SelectOrgan();

	void RendSelectBox(const Ogre::Vector3 & center , const Ogre::Vector3 & Extend);

    static void DelEditorCamera();
	//
	Ogre::Quaternion m_CameParentOrientation;
	Ogre::Quaternion m_CameOrientation;
	Ogre::Vector3 m_CameParentPosition;
	Ogre::Vector3 m_CamePosition;
	//

	Ogre::Camera * m_SceneCamera;
	MisNewTraining * m_NewTrain;
	MisMedicOrgan_Ordinary* m_SelectOrgan;
	Ogre::SceneNode * m_SelectBoxNode;
	Ogre::ManualObject* m_SelectBoxObject;
// 	EditCameraMenuWidget* m_CameMenuWidget;
// 	QMenu *m_EditCameraMenu;

	Ogre::Vector3 m_OrganCenter;
	int m_MousePosX;
	int m_MousePosY;

	bool m_IsFreezed;//
	bool m_IsCtrlPressed;
};
