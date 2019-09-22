#pragma once
#include "MisNewTraining.h"
#include <vector>
#include "CustomConstraint.h"
#include "Dynamic\DynamicWorld\GoPhysDiscreteDynamicsWorld.h"
//#include "Math\GoPhysVector3.h"

using namespace GoPhys;

class GoPhys::GFPhysVector3;
class DrawObject;


class MisNewTrainingDebugObject : public OgreWidgetEventListener
{
public:
	enum KeyboardModifier {
        NoModifier           = 0x00000000,
        ShiftModifier        = 0x02000000,
        ControlModifier      = 0x04000000,
        AltModifier          = 0x08000000,
        MetaModifier         = 0x10000000,
        KeypadModifier       = 0x20000000,
        GroupSwitchModifier  = 0x40000000,
        // Do not extend the mask to include 0x01000000
        KeyboardModifierMask = 0xfe000000
    };

    //shorter names for shortcuts
    enum Modifier {
        META          = MetaModifier,
        SHIFT         = ShiftModifier,
        CTRL          = ControlModifier,
        ALT           = AltModifier,
        MODIFIER_MASK = KeyboardModifierMask,
        UNICODE_ACCEL = 0x00000000
    };

    enum MouseButton {
        NoButton         = 0x00000000,
        LeftButton       = 0x00000001,
        RightButton      = 0x00000002,
        MidButton        = 0x00000004,
        MiddleButton     = MidButton,
        XButton1         = 0x00000008,
        XButton2         = 0x00000010,
        MouseButtonMask  = 0x000000ff
    };

	static MisNewTrainingDebugObject* GetInstance();

	void Destroy();
	
	void SetTraining(MisNewTraining* pNewTraining);

	DrawObject* GetDrawObject();

	/** 注意用于DrawObject对象的更新 */
	void Update(float dt);

private:
	MisNewTrainingDebugObject();
	~MisNewTrainingDebugObject();

	void NormalizeScreenCoordinate(Ogre::Vector2& coordinate,int x,int y);

	inline OgreWidget* GetOgreWidget() {return MXOgreWrapper::Instance()->GetOgreWidgetByName(RENDER_WINDOW_LARGE);}
	
protected:
	virtual void OnMouseReleased(char button , int x , int y);
	virtual void OnMousePressed(char button , int x , int y);
	virtual void OnMouseMoved(char button , int x , int y);
	virtual void OnWheelEvent(int delta);

private:
	static MisNewTrainingDebugObject m_debugObject;
	MisNewTraining * m_pTraining;
	DrawObject * m_pDrawObject;
};

//graphic

//该类负责绘制一些图元以便调试所用，如：点、线、面等
class DrawObject : public GFPhysDiscreteDynamicsWorld::GFPhysCustomWorldActionListener
{
public:

	DrawObject();

	~DrawObject();

	//define Point
	struct Point
	{
		Point(const Ogre::Vector3& p,const float size,const Ogre::ColourValue & color)
			:m_point(p),
			m_size(size),
			m_color(color),
			m_pPhysNode(NULL)
		{

		}

		Point(const GFPhysSoftBodyNode* pNode,float size,const Ogre::ColourValue& color)
			:m_point(0,0,0),
			m_pPhysNode(pNode),
			m_size(size),
			m_color(color)
		{

		}

		bool operator<(const Point& other) const
		{
			return m_pPhysNode < other.m_pPhysNode;
		}


		Ogre::Vector3 m_point;
		float m_size;
		Ogre::ColourValue m_color;
		/// for dynamic point
		const GFPhysSoftBodyNode * m_pPhysNode;
	};


	//绘制一个点
	//p：点的坐标
	//size:点的大小
	//color:点的颜色,默认绿色
	void DrawPoint(const Ogre::Vector3& p,float size = 0.01f,const Ogre::ColourValue& color = Ogre::ColourValue::Blue);

	void DrawPoint(const GFPhysVector3& pos,float size = 0.01,const Ogre::ColourValue& color = Ogre::ColourValue::Blue);

	struct BoundingBox
	{
		BoundingBox(const Ogre::AxisAlignedBox & boundingBox,Ogre::ColourValue color = Ogre::ColourValue(0.f,1.f,0.f))
			:m_boundingBox(boundingBox),
			m_color(color)
		{

		}

		Ogre::AxisAlignedBox m_boundingBox;
		Ogre::ColourValue m_color;
	};
	void AddDynamicPoint(const Ogre::Vector3& p,float size = 0.01f,Ogre::ColourValue color = Ogre::ColourValue(0.f,1.f,0.f),bool bNeedUpdate = false);

	void AddDynamicPoint(const GFPhysSoftBodyNode* pNode,float size = 0.01f,Ogre::ColourValue color = Ogre::ColourValue(0.f,1.f,0.f));

	void RemoveDynamicPoint(GFPhysSoftBodyNode* pNode);

	//define line
	struct Line
	{
		Line(const Ogre::Vector3 & p1,const Ogre::Vector3 & p2,const Ogre::ColourValue & color)
			: m_point1(p1),
			m_point2(p2),
			m_color(color)
		{

		}
		Ogre::Vector3 m_point1;
		Ogre::Vector3 m_point2;
		Ogre::ColourValue m_color;
	};

	//绘制一条线
	//p1：端点1
	//p2：端点2
	//color : 线段颜色
	void DrawLine(const Ogre::Vector3& p1,const Ogre::Vector3& p2,const Ogre::ColourValue& color = Ogre::ColourValue::Green);

	void DrawLine(const GFPhysVector3& p1,const GFPhysVector3& p2,const Ogre::ColourValue& color = Ogre::ColourValue::Green);

	//绘制一个包围盒
	void DrawAxisAlignedBox(const Ogre::AxisAlignedBox boundingBox,Ogre::ColourValue color = Ogre::ColourValue(0.f,1.f,0.f));

	void AddDynamicLine(const Ogre::Vector3& p1,const Ogre::Vector3& p2,Ogre::ColourValue color = Ogre::ColourValue(0.f,1.f,0.f),bool needUpdate = false);

	//清除动态更新的缓存 clear all object
	void Clear();

	//clear static
	void ClearStaticPoints();
	void ClearStaticLines();
	//clear dynamic
	void ClearDynamicPoints();
	void ClearDynamicLines();


	//update all
	void Update();
	//update dynamic points
	void UpdateDynamicPoints();
	//update dynamic Lines;
	void UpdateDynamicLines();

private:
	void InitSection();
	void UpdateStaticPoints();
	void UpdateStaticLines();
	void UpdateStaticBoundingBox();

	void _DrawPoint(const Point& point,std::size_t pointIndex);

	void _DrawLine(const Line& line);

protected:
	virtual void PerformCustomCollision(GFPhysDiscreteDynamicsWorld * dynworld) {};
	virtual void GetTempPosConstraints(GFPhysVectorObj<GFPhysPositionConstraint*>){};

	virtual void OnAddTetraToSB(GFPhysSoftBodyTetrahedron * tetra , GFPhysSoftBodyShape * hostshape) {};
	virtual void OnAddEdgeToSB(GFPhysSoftBodyEdge * edge , GFPhysSoftBodyShape * hostshape) {};
	virtual void OnAddFaceToSB(GFPhysSoftBodyFace * face , GFPhysSoftBodyShape * hostshape) {};
	virtual void OnAddNodeToSB(GFPhysSoftBodyNode * node , GFPhysSoftBodyShape * hostshape) {};

	virtual void OnRemoveTetraToSB(GFPhysSoftBodyTetrahedron * tetra , GFPhysSoftBodyShape * hostshape);
	virtual void OnRemoveEdgeToSB(GFPhysSoftBodyEdge * edge , GFPhysSoftBodyShape * hostshape);
	virtual void OnRemoveFaceToSB(GFPhysSoftBodyFace * face , GFPhysSoftBodyShape * hostshape);
	virtual void OnRemoveNodeToSB(GFPhysSoftBodyNode * node , GFPhysSoftBodyShape * hostshape);

private:
	Ogre::SceneManager * m_pSceneManager;
	Ogre::ManualObject * m_pManualObject;

	//static
	std::vector<Point> m_vecPoints;	
	std::vector<Line> m_vecLines;
	std::vector<BoundingBox> m_vecBoundingBox;

	//dynamic
	std::set<Point> m_dynamicPoints;
	std::vector<Line> m_vecDynamicLines;
};