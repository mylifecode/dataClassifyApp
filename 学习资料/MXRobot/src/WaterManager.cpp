#include "WaterManager.h"
#include "MisNewTraining.h"

//===============================================================================================================
//used for water with organ

class WaterTouchFace
{
public:
	WaterTouchFace() {}
	WaterTouchFace(int segIndex ,MisMedicOrgan_Ordinary *pOrgan , GFPhysSoftBodyFace *pFace , GFPhysVector3 & closestPoint) 
		: m_SegIndex(segIndex) , 
		m_pTouchOrgan(pOrgan) ,  
		m_pTouchFace(pFace) , 
		m_ClosestPoint(closestPoint) {}
	int m_SegIndex;
	MisMedicOrgan_Ordinary *m_pTouchOrgan;
	GFPhysSoftBodyFace * m_pTouchFace;
	GFPhysVector3 m_ClosestPoint;
};

class WaterColumnCollideSoftBodyFaceCallBack : public GFPhysNodeOverlapCallback
{
public:
	WaterColumnCollideSoftBodyFaceCallBack(WaterColumn* pWaterColumn , float threshold)
		: m_pWaterColumn(pWaterColumn) ,
		  m_pIntersectedFace(NULL),
		  m_pIntersectedOrgan(NULL),
		  m_Threshold(threshold) , 
		  m_IsIntersect(false) , 
		  m_IsTouch(false),
		  m_SegmentOfIntersection(INT_MAX),
		  m_pCurrOrgan(NULL)
	{	
	}

	void SetCurrOrgan(MisMedicOrgan_Ordinary *pOrgan) { m_pCurrOrgan = pOrgan; }

	void ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData)
	{
// 		GFPhysSoftBodyFace * pFace = (GFPhysSoftBodyFace*)UserData;
// 
// 		std::vector<Ogre::Vector3>& SegmentsPos = m_pWaterColumn->getSegmentsPos();
// 		
// 		float radius = m_pWaterColumn->GetRadius();
// 
// 		for(size_t s = 0 ; s < SegmentsPos.size() ; s++)
// 		{
// 			GFPhysVector3 currPos = OgreToGPVec3(SegmentsPos[s]);
// 			GFPhysVector3 closetPoint = ClosestPtPointTriangle(currPos, 
// 				pFace->m_Nodes[0]->m_CurrPosition,
// 				pFace->m_Nodes[1]->m_CurrPosition,
// 				pFace->m_Nodes[2]->m_CurrPosition);
// 
// 			GFPhysVector3 diff = currPos - closetPoint;
// 			if(diff.Dot(pFace->m_FaceNormal) >= 0 && diff.Length() < radius && s < m_SegmentOfIntersection)
// 			{
// 				m_IsIntersect = true;
// 				m_pIntersectedFace = pFace;
// 				m_SegmentOfIntersection = s;
// 			}
// 		}
	}

	virtual void ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA , GFPhysDBVNode * dynNodeB)
	{
		GFPhysSoftBodyFace * pSoftFace = (GFPhysSoftBodyFace*)dynNodeA->m_UserData;
		
		int SegIndex = (int)dynNodeB->m_UserData;
		
		Ogre::Vector3 node0 , node1;

		bool isExist = m_pWaterColumn->GetSegmentNode(SegIndex , node0 , node1);

		if(isExist)
		{
			//已有segment与face相交，且当前segment 比相交segment后
			if(m_IsIntersect && SegIndex > m_SegmentOfIntersection)
			{
				return;
			}
			else
			{
				GFPhysVector3 n0 = OgreToGPVec3(node0);

				GFPhysVector3 n1 = OgreToGPVec3(node1);

				Real rayWeight;
				GFPhysVector3 intersectpt;
				Real triangleWeight[3];

				//先检测是否相交
				bool isIntersect = LineIntersectTriangle(pSoftFace->m_Nodes[0]->m_CurrPosition , 
					pSoftFace->m_Nodes[1]->m_CurrPosition , 
					pSoftFace->m_Nodes[2]->m_CurrPosition , 
					n0 , 
					n1 ,
					rayWeight , 
					intersectpt , triangleWeight);
				
				if(m_IsIntersect && SegIndex == m_SegmentOfIntersection  && isIntersect && rayWeight >= 0 && rayWeight <= 1)
				{
					if(rayWeight < m_ScaleFactor)
					{
						m_IsIntersect = true;
						m_SegmentOfIntersection = SegIndex;
						m_pIntersectedOrgan = m_pCurrOrgan;
						m_pIntersectedFace = pSoftFace;
						std::copy(triangleWeight,triangleWeight+3,m_IntersectionWeight);
						m_ScaleFactor = rayWeight;
					}
				}
				else if(SegIndex < m_SegmentOfIntersection  && isIntersect && rayWeight >= 0 && rayWeight <= 1)
				{
					m_IsIntersect = true;
					m_SegmentOfIntersection = SegIndex;
					m_pIntersectedOrgan = m_pCurrOrgan;
					m_pIntersectedFace = pSoftFace;
					std::copy(triangleWeight,triangleWeight+3,m_IntersectionWeight);
					m_ScaleFactor = rayWeight;
				}
				//检测是否擦过
				else if(m_TouchFaces.find(SegIndex) == m_TouchFaces.end())
				{
					GFPhysVector3 closetPoint = ClosestPtPointTriangle(n0, 
					pSoftFace->m_Nodes[0]->m_CurrPosition,
					pSoftFace->m_Nodes[1]->m_CurrPosition,
					pSoftFace->m_Nodes[2]->m_CurrPosition);

					GFPhysVector3 segLine = n1 - n0;
					Real seglength = segLine.Length();
					segLine.Normalize();
					GFPhysVector3 diff = closetPoint - n0;
					Real parallelComponent = diff.Dot(segLine);
					if(parallelComponent <= seglength)
					{
						GFPhysVector3 verticalVec = diff - parallelComponent * segLine;
						if(verticalVec.Length() < m_pWaterColumn->GetRadius())
						{
							m_TouchFaces[SegIndex] = WaterTouchFace(SegIndex,m_pCurrOrgan,pSoftFace , closetPoint);
						}
					}
				}
			}
		}
	}

	bool IsIntersect() { return m_IsIntersect;}
	int GetSegmentIndexOfIntersection() { return m_SegmentOfIntersection;}
	GFPhysSoftBodyFace *GetIntersectedFace() { return m_pIntersectedFace;}
	MisMedicOrgan_Ordinary *GetIntersectedOrgan() { return m_pIntersectedOrgan;}
	void GetIntersectionWeight(float weights[3]) { std::copy(m_IntersectionWeight , m_IntersectionWeight+3 , weights);}

	std::map<int, WaterTouchFace> m_TouchFaces;
	float m_ScaleFactor;

private:
	WaterColumn *m_pWaterColumn;
	GFPhysSoftBodyFace * m_pIntersectedFace;
	MisMedicOrgan_Ordinary * m_pIntersectedOrgan;
	Real m_IntersectionWeight[3];
	MisMedicOrgan_Ordinary * m_pCurrOrgan;
	float m_Threshold;
	bool m_IsIntersect;
	bool m_IsTouch;
	int m_SegmentOfIntersection;
	//std::vector<TouchFace> m_TouchFaces;

};
//===============================================================================================================
WaterColumn::WaterColumn(Ogre::Real fTime, Ogre::uint32 nSegmentNum, Ogre::uint32 nDetails)
{
	m_realTime = fTime;
	m_nSegmentNum = nSegmentNum;
	m_ActualSegmentNum = m_nSegmentNum;
	m_nDetails = nDetails;
	m_bIsSetPos = false;
	m_bIsSetVelocity = false;
	m_realSpeed = 1.0f;
	m_CurrLastSegmentScaleFactor = 1.0f;

	//m_vectorSegments.resize(m_nSegmentNum);
	m_vectorSegmentsPos.resize(m_nSegmentNum);

	m_vectorVertex.resize(m_nSegmentNum*m_nDetails);
	m_vectorIndices.resize((m_nSegmentNum-1)*m_nDetails*6);
	
	for(Ogre::uint32 i = 0; i<m_nSegmentNum; ++i)
	{
		for(Ogre::uint32 j = 0; j<m_nDetails; ++j)
		{
			//生成纹理坐标
			Ogre::Real u = j*2.0f/m_nDetails;
			if(u>1.0f)
				u = 2.0f-u;
			m_vectorVertex[i*m_nDetails+j].m_realU = u;

			m_vectorVertex[i*m_nDetails+j].m_realV = i*4.0f/m_nSegmentNum;
			if(i==m_nSegmentNum-1)
				continue;
			//生成索引
			Ogre::uint32 n = j+1;
			if(n==m_nDetails)
				n = 0;
			m_vectorIndices[(i*m_nDetails+j)*6] = i*m_nDetails+j;
			m_vectorIndices[(i*m_nDetails+j)*6+1] = (i+1)*m_nDetails+n;
			m_vectorIndices[(i*m_nDetails+j)*6+2] = (i+1)*m_nDetails+j;

			m_vectorIndices[(i*m_nDetails+j)*6+3] = i*m_nDetails+j;
			m_vectorIndices[(i*m_nDetails+j)*6+4] = i*m_nDetails+n;
			m_vectorIndices[(i*m_nDetails+j)*6+5] = (i+1)*m_nDetails+n;
		}
	}
}

WaterColumn::~WaterColumn(void)
{
}

//设置喷水口位置
void WaterColumn::SetPosition(Ogre::Vector3 p)
{
	m_v3Pos = p;
	m_bIsSetPos = true;
}

//设置喷水的速度
void WaterColumn::SetVelocity(Ogre::Vector3 v)
{
	m_v3Velocity = v;
	m_realSpeed = m_v3Velocity.length();
	m_bIsSetVelocity = true;
}

//设置喷水口半径
void WaterColumn::SetRadius(Ogre::Real r)
{
	m_realRadius = r;
}

//设置重力加速度
void WaterColumn::SetGravity(Ogre::Vector3 g)
{
	m_realGravity = g;
}

void WaterColumn::UpdateSegment(void)
{
	if(!m_bIsSetVelocity||!m_bIsSetPos)
		return;
	Ogre::Vector3 vGravity = m_realGravity;
	Ogre::Vector3 vX = m_v3Velocity;
	vX.y = 0;//水平速度
	Ogre::Vector3 vY = m_v3Velocity;
	vY.x = 0;
	vY.z = 0;//垂直速度
	for(Ogre::uint32 i = 0; i<m_nSegmentNum; ++i)
	{
		Ogre::Real t = i*m_realTime/m_nSegmentNum;
		Ogre::Vector3 posX = vX*t;
		Ogre::Vector3 posY = vY*t+vGravity*t*t*0.5f;
		m_vectorSegmentsPos[i] = m_v3Pos+posX+posY;
	}
	//刷新顶点坐标
	Ogre::Vector3 aheadDir(0.0f,0.0f,0.0f);
	Ogre::Real realAngle = Ogre::Math::PI*2.0f/(m_nDetails-1);
	Ogre::Real realRight = m_realRadius*Ogre::Math::Tan(realAngle);
	Ogre::Real realCos = Ogre::Math::Cos(realAngle);
	for(Ogre::uint32 i = 0; i<m_nSegmentNum; ++i)
	{
		Ogre::Vector3 curDir = aheadDir;
		if(i<m_nSegmentNum-1)
			curDir = (aheadDir+(m_vectorSegmentsPos[i+1]-m_vectorSegmentsPos[i]).normalisedCopy()).normalisedCopy();

		Ogre::Vector3 aheadRight = curDir.crossProduct(Ogre::Vector3(0.0f,1.0f,0.0f));
		aheadRight.normalise();
		Ogre::Vector3 aheadOffs = aheadRight.crossProduct(curDir).normalisedCopy()*m_realRadius;
		m_vectorVertex[i*m_nDetails].m_v3Position = aheadOffs+m_vectorSegmentsPos[i];
		m_vectorVertex[i*m_nDetails].m_v3Normal = aheadOffs.normalisedCopy();
		m_vectorVertex[i*m_nDetails].m_v3Tangent = aheadRight;
		m_vectorVertex[i*m_nDetails].m_v3Binormal = curDir;
		for(Ogre::uint32 j = 1; j<m_nDetails; ++j)
		{
			Ogre::Vector3 curOffs = (aheadOffs+aheadRight*realRight)*realCos;
			m_vectorVertex[i*m_nDetails+j].m_v3Position = curOffs+m_vectorSegmentsPos[i];
			aheadRight = curDir.crossProduct(curOffs).normalisedCopy();
			aheadOffs = curOffs;

			m_vectorVertex[i*m_nDetails+j].m_v3Normal = curOffs.normalisedCopy();
			m_vectorVertex[i*m_nDetails+j].m_v3Tangent = aheadRight;
			m_vectorVertex[i*m_nDetails+j].m_v3Binormal = curDir;
		}
		aheadDir = curDir;
	}
}

//设置喷射长度
void WaterColumn::setWaterColumnLength(Ogre::Real l)
{
	m_realTime = l/m_realSpeed;
	m_realLength = l;
}

//判断是否需要刷新
bool WaterColumn::isNeedUpdate(Ogre::Vector3 v3Position, Ogre::Vector3 v3Velocity, Ogre::Real realRadius, Ogre::Real reaLength)
{
	Ogre::Real d = 0.001f;
	if((v3Velocity-m_v3Velocity).length()>d)
		return true;
	if((v3Position-m_v3Pos).length()>d)
		return true;
	if(realRadius-m_realRadius>d)
		return true;
	if(reaLength-m_realLength)
		return true;
	return false;
}



void WaterColumn::UpdateCollideInfo(ITraining *pTraining)
{
	GFPhysVector3 aabbMin , aabbMax;
	if(m_nSegmentNum > 0)
	{
		UpdateCollideTree();

		WaterColumnCollideSoftBodyFaceCallBack cb(this , 0);

		std::vector<MisMedicOrganInterface*> pOrgans;
		pTraining->GetAllOrgan(pOrgans);
		for(size_t o = 0 ; o < pOrgans.size() ; o++)
		{
			MisMedicOrganInterface *pOrganInterface = pOrgans[o];
			if(pOrganInterface->GetCreateInfo().m_objTopologyType ==  DOT_VOLMESH || pOrganInterface->GetCreateInfo().m_objTopologyType == DOT_MEMBRANE)
			{
				MisMedicOrgan_Ordinary *pOrgan = dynamic_cast<MisMedicOrgan_Ordinary*>(pOrganInterface);
				if(pOrgan && pOrgan->m_physbody)
				{
					cb.SetCurrOrgan(pOrgan);

					GFPhysVectorObj<GFPhysDBVTree*> bvTrees = pOrgan->m_physbody->GetSoftBodyShape().GetFaceBVTrees();

					for(size_t t = 0 ; t < bvTrees.size() ; t++)
					{
						GFPhysDBVTree * bvTree = bvTrees[t];
						bvTree->CollideWithDBVTree(m_SegmentTree , &cb);
					}
				}
			}
		}

		if(cb.IsIntersect())
		{
			m_ActualSegmentNum = cb.GetSegmentIndexOfIntersection() + 1;
			float weights[3];
			cb.GetIntersectionWeight(weights);
			cb.GetIntersectedOrgan()->createWaterTrack(cb.GetIntersectedFace(),weights,0,true);
			m_CurrLastSegmentScaleFactor = cb.m_ScaleFactor;
		}
		else
		{
			m_CurrLastSegmentScaleFactor = 1.0f;
			m_ActualSegmentNum = m_nSegmentNum - 1;
		}
		//擦过的面创建水流
		std::map	<int,WaterTouchFace>::iterator faceItor = cb.m_TouchFaces.begin();
		while(faceItor != cb.m_TouchFaces.end())
		{
			WaterTouchFace & touchFace = faceItor->second;
			if((cb.IsIntersect() && touchFace.m_SegIndex < cb.GetSegmentIndexOfIntersection()) || (!cb.IsIntersect()) )
			{
				float weights[3] = {0.333,0.333,0.333};
				touchFace.m_pTouchOrgan->createWaterTrack(touchFace.m_pTouchFace,weights,0,true);
			}
			faceItor++;
		}
	}
}
//
bool	 WaterColumn::GetSegmentNode(int segment , Ogre::Vector3 & node0 , Ogre::Vector3 & node1)
{
	if(segment >= (m_nSegmentNum -1))
		return false;
	else
	{
		node0 = m_vectorSegmentsPos[segment];
		node1 = m_vectorSegmentsPos[segment + 1];
		return true;
	}
}
//碰撞后的渲染,返回实际顶点数
int WaterColumn::Draw(Ogre::ManualObject *pManual , float t , int indexOffset)
{
	int headNum = m_ActualSegmentNum * m_nDetails;	//the num of vertices is (m_ActualSegmentNum + 1) * m_nDetails 
	int indicesNum =  m_ActualSegmentNum * m_nDetails * 6;

	for(int v = 0 ; v < headNum ; v++)
	{
		WaterColumnVertex & vertex = m_vectorVertex[v];
		pManual->position(vertex.m_v3Position);
		pManual->textureCoord(vertex.m_realU,vertex.m_realV);
		pManual->textureCoord(t);
		pManual->textureCoord(vertex.m_v3Normal);
		pManual->textureCoord(vertex.m_v3Binormal);
		pManual->textureCoord(vertex.m_v3Tangent);
	}
	for(int v = headNum ; v < headNum + m_nDetails ; v++)
	{
		WaterColumnVertex & vertex = m_vectorVertex[v];
		WaterColumnVertex & lastEnd = m_vectorVertex[v - m_nDetails];
		pManual->position(lastEnd.m_v3Position + ( vertex.m_v3Position - lastEnd.m_v3Position) * m_CurrLastSegmentScaleFactor + 0.01);
		pManual->textureCoord(vertex.m_realU,vertex.m_realV);
		pManual->textureCoord(t);
		pManual->textureCoord(vertex.m_v3Normal);
		pManual->textureCoord(vertex.m_v3Binormal);
		pManual->textureCoord(vertex.m_v3Tangent);
		
	}

	for(int i = 0 ; i <  indicesNum; i++)
	{
			pManual->index(m_vectorIndices[i] + indexOffset);
	}

	return (m_ActualSegmentNum + 1) * m_nDetails;
}

//更新碰撞树
void WaterColumn::UpdateCollideTree()
{
	m_SegmentTree.Clear();
	
	GFPhysVector3 extend(m_realRadius,m_realRadius,m_realRadius);
	
	for(int i = 0; i < m_nSegmentNum - 1; ++i)
	{
		GFPhysVector3 node0 =  OgreToGPVec3(m_vectorSegmentsPos[i]);
		GFPhysVector3 node1 =  OgreToGPVec3(m_vectorSegmentsPos[i + 1]);

		GFPhysVector3 aabbMin = node0;
		GFPhysVector3 aabbMax = node0;

		aabbMin.SetMin(node1);
		aabbMax.SetMax(node1);

		aabbMin -= extend;
		aabbMax += extend;

		GFPhysDBVNode * pTreeNode = m_SegmentTree.InsertAABBNode(aabbMin , aabbMax);
		pTreeNode->m_UserData = (void *)i;
	}
}
WaterManager::WaterManager(void)
{
	m_manual =MXOgreWrapper::Get()->GetDefaultSceneManger()->createManualObject();
	m_manual->setDynamic(true);
	m_manual->begin("waterColumn");

	m_manual->position(0,0,0);
	
	m_manual->textureCoord(0,0);
	m_manual->textureCoord(1);
	m_manual->textureCoord(0,0,0);
	m_manual->textureCoord(0,0,0);
	m_manual->textureCoord(0,0,0);

	m_manual->position(0,0,0);
	
	m_manual->textureCoord(0,1);
	m_manual->textureCoord(1);
	m_manual->textureCoord(0,0,0);
	m_manual->textureCoord(0,0,0);
	m_manual->textureCoord(0,0,0);

	m_manual->position(0,0,0);
	m_manual->textureCoord(1,0);
	m_manual->textureCoord(1);
	m_manual->textureCoord(0,0,0);
	m_manual->textureCoord(0,0,0);
	m_manual->textureCoord(0,0,0);

	m_manual->index(0);
	m_manual->index(2);
	m_manual->index(1);

	m_manual->end();

	m_manual->setRenderQueueGroup(Ogre::RENDER_QUEUE_MAIN+1);//临时解决水柱和血池alpha排序错误问题需要检查！！
	MXOgreWrapper::Get()->GetDefaultSceneManger()->getRootSceneNode()->attachObject(m_manual);

	m_realGravity = Ogre::Vector3(0.0f,-9.8f,0.0f);
	m_nSerial = 0;
}

WaterManager::~WaterManager(void)
{
	m_manual->detachFromParent();
	MXOgreWrapper::Get()->GetDefaultSceneManger()->destroyManualObject(m_manual);

	for(std::map<Ogre::uint32,WaterColumn*>::iterator it = m_mapWaterColumn.begin(); it!=m_mapWaterColumn.end();++it)
		delete it->second;
	m_mapWaterColumn.clear();
}

void WaterManager::SetCurrTraining(ITraining *pTraining)
{
	m_pCurrTraining = pTraining;
}
int WaterManager::addWaterColumn(Ogre::Vector3 v3Position, Ogre::Vector3 v3Velocity, Ogre::Real realRadius, Ogre::Real reaLength)
{
	Ogre::Vector3 v = v3Velocity;
	v.y = 0.0f;
	if(v.length()<0.01f)
		return -1;//水平速度过小会导致模型过大出错

	m_mapWaterColumn[m_nSerial] = new WaterColumn();
	m_mapWaterColumn[m_nSerial]->SetPosition(v3Position);
	m_mapWaterColumn[m_nSerial]->SetRadius(realRadius);
	m_mapWaterColumn[m_nSerial]->SetVelocity(v3Velocity);
	m_mapWaterColumn[m_nSerial]->setWaterColumnLength(reaLength);
	m_mapWaterColumn[m_nSerial]->SetGravity(m_realGravity);
	m_mapWaterColumn[m_nSerial]->UpdateSegment();
	return m_nSerial++;
}

bool WaterManager::setWaterColumn(int nWaterColumn, Ogre::Vector3 v3Position, Ogre::Vector3 v3Velocity, Ogre::Real realRadius, Ogre::Real reaLength)
{
	if(m_mapWaterColumn.find(nWaterColumn)==m_mapWaterColumn.end())
		return false;
	if(m_mapWaterColumn[nWaterColumn]->isNeedUpdate(v3Position,v3Velocity,realRadius,reaLength))
	{
		m_mapWaterColumn[nWaterColumn]->SetPosition(v3Position);
		m_mapWaterColumn[nWaterColumn]->SetRadius(realRadius);
		m_mapWaterColumn[nWaterColumn]->SetVelocity(v3Velocity);
		m_mapWaterColumn[nWaterColumn]->setWaterColumnLength(reaLength);
		m_mapWaterColumn[nWaterColumn]->UpdateSegment();
	}
	return true;
}

bool WaterManager::delWaterColumn(int nWaterColumn)
{
	std::map<Ogre::uint32,WaterColumn*>::iterator it = m_mapWaterColumn.find(nWaterColumn);
	if(it==m_mapWaterColumn.end())
		return false;
	delete it->second;
	m_mapWaterColumn.erase(it);

	m_manual->beginUpdate(0);

	m_manual->position(0,0,0);
	m_manual->textureCoord(0,0);
	m_manual->textureCoord(1);

	m_manual->position(0,0,0);
	m_manual->textureCoord(0,1);
	m_manual->textureCoord(1);

	m_manual->position(0,0,0);
	m_manual->textureCoord(1,0);
	m_manual->textureCoord(1);

	m_manual->index(0);
	m_manual->index(2);
	m_manual->index(1);

	m_manual->end();
	return true;
}

void WaterManager::update(float dt)
{
	for(std::map<Ogre::uint32,WaterColumn*>::iterator it = m_mapWaterColumn.begin(); it!=m_mapWaterColumn.end();++it)
	{
		WaterColumn* pWaterColumn = it->second;
		pWaterColumn->UpdateCollideInfo(m_pCurrTraining);
	}
	
	int indexOffset = 0;
	m_manual->beginUpdate(0);
	for(std::map<Ogre::uint32,WaterColumn*>::iterator it = m_mapWaterColumn.begin(); it!=m_mapWaterColumn.end();++it)
	{
		ULONGLONG T = ::GetTickCount64();
		ULONGLONG C = static_cast<ULONGLONG>(it->second->getWaterSpeed()/0.0025f);
		Ogre::Real t = (T%C)*1.0f/C;
//		std::vector<WaterColumnVertex>& vectorVertex = it->second->getVertex();
//		std::vector<Ogre::uint32>& vectorIndices = it->second->getIndices();
// 		for(std::vector<WaterColumnVertex>::iterator itVertex = vectorVertex.begin(); itVertex!=vectorVertex.end(); ++itVertex)
// 		{
// 			m_manual->position(itVertex->m_v3Position);
// 			m_manual->textureCoord(itVertex->m_realU,itVertex->m_realV);
// 			m_manual->textureCoord(t);
// 		}
// 		for(std::vector<Ogre::uint32>::iterator itIndices = vectorIndices.begin(); itIndices!=vectorIndices.end(); ++itIndices)
// 			m_manual->index(*itIndices);
		
		//碰撞后的渲染
		indexOffset += it->second->Draw(m_manual , t , indexOffset);
	}
	m_manual->end();
}

void WaterManager::SetGravity(Ogre::Vector3 g)
{
	m_realGravity = g;
}

std::vector<Ogre::Vector3>& WaterManager::GetSegmentPos(Ogre::uint32 nWaterColumn)
{
	static std::vector<Ogre::Vector3> vectorNULL;
	std::map<Ogre::uint32,WaterColumn*>::iterator it = m_mapWaterColumn.find(nWaterColumn);
	if(it==m_mapWaterColumn.end()||it->second==NULL)
		return vectorNULL;
	return it->second->getSegmentsPos();
}

void WaterManager::preRenderShadowDepth()
{
	if(m_manual)
	   m_manual->setVisible(false);
}

void WaterManager::postRenderShadowDepth()
{
	if(m_manual)
	   m_manual->setVisible(true);
}


