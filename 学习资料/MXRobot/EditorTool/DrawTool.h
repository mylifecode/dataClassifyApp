#pragma once
#include <Ogre.h>
#include "collision/GoPhysCollisionlib.h"
#include "dynamic/PhysicBody/GoPhysSoftTube.h"
#include "math/GoPhysTransformUtil.h"
#include "../MXRobot/NewTrain/CustomCollision.h"
#include"UtilityForEditor.h"
class MisMedicOrgan_Ordinary;

namespace EditorTool
{
	enum InputType
	{
		IA_MOUSE_MOVE_NOBUTTON	,
		IA_MOUSE_MOVE_LEFTBUTTON,
		IA_MOUSE_MOVE_RIGHTBUTTON,
		IA_MOUSE_LEFT_CLICK,
		IA_MOUSE_LEFT_RELEASE,
		IA_MOUSE_RIGHT_CLICK,
		IA_MOUSE_RIGHT_RELEASE,
		//TODO
	};

	struct InputState
	{
		InputType Action;
		float MousePosX;
		float MousePosY;
		//TODO
	};
//Ã»Ð´¿½±´ =ÓÐ¿Ó
	class VisualObject
	{
	public:
		VisualObject();
		virtual ~VisualObject();
		bool IsVisible();
		void SetVisible(bool visible) { if(m_pManual) m_pManual->setVisible(visible);}
		void SetColor(const Ogre::ColourValue & color) { m_Color = color;}
		virtual void RespondToInput(const InputState & state) {}
	protected:
		Ogre::ManualObject *m_pManual;
		Ogre::ColourValue m_Color;
	};

	class EditorObject
	{
	public:
		EditorObject();
		EditorObject(const std::string & name);
		virtual ~EditorObject();

		virtual bool Update(float dt , Ogre::Camera *pCamera) {return true;}

		virtual void RespondToInput(const InputState & state) {}

		virtual void SetPosition(const Ogre::Vector3 & pos) { m_Position = pos;}

		virtual const Ogre::Vector3 & GetPosition() { return m_Position;}

		virtual void Translate(const Ogre::Vector3 & translation) { m_Position += translation;}
		
		const std::string & GetName() { return m_Name;}

		std::vector<std::string> m_Messages;
	protected:
		static int sObjectCount;

		Ogre::Vector3 m_Position;
		int m_ObjectId;
		std::string m_Name;
	};

//===============================================================================

	class Doodle
	{
		public:
			Doodle(int width , int height , const Ogre::String & name);

			~Doodle();

			void Paint(const Ogre::Vector2 & pos , bool erase = false);

			void Erase(const Ogre::Vector2 & pos);

			void BrushPos(const Ogre::Vector2 & pos);

			void ClearBrushTrack();

			void ClearDrawingBoard();

			void SetBrushSize(float size) { m_BrushSize = size;}

			void SetBrushColor(const Ogre::ColourValue & color) { m_BrushColor = color;}   

			void SetShowEdge(bool showEdge);

			Ogre::MaterialPtr GetDoodleMatPtr() { return m_MatPtr;}

			void SaveResultToFile(Ogre::String & fileName);

			void LoadImage(Ogre::String & fileName);


		private:  
			void CreateSceneMgrAndCamera(const Ogre::String & name);

			void CreateTexture(const Ogre::String & name);

			void CreateMaterial(const Ogre::String & name);

			void InitRenderTexture(Ogre::TexturePtr texture , const Ogre::ColourValue & color);

			void CleanRenderTexture(Ogre::TexturePtr texture , const Ogre::ColourValue & color);	

			Ogre::MaterialPtr m_MatPtr;

			//PaintingResult
			Ogre::SceneManager * m_pPaintingResultSceneMgr;
			Ogre::ManualObject * m_pDoodleResultManual;
			Ogre::TexturePtr  m_PaintingResultTexPtr;
			Ogre::RenderTarget *m_pPaintingResultTexRT;
			Ogre::Viewport *  m_PaintingResultTexViewPort;
			Ogre::ManualObject *m_pManualForResult;

			//ShowBrush
			Ogre::SceneManager * m_pBrushPosSceneMgr;
			Ogre::ManualObject * m_pBrushPosManual;
			Ogre::TexturePtr  m_BrushPosTexPtr;
			Ogre::RenderTarget *m_pBrushPosTexRT;
			Ogre::Viewport *  m_BrushPosTexViewPort;
			Ogre::ManualObject *m_pManualForBrush;

			Ogre::Camera * m_pDoodleResultCamera;
			Ogre::Camera * m_pBrushPosCamera;

			float m_BrushSize;

			Ogre::ColourValue m_BrushColor;

			float m_Width;
			float m_Height;

			bool m_IsShowEdge;
	};

	class CoordAxis : public VisualObject
	{
	public:
		CoordAxis();
		~CoordAxis() {};
		void SetPosition(const Ogre::Vector3 & pos) { m_Position = pos;}
		const Ogre::Vector3 & GetPosition() { return m_Position;}
		void SetSize(float size) {m_Size = size;}
		void Update(float dt , Ogre::Camera *pCamera);
		//virtual void RespondToInput(const InputState & state);
	private:
		void Draw();
		float m_Size;
		Ogre::Vector3 m_Position;
	};

	class OrganShell : public VisualObject
	{
	public:
		OrganShell();
		~OrganShell();
		void SetOrgan(MisMedicOrgan_Ordinary * pOrgan) {m_pOrgan = pOrgan;}
		void SetThickness(float thickness) { m_Thickness = thickness;}
		void SetMaterial(Ogre::MaterialPtr & matPtr) {m_MatPtr = matPtr;}
		void Update(float dt , Ogre::Camera *pCamera);
	private:
		void Draw();
		
		MisMedicOrgan_Ordinary * m_pOrgan;
		float m_Thickness;
		Ogre::MaterialPtr m_MatPtr;
	};



	class ViewDetectionForEditor : public EditorObject
	{
	public:
		ViewDetectionForEditor();
		virtual bool Update(float dt , Ogre::Camera *pCamera);
		virtual void SetPosition(const Ogre::Vector3 & pos);
		virtual void Translate(const Ogre::Vector3 & translation);
		void SetMinCos(float minCos) { m_ViewDetection.SetMinCos(minCos);}
		void SetDetectDist(float dist) { m_ViewDetection.SetDetectDist(dist);}
		void SetFaceTo(const Ogre::Vector3 & dir) { m_ViewDetection.SetFaceTo(dir);}

		float GetMinCos() { return m_ViewDetection.GetMinCos();}
		float GetDetectDist() { return m_ViewDetection.GetDetectDist();}

		void DetectDist(bool detect) { m_ViewDetection.DetectDist(detect);}
		void DetectDir(bool detect) { m_ViewDetection.DetectDir(detect);}

	private:
		ViewDetection m_ViewDetection;

	};
//=================================================================================================================
class OrganPainter
{
public:
	OrganPainter(int width = 512, int height = 512);
	~OrganPainter();
	void SetOrgan(MisMedicOrgan_Ordinary *pOrgan);
	void Update(float dt , Ogre::Camera * pCamera);
	void RespondToInput(Ogre::Camera *pCamera ,const InputState & state);
	void SetBrushColor(const Ogre::ColourValue & color);
	void SetBrushSize(float size);
	void SaveImageToFile();
	void LoadImage(Ogre::String & imageName);

	void SetActive(bool isActive) { m_IsActive = isActive; }

	bool IsActive() { return m_IsActive; }

	void SetVisible(bool isVisible) { m_pOrganShell->SetVisible(isVisible); }

	bool IsShowEdge() { return m_IsShowEdge; }

	void ShowEdge(bool showEdge);

private:
	MisMedicOrgan_Ordinary *m_pOrgan;
	OrganShell *m_pOrganShell;
	Doodle *m_pDoodle;

	int m_Width;
	int m_Height;

	bool m_IsActive;
	bool m_IsShowEdge;

};

}
//==================================================================
class OrganRender
{
public:
	OrganRender();
	~OrganRender();

	void Initialize(int texWidth , int texHeight , MisMedicOrgan_Ordinary * pOrgan);

	void InitRenderTexture(Ogre::TexturePtr texture , Ogre::ColourValue color);

	void CleanRenderTexture(Ogre::TexturePtr texture , Ogre::ColourValue color);

	void Update(float dt , Ogre::Camera * pCamera);

private:
	Ogre::TexturePtr m_DiffuseTexPtr;
	Ogre::RenderTarget *m_DiffuseTexRT;

	Ogre::TexturePtr m_SecondTargetTexPtr;
	Ogre::RenderTarget *m_SecondTexRT;

	Ogre::SceneManager * m_FirstStageSceneMgr;
	Ogre::Viewport * m_FirstStageViewPort;

	Ogre::MaterialPtr m_FirstStageMatPtr;
	Ogre::MaterialPtr m_SecondStageMatPtr;

	
	Ogre::Camera * m_pCamera;
	Ogre::Light * m_pLight;


	Ogre::ManualObject *m_pFirstStageManual;
	Ogre::ManualObject *m_pSecondStageManual;

	
};