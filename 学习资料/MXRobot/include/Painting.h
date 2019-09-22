#pragma once
#include "ITool.h"
#include <vector>
#include "Dynamic/GoPhysDynamicLib.h"
#include "MXOgreGraphic.h"
using namespace::std;
using namespace GoPhys;

class CustomPoint
{
public:
	//CustomPoint(GFPhysVector3 *position);
	CustomPoint(GFPhysVector3 *position , Ogre::ColourValue color = Ogre::ColourValue::White , float size = 0.05);
	GFPhysVector3 *m_pPosition;
	Ogre::ColourValue m_Color;
	float m_Size;
};

class CustomFace
{
public:
	CustomFace(GFPhysSoftBodyFace * face , Ogre::ColourValue color = Ogre::ColourValue::White);
	GFPhysSoftBodyFace *m_pFace;
	Ogre::ColourValue m_Color;
};

class CustomEdgeWithNode
{
public:
	CustomEdgeWithNode(GFPhysSoftBodyNode * pNode1 , GFPhysSoftBodyNode * pNode2 , Ogre::ColourValue color = Ogre::ColourValue::White);
	GFPhysSoftBodyNode * m_pNodes[2];
	Ogre::ColourValue m_Color;
};

class CustomCoordAxis
{
public:
	CustomCoordAxis(GFPhysVector3 *pOrigin , GFPhysVector3 *pXDir , GFPhysVector3 *pYDir , GFPhysVector3 *pZDir ,
							Ogre::ColourValue xColor = Ogre::ColourValue::Red , 
							Ogre::ColourValue yColor = Ogre::ColourValue::Green , 
							Ogre::ColourValue zColor = Ogre::ColourValue::Blue , 
							float axisLength = 1.f);

	GFPhysVector3 *m_pOrigin;
	GFPhysVector3 *m_pXDir, *m_pYDir, *m_pZDir;
	Ogre::ColourValue m_xColor,m_yColor,m_zColor;
	float m_AxisLength;
};

class CustomPolygon2D
{
public:
	CustomPolygon2D() : m_IsDrawTri(false) {}
	std::vector<Ogre::Vector2> m_Vertices;
	bool m_IsDrawTri;
};

class TextDisplay
{
public:
	TextDisplay(Real width , Real height , Real posLeft ,Real posTop , int maxLine = 50);
	~TextDisplay();
	void SetVisible(bool isVisible);
	void SetText(const Ogre::String & text);
	void AddText(const Ogre::String & text);
	void Clear();
	void Update(float dt = 0);
	void SetMaxLine(int max) { m_MaxNumOfLine = max;}
private:
	static int sTextDisplayId;
	bool m_IsVisible;
	Ogre::Overlay * m_pOverlay;
	Ogre::OverlayElement * m_pText;
	Ogre::OverlayContainer * m_pContainer;
	Ogre::String m_TextStr;

	Ogre::Vector2 m_Size;
	Ogre::Vector2 m_Pos;

	int m_MaxNumOfLine;
	int m_NumOfCurrLine;
};


class PaintingTool
{
public:
	PaintingTool();
	~PaintingTool(void);
	
	void PushBackPoint(CustomPoint point);
	void PushBackFace(CustomFace face);
	void PushBackEdge(CustomEdgeWithNode & edge);
	void PushBackCoordAxis(CustomCoordAxis coordAxis);
	void PushBackPoly2D(const std::vector<Ogre::Vector2> & vertices , bool IsDrawTri = false);


	void ChangeFaceColor(GFPhysSoftBodyFace *pTargetFace , const Ogre::ColourValue & color);
	void EraseFace(GFPhysSoftBodyFace *pTargetFace);
	void ClearPoints();
	void ClearFaces();
	void ClearEdges();

	bool Update(float dt , Ogre::Camera *pCamera = NULL);

private:
	void DrawPoints();
	void DrawOnePoint(const GFPhysVector3& gvposition , Ogre::ColourValue color , float size , int &offset);
	void DrawFace();
	void DrawEdge();
	void DrawAxis();
	void DrawPolygon2D(Ogre::Camera *pCamera , float scale = 1.0);

	std::vector<CustomPoint> m_Points;
	std::list<CustomFace> m_Faces;
	std::list<CustomEdgeWithNode> m_EdgesWithNode;
	std::list<CustomCoordAxis> m_Axes;

	std::list<CustomPolygon2D> m_Polygon2Ds;


	Ogre::ManualObject *m_pManual;


};
