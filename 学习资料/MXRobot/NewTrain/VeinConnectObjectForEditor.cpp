#include "stdafx.h"
//#include "VeinConnectObjectV2.h"
#include "VeinConnectObjectForEditor.h"
#include "PhysicsWrapper.h"
//#include "CustomConstraint.h"
//#include "CustomCollision.h"
#include "MisMedicOrganOrdinary.h"
//#include "MisMedicOrganTube.h"
#include "ITraining.h"
//#include "TextureBloodEffect.h"
#include "MXOgreWrapper.h"
#include "MXOgreGraphic.h"
#include "math/GoPhysTransformUtil.h"

#define EDOT_GALLBLADDER 3
#define EDOT_LIVER 5

bool ReadFromObjFile(const std::string & fileName , ObjFormatModel & objModel);

std::ifstream & operator>>(std::ifstream & fin , BicubicBezierSurface::SurfacePoint & point)
{
	double x  = 0 ,  y = 0 , z = 0 ;
	double r = 0 , g = 0 , b = 0 , a = 0;
	double u = 0 , v = 0 ;
	double alphau = 0 , alphav = 0 ;
	fin >> x >> y >> z;
	fin >> r >> g >> b >> a;
	fin >> u >> v;
	fin >> alphau >> alphav;
	point.m_Position = Ogre::Vector3(x,y,z);
	point.m_Color = Ogre::ColourValue(r,g,b,a);
	point.m_UV = Ogre::Vector2(u , v);
	point.m_AlphaUV = Ogre::Vector2(alphau , alphav);
	return fin;
}


std::ifstream & operator>>(std::ifstream & fin , AttachRecordForEditor &record)
{
	double ox, oy , oz;
	double cx, cy , cz;
	fin >> record.m_SIndex >> record.m_TIndex;
	fin >> record.m_RealIndex;
	fin >> record.m_OrganId >> record.m_FaceId;
	fin >> cx >> cy >> cz;
	fin >> ox >> oy >> oz;
	fin >> record.m_AttatchWeight[0] >> record.m_AttatchWeight[1] >> record.m_AttatchWeight[2] ;
	
	record.m_closestPoint = Ogre::Vector3(cx,cy,cz);
	record.m_originalPosition = Ogre::Vector3(ox,oy,oz);
	return fin;
}

BicubicBezierSurface::BicubicBezierSurface()
	:	m_NumOfS(10),
		m_NumOfT(10),
		m_IsWireFrame(false),
		m_IsShowConvexHull(false),
		m_pManual(NULL),
		m_IsShowPair(false)
{
	m_DirOfPairDivided = 0;
}

BicubicBezierSurface::~BicubicBezierSurface()
{
	if(m_pManual)
	{
		m_pManual->detachFromParent();
		MXOgreWrapper::Get()->GetDefaultSceneManger()->destroyManualObject(m_pManual);
		m_pManual = NULL;
	}
}
void BicubicBezierSurface::Initialize(Ogre::Vector3 points_i1[] , Ogre::Vector3 points_i2[] , 
															Ogre::Vector3 points_i3[] , Ogre::Vector3 points_i4[] , 
															int nS /* = 10 */, int nT /* = 10 */, 
															int inum /* = 4  */, int jnum /* = 4 */)
{
	m_pManual = MXOgreWrapper::Get()->GetDefaultSceneManger()->createManualObject();
	MXOgreWrapper::Get()->GetDefaultSceneManger()->getRootSceneNode()->attachObject(m_pManual);
	m_pManual->setDynamic(true);

	//todo
	std::copy(points_i1 , points_i1 + inum , m_Points[0]);
	std::copy(points_i2 , points_i2 + inum , m_Points[1]);
	std::copy(points_i3 , points_i3 + inum , m_Points[2]);
	std::copy(points_i4 , points_i4 + inum , m_Points[3]);

	m_NumOfS = nS;
	m_NumOfT = nT;

	m_PointsDivided[0][0].m_UV = Ogre::Vector2(0,0);
	m_PointsDivided[0][m_NumOfT -1].m_UV = Ogre::Vector2(0,1);
	m_PointsDivided[m_NumOfS - 1][m_NumOfT -1].m_UV = Ogre::Vector2(1,1);
	m_PointsDivided[m_NumOfS -1][0].m_UV = Ogre::Vector2(1,0);

	ComputerBezierCoefficientsBs(m_NumOfS);
	ComputerBezierCoefficientsBt(m_NumOfT);

	CreateQuadInfo();
	//CreateTetraInfo();

	ReComputeSurfacePoints();
}


// Ogre::Vector3 BicubicBezierSurface::EvaluateCubicBezierCurvePointAt(Ogre::Vector3 points[] ,const float t)
// {
// 	float one_minus_t = 1 - t;
// 	float one_minus_t_2 = one_minus_t * one_minus_t;
// 	float one_minus_t_3 = one_minus_t * one_minus_t_2;
// 
// 	float B0 = one_minus_t_3;
// 	float B1 = 3 * one_minus_t_2 * t;
// 	float B2 = 3 * one_minus_t  * t * t;
// 	float B3 = t * t * t;
// 
// 	Ogre::Vector3 result = B0 * points[0] + B1 * points[1] + B2 * points[2] + B3 * points[3]; 
// 	return result;
// }

void BicubicBezierSurface::SetPointNumOfS(int n)
{
// 	if(n != m_NumOfS)
// 	{
// 		m_NumOfS = n;
// 		for(int i = 0 ; i < CONTROL_POINTS_NUM ; i++)
// 		{
// 			if(m_Bs[i] != NULL)
// 			{
// 				delete[] m_Bs[i];
// 				m_Bs[i] = new float[n];
// 			}
// 		}
// 		ComputerBezierCoefficients(m_Bs  , n);
// 		ReAllocAndReComputeSurfacePoints();
// 	}
	if(n > MAX_S)
		n = MAX_S;
	if(n != m_NumOfS)
	{	
		m_NumOfS = n;
		ComputerBezierCoefficientsBs(n);
		ReComputeSurfacePoints();

		CreateQuadInfo();
		//CreateTetraInfo();
	}
}

void BicubicBezierSurface::SetPointNumOfT(int n)
{
// 	if(n != m_NumOfT)
// 	{
// 		m_NumOfT = n;
// 		for(int i = 0 ; i < CONTROL_POINTS_NUM ; i++)
// 		{
// 			if(m_Bt[i] != NULL)
// 			{
// 				delete[] m_Bt[i];
// 				m_Bt[i] = new float[n];
// 			}
// 		}
// 		ComputerBezierCoefficients(m_Bt , n);
// 		ReAllocAndReComputeSurfacePoints();
// 	}
	if(n > MAX_T)
		n = MAX_T;
	if(n != m_NumOfT)
	{	
		m_NumOfT = n;
		ComputerBezierCoefficientsBt(n);
		ReComputeSurfacePoints();

		CreateQuadInfo();
		//CreateTetraInfo();
	}
}

// void BicubicBezierSurface::ReAllocAndReComputeSurfacePoints()
// {
// 	if(m_Points)
//  		delete[] m_Points;
// 
// 	
// }

void BicubicBezierSurface::GetConnerUV(Ogre::Vector2 & leftbottom , Ogre::Vector2 & lefttop , Ogre::Vector2 &righttop , Ogre::Vector2 & rightbottom)
{
	leftbottom = m_PointsDivided[0][0].m_UV;
	lefttop = m_PointsDivided[0][m_NumOfT - 1].m_UV;
	righttop = m_PointsDivided[m_NumOfS - 1][0].m_UV;
	rightbottom = m_PointsDivided[m_NumOfS - 1][m_NumOfT -1].m_UV;
}

void BicubicBezierSurface::SetPoint(int i , int j , const Ogre::Vector3 & point)
{
	m_Points[i][j] = point;
	ReComputeSurfacePoints();
}

void BicubicBezierSurface::ChangeSelectedControlPointsPos(const Ogre::Vector3 &delta)
{
	for(int i = 0 ; i < m_SelectedControlPointIndices.size() ; i++)
	{
		SurfaceControlPointIndex & index = m_SelectedControlPointIndices[i];
		m_Points[index.m_IIndex][index.m_JIndex] += delta;
	}
	ReComputeSurfacePoints();
}

void BicubicBezierSurface::ChangeSelectedSubdividedPointsPos(const Ogre::Vector3 & delta)
{
	for(int i = 0 ; i < m_SelectedPointIndices.size() ; i++)
	{
		SurfacePointIndex & index = m_SelectedPointIndices[i];
		m_PointsDivided[index.m_SIndex][index.m_TIndex].m_Position += delta;
	}
}

void BicubicBezierSurface::Translate(const Ogre::Vector3 & translation)
{
	for(int i = 0 ; i < CONTROL_POINTS_NUM ; i++ )
	{
		for(int j = 0 ; j < CONTROL_POINTS_NUM ; j++)
		{
			m_Points[i][j] += translation;
		}
	}
	ReComputeSurfacePoints();
}

void BicubicBezierSurface::Rotate(const Ogre::Vector3 & origin , const Ogre::Vector3 & u , const double theta)
{
	double cosTheta = cos(theta);
	double sinTheta = sin(theta);
	double one_minus_cosTheta = 1 - cosTheta;

	for(int i = 0 ; i < CONTROL_POINTS_NUM ; i++)
	{
		for(int j = 0 ; j < CONTROL_POINTS_NUM ; j++)
		{
			Ogre::Vector3 v = m_Points[i][j] - origin;
			m_Points[i][j] = origin + cosTheta * v + one_minus_cosTheta * u.dotProduct(v) * u + sinTheta * (u.crossProduct(v));
		}
	}

	ReComputeSurfacePoints();
}

void BicubicBezierSurface::Scale(double factor)
{
	for(int i = 0 ; i < CONTROL_POINTS_NUM ; i++)
	{
		for(int j = 0 ; j < CONTROL_POINTS_NUM ; j++)
		{
			Ogre::Vector3 & point = m_Points[i][j];
			Ogre::Vector3 dir = point - m_CenterPos;
			point = dir * factor + m_CenterPos;
		}
	}
	ReComputeSurfacePoints();
}

void BicubicBezierSurface::AutoAttachPointToOrgan(float detectionRadius , std::vector<MisMedicOrganInterface*> & organInterfaces)
{
	for(int s = 0 ; s < m_NumOfS ; s++)
	{
		for(int t = 0 ; t < m_NumOfT ; t++)
		{
			AttachOnePointToOrgan(s , t , detectionRadius , organInterfaces);
		}
	}
}

void BicubicBezierSurface::AttachSelectedPointsToOrgan(float detectionRadius , std::vector<MisMedicOrganInterface*> & organInterfaces)
{
	for(int p = 0 ; p < m_SelectedPointIndices.size() ; p++)
	{
		SurfacePointIndex &index = m_SelectedPointIndices[p];
		AttachOnePointToOrgan(index.m_SIndex , index.m_TIndex , detectionRadius , organInterfaces);
	}
}

void BicubicBezierSurface::SetSelectedQuadInvalid()
{
	for(int q = 0 ; q < m_SelectedQuadIndices.size() ; q++)
	{
		m_QuadInfos[m_SelectedQuadIndices[q]].m_Vaild = false;
		m_InValidQuadIndices.push_back(m_SelectedQuadIndices[q]);
	}
	m_SelectedQuadIndices.clear();
}

void BicubicBezierSurface::PickQuad(const Ogre::Ray & ray)
{
	float closest_distance = -1.0f;
	int quad_index = -1;

	for(int q = 0 ; q < m_QuadInfos.size() ; q++)
	{
		BezierSurfaceQuadForEditor &quad = m_QuadInfos[q];

		if(!quad.m_Vaild)
			continue;
		
		std::pair<bool, Ogre::Real> hit1 = Ogre::Math::intersects(ray, m_PointsDivided[quad.m_SIndexOfVertices[0]][quad.m_TIndexOfVertices[0]].m_Position , 
																												m_PointsDivided[quad.m_SIndexOfVertices[1]][quad.m_TIndexOfVertices[1]].m_Position , 
																												m_PointsDivided[quad.m_SIndexOfVertices[2]][quad.m_TIndexOfVertices[2]].m_Position , 
																												true ,false);
		
		std::pair<bool, Ogre::Real> hit2 = Ogre::Math::intersects(ray, m_PointsDivided[quad.m_SIndexOfVertices[0]][quad.m_TIndexOfVertices[0]].m_Position , 
																												m_PointsDivided[quad.m_SIndexOfVertices[2]][quad.m_TIndexOfVertices[2]].m_Position , 
																												m_PointsDivided[quad.m_SIndexOfVertices[3]][quad.m_TIndexOfVertices[3]].m_Position , 
																												true ,false);

		if(hit1.first)
		{
			if ((closest_distance < 0.0f) || (hit1.second < closest_distance))
			{
				closest_distance = hit1.second;
				quad_index = q;
			}
		}
		if(hit2.first)
		{
			if ((closest_distance < 0.0f) || (hit2.second < closest_distance))
			{
				closest_distance = hit2.second;
				quad_index = q;
			}
		}
	}
	if(quad_index >= 0)
	{
		bool selected =false;
		std::vector<int>::iterator it = m_SelectedQuadIndices.begin();
		while(it != m_SelectedQuadIndices.end())
		{
			if(*it == quad_index)
			{
				selected = true;
				m_SelectedQuadIndices.erase(it);
				break;
			}
			it++;
		}
		if(!selected)
			m_SelectedQuadIndices.push_back(quad_index);
	}
}

bool BicubicBezierSurface::PickControlPoint(const Ogre::Ray & ray ,bool isMultiSelect)
{
	if(!isMultiSelect)
		m_SelectedControlPointIndices.clear();

	const Ogre::Vector3 & origin = ray.getOrigin();
	const Ogre::Vector3 & dir = ray.getDirection();

	int i_index = -1;
	int j_index = -1;

	const float selected_distance_square = 0.05f * 0.05f;
	float min_distance_square = FLT_MAX;

	for(int i  = 0 ; i < CONTROL_POINTS_NUM ; i++)
	{
		for(int j = 0 ; j < CONTROL_POINTS_NUM ; j++)
		{
			const Ogre::Vector3 & point = m_Points[i][j];
			Ogre::Vector3 o_to_p = point - origin ;
			Ogre::Vector3 projection = o_to_p.dotProduct(dir) * dir;
			Ogre::Vector3 temp = o_to_p - projection;
			double length_square = temp.squaredLength();

			if(length_square < selected_distance_square && length_square < min_distance_square)
			{
				min_distance_square = length_square;
				i_index = i;
				j_index = j;
			}
		}
	}

	if(i_index != - 1 && j_index != -1)
	{
		bool selected =false;
		std::vector<SurfaceControlPointIndex>::iterator it = m_SelectedControlPointIndices.begin();
		while(it != m_SelectedControlPointIndices.end())
		{
			if((it->m_IIndex == i_index) && (it->m_JIndex == j_index))
			{
				selected = true;
				m_SelectedControlPointIndices.erase(it);
				break;
			}
			it++;
		}
		if(!selected)
			m_SelectedControlPointIndices.push_back(SurfaceControlPointIndex(i_index,j_index));
	}
	
	m_SelectedControlCenterPos = Ogre::Vector3::ZERO;
	for(int p = 0 ; p < m_SelectedControlPointIndices.size() ; p++)
	{
		m_SelectedControlCenterPos += m_Points[m_SelectedControlPointIndices[p].m_IIndex][m_SelectedControlPointIndices[p].m_JIndex];
	}
	

	if(m_SelectedControlPointIndices.size() != 0)
	{
		m_SelectedControlCenterPos /= m_SelectedControlPointIndices.size();
		return true;
	}
	else
		return false;
}

bool BicubicBezierSurface::PickPoint(const Ogre::Ray & ray , bool isMultiSelect)
{
	if(!isMultiSelect)
		m_SelectedPointIndices.clear();

	const Ogre::Vector3 & origin = ray.getOrigin();
	const Ogre::Vector3 & dir = ray.getDirection();

	int s_index = -1;
	int t_index = -1;

	const float selected_distance_square = 0.05f * 0.05f;
	float min_distance_square = FLT_MAX;

	for(int s  = 0 ; s < m_NumOfS ; s++)
	{
		for(int t = 0 ; t < m_NumOfT ; t++)
		{
			const Ogre::Vector3 & point = m_PointsDivided[s][t].m_Position;
			Ogre::Vector3 o_to_p = point - origin ;
			Ogre::Vector3 projection = o_to_p.dotProduct(dir) * dir;
			Ogre::Vector3 temp = o_to_p - projection;
			double length_square = temp.squaredLength();

			if(length_square < selected_distance_square && length_square < min_distance_square)
			{
				min_distance_square = length_square;
				s_index = s;
				t_index = t;
			}
		}
	}

	if(s_index != - 1 && t_index != -1)
	{
		bool selected =false;
		std::vector<SurfacePointIndex>::iterator it = m_SelectedPointIndices.begin();
		while(it != m_SelectedPointIndices.end())
		{
			if((it->m_SIndex == s_index) && (it->m_TIndex == t_index))
			{
				selected = true;
				m_SelectedPointIndices.erase(it);
				break;
			}
			it++;
		}
		if(!selected)
			m_SelectedPointIndices.push_back(SurfacePointIndex(s_index,t_index));
	}

	m_SelectedPointsCenterPos = Ogre::Vector3::ZERO;
	for(int p = 0 ; p < m_SelectedPointIndices.size() ; p++)
	{
		m_SelectedPointsCenterPos += m_PointsDivided[m_SelectedPointIndices[p].m_SIndex][m_SelectedPointIndices[p].m_TIndex].m_Position;
	}
	
	if(m_SelectedPointIndices.size() != 0)
	{
		m_SelectedPointsCenterPos /= m_SelectedPointIndices.size();
		return true;
	}
	else
		return false;
}

void BicubicBezierSurface::Update(float dt , Ogre::Camera *pCam)
{
	m_pManual->clear();
	if(m_IsWireFrame)
		m_pManual->begin("VeinEditor/BezierWireFrame");
	else 
		m_pManual->begin("VeinEditor/BezierSurface");

	for(int t = 0 ; t < m_NumOfT ; t++)
	{
		for(int s = 0 ; s < m_NumOfS ; s++)
		{
			SurfacePoint &point = m_PointsDivided[s][t];
			m_pManual->position(point.m_Position);
			if(m_IsWireFrame)
				m_pManual->colour(point.m_SpecialColor);
			else
				m_pManual->colour(point.m_Color);
			m_pManual->textureCoord(point.m_UV);
		}
	}

	for(int f = 0 ; f < m_QuadInfos.size() ; f++)
	{
		BezierSurfaceQuadForEditor & quad = m_QuadInfos[f];
		if(quad.m_Vaild)
		{
			m_pManual->quad(quad.m_RealIndexOfVertices[0] , 
				quad.m_RealIndexOfVertices[1] ,
				quad.m_RealIndexOfVertices[2] ,
				quad.m_RealIndexOfVertices[3]);
		}
	}
	m_pManual->end();

	//--temp
// 	m_pManual->begin("VeinEditor/BezierWireFrame");
// 	
// 	Ogre::Real SCALE = 0.5;
// 	Ogre::Real near_clip = pCam->getNearClipDistance();
// 	Ogre::Vector3 origin = pCam->getRealDirection() * (near_clip + 2)+ pCam->getRealPosition();
// 	Ogre::Vector3 right = pCam->getRealRight() * SCALE;
// 	Ogre::Vector3  up = pCam->getRealUp() * SCALE;
// 
// 
// 	for(int t = 0 ; t < m_NumOfT ; t++)
// 	{
// 		for(int s = 0 ; s < m_NumOfS ; s++)
// 		{
// 			SurfacePoint &point = m_PointsDivided[s][t];
// 			m_pManual->position(origin + right * point.m_UV.x + up * point.m_UV.y);
// 			m_pManual->colour(point.m_SpecialColor);
// 		}
// 	}
// 
// 	for(int f = 0 ; f < m_QuadInfos.size() ; f++)
// 	{
// 		BezierSurfaceQuadForEditor & quad = m_QuadInfos[f];
// 		if(quad.m_Vaild)
// 		{
// 			m_pManual->quad(quad.m_RealIndexOfVertices[0] , 
// 				quad.m_RealIndexOfVertices[1] ,
// 				quad.m_RealIndexOfVertices[2] ,
// 				quad.m_RealIndexOfVertices[3]);
// 		}
// 	}
// 	m_pManual->end();
	//--temp

	//draw the seleted face
	if(m_SelectedQuadIndices.size() != 0)
	{
		m_pManual->begin("BaseWhiteNoLighting" , Ogre::RenderOperation::OT_LINE_LIST);

		for(int s = 0 ; s < m_SelectedQuadIndices.size() ; s++)
		{
			BezierSurfaceQuadForEditor & quad = m_QuadInfos[m_SelectedQuadIndices[s]];

			m_pManual->position(m_PointsDivided[quad.m_SIndexOfVertices[0]][quad.m_TIndexOfVertices[0]].m_Position);
			m_pManual->colour(Ogre::ColourValue::Black);
			m_pManual->position(m_PointsDivided[quad.m_SIndexOfVertices[1]][quad.m_TIndexOfVertices[1]].m_Position);
			m_pManual->colour(Ogre::ColourValue::Black);

			m_pManual->position(m_PointsDivided[quad.m_SIndexOfVertices[1]][quad.m_TIndexOfVertices[1]].m_Position);
			m_pManual->colour(Ogre::ColourValue::Black);
			m_pManual->position(m_PointsDivided[quad.m_SIndexOfVertices[2]][quad.m_TIndexOfVertices[2]].m_Position);
			m_pManual->colour(Ogre::ColourValue::Black);

			m_pManual->position(m_PointsDivided[quad.m_SIndexOfVertices[2]][quad.m_TIndexOfVertices[2]].m_Position);
			m_pManual->colour(Ogre::ColourValue::Black);
			m_pManual->position(m_PointsDivided[quad.m_SIndexOfVertices[3]][quad.m_TIndexOfVertices[3]].m_Position);
			m_pManual->colour(Ogre::ColourValue::Black);

			m_pManual->position(m_PointsDivided[quad.m_SIndexOfVertices[3]][quad.m_TIndexOfVertices[3]].m_Position);
			m_pManual->colour(Ogre::ColourValue::Black);
			m_pManual->position(m_PointsDivided[quad.m_SIndexOfVertices[0]][quad.m_TIndexOfVertices[0]].m_Position);
			m_pManual->colour(Ogre::ColourValue::Black);

		}
		m_pManual->end();
	}


	if(m_IsShowConvexHull)
	{
		m_pManual->begin("VeinEditor/BezierWireFrame");

		for(int t = 0 ; t < CONTROL_POINTS_NUM ; t++)
		{
			for(int s = 0 ; s < CONTROL_POINTS_NUM ; s++)
			{
				m_pManual->position(m_Points[s][t]);
				m_pManual->colour(Ogre::ColourValue::Blue);
			}
		}

		for(int t = 0 ; t < CONTROL_POINTS_NUM - 1 ; t++)
		{
			for(int s = 0 ; s < CONTROL_POINTS_NUM - 1 ; s++)
			{
				int index = t * CONTROL_POINTS_NUM + s ;
				m_pManual->quad(index , index + 1 , index + CONTROL_POINTS_NUM + 1 , index + CONTROL_POINTS_NUM);
			}
		}
		m_pManual->end();
	}

	if(m_AttachRecords.size() != 0 )
	{
		int offset = 0;
		m_pManual->begin("VeinEditor/Cube");
		for(int s = 0 ; s < m_NumOfS ; s++)
		{
			for(int t = 0 ; t < m_NumOfT ; t++)
			{
				SurfacePoint &point = m_PointsDivided[s][t];
				if(point.m_AttatchRecordID >= 0)
					DrawOnePoint(point.m_Position , Ogre::ColourValue(1 ,1 , 0 , 1) , 0.02 , offset);
			}
		}
		m_pManual->end();
	}
	if(m_IsShowPair)
	{
		if(m_PairsDivided.size() == 0)
			CreatePairs(m_DirOfPairDivided);
		
		m_pManual->begin("BaseWhiteNoLighting" , Ogre::RenderOperation::OT_LINE_LIST);

		for(size_t pairIndex = 0 ; pairIndex < m_PairsDivided.size() ; pairIndex++)
		{
			PairDivided & pair = m_PairsDivided[pairIndex];

			std::vector<Ogre::Vector3> vertices;
			for(size_t i = 0 ; i < pair.m_OneSideIndices.size() ; i++)
			{
				Ogre::Vector3 & p1 = m_PointsDivided[pair.m_OneSideIndices[i] % m_NumOfS][pair.m_OneSideIndices[i] / m_NumOfS].m_Position;
				Ogre::Vector3 & p2 = m_PointsDivided[pair.m_OtherSideIndices[i] % m_NumOfS][pair.m_OtherSideIndices[i] / m_NumOfS].m_Position;
				vertices.push_back((p1 + p2) * 0.5 + pCam->getDirection() * 0.01);
			}

			for(int p = 0 ; p < vertices.size() - 1 ; p++)
			{
				m_pManual->position(vertices[p]);
				m_pManual->colour(Ogre::ColourValue::Blue);
				m_pManual->position(vertices[p + 1]);
				m_pManual->colour(Ogre::ColourValue::Blue);
			}
		}
		m_pManual->end();
	}
}

void BicubicBezierSurface::UpdateBTN()
{
	
}

void BicubicBezierSurface::CreateQuadInfo()
{
	m_QuadInfos.clear();
	//int index = 0;
	int s_quad_num = m_NumOfS - 1;
	int t_quad_num = m_NumOfT - 1;
	for(int t = 0 ; t < t_quad_num ; t++)
	{
		for(int s = 0 ; s < s_quad_num ; s++)
		{
			BezierSurfaceQuadForEditor quad;
			
			quad.m_SIndexOfVertices[0] = s;
			quad.m_SIndexOfVertices[1] = s + 1;
			quad.m_SIndexOfVertices[2] = s + 1;
			quad.m_SIndexOfVertices[3] = s ;

			quad.m_TIndexOfVertices[0] = t;
			quad.m_TIndexOfVertices[1] = t;
			quad.m_TIndexOfVertices[2] = t + 1;
			quad.m_TIndexOfVertices[3] = t + 1 ;

			quad.m_RealIndexOfVertices[0] = s + t * m_NumOfS;
			quad.m_RealIndexOfVertices[1] = quad.m_RealIndexOfVertices[0] + 1;
			quad.m_RealIndexOfVertices[2] = quad.m_RealIndexOfVertices[1] + m_NumOfS;
			quad.m_RealIndexOfVertices[3] = quad.m_RealIndexOfVertices[2] - 1;

			quad.m_QuadIndex = s + t * s_quad_num;

			if(s == (s_quad_num - 1)) {
				quad.m_RightQuadIndex = -1;
			} else {
				quad.m_RightQuadIndex = quad.m_QuadIndex + 1;
			}

			if(t == (t_quad_num - 1)) {
				quad.m_UpperQuadIndex = -1;
			} else {
				quad.m_UpperQuadIndex = quad.m_QuadIndex + s_quad_num;
			}

			if(s == 0) {
				quad.m_LeftQuadIndex = -1;
			} else {
				quad.m_LeftQuadIndex = quad.m_QuadIndex - 1;
			}

			if(t == 0) {
				quad.m_UnderQuadIndex = -1;
			} else {
				quad.m_UnderQuadIndex = quad.m_QuadIndex - s_quad_num;
			}

			m_QuadInfos.push_back(quad);
		}
	}
}

void BicubicBezierSurface::CreateTetraInfo()
{
	m_TetrasInfos.clear();
	for(int q = 0 ; q < m_QuadInfos.size() ; q++)
	{
		BezierSurfaceQuadForEditor & quad = m_QuadInfos[q];
		if(quad.m_Vaild)
		{
			TetrahedronForEditor tetra;
			std::copy(quad.m_SIndexOfVertices , quad.m_SIndexOfVertices + 4 , tetra.m_SIndex);
			std::copy(quad.m_TIndexOfVertices , quad.m_TIndexOfVertices + 4 , tetra.m_TIndex);
			std::copy(quad.m_RealIndexOfVertices , quad.m_RealIndexOfVertices + 4 , tetra.m_RealIndex);
			m_TetrasInfos.push_back(tetra);
		}
		
		if(quad.m_RightQuadIndex > 0)
		{
			BezierSurfaceQuadForEditor & right_quad = m_QuadInfos[quad.m_RightQuadIndex];
			if(right_quad.m_Vaild)
			{
				TetrahedronForEditor tetra;
				tetra.m_SIndex[0] = quad.m_SIndexOfVertices[0];
				tetra.m_TIndex[0] = quad.m_TIndexOfVertices[0];
				tetra.m_RealIndex[0] = quad.m_RealIndexOfVertices[0];

				tetra.m_SIndex[1] = quad.m_SIndexOfVertices[1];
				tetra.m_TIndex[1] = quad.m_TIndexOfVertices[1];
				tetra.m_RealIndex[1] = quad.m_RealIndexOfVertices[1];

				tetra.m_SIndex[2] = right_quad.m_SIndexOfVertices[2];
				tetra.m_TIndex[2] = right_quad.m_TIndexOfVertices[2];
				tetra.m_RealIndex[2] = right_quad.m_RealIndexOfVertices[2];

				tetra.m_SIndex[3] = right_quad.m_SIndexOfVertices[3];
				tetra.m_TIndex[3] = right_quad.m_TIndexOfVertices[3];
				tetra.m_RealIndex[3] = right_quad.m_RealIndexOfVertices[3];

				m_TetrasInfos.push_back(tetra);
			}
		}

		if(quad.m_UpperQuadIndex > 0)
		{
			BezierSurfaceQuadForEditor & upper_quad = m_QuadInfos[quad.m_UpperQuadIndex];
			if(upper_quad.m_Vaild)
			{
				TetrahedronForEditor tetra;
				tetra.m_SIndex[0] = quad.m_SIndexOfVertices[0];
				tetra.m_TIndex[0] = quad.m_TIndexOfVertices[0];
				tetra.m_RealIndex[0] = quad.m_RealIndexOfVertices[0];

				tetra.m_SIndex[1] = quad.m_SIndexOfVertices[2];
				tetra.m_TIndex[1] = quad.m_TIndexOfVertices[2];
				tetra.m_RealIndex[1] = quad.m_RealIndexOfVertices[2];

				tetra.m_SIndex[2] = upper_quad.m_SIndexOfVertices[2];
				tetra.m_TIndex[2] = upper_quad.m_TIndexOfVertices[2];
				tetra.m_RealIndex[2] = upper_quad.m_RealIndexOfVertices[2];

				tetra.m_SIndex[3] = upper_quad.m_SIndexOfVertices[0];
				tetra.m_TIndex[3] = upper_quad.m_TIndexOfVertices[0];
				tetra.m_RealIndex[3] = upper_quad.m_RealIndexOfVertices[0];

				m_TetrasInfos.push_back(tetra);
			}
		}

		if(quad.m_LeftQuadIndex > 0)
		{
			BezierSurfaceQuadForEditor & left_quad = m_QuadInfos[quad.m_LeftQuadIndex];
			if(left_quad.m_Vaild)
			{
				TetrahedronForEditor tetra;
				tetra.m_SIndex[0] = quad.m_SIndexOfVertices[0];
				tetra.m_TIndex[0] = quad.m_TIndexOfVertices[0];
				tetra.m_RealIndex[0] = quad.m_RealIndexOfVertices[0];

				tetra.m_SIndex[1] = quad.m_SIndexOfVertices[1];
				tetra.m_TIndex[1] = quad.m_TIndexOfVertices[1];
				tetra.m_RealIndex[1] = quad.m_RealIndexOfVertices[1];

				tetra.m_SIndex[2] = left_quad.m_SIndexOfVertices[2];
				tetra.m_TIndex[2] = left_quad.m_TIndexOfVertices[2];
				tetra.m_RealIndex[2] = left_quad.m_RealIndexOfVertices[2];

				tetra.m_SIndex[3] = left_quad.m_SIndexOfVertices[3];
				tetra.m_TIndex[3] = left_quad.m_TIndexOfVertices[3];
				tetra.m_RealIndex[3] = left_quad.m_RealIndexOfVertices[3];

				m_TetrasInfos.push_back(tetra);
			}
		}

		if(quad.m_UnderQuadIndex > 0)
		{
			BezierSurfaceQuadForEditor & under_quad = m_QuadInfos[quad.m_UnderQuadIndex];
			if(under_quad.m_Vaild)
			{
				TetrahedronForEditor tetra;
				tetra.m_SIndex[0] = quad.m_SIndexOfVertices[3];
				tetra.m_TIndex[0] = quad.m_TIndexOfVertices[3];
				tetra.m_RealIndex[0] = quad.m_RealIndexOfVertices[3];

				tetra.m_SIndex[1] = quad.m_SIndexOfVertices[0];
				tetra.m_TIndex[1] = quad.m_TIndexOfVertices[0];
				tetra.m_RealIndex[1] = quad.m_RealIndexOfVertices[0];

				tetra.m_SIndex[2] = under_quad.m_SIndexOfVertices[1];
				tetra.m_TIndex[2] = under_quad.m_TIndexOfVertices[1];
				tetra.m_RealIndex[2] = under_quad.m_RealIndexOfVertices[1];

				tetra.m_SIndex[3] = under_quad.m_SIndexOfVertices[2];
				tetra.m_TIndex[3] = under_quad.m_TIndexOfVertices[2];
				tetra.m_RealIndex[3] = under_quad.m_RealIndexOfVertices[2];

				m_TetrasInfos.push_back(tetra);
			}
		}
	}
}

void BicubicBezierSurface::CreatePairs(int dir)
{
	m_PairsDivided.clear();
	//0 --  s direction  1 -- t direction
	if(dir == 0 )
	{
		for(int t = 0 ; t < m_NumOfT - 1 ; t++)
		{
			PairDivided pair;
			for(int s = 0 ; s <  m_NumOfS ; s++)
			{
				pair.m_OneSideIndices.push_back(m_PointsDivided[s][t].m_RealIndex);
				pair.m_OtherSideIndices.push_back(m_PointsDivided[s][t+1].m_RealIndex);
			}
			m_PairsDivided.push_back(pair);
		}
	}
	else if(dir == 1)
	{
		for(int s = 0 ; s < m_NumOfS - 1 ; s++)
		{
			PairDivided pair;
			for(int t = 0 ; t <  m_NumOfT ; t++)
			{
				pair.m_OneSideIndices.push_back(m_PointsDivided[s][t].m_RealIndex);
				pair.m_OtherSideIndices.push_back(m_PointsDivided[s+1][t].m_RealIndex);
			}
			m_PairsDivided.push_back(pair);
		}
	}
}

void BicubicBezierSurface::ExportTriFace(std::vector<int> & faceIndices , int offset /* = 0 */)
{
	for(int q = 0 ; q < m_QuadInfos.size() ; q++)
	{
		BezierSurfaceQuadForEditor &quad = m_QuadInfos[q];
		if(quad.m_Vaild)
		{
			faceIndices.push_back(quad.m_RealIndexOfVertices[0] + offset);
			faceIndices.push_back(quad.m_RealIndexOfVertices[2] + offset);
			faceIndices.push_back(quad.m_RealIndexOfVertices[3] + offset);

			faceIndices.push_back(quad.m_RealIndexOfVertices[0] + offset);
			faceIndices.push_back(quad.m_RealIndexOfVertices[1] + offset);
			faceIndices.push_back(quad.m_RealIndexOfVertices[2] + offset);
		}
	}
}

void BicubicBezierSurface::ExportEdgeInfo(std::vector<SurfaceEdge> & edges , int offset)
{
	//edges.clear();

	for(int t = 0 ; t < m_NumOfT -1 ; t++)
	{
		for (int s = 0 ; s < m_NumOfS -1 ; s++)
		{
			int index = t * m_NumOfS + s + offset;
			edges.push_back(SurfaceEdge(index , index+1));
			edges.push_back(SurfaceEdge(index , index + m_NumOfS ));
			edges.push_back(SurfaceEdge(index , index + m_NumOfS +1));
		}
	}
}

void BicubicBezierSurface::ResetSelectedVertices()
{
	for(int v = 0 ; v < m_SelectedPointIndices.size() ;  v++)
	{
		SurfacePointIndex &index = m_SelectedPointIndices[v];
		SurfacePoint &vertex =  m_PointsDivided[index.m_SIndex][index.m_TIndex];
		if(vertex.m_AttatchRecordID >= 0)
		{
				m_AttachRecords[vertex.m_AttatchRecordID].m_IsVaild = false;
				vertex.m_Position = m_AttachRecords[vertex.m_AttatchRecordID].m_originalPosition;
		}
	}
}

void BicubicBezierSurface::SaveToFile(std::ofstream & of)
{

	//of << m_NumOfS << " " << m_NumOfT << std::endl;
	of << "vertex position color uv alphauv" << std::endl;		//add
	of << m_NumOfS * m_NumOfT << std::endl;		//add

	for(int t = 0 ; t < m_NumOfT ; t++)
	{
		for(int s = 0 ; s < m_NumOfS ; s++)
		{
			const SurfacePoint & surface_point = m_PointsDivided[s][t];
			//vertex position
			of << surface_point.m_Position.x << " " << surface_point.m_Position.y << " " << surface_point.m_Position.z << std::endl;
			//vertex color
			of << surface_point.m_Color.r << " " 
				<< surface_point.m_Color.g  << " " 
				<< surface_point.m_Color.b  << " " 
				<< surface_point.m_Color.a  << std::endl;
			//vertex uv
			of << surface_point.m_UV.x << " "	
				<< surface_point.m_UV.y  <<  std::endl;
			//vertex alpha uv
			of << surface_point.m_AlphaUV.x << " " 
				<< surface_point.m_AlphaUV.y << std::endl;
		}
	}
	//face indices
	std::vector<int> faceindices;
	ExportTriFace(faceindices);
	int facenum = faceindices.size() / 3;
	of << "face" << std::endl;
	of << facenum<< std::endl;
	for(int findex = 0 ; findex < faceindices.size() ; findex+= 3)
	{
		of << faceindices[findex] << " " << faceindices[findex + 1] << " " << faceindices[findex + 2] << std::endl;
	}
	//vertex indices in pairs
	CreatePairs(m_DirOfPairDivided);
	int pairnum = m_PairsDivided.size();
	of << "pair" << std::endl;
	of << pairnum<< std::endl;
	for(int pairindex = 0 ; pairindex < pairnum ; pairindex++)
	{
		PairDivided & pair = m_PairsDivided[pairindex];
		int indexnum = pair.m_OneSideIndices.size();

		of << indexnum<< std::endl;

		for(int i = 0 ; i < indexnum ; i++) {
			of << pair.m_OneSideIndices[i] << " ";
		}
		of << std::endl;
		for(int i = 0 ; i < indexnum ; i++) {
			of << pair.m_OtherSideIndices[i] << " ";
		}
		of << std::endl;
	}
	//attach record
	/*
	vertex index
	organid
	faceid
	*/
	int valid_record_num = 0;
	for(int r = 0 ; r < m_AttachRecords.size() ; r++)
	{
		if(m_AttachRecords[r].m_IsVaild)
			valid_record_num++;
	}
	of << "attachpoint num: " <<  valid_record_num << std::endl;
	for(int r = 0 ; r < m_AttachRecords.size() ; r++)
	{
		AttachRecordForEditor & record = m_AttachRecords[r];
		if(record.m_IsVaild)
		{
			of << record.m_RealIndex << std::endl;//of << record.m_SIndex << " " << record.m_TIndex << std::endl;
			of << record.m_OrganId << " " << record.m_FaceId << std::endl;
			of << record.m_AttatchWeight[0] << " " << record.m_AttatchWeight[1] << " " << record.m_AttatchWeight[2] << std::endl;
		}
	}
}

void BicubicBezierSurface::SaveEditableFormatToFile(std::ofstream & of)
{
	//Divide Direction Of Pair 
	of << "Divide Direction Of Pair ";
	of << m_DirOfPairDivided << std::endl;		
	//control points
	of << "control points" << std::endl;		
	of << CONTROL_POINTS_NUM << " " << CONTROL_POINTS_NUM << std::endl;
	for(int i = 0 ; i < CONTROL_POINTS_NUM ; i++)
	{
		for(int j = 0 ; j < CONTROL_POINTS_NUM ; j++)
		{
			of << m_Points[i][j].x << " " << m_Points[i][j].y << " " << m_Points[i][j].z << std::endl;
		}
	}
	//vertices
	of << "vertex position color uv alphauv" << std::endl;		
	of << m_NumOfS << " " <<  m_NumOfT << std::endl;		

	for(int t = 0 ; t < m_NumOfT ; t++)
	{
		for(int s = 0 ; s < m_NumOfS ; s++)
		{
			const SurfacePoint & surface_point = m_PointsDivided[s][t];
			//vertex position
			of << surface_point.m_Position.x << " " << surface_point.m_Position.y << " " << surface_point.m_Position.z << std::endl;
			//vertex color
			of << surface_point.m_Color.r << " " 
				<< surface_point.m_Color.g  << " " 
				<< surface_point.m_Color.b  << " " 
				<< surface_point.m_Color.a  << std::endl;
			//vertex uv
			of << surface_point.m_UV.x << " "	
				<< surface_point.m_UV.y  <<  std::endl;
			//vertex alpha uv
			of << surface_point.m_AlphaUV.x << " " 
				<< surface_point.m_AlphaUV.y << std::endl;
		}
	}

	//invalid quad info
	of << "invalid quad info" << std::endl;
	of << m_InValidQuadIndices.size() << std::endl;
	for(int q = 0 ; q < m_InValidQuadIndices.size() ; q++)
	{
		of << m_InValidQuadIndices[q] << std::endl;
	}
	//attach record
	/*
	vertex index
	organid
	faceid
	*/
	int valid_record_num = 0;
	for(int r = 0 ; r < m_AttachRecords.size() ; r++)
	{
		if(m_AttachRecords[r].m_IsVaild)
			valid_record_num++;
	}
	of << "attachpoint num: " <<  valid_record_num << std::endl;
	for(int r = 0 ; r < m_AttachRecords.size() ; r++)
	{
		AttachRecordForEditor & record = m_AttachRecords[r];
		if(record.m_IsVaild)
		{
			of << record.m_SIndex << " " << record.m_TIndex << std::endl;
			of << record.m_RealIndex << std::endl;
			of << record.m_OrganId << " " << record.m_FaceId << std::endl;
			of << record.m_closestPoint.x << " " << record.m_closestPoint.y << " " << record.m_closestPoint.z << std::endl;
			of << record.m_originalPosition.x << " " << record.m_originalPosition.y << " " << record.m_originalPosition.z << std::endl;
			of << record.m_AttatchWeight[0] << " " << record.m_AttatchWeight[1] << " " << record.m_AttatchWeight[2] << std::endl;
		}
	}
}

void BicubicBezierSurface::ComputerBezierCoefficientsBs(int pointNum)
{

	float step = 1.0f / (pointNum - 1);
	float t = 0;
	for(int i = 0 ; i  < pointNum - 1 ; i++)
	{
		float one_minus_t = 1 - t;
		float one_minus_t_2 = one_minus_t * one_minus_t;
		float one_minus_t_3 = one_minus_t * one_minus_t_2;

		m_Bs[0][i] = one_minus_t_3;
		m_Bs[1][i] = 3 * one_minus_t_2 * t;
		m_Bs[2][i] = 3 * one_minus_t  * t * t;
		m_Bs[3][i] = t * t * t;

		t+= step;
	}

	m_Bs[0][pointNum-1] = 0;
	m_Bs[1][pointNum-1] = 0;
	m_Bs[2][pointNum-1] = 0;
	m_Bs[3][pointNum-1] = 1;

}

void BicubicBezierSurface::ComputerBezierCoefficientsBt(int pointNum)
{

	float step = 1.0f / (pointNum - 1);
	float t = 0;
	for(int i = 0 ; i  < pointNum - 1 ; i++)
	{
		float one_minus_t = 1 - t;
		float one_minus_t_2 = one_minus_t * one_minus_t;
		float one_minus_t_3 = one_minus_t * one_minus_t_2;

		m_Bt[0][i] = one_minus_t_3;
		m_Bt[1][i] = 3 * one_minus_t_2 * t;
		m_Bt[2][i] = 3 * one_minus_t  * t * t;
		m_Bt[3][i] = t * t * t;

		t+= step;
	}

	m_Bt[0][pointNum-1] = 0;
	m_Bt[1][pointNum-1] = 0;
	m_Bt[2][pointNum-1] = 0;
	m_Bt[3][pointNum-1] = 1;

}

void BicubicBezierSurface::ComputeUVFactor()
{
	//computer the u factor
	for(int t = 0 ; t < m_NumOfT ;  t++)
	{
		Ogre::Real total_ulength = 0;
		for(int s = 0 ; s < m_NumOfS - 1 ; s++)
		{
			SurfacePoint & point = m_PointsDivided[s][t];
			SurfacePoint & next_point = m_PointsDivided[s + 1][t];
			point.m_UvFactor.x = total_ulength;
			total_ulength += (next_point.m_Position - point.m_Position).length();
		}
		m_PointsDivided[m_NumOfS - 1][t].m_UvFactor.x = total_ulength;


		for(int s = 0 ; s < m_NumOfS; s++)
		{
			SurfacePoint & point = m_PointsDivided[s][t];
			point.m_UvFactor.x /= total_ulength;
		}
	}

	//computer the v factor
	for(int s = 0 ; s < m_NumOfS ;  s++)
	{
		Ogre::Real total_vlength = 0;
		for(int t = 0 ; t < m_NumOfT - 1 ; t++)
		{
			SurfacePoint & point = m_PointsDivided[s][t];
			SurfacePoint & next_point = m_PointsDivided[s][t + 1];
			point.m_UvFactor.y = total_vlength;
			total_vlength += (next_point.m_Position - point.m_Position).length();
		}
		m_PointsDivided[s][m_NumOfT - 1].m_UvFactor.y = total_vlength;


		for(int t = 0 ; t < m_NumOfT; t++)
		{
			SurfacePoint & point = m_PointsDivided[s][t];
			point.m_UvFactor.y /= total_vlength;
		}
	}
}

void BicubicBezierSurface::ComputeUV()
{
	ComputeUVFactor();

	m_PointsDivided[0][0].m_UV = Ogre::Vector2(0,0);
	m_PointsDivided[0][m_NumOfT -1].m_UV = Ogre::Vector2(0,1);
	m_PointsDivided[m_NumOfS - 1][0].m_UV = Ogre::Vector2(1,0);
	m_PointsDivided[m_NumOfS -1][m_NumOfT -1].m_UV = Ogre::Vector2(1,1);
	
	Ogre::Vector2 left_bottom = m_PointsDivided[0][0].m_UV;
	Ogre::Vector2 left_top = m_PointsDivided[0][m_NumOfT -1].m_UV;
	Ogre::Vector2 right_bottom = m_PointsDivided[m_NumOfS - 1][0].m_UV;
	Ogre::Vector2 right_top = m_PointsDivided[m_NumOfS -1][m_NumOfT -1].m_UV;

	for(int s = 0 ; s < m_NumOfS ; s++)
	{
		for(int t = 0 ; t < m_NumOfT ; t++)
		{
			SurfacePoint & point = m_PointsDivided[s][t];
			const Ogre::Vector2 factor = point.m_UvFactor;
			point.m_UV = ((1.0f - factor.x) * left_bottom + factor.x * right_bottom) * (1 - factor.y) +
									 ((1.0f - factor.x) * left_top + factor.x * right_top) * factor.y;

			point.m_AlphaUV = point.m_UV;
		}
	}
}

void BicubicBezierSurface::AttachOnePointToOrgan(int sIndex , int tIndex , float detectionRadius , std::vector<MisMedicOrganInterface*> & organInterfaces)
{
	SurfacePoint &surfacePoint = m_PointsDivided[sIndex][tIndex];
	GFPhysVector3 position = OgreToGPVec3(surfacePoint.m_Position);

	float current_length = detectionRadius;
	bool isAttached = false;

	AttachRecordForEditor attachRecord;  
	attachRecord.m_IsVaild = true;

	for(size_t c = 0 ; c < organInterfaces.size() ; c++)
	{
		MisMedicOrgan_Ordinary * organ = dynamic_cast<MisMedicOrgan_Ordinary*>(organInterfaces[c]);
		if(organ)
		{
			float closestDist;
			GFPhysSoftBodyFace * closestFace = NULL;
			GFPhysVector3 closestDistPoint;
			ClosetFaceToPoint(organ, position, closestDist, closestFace, closestDistPoint);

			if(closestDist <= current_length)
			{
				isAttached = true;
				current_length = closestDist;

				attachRecord.m_pOrgan = organ;
				attachRecord.m_OrganId = organ->m_OrganID;
				attachRecord.m_pFace = closestFace;
				attachRecord.m_closestPoint = GPVec3ToOgre(closestDistPoint);
			}
		}
	}

	if(isAttached == true)
	{
		int faceId ,materialId;
		attachRecord.m_pOrgan->ExtractFaceIdAndMaterialIdFromUsrData(attachRecord.m_pFace , materialId , faceId);
		attachRecord.m_FaceId = faceId;

		CalcBaryCentric(attachRecord.m_pFace->m_Nodes[0]->m_UnDeformedPos,
			attachRecord.m_pFace->m_Nodes[1]->m_UnDeformedPos,
			attachRecord.m_pFace->m_Nodes[2]->m_UnDeformedPos,
			OgreToGPVec3(attachRecord.m_closestPoint),
			attachRecord.m_AttatchWeight[0],
			attachRecord.m_AttatchWeight[1],
			attachRecord.m_AttatchWeight[2]);

		attachRecord.m_SIndex = surfacePoint.m_SIndex;
		attachRecord.m_TIndex = surfacePoint.m_TIndex;
		attachRecord.m_RealIndex = attachRecord.m_TIndex * m_NumOfS + attachRecord.m_SIndex;

		attachRecord.m_originalPosition = surfacePoint.m_Position;
		//surfacePoint.m_Position = attachRecord.m_closestPoint;

		if(surfacePoint.m_AttatchRecordID >= 0)
		{
			m_AttachRecords[surfacePoint.m_AttatchRecordID] = attachRecord;
		}
		else
		{
			surfacePoint.m_AttatchRecordID = m_AttachRecords.size();
			m_AttachRecords.push_back(attachRecord);
		}
	}
}

void BicubicBezierSurface::DrawOnePoint(const Ogre::Vector3 & position ,const Ogre::ColourValue & color , float size ,int &offset)
{
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

void BicubicBezierSurface::ReComputeSurfacePoints()
{
	Ogre::Real srange = m_NumOfS - 1;
	Ogre::Real trange = m_NumOfT - 1;

	m_CenterPos = Ogre::Vector3::ZERO;

	m_AttachRecords.clear();

	for(int s = 0 ; s < m_NumOfS ; s++)
	{
		for(int t = 0 ; t < m_NumOfT ; t++)
		{
			m_PointsDivided[s][t].m_Position = Ogre::Vector3::ZERO;
			m_PointsDivided[s][t].m_SIndex = s;
			m_PointsDivided[s][t].m_TIndex = t;
			m_PointsDivided[s][t].m_RealIndex =  t * m_NumOfS + s;
// 			Ogre::Real u  =  s / srange ;
// 			Ogre::Real v =   t / trange ;
// 			u = u > 1.0 ? 1.0 : u;
// 			v = v > 1.0 ? 1.0 :  v;
//			m_PointsDivided[s][t].m_SpecialColor = ( ( 1 - u ) * Ogre::ColourValue::Red + u * Ogre::ColourValue::Blue ) * (1 - v) + ( ( 1 - u ) * Ogre::ColourValue::Green + u * Ogre::ColourValue::White ) * v;
//			m_PointsDivided[s][t].m_UV = Ogre::Vector2(u , v);
			
			m_PointsDivided[s][t].m_AttatchRecordID = -1;


			for(int i = 0 ; i < CONTROL_POINTS_NUM ; i++)
				for(int j = 0 ; j < CONTROL_POINTS_NUM ; j++)
				{
					m_PointsDivided[s][t].m_Position  += m_Bs[i][s] * m_Bt[j][t] * m_Points[i][j];
				}

			//m_CenterPos += m_PointsDivided[s][t].m_Position;
		}
	}

	ComputeUV();
	for(int s = 0 ; s < m_NumOfS ; s++)
	{
		for(int t = 0 ; t < m_NumOfT ; t++)
		{
			Ogre::Real u = m_PointsDivided[s][t].m_UvFactor.x;
			Ogre::Real v = m_PointsDivided[s][t].m_UvFactor.y;
			m_PointsDivided[s][t].m_SpecialColor = ( ( 1 - u ) * Ogre::ColourValue::Red + u * Ogre::ColourValue::Blue ) * (1 - v) + ( ( 1 - u ) * Ogre::ColourValue::Green + u * Ogre::ColourValue::White ) * v;
		}
	}


		//m_CenterPos /= (m_NumOfS * m_NumOfT);
		for(int i = 0 ; i < CONTROL_POINTS_NUM ; i++)
		{
			for(int j = 0 ; j < CONTROL_POINTS_NUM ; j++)
			{
				m_CenterPos += m_Points[i][j];
			}
		}
		m_CenterPos /= (CONTROL_POINTS_NUM * CONTROL_POINTS_NUM);

		m_SelectedControlCenterPos = Ogre::Vector3::ZERO;
		for(int p = 0 ; p < m_SelectedControlPointIndices.size() ; p++)
		{
			m_SelectedControlCenterPos += m_Points[m_SelectedControlPointIndices[p].m_IIndex][m_SelectedControlPointIndices[p].m_JIndex];
		}
		if(m_SelectedControlPointIndices.size() != 0)
			m_SelectedControlCenterPos /= m_SelectedControlPointIndices.size();

		m_SelectedPointsCenterPos = Ogre::Vector3::ZERO;
		for(int p = 0 ; p < m_SelectedPointIndices.size() ; p++)
		{
			m_SelectedPointsCenterPos += m_Points[m_SelectedPointIndices[p].m_SIndex][m_SelectedPointIndices[p].m_TIndex];
		}
		if(m_SelectedPointIndices.size() != 0)
			m_SelectedPointsCenterPos  /= m_SelectedPointIndices.size();
}



VeinConnectObjForEditor::VeinConnectObjForEditor(Ogre::Camera * cam)
: m_SurfaceSelectedIndex(-1) ,
  m_pCamera(cam) , 
  m_IsBeingEdited(false) ,  
  m_EditMode(MODE_POINT) ,
  m_EditState(STATE_NONE),
  m_SelectedAxis(CV_NONE),
  m_DetectRadius(0.05f)
{
	m_pManual = MXOgreWrapper::Get()->GetDefaultSceneManger()->createManualObject();
	MXOgreWrapper::Get()->GetDefaultSceneManger()->getRootSceneNode()->attachObject(m_pManual);
	m_pManual->setDynamic(true);
}

VeinConnectObjForEditor::~VeinConnectObjForEditor()
{
	if(m_pManual)
	{
		m_pManual->detachFromParent();
		MXOgreWrapper::Get()->GetDefaultSceneManger()->destroyManualObject(m_pManual);
		m_pManual = NULL;
	}
}

void VeinConnectObjForEditor::AddNewBicubicBezierSurface(Ogre::Vector3 points_s1[] , 
																									Ogre::Vector3 points_s2[] , 
																									Ogre::Vector3 points_s3[] , 
																									Ogre::Vector3 points_s4[])
{
	BicubicBezierSurface *p_surface = new BicubicBezierSurface;
	p_surface->Initialize(points_s1, points_s2, points_s3, points_s4);
	m_Surfaces.push_back(p_surface);

}

void VeinConnectObjForEditor::RemoveSurface(int index)
{

}

void VeinConnectObjForEditor::SetSelectedSurface(int index)
{
	m_SurfaceSelectedIndex = index;
	if(index >= 0)
	{
		BicubicBezierSurface *p_surface = m_Surfaces[index];
// 		m_SurfaceInfo.m_CurrentSurfaceCenter = p_surface->m_CenterPos;
// 		for(int i = 0 ; i < 4 ;i++)
// 		{
// 			for(int j = 0 ; j < 4; j++)
// 			{
// 				m_SurfaceInfo.m_Points[i][j] = p_surface->m_Points[i][j];
// 			}
// 		}
// 		m_SurfaceInfo.m_IndexOfI = -1;
// 		m_SurfaceInfo.m_IndexOfJ = -1;
	}
}

void VeinConnectObjForEditor::Update(float dt)
{
	//todo draw the coord
	if(m_IsBeingEdited && m_SurfaceSelectedIndex >= 0)
	{
		BicubicBezierSurface *p_surface = m_Surfaces[m_SurfaceSelectedIndex];
		m_pManual->clear();
		m_pManual->begin("VeinEditor/Cube");
		int offset = 0;
		if(m_EditMode != MODE_DIVIEDED_POINT)
		{
			for(int i = 0 ; i < CONTROL_POINTS_NUM ; i++)
			{
				for(int j = 0 ; j < CONTROL_POINTS_NUM ; j++)
				{
					//DrawOnePoint(m_SurfaceInfo.m_Points[i][j] , Ogre::ColourValue::Green , 0.05 , offset);
					DrawOnePoint(p_surface->m_Points[i][j] , Ogre::ColourValue::Green , 0.05 , offset);
				}
			}
		}
		else
		{
			int snum = p_surface->GetPointNumOfS();
			int tnum = p_surface->GetPointNumOfT();
			for(int s = 0 ; s < snum ;s++)
			{
				for(int t = 0 ; t < tnum ; t++)
				{
					DrawOnePoint(p_surface->m_PointsDivided[s][t].m_Position , Ogre::ColourValue::Green , 0.03 , offset);
				}
			}
		}

		if(m_EditMode == MODE_POINT)
		{
// 			if(m_SurfaceInfo.m_IndexOfI != -1 && m_SurfaceInfo.m_IndexOfJ != -1)
// 				DrawOnePoint(m_SurfaceInfo.m_Points[m_SurfaceInfo.m_IndexOfI][m_SurfaceInfo.m_IndexOfJ] , Ogre::ColourValue::Red , 0.05 , offset);
			if(p_surface->m_SelectedControlPointIndices.size() != 0)
			{
				const std::vector<SurfaceControlPointIndex> & indices = p_surface->m_SelectedControlPointIndices;
				for(int p = 0 ; p < indices.size() ; p++)
				{
					DrawOnePoint(p_surface->m_Points[indices[p].m_IIndex][indices[p].m_JIndex] , Ogre::ColourValue::Red , 0.05 , offset);
				}
			}
		}
		else if(m_EditMode == MODE_DIVIEDED_POINT)
		{
			if(p_surface->m_SelectedPointIndices.size() != 0)
			{
				const std::vector<SurfacePointIndex> & indices = p_surface->m_SelectedPointIndices;
				for(int p = 0 ; p < indices.size() ; p++)
				{
					DrawOnePoint(p_surface->m_PointsDivided[indices[p].m_SIndex][indices[p].m_TIndex].m_Position, Ogre::ColourValue::Red , m_DetectRadius , offset);
				}
			}
		}
		m_pManual->end();

		if(m_EditMode == MODE_POINT)
		{
//			if(m_SurfaceInfo.m_IndexOfI != -1 && m_SurfaceInfo.m_IndexOfJ != -1)
			if(p_surface->m_SelectedControlPointIndices.size() != 0)
			{
				Ogre::Vector3 right = m_pCamera->getRealRight();
				Ogre::Vector3 up = m_pCamera->getRealUp();
				Ogre::Vector3 dir = m_pCamera->getRealDirection();
				//DrawCoord(m_SurfaceInfo.m_Points[m_SurfaceInfo.m_IndexOfI][m_SurfaceInfo.m_IndexOfJ] , right , up , -dir ) ;
				DrawCoord(p_surface->m_SelectedControlCenterPos , right , up , -dir ) ;
			}
		}
		if(m_EditMode == MODE_DIVIEDED_POINT)
		{
			if(p_surface->m_SelectedPointIndices.size() != 0)
			{
				Ogre::Vector3 right = m_pCamera->getRealRight();
				Ogre::Vector3 up = m_pCamera->getRealUp();
				Ogre::Vector3 dir = m_pCamera->getRealDirection();
				DrawCoord(p_surface->m_SelectedPointsCenterPos , right , up , -dir ) ;
			}
		}
		if(m_EditMode == MODE_WHOLE || m_EditMode == MODE_ROTATE_WHOLE || m_EditMode == MODE_SCALE)
		{
			Ogre::Vector3 right = m_pCamera->getRealRight();
			Ogre::Vector3 up = m_pCamera->getRealUp();
			Ogre::Vector3 dir = m_pCamera->getRealDirection();
			//DrawCoord(m_SurfaceInfo.m_CurrentSurfaceCenter , right , up , -dir);
			DrawCoord(p_surface->m_CenterPos , right , up , -dir);
		}
	}

	for(int i = 0 ; i < m_Surfaces.size() ; i++)
		m_Surfaces[i]->Update(dt , m_pCamera);
}

void VeinConnectObjForEditor::OnMouseLeftPressed(float tx , float ty , bool isMultiSelect)
{
	//编辑控制点
	if(m_EditMode ==  MODE_POINT)						
	{
		if(m_EditState == STATE_NONE)
		{
			bool picked =  PickControlPoint(m_pCamera , tx ,ty , isMultiSelect);
				if(picked)
					m_EditState = STATE_POINT_SELECTED;
		}
		else if(m_EditState == STATE_POINT_SELECTED)
		{
			if(m_SelectedAxis != CV_NONE)
			{
				m_EditState = STATE_MOVING_POINT;
				m_LastMx = tx;
				m_LastMy = ty;
			}
			else
			{
				bool picked =  PickControlPoint(m_pCamera , tx ,ty , isMultiSelect);
				if(picked)
					m_EditState = STATE_POINT_SELECTED;
				else 
					m_EditState = STATE_NONE;
			}
		}
	}
	//编辑细分的点
	else if(m_EditMode ==  MODE_DIVIEDED_POINT)						
	{
		if(m_EditState == STATE_NONE)
		{
			bool picked =  PickSubDividedPoint(m_pCamera , tx ,ty , isMultiSelect);
			if(picked)
				m_EditState = STATE_POINT_SELECTED;
		}
		else if(m_EditState == STATE_POINT_SELECTED)
		{
			if(m_SelectedAxis != CV_NONE)
			{
				m_EditState = STATE_MOVING_POINT;
				m_LastMx = tx;
				m_LastMy = ty;
			}
			else
			{
				bool picked =  PickSubDividedPoint(m_pCamera , tx ,ty , isMultiSelect);
				if(picked)
					m_EditState = STATE_POINT_SELECTED;
				else 
					m_EditState = STATE_NONE;
			}
		}
	}
	 //移动整体
	else if(m_EditMode == MODE_WHOLE)								
	{
		 if(m_EditState == STATE_POINT_SELECTED)
		 {
			 if(m_SelectedAxis != CV_NONE)
			 {
				 m_EditState = STATE_MOVING_POINT;
				 m_LastMx = tx;
				 m_LastMy = ty;
			 }
		 }
	}
	//旋转整体
	else if(m_EditMode == MODE_ROTATE_WHOLE)
	{
		if(m_EditState == STATE_POINT_SELECTED)
		{
			if(m_SelectedAxis != CV_NONE)
			{
				m_EditState = STATE_ROTATING_POINT;
				m_LastMx = tx;
				m_LastMy = ty;
			}
		}
	}
	//选择四边形
	else if(m_EditMode == MODE_QUAD)
	{
		if(m_SurfaceSelectedIndex >= 0)
		{
			Ogre::Ray mouseRay;
			m_pCamera->getCameraToViewportRay(tx , ty ,&mouseRay);
			m_Surfaces[m_SurfaceSelectedIndex]->PickQuad(mouseRay);
		}
	}

}

void VeinConnectObjForEditor::OnMouseRightPressed(float tx , float ty)
{
	if(m_EditMode ==  MODE_POINT)
	{
		if(m_EditState == STATE_POINT_SELECTED)
		{
			m_Surfaces[m_SurfaceSelectedIndex]->m_SelectedControlPointIndices.clear();
			m_EditState = STATE_NONE;
		}
	}
	else if(m_EditMode == MODE_DIVIEDED_POINT)
	{
		if(m_EditState == STATE_POINT_SELECTED)
		{
			m_Surfaces[m_SurfaceSelectedIndex]->m_SelectedPointIndices.clear();
			m_EditState = STATE_NONE;
		}
	}
}

void VeinConnectObjForEditor::OnMouseMoved(char button ,float tx , float ty)
{
	if(m_EditState == STATE_POINT_SELECTED)
	{
		float r;
		m_SelectedAxis = PickAxis(m_pCamera , tx , ty ,r);
	}
	else if(m_EditState == STATE_MOVING_POINT)
	{
		ChangePointPosition(tx,ty);
	}
	else if(m_EditState == STATE_ROTATING_POINT)
	{
		RotateControlPoint(tx , ty);
	}
}

void VeinConnectObjForEditor::OnMouseReleased(char button ,float tx , float ty)
{
	if(m_EditState == STATE_MOVING_POINT)
	{
		m_EditState = STATE_POINT_SELECTED;
	}
	else if(m_EditState == STATE_ROTATING_POINT)
	{
		m_EditState = STATE_POINT_SELECTED;
	}
}

void VeinConnectObjForEditor::OnWheelEvent(int delta)
{
	if(m_EditMode == MODE_POINT)
	{
		if(m_EditState == STATE_POINT_SELECTED)
		{
			Ogre::Vector3 dir = m_pCamera->getRealDirection();

			if(m_SurfaceSelectedIndex < 0)
				return;
			m_Surfaces[m_SurfaceSelectedIndex]->ChangeSelectedControlPointsPos(dir * delta / 360);
			m_SelectedAxis = CV_DIR;


		}
	}
	else if(m_EditMode == MODE_DIVIEDED_POINT)
	{
		if(m_EditState == STATE_POINT_SELECTED)
		{
			Ogre::Vector3 dir = m_pCamera->getRealDirection();

			if(m_SurfaceSelectedIndex < 0)
				return;
			m_Surfaces[m_SurfaceSelectedIndex]->ChangeSelectedSubdividedPointsPos(dir * delta / 360);
			m_SelectedAxis = CV_DIR;
		}
	}
	else if(m_EditMode == MODE_WHOLE)
	{
		if(m_EditState == STATE_POINT_SELECTED)
		{
			if(m_SurfaceSelectedIndex < 0)
				return;
			Ogre::Vector3 dir = m_pCamera->getRealDirection();
			m_Surfaces[m_SurfaceSelectedIndex]->Translate(dir * (double)delta / 360);
			//UpdateInfo();
			m_SelectedAxis = CV_DIR;
		}
	}
	else if(m_EditMode == MODE_ROTATE_WHOLE)
	{
		if(m_EditState == STATE_POINT_SELECTED)
		{
			const double TWO_PI = 6.283185;

			if(m_SurfaceSelectedIndex < 0)
				return;
			m_Surfaces[m_SurfaceSelectedIndex]->Rotate(m_Surfaces[m_SurfaceSelectedIndex]->m_CenterPos, m_pCamera->getRealDirection() , (double)delta / 360 *  TWO_PI);
			//UpdateInfo();
		}
	}
	else if(m_EditMode == MODE_SCALE)
	{
		if(m_EditState == STATE_POINT_SELECTED)
		{
			if(m_SurfaceSelectedIndex < 0)
				return;
			double factor  = 0;
			if(delta >= 0)
			{
				factor = 1 + (double)delta / 360;
			}
			else 
			{
				factor =  1 + (double)delta / 360;
			}
			m_Surfaces[m_SurfaceSelectedIndex]->Scale(factor);
			//UpdateInfo();
		}
	}
}

bool VeinConnectObjForEditor::PickControlPoint(Ogre::Camera * camera,float mousex,float mousey , bool isMultiSelect)
{
	if(m_SurfaceSelectedIndex < 0)
		return false;

	//编辑整体
	if(m_EditMode == MODE_WHOLE)
	{
// 		m_SurfaceInfo.m_IndexOfI = -1;
// 		m_SurfaceInfo.m_IndexOfJ = -1;
		return true;
	}

	//选择单个控制点
	else if(m_EditMode == MODE_POINT)
	{
		Ogre::Ray mouse_ray;
		camera->getCameraToViewportRay(mousex , mousey ,&mouse_ray);  
		return m_Surfaces[m_SurfaceSelectedIndex]->PickControlPoint(mouse_ray , isMultiSelect);
	}

}

bool VeinConnectObjForEditor::PickSubDividedPoint(Ogre::Camera * camera,float mousex,float mousey , bool isMultiSelect)
{
	if(m_SurfaceSelectedIndex < 0)
		return false;
	Ogre::Ray mouse_ray;
	camera->getCameraToViewportRay(mousex , mousey ,&mouse_ray);  
	return m_Surfaces[m_SurfaceSelectedIndex]->PickPoint(mouse_ray , isMultiSelect);
}

CameraVector VeinConnectObjForEditor::PickAxis(Ogre::Camera *camera, float mousex, float mousey,float &ratio)
{
	if(m_SurfaceSelectedIndex < 0)
		return CV_NONE;
	
	BicubicBezierSurface *p_surface = m_Surfaces[m_SurfaceSelectedIndex];

	Ogre::Ray mouse_ray;
	camera->getCameraToViewportRay(mousex , mousey ,&mouse_ray);  

	Ogre::Vector3 point;

	if(m_EditMode == MODE_POINT)
		point = p_surface->m_SelectedControlCenterPos;//m_SurfaceInfo.m_Points[m_SurfaceInfo.m_IndexOfI][m_SurfaceInfo.m_IndexOfJ];
	else if(m_EditMode == MODE_DIVIEDED_POINT)
		point = p_surface->m_SelectedPointsCenterPos; //m_SurfaceInfo.m_CurrentSurfaceCenter;
	else 
		point = p_surface->m_CenterPos;

	const Ogre::Vector3 &origin = mouse_ray.getOrigin();
	const Ogre::Vector3 &direction = mouse_ray.getDirection();

	Ogre::Vector3 o_to_p = point - origin;
	Ogre::Vector3 projection = o_to_p.dotProduct(direction) * direction;
	Ogre::Vector3 temp = projection - o_to_p;
	double length_square = temp.squaredLength();

	Ogre::Vector3 U,V,W;

	Ogre::Vector3 right = camera->getRealRight();
	Ogre::Vector3 up = camera->getRealUp();
	Ogre::Vector3 dir = -camera->getRealDirection();

	CameraVector axis_type = CV_NONE;

	ratio = o_to_p.length() / camera->getNearClipDistance();

	if(length_square < 0.04f)
	{
		float max = -FLT_MAX;
		float temp_dot_right =  temp.dotProduct(right);
		if(temp_dot_right > max && temp_dot_right > 0)
		{
			axis_type = CV_RIGHT;
			max = temp_dot_right;
		}

		float temp_dot_up = temp.dotProduct(up);
		if(temp_dot_up > max && temp_dot_up > 0)
		{
			axis_type = CV_UP;
			max = temp_dot_up;
		}

		//for test
		float temp_dot_dir = temp.dotProduct(dir);
		if (temp_dot_dir > max && temp_dot_dir > 0)
		{
			axis_type = CV_DIR;
		}
	}
	return axis_type;
}

void VeinConnectObjForEditor::ChangePointPosition(float mousex ,float mousey)
{
	if(m_SurfaceSelectedIndex < 0)
		return;
	BicubicBezierSurface *p_surface = m_Surfaces[m_SurfaceSelectedIndex];

	Ogre::Vector3 camera_position = m_pCamera->getRealPosition();
	
	Ogre::Real left,right,top,bottom;
	m_pCamera->getFrustumExtents(left,right,top,bottom);

	if(m_EditMode == MODE_POINT && m_SelectedAxis != CV_DIR)
	{
		Ogre::Vector3 point = p_surface->m_SelectedControlCenterPos;//m_SurfaceInfo.m_Points[m_SurfaceInfo.m_IndexOfI][m_SurfaceInfo.m_IndexOfJ];
		Ogre::Vector3 delta = Ogre::Vector3::ZERO;
		Ogre::Vector3 camera_to_point = point - camera_position;
		float distance = camera_to_point.length();
		float ratio = distance / m_pCamera->getNearClipDistance();

		if(m_SelectedAxis == CV_RIGHT)
		{
			float dx = mousex - m_LastMx;
			float change = dx * (right - left) * ratio;
			//point += m_pCamera->getRealRight() * change;
			delta =  m_pCamera->getRealRight() * change;
		}
		else if(m_SelectedAxis == CV_UP)
		{
			float dy = mousey - m_LastMy;
			float change = dy * (bottom - top) * ratio;
//			point += m_pCamera->getRealUp() * change;
			delta = m_pCamera->getRealUp() * change;
		}
		else if(m_SelectedAxis == CV_DIR)
		{
// 			float dy = mousey - m_LastMy;
// //			float change = dy * (bottom - top) * ratio;
// 			float change =  dy / CalDirLength(point);
// 
// 			Ogre::Vector3 dir = m_pCamera->getRealDirection();
// 			
// // 			camera_to_point.normalise();
// // 			change /= camera_to_point.dotProduct(dir);
// 			point += dir * change;
		}
		
// 		 m_SurfaceInfo.m_Points[m_SurfaceInfo.m_IndexOfI][m_SurfaceInfo.m_IndexOfJ] = point;
// 		 m_Surfaces[m_SurfaceSelectedIndex]->SetPoint(m_SurfaceInfo.m_IndexOfI , m_SurfaceInfo.m_IndexOfJ , point);
		p_surface->ChangeSelectedControlPointsPos(delta);
	}
	else if(m_EditMode == MODE_DIVIEDED_POINT && m_SelectedAxis != CV_DIR)
	{
		Ogre::Vector3 point = p_surface->m_SelectedPointsCenterPos;//m_SurfaceInfo.m_Points[m_SurfaceInfo.m_IndexOfI][m_SurfaceInfo.m_IndexOfJ];
		Ogre::Vector3 delta = Ogre::Vector3::ZERO;
		Ogre::Vector3 camera_to_point = point - camera_position;
		float distance = camera_to_point.length();
		float ratio = distance / m_pCamera->getNearClipDistance();

		if(m_SelectedAxis == CV_RIGHT)
		{
			float dx = mousex - m_LastMx;
			float change = dx * (right - left) * ratio;
			delta =  m_pCamera->getRealRight() * change;
		}
		else if(m_SelectedAxis == CV_UP)
		{
			float dy = mousey - m_LastMy;
			float change = dy * (bottom - top) * ratio;
			delta = m_pCamera->getRealUp() * change;
		}
		p_surface->ChangeSelectedSubdividedPointsPos(delta);

	}
	else if(m_EditMode == MODE_WHOLE && m_SelectedAxis != CV_DIR)
	{
		Ogre::Vector3 point = p_surface->m_CenterPos;//m_SurfaceInfo.m_CurrentSurfaceCenter;
		Ogre::Vector3 camera_to_point = point - camera_position;
		float distance = camera_to_point.length();
		float ratio = distance / m_pCamera->getNearClipDistance();

		if(m_SelectedAxis == CV_RIGHT)
		{
			float dx = mousex - m_LastMx;
			float change = dx * (right - left) * ratio;
			p_surface->Translate(m_pCamera->getRealRight() * change);
			//UpdateInfo();
		}
		else if(m_SelectedAxis == CV_UP)
		{
			float dy = mousey - m_LastMy;
			float change = dy * (bottom - top) * ratio;
			p_surface->Translate(m_pCamera->getRealUp() * change);
			//UpdateInfo();
		}
// 		else if(m_SelectedAxis == DIR)
// 		{
// 		}
	}
	m_LastMx = mousex;
	m_LastMy = mousey;
}

void VeinConnectObjForEditor::RotateControlPoint(float mousex ,float mousey)
{
	if(m_SelectedAxis != CV_DIR)
	{
		const double TWO_PI = 6.283185;
		if(m_SelectedAxis == CV_RIGHT)
		{
			float dy = mousey - m_LastMy;
			if(m_SurfaceSelectedIndex >= 0 )
				m_Surfaces[m_SurfaceSelectedIndex]->Rotate(m_Surfaces[m_SurfaceSelectedIndex]->m_CenterPos, m_pCamera->getRealRight() , dy * TWO_PI);
			//UpdateInfo();
		}
		else if(m_SelectedAxis == CV_UP)
		{
			float dx = mousex - m_LastMx;
			if(m_SurfaceSelectedIndex >= 0 )
				m_Surfaces[m_SurfaceSelectedIndex]->Rotate(m_Surfaces[m_SurfaceSelectedIndex]->m_CenterPos , m_pCamera->getRealUp() , dx * TWO_PI);
			//UpdateInfo();
		}
	}
	m_LastMx = mousex;
	m_LastMy = mousey;
}

void VeinConnectObjForEditor::SetSurfaceSelectedQuadInvalid()
{
	if(m_SurfaceSelectedIndex >= 0 )
		m_Surfaces[m_SurfaceSelectedIndex]->SetSelectedQuadInvalid();
}

void VeinConnectObjForEditor::TextureMapping()
{
}

void VeinConnectObjForEditor::SetSurfaceS(int n)
{
	if(m_SurfaceSelectedIndex >= 0)
		m_Surfaces[m_SurfaceSelectedIndex]->SetPointNumOfS(n);
}

void VeinConnectObjForEditor::SetSurfaceT(int n)
{
	if(m_SurfaceSelectedIndex >= 0)
		m_Surfaces[m_SurfaceSelectedIndex]->SetPointNumOfT(n);
}

void VeinConnectObjForEditor::ToggleCurrentSurfaceVisible()
{
	if(m_SurfaceSelectedIndex >= 0)
		m_Surfaces[m_SurfaceSelectedIndex]->ToggleVisibleMode();
}

void VeinConnectObjForEditor::ToggleCurentSurfaceWireFrameMode()
{
	if(m_SurfaceSelectedIndex >= 0)
		m_Surfaces[m_SurfaceSelectedIndex]->ToggleWireFrameMode();
}

void VeinConnectObjForEditor::ToggleCurrentSurfaceConvexHullMode()
{
	{
		if(m_SurfaceSelectedIndex >= 0)
			m_Surfaces[m_SurfaceSelectedIndex]->ToggleConvexHullMode();
	}
}

void VeinConnectObjForEditor::ToggleTransformationMode()
{
	if(m_EditMode == MODE_WHOLE)
	{
		if(m_EditState == STATE_POINT_SELECTED)
			m_EditState = STATE_ROTATING_POINT;
		else if(m_EditState == STATE_ROTATING_POINT)
			m_EditState = STATE_POINT_SELECTED;
	}
}

void VeinConnectObjForEditor::AutoAttachSurfacePointToOrgan(float detectionRadius , std::vector<MisMedicOrganInterface*> & organInterfaces , int mode)
{
	if(m_SurfaceSelectedIndex >= 0 && mode == 0)
		m_Surfaces[m_SurfaceSelectedIndex]->AutoAttachPointToOrgan(detectionRadius , organInterfaces);
	
	if(m_SurfaceSelectedIndex >= 0 && mode == 1)
		m_Surfaces[m_SurfaceSelectedIndex]->AttachSelectedPointsToOrgan(detectionRadius , organInterfaces);
}

void VeinConnectObjForEditor::ToggleCurrentSurfacePairVisible()
{
	if(m_SurfaceSelectedIndex >= 0)
		m_Surfaces[m_SurfaceSelectedIndex]->ToggleShowPairMode();
}

void VeinConnectObjForEditor::CreatePairsOfCurrentSurface(int dir)
{
	if(m_SurfaceSelectedIndex >= 0)
	{
		m_Surfaces[m_SurfaceSelectedIndex]->m_DirOfPairDivided = dir;
		m_Surfaces[m_SurfaceSelectedIndex]->CreatePairs(dir);
	}
}

void VeinConnectObjForEditor::ResetSurface()
{
	if(m_SurfaceSelectedIndex >= 0 )
		m_Surfaces[m_SurfaceSelectedIndex]->Reset();
}

bool VeinConnectObjForEditor::SaveVeinObjToFile(const std::string & fileName)
{
	/*
	surface num: x
	surface index
	nums numt
	x y z			//position
	r g b a		//color
	u v			//texcoord
	.....
	attachpoint

	*/
	std::ofstream filestream(fileName.c_str());
	if(!filestream)
		return false;
	
	filestream << "surface num: " << m_Surfaces.size() << std::endl;
	
	for(int surface_index = 0 ; surface_index < m_Surfaces.size() ; surface_index++)
	{
		filestream << "surface " << surface_index << std::endl;
		BicubicBezierSurface *p_surface = m_Surfaces[surface_index];
		p_surface->SaveToFile(filestream);
	}
	filestream.close();

	return true;
}

bool VeinConnectObjForEditor::SaveVeinObjForEditorToFile(const std::string & fileName)
{
	/*
	VeinObj EditableFormat
	surface num: x
	surface index
	numi numj	//control points
	x y z 
	nums numt
	x y z			//position
	r g b a		//color
	u v			//texcoord
	.....
	attachpoint

	*/
	std::ofstream filestream(fileName.c_str());
	if(!filestream)
		return false;
	
	filestream << "VeinObj EditableFormat"  << std::endl;
	filestream << "surface num: " << m_Surfaces.size() << std::endl;
	
	for(int surface_index = 0 ; surface_index < m_Surfaces.size() ; surface_index++)
	{
		filestream << "surface " << surface_index << std::endl;
		BicubicBezierSurface *p_surface = m_Surfaces[surface_index];
		p_surface->SaveEditableFormatToFile(filestream);
	}
	filestream.close();

	return true;
}

bool VeinConnectObjForEditor::ExportToObjFile(const std::string& fileName)
{
	std::ofstream filestream(fileName.c_str());
	if(!filestream)
		return false;
	
	for(int f = 0 ; f < m_Surfaces.size() ; f++)
	{
		BicubicBezierSurface *pSurface = m_Surfaces[f];
		
		int numt = pSurface->GetPointNumOfT();
		int nums = pSurface->GetPointNumOfS();

		for(int t = 0 ; t < numt ; t++)
		{
			for(int s = 0 ; s < nums ; s++)
			{
				const BicubicBezierSurface::SurfacePoint & surface_point = pSurface->m_PointsDivided[s][t];
				//vertex position
				filestream << "v  " << surface_point.m_Position.x << " " << surface_point.m_Position.y << " " << surface_point.m_Position.z << std::endl;
			}
		}
	}
	
	int vertexOffset = 1;

	filestream << "g veinobj"<< std::endl;

	for(int f = 0 ; f < m_Surfaces.size() ; f++)
	{
		BicubicBezierSurface *pSurface = m_Surfaces[f];

		int numt = pSurface->GetPointNumOfT();
		int nums = pSurface->GetPointNumOfS();
		
		std::vector<int> faceindices;
		pSurface->ExportTriFace(faceindices);

		for(int findex = 0 ; findex < faceindices.size() ; findex+= 3)
		{
			filestream << "f " << faceindices[findex] + vertexOffset << " " << faceindices[findex + 1] + vertexOffset << " " << faceindices[findex + 2] + vertexOffset << std::endl;
		}

		vertexOffset += pSurface->GetPointNumOfS() * pSurface->GetPointNumOfT();
	}
	filestream.close();
}

bool VeinConnectObjForEditor::GetTexCoordFromObjFile(const std::string &fileName)
{
	ObjFormatModel objModel;

	bool readResult = ReadFromObjFile(fileName ,objModel);

	return readResult;
}

bool VeinConnectObjForEditor::InitVeinObjFromFile(std::string &filename)
{
	ifstream fin(filename.c_str());
	if(!fin)
		return false;
	std::string temp_str;
	fin >> temp_str >> temp_str;			//unused info "VeinObj EditableFormat"
	fin >> temp_str >> temp_str;			//"surface num:"
	int surface_num;
	fin >> surface_num;
	
	for(int surface_index = 0 ; surface_index < surface_num ; surface_index++)
	{
			fin >> temp_str >> temp_str;		//unused info "surface n"
			fin >> temp_str >>  temp_str >>  temp_str >>  temp_str; //"Divide Direction Of Pair"
			int PairDir = 0;
			fin >> PairDir;
			fin >> temp_str >> temp_str;		//unused info "control points"
			int inum = 0 ; 
			int jnum = 0 ; 
			
			Ogre::Vector3 control_points[4][4];

			fin >> inum >> jnum;			//此版本暂时固定为4 4
			for(int i = 0 ; i < inum ; i++)
			{
				for(int j = 0 ; j < jnum ; j++)
				{
					double x = 0 , y = 0 , z = 0 ;
					fin >> x >> y >> z;
					control_points[i][j] = Ogre::Vector3(x,y,z);
				}
			}
			
			//about vertices
			fin >> temp_str >> temp_str >> temp_str >> temp_str >> temp_str;		//unused info "vertex position color uv alphauv"
			int snum = 0 , tnum = 0 ; 
			fin >> snum >> tnum;

			BicubicBezierSurface *p_surface = new BicubicBezierSurface;
			p_surface->Initialize(control_points[0] , control_points[1] ,
												control_points[2] , control_points[3] ,
												snum , tnum , inum , jnum);

			p_surface->Reset();
			p_surface->CreateQuadInfo();
			p_surface->m_DirOfPairDivided = PairDir;

			for(int t = 0 ; t < tnum ; t++)
			{
				for(int s = 0 ; s < snum ; s++)
				{
					BicubicBezierSurface::SurfacePoint & point = p_surface->m_PointsDivided[s][t];
					fin >> point;
				}
			}

			//about the invalid quad index
			fin >> temp_str >> temp_str >> temp_str;										//unused info "invalid quad info"
			int invalid_quad_num = 0 ;
			fin >> invalid_quad_num;
			for(int i = 0 ; i < invalid_quad_num ; i++)
			{
				int invalid_index = - 1;
				fin >> invalid_index;
				p_surface->m_InValidQuadIndices.push_back(invalid_index);
				p_surface->m_QuadInfos[invalid_index].m_Vaild = false;
			}
			
			//about the attach records
			int record_num = 0;	
			fin >> temp_str >> temp_str;			//"attachpoint num:"
			fin >> record_num;
			for(int r = 0 ; r < record_num ; r++)
			{
				AttachRecordForEditor record;
				record.m_IsVaild = true;
				fin >> record;
				p_surface->m_PointsDivided[record.m_SIndex][record.m_TIndex].m_AttatchRecordID = r;
				p_surface->m_AttachRecords.push_back(record);
			}

			m_Surfaces.push_back(p_surface);
	}
	
	fin.close();
	return true;
	
}

void VeinConnectObjForEditor::DrawOnePoint(const Ogre::Vector3 & position ,const Ogre::ColourValue & color , float size ,int &offset)
{

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

void VeinConnectObjForEditor::DrawCoord(const Ogre::Vector3& origin,const Ogre::Vector3& U,const Ogre::Vector3& V,const Ogre::Vector3& W)
{
	m_pManual->begin("BaseWhiteNoLighting",Ogre::RenderOperation::OT_LINE_LIST);
	m_pManual->position(origin);
	m_pManual->colour(Ogre::ColourValue::Red);
	m_pManual->position(origin + U);
	m_pManual->colour(Ogre::ColourValue::Red);

	m_pManual->position(origin);
	m_pManual->colour(Ogre::ColourValue::Green);
	m_pManual->position(origin + V);
	m_pManual->colour(Ogre::ColourValue::Green);

	m_pManual->position(origin);
	m_pManual->colour(Ogre::ColourValue::Blue);
	m_pManual->position(origin + W);
	m_pManual->colour(Ogre::ColourValue::Blue);

	if(m_SelectedAxis != CV_NONE)
	{
		m_pManual->position(origin);
		m_pManual->colour(1,1,0);
		if(m_SelectedAxis == CV_RIGHT)
		{
			m_pManual->position(origin + U);
			m_pManual->colour(1,1,0);
		}
		else if(m_SelectedAxis == CV_UP)
		{
			m_pManual->position(origin + V);
			m_pManual->colour(1,1,0);
		}
		else if(m_SelectedAxis == CV_DIR)
		{
			m_pManual->position(origin + W);
			m_pManual->colour(1,1,0);
		}
	}
	m_pManual->end();
}

void VeinConnectObjForEditor::UpdateInfo()
{
	if(m_SurfaceSelectedIndex >= 0)
	{
		BicubicBezierSurface *p_surface = m_Surfaces[m_SurfaceSelectedIndex];
		m_SurfaceInfo.m_CurrentSurfaceCenter = p_surface->m_CenterPos;
		for(int i = 0 ; i < CONTROL_POINTS_NUM ;i++)
		{
			for(int j = 0 ; j < CONTROL_POINTS_NUM; j++)
			{
				m_SurfaceInfo.m_Points[i][j] = p_surface->m_Points[i][j];
			}
		}
// 		m_SurfaceInfo.m_IndexOfI = -1;
// 		m_SurfaceInfo.m_IndexOfJ = -1;
	}
}

float VeinConnectObjForEditor::CalDirLength(const Ogre::Vector3 & origin)
{
	Ogre::Vector3 ray_o = m_pCamera->getRealPosition() ;
	Ogre::Vector3 ray_d1 = origin - ray_o;
	Ogre::Vector3 ray_d2 = origin - m_pCamera->getRealDirection(); 
	ray_d1.normalise();
	ray_d2.normalise();

	Ogre::Vector3 plane_a = m_pCamera->getRealDirection() * m_pCamera->getNearClipDistance() + ray_o;
	Ogre::Vector3 plane_normal = -m_pCamera->getRealDirection();

	float an_on = plane_a.dotProduct(plane_normal) - ray_o.dotProduct(plane_normal);
	float t1 = an_on / (ray_d1.dotProduct(plane_normal));
	float t2 = an_on / (ray_d2.dotProduct(plane_normal));

	Ogre::Vector3 p1 = ray_o + t1 * ray_d1;
	Ogre::Vector3 p2 = ray_o + t2 * ray_d2;

	Ogre::Vector3 p1p2 = p1 - p2;
	float up_component = p1p2.dotProduct(m_pCamera->getRealUp());
	
	Ogre::Real left,right,top,bottom;
	m_pCamera->getFrustumExtents(left,right,top,bottom);

	return abs(up_component * (top - bottom) * 100);

}


static bool ReadFromObjFile(const std::string & fileName , ObjFormatModel & objModel)
{
	ifstream in;
	in.open(fileName.c_str());
	
	if(!in)
		return false;

	std::string lineStr;
	while(getline(in , lineStr))
	{
		std::stringstream ss(lineStr);

		std::string tempStr;

		if(lineStr[0] == 'v' && lineStr[1] == 't')	//texcoord
		{
			Ogre::Vector2 tex;
			ss >> tempStr >> tex.x >> tex.y;

			objModel.TexCoords.push_back(tex);
		}
		else if(lineStr[0] == 'v')								//vertex coord
		{
			Ogre::Vector3 vertex;
			ss >> tempStr >> vertex.x >> vertex.y >> vertex.z;

			objModel.Vertices.push_back(vertex);
		}
		else if(lineStr[0] == 'f')
		{
			int v0 , v1 , v2;
			int t0 , t1 , t2;
			char sprit;

			ss >> tempStr; 
			ss>> v0;
			ss.get(sprit);
			ss>> t0;

			ss>> v1;
			ss.get(sprit);
			ss>> t1;

			ss>> v2;
			ss.get(sprit);
			ss>> t2;

			objModel.VertexIndices.push_back(v0);
			objModel.VertexIndices.push_back(v1);
			objModel.VertexIndices.push_back(v2);

			objModel.TexCoordIndices.push_back(t0);
			objModel.TexCoordIndices.push_back(t1);
			objModel.TexCoordIndices.push_back(t2);
		}
	}
}