#pragma once
#include <map>
#include <vector>
#include "ogre.h"

#include "DrawTool.h"

class CBasicTraining;
//================================================================================================================================


class HotchpotchEditorEventListener
{
public:
	HotchpotchEditorEventListener(){}
	~HotchpotchEditorEventListener(){}
	virtual void OnNewDebugInfo(const QString &str) = 0;
};


class HotchpotchEditor : public OgreWidgetEventListener
{
public:
	static HotchpotchEditor* GetCurrentEditor();
	
	HotchpotchEditor();

	~HotchpotchEditor();

	void Construct(Ogre::SceneManager * scenemgr , CBasicTraining * basic_train);

	//@overridden listener
	void OnMousePressed(char button , int x , int y);

	//@overridden listener
	void OnMouseMoved(char button ,int x , int y);

	//@override listener
	void OnMouseReleased(char button ,int x , int y);

	//@override listener 
	void OnWheelEvent(int delta);
	
	void  AddEventListener(HotchpotchEditorEventListener * listener);

	void  RemoveEventListener(HotchpotchEditorEventListener * listener);

	void Update(float dt, Ogre::Camera * camera);

	int AddViewDetectionObject();

	int AddOrganPainter(MisMedicOrgan_Ordinary *pOrgan);

	bool ExportOrganToObjFile(MisMedicOrgan_Ordinary *pOrgan);

	void SaveOrganPainterResult();

	void LoadImageIntoOrganPainter(Ogre::String & imageName);

	void OrganPainterToggleShowEdge();

	void SetOrganPainterBrushColor(const Ogre::ColourValue & color);

	void SetOrganPainterBrushSize(float size);

	std::vector<HotchpotchEditorEventListener*>m_EditorListeners;

	std::vector<EditorTool::EditorObject *> m_EditorObjs;

	std::vector<EditorTool::ViewDetectionForEditor *> m_ViewDetections;

	std::vector<EditorTool::OrganPainter *> m_pOrganPainters;

private:
	CBasicTraining *m_pHostTrain;
	
};

