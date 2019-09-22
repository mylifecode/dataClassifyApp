#include "ToolSpenetrateMgr.h"
#include "ITool.h"
#include "ToolsMgr.h"
#include "Instruments/Tool.h"
#include "InputSystem.h"
#include "MisMedicRigidPrimtive.h"
#include "./collision/NarrowPhase/GoPhysCCDBase.h"
#include <BasicTraining.h>

#define StaticCCheckFactor 1.1f
ToolSpenetrateMgr* ToolSpenetrateMgr::m_toolSpenetrateMgr = nullptr;

ToolSpenetrateMgr::ToolSpenetrateMgr()
{
	lastPosH0 = lastPosS0 = lastPosH1 = lastPosS1 = Ogre::Vector3::ZERO;
	lastOriH0 = lastOriS0 = lastOriH1 = lastOriS1 = Ogre::Quaternion::IDENTITY;
	k = 15;
}

ToolSpenetrateMgr::ToolSpenetrateMgr(double coefficient)
{
	lastPosH0=lastPosS0=lastPosH1=lastPosS1=Ogre::Vector3::ZERO;
	lastOriH0=lastOriS0=lastOriH1=lastOriS1=Ogre::Quaternion::IDENTITY;
	k=coefficient;
}

ToolSpenetrateMgr::~ToolSpenetrateMgr()
{
	m_staticRigidObjects.clear();
}

ToolSpenetrateMgr* ToolSpenetrateMgr::GetInstance()
{
	if (m_toolSpenetrateMgr == nullptr)
	{
		m_toolSpenetrateMgr = new ToolSpenetrateMgr;
	}

	return m_toolSpenetrateMgr;
}

void ToolSpenetrateMgr::Destroy()
{
	if (m_toolSpenetrateMgr)
	{
		delete m_toolSpenetrateMgr;
		m_toolSpenetrateMgr = nullptr;
	}
}

void ToolSpenetrateMgr::GetNearestPoints(Ogre::Vector3 P0,Ogre::Vector3 L0,Ogre::Vector3 P1,Ogre::Vector3 L1,double &t0,Ogre::Vector3 &Q0,double &t1,Ogre::Vector3 &Q1)
{
	Ogre::Vector3 a = L0-P0,b = L1-P1;
	Ogre::Vector3 p = P1-P0;
	double c=a.dotProduct(b),d=p.dotProduct(a),f=p.dotProduct(b),g=a.dotProduct(a),h=b.dotProduct(b);

	if(fabsf(a.crossProduct(b).length()) < FLT_EPSILON)
	{
		t0=t1=0;
		Q0=P0;Q1=P1;
	}
	else
	{
		if(fabsf(c) < FLT_EPSILON) 
		{
			t0=d/g;
			t1=-f/h;
		}
		else
		{
			t0=(d*h-c*f)/(g*h-c*c);
			t1=(t0*g-d)/c;
		}

		Q0=t0*a+P0;Q1=t1*b+P1;
	}
}

void ToolSpenetrateMgr::CorrectToolTransform(CTrainingMgr * m_pTrainingMgr,float dt)
{
	if(m_pTrainingMgr->GetCurTraining() == 0)
	   return;
	
	if(m_pTrainingMgr->GetCurTraining()->m_pToolsMgr == 0)
	   return;
	
	ITool *leftTool  = m_pTrainingMgr->GetCurTraining()->m_pToolsMgr->GetLeftTool();
	ITool *rightTool = m_pTrainingMgr->GetCurTraining()->m_pToolsMgr->GetRightTool();

	CorrectToolPositionByStaticRigidObjects((CTool*)leftTool, (CTool*)rightTool,dt);
	
	if (0)//leftTool && rightTool)
	{
		GFPhysVector3 force(0,0,0);
		((CTool *)leftTool)->m_ToolFightingForce=force;
		((CTool *)rightTool)->m_ToolFightingForce=force;

		float leftHeadLen  = ((CTool *)leftTool)->GetHeadPartCollisionLen();
		float rightHeadLen = ((CTool *)rightTool)->GetHeadPartCollisionLen();

		Ogre::Vector3 currentPosH0=leftTool->GetKernelNode()->getPosition();
		Ogre::Quaternion currentOriH0=leftTool->GetKernelNode()->getOrientation();

		Ogre::Vector3 currentPosH1=rightTool->GetKernelNode()->getPosition();
		Ogre::Quaternion currentOriH1=rightTool->GetKernelNode()->getOrientation();


		Ogre::Vector3 dietaPH0=currentPosH0-lastPosH0;	Ogre::Quaternion dietaOriH0=currentOriH0*lastOriH0.Inverse();
		Ogre::Vector3 dietaPH1=currentPosH1-lastPosH1;	Ogre::Quaternion dietaOriH1=currentOriH1*lastOriH1.Inverse();


		Ogre::Vector3 currentPosS0=lastPosS0+dietaPH0;	Ogre::Quaternion currentOriS0=currentOriH0;//dietaOriH0*lastOriS0;
		Ogre::Vector3 currentPosS1=lastPosS1+dietaPH1;	Ogre::Quaternion currentOriS1=currentOriH1;//*lastOriS1;		



		Ogre::Vector3 vSP;
		Ogre::Vector3 P0 = InputSystem::GetInstance(DEVICETYPE_LEFT)->GetPivotPosition();
		vSP=currentPosS0-P0;
		vSP.normalise();
		Ogre::Vector3 L0 = currentPosH0.distance(P0)*vSP+P0;	
		currentPosS0=L0;
		Ogre::Vector3 a = L0 - P0;

		Ogre::Vector3 P1 = InputSystem::GetInstance(DEVICETYPE_RIGHT)->GetPivotPosition();
		vSP=currentPosS1-P1;
		vSP.normalise();
		Ogre::Vector3 L1 = currentPosH1.distance(P1)*vSP+P1;
		currentPosS1=L1;
		Ogre::Vector3 b = L1 - P1;


		Ogre::Vector3 zS0=currentOriS0*Ogre::Vector3::UNIT_Z;
		currentOriS0=zS0.getRotationTo(currentPosS0-P0)*currentOriS0;

		Ogre::Vector3 zS1=currentOriS1*Ogre::Vector3::UNIT_Z;
		currentOriS1=zS1.getRotationTo(currentPosS1-P1) *currentOriS1;
		
		Ogre::Vector3 Q0,Q1;
		double t0,t1;

		float lenA = a.length();
		float lenB = b.length();
		
		if(lenA < 0.0001f || lenB < 0.0001f)
		   return;

		Ogre::Vector3 normA = a / lenA;
		Ogre::Vector3 normB = b / lenB;

		Ogre::Vector3 CorrectAvec = normA * (lenA-leftHeadLen);
		Ogre::Vector3 CorrectBvec = normB * (lenB-rightHeadLen);

		GetNearestPoints(P0 , P0 + CorrectAvec , P1 ,P1 + CorrectBvec ,t0,Q0,t1,Q1);

		double dis=Q1.distance(Q0);		
		if(dis<0.4 && t0>0 && t0<1 && t1>0 && t1<1)
		{
			Ogre::Vector3 axis;
			double jiaoDu=0;
			Ogre::Quaternion q;
			Ogre::Vector3 vAB;						
			if(dietaPH0 > dietaPH1)
			{
				axis=b.crossProduct(Q1-Q0);
				axis.normalise();

				jiaoDu=atan((0.4-dis)/P1.distance(Q1));
				q=Ogre::Quaternion(Ogre::Radian(jiaoDu),axis);				
				vAB=q*b;
				currentPosS1=vAB+P1;			
				Ogre::Vector3 zS1=currentOriS1*Ogre::Vector3::UNIT_Z;
				currentOriS1 = zS1.getRotationTo(vAB)*currentOriS1;						
				
			}
			else
			{
				axis=a.crossProduct(Q0-Q1);
				axis.normalise();

				jiaoDu=atan((0.4-dis)/P0.distance(Q0));
				q=Ogre::Quaternion(Ogre::Radian(jiaoDu),axis);				
				vAB=q*a;
				currentPosS0=vAB+P0;			
				Ogre::Vector3 zS0=currentOriS0*Ogre::Vector3::UNIT_Z;
				currentOriS0 = zS0.getRotationTo(vAB)*currentOriS0;
		
			}

		}

		else
		{

			float step = 0.001f;
			Ogre::Vector3 vH=currentPosH0-P0;
			Ogre::Radian max = a.angleBetween(vH);
			Ogre::Vector3 axis;
			double jiaoDu=0;
			Ogre::Quaternion q=Ogre::Quaternion::IDENTITY;
			Ogre::Vector3 temp,vAB;
			if (max > Ogre::Radian(0.01))
			{
				axis=a.crossProduct(vH);
				axis.normalise();				

				for (;Ogre::Radian(jiaoDu) <= max;jiaoDu += step)
				{
					q=Ogre::Quaternion(Ogre::Radian(jiaoDu),axis);
					temp=q*CorrectAvec;
					GetNearestPoints(P0,temp+P0,P1,P1+CorrectBvec,t0,Q0,t1,Q1);
					if(Q0.distance(Q1)<0.4 && t0>0 && t0<1 && t1>0 && t1<1)							
						break;
					else
						vAB=q*a;
				}				
				currentPosS0=vAB+P0;

				Ogre::Vector3 zS0=currentOriS0*Ogre::Vector3::UNIT_Z;
				currentOriS0 = zS0.getRotationTo(vAB)*currentOriS0;
			}
#if(1)
		
			//refresh vector
			CorrectAvec = (currentPosS0-P0);

			lenA = CorrectAvec.length();

			if(lenA < 0.0001f)
				return;

			CorrectAvec = CorrectAvec * ((lenA-leftHeadLen) / lenA);
#endif

			vH=currentPosH1-P1;
			max = b.angleBetween(vH);
			jiaoDu=0;
			if (max > Ogre::Radian(0.01))
			{
				axis=b.crossProduct(vH);
				axis.normalise();

				for (;Ogre::Radian(jiaoDu) <= max;jiaoDu += step)
				{
					q=Ogre::Quaternion(Ogre::Radian(jiaoDu),axis);
					temp=q*CorrectBvec;
					GetNearestPoints(P0,P0+CorrectAvec,P1,temp+P1,t0,Q0,t1,Q1);
					if(Q0.distance(Q1)<0.4 && t0>0 && t0<1 && t1>0 && t1<1)							
						break;
					else
						vAB=q*b;
				}

				currentPosS1=vAB+P1;

				Ogre::Vector3 zS1=currentOriS1*Ogre::Vector3::UNIT_Z;
				currentOriS1 = zS1.getRotationTo(vAB)*currentOriS1;
			}
#if(1)
			CorrectBvec = (currentPosS1-P1);

			lenB = CorrectBvec.length();

			if(lenB < 0.0001f)
			   return;

			CorrectBvec = CorrectBvec * ((lenB-rightHeadLen) / lenB);
#endif

		}													
		

		leftTool->GetKernelNode()->setPosition(currentPosS0);leftTool->GetKernelNode()->setOrientation(currentOriS0);
		rightTool->GetKernelNode()->setPosition(currentPosS1);rightTool->GetKernelNode()->setOrientation(currentOriS1);


		lastPosH0=currentPosH0;lastOriH0=currentOriH0;
		lastPosS0=currentPosS0;lastOriS0=currentOriS0;

		lastPosH1=currentPosH1;lastOriH1=currentOriH1;
		lastPosS1=currentPosS1;lastOriS1=currentOriS1;
		
		
		GetNearestPoints(P0,P0+CorrectAvec,P1,P1+CorrectBvec,t0,Q0,t1,Q1);
		/*if(Q0.distance(Q1)>0.5 || t0<0 || t0>1 || t1<0 || t1>1)	
			;
		else
		{*/
			
			Ogre::Radian dietaJiaoDu;	
			Ogre::Vector3 direction;
			//dis=currentPosH0.distance(currentPosS0);
			a=currentPosS0-P0;
			dietaJiaoDu=a.angleBetween(currentPosH0-P0);	
			if(dietaJiaoDu>Ogre::Radian(0.01))
			{
				//direction=currentPosH0-currentPosS0;
				direction=Q1-Q0;
				direction.normalise();
				 //dietaJiaoDu=a.angleBetween(currentPosH0-P0);						
				force=GFPhysVector3(k*dietaJiaoDu.valueRadians()*direction.x,k*dietaJiaoDu.valueRadians()*direction.y,k*dietaJiaoDu.valueRadians()*direction.z);
				
				((CTool *)leftTool)->m_ToolFightingForce=-force;
				((CTool *)rightTool)->m_ToolFightingForce=force;
				
			} 

			
			//dis=currentPosH1.distance(currentPosS1);
			b=currentPosS1-P1;
			dietaJiaoDu=b.angleBetween(currentPosH1-P1);
			if(dietaJiaoDu>Ogre::Radian(0.01))
			{				
				//direction=currentPosH1-currentPosS1;
				direction=Q0-Q1;
				direction.normalise();								
				//dietaJiaoDu=b.angleBetween(currentPosH1-P1);
				force=GFPhysVector3(k*dietaJiaoDu.valueRadians()*direction.x,k*dietaJiaoDu.valueRadians()*direction.y,k*dietaJiaoDu.valueRadians()*direction.z);
				((CTool *)leftTool)->m_ToolFightingForce+=force;
				((CTool *)rightTool)->m_ToolFightingForce+=-force;

			}	
		//}

		}
}

class ACToolRigidCallback : public GFPhysNodeOverlapCallback
{
public:

	struct CollidedFace
	{
		CollidedFace()
		{
			isValid = true;
			collideTime = -1.0f;
			isInnerTriangle = true;
			accumedNormalAdjust = 0;
		}

		void PrepareAdjust(const GFPhysVector3 & LastKernalPos)
		{
			distFromKNode = (LastKernalPos - toolPoint).Length();
			OriginDist = (toolPoint - facePoint).Dot(collideNormal);
			accumedNormalAdjust = 0;
		}
		GFPhysVector3 AdjustCollidePoint(const GFPhysVector3 &currKernalPos, const GFPhysVector3 &pivotPos , float radius)
		{
			float targetRadius = (OriginDist < radius ? radius : OriginDist);

			//calculate current collision point position 
			GFPhysVector3 currToolPos = currKernalPos + (pivotPos - currKernalPos).Normalized() * distFromKNode;

			//try to adjust collision outwards collision normal
			float dist = (currToolPos - facePoint).Dot(collideNormal);

			float deltaAdjust = (targetRadius - dist); //0.1 is smotth coeff to check

			if (deltaAdjust + accumedNormalAdjust < 0)//clamp like LCP
			{
				deltaAdjust = -accumedNormalAdjust;
				accumedNormalAdjust = 0;
			}
			else
			{
				accumedNormalAdjust += deltaAdjust;
			}

			currToolPos += collideNormal*deltaAdjust;

			//new kernel node pos
			GFPhysVector3 updatedKernalPos = currToolPos + (currToolPos - pivotPos).Normalized() * distFromKNode;

			return updatedKernalPos;
		}

		int physicIndex;
		GFPhysVector3 collideNormal;
		GFPhysVector3 faceVertex[3];
		GFPhysVector3 faceNormal;
		GFPhysVector3 facePoint;
		GFPhysVector3 toolPoint;
		GFPhysVector3 slipDir;
		GFPhysVector3 KNodeSlipToPos;

		//GFPhysVector3 correctedToolPos;
		float  distFromKNode;

		float  collideTime;
		float  collideptWeight[3];
		bool   isInnerTriangle;

		float  accumedNormalAdjust;
		float  OriginDist;
		bool   isValid;
	};

	ACToolRigidCallback(GFPhysTriangleMesh* triangleMesh,
		                const GFPhysTransform & meshTrans)
						:m_triangleMesh(triangleMesh),
						m_MeshTrans(meshTrans)
	{
		//float scale = 2.2;
		//m_lastOffset = (m_pivotPosition - m_lastOffsetedKernelPosition).Normalize() * (m_collideRadius * scale);
		//m_lastOffsetedKernelPosition += m_lastOffset;

		//m_curOffset = (m_pivotPosition - m_curOffsetedKernelPosition).Normalize() * (m_collideRadius * scale);
		//m_curOffsetedKernelPosition += m_curOffset;
	}

	virtual void ProcessOverlappedNode(int subPart, int triangleIndex, void * UserData = NULL)
	{
		GFPhysVector3* vertices = (GFPhysVector3*)m_triangleMesh->m_Vertices;
		GFPhysMeshDataTriangle* meshData = m_triangleMesh->m_Triangles;

		GFPhysVector3 triangleVertex[3];

		for(int i = 0; i < 3; ++i)
		{
			int vertexIndex = meshData[triangleIndex].m_VertexIndex[i];
			triangleVertex[i] = m_MeshTrans(vertices[vertexIndex]);//can be optimize transform tool vector to mesh space instead
		}

		CollidedFace collidedFace;

		collidedFace.physicIndex = triangleIndex;
		collidedFace.faceNormal = (triangleVertex[1] - triangleVertex[0]).Cross((triangleVertex[2] - triangleVertex[0])).Normalized();
		collidedFace.faceVertex[0] = triangleVertex[0];
		collidedFace.faceVertex[1] = triangleVertex[1];
		collidedFace.faceVertex[2] = triangleVertex[2];
		m_collidedFaces.push_back(collidedFace);
	}

	virtual void ProcessOverlappedNodes(GFPhysDBVNode * dynNodes, GFPhysAABBNode * staticnode)
	{

	}

	virtual void ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA, GFPhysDBVNode * dynNodeB)
	{

	}

	std::vector<CollidedFace>& GetFacesToCheck() { return m_collidedFaces; }

private:
	GFPhysTransform     m_MeshTrans;
	GFPhysTriangleMesh* m_triangleMesh;
	std::vector<CollidedFace> m_collidedFaces;
};

void ToolSpenetrateMgr::CorrectToolPositionByStaticRigidObjects(CTool* leftTool,CTool* rightTool,float dt)
{
	CTool* tools[2] = { leftTool, rightTool };

	for(int i = 0; i < 2; ++i){
		CTool* tool = tools[i];
		if(tool == nullptr)
			continue;

		//ogre
		Ogre::SceneNode* kernelNode = tool->GetKernelNode();
		GFPhysVector3 lastKernelPosition = OgreToGPVec3(tool->GetLastKernelNodePosition());
		GFPhysVector3 curKernelPosition = OgreToGPVec3(kernelNode->getPosition());
		const GFPhysVector3 pivotPosition = OgreToGPVec3(tool->GetPivotPosition());
		GFPhysQuaternion curQuaternion = OgreToGPQuaternion(kernelNode->getOrientation());
		GFPhysQuaternion lastQuaternion = OgreToGPQuaternion(tool->GetLastKernelNodeQuaternion());
		const float toolRadius = 0.2f;//0.25

		//physic
		GFPhysVector3 extent(toolRadius * StaticCCheckFactor, toolRadius * StaticCCheckFactor, toolRadius * StaticCCheckFactor);
		GFPhysVector3 aabbMin = curKernelPosition;
		GFPhysVector3 aabbMax = aabbMin;

		aabbMin.SetMin(pivotPosition);
		aabbMax.SetMax(pivotPosition);

		//防止穿透
		aabbMin.SetMin(lastKernelPosition);
		aabbMax.SetMax(lastKernelPosition);

		//extent
		aabbMin -= extent;
		aabbMax += extent;

		//offset
		float HeadMargin = 1.0f * toolRadius;
		const GFPhysVector3 lastOffset = HeadMargin * (pivotPosition - lastKernelPosition).Normalized();
		const GFPhysVector3 curOffset  = HeadMargin * (pivotPosition - curKernelPosition).Normalized();
		lastKernelPosition += lastOffset;
		curKernelPosition += curOffset;

		bool isCorrected = false;
		for(auto rigidObject : m_staticRigidObjects){
			GoPhys::GFPhysCollideShape* collideObject = rigidObject->m_body->GetCollisionShape();
			GoPhys::GFPhysBvhTriMeshShape* triMeshCollideShape = dynamic_cast<GoPhys::GFPhysBvhTriMeshShape*>(collideObject);
			GoPhys::GFPhysTriangleMesh* triangleMesh = triMeshCollideShape->GetMeshData();

			if(triMeshCollideShape){
				GFPhysTransform rigidTrans = rigidObject->m_body->GetWorldTransform();
				ACToolRigidCallback callback(triangleMesh,rigidTrans);
				GFPhysMeshAABBTree* aabbTree = triMeshCollideShape->GetMeshBVH();
				aabbTree->TestAABBOverlap(&callback, rigidTrans.Inverse()(aabbMin), rigidTrans.Inverse()(aabbMax));

				
				std::vector<ACToolRigidCallback::CollidedFace>& FacesToCheck = callback.GetFacesToCheck();
				
				const std::size_t nFace = FacesToCheck.size();
				
				m_CollideEdgeList.clear();

				if(nFace > 0){
					

					//function 1
					auto IsSamePoint = [](const GFPhysVector3& pt1, const GFPhysVector3& pt2,float threshold){
						if((abs(pt1.m_x - pt2.m_x) < threshold) && (abs(pt1.m_y - pt2.m_y) < threshold) && (abs(pt1.m_z - pt2.m_z) < threshold))
							return true;
						else
							return false;
					};

					//function2
					auto AddEdge = [IsSamePoint](int triangleFaceIndex,const GFPhysVector3 triangleVertex[3], const GFPhysVector3& collideNormal,std::vector<EdgeInfo>& edgeList){
						
						float  threshold = 0.00001f;
						
						//skip degenerated triangle
						if (IsSamePoint(triangleVertex[1], triangleVertex[2], threshold)
		                 || IsSamePoint(triangleVertex[1], triangleVertex[3], threshold)
						 || IsSamePoint(triangleVertex[2], triangleVertex[3], threshold))
							return;

						for(int i = 0; i < 3; ++i){
							const GFPhysVector3& vertex1 = triangleVertex[i];
							const GFPhysVector3& vertex2 = triangleVertex[(i + 1) % 3];
							const GFPhysVector3& vertex3 = triangleVertex[(i + 2) % 3];
							
							bool canAddEdge = true;
		
							//fin same edge
							for(auto& edgeInfo : edgeList){
								

								if((IsSamePoint(edgeInfo.point1, vertex1, threshold) && IsSamePoint(edgeInfo.point2, vertex2, threshold))
								|| (IsSamePoint(edgeInfo.point1, vertex2, threshold) && IsSamePoint(edgeInfo.point2, vertex1, threshold))){
							
									edgeInfo.AddTriangle(vertex3, collideNormal, triangleFaceIndex);
									canAddEdge = false;
									break;
								}
							}

							if(canAddEdge){
								
								EdgeInfo edge;
								
								edge.AddTriangle(vertex1, vertex2, vertex3, collideNormal, triangleFaceIndex);
								
								edgeList.push_back(edge);
							}
						}
					};

					//function3
					auto GetAdjustedKernelPosition = [](const std::vector<ACToolRigidCallback::CollidedFace>& collectedFaces,
														const GFPhysVector3& pivotPosition,
														const GFPhysVector3& curKernelPosition,
														GFPhysVector3 & offset){
						bool adjusted = false;
						std::vector<std::pair<float, GFPhysVector3>> adjustedInfos;
						float rayWeight, triangleWeight[3];
						GFPhysVector3 adjustedPosition;
						GFPhysVector3 intersectedPoint;
						bool ret;
						float minDistance = (pivotPosition - curKernelPosition).Length();

						for(const auto& cf : collectedFaces){
							ret = LineIntersectTriangle(cf.faceVertex[0], cf.faceVertex[1], cf.faceVertex[2],
														pivotPosition, curKernelPosition,
														rayWeight,
														intersectedPoint,
														triangleWeight);
						
							if(ret && rayWeight > FLT_EPSILON && rayWeight < 1.f){
								float dis = (intersectedPoint - pivotPosition).Length();
								if(dis < minDistance - 0.05f){
									minDistance = dis;
									adjustedPosition = intersectedPoint;
									adjusted = true;
								}
							}
						}

						if (adjusted)
							offset = adjustedPosition - curKernelPosition;
						else
							offset = GFPhysVector3(0, 0, 0);
						
						return adjusted;
					};

					//function4
					auto UpdateKernelNodePosition = [](CTool* tool,
													   const GFPhysQuaternion& curQuaternion,
													   const GFPhysVector3& curKernelPosition,
													   const GFPhysVector3& newKernelPosition,
													   const GFPhysVector3& pivotPosition){
						GFPhysVector3 oldAxes = curKernelPosition - pivotPosition;
						GFPhysVector3 newAxes = newKernelPosition - pivotPosition;

						oldAxes.Normalize();
						newAxes.Normalize();

						GFPhysQuaternion newQuaternion = ShortestArcQuat(oldAxes, newAxes);
						//newPosition = QuatRotate(newQuaternion, newPosition - pivotPosition) + pivotPosition;
						newQuaternion = newQuaternion * curQuaternion;

						tool->CorrectKernelNode(GPVec3ToOgre(newKernelPosition),
												GPQuaternionToOgre(newQuaternion));
					};

					//edge infos
					const GFPhysVector3 moveDir = curKernelPosition - lastKernelPosition;
					GFPhysVector3 edgeVertex[2] = {pivotPosition, lastKernelPosition};
					GFPhysVector3 triangleVelocity[3] = {GFPhysVector3(0.f, 0.f, 0.f), GFPhysVector3(0.f, 0.f, 0.f), GFPhysVector3(0.f, 0.f, 0.f)};
					GFPhysVector3 edgeVelocity[2] = {GFPhysVector3(0.f, 0.f, 0.f), moveDir};
					GFPhysVector3 cpTriangle, cpEdge, cpNormal;
					bool exceedMaxItr = false;

					//CorrectInfo correctInfo;
					//std::vector<CorrectInfo> correctInfos;
					std::vector<ACToolRigidCallback::CollidedFace> collidedFaces;
					
					for(std::size_t fi = 0; fi < nFace; ++fi){
						 
						ACToolRigidCallback::CollidedFace & CheckFace = FacesToCheck[fi];

						float collideTime = -1.0f;

						float d0 = ClosetPtSegmentTriangle(const_cast<GFPhysVector3*>(CheckFace.faceVertex),
							                               edgeVertex,
							                               cpTriangle,
													       cpEdge,
														   cpNormal);

						if (d0 < toolRadius * StaticCCheckFactor)
						{
							collideTime = 0.0f;
							exceedMaxItr = false;
						}
						else
						{
							collideTime = GFPhysCABasedCCD::GetFaceEdgeWithRadiusCollideTime(const_cast<GFPhysVector3*>(CheckFace.faceVertex),
																							 edgeVertex,

																							 triangleVelocity,
																							 edgeVelocity,

																							 1.0f,
																							 toolRadius,
																							 toolRadius * 0.02f,

																							 cpTriangle,
																							 cpEdge,
																							 cpNormal,
																							 exceedMaxItr);
						}
						
						if (cpNormal.Dot(CheckFace.faceNormal) <= 0)//use single face modle exclude negative face
						{
							continue;
						}

						
						if(collideTime >= 0 && exceedMaxItr == false){
							
							CalcBaryCentric(CheckFace.faceVertex[0], CheckFace.faceVertex[1], CheckFace.faceVertex[2],
								cpTriangle, CheckFace.collideptWeight[0], CheckFace.collideptWeight[1], CheckFace.collideptWeight[2]);
							
							if (CheckFace.collideptWeight[0] > 0.00001 && CheckFace.collideptWeight[1] > 0.00001 &&CheckFace.collideptWeight[2] > 0.00001){
								
								CheckFace.isInnerTriangle = true;
							
							}else{
								
								CheckFace.isInnerTriangle = false;
							}

							if (collideTime < FLT_EPSILON)
								collideTime = 0;

							CheckFace.facePoint = cpTriangle;
							CheckFace.toolPoint = cpEdge;
							CheckFace.collideTime = collideTime;
							CheckFace.collideNormal = cpNormal;
							collidedFaces.push_back(CheckFace);
							
							
							//加入碰撞边列表,只有静态碰撞（0时刻发生的碰撞才需要加入）
							if (collideTime == 0)
								AddEdge(collidedFaces.size()-1, CheckFace.faceVertex, cpNormal, m_CollideEdgeList);
	
						}
					}//fi < nFace

					//remove
					/*
					for (std::size_t i = 0; i < edgeList.size(); ++i){
						
						auto& removedInfo = edgeList[i];
						
						if(removedInfo.faceIndices.size() == 1){
							
							for (std::size_t j = 0; j < edgeList.size(); ++j){
								if(i == j)
									continue;
								auto& sharedInfo = edgeList[j];
								if(sharedInfo.faceIndices.size() >= 1){
									if(std::find(sharedInfo.faceIndices.begin(), sharedInfo.faceIndices.end(), removedInfo.faceIndices[0]) != sharedInfo.faceIndices.end()){
										removedInfo.faceIndices.clear();
										removedInfo.thirdPoints.clear();
										break;
									}
								}
							}
						}
					}*/

					//移除孤立的面的边
					for (auto itr = m_CollideEdgeList.begin(); itr != m_CollideEdgeList.end();)
					{
						if(itr->GetNumTriangle() <= 1)
						   itr = m_CollideEdgeList.erase(itr);
						else
						   ++itr;
					}

					const float moveCoeff = 0.9999f;
					float d = 0;
					bool ret;

					//检测共享边的2个面是否凹或者凸
					for (int c = 0; c < (int)m_CollideEdgeList.size(); c++)
					{
						EdgeInfo & edgeInfo = m_CollideEdgeList[c];
						
						const int nFace = edgeInfo.GetNumTriangle();
						
						if (nFace != 2)
							throw "error.";

						ACToolRigidCallback::CollidedFace & shareCollideFace0 = collidedFaces[edgeInfo.faceIndices[0]];
						const GFPhysVector3& faceNormal1 = shareCollideFace0.faceNormal;

						ACToolRigidCallback::CollidedFace & shareCollideFace1 = collidedFaces[edgeInfo.faceIndices[1]];
						const GFPhysVector3& faceNormal2 = shareCollideFace1.faceNormal;
						const GFPhysVector3& ptOnTriangle2 = edgeInfo.thirdPoints[1];
					
						GFPhysVector3 edgeDir = (edgeInfo.point2 - edgeInfo.point1).Normalized();
							
						GFPhysVector3 dirOnTriangle2 = ptOnTriangle2 - edgeInfo.point1;

						dirOnTriangle2 = (dirOnTriangle2 - edgeDir * dirOnTriangle2.Dot(edgeDir)).Normalized();

						d = faceNormal1.Dot(dirOnTriangle2);

						bool isConCave = false;

						if (d > -0.005)//&& (shareCollideFace0.isInnerTriangle || shareCollideFace1.isInnerTriangle))
							isConCave = true;
							
						for (int c = 0; c < 2; c++)
						{
							 int faceIndex = edgeInfo.faceIndices[c];
	
							 if (isConCave)
							 {
								 //凹 & 平 choose face normal as collision normal
								 collidedFaces[faceIndex].collideNormal = collidedFaces[faceIndex].faceNormal;
							 }
						}
					}

					std::vector<ACToolRigidCallback::CollidedFace> time0CollideFaces;//0时刻碰撞的面

					std::vector<ACToolRigidCallback::CollidedFace> OtherCollideFaces;//>0时刻碰撞的面

					for (int fi = 0; fi < (int)collidedFaces.size(); ++fi)
					{
						if (collidedFaces[fi].isValid)
						{
							if (collidedFaces[fi].collideTime < FLT_EPSILON)
								time0CollideFaces.push_back(collidedFaces[fi]);
							else
								OtherCollideFaces.push_back(collidedFaces[fi]);
						}
					}

					GFPhysVector3 DesiredKPos = curKernelPosition;
					if (time0CollideFaces.size() > 0)
					{
						float moveDist = (curKernelPosition - lastKernelPosition).Length();
						
						if (moveDist > 0.05)
							moveDist = 0.05f;
						
						DesiredKPos = lastKernelPosition + (curKernelPosition - lastKernelPosition).Normalized() * moveDist;

					}
					for (int fi = 0; fi < (int)time0CollideFaces.size(); ++fi)
					{
						GFPhysVector3 dir = (pivotPosition - DesiredKPos).Normalized();

						float distFromKernal = (time0CollideFaces[fi].toolPoint - lastKernelPosition).Length();

						GFPhysVector3 preditcToolPos = DesiredKPos + dir * distFromKernal;

						GFPhysVector3 wantMoveVec = (preditcToolPos - time0CollideFaces[fi].toolPoint);

						GFPhysVector3 wantMoveDir = wantMoveVec.Normalized();

						GFPhysVector3 collideNormal = time0CollideFaces[fi].collideNormal;

						if (wantMoveDir.Dot(collideNormal) >= 0)//out ward collision face
							time0CollideFaces[fi].slipDir = wantMoveDir;
						else
							time0CollideFaces[fi].slipDir = (wantMoveDir - collideNormal * wantMoveDir.Dot(collideNormal)).Normalized();

						
						float dist = (time0CollideFaces[fi].toolPoint - time0CollideFaces[fi].facePoint).Dot(collideNormal);

						if (dist < toolRadius)
							time0CollideFaces[fi].toolPoint += collideNormal * (toolRadius - dist);
						
						preditcToolPos = time0CollideFaces[fi].toolPoint + time0CollideFaces[fi].slipDir * wantMoveVec.Dot(time0CollideFaces[fi].slipDir);

						GFPhysVector3 temp = (preditcToolPos - pivotPosition).Normalized();

						time0CollideFaces[fi].KNodeSlipToPos = preditcToolPos + temp * distFromKernal;
					}
					

					GFPhysVector3 finalPosition(0, 0, 0);//const std::size_t nSelectedInfo = selectedCorrectInfos.size();
					
					//在0时刻存在碰撞点
					if (time0CollideFaces.size() > 0)
					{
						for (int c = 0; c < (int)time0CollideFaces.size(); c++)
						{
							time0CollideFaces[c].PrepareAdjust(lastKernelPosition);
						}
					    
						for (int itor = 0; itor < 15; itor++)
						{
							for (int c = 0; c < (int)time0CollideFaces.size(); c++)
							{
								DesiredKPos = time0CollideFaces[c].AdjustCollidePoint(DesiredKPos, pivotPosition, toolRadius);
							}
						}

						curKernelPosition -= curOffset;

						DesiredKPos -= HeadMargin * (pivotPosition - DesiredKPos).Normalized();

						GFPhysVector3 offset;
						if (GetAdjustedKernelPosition(FacesToCheck, pivotPosition, DesiredKPos, offset))
							DesiredKPos += offset;

						UpdateKernelNodePosition(tool, curQuaternion, curKernelPosition, DesiredKPos, pivotPosition);
						
					}
					else//0时刻没有发生碰撞
					{
						int correctIndex = -1;
						
						float minTime = 100;

						for (std::size_t i = 0; i < OtherCollideFaces.size(); ++i)
						{
							if (minTime > OtherCollideFaces[i].collideTime)
							{
								minTime = OtherCollideFaces[i].collideTime;
								correctIndex = i;
							}
						}

						if(correctIndex != -1)
						{
							GFPhysVector3 newPosition = lastKernelPosition + edgeVelocity[1] * OtherCollideFaces[correctIndex].collideTime;

							curKernelPosition -= curOffset;
							
							newPosition -= HeadMargin * (pivotPosition - newPosition).Normalized();

							GFPhysVector3 offset;
							
							if (GetAdjustedKernelPosition(FacesToCheck, pivotPosition, newPosition, offset))
								newPosition += offset;

							
							UpdateKernelNodePosition(tool, curQuaternion, curKernelPosition, newPosition, pivotPosition);
						}
					}
				}//nFace > 0
			}
		}
	}
}