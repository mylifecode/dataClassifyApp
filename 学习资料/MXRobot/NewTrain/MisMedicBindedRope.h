#ifndef _MISMEDICBINDEDROPE_
#define _MISMEDICBINDEDROPE_
#include "MisMedicTubeShapeRendObject.h"
#include "MisMedicOrganAttachment.h"
class MisMedicThreadRope;


class MisMedicBindedRope : public MisMedicOrganAttachment
{
public:
	enum ConnectionState
	{
		CS_CONNECTED,
		CS_UNCONNECTED
	};

	class FaceIntersectPoint
	{
	public:
	//	FaceIntersectPoint();

		FaceIntersectPoint(GFPhysSoftBodyFace * face);

		void AddEdgePoint(GFPhysSoftBodyNode * epoint0 , GFPhysSoftBodyNode * epoint1 ,float weight0);

		int m_PointCount;
		GFPhysSoftBodyNode * m_s[2][2];
		float m_Weight[2];
		Ogre::Vector3 m_Position[2];
		GFPhysSoftBodyFace * m_Face;

		int m_LinkedNext;
	};

	class ThreadBindPoint
	{
	public:
		ThreadBindPoint();

		ThreadBindPoint(const Ogre::Vector3 & position , GFPhysSoftBodyFace * face);

		void ReAttach(GFPhysSoftBodyFace * face , float weights[3]);

		Ogre::Vector3 GetPassPointPosition() const;

		Ogre::Vector3 GetPassPointUndeformedPosition() const;

		Ogre::Vector3 m_UndeformedPos;

		Ogre::Vector3 m_OriginUndeformPos;

		Ogre::Vector3 m_ShrinkUndefPos;

		GFPhysSoftBodyFace * m_AttachFace;

		float m_Weights[3];

		float m_Temp;//only for temp use

		float m_EdgeLen;
	};

	MisMedicBindedRope(Ogre::SceneManager * sceneMgr);
	
	~MisMedicBindedRope();
	
	bool TryBindThread( MisMedicOrgan_Ordinary & organ , 
						GFPhysAlignedVectorObj<GFPhysVector3> & threadPoints,
						const GFPhysVector3 & loopFixPoint,
						float MaxDeviate);
	
	
	/*@@tighten the already binded organ*/
	void TightenBindedOrgan(float tightpercent);

	/*@@calculate the closet point of the binded point to src position*/
	bool GetClosetBindedPoint(const GFPhysVector3 & srcPos , MisMedicBindedRope::ThreadBindPoint & bindpoint);

	/*@@ total length of the binded thread in the organ*/
	float GetBindTotalLength(bool materialSpace = true);

	/*@@*/
	void BuildBindRopeSegments();

	void CalcBindCircleSegLen();
	//@@overridden attachment
	virtual void Update(float deltatime);

	//@@overridden
	//rope segment attached face may be splits due to cut or electric hollow
	//this method rebind the node point to closet face

    virtual void OnFaceSplittedByCut(std::vector<FaceSplitByCut> & facesSplitted ,GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & facesDeleted, MisMedicOrgan_Ordinary * organ);

    
	//@@overridden
	virtual void OnCutByToolFinish();

	void OnRemoveTetrahedron(GFPhysSoftBodyTetrahedron * tetra);

	//virtual void OnRemoveCuttedTetrahedron(const GFPhysVectorObj<GFPhysSoftBodyTetrahedron*> & removedtetras);

	int GetNumBindPoints();

	const ThreadBindPoint & GetBindPoint(int index);

	void GetBindPoints(std::vector<ThreadBindPoint>& bindPoints);

	MisMedicOrgan_Ordinary* GetBindedOrgan();

	bool  GetApproximateLoopPlane_MaterialSpace(GFPhysVector3 & planeNormal , GFPhysVector3 & planePoint);

	void CreateKnotNode(Ogre::SceneManager * sceneMgr , GFPhysSoftBodyFace *pAttachedFace , float weights[3]);

	void SetKnotNodeDir(const Ogre::Vector3 & dir);

	void SetConnected() {m_ConnectState = CS_CONNECTED;}

	void SetUnConnected() {m_ConnectState = CS_UNCONNECTED;}

	inline void SetUserData(int data) {m_UserData = data;}

	inline int GetUserData() {return m_UserData;}
	
	inline const ThreadBindPoint& GetKnotPoint() {return m_KnotPoint;}


protected:

	void SelectInterSectLoopTetras(GFPhysVectorObj<GFPhysSoftBodyTetrahedron*> & intersectTetras);

	bool m_NeedRend;

	std::vector<std::vector<Ogre::Vector3>> m_SegmentRendPos;

	std::vector<int> m_BindSegments;

	//rend object for ogre
	MisMedicTubeShapeRendObject m_RendObject;
	
	//temp variable for convience
	std::vector<FaceIntersectPoint> m_FaceIntersectPoints;

	//bind mode point
	std::vector<ThreadBindPoint>  m_ThreadBindPoint;
	
	MisMedicOrgan_Ordinary * m_BindedOrgan;//绑住的器官

	ThreadBindPoint m_KnotPoint;
	Ogre::SceneNode * m_KnotNode;						//绳结节点
	
	Ogre::Entity * m_KnotEnitity;						//绳结entity

	//GFPhysSoftBodyFace * m_pFaceWithKnot;

	//float m_FaceWeightsForKnot[3];

	ConnectionState m_ConnectState;

	int m_BoundRopeID;

	int m_UserData;

};

#endif