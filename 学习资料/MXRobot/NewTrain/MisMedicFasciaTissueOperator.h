#ifndef _MISMEDICFASICATISSUEOPERATOR_
#define _MISMEDICFASICATISSUEOPERATOR_
#include "collision/GoPhysCollisionlib.h"
#include "Dynamic/GoPhysDynamicLib.h"
#include "Topology/GoPhysSoftBodyCutter.h"

using namespace GoPhys;

class MisMedicOrgan_Ordinary;

class MisMedicFasciaTissueOperator
{
public:
	MisMedicFasciaTissueOperator(MisMedicOrgan_Ordinary * organ);

	virtual ~MisMedicFasciaTissueOperator();

	void SeperateFasciaAround(const std::vector<GFPhysSoftBodyFace*> & faces);

	void SeperateFasciaAround(const GFPhysVector3 & centerPos);//test

	MisMedicOrgan_Ordinary * m_OrganObject;
};

#endif