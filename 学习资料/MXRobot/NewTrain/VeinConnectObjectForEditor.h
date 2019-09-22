#pragma once
#include <Ogre.h>
#include "collision/GoPhysCollisionlib.h"
#include "dynamic/PhysicBody/GoPhysSoftTube.h"
#include "MisMedicOrganOrdinary.h"
//#include "MisMedicOrganTube.h"
#include "VeinConnectRender.h"
#include "math/GoPhysTransformUtil.h"
//#include "OrganBloodMotionSimulator.h"
#define CONTROL_POINTS_NUM 4
#define MAX_S  50
#define MAX_T  50

struct PointDist
{
	PointDist(int index , float dist) : m_Index(index) , m_Dist(dist) {}
	int m_Index;
	float m_Dist;
};


struct SurfaceEdge
{
	SurfaceEdge(int i , int j) : m_IIndex(i) ,m_JIndex(j){}
	int m_IIndex;
	int m_JIndex;
};


//=============================================================================================

struct ObjFormatModel
{
	std::vector<Ogre::Vector3> Vertices;
	std::vector<Ogre::Vector2> TexCoords;
	std::vector<int> VertexIndices;
	std::vector<int> TexCoordIndices;
};

enum CameraVector
{
	CV_NONE,
	CV_RIGHT,
	CV_UP,
	CV_DIR,
};

struct SurfacePointIndex
{
	SurfacePointIndex(int s = -1 , int t = -1) : m_SIndex(s) , m_TIndex(t) {}
	int m_SIndex;
	int m_TIndex;
};


struct SurfaceControlPointIndex
{
	SurfaceControlPointIndex(int i = -1 , int j = -1) : m_IIndex(i) , m_JIndex(j) {}
	int m_IIndex;
	int m_JIndex;
};

struct AttachRecordForEditor
{
	int m_SIndex;
	int m_TIndex;
	int m_RealIndex;
	bool m_IsVaild;
	MisMedicOrgan_Ordinary *m_pOrgan;
	int m_OrganId;
	GFPhysSoftBodyFace *m_pFace;
	int m_FaceId;
	Ogre::Vector3 m_closestPoint;
	Ogre::Vector3 m_originalPosition;
	float m_AttatchWeight[3];
};

struct BezierSurfaceQuadForEditor
{
	BezierSurfaceQuadForEditor() : m_Vaild(true) , m_UpperQuadIndex(-1) , m_RightQuadIndex(-1) {} 
	int m_SIndexOfVertices[4];
	int m_TIndexOfVertices[4];
	int m_RealIndexOfVertices[4];
	bool m_Vaild;
	int m_QuadIndex;
	int m_UpperQuadIndex;
	int m_RightQuadIndex;

	int m_UnderQuadIndex;
	int m_LeftQuadIndex;
};

struct TetrahedronForEditor
{
	int m_SIndex[4];
	int m_TIndex[4];
	int m_RealIndex[4];
};

struct PairDivided
{
	std::vector<int> m_OneSideIndices;
	std::vector<int> m_OtherSideIndices;
};

class BicubicBezierSurface
{
public:
	struct SurfacePoint
	{
		SurfacePoint() : m_Color(Ogre::ColourValue::White) , m_AttatchRecordID(-1) , m_IsUsed(false) {}
		int m_SIndex;
		int m_TIndex;
		int m_RealIndex;
		Ogre::Vector3 m_Position;
		Ogre::ColourValue m_Color;
		Ogre::ColourValue m_SpecialColor; //used for identify the direction of the s and t    //Q(s,t)
		Ogre::Vector2 m_UvFactor;
		Ogre::Vector2 m_UV;
		Ogre::Vector2 m_AlphaUV;
		//
		bool m_IsUsed;
		int m_AttatchRecordID;
	};

	BicubicBezierSurface();

	~BicubicBezierSurface();

	void Initialize(Ogre::Vector3 points_s1[] , 
							Ogre::Vector3 points_s2[] ,
							Ogre::Vector3 points_s3[] ,
							Ogre::Vector3 points_s4[] , 
							int nS = 10,
							int nT = 10,
							int inum = 4 ,
							int jnum = 4);

	void SetPointNumOfS(int n);

	void SetPointNumOfT(int n);

	int GetPointNumOfS() { return m_NumOfS;}

	int GetPointNumOfT() {return m_NumOfT;}

	void GetConnerUV(Ogre::Vector2 & leftbottom , Ogre::Vector2 & lefttop , Ogre::Vector2 &righttop , Ogre::Vector2 & rightbottom);

	void SetPoint(int i , int j , const Ogre::Vector3 & point);

	void ChangeSelectedControlPointsPos(const Ogre::Vector3 &delta);

	void ChangeSelectedSubdividedPointsPos(const Ogre::Vector3 & delta);

	void Translate(const Ogre::Vector3 & translation);
	
	//u -- the axis
	void Rotate(const Ogre::Vector3 & origin , const Ogre::Vector3 & u , const double theta);

	void Scale(double factor);

	void ToggleVisibleMode() { bool visible = m_pManual->getVisible();  m_pManual->setVisible(!visible); }  

	void ToggleWireFrameMode() { m_IsWireFrame = !m_IsWireFrame; }

	void ToggleConvexHullMode() { m_IsShowConvexHull = !m_IsShowConvexHull; }

	void ToggleShowPairMode() { m_IsShowPair = !m_IsShowPair; }

	void AutoAttachPointToOrgan(float detectionRadius , std::vector<MisMedicOrganInterface*> & organInterfaces);

	void AttachSelectedPointsToOrgan(float detectionRadius , std::vector<MisMedicOrganInterface*> & organInterfaces);

	void SetSelectedQuadInvalid();
	
	void PickQuad(const Ogre::Ray & ray);
	
	bool PickControlPoint(const Ogre::Ray & ray , bool isMultiSelect);

	bool PickPoint(const Ogre::Ray & ray , bool isMultiSelect );

	void Update(float dt , Ogre::Camera *pCam);

	void UpdateBTN();

	void CreateQuadInfo();

	void CreateTetraInfo(); //unused

	//0 --  s direction  1 -- t direction
	void CreatePairs(int dir);

	void ExportTriFace(std::vector<int> & faceIndices , int offset = 0);

	void ExportEdgeInfo(std::vector<SurfaceEdge> & edges , int offset = 0);

	void ReComputeSurfacePoints();

	void Reset() { ReComputeSurfacePoints();}

	void ResetSelectedVertices();

	void SaveToFile(std::ofstream & of);

	void SaveEditableFormatToFile(std::ofstream & of);

	Ogre::Vector3 m_Points[CONTROL_POINTS_NUM][CONTROL_POINTS_NUM];

	SurfacePoint m_PointsDivided[MAX_S][MAX_T];

	//the center of  all control points
	Ogre::Vector3 m_CenterPos;

	//the center of the selected control points
	Ogre::Vector3 m_SelectedControlCenterPos;

	//the center of the selected points
	Ogre::Vector3 m_SelectedPointsCenterPos;

	std::vector<AttachRecordForEditor> m_AttachRecords;	

	std::vector<int> m_SelectedQuadIndices;

	std::vector<int> m_InValidQuadIndices;

	std::vector<SurfaceControlPointIndex> m_SelectedControlPointIndices;

	std::vector<SurfacePointIndex> m_SelectedPointIndices;

	std::vector<BezierSurfaceQuadForEditor> m_QuadInfos;

	std::vector<PairDivided> m_PairsDivided;

	int m_DirOfPairDivided;
	
private:
// 	SurfacePoint * m_pPointsDivided;
// 	float *m_Bs[CONTROL_POINTS_NUM];
// 	float *m_Bt[CONTROL_POINTS_NUM];

	float m_Bs[CONTROL_POINTS_NUM][MAX_S];
	float m_Bt[CONTROL_POINTS_NUM][MAX_T];

	void ComputerBezierCoefficientsBs(int pointNum);

	void ComputerBezierCoefficientsBt(int pointNum);

	void ComputeUVFactor();

	void ComputeUV();

	void AttachOnePointToOrgan(int sIndex , int tIndex , float detectionRadius , std::vector<MisMedicOrganInterface*> & organInterfaces);

	void DrawOnePoint(const Ogre::Vector3 & position ,const Ogre::ColourValue & color , float size ,int &offset);

//	Ogre::Vector3 EvaluateCubicBezierCurvePointAt(Ogre::Vector3 points[] ,const float t);

	std::vector<TetrahedronForEditor> m_TetrasInfos;
	
	Ogre::ManualObject *m_pManual;

	bool m_IsWireFrame;

	bool m_IsShowConvexHull;

	bool m_IsShowPair;

	int m_NumOfS;
	int m_NumOfT;
};

class VeinConnectObjForEditor
{
public:
	struct SelectedSurfaceInfo
	{
		SelectedSurfaceInfo() : m_IndexOfI( -1 ) , m_IndexOfJ( -1 ) {} 
		Ogre::Vector3 m_Points[4][4];
		Ogre::Vector3 m_CurrentSurfaceCenter;
		int m_IndexOfI;
		int m_IndexOfJ;
	};

	enum EditMode
	{
		MODE_POINT,
		MODE_WHOLE , 
		MODE_ROTATE_WHOLE,
		MODE_SCALE,
		MODE_QUAD,
		MODE_DIVIEDED_POINT,
	};

	enum EditState
	{
		STATE_NONE , 
		STATE_POINT_SELECTED , 
		STATE_MOVING_POINT  ,
		STATE_ROTATING_POINT , 
	};

	VeinConnectObjForEditor(Ogre::Camera * cam);

	~VeinConnectObjForEditor();

	void AddNewBicubicBezierSurface(Ogre::Vector3 points_i1[] , 
															Ogre::Vector3 points_i2[] ,
															Ogre::Vector3 points_i3[] ,
															Ogre::Vector3 points_i4[]);

	void RemoveSurface(int index);

	void SetSelectedSurface(int index);

	void Update(float dt);

	void SetEditing(bool isediting) { m_IsBeingEdited = isediting ;}

	void SetEditingPoint() { m_EditMode = MODE_POINT; m_EditState = STATE_NONE;}

	void SetEditingSubdividedPoint() { m_EditMode = MODE_DIVIEDED_POINT; m_EditState = STATE_NONE;}

	void SetEditingWhole() { m_EditMode = MODE_WHOLE; m_EditState = STATE_POINT_SELECTED; }

	void SetRotatingWhole() { m_EditMode = MODE_ROTATE_WHOLE; m_EditState = STATE_POINT_SELECTED; }

	void SetScalingWhole() { m_EditMode = MODE_SCALE; m_EditState = STATE_POINT_SELECTED; }

	void SetPickingQuad() { m_EditMode = MODE_QUAD; m_EditState = STATE_NONE; }

	void SetDetectRadius(float radius) {m_DetectRadius = radius;}

	void OnMouseLeftPressed(float tx , float ty , bool isMultiSelect = false);

	void OnMouseRightPressed(float tx , float ty);

	void OnMouseMoved(char button ,float tx , float ty);

	void OnMouseReleased(char button  ,float tx , float ty);

	void OnWheelEvent(int delta);

	//select a control point
	bool PickControlPoint(Ogre::Camera * camera,float mousex,float mousey , bool isMultiSelect);

	bool PickSubDividedPoint(Ogre::Camera * camera,float mousex,float mousey , bool isMultiSelect);

	CameraVector PickAxis(Ogre::Camera *camera, float mousex, float mousey,float &ratio);

	void ChangePointPosition(float mousex ,float mousey);

	void RotateControlPoint(float mousex ,float mousey);

	void SetSurfaceSelectedQuadInvalid();

	void TextureMapping();

	//change the positon  of a control point
	void ChangeControlPoint(const Ogre::Vector3 & direction , float value);

	void SetSurfaceS(int n);

	void SetSurfaceT(int n);

	void ToggleCurrentSurfaceVisible();

	void ToggleCurentSurfaceWireFrameMode();

	void ToggleCurrentSurfaceConvexHullMode();

	void ToggleTransformationMode();	//Ðý×ª

	//mode 0 -- detect all; 1 -- just detect the selected vertices 
	void AutoAttachSurfacePointToOrgan(float detectionRadius , std::vector<MisMedicOrganInterface*> & organInterfaces , int mode);

	void ToggleCurrentSurfacePairVisible();

	void CreatePairsOfCurrentSurface(int dir);

	void ResetSurface();

	int GetSurfaceNum() { return m_Surfaces.size(); }

	bool SaveVeinObjToFile(const std::string & fileName);

	bool SaveVeinObjForEditorToFile(const std::string & fileName);

	bool ExportToObjFile(const std::string& fileName);

	bool GetTexCoordFromObjFile(const std::string &fileName);

	bool InitVeinObjFromFile(std::string &filename);

private:
	void DrawOnePoint(const Ogre::Vector3 & position ,const Ogre::ColourValue & color , float size ,int &offset);

	void DrawCoord(const Ogre::Vector3& origin,const Ogre::Vector3& U,const Ogre::Vector3& V,const Ogre::Vector3& W);

	void UpdateInfo();

	float CalDirLength(const Ogre::Vector3 & origin);

	Ogre::Camera * m_pCamera;
	
	std::vector<BicubicBezierSurface *> m_Surfaces;
	
	int m_SurfaceSelectedIndex;

	SelectedSurfaceInfo m_SurfaceInfo;

	Ogre::ManualObject *m_pManual;

	bool m_IsBeingEdited;
	
	EditMode m_EditMode;

	EditState m_EditState;

	CameraVector m_SelectedAxis;

	float m_DetectRadius;

	float m_LastMx;
	float m_LastMy;

};
