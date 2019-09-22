#ifndef _TEXTUREBLOODEFFECT_
#define _TEXTUREBLOODEFFECT_
#include "Ogre.h"
#include "collision/GoPhysCollisionlib.h"
#include "Dynamic/GoPhysDynamicLib.h"
#include "OrganBloodMotionSimulator.h"
class OrganBloodingSystem;
class DynamicBloodParticle;

#define _BLOOD_TRACE_MAP_SIZE_ 350
#define _BLOOD_TRACE_MAP_GRID_SIZE_ (1.0f/_BLOOD_TRACE_MAP_SIZE_)
#define _BLEED_TIME_ 5000

using namespace GoPhys;
class VeinConnectPair;
//class OrganSurfaceBloodTrack
//a track represent the move path history of a blood particle
//
enum _ENUM_BLOOD_MAP_NODE_STATE_
{
	_ENUM_BLOOD_MAP_NODE_STATE_DEFAULT_,
	_ENUM_BLOOD_MAP_NODE_STATE_TRACE_,
	_ENUM_BLOOD_MAP_NODE_STATE_BLEED_,
};
struct SBloodTraceMapNode
{
	SBloodTraceMapNode()
	{
		m_eState = _ENUM_BLOOD_MAP_NODE_STATE_DEFAULT_;
		m_nStartTime = 0xffffffff;
		m_fRadius = 0.0f;
		m_fRotate = 0.0f;
	}
	_ENUM_BLOOD_MAP_NODE_STATE_ m_eState;
	DWORD m_nStartTime;
	Ogre::Vector2 m_vPos;
	float m_fRadius;
	float m_fRotate;
};
class OrganSurfaceBloodTextureTrack
{
	friend class TextureBloodTrackEffect;
public:
	/*
	class PathFollowParticle
	{
	public:
		PathFollowParticle()
		{
			m_CurrPathIndex = 0;
			m_Weight = m_StayTime = 0;
		}
		int   m_CurrPathIndex;
		float m_Weight;
		float m_StayTime;
	};
	*/

	class BloodPathPoint
	{
	public:
		BloodPathPoint()
		{
			m_MovedDist = 0;
			m_MovedTime = 0;
			bleedrotateRadian = 0;
			m_TimeElaspedSinceLastBleed = 1000;//second
		}
		Ogre::Vector2 m_blood;
		Ogre::Vector2 m_bleed;
		float bleedrotateRadian;
		float m_TimeElaspedSinceLastBleed;
		float m_MovedDist;
		float m_MovedTime;
	};
	
	OrganSurfaceBloodTextureTrack(int startFaceID,TextureBloodTrackEffect *pEffect);
	~OrganSurfaceBloodTextureTrack();

	void GetBloodFaceTextureCoordinate(MisMedicOrgan_Ordinary * organ , int faceid , Ogre::Vector2 vertCoord[3]);

	Ogre::Vector3 GetBloodFacePointUndeformedPos(MisMedicOrgan_Ordinary * organ , int faceid , float pointweights[3]);

	//set particle this track will record
	bool SetLeaderParticle(DynamicBloodParticle * par);

	//
	bool Update(float dt);

	bool ScaleBloodDropRadius(float scalefactor);

	void Stop();

	void SetCanScaleBloodDropRadius(bool bCanScale) { m_bCanScaleBloodDropRadius = bCanScale;}
	bool CanScaleBolldDropRadius() {return m_bCanScaleBloodDropRadius;}

	bool BloodTraceMapRecord( BloodPathPoint& record, float fRadius );

	Ogre::Vector3 m_StartPosUndeformedFrame;//in undeformed frame

	//std::vector<PathFollowParticle> m_ParFollower;
	
	std::vector<BloodPathPoint> m_LeaderMovePath;

	float	m_currentActiveBPPIndex;

	bool m_bIsStopBleed;				//是否已经停止流血
	
	OrganBloodingSystem * m_HostSystem;

	DynamicBloodParticle * m_LeaderParticle;//first particle

	float m_BloodStartRadius;//blood radius when begin move

	float m_BloodEndRadius;//blood radius when end move

	float m_RadiusScale;

	float m_PathPointInterval;//the minimum distance between 2 adjacent blood path point

	Ogre::ColourValue m_bloodcolor;

	Ogre::Vector2 m_locationInTexture;

	float m_scorchTransparent;

	int   m_BloodTrackId;

	int   m_StartFaceID;
	bool  m_NeedReRender;

	bool m_bIsDynamicBloodStream;	//动态流血效果，默认false

	float m_elapseTime;

	DWORD m_nStartTime;

	TextureBloodTrackEffect* m_pEffect;

	bool m_bIsWater;

	DWORD m_nWaterDisapearTime;//水流开始消失的时刻

private:
	bool m_bCanScaleBloodDropRadius;//决定调用ScaleBloodDropRadius函数是否有效，default : true
};

//blood track in vein connect
class VeinConnectBloodTextureTrack
{
public:
	VeinConnectBloodTextureTrack(const Ogre::Vector2 & startTexCoor, const Ogre::Vector2 & endTexCoor);

	//data
	//VeinConnectObject * m_veinconnectobj;
	//int   m_clusterID;
	//int   m_PairID;
	Ogre::Vector2  m_StartTexCoord;
	Ogre::Vector2  m_EndTexCoord;

	float m_PositionWeight;
	bool  m_randtangent ;
	bool  m_isactive;
	float m_currslipspeed;
	float m_slipaccelerate;
	float m_maxslipspeed;
	float m_AplhaFade;
	//

	unsigned int id;

	//BloodInVeinConnect m_VeinBloodHead;

	//float m_bloodquanity;

	//float m_startquantity;

	//float m_stopquanity;

	//float m_blooddecpermove;

	float m_blooddensity;

	float m_bloodstartradius;

	float m_bloodendradius;

	unsigned char fadeoutalpha;

	unsigned char initalpha;

	Ogre::ColourValue bloodcolor;
};

//
//simple blood effusion
class OrganDynEffusionTextureBlood
{
public:
	OrganDynEffusionTextureBlood()
	{
		m_opcacity = 0;
	}
	Ogre::Vector2 m_texturecoord;
	float m_opcacity;
	float m_radius;
};

//
class DynamicBloodPoint
{
	friend class TextureBloodTrackEffect;
public:
	DynamicBloodPoint(const GFPhysSoftBodyFace * pFace,const float weights[3],unsigned int numParticle = 1000);
	~DynamicBloodPoint();
	
	void Update(float dt);
private:
	void InitManualObject();
	void Draw();

	const GFPhysSoftBodyFace * m_pFace;
	float m_weights[3];
	float m_emisstionRate;
	Ogre::Vector3 m_position;
	Ogre::Vector3 m_up;
	Ogre::ColourValue m_changedColor;
	

	std::list<DynamicBloodPointParticle*> m_liveParticles;
	std::list<DynamicBloodPointParticle*> m_deadParticles;
	Ogre::ManualObject * m_pManualObject;
};

class TextureBloodTrackEffect : public OrganBloodMotionSimulator::Listener
{
public:
	TextureBloodTrackEffect(MisMedicOrganInterface * organif);

	~TextureBloodTrackEffect();

	void SetBloodSystem(OrganBloodMotionSimulator * bloodsys);

	void SetBloodTexture(const Ogre::String & bloodtexture);

	void SetBloodColor(Ogre::ColourValue colorvalue);

	const std::vector<OrganSurfaceBloodTextureTrack*> & GetAllBloodTrack();

	OrganSurfaceBloodTextureTrack *  GetNearestTextureBloodTrackIndex(const Ogre::Vector2 & srclocate , Ogre::Vector2 & dstlocate);

	//获取最新的一条流血,失败返回NULL
	OrganSurfaceBloodTextureTrack * GetLatestTextureBloodTrack();

	OrganSurfaceBloodTextureTrack * createBloodTrackInRoughFace(MisMedicOrgan_Ordinary * oragn , GFPhysSoftBodyFace * faceRough , const float Roughweights[3] , float radius = 0.0027f);

	OrganSurfaceBloodTextureTrack * createBloodTrack(MisMedicOrgan_Ordinary * organ , 
		float weights[3] , 
		float mass  , 
		int initfaceid, 
		float radius/* = 0.0027f*/, 
		float BloodSlipAccelrate, float BloodMaxSlipVel,
		float fDir = 0.0f);

	OrganSurfaceBloodTextureTrack * createBloodTrackUseForOrganBeCutFace(MisMedicOrgan_Ordinary * organ , 
		float weights[3] ,  
		float mass  , 
		int initfaceid, 
		float radius,
		float BloodSlipAccelrate, float BloodMaxSlipVel
		);
	
	int  StopFirstBloodTrack();

	void StopBloodsTooOlder(float timeseconds);

	//移除指定纹理坐标附近的动态流血效果
	void RemoveBloodTrack(const std::vector<Ogre::Vector2> & texCoords,std::vector<int> &removeTrackIDs ,float range = 0.0035f);

	int GetNumOfDynamicBlood();

	VeinConnectBloodTextureTrack * CreateBloodTrackForVeinConnect(VeinConnectObject * stripobj , const VeinConnectPair & coonpair , float initweight);

	bool createBloodEffusionPoint(Ogre::Vector2 texturecoord , float radius);

	bool BuildBloodStreamBleed(Ogre::ManualObject * bloodQuads);

	bool BuildBloodStreamBleed1(Ogre::ManualObject * bloodQuads);

	bool BuildBloodStream(Ogre::ManualObject * bloodQuads, bool& bNeedBleed);

	bool BuildDynamicBloodPoints(Ogre::ManualObject * pManualObject);

	void ScaleStreamBloodDropRadius(float scalefactor , int indexstreamindex);

	bool Update(float dt , std::vector<int> &removeTrackIDs);

	void UpdateVeinConnectBloodTrack(float dt);//(Ogre::TexturePtr bloodtexture , float deltatime);

	bool BuildVeinBlood(Ogre::ManualObject * bloodQuads);

	//创建一个冒血点
	DynamicBloodPoint * CreateDynamicBloodPoint(const GFPhysSoftBodyFace * pFace,const float weights[3]);
	//移除一个冒血点
	bool RemoveDynamicBloodPoint(const GFPhysSoftBodyFace * pFace);

	SBloodTraceMapNode* GetBloodTraceMapNode( int nX, int nY );

	bool AddBloodTraceMapNode( int nX, int nY, SBloodTraceMapNode& record );

	MisMedicOrganInterface * m_OrganIf;

protected:
	//overridden
	void onParticleRemoved(DynamicBloodParticle*parremove);

	void onParticleCreated(DynamicBloodParticle *);

	void onSimulatorDestoryed();

	std::vector<OrganSurfaceBloodTextureTrack*> m_OrganSurfaceBloodStreams;

	std::vector<VeinConnectBloodTextureTrack*>  m_VeinConnectBloodTracks;

	std::vector<OrganDynEffusionTextureBlood> m_BloodEffusionPoints;

	std::vector<DynamicBloodPoint*> m_dynamicBloodPoints;

	OrganBloodMotionSimulator * m_bloodsys;

	Ogre::MaterialPtr m_dynamicBloodBuildMat;

	Ogre::ColourValue m_dynamicBloodValue;

	int m_nBloodSpeed;

	//bool m_NeedReRender;
protected:
	
	void GetClosetPointInRefinedFaces(GFPhysSoftBodyFace * roughface , const float roughweights[3] ,  int & finefaceid , float fineweights[3]);
	float m_AccmulateTime;
	float m_bloodFlowVelocity;	// for stopping bleeding only
	std::map<int,SBloodTraceMapNode> m_mapBloodTraceMap;
};
#endif