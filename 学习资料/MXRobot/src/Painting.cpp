#include "Painting.h"

#include "Topology/GoPhysSoftBodyRestShapeModify.h"
#include "Dynamic/GoPhysDynamicLib.h"
#include "Math/GoPhysTransformUtil.h"

#include "XMLWrapperTool.h"
#include "XMLWrapperOrgan.h"
#include "BasicTraining.h"
#include "MisRobotInput.h"
#include "MXDebugInf.h"
#include "MXEvent.h"
#include "MXEventsDump.h"
#include "EffectManager.h"
#include "XMLWrapperPart.h"
#include "XMLWrapperPursue.h"
#include "InputSystem.h"
//#include "../NewTrain/CustomConstraint.h"
#include "../NewTrain/PhysicsWrapper.h"
#include "../NewTrain/MisMedicOrganOrdinary.h"



CustomPoint::CustomPoint(GFPhysVector3 *position , Ogre::ColourValue color , float size) 
: m_pPosition(position) , m_Color(color) , m_Size(size)
{

}

CustomFace::CustomFace(GFPhysSoftBodyFace * face , Ogre::ColourValue color /* = Ogre::ColourValue::White */)
: m_pFace(face) , m_Color(color)
{

}

CustomEdgeWithNode::CustomEdgeWithNode(GFPhysSoftBodyNode * pNode1 , GFPhysSoftBodyNode * pNode2 , Ogre::ColourValue color /* = Ogre::ColourValue::White */) 
:  m_Color(color)
{
	m_pNodes[0] = pNode1;
	m_pNodes[1] = pNode2;
}


CustomCoordAxis::CustomCoordAxis(GFPhysVector3 *pOrigin , GFPhysVector3 *pXDir , GFPhysVector3 *pYDir , GFPhysVector3 *pZDir ,
								 Ogre::ColourValue xColor /* = Ogre::ColourValue::Red  */, 
								 Ogre::ColourValue yColor /* = Ogre::ColourValue::Green  */, 
								 Ogre::ColourValue zColor /* = Ogre::ColourValue::Blue  */,
								 float axisLength /* = 1.f */)
:m_pOrigin(pOrigin),m_pXDir(pXDir),m_pYDir(pYDir),m_pZDir(pZDir),
m_xColor(xColor) , m_yColor(yColor) , m_zColor(zColor) , 
m_AxisLength(axisLength)
{
}

int TextDisplay::sTextDisplayId = 0;

TextDisplay::TextDisplay(Real width , Real height , Real posLeft ,Real posTop , int maxLine /* = 50 */) : 
m_IsVisible(true) ,
m_pOverlay(NULL),
m_pText(NULL),
m_pContainer(NULL) , 
m_MaxNumOfLine(maxLine) ,
m_NumOfCurrLine(0)
{
	m_TextStr = "";

	m_pOverlay = Ogre::OverlayManager::getSingleton().create("TextDisplay" + Ogre::StringConverter::toString(sTextDisplayId));

	m_pOverlay->setZOrder(500);
	
	m_pContainer = static_cast<Ogre::OverlayContainer*>(Ogre::OverlayManager::getSingleton().createOverlayElement("Panel", "container" + sTextDisplayId));

	m_pOverlay->add2D(m_pContainer);

	m_pText = Ogre::OverlayManager::getSingleton().createOverlayElement("TextArea" , "TextDisplayArea" + Ogre::StringConverter::toString(sTextDisplayId));

	m_pText->setDimensions(width , height);

	m_pText->setMetricsMode(Ogre::GMM_PIXELS);

	m_pText->setPosition(posLeft , posTop);

	
	m_pText->setParameter("font_name", "mx_number");
	m_pText->setParameter("char_height", "16");
	m_pText->setParameter("horz_align", "center");
	m_pText->setColour(Ogre::ColourValue(1.0, 1.0, 1.0));

	m_pContainer->addChild(m_pText);

	m_pOverlay->show();

	sTextDisplayId++;
}

TextDisplay::~TextDisplay()
{
	m_pOverlay->hide();
	Ogre::OverlayManager *overlayManager = Ogre::OverlayManager::getSingletonPtr();
	m_pContainer->removeChild("TextDisplayArea" +  Ogre::StringConverter::toString(sTextDisplayId - 1));
	m_pOverlay->remove2D(m_pContainer);
	overlayManager->destroyOverlayElement(m_pText);
	overlayManager->destroyOverlayElement(m_pContainer);
	overlayManager->destroy(m_pOverlay);
}

void TextDisplay::SetVisible(bool isVisible)
{
	m_IsVisible = isVisible;
	if(m_IsVisible) { 
		m_pOverlay->show();
	} else {
		m_pOverlay->hide();
	}
}

void TextDisplay::SetText(const Ogre::String & text)
{
	m_TextStr = text;
	m_pText->setCaption(m_TextStr);
}

void TextDisplay::AddText(const Ogre::String & text)
{
	if(m_NumOfCurrLine > m_MaxNumOfLine){
		m_NumOfCurrLine = 0;
		m_TextStr = "";
	} else {
		m_TextStr += "\n" + text;
		m_NumOfCurrLine++;
	}
	m_pText->setCaption(m_TextStr);
}

void TextDisplay::Clear()
{
	m_TextStr = "";
	m_pText->setCaption(m_TextStr);
}

void TextDisplay::Update(float dt /* = 0 */)
{

}

PaintingTool::PaintingTool() : m_pManual(NULL)
{
	m_pManual =MXOgreWrapper::Get()->GetDefaultSceneManger()->createManualObject();
	MXOgreWrapper::Get()->GetDefaultSceneManger()->getRootSceneNode()->attachObject(m_pManual);
	m_pManual->setDynamic(true);

}

PaintingTool::~PaintingTool()
{
	if(m_pManual)
	{
		m_pManual->detachFromParent();
		MXOgreWrapper::Get()->GetDefaultSceneManger()->destroyManualObject(m_pManual);
		m_pManual = NULL;
	}

}

void PaintingTool::PushBackPoint(CustomPoint point)
{
	m_Points.push_back(point);
}

void PaintingTool::PushBackFace(CustomFace face)
{
	m_Faces.push_back(face);
}

void PaintingTool::PushBackEdge(CustomEdgeWithNode & edge)
{
	m_EdgesWithNode.push_back(edge);
}


void PaintingTool::PushBackCoordAxis(CustomCoordAxis coordAxis)
{
	m_Axes.push_back(coordAxis);
}

void PaintingTool::PushBackPoly2D(const std::vector<Ogre::Vector2> & vertices , bool IsDrawTri)
{
	CustomPolygon2D poly;
	poly.m_Vertices = vertices;
	poly.m_IsDrawTri = IsDrawTri;
	m_Polygon2Ds.push_back(poly);
}

void PaintingTool::ChangeFaceColor(GFPhysSoftBodyFace *pTargetFace , const Ogre::ColourValue & color)
{
	std::list<CustomFace>::iterator itor = m_Faces.begin();
	for(; itor != m_Faces.end() ; ++itor)
	{
		CustomFace & face = *itor;
		if(pTargetFace ==  face.m_pFace)
		{
			face.m_Color = color;
			return;
		}
	}
}

void PaintingTool::EraseFace(GFPhysSoftBodyFace *pTargetFace)
{
	std::list<CustomFace>::iterator itor = m_Faces.begin();
	for(; itor != m_Faces.end() ; ++itor)
	{
		CustomFace & face = *itor;
		if(pTargetFace ==  face.m_pFace)
		{
			m_Faces.erase(itor);
			return;
		}
	}
}

void PaintingTool::ClearPoints()
{
	m_Points.clear();
}

void PaintingTool::ClearFaces()
{
	m_Faces.clear();
}

void PaintingTool::ClearEdges()
{
	m_EdgesWithNode.clear();
}

bool PaintingTool::Update(float dt , Ogre::Camera *pCamera /* = NULL */)
{
	m_pManual->clear();
	DrawPoints();
	DrawFace();
	DrawEdge();
	DrawAxis();
	if(pCamera)
		DrawPolygon2D(pCamera , 0.5);
	return true;
}

void PaintingTool::DrawPoints()
{
	if(m_Points.empty())
		return;

    m_pManual->begin("MisMedical/BaseOpaqueTemplate_NoLit");

	int offset = 0;

	for(int i = 0 ; i < m_Points.size() ; i++)
	{
		CustomPoint &point = m_Points[i];
		DrawOnePoint(*point.m_pPosition , point.m_Color , point.m_Size ,offset);
	}

	m_pManual->end();
}

void PaintingTool::DrawFace()
{
	if(m_Faces.empty())
		return;

	m_pManual->begin("BaseWhiteNoLighting");
	int index = 0;
	std::list<CustomFace>::iterator itor = m_Faces.begin();

	for(; itor != m_Faces.end() ; ++itor)
	{
		CustomFace & face = *itor;/*m_Faces[f];*/
		GFPhysSoftBodyFace * pFace = face.m_pFace;
		Ogre::Vector3 increment = GPVec3ToOgre(pFace->m_FaceNormal * 0.01);
		m_pManual->position(Ogre::Vector3(pFace->m_Nodes[0]->m_CurrPosition.m_x , pFace->m_Nodes[0]->m_CurrPosition.m_y , pFace->m_Nodes[0]->m_CurrPosition.m_z) + increment); 
		m_pManual->colour(face.m_Color);

		m_pManual->position(Ogre::Vector3(pFace->m_Nodes[1]->m_CurrPosition.m_x , pFace->m_Nodes[1]->m_CurrPosition.m_y , pFace->m_Nodes[1]->m_CurrPosition.m_z) + increment); 
		m_pManual->colour(face.m_Color);

		m_pManual->position(Ogre::Vector3(pFace->m_Nodes[2]->m_CurrPosition.m_x , pFace->m_Nodes[2]->m_CurrPosition.m_y , pFace->m_Nodes[2]->m_CurrPosition.m_z) + increment); 
		m_pManual->colour(face.m_Color);

		m_pManual->triangle(index + 0 , index + 1 ,index + 2);
		index+=3;
	}

	m_pManual->end();
}

void PaintingTool::DrawEdge()
{
	if(m_EdgesWithNode.empty())
		return;

	m_pManual->begin("BaseWhiteNoLighting" , Ogre::RenderOperation::OT_LINE_LIST);

	std::list<CustomEdgeWithNode>::iterator itor = m_EdgesWithNode.begin();
	
	for( ; itor != m_EdgesWithNode.end() ; ++itor)
	{
		CustomEdgeWithNode & edge = *itor;
		
		if(edge.m_pNodes[0] && edge.m_pNodes[1])
		{
			m_pManual->position(GPVec3ToOgre(edge.m_pNodes[0]->m_CurrPosition));
			m_pManual->colour(edge.m_Color);
			m_pManual->position(GPVec3ToOgre(edge.m_pNodes[1]->m_CurrPosition));
			m_pManual->colour(edge.m_Color);
		}

	}

	m_pManual->end();

}

void PaintingTool::DrawAxis()
{
	if(m_Axes.empty())
		return;

	m_pManual->begin("BaseWhiteNoLighting" , Ogre::RenderOperation::OT_LINE_LIST);

	std::list<CustomCoordAxis>::iterator itor = m_Axes.begin();

	for(; itor != m_Axes.end() ; ++itor)
	{
		CustomCoordAxis & coordAxis = *itor;
		
		m_pManual->position(GPVec3ToOgre(*coordAxis.m_pOrigin));
		m_pManual->colour(coordAxis.m_xColor);
		m_pManual->position(GPVec3ToOgre(*coordAxis.m_pOrigin + *coordAxis.m_pXDir));
		m_pManual->colour(coordAxis.m_xColor);

		m_pManual->position(GPVec3ToOgre(*coordAxis.m_pOrigin));
		m_pManual->colour(coordAxis.m_yColor);
		m_pManual->position(GPVec3ToOgre(*coordAxis.m_pOrigin + *coordAxis.m_pYDir));
		m_pManual->colour(coordAxis.m_yColor);

		m_pManual->position(GPVec3ToOgre(*coordAxis.m_pOrigin));
		m_pManual->colour(coordAxis.m_zColor);
		m_pManual->position(GPVec3ToOgre(*coordAxis.m_pOrigin + *coordAxis.m_pZDir));
		m_pManual->colour(coordAxis.m_zColor);
	}
	m_pManual->end();
}

void PaintingTool::DrawOnePoint(const GFPhysVector3& gvposition , Ogre::ColourValue color , float size , int &offset)
{
	Ogre::Vector3 position = GPVec3ToOgre(gvposition);

	m_pManual->position( position + Ogre::Vector3(-size , -size , -size) );   //0
	m_pManual->colour(color);

	m_pManual->position( position + Ogre::Vector3(size , -size , -size) );    //1
	m_pManual->colour(color);	

	m_pManual->position( position + Ogre::Vector3(size , -size , size) );    //2
	m_pManual->colour(color);

	m_pManual->position( position + Ogre::Vector3(-size , -size , size) );    //3
	m_pManual->colour(color);

	m_pManual->position( position + Ogre::Vector3(-size , size , -size) );    //4
	m_pManual->colour(color);

	m_pManual->position( position + Ogre::Vector3(size , size , -size) );    //5
	m_pManual->colour(color);

	m_pManual->position( position + Ogre::Vector3(size , size , size) );    //6
	m_pManual->colour(color);

	m_pManual->position( position + Ogre::Vector3(-size , size , size));    //7
	m_pManual->colour(color);

	//index
	m_pManual->triangle(0 + offset, 2 + offset, 1 + offset);
	m_pManual->triangle(0 + offset, 2+ offset, 3+ offset);
	m_pManual->triangle(3+ offset, 4+ offset, 0+ offset);
	m_pManual->triangle(3+ offset, 7+ offset, 4+ offset);
	m_pManual->triangle(4+ offset, 7+ offset, 6+ offset);
	m_pManual->triangle(4+ offset, 6+ offset, 5+ offset);
	m_pManual->triangle(5+ offset, 2+ offset, 1+ offset);
	m_pManual->triangle(5+ offset, 6+ offset, 2+ offset);
	m_pManual->triangle(0+ offset, 4+ offset, 1+ offset);
	m_pManual->triangle(5+ offset, 1+ offset, 4+ offset);
	m_pManual->triangle(3+ offset, 6+ offset, 7+ offset);
	m_pManual->triangle(3+ offset, 2+ offset, 6+ offset);

	offset += 8;
}

void PaintingTool::DrawPolygon2D(Ogre::Camera *pCamera , float scale /* = 1.0 */)
{
	if(m_Polygon2Ds.empty())
		return;
	static Ogre::ColourValue color = Ogre::ColourValue(0,1,1,1);
	Ogre::Real		nearClip = pCamera->getNearClipDistance();
	Ogre::Vector3	origin = pCamera->getRealDirection() * nearClip + pCamera->getRealPosition();
	Ogre::Vector3	right = pCamera->getRealRight() * scale;
	Ogre::Vector3	up = pCamera->getRealUp() * scale;
	Ogre::Vector3	dir = pCamera->getRealDirection() * scale;

	m_pManual->begin("BaseWhiteNoLighting" , Ogre::RenderOperation::OT_LINE_LIST);

	std::list<CustomPolygon2D>::iterator itor = m_Polygon2Ds.begin();
		
	std::vector<Ogre::Vector3> temp;

	for( ; itor != m_Polygon2Ds.end() ; ++itor)
	{
		temp.clear();
		CustomPolygon2D & polygon = *itor;
		for(int v = 0 ; v < polygon.m_Vertices.size() ; v++)
		{
			temp.push_back(origin + right * polygon.m_Vertices[v].x + up * polygon.m_Vertices[v].y);
		}
		if(polygon.m_IsDrawTri && temp.size() >= 3)
		{
			Ogre::Vector3 & first = temp[0];
			for(int v = 1 ; v < temp.size() - 1 ; v++)
			{
				m_pManual->position(first);
				m_pManual->colour(color);
				m_pManual->position(temp[v]);
				m_pManual->colour(color);

				m_pManual->position(temp[v]);
				m_pManual->colour(color);
				m_pManual->position(temp[v + 1]);
				m_pManual->colour(color);

				m_pManual->position(temp[v + 1]);
				m_pManual->colour(color);
				m_pManual->position(first);
				m_pManual->colour(color);
			}

		}
		else
		{
			for(int v = 0 ; v < temp.size() ; v++)
			{
				m_pManual->position(temp[v]);
				m_pManual->colour(color);
				m_pManual->position(temp[(v + 1) % temp.size()]);
				m_pManual->colour(color);
			}
		}
	}
	m_pManual->end();
}