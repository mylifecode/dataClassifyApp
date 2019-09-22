#ifndef _MISORGANANIMATION_
#define _MISORGANANIMATION_
#include <Ogre.h>
#include "PhysicsWrapper.h"

class MisOrganAnimation : public GFPhysSoftBodyConstraint
{
public:

	class FrameControlData
	{
	public:
		std::vector<float> m_TetraEnlarge;
		std::vector<float> m_EdgeEnlarge;
	};

	class AnimatedEdge
	{
	public:
		AnimatedEdge(GFPhysSoftBodyEdge * edge)
		{
			m_physEdge = edge;
			m_RestLen = m_physEdge->m_RestLength;
		}
		GFPhysSoftBodyEdge * m_physEdge;
		int m_NodeIndex[2];//for convenient
		float m_RestLen;
	};

	class AnimatedTetra
	{
	public:
		AnimatedTetra(GFPhysSoftBodyTetrahedron * tetra)
		{
			m_PhysTetra = tetra;
			m_RestVol = tetra->m_RestSignedVolume;
		}
		GFPhysSoftBodyTetrahedron * m_PhysTetra;
		int m_NodeIndex[4];//for convenient
		float m_RestVol;
	};

	MisOrganAnimation();

	~MisOrganAnimation();

	virtual void PrepareSolveConstraint(Real Stiffness,Real TimeStep);

	virtual void SolveConstraint(Real Stiffness,Real TimeStep);

	void SerializeFromCookedAnimFile(const Ogre::String & filename , GFPhysSoftBody * DestSb);
	
	void LoadFromAnimedMesh( Ogre::Mesh * animMesh , GFPhysSoftBody * DestSb , const Ogre::Vector3 & offset );

protected:
	bool  m_SolveThisFrame;
	float m_TimeElapsed;
	int   m_FrameCurrent;
	std::vector<AnimatedEdge>  m_AnimatedEdges;
	std::vector<AnimatedTetra> m_AnimatedTetras;

	
	std::vector<FrameControlData*> m_ResultControlData;
	std::vector<Ogre::Vector3>     m_ResultInitFramePos;
	std::map<GFPhysSoftBodyNode* , GFPhysVector3> m_NodeDestPos;
};


#endif