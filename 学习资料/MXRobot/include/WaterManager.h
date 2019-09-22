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
	//设置喷水口位置
	void SetPosition(Ogre::Vector3 p);
	//设置喷水的速度
	void SetVelocity(Ogre::Vector3 v);
	//设置喷水口半径
	void SetRadius(Ogre::Real r);
	//设置重力加速度
	void SetGravity(Ogre::Vector3 g);
	//获得水柱节点坐标
	std::vector<Ogre::Vector3>& getSegmentsPos(void) {return m_vectorSegmentsPos;}
	//获得顶点坐标
	std::vector<WaterColumnVertex>& getVertex(void) { return m_vectorVertex; }
	//获得索引
	std::vector<Ogre::uint32>& getIndices(void) { return m_vectorIndices; }
	//获得喷水口半径
	Ogre::Real GetRadius() { return m_realRadius; }
	//刷新模型
	void UpdateSegment(void);
	//获得水流速度
	Ogre::Real getWaterSpeed(void) { return m_realSpeed; }
	//设置喷射长度
	void setWaterColumnLength(Ogre::Real l);
	//判断是否需要刷新
	bool isNeedUpdate(Ogre::Vector3 v3Position, Ogre::Vector3 v3Velocity, Ogre::Real realRadius, Ogre::Real reaLength);
	//更新与器官碰撞
	void UpdateCollideInfo(ITraining *pTraining);
	//获得一节两个中心点
	bool GetSegmentNode(int segment , Ogre::Vector3 & node0 ,  Ogre::Vector3 & node1);
	//碰撞后的渲染,返回实际顶点数
	int Draw(Ogre::ManualObject *pManual , float t , int indexOffset);
private:
	//更新碰撞树
	void UpdateCollideTree();

	bool			m_bIsSetPos;
	bool			m_bIsSetVelocity;
	Ogre::Vector3	m_v3Pos;//喷水口位置
	Ogre::Vector3	m_v3Velocity;//速度
	Ogre::Real		m_realSpeed;//m_v3Velocity.length();
	//std::vector<Ogre::Vector3> m_vectorSegments;
	std::vector<Ogre::Vector3> m_vectorSegmentsPos;
	int m_ActualSegmentNum;
	Ogre::uint32	m_nSegmentNum;
	Ogre::uint32	m_nDetails;
	Ogre::Real		m_realLength;
	Ogre::Real		m_realTime;//出发到目的地的时间
	Ogre::Vector3	m_realGravity;//重力加速度
	Ogre::Real		m_realRadius;//喷射水柱的截面半径

	std::vector<WaterColumnVertex> m_vectorVertex;
	std::vector<Ogre::uint32> m_vectorIndices;
	
	//碰撞树
	GFPhysDBVTree m_SegmentTree;
	//渲染碰撞最后一节的缩放因子
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



