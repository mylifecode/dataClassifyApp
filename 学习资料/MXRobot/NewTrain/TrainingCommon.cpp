#include "TrainingCommon.h"
#include "stdafx.h"
#include "MXOgreGraphic.h"
#include "Math/GoPhysVector3.h"
#include "AppendectomyTraining.h"

//===========================MisNewTrainingDebugObject=========================== //

MisNewTrainingDebugObject::MisNewTrainingDebugObject()
:m_pTraining(NULL),
m_pDrawObject(NULL)
{
	OgreWidget *  widget = GetOgreWidget();
	widget->AddListener(this);

}

MisNewTrainingDebugObject::~MisNewTrainingDebugObject()
{
	Destroy();
}

MisNewTrainingDebugObject* MisNewTrainingDebugObject::GetInstance()
{
	static MisNewTrainingDebugObject debugObject;
	return &debugObject;
}

void MisNewTrainingDebugObject::Destroy()
{
	OgreWidget *  widget = GetOgreWidget();
	if(widget)
		widget->RemoveListener(this);

	if(m_pDrawObject)
	{
		delete m_pDrawObject;
		m_pDrawObject = NULL;
	}
}

void MisNewTrainingDebugObject::Update(float dt)
{
	if(m_pDrawObject)
		m_pDrawObject->Update();
}

void MisNewTrainingDebugObject::SetTraining(MisNewTraining* pNewTraining)
{
	m_pTraining = pNewTraining;
	if(m_pTraining)
	{
		OgreWidget *  widget = GetOgreWidget();
		if(widget)
		{
			widget->RemoveListener(this);
			widget->AddListener(this);
		}
	}
	else
	{
		delete m_pDrawObject;
		m_pDrawObject = NULL;
	}
}

DrawObject* MisNewTrainingDebugObject::GetDrawObject()
{
	if(m_pDrawObject == NULL)
		m_pDrawObject = new DrawObject;

	return m_pDrawObject;
}

void MisNewTrainingDebugObject::NormalizeScreenCoordinate(Ogre::Vector2& coordinate,int x,int y)
{
	OgreWidget *  widget = GetOgreWidget();
	int width = widget->width();
	int height = widget->height();

	if(width)
		coordinate.x = (float)x / width;
	else
		coordinate.x = 0.f;

	if(height)
		coordinate.y = (float)y / height;
	else
		coordinate.y = 0.f;
}

void MisNewTrainingDebugObject::OnMousePressed(char button , int x , int y)
{
	if(m_pTraining)
	{
		//左键按下，获取点击的信息
		if(button & LeftButton)
		{
			Ogre::Vector2 normalizedCoordinate;
			NormalizeScreenCoordinate(normalizedCoordinate,x,y);

			Ogre::Ray distRay;
			Ogre::Camera * camera = m_pTraining->m_pLargeCamera;
			camera->getCameraToViewportRay(normalizedCoordinate.x,normalizedCoordinate.y,&distRay);
			
			
			std::vector<MisMedicOrganInterface*> organs;
			m_pTraining->GetAllOrgan(organs);

			float minDis = FLT_MAX;
			GFPhysSoftBodyFace * selectedFace = NULL;

			for(std::size_t o = 0;o < organs.size();++o)
			{
				MisMedicOrgan_Ordinary * ordinaryOrgan = dynamic_cast<MisMedicOrgan_Ordinary*>(organs[o]);
				if(ordinaryOrgan)
				{
					int numFace = ordinaryOrgan->m_physbody->GetNumFace();
					for(int f = 0;f < numFace;++f)
					{
						GFPhysSoftBodyFace * face = ordinaryOrgan->m_physbody->GetFaceAtIndex(f);
						std::pair<bool,Real> result = Ogre::Math::intersects(distRay,
																			GPVec3ToOgre(face->m_Nodes[0]->m_CurrPosition),
																			GPVec3ToOgre(face->m_Nodes[1]->m_CurrPosition),
																			GPVec3ToOgre(face->m_Nodes[2]->m_CurrPosition),
																			true,
																			false);
						if(result.first && result.second < minDis)
						{
							minDis = result.second;
							selectedFace = face;
						}
					}
				}
			}

			if(selectedFace)
			{
				//test
//   				CAppendectomyTraining* curTraining = dynamic_cast<CAppendectomyTraining*>(m_pTraining);
//   				if(curTraining)
//   				{
//   					DrawObject * drawObject = curTraining->m_pDrawObject;
//   
//   					drawObject->AddDynamicPoint(selectedFace->m_Nodes[0],0.025,Ogre::ColourValue::Red);
//   					drawObject->AddDynamicPoint(selectedFace->m_Nodes[1],0.05,Ogre::ColourValue::Red);
//   					drawObject->AddDynamicPoint(selectedFace->m_Nodes[2],0.075,Ogre::ColourValue::Red);
//   
//   					//bool b = curTraining->PositionBetweenInKnotPlane(selectedFace->m_Nodes[0]->m_UnDeformedPos);
//   					//bool b = curTraining->PositionInAppendixArea(selectedFace->m_Nodes[0]->m_UnDeformedPos);
//   					//curTraining->showDebugInfo("0.25  PositionBetweenInKnotPlane test!",b ? 1.f : 0.f);
//   				}
// 				
// 
// 				//show selected info
//  				for(int i = 0;i < 3;++i)
//  				{
//  					m_pTraining->showDebugInfo(selectedFace->m_Nodes[i]->m_CurrPosition.GetX(),
//  						selectedFace->m_Nodes[i]->m_CurrPosition.GetY(),
//  						selectedFace->m_Nodes[i]->m_CurrPosition.GetZ(),',');
//  				}
			}
		}
	}
}

void MisNewTrainingDebugObject::OnMouseReleased(char button , int x , int y)
{

}

void MisNewTrainingDebugObject::OnMouseMoved(char button , int x , int y)
{

}

void MisNewTrainingDebugObject::OnWheelEvent(int delta)
{

}

//===========================MisNewTrainingDebugObject=========================== //


//graphic
//DrawObject
DrawObject::DrawObject()
{
	m_pSceneManager = MXOgre_SCENEMANAGER;
	SY_ASSERT(m_pSceneManager);

	m_pManualObject = m_pSceneManager->createManualObject();
	SY_ASSERT(m_pManualObject);
	m_pManualObject->setDynamic(true);
	m_pManualObject->estimateVertexCount(100);
	m_pManualObject->estimateIndexCount(300);
	m_pSceneManager->getRootSceneNode()->attachObject(m_pManualObject);


	InitSection();
}

DrawObject::~DrawObject()
{
	m_pManualObject->detachFromParent();
	m_pSceneManager->destroyManualObject(m_pManualObject);

}

//material names
static const std::string g_strStaticPointMaterial = "DrawObject/StaticPoints";
static const std::string g_strStaticLineMaterial = "DrawObject/StaticLines";
static const std::string g_strDynamicLineMaterial = "DrawObject/StaticLines";
static const std::string g_strDynamicPointMaterial = "DrawObject/StaticLines";

void DrawObject::InitSection()
{
	//static points - section 0
	m_pManualObject->begin(g_strStaticPointMaterial);
	_DrawPoint(Point(Ogre::Vector3(0,0,0),0.0f,Ogre::ColourValue(1,0,0)),0);
	m_pManualObject->end();

	//static lines - section 1
	m_pManualObject->begin(g_strStaticLineMaterial,Ogre::RenderOperation::OT_LINE_LIST);
	_DrawLine(Line(Ogre::Vector3(0,0,0),Ogre::Vector3(0,0,0),Ogre::ColourValue(0,1.f,0)));
	m_pManualObject->end();

	//dynamic lines - section 2
	m_pManualObject->begin(g_strDynamicLineMaterial,Ogre::RenderOperation::OT_LINE_LIST);
	_DrawLine(Line(Ogre::Vector3(0,0,0),Ogre::Vector3(0,0,0),Ogre::ColourValue(0,1.f,0)));
	m_pManualObject->end();

	//dynamic points - section 3
	m_pManualObject->begin(g_strDynamicPointMaterial);
	_DrawPoint(Point(Ogre::Vector3(0,0,0),0.f,Ogre::ColourValue(1.f,0,0)),0);
	m_pManualObject->end();
}


//===========================Clear=========================== //

void DrawObject::Clear()
{
	m_vecDynamicLines.clear();
}

void DrawObject::ClearStaticPoints()
{
	m_vecPoints.clear();
}

void DrawObject::ClearStaticLines()
{
	m_vecBoundingBox.clear();
	m_vecLines.clear();
}

void DrawObject::ClearDynamicPoints()
{
	m_dynamicPoints.clear();
}

void DrawObject::ClearDynamicLines()
{
	m_vecDynamicLines.clear();
}

//===========================Update=========================== //

void DrawObject::Update()
{
	UpdateDynamicLines();				// can optimalize
	UpdateDynamicPoints();
}

void DrawObject::UpdateStaticPoints()
{
	//section 0
	m_pManualObject->beginUpdate(0);
	for(std::size_t p = 0;p < m_vecPoints.size();++p)
	{
		_DrawPoint(m_vecPoints[p],p);
	}
	m_pManualObject->end();

}

void DrawObject::UpdateStaticLines()
{
	//section 1
	m_pManualObject->beginUpdate(1);
	for(std::size_t l = 0;l < m_vecLines.size();++l)
	{
		_DrawLine(m_vecLines[l]);
	}
	m_pManualObject->end();
}

void DrawObject::UpdateStaticBoundingBox()
{
		/*
		  1-----2
		 /|    /|
		/ |   / |
		5-----4  |
		|  0--|--3
		| /   | /
		|/    |/
		6-----7
		*/
	assert(m_vecBoundingBox.size());	//确保有BoundingBox
	BoundingBox & newBoundingBox = m_vecBoundingBox[m_vecBoundingBox.size() - 1];
	Ogre::ColourValue & color = m_vecBoundingBox[m_vecBoundingBox.size() - 1].m_color;
	std::vector<Ogre::Vector3> tempPoints;	

	tempPoints.push_back(newBoundingBox.m_boundingBox.getCorner(Ogre::AxisAlignedBox::FAR_LEFT_BOTTOM));	//0
	tempPoints.push_back(newBoundingBox.m_boundingBox.getCorner(Ogre::AxisAlignedBox::FAR_LEFT_TOP));		//1
	tempPoints.push_back(newBoundingBox.m_boundingBox.getCorner(Ogre::AxisAlignedBox::FAR_RIGHT_TOP));		//2
	tempPoints.push_back(newBoundingBox.m_boundingBox.getCorner(Ogre::AxisAlignedBox::FAR_RIGHT_BOTTOM));	//3
	tempPoints.push_back(newBoundingBox.m_boundingBox.getCorner(Ogre::AxisAlignedBox::NEAR_RIGHT_TOP));		//4
	tempPoints.push_back(newBoundingBox.m_boundingBox.getCorner(Ogre::AxisAlignedBox::NEAR_LEFT_TOP));		//5
	tempPoints.push_back(newBoundingBox.m_boundingBox.getCorner(Ogre::AxisAlignedBox::NEAR_LEFT_BOTTOM));	//6
	tempPoints.push_back(newBoundingBox.m_boundingBox.getCorner(Ogre::AxisAlignedBox::NEAR_RIGHT_BOTTOM));	//7

	//top lines
	m_vecLines.push_back(Line(tempPoints[1],tempPoints[2],color));	//1
	m_vecLines.push_back(Line(tempPoints[1],tempPoints[5],color));	//2
	m_vecLines.push_back(Line(tempPoints[5],tempPoints[4],color));	//3
	m_vecLines.push_back(Line(tempPoints[2],tempPoints[4],color));	//4
	//mid lines
	m_vecLines.push_back(Line(tempPoints[0],tempPoints[1],color));	//5
	m_vecLines.push_back(Line(tempPoints[6],tempPoints[5],color));	//6
	m_vecLines.push_back(Line(tempPoints[7],tempPoints[4],color));	//7
	m_vecLines.push_back(Line(tempPoints[3],tempPoints[2],color));	//8
	//bottom lines
	m_vecLines.push_back(Line(tempPoints[0],tempPoints[3],color));	//9
	m_vecLines.push_back(Line(tempPoints[0],tempPoints[6],color));	//10
	m_vecLines.push_back(Line(tempPoints[6],tempPoints[7],color));	//11
	m_vecLines.push_back(Line(tempPoints[3],tempPoints[7],color));	//12
	//corss lines
	m_vecLines.push_back(Line(tempPoints[1],tempPoints[4],color));	//12
	m_vecLines.push_back(Line(tempPoints[2],tempPoints[5],color));	//12
	m_vecLines.push_back(Line(tempPoints[0],tempPoints[7],color));	//12
	m_vecLines.push_back(Line(tempPoints[3],tempPoints[6],color));	//12
	m_vecLines.push_back(Line(tempPoints[0],tempPoints[5],color));	//12
	m_vecLines.push_back(Line(tempPoints[1],tempPoints[6],color));	//12
	m_vecLines.push_back(Line(tempPoints[2],tempPoints[7],color));	//12
	m_vecLines.push_back(Line(tempPoints[3],tempPoints[4],color));	//12
	m_vecLines.push_back(Line(tempPoints[1],tempPoints[3],color));	//12
	m_vecLines.push_back(Line(tempPoints[0],tempPoints[2],color));	//12
	m_vecLines.push_back(Line(tempPoints[4],tempPoints[6],color));	//12
	m_vecLines.push_back(Line(tempPoints[5],tempPoints[7],color));	//12

	UpdateStaticLines();
}

void DrawObject::UpdateDynamicLines()
{
	//section 2
	m_pManualObject->beginUpdate(2);
	for(std::size_t l = 0;l < m_vecDynamicLines.size();++l)
	{
		_DrawLine(m_vecDynamicLines[l]);
	}
	m_pManualObject->end();
}

void DrawObject::UpdateDynamicPoints()
{
	//section 3
	m_pManualObject->beginUpdate(3);
	int i = 0;
	for(std::set<Point>::iterator itr = m_dynamicPoints.begin();itr != m_dynamicPoints.end();++itr)
	{
		_DrawPoint(*itr,i++);
	}
	m_pManualObject->end();
}
//=========================================== Point =============================================//
void DrawObject::DrawPoint(const Ogre::Vector3& p,float size,const Ogre::ColourValue& color)
{
	m_vecPoints.push_back(Point(p,size,color));
	UpdateStaticPoints();
}

void DrawObject::DrawPoint(const GFPhysVector3& pos,float size,const Ogre::ColourValue& color)
{
	DrawPoint(GPVec3ToOgre(pos),size,color);
}

void DrawObject::_DrawPoint(const Point& point,size_t pointIndex)
{
	Ogre::Vector3 position;
	if(point.m_pPhysNode)
		position = GPVec3ToOgre(point.m_pPhysNode->m_CurrPosition);
	else
		position = point.m_point;

	float size = point.m_size;
	const Ogre::ColourValue & color = point.m_color;
	size_t offset = pointIndex * 8;

	m_pManualObject->position( position + Ogre::Vector3(-size , -size , -size) );   //0
	m_pManualObject->colour(color);

	m_pManualObject->position( position + Ogre::Vector3(size , -size , -size) );    //1
	m_pManualObject->colour(color);

	m_pManualObject->position( position + Ogre::Vector3(size , -size , size) );		//2
	m_pManualObject->colour(color);

	m_pManualObject->position( position + Ogre::Vector3(-size , -size , size) );    //3
	m_pManualObject->colour(color);

	m_pManualObject->position( position + Ogre::Vector3(-size , size , -size) );    //4
	m_pManualObject->colour(color);

	m_pManualObject->position( position + Ogre::Vector3(size , size , -size) );		//5
	m_pManualObject->colour(color);

	m_pManualObject->position( position + Ogre::Vector3(size , size , size) );		//6
	m_pManualObject->colour(color);

	m_pManualObject->position( position + Ogre::Vector3(-size , size , size));		//7
	m_pManualObject->colour(color);

	//index
	m_pManualObject->triangle(0 + offset, 2 + offset, 1 + offset);
	m_pManualObject->triangle(0 + offset, 2+ offset, 3+ offset);
	m_pManualObject->triangle(3+ offset, 4+ offset, 0+ offset);
	m_pManualObject->triangle(3+ offset, 7+ offset, 4+ offset);
	m_pManualObject->triangle(4+ offset, 7+ offset, 6+ offset);
	m_pManualObject->triangle(4+ offset, 6+ offset, 5+ offset);
	m_pManualObject->triangle(5+ offset, 2+ offset, 1+ offset);
	m_pManualObject->triangle(5+ offset, 6+ offset, 2+ offset);
	m_pManualObject->triangle(0+ offset, 4+ offset, 1+ offset);
	m_pManualObject->triangle(5+ offset, 1+ offset, 4+ offset);
	m_pManualObject->triangle(3+ offset, 6+ offset, 7+ offset);
	m_pManualObject->triangle(3+ offset, 2+ offset, 6+ offset);
}

void DrawObject::AddDynamicPoint(const Ogre::Vector3& p,float size /* = 0.01f */,Ogre::ColourValue color,bool bNeedUpdate)
{

}

void DrawObject::AddDynamicPoint(const GFPhysSoftBodyNode* pNode,float size ,Ogre::ColourValue color)
{
	if(pNode)
		m_dynamicPoints.insert(Point(pNode,size,color));
}

void DrawObject::RemoveDynamicPoint(GFPhysSoftBodyNode* pNode)
{
	for(std::set<Point>::iterator itr = m_dynamicPoints.begin(); itr != m_dynamicPoints.end();++itr)
	{
		if((*itr).m_pPhysNode == pNode)
		{
			m_dynamicPoints.erase(itr);
			break;
		}
	}
}
	
//=========================================== Line ===============================================//
void DrawObject::DrawLine(const Ogre::Vector3& p1,const Ogre::Vector3& p2,const Ogre::ColourValue& color)
{
	m_vecLines.push_back(Line(p1,p2,color));
	UpdateStaticLines();
}

void DrawObject::DrawLine(const GFPhysVector3& p1,const GFPhysVector3& p2,const Ogre::ColourValue& color)
{
	DrawLine(GPVec3ToOgre(p1),GPVec3ToOgre(p2),color);
}

void DrawObject::_DrawLine(const Line& line)
{
	const Ogre::Vector3 & p1 = line.m_point1;
	const Ogre::Vector3 & p2 = line.m_point2;
	const Ogre::ColourValue & color = line.m_color;

	m_pManualObject->position(p1);
	m_pManualObject->colour(color);

	m_pManualObject->position(p2);
	m_pManualObject->colour(color);
}

void DrawObject::AddDynamicLine(const Ogre::Vector3& p1,const Ogre::Vector3& p2,Ogre::ColourValue color,bool needUpdate)
{
	m_vecDynamicLines.push_back(Line(p1,p2,color));
	if(needUpdate)
		UpdateDynamicLines();
}

//=========================================== AxisAlignedBox ===============================================//
	
void DrawObject::DrawAxisAlignedBox(const Ogre::AxisAlignedBox boundingBox,Ogre::ColourValue color)
{
	m_vecBoundingBox.push_back(BoundingBox(boundingBox,color));
	UpdateStaticBoundingBox();
}

void DrawObject::OnRemoveNodeToSB(GFPhysSoftBodyNode * node , GFPhysSoftBodyShape * hostshape)
{
	RemoveDynamicPoint(node);
}

void DrawObject::OnRemoveEdgeToSB(GFPhysSoftBodyEdge * edge , GFPhysSoftBodyShape * hostshape)
{

}

void DrawObject::OnRemoveFaceToSB(GFPhysSoftBodyFace * face , GFPhysSoftBodyShape * hostshape)
{
	for(int n = 0;n < 3;++n)
		RemoveDynamicPoint(face->m_Nodes[n]);
}

void DrawObject::OnRemoveTetraToSB(GFPhysSoftBodyTetrahedron * tetra , GFPhysSoftBodyShape * hostshape)
{

}