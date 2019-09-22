#ifndef _TEXTURE_WATER_EFFECT_
#define _TEXTURE_WATER_EFFECT_

#include "TextureBloodEffect.h"

class TextureWaterTrackEffect : public TextureBloodTrackEffect
{
public:
	TextureWaterTrackEffect(MisMedicOrganInterface * organif);
	~TextureWaterTrackEffect();

	bool Update(float dt);

	OrganSurfaceBloodTextureTrack * createWaterTrack(MisMedicOrgan_Ordinary * organ , float weights[3] ,  float mass  , int initfaceid, float radius/* = 0.0027f*/, float fDir = 0.0f);

	bool removeWaterTrack(size_t p);
	
	//�ڶ�̬����������ʵ��ˮ���߹�Ч��
	//bool BuildWaterStream(Ogre::ManualObject * waterQuads);
	
	//��Ѫ�������ϻ��ư�͸����ˮ��Ч��
	bool BuildWaterStreamTex(Ogre::ManualObject * bloodQuads);

	bool WaterWashBlood(Ogre::ManualObject * waterQuads);

private:
	DWORD m_nFrameNum;
	std::vector<float> m_vectorFrameRand;

	DWORD m_nWaterSpeed;//ms
	int m_nRemoveWaterStream;
};

#endif//#ifndef _TEXTURE_WATER_EFFECT_