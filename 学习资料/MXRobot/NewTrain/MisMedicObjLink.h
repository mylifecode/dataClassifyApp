#ifndef _MISMEDICOBJLINK_
#define _MISMEDICOBJLINK_
#include "Ogre.h"
#include "collision/GoPhysCollisionlib.h"
#include "Dynamic/GoPhysDynamicLib.h"
#include "Dynamic/Constraint/GoPhysSoftBodyDistConstraint.h"
#include "CustomConstraint.h"

class MisMedicOrgan_Ordinary;
class MisMedicObjetSerializer;

using namespace GoPhys;

class MisMedicObjLink
{
public:

	MisMedicObjLink() : m_ConnectOrganA(0) , m_ConnectOrganB(0)
	{

	}

	virtual ~MisMedicObjLink(){}

	virtual void OnFaceRemoved(GFPhysSoftBodyFace * face){}

	virtual void OnNodesRemoved(const GFPhysAlignedVectorObj<GFPhysSoftBodyNode*> & nodes){}

	virtual void OnTetrasRemoved(const GFPhysAlignedVectorObj<GFPhysSoftBodyTetrahedron*> & tetras){}
	
	MisMedicOrgan_Ordinary * m_ConnectOrganA;

	MisMedicOrgan_Ordinary * m_ConnectOrganB;
};

class MM_NodeToNodeLinkPair
{
public:
	MM_NodeToNodeLinkPair(GFPhysSoftBodyNode * na , GFPhysSoftBodyNode * nb)
	{
		m_NodeInA = na;
		m_NodeInB = nb;
		m_UseCustomWeight = false;
		m_LinksStiffness = 0.99f;
		m_IsValid = true;
	}

	void SetCustomWeights(float wa , float wb)
	{
		m_CustomedWeight[0] = wa;
		m_CustomedWeight[1] = wb;
		m_UseCustomWeight = true;
	}

	GFPhysSoftBodyNode * m_NodeInA;
	GFPhysSoftBodyNode * m_NodeInB;
	float m_CustomedWeight[2];
	float m_LinksStiffness;
	bool  m_UseCustomWeight;
	bool  m_IsValid;
};

#endif