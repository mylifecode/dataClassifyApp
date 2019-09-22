#pragma once
#include "XMLSerialize.h"
/*
Example:
<WaterPools>
	//CenterAndStageHeight is uses as (originCenterX,originCenterY,stageHeight)
	<WaterPool Origin="0,0,0" Normal="0,1,0" MeshWidth="5"   MeshHeight="5"   ActualWidth="1.1"   ActualHeight="1.1" MaxHeight="5" RejectOrganIDs="1,2,3"  CenterAndStageHeight="0.5,0.5,2.2" ></WaterPool>
</WaterPools>
*/
class CXMLWrapperWaterPool : public CXMLSerialize
{
public:
	CXMLWrapperWaterPool(void);
	~CXMLWrapperWaterPool(void);

	IMPL_ATTRIBUTE_FLOAT3(Origin)
	IMPL_ATTRIBUTE_FLOAT3(Normal)

	IMPL_ATTRIBUTE_FLOAT(MeshWidth)
	IMPL_ATTRIBUTE_FLOAT(MeshHeight)
	IMPL_ATTRIBUTE_FLOAT(ActualWidth)
	IMPL_ATTRIBUTE_FLOAT(ActualHeight)
	IMPL_ATTRIBUTE_FLOAT(MaxHeight)
	IMPL_ATTRIBUTE_FLOAT3(CenterAndStageHeight)  //originCenterX,originCenterY,stageHeight
	IMPL_ATTRIBUTE_BOOL(IsCenterChangeBySuction)
	IMPL_ATTRIBUTE_STRING(MeshName)

	IMPL_ATTRIBUTE_STRING(RejectOrganIDs)
	IMPL_ATTRIBUTE_BOOL(IsSoakOrgans)

	DECLARE_SERIALIZATION_CLASS(CXMLWrapperWaterPool)
};

