#include "WaterPool.h"
#include "MXOgreGraphic.h"
#include <OgreMaterialManager.h>
#include "MisNewTraining.h"
using namespace std;

int WaterPool::sNumOfWaterPool = 0;

WaterPool::WaterPool(MisNewTraining * ownerTrain, float meshWidth, float meshHeight,
					 float actualWidth , float actualHeight ,  
					 const Ogre::Vector3 & origin,
					 const Ogre::Vector3 & normal,
					 float maxHeight , 
					 const Ogre::Vector3 & CenterAndStageHeight ,
					 bool IsCenterChangeBySuction,
					 const Ogre::String& meshName)
: m_pWaterPlane(NULL) , 
m_Origin(origin),
m_Normal(normal),
m_MeshWidth(meshWidth),
m_MeshHeight(meshHeight),
m_ActualBottomArea(actualWidth * actualHeight) , 
m_CurrHeight(0) , 
m_StageHeight(CenterAndStageHeight.z),
m_OriginCenter(CenterAndStageHeight.x , CenterAndStageHeight.y),
m_MaxHeight(maxHeight),
m_MaxVolume(actualWidth * actualHeight * maxHeight) ,
m_WaterVolume(0),
m_BloodVolume(0),
m_IsCenterChangeBySuction(IsCenterChangeBySuction),
m_planeNode(nullptr),
m_planeEntity(nullptr),
m_planeInitPosition(m_Origin),
m_OwnerTrain(ownerTrain),
m_IsSoakOrgans(false)
{
	sNumOfWaterPool++;

	m_Normal.normalise();

	//m_pWaterPlane = MXOgreWrapper::Get()->GetDefaultSceneManger()->createManualObject();

	if(m_pWaterPlane)
	{
		m_Normal.normalise();

		Ogre::Vector3 temp = Ogre::Vector3::UNIT_Y;

		if(m_Normal.dotProduct(temp) > 0.95)
			temp = Ogre::Vector3::UNIT_Z;

		m_Axis0 = m_Normal.crossProduct(temp);

		m_Axis1 = m_Normal.crossProduct(m_Axis0);

		m_Axis0.normalise();

		m_Axis1.normalise();
		
		m_WaterColor = Ogre::ColourValue(1.0f , 1.0f , 1.0f , 0.8f);

		m_BloodColor = Ogre::ColourValue(62.0 / 255.0 , 0.0 / 255.0, 0.0 / 255.0 , 1.0);

		m_HybridColor = Ogre::ColourValue(0,0,0,0);

		
		
		m_pWaterPlane->begin("MisMedic/WaterPool");

		DrawWaterPlane(false);

		m_pWaterPlane->end();

		MXOgreWrapper::Get()->GetDefaultSceneManger()->getRootSceneNode()->attachObject(m_pWaterPlane);

		CShadowMap::Instance()->AddListener(this);
	}
	m_BloodRatio = 0.8f;
	Ogre::String planeName = "WaterPool_" + Ogre::StringConverter::toString(sNumOfWaterPool);
	m_materialName = planeName;

	auto materialPtr = static_cast<Ogre::MaterialPtr>(Ogre::MaterialManager::getSingletonPtr()->getByName(m_materialName));
	if(materialPtr.isNull()){
		materialPtr = static_cast<Ogre::MaterialPtr>(Ogre::MaterialManager::getSingletonPtr()->getByName("MisMedic/WaterPool"));
		materialPtr->clone(m_materialName);
	}

	m_planeEntity = MXOgreWrapper::Get()->GetDefaultSceneManger()->createEntity(m_materialName, meshName);
	m_planeEntity->setMaterialName(m_materialName);
	
	m_planeNode = MXOgreWrapper::Get()->GetDefaultSceneManger()->getRootSceneNode()->createChildSceneNode();
	m_planeNode->attachObject(m_planeEntity);
	m_planeNode->setPosition(m_planeInitPosition);

	m_ExtOffset = Ogre::Vector3::ZERO;

	m_RefNode = 0;

	m_TimeSinceLast = 0;

	if (m_planeEntity)
	{
		std::vector<Ogre::Vector2> textures;
		MisMedicOrganInterface::ExtractOgreMeshInfo(m_planeEntity->getMesh(),
                                                    m_vertices, textures, m_indices);

	}
}

WaterPool::~WaterPool()
{
	if(m_pWaterPlane)
	{
		CShadowMap::Instance()->RemoveListener(this);

		m_pWaterPlane->detachFromParent();

		MXOgreWrapper::Get()->GetDefaultSceneManger()->destroyManualObject(m_pWaterPlane);
	}

	if(m_planeNode){
		m_planeEntity->detachFromParent();
		MXOgreWrapper::Get()->GetDefaultSceneManger()->destroyEntity(m_planeEntity);
		MXOgreWrapper::Get()->GetDefaultSceneManger()->destroySceneNode(m_planeNode);
	}

	sNumOfWaterPool--;
}

void WaterPool::preRenderShadowDepth()
{
	if(m_pWaterPlane)
	   m_pWaterPlane->setVisible(false);

	if (m_planeEntity)
		m_planeEntity->setVisible(false);
}
void WaterPool::postRenderShadowDepth()
{
	if(m_pWaterPlane)
	   m_pWaterPlane->setVisible(true);

	if (m_planeEntity)
		m_planeEntity->setVisible(true);
}

void WaterPool::OnOrganBleeding(int organId , int trackId)
{
	if(m_RejectOrganIDs.count(organId) == 0)
	{
		m_BloodTracks.push_back(BloodTrack(organId,trackId));
	}
}

void WaterPool::OnOrganStopBleeding(int organId , int trackId)
{
	if(m_RejectOrganIDs.count(organId) == 0)
	{
		for(size_t t = 0 ; t < m_BloodTracks.size() ; t++)
		{
			BloodTrack & track = m_BloodTracks[t];
			if(track.OrganId == organId && track.TrackId == trackId)
			{
				m_BloodTracks.erase(m_BloodTracks.begin() + t);
				return;
			}
		}
	}	
}

void WaterPool::SetRejectOrgan(const std::set<int> & rejectOrganIDs)
{
	m_RejectOrganIDs = rejectOrganIDs;
}

bool WaterPool::IsReject(int organId)
{
	return m_RejectOrganIDs.count(organId) > 0;
}

void WaterPool::Update(float dt)
{
	if(!m_BloodTracks.empty())
	{
		AddBlood(dt * m_BloodTracks.size() * 0.1f);
	}

	m_TimeSinceLast += dt;

	if (m_TimeSinceLast > 0.1f)
	{
		std::vector<MisMedicOrganInterface*> organs;

		m_OwnerTrain->GetAllOrgan(organs);

		for (int c = 0; c < (int)organs.size(); c++)
		{
			MisMedicOrgan_Ordinary * organ = dynamic_cast<MisMedicOrgan_Ordinary*>(organs[c]);
			if (organ && !IsReject(organ->m_OrganID))
			{
				Ogre::Vector3 planeorigin = GetCurOrigin();
				Ogre::Vector3 planenormal = GetPlaneNormal();///Ö»Ò»¸öÑª³Ø

				if (m_IsSoakOrgans)
				   organ->ApplyEffect_Soak(planeorigin, planenormal);
			}
		}
		m_TimeSinceLast = 0;
	}
}

void WaterPool::AddWater(float volume)
{
	/*
	float tempVolume = m_BloodVolume + m_WaterVolume + volume;
	float diff = tempVolume - m_MaxVolume;
	if(diff > 0) 
	{
		if(m_BloodVolume >= diff)
		{
			m_BloodVolume -= diff;
			m_WaterVolume += volume;
		}
		else
		{
			m_BloodVolume = 0.f ;
			m_WaterVolume = m_MaxVolume;
		}
		m_CurrHeight = m_MaxHeight;
	}
	else
	{
		m_WaterVolume += volume;
		m_CurrHeight += volume / m_ActualBottomArea;
	}*/
	float volumedelta = volume * 0.01f;
	if(m_CurrHeight + volumedelta > m_MaxHeight)
	{
       m_CurrHeight = m_MaxHeight;
	}
	else
	{
       m_CurrHeight = m_CurrHeight + volumedelta;
	}
    
	m_WaterVolume += volumedelta;

    //recalculate
	if(m_WaterVolume+m_BloodVolume > 0.00001f)
	{
	   m_WaterVolume = m_CurrHeight * m_WaterVolume / (m_WaterVolume+m_BloodVolume);
	   m_BloodVolume = m_CurrHeight * m_BloodVolume / (m_WaterVolume+m_BloodVolume);
	  // m_BloodRatio  = m_BloodVolume / (m_WaterVolume+m_BloodVolume);
	}
	else
	{
       //m_BloodRatio  = 1.0f;
	}
	//GPClamp(m_BloodRatio , 0.15f , 0.8f);

	//m_BloodRatio -= volume * 0.01f;
	//GPClamp(m_BloodRatio , 0.15f , 0.8f);

	//m_CurrHeight += volume * 0.01f;// / m_ActualBottomArea;
	//if(m_CurrHeight > m_MaxHeight)
	 //  m_CurrHeight = m_MaxHeight;

	UpdateLiquids();
	UpdateWaterPlane();
}

void WaterPool::AddBlood(float volume)
{
	/*float tempVolume = m_BloodVolume + m_WaterVolume + volume;
	float diff = tempVolume - m_MaxVolume;
	if(diff > 0) 
	{
		if(m_WaterVolume >= diff)
		{
			m_WaterVolume -= diff;
			m_BloodVolume += volume;
		}
		else
		{
			m_WaterVolume = 0.f ;
			m_BloodVolume = m_MaxVolume;
		}
		m_CurrHeight = m_MaxHeight;
	}
	else
	{
		m_BloodVolume += volume;
		m_CurrHeight += volume / m_ActualBottomArea;
	}
	*/

	//m_BloodRatio += volume * 0.01f;
	//GPClamp(m_BloodRatio , 0.15f , 0.8f);

	//m_CurrHeight += volume *0.01f;/// m_ActualBottomArea;
	//if(m_CurrHeight > m_MaxHeight)
	 //  m_CurrHeight = m_MaxHeight;

	float volumedelta = volume * 0.04f;
	if(m_CurrHeight + volumedelta > m_MaxHeight)
	{
	   m_CurrHeight = m_MaxHeight;
	}
	else
	{
	   m_CurrHeight = m_CurrHeight + volumedelta;
	}

	m_BloodVolume += volumedelta;
	//recalculate
	if(m_WaterVolume+m_BloodVolume > 0.00001f)
	{
	   m_WaterVolume = m_CurrHeight * m_WaterVolume / (m_WaterVolume+m_BloodVolume);
	   m_BloodVolume = m_CurrHeight * m_BloodVolume / (m_WaterVolume+m_BloodVolume);
	   m_BloodRatio  = m_BloodVolume / (m_WaterVolume+m_BloodVolume);
	}
	else
	{
	   m_BloodRatio  = 1.0f;
	}
	GPClamp(m_BloodRatio , 0.15f , 0.8f);

	UpdateLiquids();
	
	UpdateWaterPlane();
}

void WaterPool::Reduce(float volume ,  const Ogre::Vector2 & planePos)
{
	//temp
	if(m_CurrHeight <= 0.250)
	{
		//m_TotalVolume = 0.0f;
		//m_BloodVolume = 0.0f;
		//m_WaterVolume = 0.0f;
		//m_BloodRatio = 0.0f;
		//m_WaterRatio = 0.0f;
		//m_CurrHeight = 0.0f;
		//m_ActualCenter = m_OriginCenter;
	}
	//else if(m_TotalVolume > volume)
	//{
		//m_TotalVolume -= volume;
		//m_BloodVolume  = m_TotalVolume * m_BloodRatio;
		//m_WaterVolume = m_TotalVolume * m_WaterRatio;
		//m_CurrHeight -= volume / m_ActualBottomArea;
		//if(m_CurrHeight < 0 )
		 //  m_CurrHeight = 0;

		//if(m_IsCenterChangeBySuction)
		//	m_ActualCenter = planePos;
	//}
	//else
	//{
	//	m_TotalVolume = 0.0f;
		//m_BloodVolume = 0.0f;
		//m_WaterVolume = 0.0f;
		//m_BloodRatio = 0.0f;
		//m_WaterRatio = 0.0f;
		//m_CurrHeight = 0.0f;
		//m_ActualCenter = m_OriginCenter;
	//}
	//volume = volume * 0.0025f;
	//if(m_CurrHeight - volume < 0)
	//{
    //   m_CurrHeight = 0;
     //  m_BloodVolume = m_WaterVolume = 0;
	//}
	//else
	//{
	   m_CurrHeight = m_CurrHeight - 0.01f;// volume;
      // if(m_BloodVolume + m_WaterVolume > 0.0001)
	   //{
		 //// m_WaterVolume = m_CurrHeight * m_WaterVolume / (m_WaterVolume+m_BloodVolume);
		 // m_BloodVolume = m_CurrHeight * m_BloodVolume / (m_WaterVolume+m_BloodVolume);
		 // m_BloodRatio  = m_BloodVolume / (m_WaterVolume+m_BloodVolume);
	   //}
	//}
	UpdateWaterPlane();
}

bool WaterPool::IsOnePointInWaterPool(const Ogre::Vector3 & point , Ogre::Vector2 & pos)
{
	float tolerant = 0.5f;

	Ogre::Vector3 diff = point - m_Origin;
	float zheight = diff.dotProduct(m_Normal);
	pos.x = diff.dotProduct(m_Axis0);
	pos.y = diff.dotProduct(m_Axis1);
	
	float meshWidth = m_MeshWidth;
	float meshHeight = m_MeshHeight;

	if(m_CurrHeight < m_StageHeight && m_CurrHeight > 0.01)
	{
		float scale  = m_CurrHeight / m_StageHeight;
		scale = scale * scale;
		meshHeight *= scale;
		meshWidth *= scale;
	}

	if(zheight > -0.05 && zheight < (m_CurrHeight+tolerant) && abs(pos.x) < (meshWidth * 0.5) && abs(pos.y) < (meshHeight * 0.5))
	{
		pos.x = pos.x / m_MeshWidth + 0.5;
		pos.y = pos.y / m_MeshHeight + 0.5;
		return true;
	}
	else
		return false;
}

bool WaterPool::IsSegementInsectWaterPool(const Ogre::Vector3 & point0, const Ogre::Vector3 & point1)
{
	for (int t = 0; t < m_indices.size(); t += 3)
	{
		Ogre::Vector3 origin = GetCurOrigin();

		GFPhysVector3 a = OgreToGPVec3(m_vertices[m_indices[t]] + origin);
		GFPhysVector3 b = OgreToGPVec3(m_vertices[m_indices[t + 1]] + origin);
		GFPhysVector3 c = OgreToGPVec3(m_vertices[m_indices[t + 2]] + origin);

		float rayweight;
		float triWeight[3];
		GFPhysVector3 intersectPt;
		bool succed = LineIntersectTriangle(a, b, c, OgreToGPVec3(point0), OgreToGPVec3(point1), rayweight, intersectPt, triWeight);

		if (succed && rayweight > 0 && rayweight < 1)
		{
			return true;
		}
	}
	return false;
}
void WaterPool::DrawWaterPlane(bool isDrawTwo)
{
	Ogre::Vector3 dir0 = m_MeshWidth * m_Axis0 * 0.5;
	Ogre::Vector3 dir1 = m_MeshHeight * m_Axis1 * 0.5;

	Ogre::Vector3 currOrigin = m_Origin + m_CurrHeight * m_Normal + m_ExtOffset;

	m_pWaterPlane->position(currOrigin + ( -dir0  - dir1));
	m_pWaterPlane->normal(m_Normal);
	m_pWaterPlane->textureCoord(0,0);
	m_pWaterPlane->textureCoord(m_WaterRatio,m_BloodRatio,m_CurrHeight,m_StageHeight);

	m_pWaterPlane->position(currOrigin + ( dir0 - dir1));
	m_pWaterPlane->normal(m_Normal);
	m_pWaterPlane->textureCoord(1,0);
	m_pWaterPlane->textureCoord(m_WaterRatio,m_BloodRatio,m_CurrHeight,m_StageHeight);

	m_pWaterPlane->position(currOrigin + ( dir0 + dir1));
	m_pWaterPlane->normal(m_Normal);
	m_pWaterPlane->textureCoord(1,1);
	m_pWaterPlane->textureCoord(m_WaterRatio,m_BloodRatio,m_CurrHeight,m_StageHeight);

	m_pWaterPlane->position(currOrigin + ( -dir0 + dir1));
	m_pWaterPlane->normal(m_Normal);
	m_pWaterPlane->textureCoord(0,1);
	m_pWaterPlane->textureCoord(m_WaterRatio,m_BloodRatio,m_CurrHeight,m_StageHeight);

	m_pWaterPlane->quad(0,1,2,3);
}

void WaterPool::UpdateWaterPlane()
{

	if (m_RefNode)
	{
		GFPhysVector3 offset = m_RefNode->m_CurrPosition - m_RefNode->m_UnDeformedPos;
		m_ExtOffset = GPVec3ToOgre(offset).dotProduct(m_Normal)*m_Normal*0.8f;//0.5 is magic num^_^
	}
	else
	{
		m_ExtOffset = Ogre::Vector3(0, 0, 0);

	}
	if(m_pWaterPlane)
	{
		CalcHybridColor();

		m_pWaterPlane->beginUpdate(0);
		DrawWaterPlane(false);
		m_pWaterPlane->end();
	}

	if(m_planeNode){
		//adjust plane position
		Ogre::Vector3 newPosition = m_planeInitPosition +m_CurrHeight * m_Normal;// +m_ExtOffset;
		m_planeNode->setPosition(newPosition);

		//update uniform parameters
		//auto materialPtr = static_cast<Ogre::MaterialPtr>(Ogre::MaterialManager::getSingletonPtr()->getByName(m_materialName));
		//Ogre::GpuProgramParametersSharedPtr shaderParameters = materialPtr->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
		//shaderParameters->setNamedConstant("attribute", Ogre::Vector4(m_WaterRatio, m_BloodRatio, m_CurrHeight, m_StageHeight));
	}
	
}

void WaterPool::UpdateLiquids()
{ 
	//m_TotalVolume = m_WaterVolume + m_BloodVolume; 
	//if(m_TotalVolume > FLT_EPSILON)
	{
		//m_WaterRatio = m_WaterVolume / m_TotalVolume;
		//m_BloodRatio = (1 - m_WaterRatio)*0.9f + 0.1f;
	}
	//else
	{
		//m_WaterRatio = m_BloodRatio = 0.1f;
	}
}

void WaterPool::CalcHybridColor()
{
	if(m_TotalVolume < FLT_EPSILON)
		m_HybridColor.a = 0.0;
	else
	{
		float waterRatio = m_WaterVolume / m_TotalVolume;
		float bloodRatio = 1.0f - waterRatio;
		
		m_HybridColor = m_WaterColor * waterRatio + m_BloodColor * bloodRatio;
	}
}
