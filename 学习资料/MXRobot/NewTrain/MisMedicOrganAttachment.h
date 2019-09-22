#ifndef _MISMEDICORGANATTACHMENT_
#define _MISMEDICORGANATTACHMENT_
#include <Ogre.h>
#include "collision/GoPhysCollisionlib.h"
#include "Dynamic/GoPhysDynamicLib.h"
#include "MisMedicOrganInterface.h"
#include "MXOgreGraphic.h"
#include "GeometryInstanceManager.h" 

using namespace GoPhys;

class ITool;
class MisMedicOrgan_Tube;
class MisMedicOrgan_Ordinary;
class NodeOnPlaneCS;

enum MisOrganAttachmentType
{
	MOAType_UnKnow,
	MOAType_TiantumClip,
	MOAType_SilverClip,
	MOAType_BindedRope,
    MOAType_Nail,//直线切割器版本1
	MOAType_StraightClip,//直线切割器版本2
};

class MisMedicOrganAttachment
{
public:
	MisMedicOrganAttachment()
	{
		m_type = MOAType_UnKnow;
	}
	virtual ~MisMedicOrganAttachment(void){}
	MisOrganAttachmentType m_type;
	virtual void Update(float deltatime) = 0;
	virtual void OnCutByToolFinish()
	{}

	virtual void OnRemoveTetrahedron(GFPhysSoftBodyTetrahedron * tetra)
	{}

	virtual void OnFaceSplittedByCut(std::vector<FaceSplitByCut> & facesSplitted ,GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & facesDeleted,  MisMedicOrgan_Ordinary * organ) = 0;    

	Ogre::Quaternion CalcFaceRotate(const GFPhysVector3 srcPos[3] , const GFPhysVector3 dstPos[3]);

};


class MisMedicTitaniumClampV2 : public MisMedicOrganAttachment
{
public:
	struct ClipCoord
	{
		float m_AxisPointWeight[3];
	};
	MisMedicTitaniumClampV2(float thick,
		                    Ogre::Vector3 clipAxis[3],
		                    Ogre::Vector2 clipBound[2],
							GFPhysSoftBodyFace * attachFace,
							MisMedicOrgan_Ordinary * organ,
							int toolflag);
	~MisMedicTitaniumClampV2();

	void ExtractTriangelCoord(Ogre::Vector3 triVerts[3] , const Ogre::Vector3 & refPoint);

	bool GetTriangleCoordAxis(Ogre::Vector3 vert[3], Ogre::Vector3 & resultOrigin , Ogre::Vector3 resultAxis[3]);

	void Update(float deltatime);
	
    void OnFaceSplittedByCut(std::vector<FaceSplitByCut> & facesSplitted ,GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & facesDeleted, MisMedicOrgan_Ordinary * organ)
    {}
	void OnCutByToolFinish();

	int  getGenerateToolState();
	
	bool RefindAttachFace();

	GFPhysSoftBodyFace * getRelativeFace()
	{
		return m_AttachedFace;
	}

	/** aabb */
	void GetLocalAABB(GFPhysVector3& min,GFPhysVector3& max);

	/** translation */
	inline GFPhysVector3 GetTranslation()
	{
		if(m_ScenenNode)
			return OgreToGPVec3(m_ScenenNode->_getDerivedPosition());
		else
			return GFPhysVector3(0.f,0.f,0.f);
	}

	/** rotation*/
	inline GFPhysQuaternion GetQuaternion()
	{
		if(m_ScenenNode)
			return OgreToGPQuaternion(m_ScenenNode->_getDerivedOrientation());
		else return GFPhysQuaternion(0.f,0.f,0.f,1.f);
	}
	
	float GetValidClipPercent();

	int   GetTriEdgesIntersectClip(Ogre::Vector3 triVerts[3], Ogre::Vector3 intersectPts[3]);//this only return true or false ,improve to return intersectpt

	float m_InvalidClipLen;

	float m_TotalClipLen;

	ClipCoord m_Coord;

	//Ogre::Vector3 m_DesiredClipDir;
	Ogre::Vector3 m_DesiredClipAixs[3];

	Ogre::Vector2 m_DesiredClipBounds[3];

	float m_clipPosInTriFrame[3];
	Ogre::Quaternion m_clipRotInTriFrame;

	GFPhysVector3 m_MeshLocalMin;
	GFPhysVector3 m_MeshLocalMax;
protected:

	void CalculateTitanumLocalInformation(GFPhysSoftBodyFace * attachface);

	MisMedicOrgan_Ordinary * m_HostOrgan;
	ITool * m_pTitaniumClip;//

	//Ogre::Quaternion m_OrientInClipTime;
	//GFPhysVector3 m_FacePosInClipTime[3];
	//Ogre::Vector3 m_RelPosToFaceInClipTime;

	
	GFPhysSoftBodyFace * m_AttachedFace;
	float m_FallDownTime;

	Ogre::Vector3 m_AttachFaceRestNormal;
	int m_RefFaceId;
	int m_RefFaceMaterialID;
	int m_GenerateToolState;
	Ogre::SceneNode * m_ScenenNode;

};

class MisMedicSilverClamp : public MisMedicOrganAttachment
{
public:


	MisMedicSilverClamp(GFPhysSoftBodyFace * attachFace,
						GFPhysSoftBodyFace * DeformFaces[] , 
						int DeformFacesNum , 
						GFPhysSoftBodyFace * FaceForOreint[2],
						MisMedicOrgan_Ordinary * organ , 
						const Ogre::Vector3 & ToolWorldPos , 
						const Ogre::Quaternion & ToolWorldOrient ,
						const Ogre::Vector3 & scale, 
						int toolflag);

	~MisMedicSilverClamp();

	
	void Update(float deltatime);
	
    void OnFaceSplittedByCut(std::vector<FaceSplitByCut> & facesSplitted ,GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & facesDeleted, MisMedicOrgan_Ordinary * organ);    

protected:
	void CalculateTitanumLocalInformation(GFPhysSoftBodyFace * attachface, const Ogre::Vector3 & position , const Ogre::Quaternion & orient);

	MisMedicOrgan_Ordinary * m_HostOrgan;
	ITool * m_pSilverClip;//
	
	Ogre::Quaternion m_OrientInClipTime;
	GFPhysVector3 m_FacePosInClipTime[3];
	Ogre::Vector3 m_RelPosToFaceInClipTime;


	GFPhysSoftBodyFace * m_relativeFace;
	int m_RefFaceId;
	int m_RefFaceMaterialID;
	int m_GenerateToolState;
	Ogre::SceneNode * m_ScenenNode;
};

class MisMedicEndoGiaClips : public MisMedicOrganAttachment
{
public:
	MisMedicEndoGiaClips(const char* name_);
	~MisMedicEndoGiaClips();
	void addClip(GFPhysSoftBodyFace* face, GFPhysSoftBodyNode* line1A, GFPhysSoftBodyNode* line1B, float line1Lambda, GFPhysSoftBodyNode* line2A, GFPhysSoftBodyNode* line2B, float line2Lambda, float lambda);
	Ogre::SceneNode* createSingleClipNode(int idx);
	void calc();
	void draw();
	void Update(float deltatime) ;
	void OnCutByToolFinish();
	void OnRemoveTetrahedron(GFPhysSoftBodyTetrahedron * tetra);
	void OnFaceSplittedByCut(std::vector<FaceSplitByCut> & facesSplitted, GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & facesDeleted, MisMedicOrgan_Ordinary * organ);


protected:
	GeometryInstanceManager* pInstanceManager;
	void* nailPoint;
	int nailPointCount;
	int nailPointSize;
};



#endif