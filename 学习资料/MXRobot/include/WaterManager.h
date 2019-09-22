#pragma once
#include "MXOgreWrapper.h"
#include "Ogre.h"
#include <vector>
#include "shadowmap.h"
#include "collision\BroadPhase\GoPhysDynBVTree.h"


struct WaterColumnVertex
{
	Ogre::Vector3 m_v3Position;
	Ogre::Vector3 m_v3Normal;
	Ogre::Vector3 m_v3Tangent;
	Ogre::Vector3 m_v3Binormal;
	Ogre::Real m_realU;
	Ogre::Real m_realV;
};

class WaterColumn
{
public:
	WaterColumn(Ogre::Real fTime = 1.0f, Ogre::uint32 nSegmentNum = 18, Ogre::uint32 nDetails = 8);
	~WaterColumn(void);
	//������ˮ��λ��
	void SetPosition(Ogre::Vector3 p);
	//������ˮ���ٶ�
	void SetVelocity(Ogre::Vector3 v);
	//������ˮ�ڰ뾶
	void SetRadius(Ogre::Real r);
	//�����������ٶ�
	void SetGravity(Ogre::Vector3 g);
	//���ˮ���ڵ�����
	std::vector<Ogre::Vector3>& getSegmentsPos(void) {return m_vectorSegmentsPos;}
	//��ö�������
	std::vector<WaterColumnVertex>& getVertex(void) { return m_vectorVertex; }
	//�������
	std::vector<Ogre::uint32>& getIndices(void) { return m_vectorIndices; }
	//�����ˮ�ڰ뾶
	Ogre::Real GetRadius() { return m_realRadius; }
	//ˢ��ģ��
	void UpdateSegment(void);
	//���ˮ���ٶ�
	Ogre::Real getWaterSpeed(void) { return m_realSpeed; }
	//�������䳤��
	void setWaterColumnLength(Ogre::Real l);
	//�ж��Ƿ���Ҫˢ��
	bool isNeedUpdate(Ogre::Vector3 v3Position, Ogre::Vector3 v3Velocity, Ogre::Real realRadius, Ogre::Real reaLength);
	//������������ײ
	void UpdateCollideInfo(ITraining *pTraining);
	//���һ���������ĵ�
	bool GetSegmentNode(int segment , Ogre::Vector3 & node0 ,  Ogre::Vector3 & node1);
	//��ײ�����Ⱦ,����ʵ�ʶ�����
	int Draw(Ogre::ManualObject *pManual , float t , int indexOffset);
private:
	//������ײ��
	void UpdateCollideTree();

	bool			m_bIsSetPos;
	bool			m_bIsSetVelocity;
	Ogre::Vector3	m_v3Pos;//��ˮ��λ��
	Ogre::Vector3	m_v3Velocity;//�ٶ�
	Ogre::Real		m_realSpeed;//m_v3Velocity.length();
	//std::vector<Ogre::Vector3> m_vectorSegments;
	std::vector<Ogre::Vector3> m_vectorSegmentsPos;
	int m_ActualSegmentNum;
	Ogre::uint32	m_nSegmentNum;
	Ogre::uint32	m_nDetails;
	Ogre::Real		m_realLength;
	Ogre::Real		m_realTime;//������Ŀ�ĵص�ʱ��
	Ogre::Vector3	m_realGravity;//�������ٶ�
	Ogre::Real		m_realRadius;//����ˮ���Ľ���뾶

	std::vector<WaterColumnVertex> m_vectorVertex;
	std::vector<Ogre::uint32> m_vectorIndices;
	
	//��ײ��
	GFPhysDBVTree m_SegmentTree;
	//��Ⱦ��ײ���һ�ڵ���������
	float m_CurrLastSegmentScaleFactor;
};

class WaterManager : public CShadowMap::ShadowRendListener
{
public:
	WaterManager(void);
	~WaterManager(void);

	void SetCurrTraining(ITraining *pTraining);

	int addWaterColumn(Ogre::Vector3 v3Position, Ogre::Vector3 v3Velocity, Ogre::Real realRadius, Ogre::Real reaLength = 5.0f);

	bool setWaterColumn(int nWaterColumn, Ogre::Vector3 v3Position, Ogre::Vector3 v3Velocity, Ogre::Real realRadius, Ogre::Real reaLength = 5.0f);

	bool delWaterColumn(int nWaterColumn);

	void update(float dt);

	void SetGravity(Ogre::Vector3 g);

	std::vector<Ogre::Vector3>& GetSegmentPos(Ogre::uint32 nWaterColumn);

	bool IsWaterColumnExsit() { return !m_mapWaterColumn.empty();}

	//@overridden shadow map listener
	virtual void preRenderShadowDepth();
	virtual void postRenderShadowDepth();

private:
	ITraining *m_pCurrTraining;
	Ogre::ManualObject *m_manual;
	Ogre::Camera *m_camera;
	std::map<Ogre::uint32,WaterColumn*>	m_mapWaterColumn;
	Ogre::Vector3	m_realGravity;

	Ogre::uint32	m_nSerial;
};



