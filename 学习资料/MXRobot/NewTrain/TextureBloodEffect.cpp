#include "math/GophysTransformUtil.h"
#include "TextureBloodEffect.h"
#include "VeinConnectObject.h"
#include "MisMedicOrganOrdinary.h"
#include "DynamicObjectRenderable.h"
#include "MXOgreGraphic.h"
#include "MXEventsDump.h"
#include "MXToolEvent.h"
#include <cstdlib>
VeinConnectBloodTextureTrack::VeinConnectBloodTextureTrack(const Ogre::Vector2 & startTexCoor, const Ogre::Vector2 & endTexCoor)
{
	m_isactive = false;
	m_randtangent = false;
	m_PositionWeight = 0.0f;
	m_currslipspeed = 0.01f;
	m_slipaccelerate = 0.06f;
	m_maxslipspeed = 0.08f;
	//m_veinconnectobj = 0;

	fadeoutalpha = 180;
	initalpha = 255;
	//m_startquantity = 100;
	//m_bloodquanity = 2000;
	//m_blooddecpermove = 1;
	//m_stopquanity = 20;
	m_bloodstartradius = 3;
	m_bloodendradius = 2;
	m_AplhaFade = 1.0f;
	m_StartTexCoord = startTexCoor;
	m_EndTexCoord = endTexCoor;
}
//====================================================================================================
OrganSurfaceBloodTextureTrack::OrganSurfaceBloodTextureTrack(int startFaceID, TextureBloodTrackEffect* pEffect)
{
	static int s_bloodtrackid = 0;

	m_StartFaceID = startFaceID;

	m_BloodStartRadius = 0.0027f;//3.0f;

	m_BloodEndRadius = m_BloodStartRadius;

	m_RadiusScale = 1.0f;

	m_LeaderParticle = 0;

	m_PathPointInterval = 0;//m_BloodStartRadius / 5.0f;

	m_bloodcolor = Ogre::ColourValue(1.0f , 1.0f , 1.0f , 1);

	m_scorchTransparent = 0;

	m_BloodTrackId = s_bloodtrackid;

	m_NeedReRender = true;

	s_bloodtrackid++;

	m_currentActiveBPPIndex = -1.0f;

	m_bIsDynamicBloodStream = false;

	m_elapseTime = 0.0f;

	m_nStartTime = ::GetTickCount();

	m_nWaterDisapearTime = m_nStartTime+3000;//默认水流3秒后消失

	m_bIsStopBleed = false;

	m_bCanScaleBloodDropRadius = true;

	m_pEffect = pEffect;

	m_bIsWater = false;
}
//============================================================================================================
OrganSurfaceBloodTextureTrack::~OrganSurfaceBloodTextureTrack()
{
	if(!m_bIsWater)
	{
		//血流被移除
		MxToolEvent * pEvent = CMXEventsDump::Instance()->CreateEventNew(MxEvent::MXET_BleedRemove,NULL,this);
		pEvent->m_UserData = m_StartFaceID;
		CMXEventsDump::Instance()->PushEvent(pEvent,true);
	}
}
//============================================================================================================
bool OrganSurfaceBloodTextureTrack::SetLeaderParticle(DynamicBloodParticle * particle)
{
	if(m_LeaderParticle != 0)
		return false;

	m_LeaderParticle = particle;

	m_LeaderParticle->m_Startradius = m_BloodStartRadius;

	Ogre::Vector2 textureCoords[3];

	BloodMoveData * parmovedata = (BloodMoveData *)particle->m_usedata;

	MisMedicOrgan_Ordinary * organ = parmovedata->m_stickedOrgan;

	int StickFaceId = parmovedata->m_stickedfaceid;

	if(StickFaceId >= organ->m_OriginFaces.size())
		return false;

	MMO_Face face = organ->m_OriginFaces[StickFaceId];
	if(face.m_physface == 0)
		return false;

	GetBloodFaceTextureCoordinate(parmovedata->m_stickedOrgan , parmovedata->m_stickedfaceid , textureCoords);

	m_StartPosUndeformedFrame = GetBloodFacePointUndeformedPos(parmovedata->m_stickedOrgan , parmovedata->m_stickedfaceid , parmovedata->m_positionweights);

	m_locationInTexture = textureCoords[0]*parmovedata->m_positionweights[0] 
	+textureCoords[1]*parmovedata->m_positionweights[1] 
	+textureCoords[2]*parmovedata->m_positionweights[2];

	m_LeaderMovePath.clear();

	OrganSurfaceBloodTextureTrack::BloodPathPoint record;

	record.m_blood = record.m_bleed = m_locationInTexture;//m_LeaderParticle->m_TextureUV;

	record.bleedrotateRadian = 3.141592f*2.0f * float(rand() % 5) / 5.0f;

	m_LeaderMovePath.push_back(record);
	BloodTraceMapRecord(record,m_BloodStartRadius);

	m_NeedReRender = true;

	return true;
}
//===================================================================================================================
bool OrganSurfaceBloodTextureTrack::ScaleBloodDropRadius(float scalefactor)
{
	if(m_bCanScaleBloodDropRadius)
	{
		if (m_RadiusScale < 0.75f)
		{
			m_currentActiveBPPIndex = std::max(1.1f, m_currentActiveBPPIndex);

			if(m_bIsStopBleed == false)
			{
			   m_bIsStopBleed = true;
			   
			   //产生流血停止事件
			   MxToolEvent * pEvent = CMXEventsDump::Instance()->CreateEventNew(MxEvent::MXET_BleedEnd,NULL,this);
			   pEvent->m_UserData = m_StartFaceID;
			   CMXEventsDump::Instance()->PushEvent(pEvent,true);
			}
		}
		else
		{
			m_RadiusScale *= scalefactor;
		}
		//m_NeedReRender = true;
	}
	return m_bIsStopBleed;
}
//=========================================================================================================================
void OrganSurfaceBloodTextureTrack::Stop()
{
	//used for nonwound
	//m_RadiusScale *= 0.6;
	//ScaleBloodDropRadius(0.6);
	if(m_bIsStopBleed == false)
	{
		m_currentActiveBPPIndex = std::max(1.1f, m_currentActiveBPPIndex);
		
		m_bIsStopBleed = true;
		
		//产生流血停止事件
		MxToolEvent * pEvent = CMXEventsDump::Instance()->CreateEventNew(MxEvent::MXET_BleedEnd,NULL,this);
		pEvent->m_UserData = m_StartFaceID;
		CMXEventsDump::Instance()->PushEvent(pEvent,true);
	}
}
//=========================================================================================================================
bool OrganSurfaceBloodTextureTrack::BloodTraceMapRecord( BloodPathPoint& record, float fRadius )
{
	return true;
	float tu = record.m_blood.x-0.5f;
	float tv = 0.5f-record.m_blood.y;
	int nX = -tu/1.0f*_BLOOD_TRACE_MAP_SIZE_;
	int nY = -tv/1.0f*_BLOOD_TRACE_MAP_SIZE_;
	if( nX<0 )
		nX+=_BLOOD_TRACE_MAP_SIZE_;
	if( nY<0 ) 
		nY+=_BLOOD_TRACE_MAP_SIZE_; 
	SBloodTraceMapNode* pNode = m_pEffect->GetBloodTraceMapNode( nX, nY );
	if( pNode == NULL )
	{
		SBloodTraceMapNode node;
		node.m_eState = _ENUM_BLOOD_MAP_NODE_STATE_TRACE_;
		node.m_fRadius = fRadius;
		node.m_nStartTime = ::GetTickCount();
		node.m_vPos = Ogre::Vector2(tu,tv);
		node.m_fRotate = 0.0f;
		m_pEffect->AddBloodTraceMapNode( nX, nY, node );
		return true;
	}
	else
	{
		pNode->m_nStartTime = ::GetTickCount()-static_cast<DWORD>(_BLEED_TIME_*(fRadius-pNode->m_fRadius)/fRadius);
		pNode->m_fRadius = fRadius;
	}
	return true;
}

//=========================================================================================================================
void OrganSurfaceBloodTextureTrack::GetBloodFaceTextureCoordinate(MisMedicOrgan_Ordinary * organ , int faceid , Ogre::Vector2 vertCoord[3])
{
	if(organ->m_FFDObject)
	{
		std::vector<DynamicSurfaceFreeDeformObject::RendFaceData> & rendFaces = organ->m_FFDObject->m_rendfaces;

		std::vector<DynamicSurfaceFreeDeformObject::ManipulatedVertex> & maniVertex = organ->m_FFDObject->m_manivertex;

		DynamicSurfaceFreeDeformObject::RendFaceData & rendFace = rendFaces[faceid];

		vertCoord[0]  = rendFace.texcoord[0];

		vertCoord[1]  = rendFace.texcoord[1];

		vertCoord[2]  = rendFace.texcoord[2];
	}
	else
	{	
		MMO_Face face = organ->m_OriginFaces[faceid];

		vertCoord[0] = face.GetTextureCoord(0);

		vertCoord[1] = face.GetTextureCoord(1);

		vertCoord[2] = face.GetTextureCoord(2);
	}
}
Ogre::Vector3 OrganSurfaceBloodTextureTrack::GetBloodFacePointUndeformedPos(MisMedicOrgan_Ordinary * organ , int faceid , float pointweights[3])
{
	if(organ->m_FFDObject)
	{
		std::vector<DynamicSurfaceFreeDeformObject::RendFaceData> & rendFaces = organ->m_FFDObject->m_rendfaces;

		std::vector<DynamicSurfaceFreeDeformObject::ManipulatedVertex> & maniVertex = organ->m_FFDObject->m_manivertex;

		DynamicSurfaceFreeDeformObject::RendFaceData & rendFace = rendFaces[faceid];

		Ogre::Vector3 undefpos0 = maniVertex[rendFace.manivertindex[0]].m_UnDeformedPos;

		Ogre::Vector3 undefpos1 = maniVertex[rendFace.manivertindex[1]].m_UnDeformedPos;

		Ogre::Vector3 undefpos2 = maniVertex[rendFace.manivertindex[2]].m_UnDeformedPos;

		return undefpos0*pointweights[0]+undefpos1*pointweights[1]+undefpos2*pointweights[2];
	}
	else
	{	
		MMO_Face face = organ->m_OriginFaces[faceid];

		GFPhysSoftBodyFace * physface = face.m_physface;
		GFPhysVector3 temp = physface->m_Nodes[0]->m_UnDeformedPos*pointweights[0]
		+physface->m_Nodes[1]->m_UnDeformedPos*pointweights[1]
		+physface->m_Nodes[2]->m_UnDeformedPos*pointweights[2];

		return Ogre::Vector3(temp.x() , temp.y() , temp.z());
	}
}
//============================================================================================================
bool OrganSurfaceBloodTextureTrack::Update(float dt)
{
	if(m_currentActiveBPPIndex >= m_LeaderMovePath.size())
	{
		return false;//dead blood track
	}

	m_elapseTime += dt;
	if(m_bIsDynamicBloodStream && m_LeaderMovePath.size())
	{
		if(m_LeaderMovePath.size() != 1)
			m_LeaderMovePath.resize(1);
		float r = 0.00015f;
		m_BloodStartRadius += r * sin(m_elapseTime * 100);
		m_NeedReRender = true;
		return true;
	}

	if(m_LeaderParticle)
	{
		//calculate uv coordinate of this particle
		Ogre::Vector2 textureCoords[3];

		BloodMoveData & partdata = *((BloodMoveData*)m_LeaderParticle->m_usedata);

		GetBloodFaceTextureCoordinate(m_LeaderParticle->m_Organ , partdata.m_stickedfaceid , textureCoords);

		Ogre::Vector2 textureUV = textureCoords[0]*partdata.m_positionweights[0] 
		+textureCoords[1]*partdata.m_positionweights[1] 
		+textureCoords[2]*partdata.m_positionweights[2];

		//
		BloodPathPoint record;

		//record leader particle's texture path
		record.m_blood = record.m_bleed = textureUV;//m_LeaderParticle->m_TextureUV;

		//rotate the bleed texture to fake an random blood bleed effect
		record.bleedrotateRadian = (m_LeaderMovePath.size() == 0 ? 0 : m_LeaderMovePath[m_LeaderMovePath.size()-1].bleedrotateRadian);

		record.bleedrotateRadian += dt*(3.141592f*2.5f); //  record.radiusscale = 0.1f;//1.0f;

		if(m_LeaderMovePath.size() > 0)
		{
			BloodPathPoint & lastrecord = m_LeaderMovePath[m_LeaderMovePath.size()-1];

			float DeltaDist = (lastrecord.m_blood-record.m_blood).length();

			lastrecord.m_MovedDist += DeltaDist;

			lastrecord.m_MovedTime += dt;

			if(lastrecord.m_MovedDist > m_PathPointInterval)
			{
				m_LeaderMovePath.push_back(record);
				BloodTraceMapRecord(record,m_BloodStartRadius);
			}
		}
		else
		{
			m_LeaderMovePath.push_back(record);
			BloodTraceMapRecord(record,m_BloodStartRadius);
		}

		m_NeedReRender = true;
	}
	return true;
}
//==================================================================================================================================
//DynamicBloodPoint
DynamicBloodPoint::DynamicBloodPoint(const GFPhysSoftBodyFace * pFace,const float weights[3],unsigned int numParticle)
:m_pFace(pFace),
m_changedColor(0.2967,0.2968,0.7465,0)
{
	assert(pFace);
	m_emisstionRate = 433.2f;

	Ogre::Vector3 v10 = GPVec3ToOgre(pFace->m_Nodes[1]->m_UnDeformedPos  - pFace->m_Nodes[0]->m_UnDeformedPos);
	Ogre::Vector3 v12 = GPVec3ToOgre(pFace->m_Nodes[1]->m_UnDeformedPos  - pFace->m_Nodes[2]->m_UnDeformedPos);
	m_up = v12.crossProduct(v10);
	m_up.normalise();

	memcpy(m_weights,weights,sizeof(float) * 3);

	if(numParticle > 1000)
		numParticle = 1000;
	while(numParticle--)
	{
		DynamicBloodPointParticle * pParticle = new DynamicBloodPointParticle;
		m_deadParticles.push_back(pParticle);
	}
	InitManualObject();
}

DynamicBloodPoint::~DynamicBloodPoint()
{
	std::list<DynamicBloodPointParticle*>::iterator itr;
	for(itr = m_liveParticles.begin();itr != m_liveParticles.end();++itr)
		delete *itr;
	m_liveParticles.clear();

	for(itr = m_deadParticles.begin();itr != m_deadParticles.end();++itr)
		delete *itr;
	m_deadParticles.clear();
	
	m_pManualObject->detachFromParent();
	MXOgreWrapper::Get()->GetDefaultSceneManger()->destroyManualObject(m_pManualObject);
}

void DynamicBloodPoint::InitManualObject()
{
	m_pManualObject = MXOgreWrapper::Get()->GetDefaultSceneManger()->createManualObject();
	MXOgreWrapper::Get()->GetDefaultSceneManger()->getRootSceneNode()->attachObject(m_pManualObject);

	m_pManualObject->setDynamic(true);
	m_pManualObject->estimateIndexCount(3000);
	m_pManualObject->estimateVertexCount(1000);

	m_pManualObject->begin("Particles/BloodFlow");
	m_pManualObject->position(0,0,0);
	m_pManualObject->textureCoord(0,0);
	m_pManualObject->colour(0,0,0);

	m_pManualObject->position(0,0,0);
	m_pManualObject->textureCoord(0,0);
	m_pManualObject->colour(0,0,0);

	m_pManualObject->position(0,0,0);
	m_pManualObject->textureCoord(0,0);
	m_pManualObject->colour(0,0,0);

	m_pManualObject->position(0,0,0);
	m_pManualObject->textureCoord(0,0);
	m_pManualObject->colour(0,0,0);

	m_pManualObject->index(0);
	m_pManualObject->index(1);
	m_pManualObject->index(2);

	m_pManualObject->index(0);
	m_pManualObject->index(2);
	m_pManualObject->index(3);

	m_pManualObject->end();
}

void DynamicBloodPoint::Draw()
{
	return ;
	if(m_liveParticles.size() == 0)
		return ;

	m_pManualObject->beginUpdate(0);
	int index = 0;
	Ogre::Vector3 width,height;
	for(std::list<DynamicBloodPointParticle*>::iterator itr = m_liveParticles.begin(); itr != m_liveParticles.end();++itr)
	{
		DynamicBloodPointParticle * pParticle = *itr;
		Ogre::Vector3 & binormal = pParticle->m_binormal;
		Ogre::Vector3 & tangent = pParticle->m_tangent;

		width = tangent * 0.13;
		height = binormal * 0.13;
		
		m_pManualObject->position(pParticle->m_position - width + height);
		m_pManualObject->colour(pParticle->m_color);
		m_pManualObject->textureCoord(0,1);

		m_pManualObject->position(pParticle->m_position + width + height);
		m_pManualObject->colour(pParticle->m_color);
		m_pManualObject->textureCoord(1,1);

		m_pManualObject->position(pParticle->m_position - width - height);
		m_pManualObject->colour(pParticle->m_color);
		m_pManualObject->textureCoord(0,0);

		m_pManualObject->position(pParticle->m_position + width - height);
		m_pManualObject->colour(pParticle->m_color);
		m_pManualObject->textureCoord(1,0);

		index++;
	}

	for(int i = 0;i<index;++i)
	{
		m_pManualObject->index(i * 4);
		m_pManualObject->index(i * 4 +1);
		m_pManualObject->index(i * 4 + 2);

		m_pManualObject->index(i * 4 + 1);
		m_pManualObject->index(i * 4 + 3);
		m_pManualObject->index(i * 4 + 2);
	}
	m_pManualObject->end();
}

void DynamicBloodPoint::Update(float dt)
{
	std::list<DynamicBloodPointParticle*>::iterator itr;
	DynamicBloodPointParticle * pParticle;


	GFPhysVector3 v01 = m_pFace->m_Nodes[1]->m_CurrPosition - m_pFace->m_Nodes[0]->m_CurrPosition;
	GFPhysVector3 v02 = m_pFace->m_Nodes[2]->m_CurrPosition - m_pFace->m_Nodes[0]->m_CurrPosition;
	m_up = GPVec3ToOgre(v01.Cross(v02)).normalisedCopy();

	Ogre::Vector3 tempPosition = GPVec3ToOgre(m_pFace->m_Nodes[0]->m_CurrPosition * m_weights[0] + m_pFace->m_Nodes[1]->m_CurrPosition * m_weights[1] + m_pFace->m_Nodes[2]->m_CurrPosition * m_weights[2]);
	tempPosition += 0.08 * m_up;

	//1 gen new particle
	int curNeedNumParticle = dt * m_emisstionRate;

	if((int)m_deadParticles.size() < curNeedNumParticle)
	{
		int addNumParticle =curNeedNumParticle;
		while(addNumParticle--)
		{
			DynamicBloodPointParticle * pParticle = new DynamicBloodPointParticle;
			m_deadParticles.push_back(pParticle);
		}
	}


	while(curNeedNumParticle-- && m_deadParticles.size())
	{
		pParticle = *(m_deadParticles.begin());

		pParticle->m_position = tempPosition;
		pParticle->m_directionAngle = Ogre::Math::UnitRandom() * Ogre::Math::TWO_PI;
		pParticle->m_velocity = .15f * Ogre::Math::UnitRandom();
		pParticle->m_curLiveTime = 0.f;
		pParticle->m_totalLiveTime = Ogre::Math::UnitRandom() * 2.f;		
		pParticle->m_color = Ogre::ColourValue();

		m_deadParticles.erase(m_deadParticles.begin());
		m_liveParticles.push_back(pParticle);
	}

	//2 update live particles
	Ogre::Vector3 xAxis = GPVec3ToOgre(v01).normalisedCopy();
	Ogre::Vector3 direction;
	Ogre::Vector3 velocity;
	Ogre::Quaternion q;

	for(itr = m_liveParticles.begin();itr != m_liveParticles.end();)
	{
		pParticle = *itr;
		pParticle->m_curLiveTime += dt;
		if(pParticle->m_curLiveTime > pParticle->m_totalLiveTime)
		{
			m_deadParticles.push_back(*itr);
			itr = m_liveParticles.erase(itr);
		}
		else
		{
			q.FromAngleAxis(pParticle->m_directionAngle,m_up);
			direction = q * xAxis;		//direction already normalize
			velocity = direction * pParticle->m_velocity;

			pParticle->m_position = tempPosition + velocity * pParticle->m_curLiveTime;

			pParticle->m_tangent = velocity ;// + -m_up *  pParticle->m_curLiveTime;
			pParticle->m_tangent.normalise();
			pParticle->m_binormal = pParticle->m_tangent.crossProduct(m_up);
			pParticle->m_binormal.normalise();

			++itr;
		}
	}
	
	Draw();
}


//==================================================================================================================================
TextureBloodTrackEffect::TextureBloodTrackEffect(MisMedicOrganInterface * organif)
{
	m_dynamicBloodValue = Ogre::ColourValue(0.8f,0.8f,0.8f,1);

	Ogre::MaterialPtr srcmat = Ogre::MaterialManager::getSingleton().getByName("OrganSurfEffect/BloodDropStream");

	static int countbuild = 0;

	countbuild++;

	m_dynamicBloodBuildMat = srcmat->clone("dynbloodbuild"+Ogre::StringConverter::toString(countbuild));

	m_bloodsys = 0;

	m_AccmulateTime = 0;

	m_OrganIf = organif;

	//m_NeedReRender = true;
	MisMedicDynObjConstructInfo & cs = organif->GetCreateInfo();
	m_bloodFlowVelocity = cs.m_BloodFlowVelocity;

	m_nBloodSpeed = 80;
}
//==================================================================================================================================
TextureBloodTrackEffect::~TextureBloodTrackEffect()
{
	if(m_bloodsys)
	{
		m_bloodsys->RemoveListener(this);
	}

	//delete all organ stream
	for(size_t b = 0 ; b < m_OrganSurfaceBloodStreams.size() ; b++)
	{
		delete m_OrganSurfaceBloodStreams[b];
	}
	m_OrganSurfaceBloodStreams.clear();

	//delete all connect stream
	for(size_t b = 0 ; b < m_VeinConnectBloodTracks.size() ; b++)
	{
		delete m_VeinConnectBloodTracks[b];
	}
	m_VeinConnectBloodTracks.clear();

	//delete all dynamic blood point
	for(std::size_t p = 0;p<m_dynamicBloodPoints.size();++p)
		delete m_dynamicBloodPoints[p];
	m_dynamicBloodPoints.clear();
}
//=================================================================================================================
void TextureBloodTrackEffect::SetBloodTexture(const Ogre::String & bloodtexture)
{
	Ogre::TexturePtr  srctex = Ogre::TextureManager::getSingleton().load(bloodtexture , Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	/*MisMedicEffectRender::*/ApplyTextureToMaterial(m_dynamicBloodBuildMat , srctex , "BloodMap");
	m_dynamicBloodBuildMat->load();
}

//=================================================================================================================
void TextureBloodTrackEffect::SetBloodColor(Ogre::ColourValue colorvalue)
{
	m_dynamicBloodValue = colorvalue;
}
//=================================================================================================================
const std::vector<OrganSurfaceBloodTextureTrack*> & TextureBloodTrackEffect::GetAllBloodTrack()
{
	return m_OrganSurfaceBloodStreams;
}
//==============================================================================================
OrganSurfaceBloodTextureTrack * TextureBloodTrackEffect::GetNearestTextureBloodTrackIndex(const Ogre::Vector2 & srclocate , Ogre::Vector2 & dstlocate)
{
	float minlength = FLT_MAX;

	OrganSurfaceBloodTextureTrack *   nearestrack = 0;

	for(size_t s = 0 ; s < m_OrganSurfaceBloodStreams.size() ; s++)
	{
		OrganSurfaceBloodTextureTrack * bs = m_OrganSurfaceBloodStreams[s];

		float length = (bs->m_locationInTexture-srclocate).length();

		if(length < minlength)
		{
			minlength = length;
			nearestrack = bs;
			dstlocate = bs->m_locationInTexture;
		}
	}
	return nearestrack;
}
//====================================================================================================
OrganSurfaceBloodTextureTrack * TextureBloodTrackEffect::GetLatestTextureBloodTrack()
{
	std::size_t size = m_OrganSurfaceBloodStreams.size();
	if(size)
		return m_OrganSurfaceBloodStreams[size -1];
	return NULL;
}
//====================================================================================================
void TextureBloodTrackEffect::onParticleRemoved(DynamicBloodParticle*parremove)
{
	for(size_t s = 0 ; s < m_OrganSurfaceBloodStreams.size() ; s++)
	{
		if(parremove == m_OrganSurfaceBloodStreams[s]->m_LeaderParticle)
		{
			if(m_OrganSurfaceBloodStreams[s]->m_LeaderMovePath.size() > 1)
			{
				OrganSurfaceBloodTextureTrack::BloodPathPoint record;

				BloodMoveData * parmovedata = (BloodMoveData *)parremove->m_usedata;

				Ogre::Vector2 textureCoords[3];

				m_OrganSurfaceBloodStreams[s]->GetBloodFaceTextureCoordinate(parmovedata->m_stickedOrgan , parmovedata->m_stickedfaceid , textureCoords);

				Ogre::Vector2 texturecoord = textureCoords[0]*parmovedata->m_positionweights[0] 
				+textureCoords[1]*parmovedata->m_positionweights[1] 
				+textureCoords[2]*parmovedata->m_positionweights[2];

				record.m_blood = record.m_bleed = texturecoord;

				record.bleedrotateRadian = 0;

				m_OrganSurfaceBloodStreams[s]->m_LeaderMovePath.push_back(record);
				m_OrganSurfaceBloodStreams[s]->BloodTraceMapRecord(record,m_OrganSurfaceBloodStreams[s]->m_BloodStartRadius);

			}

			m_OrganSurfaceBloodStreams[s]->m_LeaderParticle = 0;
		}
	} 
}
//==================================================================================================================================
void TextureBloodTrackEffect::onParticleCreated(DynamicBloodParticle *)
{

}
void TextureBloodTrackEffect::onSimulatorDestoryed()
{
	m_bloodsys = 0;
}
//==================================================================================================================================
void TextureBloodTrackEffect::SetBloodSystem(OrganBloodMotionSimulator * bloodsys)
{
	m_bloodsys = bloodsys;
	m_bloodsys->AddListener(this);
}
//==================================================================================================================================
void TextureBloodTrackEffect::GetClosetPointInRefinedFaces(GFPhysSoftBodyFace * roughface , const float roughweights[3] , int & finefaceid , float fineweights[3])
{
	GFPhysVector3 pointInRoughFace = roughface->m_Nodes[0]->m_UnDeformedPos * roughweights[0]
	+roughface->m_Nodes[1]->m_UnDeformedPos * roughweights[1]
	+roughface->m_Nodes[2]->m_UnDeformedPos * roughweights[2];
	float mindist = FLT_MAX;

	int   minfaceIndex = -1;

	GFPhysVector3 minPoint;

	//iterator of all face
	DynamicSurfaceFreeDeformObject * FFDObject = m_bloodsys->GetHostOrgan()->m_FFDObject;

	for(size_t f = 0 ; f < FFDObject->m_rendfaces.size() ; f++)
	{
		const DynamicSurfaceFreeDeformObject::RendFaceData & rendface = FFDObject->m_rendfaces[f];

		Ogre::Vector3 tempPos[3];

		GFPhysVector3 manivertPos[3];

		tempPos[0] = FFDObject->m_manivertex[rendface.manivertindex[0]].m_UnDeformedPos;

		tempPos[1] = FFDObject->m_manivertex[rendface.manivertindex[1]].m_UnDeformedPos;

		tempPos[2] = FFDObject->m_manivertex[rendface.manivertindex[2]].m_UnDeformedPos;

		manivertPos[0] = GFPhysVector3(tempPos[0].x , tempPos[0].y , tempPos[0].z);

		manivertPos[1] = GFPhysVector3(tempPos[1].x , tempPos[1].y , tempPos[1].z);

		manivertPos[2] = GFPhysVector3(tempPos[2].x , tempPos[2].y , tempPos[2].z);

		GFPhysVector3 closetPointInTri = ClosestPtPointTriangle(pointInRoughFace, 
			manivertPos[0],
			manivertPos[1], 
			manivertPos[2]);

		float Dist = (closetPointInTri-pointInRoughFace).Length() ;
		if(Dist < mindist)
		{
			mindist = Dist;
			minfaceIndex = f;
			minPoint = closetPointInTri;
		}
	}

	if(minfaceIndex >= 0)
	{
		const DynamicSurfaceFreeDeformObject::RendFaceData & rendface = FFDObject->m_rendfaces[minfaceIndex];

		float weights[3];

		Ogre::Vector3 tempPos[3];

		GFPhysVector3 faceVertex[3];

		tempPos[0] = FFDObject->m_manivertex[rendface.manivertindex[0]].m_UnDeformedPos;

		tempPos[1] = FFDObject->m_manivertex[rendface.manivertindex[1]].m_UnDeformedPos;

		tempPos[2] = FFDObject->m_manivertex[rendface.manivertindex[2]].m_UnDeformedPos;

		faceVertex[0] = GFPhysVector3(tempPos[0].x , tempPos[0].y , tempPos[0].z);

		faceVertex[1] = GFPhysVector3(tempPos[1].x , tempPos[1].y , tempPos[1].z);

		faceVertex[2] = GFPhysVector3(tempPos[2].x , tempPos[2].y , tempPos[2].z);

		CalcBaryCentric(faceVertex[0] , faceVertex[1] , faceVertex[2], minPoint ,weights[0] , weights[1] , weights[2]);

		finefaceid = minfaceIndex;

		fineweights[0] = weights[0];

		fineweights[1] = weights[1];

		fineweights[2] = weights[2];
	}
}
//====================================================================================================================
OrganSurfaceBloodTextureTrack * TextureBloodTrackEffect::createBloodTrackInRoughFace(MisMedicOrgan_Ordinary * oragn , GFPhysSoftBodyFace * faceRough , const float Roughweights[3], float radius)
{
	int   finefaceid;

	float fineweights[3];

	GetClosetPointInRefinedFaces(faceRough , Roughweights , finefaceid , fineweights );

	return createBloodTrack(oragn , fineweights , 1.0f , finefaceid, radius , 2.0f , 2.5f);

}
//====================================================================================================================
OrganSurfaceBloodTextureTrack * TextureBloodTrackEffect::createBloodTrack(MisMedicOrgan_Ordinary * organ, float weights[3], float mass, int initfaceid, float radius, float BloodSlipAccelrate, float BloodMaxSlipVel, float fDir)
{
	radius = radius*(1.0f+(::rand()%1000)*0.001f);///random radius
	DynamicBloodParticle * particle = m_bloodsys->createBloodParticle(organ, weights, mass, initfaceid, radius,  BloodSlipAccelrate,  BloodMaxSlipVel, fDir);

	OrganSurfaceBloodTextureTrack * bloodtrack = new OrganSurfaceBloodTextureTrack(initfaceid, this);
	bloodtrack->m_BloodStartRadius = bloodtrack->m_BloodEndRadius = radius;
	
	bool succed = bloodtrack->SetLeaderParticle(particle);

	if(succed)
	{
		m_OrganSurfaceBloodStreams.push_back(bloodtrack);

		//m_NeedReRender = true;

		//产生流血事件
		MxToolEvent * pEvent = CMXEventsDump::Instance()->CreateEventNew(MxEvent::MXET_BleedStart,NULL,this);
		pEvent->m_UserData = initfaceid;
		pEvent->SetWeights(weights);
		CMXEventsDump::Instance()->PushEvent(pEvent,true);
	}
	else
	{
		delete bloodtrack;
		bloodtrack = 0;
	}
	return bloodtrack;
}

OrganSurfaceBloodTextureTrack* TextureBloodTrackEffect::createBloodTrackUseForOrganBeCutFace(MisMedicOrgan_Ordinary * organ, float weights[3], float mass, int initfaceid, float radius, float BloodSlipAccelrate, float BloodMaxSlipVel /* = 0.0027f */)
{
	DynamicBloodParticle * particle = m_bloodsys->createBloodParticle(organ, weights, mass, initfaceid,  BloodSlipAccelrate,  BloodMaxSlipVel, radius);

	OrganSurfaceBloodTextureTrack * bloodtrack = new OrganSurfaceBloodTextureTrack(initfaceid,this);
	bloodtrack->m_BloodStartRadius = bloodtrack->m_BloodEndRadius = radius;
	bool succed = bloodtrack->SetLeaderParticle(particle);

	if(succed)
	{
		particle->setParameterForDynamicInCutFace();
		m_OrganSurfaceBloodStreams.push_back(bloodtrack);

		bloodtrack->m_bIsDynamicBloodStream = true;
		//m_NeedReRender = true;

		//产生流血事件
		MxToolEvent * pEvent = CMXEventsDump::Instance()->CreateEventNew(MxEvent::MXET_BleedStart,NULL,this);
		pEvent->m_UserData = initfaceid;
		pEvent->SetWeights(weights);
		CMXEventsDump::Instance()->PushEvent(pEvent,true);
	}
	else
	{
		delete bloodtrack;
		bloodtrack = 0;
	}
	return bloodtrack;
}

//====================================================================================================================
int TextureBloodTrackEffect::StopFirstBloodTrack()
{
	int trackId = -1;
	if(m_OrganSurfaceBloodStreams.size() > 0)
	{
		OrganSurfaceBloodTextureTrack * bloodtrack = m_OrganSurfaceBloodStreams[0];
		
		trackId = bloodtrack->m_BloodTrackId;
		
		bloodtrack->Stop();
	}
	return trackId;
}
//===================================================================================================================
void TextureBloodTrackEffect::StopBloodsTooOlder(float timeseconds)
{
	for(size_t c = 0 ; c < m_OrganSurfaceBloodStreams.size() ; c++)
	{
		OrganSurfaceBloodTextureTrack * bloodtrack = m_OrganSurfaceBloodStreams[c];
	
		if(bloodtrack->m_elapseTime > timeseconds)
		{
			bloodtrack->Stop();
		}
	}
}
//===================================================================================================================
void TextureBloodTrackEffect::RemoveBloodTrack(const std::vector<Ogre::Vector2> & texCoords , std::vector<int> &removeTrackIDs ,float range)
{
	if(texCoords.size() && m_OrganSurfaceBloodStreams.size())
	{
		float weights[3] = {0.33333f,0.33333f,0.33333f};
		int faceId = -1;
		MisMedicOrgan_Ordinary * organ = m_bloodsys->GetHostOrgan();
		if(organ ==NULL)
			return ;
		for(std::size_t t = 0 ; t < texCoords.size();++t)
		{
			for(std::size_t s = 0;s < m_OrganSurfaceBloodStreams.size();++s)
			{
				if(m_OrganSurfaceBloodStreams[s]->m_LeaderMovePath.size() == 1)
				{
					faceId  = m_OrganSurfaceBloodStreams[s]->m_StartFaceID;
					if(faceId >=0 && faceId < organ->m_OriginFaces.size())
					{
						Ogre::Vector2 texcoord = organ->GetTextureCoord(organ->m_OriginFaces[faceId].m_physface,weights);
						float dis= texcoord.distance(texCoords[t]) ;
						if(dis  <= 0.0035f)
						{
							removeTrackIDs.push_back(m_OrganSurfaceBloodStreams[s]->m_BloodTrackId);
							delete m_OrganSurfaceBloodStreams[s];
							m_OrganSurfaceBloodStreams.erase(m_OrganSurfaceBloodStreams.begin() + s);
							--s;
						}
					}
				}
			}
		}
	}
}

int TextureBloodTrackEffect::GetNumOfDynamicBlood()
{
	int count = 0;
	for(size_t s = 0;s<m_OrganSurfaceBloodStreams.size();++s)
	{
		if(m_OrganSurfaceBloodStreams[s]->m_LeaderMovePath.size() == 1)
			++count;
	}
	return count;
}
//===================================================================================================================
VeinConnectBloodTextureTrack * TextureBloodTrackEffect::CreateBloodTrackForVeinConnect(VeinConnectObject * stripobj , const VeinConnectPair & coonpair , float initweight)
{
	//const VeinConnectPair & coonpair = stripobj->GetConnectPair(clusterId , pairid);
	srand(GetTickCount());

	Ogre::Vector2  adherAtext0 = coonpair.texcoord[0];

	Ogre::Vector2  adherAtext1 = coonpair.texcoord[1];

	Ogre::Vector2  adherBtext0 = coonpair.texcoord[2];

	Ogre::Vector2  adherBtext1 = coonpair.texcoord[3];

	Ogre::Vector2  tex0 = (adherAtext0+adherAtext1)*0.5f;

	Ogre::Vector2  tex1 = (adherBtext0 + adherBtext1)*0.5f;

	Ogre::Vector2  startTex = tex0 + (tex1 - tex0) * initweight;

	//random end text
	float theta = 2.0f * 3.14159f * (float(rand() % 360) / 360.0f);

	Ogre::Vector2  splitDir(cosf(theta), sinf(theta));

	Ogre::Vector2  endTex = startTex + splitDir.normalisedCopy()*0.06f;
	
	VeinConnectBloodTextureTrack * bloodstream = new VeinConnectBloodTextureTrack(startTex, endTex);

	bloodstream->m_PositionWeight = 0;

	bloodstream->m_randtangent = true;

	//bloodstream->m_blooddecpermove = 2;

	bloodstream->m_bloodstartradius = 0.02;

	bloodstream->m_bloodendradius = 0.002;

	m_VeinConnectBloodTracks.push_back(bloodstream);

	return bloodstream;
}
//===================================================================================================================
void TextureBloodTrackEffect::ScaleStreamBloodDropRadius(float scalefactor , int indexstreamindex)
{
	if(indexstreamindex >= 0 && indexstreamindex < (int)m_OrganSurfaceBloodStreams.size())
	{
		OrganSurfaceBloodTextureTrack * bstream = m_OrganSurfaceBloodStreams[indexstreamindex];
		bstream->m_RadiusScale *= scalefactor;
	}
}
//==============================================================================================================
bool TextureBloodTrackEffect::BuildBloodStream(Ogre::ManualObject * bloodQuads, bool& bNeedBleed)
{
	//check whether need rebuild stream
	bNeedBleed = false;
	//bool NeedRebuild = false;

	//if(m_NeedReRender == true)
	//{
		//NeedRebuild = true;
		//m_NeedReRender = false;
	//}
	for(size_t s = 0 ; s < m_OrganSurfaceBloodStreams.size() ; s++)
	{
		OrganSurfaceBloodTextureTrack * stream = m_OrganSurfaceBloodStreams[s];
		if(stream->m_NeedReRender)
		{
			//NeedRebuild = true;
			bNeedBleed = true;
			stream->m_NeedReRender = false;
		}

		if (stream->m_currentActiveBPPIndex > 1.0f)
		{
			if (stream->m_currentActiveBPPIndex < stream->m_LeaderMovePath.size())
			{
				//NeedRebuild = true;
				stream->m_currentActiveBPPIndex = stream->m_currentActiveBPPIndex + stream->m_LeaderMovePath.size() * m_bloodFlowVelocity;
			}
		}
		//NeedRebuild = true;
	}

	int NumStreamBeUpdated = 0;
	//if(NeedRebuild)
	//{
		//////
		DWORD nFrameNum = 30 ;//100;
		DWORD nFrame = ::GetTickCount()/m_nBloodSpeed;
		DWORD nCurFrame = nFrame%nFrameNum;
		::srand(0);
		std::vector<float> vectorFrameRand;
		vectorFrameRand.resize(nFrameNum);
		for( int i = 0; i<nFrameNum; ++i )
		{
			vectorFrameRand[i] = (1.0f-(::rand()%1000)*0.002f)*0.2f;
		}
		/////
		int startIndex = 0;

		
		for(size_t s = 0 ; s < m_OrganSurfaceBloodStreams.size() ; s++)
		{
			OrganSurfaceBloodTextureTrack * stream = m_OrganSurfaceBloodStreams[s];

			Ogre::ColourValue bloodcolor = m_dynamicBloodValue;//stream->m_bloodcolor;

			float maxparticleinterval = stream->m_BloodStartRadius*0.3f;

			DWORD ndt = ::GetTickCount()-stream->m_nStartTime;

			if(stream->m_RadiusScale > 0.3f)
			{
				float bloodradius = stream->m_BloodStartRadius*stream->m_RadiusScale;

				int pStart = std::max(0, int(stream->m_currentActiveBPPIndex));

				if(pStart < stream->m_LeaderMovePath.size())
				   NumStreamBeUpdated++;
				
				for(size_t p = pStart; p < stream->m_LeaderMovePath.size() ; p++)
				{
					Ogre::Vector2 texcoord  = stream->m_LeaderMovePath[p].m_blood;

					float tu1 = texcoord.x;

					float tv1 = texcoord.y;

					float tu0 = tu1;

					float tv0 = tv1;

					int   interpoltecount = 1;

					if(p > 0)
					{
						Ogre::Vector2 prevtexcoord = stream->m_LeaderMovePath[p-1].m_blood;

						tu0 = prevtexcoord.x;

						tv0 = prevtexcoord.y;

						float particleinterval = (texcoord-prevtexcoord).length();

						if(particleinterval < 0.1f)//prevent texture warp missed as too long interval
						{
							interpoltecount = particleinterval / maxparticleinterval;
							interpoltecount = interpoltecount+1;
						}
					}

					float tu = tu1;

					float tv = tv1;

					for(int t = 1 ; t <= interpoltecount ; t++)
					{
						float percent = (float)t / (float)interpoltecount;

						tu = tu0*(1-percent) + tu1*percent;

						tv = tv0*(1-percent) + tv1*percent;

						tu = tu-0.5f;

						tv = 0.5-tv;
						
						size_t n = ((0x7fffffff-p)+nCurFrame)%nFrameNum;
						DWORD ndt1 = 0;
						float r = 1.25f;
						float r1 = 1.0f;
						//if( ndt>5000 )
						//{
						//	int ndt1 = ndt-5000;
						//	int nT = (p-pStart)*100-ndt1;
						//	if( nT>0 && nT<100)
						//		r = nT/100.0f;
						//	else if( nT<0 )
						//		r = 0.0f;
						//	r1 = (5000-ndt1)*1.0f/5000;
						//	if( r1<0.0f )
						//		r1 = 0.0f;
						//}

						float bloodradius1 = bloodradius*(1.0f+vectorFrameRand[n]*0.7f)*r*r1;
						float fColorRand = vectorFrameRand[n]*0.01f;
						Ogre::ColourValue bloodcolorRand = bloodcolor+Ogre::ColourValue(fColorRand,fColorRand,fColorRand,0.0f);
						
						bloodQuads->position(Ogre::Vector3(tu+bloodradius1, tv-bloodradius1, -1.0));
						bloodQuads->textureCoord(1, 1);
						bloodQuads->colour(bloodcolorRand);

						bloodQuads->position(Ogre::Vector3(tu-bloodradius1, tv+bloodradius1, -1.0));
						bloodQuads->textureCoord(0, 0);
						bloodQuads->colour(bloodcolorRand);

						bloodQuads->position(Ogre::Vector3(tu-bloodradius1, tv-bloodradius1, -1.0));
						bloodQuads->textureCoord(0, 1);
						bloodQuads->colour(bloodcolorRand);

						bloodQuads->position(Ogre::Vector3(tu+bloodradius1, tv+bloodradius1, -1.0));
						bloodQuads->textureCoord(1, 0);
						bloodQuads->colour(bloodcolorRand);

						//using indices
						bloodQuads->index(0+startIndex);
						bloodQuads->index(1+startIndex);
						bloodQuads->index(2+startIndex);

						bloodQuads->index(0+startIndex);
						bloodQuads->index(3+startIndex);
						bloodQuads->index(1+startIndex);

						startIndex += 4;

						if(  false  )
						{ 
							if( ndt > 10000 ) 
								ndt = 10000;
							bloodradius1 = bloodradius*(1.0f+ndt*0.0005f);
							bloodQuads->position(Ogre::Vector3(tu+bloodradius1, tv-bloodradius1, -1.0));
							bloodQuads->textureCoord(1, 1);
							bloodQuads->colour(bloodcolorRand);

							bloodQuads->position(Ogre::Vector3(tu-bloodradius1, tv+bloodradius1, -1.0));
							bloodQuads->textureCoord(0, 0);
							bloodQuads->colour(bloodcolorRand);

							bloodQuads->position(Ogre::Vector3(tu-bloodradius1, tv-bloodradius1, -1.0));
							bloodQuads->textureCoord(0, 1);
							bloodQuads->colour(bloodcolorRand);

							bloodQuads->position(Ogre::Vector3(tu+bloodradius1, tv+bloodradius1, -1.0));
							bloodQuads->textureCoord(1, 0);
							bloodQuads->colour(bloodcolorRand);

							//using indices
							bloodQuads->index(0+startIndex);
							bloodQuads->index(1+startIndex);
							bloodQuads->index(2+startIndex);

							bloodQuads->index(0+startIndex);
							bloodQuads->index(3+startIndex);
							bloodQuads->index(1+startIndex);

							startIndex += 4;
						}
					}
				}
			}
		}
	//}
	//return NeedRebuild;
		return NumStreamBeUpdated > 0 ? true : false;
}
//==============================================================================================
bool TextureBloodTrackEffect::BuildBloodStreamBleed(Ogre::ManualObject * bloodQuads)
{
	int bleedupdatedCount = 0;

	Ogre::ColourValue bleedcolor(1.0f , 1.0f , 1.0f , 0.5f);

	int startIndex = 0;

	for(size_t s = 0 ; s < m_OrganSurfaceBloodStreams.size() ; s++)
	{
		OrganSurfaceBloodTextureTrack * stream = m_OrganSurfaceBloodStreams[s];

		if(stream->m_bIsStopBleed)
			continue;

		float bloodradius = stream->m_BloodStartRadius*stream->m_RadiusScale;

		int pStart = std::max(0, int(stream->m_currentActiveBPPIndex));
		for(size_t p = pStart; p < stream->m_LeaderMovePath.size() ; p++)
		{
			OrganSurfaceBloodTextureTrack::BloodPathPoint & pahtpoint = stream->m_LeaderMovePath[p];

			if(pahtpoint.m_TimeElaspedSinceLastBleed > 100)
			{
				pahtpoint.m_TimeElaspedSinceLastBleed = 0;

				Ogre::Vector2 texcoord = pahtpoint.m_bleed;

				float bleedrotate = pahtpoint.bleedrotateRadian;

				float tu = texcoord.x;

				float tv = texcoord.y;

				tu = tu-0.5f;

				tv = 0.5-tv;

				Ogre::Quaternion rotation( Ogre::Radian(bleedrotate), Ogre::Vector3::UNIT_Z);

				Ogre::Vector3 quadCenter(tu,tv,-1.0);

				float realradius = bloodradius;

				bloodQuads->position(quadCenter+rotation*Ogre::Vector3(realradius, -realradius, 0));
				bloodQuads->textureCoord(1, 1);
				bloodQuads->colour(bleedcolor);

				bloodQuads->position(quadCenter+rotation*Ogre::Vector3(-realradius, realradius, 0));
				bloodQuads->textureCoord(0, 0);
				bloodQuads->colour(bleedcolor);

				bloodQuads->position(quadCenter+rotation*Ogre::Vector3(-realradius, -realradius, 0));
				bloodQuads->textureCoord(0, 1);
				bloodQuads->colour(bleedcolor);

				bloodQuads->position(quadCenter+rotation*Ogre::Vector3(realradius, realradius, 0));
				bloodQuads->textureCoord(1, 0);
				bloodQuads->colour(bleedcolor);

				//using indices
				bloodQuads->index(0+startIndex);
				bloodQuads->index(1+startIndex);
				bloodQuads->index(2+startIndex);

				bloodQuads->index(0+startIndex);
				bloodQuads->index(3+startIndex);
				bloodQuads->index(1+startIndex);

				startIndex += 4;

				bleedupdatedCount++;
			}

		}
	}
	return (bleedupdatedCount > 0 ? true : false);
}
//======================================================================================================
bool TextureBloodTrackEffect::BuildBloodStreamBleed1(Ogre::ManualObject * bloodQuads)
{
	Ogre::Real rColor = 0.03f;
	Ogre::ColourValue bleedcolor(rColor , rColor , rColor , rColor);

	int startIndex = 0;

	DWORD nCurTime = ::GetTickCount();

	for( std::map<int,SBloodTraceMapNode>::iterator it = m_mapBloodTraceMap.begin(); it != m_mapBloodTraceMap.end(); ++it )
	{
		if( it->second.m_eState == _ENUM_BLOOD_MAP_NODE_STATE_DEFAULT_ )
			continue;
		
		Ogre::Quaternion rotation( Ogre::Radian(0.0f), Ogre::Vector3::UNIT_Z);
		Ogre::Vector3 quadCenter(it->second.m_vPos.x,it->second.m_vPos.y,-1.0);
		

		DWORD ndt = nCurTime-it->second.m_nStartTime;
		if( ndt>_BLEED_TIME_ ) 
			ndt = _BLEED_TIME_;

/*		float realradius = it->second.m_fRadius*ndt/_BLEED_TIME_;
		realradius = realradius*1.5f;*/	

		float realradius = it->second.m_fRadius*1.5f;

		bloodQuads->position(quadCenter+rotation*Ogre::Vector3(realradius, -realradius, 0));
		bloodQuads->textureCoord(1, 1);
		bloodQuads->colour(bleedcolor);

		bloodQuads->position(quadCenter+rotation*Ogre::Vector3(-realradius, realradius, 0));
		bloodQuads->textureCoord(0, 0);
		bloodQuads->colour(bleedcolor);

		bloodQuads->position(quadCenter+rotation*Ogre::Vector3(-realradius, -realradius, 0));
		bloodQuads->textureCoord(0, 1);
		bloodQuads->colour(bleedcolor);

		bloodQuads->position(quadCenter+rotation*Ogre::Vector3(realradius, realradius, 0));
		bloodQuads->textureCoord(1, 0);
		bloodQuads->colour(bleedcolor);

		//using indices
		bloodQuads->index(0+startIndex);
		bloodQuads->index(1+startIndex);
		bloodQuads->index(2+startIndex);

		bloodQuads->index(0+startIndex);
		bloodQuads->index(3+startIndex);
		bloodQuads->index(1+startIndex);

		startIndex += 4;
		
	}

	//for(size_t s = 0 ; s < m_OrganSurfaceBloodStreams.size() ; s++)
	//{
	//	OrganSurfaceBloodTextureTrack * stream = m_OrganSurfaceBloodStreams[s];

	//	float bloodradius = stream->m_BloodStartRadius*stream->m_RadiusScale*1.4f;

	//	int pStart = std::max(0, int(stream->m_currentActiveBPPIndex));
	//	for(size_t p = pStart; p < stream->m_LeaderMovePath.size() ; p++)
	//	{
	//		OrganSurfaceBloodTextureTrack::BloodPathPoint & pahtpoint = stream->m_LeaderMovePath[p];

	//		if(pahtpoint.m_TimeElaspedSinceLastBleed > 100)
	//		{
	//			pahtpoint.m_TimeElaspedSinceLastBleed = 0;

	//			Ogre::Vector2 texcoord = pahtpoint.m_bleed;

	//			float bleedrotate = pahtpoint.bleedrotateRadian;

	//			float tu = texcoord.x;

	//			float tv = texcoord.y;

	//			tu = tu-0.5f;

	//			tv = 0.5-tv;

	//			Ogre::Quaternion rotation( Ogre::Radian(bleedrotate), Ogre::Vector3::UNIT_Z);

	//			Ogre::Vector3 quadCenter(tu,tv,-1.0);

	//			float realradius = bloodradius;

	//			bloodQuads->position(quadCenter+rotation*Ogre::Vector3(realradius, -realradius, 0));
	//			bloodQuads->textureCoord(1, 1);
	//			bloodQuads->colour(bleedcolor);

	//			bloodQuads->position(quadCenter+rotation*Ogre::Vector3(-realradius, realradius, 0));
	//			bloodQuads->textureCoord(0, 0);
	//			bloodQuads->colour(bleedcolor);

	//			bloodQuads->position(quadCenter+rotation*Ogre::Vector3(-realradius, -realradius, 0));
	//			bloodQuads->textureCoord(0, 1);
	//			bloodQuads->colour(bleedcolor);

	//			bloodQuads->position(quadCenter+rotation*Ogre::Vector3(realradius, realradius, 0));
	//			bloodQuads->textureCoord(1, 0);
	//			bloodQuads->colour(bleedcolor);

	//			//using indices
	//			bloodQuads->index(0+startIndex);
	//			bloodQuads->index(1+startIndex);
	//			bloodQuads->index(2+startIndex);

	//			bloodQuads->index(0+startIndex);
	//			bloodQuads->index(3+startIndex);
	//			bloodQuads->index(1+startIndex);

	//			startIndex += 4;

	//			bleedupdatedCount++;
	//		}

	//	}
	//}

	return true;
}
//==============================================================================================================
bool TextureBloodTrackEffect::Update(float dt , std::vector<int> &removeTrackIDs)
{
	//update dynamicPoints
	for(std::size_t p = 0;p < m_dynamicBloodPoints.size();++p)
		m_dynamicBloodPoints[p]->Update(dt);

	//
	const float fixedTimeStep = 1.0f / 30.0f;

	m_AccmulateTime += dt;

	int stepcount = (int)(m_AccmulateTime / fixedTimeStep); 

	if(stepcount <= 0)
		return false;

	if(stepcount > 3)
	{
		stepcount = 3;
		m_AccmulateTime = 0;
	}
	else
	{
		m_AccmulateTime = m_AccmulateTime-fixedTimeStep*stepcount;
	}

	//for(int s = 0 ; s < (int)m_OrganSurfaceBloodStreams.size() ; s++)
    std::vector<OrganSurfaceBloodTextureTrack*>::iterator itor = m_OrganSurfaceBloodStreams.begin();

	while(itor != m_OrganSurfaceBloodStreams.end())
	{
		OrganSurfaceBloodTextureTrack * bloodtrack = (*itor);

		bool alive = bloodtrack->Update(fixedTimeStep);
		
		if(alive == false)
		{
		   int trackId = bloodtrack->m_BloodTrackId;
		   removeTrackIDs.push_back(trackId);

		   delete bloodtrack;
		   itor = m_OrganSurfaceBloodStreams.erase(itor);
		}
		else
		{
			itor++;
		}
	}

	if(m_OrganSurfaceBloodStreams.size() == 0 && m_dynamicBloodPoints.size() == 0)
		return false;
	else
		return true;
}
//==============================================================================================================
bool TextureBloodTrackEffect::createBloodEffusionPoint(Ogre::Vector2 texturecoord, float radius)
{
	//temp
	for(size_t b = 0 ; b < m_BloodEffusionPoints.size() ; b++)
	{
		if((m_BloodEffusionPoints[b].m_texturecoord - texturecoord).length() < 0.06f)
			return false;
	}
	//
	OrganDynEffusionTextureBlood effusionpoint;
	effusionpoint.m_texturecoord = texturecoord;
	effusionpoint.m_radius = radius;
	m_BloodEffusionPoints.push_back(effusionpoint);
	return true;
}
//=========================================================================================================================
void TextureBloodTrackEffect::UpdateVeinConnectBloodTrack(float dt)//Ogre::TexturePtr bloodtexture , float deltatime)
{ 
	const float fixedtimeinterval = 1.0f / 30.0f;

	m_AccmulateTime += dt;

	int stepcount = (int)(m_AccmulateTime / fixedtimeinterval);

	if(stepcount <= 0)
	   return;

	if(stepcount > 3)
	{
		stepcount = 3;
		m_AccmulateTime = 0;
	}
	else
	{
		m_AccmulateTime = m_AccmulateTime-fixedtimeinterval*stepcount;
	}

	if(m_VeinConnectBloodTracks.size() == 0)
	   return;

	//int c = 0;

	//while(c < stepcount)
	//{
		for(size_t i = 0 ;i < m_VeinConnectBloodTracks.size(); i++)
		{
			VeinConnectBloodTextureTrack * trackhead = m_VeinConnectBloodTracks[i];//->m_VeinBloodHead;

			trackhead->m_currslipspeed += trackhead->m_slipaccelerate*fixedtimeinterval;

			if(trackhead->m_currslipspeed > trackhead->m_maxslipspeed)
			   trackhead->m_currslipspeed = trackhead->m_maxslipspeed;

			float Speed = 0.1f;

			float totalDist = (trackhead->m_StartTexCoord - trackhead->m_EndTexCoord).length();

			float PosWeight = trackhead->m_PositionWeight;

			float currMoveDist = totalDist * PosWeight;

			currMoveDist += Speed * fixedtimeinterval;

			PosWeight = currMoveDist / totalDist; //PosWeight += 0.006f;

			if(PosWeight > 1)
			   PosWeight = 1;

			trackhead->m_PositionWeight = PosWeight;

			//trackhead->m_bloodquanity -= trackhead->m_blooddecpermove;

			//if(trackhead->m_bloodquanity <= trackhead->m_stopquanity)
			//   trackhead->m_bloodquanity = trackhead->m_stopquanity;
		}
		//c++;
	//}

	
	//remove dead bloodstream
	std::vector<VeinConnectBloodTextureTrack*>::iterator itor = m_VeinConnectBloodTracks.begin();

	while(itor != m_VeinConnectBloodTracks.end())
	{
		VeinConnectBloodTextureTrack * stream = *itor;

		if(stream == 0)
		   itor = m_VeinConnectBloodTracks.erase(itor);
		else if(stream->m_PositionWeight > 1 - GP_EPSILON)// <= stream->m_stopquanity+FLT_EPSILON)
		{
		   delete stream;
		   itor = m_VeinConnectBloodTracks.erase(itor);
		}
		else
		   itor++;
	}
}

bool TextureBloodTrackEffect::BuildVeinBlood(Ogre::ManualObject * bloodQuads)
{
	//build bleed point rend quad
	int bleedupdatedCount = 0;

	Ogre::ColourValue bleedcolor(1.0f , 1.0f , 1.0f , 0.8f);

	int startIndex = 0;

	for(size_t i = 0 ; i < m_VeinConnectBloodTracks.size() ; i++)
	{
		VeinConnectBloodTextureTrack * trackhead = m_VeinConnectBloodTracks[i];

		//float percent = float(trackhead->m_bloodquanity-trackhead->m_stopquanity) / float(trackhead->m_startquantity-trackhead->m_stopquanity);

		//if (percent < 0)  percent = 0;

		//else if(percent > 1)  percent = 1;

		//float fadefactor = trackhead->m_AplhaFade;//0.5f;//0.1f;//pass by parameter please

		trackhead->m_bloodstartradius = m_VeinConnectBloodTracks[i]->m_bloodstartradius;// 0.01f;

		trackhead->m_bloodendradius = m_VeinConnectBloodTracks[i]->m_bloodendradius; //0.002f;

		float weight = trackhead->m_PositionWeight;

		float DrawRadius = trackhead->m_bloodstartradius * (1 - weight) + trackhead->m_bloodendradius * weight;// (1 - percent);

		float DrawAlpha = trackhead->m_AplhaFade;//(int)(percent*255*fadefactor);

		Ogre::Vector2  texcoord = (1-weight) * trackhead->m_StartTexCoord + weight * trackhead->m_EndTexCoord;

		int	  randnum   = rand();

		Ogre::Vector2 turbdir = trackhead->m_StartTexCoord-trackhead->m_EndTexCoord;
		turbdir.normalise();
		turbdir = turbdir.perpendicular();

		float turbvalue = (float(randnum % 10 - 5) / 10.0f) * DrawRadius;
		texcoord.x = texcoord.x + turbvalue * turbdir.x;
		texcoord.y = texcoord.y + turbvalue * turbdir.y;

		float bleedrotate = 0;//pahtpoint.bleedrotateRadian;

		float tu = texcoord.x;

		float tv = texcoord.y;

		tu = tu-0.5f;

		tv = 0.5-tv;

		Ogre::Quaternion rotation( Ogre::Radian(bleedrotate), Ogre::Vector3::UNIT_Z);

		Ogre::Vector3 quadCenter(tu , tv , -1.0);

		Ogre::ColourValue bleedcolor(1.0f , 1.0f , 1.0f , DrawAlpha);

		bloodQuads->position(quadCenter+rotation*Ogre::Vector3(DrawRadius, -DrawRadius, 0));
		bloodQuads->textureCoord(1, 1);
		bloodQuads->colour(bleedcolor);

		bloodQuads->position(quadCenter+rotation*Ogre::Vector3(-DrawRadius, DrawRadius, 0));
		bloodQuads->textureCoord(0, 0);
		bloodQuads->colour(bleedcolor);

		bloodQuads->position(quadCenter+rotation*Ogre::Vector3(-DrawRadius, -DrawRadius, 0));
		bloodQuads->textureCoord(0, 1);
		bloodQuads->colour(bleedcolor);

		bloodQuads->position(quadCenter+rotation*Ogre::Vector3(DrawRadius, DrawRadius, 0));
		bloodQuads->textureCoord(1, 0);
		bloodQuads->colour(bleedcolor);

		//using indices
		bloodQuads->index(0+startIndex);
		bloodQuads->index(1+startIndex);
		bloodQuads->index(2+startIndex);

		bloodQuads->index(0+startIndex);
		bloodQuads->index(3+startIndex);
		bloodQuads->index(1+startIndex);

		startIndex += 4;
		bleedupdatedCount++;
	}

	return (bleedupdatedCount > 0 ? true : false);
}


DynamicBloodPoint * TextureBloodTrackEffect::CreateDynamicBloodPoint(const GFPhysSoftBodyFace * pFace,const float weights[3])
{
	DynamicBloodPoint * pBloodPoint = new DynamicBloodPoint(pFace,weights);
	m_dynamicBloodPoints.push_back(pBloodPoint);
	return pBloodPoint;
}

bool TextureBloodTrackEffect::RemoveDynamicBloodPoint(const GFPhysSoftBodyFace * pFace)
{
	for(std::size_t p = 0;p < m_dynamicBloodPoints.size();++p)
	{
		if(m_dynamicBloodPoints[p]->m_pFace == pFace)
		{
			delete m_dynamicBloodPoints[p];
			m_dynamicBloodPoints.erase(m_dynamicBloodPoints.begin() + p);
			return true;
		}
	}
	return false;
}

SBloodTraceMapNode* TextureBloodTrackEffect::GetBloodTraceMapNode( int nX, int nY )
{
	if(  nX<0 || nY<0 || nX>=_BLOOD_TRACE_MAP_SIZE_ || nY>=_BLOOD_TRACE_MAP_SIZE_ )
		return NULL;
	int nIndex = nX*_BLOOD_TRACE_MAP_SIZE_+nY;
	if( m_mapBloodTraceMap.find(nIndex) != m_mapBloodTraceMap.end() )
		return &(m_mapBloodTraceMap[nIndex]);
	else
		return false;
}

bool TextureBloodTrackEffect::AddBloodTraceMapNode( int nX, int nY, SBloodTraceMapNode& record )
{
	if(  nX<0 || nY<0 || nX>=_BLOOD_TRACE_MAP_SIZE_ || nY>=_BLOOD_TRACE_MAP_SIZE_ )
		return false;
	int nIndex = nX*_BLOOD_TRACE_MAP_SIZE_+nY;
	m_mapBloodTraceMap[nIndex] = record;
	return true;
}