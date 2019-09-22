#pragma once
#include "MXOgreWrapper.h"
#include "Ogre.h"
#include "Dynamic/GoPhysDynamicLib.h"
#include "shadowmap.h"
#include "MisMedicOrganInterface.h"

using namespace GoPhys;
class MisNewTraining;
struct BloodTrack
{
	BloodTrack(int organId , int trackId) : OrganId(organId) , TrackId(trackId){}
	int OrganId;
	int TrackId;
};

class WaterPool  : public CShadowMap::ShadowRendListener , public OrganActionListener
{
public:
	WaterPool(MisNewTraining * ownerTrain,
		      float meshWidth , float meshHeight , 
						float actualWidth , float actualHeight , 
						const Ogre::Vector3 & origin,
						const Ogre::Vector3 & normal , 
						float maxHeight , 
						const Ogre::Vector3 & CenterAndStageHeight ,
						bool IsCenterChangeBySuction,
						const Ogre::String& meshName = "");
	~WaterPool();

	//@overridden shadow map listener
	virtual void preRenderShadowDepth();
	virtual void postRenderShadowDepth();

	//@overridden OrganActionListener
	virtual void OnOrganBleeding(int organId , int trackId);

	virtual void OnOrganStopBleeding(int organId , int trackId);

	void SetRejectOrgan(const std::set<int> & rejectOrganIDs);

	bool IsReject(int organId);

	void Update(float dt);

	void AddWater(float volume);

	void AddBlood(float volume);

	void Reduce(float volume , const Ogre::Vector2 & planePos = Ogre::Vector2(0.5,0.5));

	bool IsEmpty() { return m_CurrHeight < FLT_EPSILON; }

	bool IsOnePointInWaterPool(const Ogre::Vector3 & point , Ogre::Vector2 & pos);

	bool IsSegementInsectWaterPool(const Ogre::Vector3 & point0, const Ogre::Vector3 & point1);

	float GetCurrHeight() { return m_CurrHeight;}

	void SetHeight(float height) { m_CurrHeight = height; }
	Ogre::Vector3 GetCurOrigin(){ return m_Origin + m_CurrHeight * m_Normal + m_ExtOffset; }

	Ogre::Vector3 GetPlaneNormal(){ return m_Normal; }

	void SetReferencNode(GFPhysSoftBodyNode * node)
	{
		m_RefNode = node;
	}

	void SetCanSoakOrgans(bool soakable)
	{
		m_IsSoakOrgans = soakable;
	}
private:	

	//bool m_IsWaterHeightChanged;

	float m_TimeSinceLast;

	void DrawWaterPlane(bool isDrawTwo);

	void UpdateWaterPlane();

	void UpdateLiquids();

	void CalcHybridColor();

	static int sNumOfWaterPool;

	MisNewTraining * m_OwnerTrain;

	std::set<int> m_RejectOrganIDs;

	std::vector<BloodTrack> m_BloodTracks;

	//Ogre::SceneNode *m_pWaterNode;

	Ogre::ManualObject *m_pWaterPlane;

	Ogre::Vector3 m_Origin;

	Ogre::Vector3 m_Normal;

	Ogre::Vector3 m_Axis0;

	Ogre::Vector3 m_Axis1;

	Ogre::Vector3 m_ExtOffset;

	float m_MeshWidth;

	float m_MeshHeight;

	Ogre::ColourValue m_WaterColor;

	Ogre::ColourValue m_BloodColor;

	Ogre::ColourValue m_HybridColor;

	float m_ActualBottomArea;

	float m_CurrHeight;

	float m_StageHeight;

	Ogre::Vector2 m_OriginCenter;

	float m_MaxHeight;

	float m_MaxVolume;

	float m_WaterVolume;

	float m_BloodVolume;

	float m_TotalVolume;

	float m_BloodRatio;

	float m_WaterRatio;

	bool m_IsCenterChangeBySuction;

	bool m_IsSoakOrgans;

	Ogre::Vector3 m_planeInitPosition;
	Ogre::SceneNode* m_planeNode;
	Ogre::Entity* m_planeEntity;

	Ogre::String m_materialName;

	GFPhysSoftBodyNode * m_RefNode;

	std::vector<Ogre::Vector3> m_vertices;
	std::vector<unsigned int>  m_indices;
};