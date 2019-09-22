#include "TextureWaterEffect.h"

TextureWaterTrackEffect::TextureWaterTrackEffect(MisMedicOrganInterface * organif):TextureBloodTrackEffect(organif)
{
	m_nFrameNum = 30 ;//100;
	::srand(0);
	m_vectorFrameRand.resize(m_nFrameNum);
	for( int i = 0; i<m_nFrameNum; ++i )
		m_vectorFrameRand[i] = (1.0f-(::rand()%1000)*0.002f)*0.2f;

	m_nWaterSpeed = 80;
	m_nRemoveWaterStream = -1;
}

TextureWaterTrackEffect::~TextureWaterTrackEffect()
{
}

bool TextureWaterTrackEffect::Update(float dt)
{
	//DWORD nCurTime = ::GetTickCount();
	//for(size_t s = 0 ; s < m_OrganSurfaceBloodStreams.size() ; s++)
	//{
	//	OrganSurfaceBloodTextureTrack * stream = m_OrganSurfaceBloodStreams[s];
	//	if(nCurTime>=stream->m_nWaterDisapearTime)
	//	{
	//		removeWaterTrack(s);
	//		break;
	//	}
	//}
	if(m_nRemoveWaterStream>=0)
	{
		removeWaterTrack(m_nRemoveWaterStream);
		m_nRemoveWaterStream = -1;
	}
	std::vector<int> removeTrackIDs;
	return TextureBloodTrackEffect::Update(dt , removeTrackIDs);
}

OrganSurfaceBloodTextureTrack * TextureWaterTrackEffect::createWaterTrack(MisMedicOrgan_Ordinary * organ , float weights[3] ,  float mass  , int initfaceid, float radius/* = 0.0027f*/, float fDir)
{
	radius = radius*(1.0f+(::rand()%1000)*0.001f);///random radius
	DynamicBloodParticle * particle = m_bloodsys->createBloodParticle(organ , weights , mass , initfaceid , radius, 2.0f , 2.5f ,fDir);

	OrganSurfaceBloodTextureTrack * bloodtrack = new OrganSurfaceBloodTextureTrack(initfaceid, this);
	
	bloodtrack->m_bIsWater = true;

	bloodtrack->m_BloodStartRadius = bloodtrack->m_BloodEndRadius = radius;
	
	bool succed = bloodtrack->SetLeaderParticle(particle);

	if(succed)
	{
		m_OrganSurfaceBloodStreams.push_back(bloodtrack);

		////m_NeedReRender = true;

		////产生流血事件
		//CMXToolEvent * pEvent = CMXEventsDump::Instance()->CreateEventNew(IMXEvent::MXET_BleedStart,NULL,this);
		//pEvent->m_UserData = initfaceid;
		//pEvent->SetWeights(weights);
		//CMXEventsDump::Instance()->PushEvent(pEvent,true);
	}
	else
	{
		delete bloodtrack;
		bloodtrack = 0;
	}
	return bloodtrack;
}

bool TextureWaterTrackEffect::removeWaterTrack(size_t p)
{
	if(p>=m_OrganSurfaceBloodStreams.size()||m_OrganSurfaceBloodStreams[p]==NULL)
		return false;
	delete m_OrganSurfaceBloodStreams[p];
	m_OrganSurfaceBloodStreams.erase(m_OrganSurfaceBloodStreams.begin() + p);
	return true;
}

/*
bool TextureWaterTrackEffect::BuildWaterStream(Ogre::ManualObject * waterQuads)
{
	bool bNeedRender = false;
	m_nRemoveWaterStream = -1;
	for(size_t s = 0 ; s < m_OrganSurfaceBloodStreams.size() ; s++)
	{
		OrganSurfaceBloodTextureTrack * stream = m_OrganSurfaceBloodStreams[s];
		if(stream->m_NeedReRender)
		{
			//NeedRebuild = true;
			//bNeedBleed = true;
			stream->m_NeedReRender = false;
		}

		if (stream->m_currentActiveBPPIndex > 1.0f)
		{
			if (stream->m_currentActiveBPPIndex < stream->m_LeaderMovePath.size())
			{
				stream->m_currentActiveBPPIndex = stream->m_currentActiveBPPIndex + stream->m_LeaderMovePath.size() * m_bloodFlowVelocity;
			}
		}
	}

	DWORD nCurTime = ::GetTickCount();
	DWORD nFrame = nCurTime/m_nWaterSpeed;
	DWORD nCurFrame = nFrame%m_nFrameNum;

	int NumStreamBeUpdated = 0;
	int startIndex = 0;
	for(size_t s = 0 ; s < m_OrganSurfaceBloodStreams.size() ; s++)
	{
		OrganSurfaceBloodTextureTrack * stream = m_OrganSurfaceBloodStreams[s];

		DWORD nDisapearParticalNum = 0;
		if(nCurTime>=stream->m_nWaterDisapearTime)
		{
			nDisapearParticalNum = (nCurTime-stream->m_nWaterDisapearTime)/m_nWaterSpeed;
			NumStreamBeUpdated++;
		}

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
				if(nDisapearParticalNum>0)
				{
					nDisapearParticalNum--;
					continue;
				}
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
					float tu = tu1;

					float tv = tv1;

					for(int t = 1 ; t <= interpoltecount ; t++)
					{
						float percent = (float)t / (float)interpoltecount;

						tu = tu0*(1-percent) + tu1*percent;

						tv = tv0*(1-percent) + tv1*percent;

						tu = tu-0.5f;

						tv = 0.5-tv;
						
						size_t n = ((0x7fffffff-p)+nCurFrame)%m_nFrameNum;

						DWORD ndt1 = 0;
						float r = 1.25f;
						float r1 = 1.0f;

						float bloodradius1 = bloodradius*(1.0f+m_vectorFrameRand[n]*0.7f)*r*r1;
						float fColorRand = m_vectorFrameRand[n]*0.01f;
						Ogre::ColourValue bloodcolorRand = Ogre::ColourValue(fColorRand,fColorRand,fColorRand,0.0f);

						waterQuads->position(Ogre::Vector3(tu+bloodradius1, tv-bloodradius1, -1.0));
						waterQuads->textureCoord(1, 1);
						waterQuads->colour(bloodcolorRand);

						waterQuads->position(Ogre::Vector3(tu-bloodradius1, tv+bloodradius1, -1.0));
						waterQuads->textureCoord(0, 0);
						waterQuads->colour(bloodcolorRand);

						waterQuads->position(Ogre::Vector3(tu-bloodradius1, tv-bloodradius1, -1.0));
						waterQuads->textureCoord(0, 1);
						waterQuads->colour(bloodcolorRand);

						waterQuads->position(Ogre::Vector3(tu+bloodradius1, tv+bloodradius1, -1.0));
						waterQuads->textureCoord(1, 0);
						waterQuads->colour(bloodcolorRand);

						//using indices
						waterQuads->index(0+startIndex);
						waterQuads->index(1+startIndex);
						waterQuads->index(2+startIndex);

						waterQuads->index(0+startIndex);
						waterQuads->index(3+startIndex);
						waterQuads->index(1+startIndex);

						startIndex += 4;
					}
				}
			}
		}
		if(nDisapearParticalNum>0)
			m_nRemoveWaterStream = static_cast<int>(s);
	}
	return NumStreamBeUpdated > 0 ? true : false;
}
*/

//在血流纹理上绘制半透明的水流效果
bool TextureWaterTrackEffect::BuildWaterStreamTex(Ogre::ManualObject * waterQuads)
{
	bool bNeedRender = false;
	//m_nRemoveWaterStream = -1;
	for(size_t s = 0 ; s < m_OrganSurfaceBloodStreams.size() ; s++)
	{
		OrganSurfaceBloodTextureTrack * stream = m_OrganSurfaceBloodStreams[s];
		if(stream->m_NeedReRender)
		{
			//NeedRebuild = true;
			//bNeedBleed = true;
			stream->m_NeedReRender = false;
		}

		if (stream->m_currentActiveBPPIndex > 1.0f)
		{
			if (stream->m_currentActiveBPPIndex < stream->m_LeaderMovePath.size())
			{
				stream->m_currentActiveBPPIndex = stream->m_currentActiveBPPIndex + stream->m_LeaderMovePath.size() * m_bloodFlowVelocity;
			}
		}
	}

	DWORD nCurTime = ::GetTickCount();
	DWORD nFrame = nCurTime/m_nWaterSpeed;
	DWORD nCurFrame = nFrame%m_nFrameNum;

	int NumStreamBeUpdated = 0;
	int startIndex = 0;
	for(size_t s = 0 ; s < m_OrganSurfaceBloodStreams.size() ; s++)
	{
		OrganSurfaceBloodTextureTrack * stream = m_OrganSurfaceBloodStreams[s];

		DWORD nDisapearParticalNum = 0;
		if(nCurTime>=stream->m_nWaterDisapearTime)
		{
			nDisapearParticalNum = (nCurTime-stream->m_nWaterDisapearTime)/m_nWaterSpeed;
			NumStreamBeUpdated++;
		}

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
				if(nDisapearParticalNum>0)
				{
					nDisapearParticalNum--;
					continue;
				}
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
					float tu = tu1;

					float tv = tv1;

					for(int t = 1 ; t <= interpoltecount ; t++)
					{
						float percent = (float)t / (float)interpoltecount;

						tu = tu0*(1-percent) + tu1*percent;

						tv = tv0*(1-percent) + tv1*percent;

						tu = tu-0.5f;

						tv = 0.5-tv;
						
						size_t n = ((0x7fffffff-p)+nCurFrame)%m_nFrameNum;

						DWORD ndt1 = 0;
						float r = 1.25f;
						float r1 = 1.0f;

						float bloodradius1 = bloodradius*(1.0f+m_vectorFrameRand[n]*0.7f)*r*r1;
						float fColorRand = m_vectorFrameRand[n]*0.01f;
						Ogre::ColourValue bloodcolorRand = Ogre::ColourValue(fColorRand,fColorRand,fColorRand,0.0f);

						waterQuads->position(Ogre::Vector3(tu+bloodradius1, tv-bloodradius1, -1.0));
						waterQuads->textureCoord(1, 1);
						waterQuads->colour(bloodcolorRand);

						waterQuads->position(Ogre::Vector3(tu-bloodradius1, tv+bloodradius1, -1.0));
						waterQuads->textureCoord(0, 0);
						waterQuads->colour(bloodcolorRand);

						waterQuads->position(Ogre::Vector3(tu-bloodradius1, tv-bloodradius1, -1.0));
						waterQuads->textureCoord(0, 1);
						waterQuads->colour(bloodcolorRand);

						waterQuads->position(Ogre::Vector3(tu+bloodradius1, tv+bloodradius1, -1.0));
						waterQuads->textureCoord(1, 0);
						waterQuads->colour(bloodcolorRand);

						//using indices
						waterQuads->index(0+startIndex);
						waterQuads->index(1+startIndex);
						waterQuads->index(2+startIndex);

						waterQuads->index(0+startIndex);
						waterQuads->index(3+startIndex);
						waterQuads->index(1+startIndex);

						startIndex += 4;
					}
				}
			}
		}
		if(nDisapearParticalNum>0)
			m_nRemoveWaterStream = static_cast<int>(s);
	}
	return NumStreamBeUpdated > 0 ? true : false;
}

bool TextureWaterTrackEffect::WaterWashBlood(Ogre::ManualObject * waterQuads)
{
	int bleedupdatedCount = 0;

	Ogre::ColourValue bleedcolor(1.0f , 1.0f , 1.0f , 1.0f);

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

				waterQuads->position(quadCenter+rotation*Ogre::Vector3(realradius, -realradius, 0));
				waterQuads->textureCoord(1, 1);
				waterQuads->colour(bleedcolor);

				waterQuads->position(quadCenter+rotation*Ogre::Vector3(-realradius, realradius, 0));
				waterQuads->textureCoord(0, 0);
				waterQuads->colour(bleedcolor);

				waterQuads->position(quadCenter+rotation*Ogre::Vector3(-realradius, -realradius, 0));
				waterQuads->textureCoord(0, 1);
				waterQuads->colour(bleedcolor);

				waterQuads->position(quadCenter+rotation*Ogre::Vector3(realradius, realradius, 0));
				waterQuads->textureCoord(1, 0);
				waterQuads->colour(bleedcolor);

				//using indices
				waterQuads->index(0+startIndex);
				waterQuads->index(1+startIndex);
				waterQuads->index(2+startIndex);

				waterQuads->index(0+startIndex);
				waterQuads->index(3+startIndex);
				waterQuads->index(1+startIndex);

				startIndex += 4;

				bleedupdatedCount++;
			}

		}
	}
	return (bleedupdatedCount > 0 ? true : false);
}
