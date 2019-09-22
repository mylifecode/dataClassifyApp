#ifndef _PARTICLEORGANFACECOLLIDEALOGRITHM_
#define _PARTICLEORGANFACECOLLIDEALOGRITHM_
#include <vector>
#include <hash_map>
#include "MisMedicOrganOrdinary.h"
#include "Ogre.h"
/*
 particle collide whit MismedicOrgan_Mesh's face
*/
class ParticleCollideOrganFaceData
{
public:
	int   m_FaceID;
	float m_collideWeights[3];
	Ogre::Vector3 m_FaceNormal;//cached prevent recalculate
};

/*
@ abstract class for particle organ mesh face collide algorithm
*/
class ParticleOrganMeshFaceCollideAlgorithm
{
public:
	ParticleOrganMeshFaceCollideAlgorithm()
	{}

	virtual ~ParticleOrganMeshFaceCollideAlgorithm(){}

	virtual void Initialize(MisMedicOrgan_Ordinary * object) = 0;

	virtual bool GetBloodFaceVertexPosition(int faceid , Ogre::Vector3 vertexPos[3]) = 0;

	virtual bool SelectNextFaceBloodMoveTo(int CurrFace , 
										   const float CurrWeights[],
										   const Ogre::Vector3 &CurrVel,
										   const Ogre::Vector3 &Gravitydir,
										   ParticleCollideOrganFaceData & NextFace) = 0;
	virtual bool IsFaceRemoved(int CurrFace) = 0;//face may be removed or delete via cut operation
	virtual int  GetBloodFaceCount() =0;
};

/*
  preset particle organ face collide algorithm simple implement
  just collect faces incident to vertex and edge's for collide use
  when a particle collide a face's edge or node. just collect the faces incident the node 
  or edges
*/
class PresetParticleOrganFaceCollideAlogrithm : public ParticleOrganMeshFaceCollideAlgorithm
{
public:
	class VertexIncidentFace
	{
	public:
		VertexIncidentFace(){}
		std::vector<unsigned int> m_Incidentfaces;
	};

	virtual ~PresetParticleOrganFaceCollideAlogrithm();

	virtual void Initialize(MisMedicOrgan_Ordinary * object);

	virtual void GetCollidedFaces(int FaceId , 
								  const float weigths[3] , 
								  std::vector<ParticleCollideOrganFaceData> & CollideFacesId
								  );

	virtual bool SelectNextFaceBloodMoveTo( int CurrFace , 
											const float CurrWeights[],
											const Ogre::Vector3 &CurrVel,
											const Ogre::Vector3 &Gravitydir,
											ParticleCollideOrganFaceData & NextFace);

	virtual bool GetBloodFaceVertexPosition(int faceid , Ogre::Vector3 vertexPos[3]);

	virtual int  GetBloodFaceCount();

	virtual bool IsFaceRemoved(int faceid);
protected:

	bool GetBloodFaceVertexIndex(int faceid , int vertexIndex[3]);

	void GetCollidedFacesIncidentSoftNode(GFPhysSoftBodyNode * node , std::vector<ParticleCollideOrganFaceData> & CollideFacesId);

	void GetCollidedFacesIncidentEdge(GFPhysSoftBodyNode * node0 , GFPhysSoftBodyNode * node1 , float weight0 , std::vector<ParticleCollideOrganFaceData> & CollideFacesId);

	void GetCollidedFacesIncidentVertex(int vertexID , std::vector<ParticleCollideOrganFaceData> & CollideFacesId);

	void GetCollidedFacesIncidentEdge(int vid0 , int vid1 , float weight0 , std::vector<ParticleCollideOrganFaceData> & CollideFacesId);

	void CollectFVAdjacent(MisMedicOrgan_Ordinary * object);

	stdext::hash_map<unsigned int , VertexIncidentFace*> m_vertIncidentFaces;

	stdext::hash_map<GFPhysSoftBodyNode * , VertexIncidentFace*> m_SoftNodeIncidentFaces;

	bool m_isadjinfocollected;

	MisMedicOrgan_Ordinary * m_HostObject;
};

#endif