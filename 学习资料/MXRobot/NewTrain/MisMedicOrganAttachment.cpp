#include "MisMedicOrganAttachment.h"
#include "MisMedicOrganOrdinary.h"
#include "ITool.h"
#include "ResourceManager/ResourceManager.h"

#include "PhysicsWrapper.h"
#include "CustomConstraint.h"
#include "Topology/GoPhysSoftBodyRestShapeModify.h"
#include "CustomConstraint.h"
#include "MXOgreGraphic.h"
#include "TrainUtils.h"
//============================================================================================================================
Ogre::Quaternion MisMedicOrganAttachment::CalcFaceRotate(const GFPhysVector3 srcPos[3] , const GFPhysVector3 dstPos[3])
{
	GFPhysVector3 srcCom(0,0,0);
	GFPhysVector3 dstCom(0,0,0);

	GFPhysVector3 normalSrc = (srcPos[1]-srcPos[0]).Cross(srcPos[2]-srcPos[0]).Normalized();
	GFPhysVector3 normalDst = (dstPos[1]-dstPos[0]).Cross(dstPos[2]-dstPos[0]).Normalized();

	GFPhysVector3 extendSrcPos[3];
	GFPhysVector3 extendDstPos[3];

	for(int c = 0 ; c < 3 ; c++)
	{
		extendSrcPos[c] = srcPos[c] + normalSrc * 0.1f;
		extendDstPos[c] = dstPos[c] + normalDst * 0.1f;

		srcCom += srcPos[c];
		dstCom += dstPos[c];

		srcCom += extendSrcPos[c];
		dstCom += extendDstPos[c];
	}
	srcCom /= 6.0f;
	dstCom /= 6.0f;


	GFPhysMatrix3 A_pq;
	A_pq.SetZero();

	for(int c = 0 ; c < 3 ; c++)
	{
		GFPhysVector3 q = srcPos[c] - srcCom;
		GFPhysVector3 p = dstPos[c] - dstCom;

		A_pq[0][0] += p[0] * q[0]; A_pq[0][1] += p[0] * q[1]; A_pq[0][2] += p[0] * q[2];
		A_pq[1][0] += p[1] * q[0]; A_pq[1][1] += p[1] * q[1]; A_pq[1][2] += p[1] * q[2];
		A_pq[2][0] += p[2] * q[0]; A_pq[2][1] += p[2] * q[1]; A_pq[2][2] += p[2] * q[2];
	}

	for(int c = 0 ; c < 3 ; c++)
	{
		GFPhysVector3 q = extendSrcPos[c] - srcCom;
		GFPhysVector3 p = extendDstPos[c] - dstCom;

		A_pq[0][0] += p[0] * q[0]; A_pq[0][1] += p[0] * q[1]; A_pq[0][2] += p[0] * q[2];
		A_pq[1][0] += p[1] * q[0]; A_pq[1][1] += p[1] * q[1]; A_pq[1][2] += p[1] * q[2];
		A_pq[2][0] += p[2] * q[0]; A_pq[2][1] += p[2] * q[1]; A_pq[2][2] += p[2] * q[2];
	}

	GFPhysMatrix3 R;
	R.SetIdentity();
	polarDecomposition2(A_pq , 1e-6f, R);

	GFPhysQuaternion rotQuat;
	R.GetRotation(rotQuat);

	return GPQuaternionToOgre(rotQuat);
}
//================================================================================================================
MisMedicTitaniumClampV2::MisMedicTitaniumClampV2(float invalidClipLen, 
	                                             Ogre::Vector3 clipAxis[3],
	                                             Ogre::Vector2 clipBound[2],
												 GFPhysSoftBodyFace * attachFace,
												 MisMedicOrgan_Ordinary * organ,
												 int toolflag)
{
	m_type = MOAType_TiantumClip;
	m_GenerateToolState = toolflag;
	m_HostOrgan = organ;
	m_InvalidClipLen = invalidClipLen;
	//m_DesiredClipDir = desiredClipDir;
	m_DesiredClipAixs[0] = clipAxis[0];
	m_DesiredClipAixs[1] = clipAxis[1];
	m_DesiredClipAixs[2] = clipAxis[2];

	m_DesiredClipBounds[0] = clipBound[0];
	m_DesiredClipBounds[1] = clipBound[1];
	m_DesiredClipBounds[2] = clipBound[2];

	m_FallDownTime = 0.0f;
	m_pTitaniumClip = CResourceManager::Instance()->GetOneTool(TT_HEMOCLIP, false, "");
	m_ScenenNode = m_pTitaniumClip->GetKernelNode();

	m_TotalClipLen = m_ScenenNode->getAttachedObject(0)->getBoundingBox().getSize().z;

	CalculateTitanumLocalInformation(attachFace);

	GetLocalAABB(m_MeshLocalMin, m_MeshLocalMax);

}
float MisMedicTitaniumClampV2::GetValidClipPercent()
{
	float validlen = m_TotalClipLen - fabs(m_InvalidClipLen);
	float totalcliplen = m_TotalClipLen - 0.1f;

	float percent = GPClamped(validlen / totalcliplen , 0.0f , 1.0f);

	return percent;
}

int MisMedicTitaniumClampV2::GetTriEdgesIntersectClip(Ogre::Vector3 triVerts[3], Ogre::Vector3 intersectPts[3])
{
   Ogre::Vector3 origin = m_DesiredClipAixs[0] * ((m_DesiredClipBounds[0].x + m_DesiredClipBounds[0].y) * 0.5f)
		                 + m_DesiredClipAixs[1] * ((m_DesiredClipBounds[1].x + m_DesiredClipBounds[1].y) * 0.5f)
		                 + m_DesiredClipAixs[2] * m_DesiredClipBounds[2].x;

	int numPt = 0;

	for (int c = 0; c < 3; c++)
	{
		 Ogre::Vector3 p0 = triVerts[c];
		
		 Ogre::Vector3 p1 = triVerts[(c + 1) % 3];
		
		 float d0 = (p0 - origin).dotProduct(m_DesiredClipAixs[0]);
		
		 float d1 = (p1 - origin).dotProduct(m_DesiredClipAixs[0]);

		 if (d0 * d1 <= 0 && GFPhysMath::GPFabs(d0 - d1) > GP_EPSILON)
		 {
			 float w = d0 / (d0 - d1);
			
			 Ogre::Vector3 testPt = p0 * (1 - w) + p1 * w;
		
			 //check whether intersect pt in i y-z spanned region
			 GFPhysVector3 linept[2];

			 for (int c = 0; c < 2; c++)
			 {
				 linept[c].m_x = 0; //ignore x - axis
				 linept[c].m_y = (testPt - origin).dotProduct(m_DesiredClipAixs[1]);// y
				 linept[c].m_z = (testPt - origin).dotProduct(m_DesiredClipAixs[2]);// z
			 }

			 float clipHWid = 0.08f;// (m_DesiredClipBounds[1].y - m_DesiredClipBounds[1].x)*0.5f;

			 float clipLen = 0.6f;

			 bool intersect = IsLineSegAABBOverLap(GFPhysVector3(-1.0f, -clipHWid, 0), GFPhysVector3(1.0f, clipHWid, clipLen), linept[0], linept[1]);

			 if (intersect)
			 {
				 intersectPts[numPt] = testPt;
				 numPt++;
			 }
		}
	}

	return numPt;
}
//============================================================================================================================
void MisMedicTitaniumClampV2::CalculateTitanumLocalInformation(GFPhysSoftBodyFace * AttachFace)
{
	//face center the clip's position will calculate relatively to this center
	
	//calculate corrected titanic position and orientation
	//the follow code  rotate clip's Y direction to coincide with face's normal
	//Ogre::Quaternion TitanicOrient = orient;
	
	//store face position and clip's orientation in clip action
	m_AttachedFace = AttachFace;

	Ogre::Vector3 undeformedPos[3];
	
	Ogre::Vector3 currentPos[3];

	for(int c = 0 ; c < 3 ; c++)
	{
		currentPos[c]    = GPVec3ToOgre(AttachFace->m_Nodes[c]->m_CurrPosition);//
		undeformedPos[c] = GPVec3ToOgre(AttachFace->m_Nodes[c]->m_UnDeformedPos);
	}
	
	Ogre::Vector3 triCoordAxis[3];
	Ogre::Vector3 triOrigin;
	Ogre::Quaternion triRot;
	
	//translate to material space
	//recalculate axis use rest position of triangle
	Ogre::Quaternion clipRotWorld;
	clipRotWorld.FromAxes(m_DesiredClipAixs[0], m_DesiredClipAixs[1], m_DesiredClipAixs[2]);

	Ogre::Vector3    clipPosWorld = m_DesiredClipAixs[0] * ((m_DesiredClipBounds[0].x + m_DesiredClipBounds[0].y) * 0.5f)
		                          + m_DesiredClipAixs[1] * ((m_DesiredClipBounds[1].x + m_DesiredClipBounds[1].y) * 0.5f)
		                          + m_DesiredClipAixs[2] *  m_DesiredClipBounds[2].x;

	ExtractTriangelCoord(undeformedPos, clipPosWorld);

	GetTriangleCoordAxis(undeformedPos, triOrigin, triCoordAxis);
	
	triRot.FromAxes(triCoordAxis[0], triCoordAxis[1], triCoordAxis[2]);
	
	Ogre::Vector3 RelPosWorld = clipPosWorld - triOrigin;
	

	m_clipPosInTriFrame[0] = RelPosWorld.dotProduct(triCoordAxis[0]);
	m_clipPosInTriFrame[1] = RelPosWorld.dotProduct(triCoordAxis[1]);
	m_clipPosInTriFrame[2] = RelPosWorld.dotProduct(triCoordAxis[2]);
	
	m_clipRotInTriFrame = triRot.Inverse()*clipRotWorld;

	MisMedicOrgan_Ordinary::ExtractFaceIdAndMaterialIdFromUsrData(m_AttachedFace, m_RefFaceMaterialID, m_RefFaceId);

	//no need will be update every frame
	Update(0);
}
void MisMedicTitaniumClampV2::ExtractTriangelCoord(Ogre::Vector3 triVerts[3], const Ogre::Vector3 & position)
{
	Ogre::Vector3 FaceCenter = (triVerts[0] + triVerts[1] + triVerts[2])*0.33333f;
	
	Ogre::Vector3 FaceNormal = (triVerts[1] - triVerts[0]).crossProduct(triVerts[2] - triVerts[0]).normalisedCopy();

	//calculate attach pos in titanic object space
	Ogre::Vector3 Rel_Pos = position - FaceCenter;

	Ogre::Vector3 clipPt = FaceCenter;
	Ogre::Vector3 clipNormal = Rel_Pos.crossProduct(FaceNormal);

	//m_Coord.m_PerpDist = 0;
	bool Succced = false;
	if (clipNormal.length() > GP_EPSILON)
	{
		clipNormal.normalise();
		for (int e = 0; e < 3; e++)
		{
			Ogre::Vector3 posA = triVerts[e];
			
			Ogre::Vector3 posB = triVerts[(e + 1) % 3];
			
			float dA = (posA - clipPt).dotProduct(clipNormal);
			
			float dB = (posB - clipPt).dotProduct(clipNormal);

			if (dA * dB <= 0 && GFPhysMath::GPFabs(dA - dB) > GP_EPSILON)
			{
				float w = dA / (dA - dB);
				Ogre::Vector3 intersecPos = posA  * (1 - w) + posB * w;
				if ((intersecPos - FaceCenter).dotProduct(Rel_Pos) > 0)
				{
					//m_Coord.m_PerpDist = (Prj_Pos - FaceCenter).length();
					m_Coord.m_AxisPointWeight[e] = (1 - w);
					m_Coord.m_AxisPointWeight[(e + 1) % 3] = w;
					m_Coord.m_AxisPointWeight[(e + 2) % 3] = 0;
					Succced = true;
					break;
				}
			}
		}
	}

	if (Succced == false)
	{
		//to do deteriorated case choose any
		int i = 0;
		int j = i + 1;
	}
}

bool MisMedicTitaniumClampV2::GetTriangleCoordAxis(Ogre::Vector3 vert[3], Ogre::Vector3 & resultOrigin, Ogre::Vector3 resultAxis[3])
{
	resultOrigin = (vert[0] + vert[1] + vert[2])*0.333333f;
	Ogre::Vector3 clipPt = (vert[0] * m_Coord.m_AxisPointWeight[0]
		                  + vert[1] * m_Coord.m_AxisPointWeight[1]
						  + vert[2] * m_Coord.m_AxisPointWeight[2]);

	resultAxis[0] = (clipPt - resultOrigin).normalisedCopy();//x - axis
	resultAxis[2] = (vert[1] - vert[0]).crossProduct(vert[2] - vert[0]).normalisedCopy();//z - axis
	resultAxis[1] = (resultAxis[2].crossProduct(resultAxis[0])).normalisedCopy();//y - axis
	return true;
}
//============================================================================================================================
void MisMedicTitaniumClampV2::Update(float deltatime)
{
	if (m_AttachedFace == 0)
	{
		if (m_FallDownTime < 1.0f)
		{
			Ogre::Vector3 gravityMove = m_HostOrgan->GetGravity().normalisedCopy()*5.0f * deltatime;
			m_ScenenNode->setPosition(m_ScenenNode->getPosition() + gravityMove);
			m_FallDownTime += deltatime;
		}
		else
		{
			m_ScenenNode->setVisible(false);
		}
		return;
	}

	Ogre::Vector3 currPos[3];
	for(int c = 0 ; c < 3 ; c++)
	{
		currPos[c] = GPVec3ToOgre(m_AttachedFace->m_Nodes[c]->m_CurrPosition);
	}
	
	//calculate face rotate relative to clip times
	//Ogre::Quaternion faceRotate   = CalcFaceRotate(m_FacePosInClipTime , currPos);
	//Ogre::Quaternion clipTotalRot = faceRotate * m_OrientInClipTime;

	Ogre::Vector3 triOrigin;
	Ogre::Vector3 triCoordAxis[3];
	Ogre::Quaternion triRot;
	bool succeed = GetTriangleCoordAxis(currPos, triOrigin, triCoordAxis);
	if (succeed)
	{
		triRot.FromAxes(triCoordAxis[0], triCoordAxis[1], triCoordAxis[2]);
		
		Ogre::Vector3 clipPosWorld = triCoordAxis[0] * m_clipPosInTriFrame[0] 
			                       + triCoordAxis[1] * m_clipPosInTriFrame[1]
			                       + triCoordAxis[2] * m_clipPosInTriFrame[2]
								   + triOrigin;
		Ogre::Quaternion clipRotWorld = triRot * m_clipRotInTriFrame;

		m_ScenenNode->setOrientation(clipRotWorld);
		m_ScenenNode->setPosition(clipPosWorld);//
	}
}
//============================================================================================================================
void MisMedicTitaniumClampV2::OnCutByToolFinish()
{
	if (m_AttachedFace != 0)
	{
		bool finded = RefindAttachFace();
		if (finded == false)
		{
			m_AttachedFace = 0;
			m_FallDownTime = 0;
		}
	}
}
//============================================================================================================================
MisMedicTitaniumClampV2::~MisMedicTitaniumClampV2()
{
	if (m_ScenenNode)
	{
		Ogre::SceneManager * pSceneManager = MXOgreWrapper::Instance()->GetDefaultSceneManger();

		Ogre::SceneNode::ChildNodeIterator iterObject = m_ScenenNode->getChildIterator();
		while ( iterObject.hasMoreElements() )
		{
			Ogre::SceneNode * pNode = (Ogre::SceneNode *)iterObject.getNext();
			Ogre::SceneNode::ObjectIterator iterAttachedObject = pNode->getAttachedObjectIterator();

			while ( iterAttachedObject.hasMoreElements() )
			{
				Ogre::Entity * pEntity = (Ogre::Entity *)iterAttachedObject.getNext();
				pNode->detachObject(pEntity);
				pSceneManager->destroyEntity(pEntity);

			}
			pSceneManager->destroySceneNode(pNode);
		}

		pSceneManager->getRootSceneNode()->removeChild(m_ScenenNode);
		m_ScenenNode->detachAllObjects();
		pSceneManager->destroySceneNode(m_ScenenNode);
		m_ScenenNode = NULL;
	}
}
//============================================================================================================================
int MisMedicTitaniumClampV2::getGenerateToolState()
{
		return m_GenerateToolState;
}
//============================================================================================================================
void MisMedicTitaniumClampV2::GetLocalAABB(GFPhysVector3& min,GFPhysVector3& max)
{
	if(m_ScenenNode)
	{
		int n = m_ScenenNode->numAttachedObjects();		//test tocd
		Ogre::MovableObject * movableObject = m_ScenenNode->getAttachedObject(0);
		if(movableObject)
		{
			min = OgreToGPVec3(movableObject->getBoundingBox().getMinimum());
			max = OgreToGPVec3(movableObject->getBoundingBox().getMaximum());
		}		
	}
}

bool MisMedicTitaniumClampV2::RefindAttachFace()
{
	GFPhysVector3 min;
	GFPhysVector3 max;

	GetLocalAABB(min , max);

	Ogre::Vector3 planePoints[4];
	planePoints[0] = Ogre::Vector3( 0  , min.m_y , min.m_z);
	planePoints[1] = Ogre::Vector3( 0  , min.m_y , max.m_z );
	planePoints[2] = Ogre::Vector3( 0  , max.m_y , min.m_z);
	planePoints[3] = Ogre::Vector3( 0  , max.m_y , max.m_z);
	
	//transform local clip quad to world
	GFPhysVector3 worldClipQuad[4];
	Ogre::Quaternion clipRot = m_ScenenNode->_getDerivedOrientation();
	Ogre::Vector3    clipPos = m_ScenenNode->_getDerivedPosition();

    for(int c = 0 ; c < 4 ; c++)
	{	
		Ogre::Vector3 temp = clipRot * planePoints[c] + clipPos;
		worldClipQuad[c] = OgreToGPVec3(temp);
	}
	//test this quad to soft face to find one attach
	GFPhysVectorObj<GFPhysSoftBodyFace*> IntersectFaces;

	GetSoftFaceIntersectQuad(worldClipQuad , m_HostOrgan->m_physbody , IntersectFaces);

	//Ogre::Vector3 clipWorldY = qutRot * Ogre::Vector3::UNIT_Y;

	GFPhysSoftBodyFace * selectedFace = 0;
	float maxDot = -FLT_MAX;
	for(size_t c = 0 ; c < IntersectFaces.size() ; c++)
	{
		GFPhysSoftBodyFace * face = IntersectFaces[c];
		Ogre::Vector3 FaceNormal = GPVec3ToOgre((face->m_Nodes[1]->m_UnDeformedPos-face->m_Nodes[0]->m_UnDeformedPos).Cross(face->m_Nodes[2]->m_UnDeformedPos-face->m_Nodes[0]->m_UnDeformedPos));
		FaceNormal.normalise();

		if (fabsf(FaceNormal.dotProduct(m_AttachFaceRestNormal)) > maxDot)
		{
			maxDot = fabsf(FaceNormal.dotProduct(m_AttachFaceRestNormal));
			selectedFace = face;
		}
		if (face == m_AttachedFace)
		{
			selectedFace = face;
			//return true;
		}
	}

	if (selectedFace)
	{
		CalculateTitanumLocalInformation(selectedFace);
		return true;
	}
	else
	{
		return false;
	}
}
//===================================================================================================
MisMedicSilverClamp::MisMedicSilverClamp(GFPhysSoftBodyFace * attachFace,
										 GFPhysSoftBodyFace * DeformFaces[] , 
										 int DeformFacesNum , 
										 GFPhysSoftBodyFace * FaceForOreint[2],
										 MisMedicOrgan_Ordinary * organ , 
										 const Ogre::Vector3 & ToolWorldPos , 
										 const Ogre::Quaternion & ToolWorldOrient ,
										 const Ogre::Vector3 & scale, 
										 int toolflag)
{
	m_type = MOAType_SilverClip;

	//create 
	m_pSilverClip = CResourceManager::Instance()->GetOneTool(TT_SILVERCLIP, false, ""); 
	m_ScenenNode = m_pSilverClip->GetKernelNode();

	Ogre::Vector3 ClipLocalPos = m_ScenenNode->getPosition();
	Ogre::Quaternion ClipLocalOrient = m_ScenenNode->getOrientation();

	Ogre::Vector3  ClipWorldPosition = ToolWorldPos + ToolWorldOrient * ClipLocalPos;
	Ogre::Quaternion ClipWorldOrient = ToolWorldOrient * ClipLocalOrient;

	CalculateTitanumLocalInformation(attachFace , ClipWorldPosition , ClipWorldOrient);

	m_ScenenNode->setOrientation(ClipWorldOrient);
	m_ScenenNode->setPosition(ClipWorldPosition);

}
void MisMedicSilverClamp::CalculateTitanumLocalInformation(GFPhysSoftBodyFace * AttachFace , const Ogre::Vector3 & position , const Ogre::Quaternion & orient)
{
	Ogre::Vector3 titanClipDirLocal(0,1,0);

	Ogre::Vector3 titanClipDirWorld = orient*titanClipDirLocal;

	//face center the clip's position will calculate relatively to this center
	Ogre::Vector3 FaceCenter = GPVec3ToOgre(AttachFace->m_Nodes[0]->m_CurrPosition*0.333f
		+AttachFace->m_Nodes[1]->m_CurrPosition*0.333f
		+AttachFace->m_Nodes[2]->m_CurrPosition*0.333f);

	Ogre::Vector3 FaceNormal = GPVec3ToOgre((AttachFace->m_Nodes[1]->m_CurrPosition-AttachFace->m_Nodes[0]->m_CurrPosition).Cross(AttachFace->m_Nodes[2]->m_CurrPosition-AttachFace->m_Nodes[0]->m_CurrPosition));
	FaceNormal.normalise();

	//calculate corrected titanic position and orientation
	//the follow code  rotate clip's Y direction to coincide with face's normal
	Ogre::Quaternion TitanicOrient = orient;
	Ogre::Vector3 clipWorldY = TitanicOrient * Ogre::Vector3::UNIT_Y;
	Ogre::Quaternion qprim = (clipWorldY.dotProduct(FaceNormal) >= 0 ? clipWorldY.getRotationTo(FaceNormal) : (-clipWorldY).getRotationTo(FaceNormal));
	TitanicOrient = qprim * TitanicOrient;


	//store face position and clip's orientation in clip action
	m_relativeFace = AttachFace;
	for(int c = 0 ; c < 3 ; c++)
	{
		m_FacePosInClipTime[c] = AttachFace->m_Nodes[c]->m_CurrPosition;//
	}
	m_OrientInClipTime = TitanicOrient;

	//calculate attach pos in titanic object space
	m_RelPosToFaceInClipTime = position - FaceCenter;
	m_RelPosToFaceInClipTime = m_RelPosToFaceInClipTime - FaceNormal * m_RelPosToFaceInClipTime.dotProduct(FaceNormal);

	if(FaceNormal.dotProduct(titanClipDirWorld) > 0)
		m_RelPosToFaceInClipTime += FaceNormal*0.03f;
	else
		m_RelPosToFaceInClipTime -= FaceNormal*0.03f;


	MisMedicOrgan_Ordinary::ExtractFaceIdAndMaterialIdFromUsrData(m_relativeFace , m_RefFaceMaterialID , m_RefFaceId );

	//no need will be update every frame
	m_ScenenNode->setOrientation(TitanicOrient);
	m_ScenenNode->setPosition(m_RelPosToFaceInClipTime + FaceCenter);
}
//======================================================================================================================
void MisMedicSilverClamp::Update(float deltatime)
{
	if(m_relativeFace == 0)
		return;

	GFPhysVector3 faceCenter = m_relativeFace->m_Nodes[0]->m_CurrPosition*0.333f
		+m_relativeFace->m_Nodes[1]->m_CurrPosition*0.333f
		+m_relativeFace->m_Nodes[2]->m_CurrPosition*0.333f;

	GFPhysVector3 currPos[3];
	for(int c = 0 ; c < 3 ; c++)
	{
		currPos[c] = m_relativeFace->m_Nodes[c]->m_CurrPosition;
	}

	//calculate face rotate relative to clip times
	Ogre::Quaternion faceRotate   = CalcFaceRotate(m_FacePosInClipTime , currPos);
	Ogre::Quaternion clipTotalRot = faceRotate * m_OrientInClipTime;

	m_ScenenNode->setOrientation(clipTotalRot);
	m_ScenenNode->setPosition(GPVec3ToOgre(faceCenter) + faceRotate * m_RelPosToFaceInClipTime);//
}
//======================================================================================================================
void MisMedicSilverClamp::OnFaceSplittedByCut(std::vector<FaceSplitByCut> & facesSplitted ,GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & facesDeleted, MisMedicOrgan_Ordinary * organ)
{
	//int RefFaceId;

	//int RefFaceMaterialID;

	////if(m_relativeFace && oldfaceid == m_RefFaceId && oldfacecate == m_RefFaceMaterialID)
	//{
		//CalculateTitanumLocalInformation(newface , m_ScenenNode->getPosition() , m_ScenenNode->getOrientation());
	//}
}
//======================================================================================================================
MisMedicSilverClamp::~MisMedicSilverClamp()
{

}


typedef struct {
	GFPhysSoftBodyFace* face;
	GFPhysSoftBodyNode* line1A;
	GFPhysSoftBodyNode* line1B;
	float line1Lambda;
	GFPhysSoftBodyNode* line2A;
	GFPhysSoftBodyNode* line2B;
	float line2Lambda;
	float lambda;
	GFPhysVector3 currentPos;
	GFPhysVector3 currentDirX;
	GFPhysVector3 currentDirY;
	GFPhysVector3 currentDirZ;
	int visible;
	int instanceID;
} nailPoint_t;

MisMedicEndoGiaClips::MisMedicEndoGiaClips(const char* name_) {
	m_type = MOAType_StraightClip;
	nailPointSize = 1000;
	nailPointCount = 0;
	nailPoint = malloc(nailPointSize*sizeof(nailPoint_t));
	pInstanceManager = new GeometryInstanceManager(1, name_, "clipstraight.mesh", "MisMedical/SimpleGreyLittedInstance");
	///printf("Create Me: %s => %d\n", name_, pInstanceManager);
	//pInstanceManager = new GeometryInstanceManager(0, "InstanceMgr", "clipstraight.mesh", "MisMedical/SimpleGreyLitted");
}

MisMedicEndoGiaClips::~MisMedicEndoGiaClips() {
	//printf("Delete Me: %d\n", pInstanceManager);
	free(nailPoint);
	delete pInstanceManager;
}

void MisMedicEndoGiaClips::addClip(GFPhysSoftBodyFace* face, GFPhysSoftBodyNode* line1A, GFPhysSoftBodyNode* line1B, float line1Lambda, GFPhysSoftBodyNode* line2A, GFPhysSoftBodyNode* line2B, float line2Lambda, float lambda) {
	nailPoint_t* currentNailPoint;
	currentNailPoint = ((nailPoint_t*)nailPoint)+nailPointCount;
	currentNailPoint->face = face;
	currentNailPoint->line1A = line1A;
	currentNailPoint->line1B = line1B;
	currentNailPoint->line1Lambda = line1Lambda;
	currentNailPoint->line2A = line2A;
	currentNailPoint->line2B = line2B;
	currentNailPoint->line2Lambda = line2Lambda;
	currentNailPoint->lambda = 1.0;
	currentNailPoint->visible = 1;
	
	currentNailPoint->instanceID = pInstanceManager->addInstance();

	nailPointCount++;
}

void MisMedicEndoGiaClips::Update(float deltatime) {
	this->calc();
	this->draw();
}

void MisMedicEndoGiaClips::OnCutByToolFinish() {

}
void MisMedicEndoGiaClips::OnRemoveTetrahedron(GFPhysSoftBodyTetrahedron * tetra) {

}

void MisMedicEndoGiaClips::OnFaceSplittedByCut(std::vector<FaceSplitByCut> & facesSplitted, GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & facesDeleted, MisMedicOrgan_Ordinary * organ) {
	int i, j;
	nailPoint_t* currentNailPoint;

	for (i = 0; i < facesDeleted.size(); i++) {
		for (j = 0; j < nailPointCount; j++) {
			currentNailPoint = ((nailPoint_t*)nailPoint) + j;
			if (!currentNailPoint->visible) continue;
			if (currentNailPoint->face == facesDeleted[i]) {
				pInstanceManager->delInstance(currentNailPoint->instanceID);
				currentNailPoint->visible = 0;

			}
		}
	}

	int beginid = 0;
	int endid = 0;
	while (endid < nailPointCount)
	{
		nailPoint_t & cnailPt = ((nailPoint_t*)nailPoint)[endid];
		if (cnailPt.visible != 0)
		{
			((nailPoint_t*)nailPoint)[beginid] = cnailPt;
			//if (beginid != endid)//NOT NECESSARARY
			 //   cnailPt.visible = 0;//NOT NECESSARARY
			beginid++;
		}
		endid++;
	}
	nailPointCount = beginid;
}


void MisMedicEndoGiaClips::calc() {
	int i;
	GFPhysVector3 gp1, gp2, gp3;
	nailPoint_t* currentNailPoint;

	for (i = 0; i < nailPointCount; i++) {
		currentNailPoint = ((nailPoint_t*)nailPoint) + i;
		if (!currentNailPoint->visible) continue;
		gp1 = currentNailPoint->line1A->m_CurrPosition*currentNailPoint->line1Lambda + currentNailPoint->line1B->m_CurrPosition*(1.0f - currentNailPoint->line1Lambda);
		gp2 = currentNailPoint->line2A->m_CurrPosition*currentNailPoint->line2Lambda + currentNailPoint->line2B->m_CurrPosition*(1.0f - currentNailPoint->line2Lambda);
		gp3 = gp1*currentNailPoint->lambda + gp2*(1.0f - currentNailPoint->lambda);
		currentNailPoint->currentPos = gp3;
		
		// Ãæ·¨Ïß
		gp1 = currentNailPoint->line1A->m_CurrPosition;
		gp2 = currentNailPoint->line1B->m_CurrPosition;
		gp3 = currentNailPoint->line2A->m_CurrPosition;
		if (gp1.Distance(gp3) < 1e-6 || gp2.Distance(gp3) < 1e-6) gp3 = currentNailPoint->line2B->m_CurrPosition;
		currentNailPoint->currentDirZ = (gp1 - gp2).Cross(gp1 - gp3);
		
		// Point Normal
		gp1 = currentNailPoint->line1A->m_Normal*currentNailPoint->line1Lambda + currentNailPoint->line1B->m_Normal*(1.0f - currentNailPoint->line1Lambda);
		gp2 = currentNailPoint->line2A->m_Normal*currentNailPoint->line2Lambda + currentNailPoint->line2B->m_Normal*(1.0f - currentNailPoint->line2Lambda);
		gp3 = gp1.Normalized()*currentNailPoint->lambda + gp2.Normalized()*(1.0f - currentNailPoint->lambda);
		currentNailPoint->currentDirZ = gp3;

		gp1 = currentNailPoint->line1A->m_CurrPosition*currentNailPoint->line1Lambda + currentNailPoint->line1B->m_CurrPosition*(1.0f - currentNailPoint->line1Lambda);
		gp2 = currentNailPoint->line2A->m_CurrPosition*currentNailPoint->line2Lambda + currentNailPoint->line2B->m_CurrPosition*(1.0f - currentNailPoint->line2Lambda);
		currentNailPoint->currentDirX = gp1 - gp2;
		currentNailPoint->currentDirY = currentNailPoint->currentDirZ.Cross(currentNailPoint->currentDirX);
		currentNailPoint->currentDirX = currentNailPoint->currentDirY.Cross(currentNailPoint->currentDirZ);

		currentNailPoint->currentDirX.Normalize();
		currentNailPoint->currentDirY.Normalize();
		currentNailPoint->currentDirZ.Normalize();


		Ogre::Quaternion q=Ogre::Quaternion(GPVec3ToOgre(currentNailPoint->currentDirX), GPVec3ToOgre(currentNailPoint->currentDirY), GPVec3ToOgre(currentNailPoint->currentDirZ));


		pInstanceManager->updateInstance(currentNailPoint->instanceID, Ogre::Vector3(0.15f, 0.15f, 0.15f), Ogre::Vector3(currentNailPoint->currentPos.GetX(), currentNailPoint->currentPos.GetY(), currentNailPoint->currentPos.GetZ()), q);

		//sprintf(name, "pt%d", i);
		//cw_vec3(gp3.GetX(), gp3.GetY(), gp3.GetZ(), p3);
		//MisNewTraining_drawSingleSphere(name, p3, 0.05f);

	}
}

void MisMedicEndoGiaClips::draw() {
	
}

