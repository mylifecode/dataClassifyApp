#pragma once
#include "XMLSerialize.h"

class CXMLWrapperAdhesionCluster : public CXMLSerialize
{
public:
	CXMLWrapperAdhesionCluster(void);
	~CXMLWrapperAdhesionCluster(void);

	IMPL_ATTRIBUTE_LONG(AdhesionID)
	IMPL_ATTRIBUTE_FLOAT(Range)
	IMPL_ATTRIBUTE_FLOAT(Ratio)
	IMPL_ATTRIBUTE_STRING(OrganIDs)
	IMPL_ATTRIBUTE_BOOL(IsScale)
	IMPL_ATTRIBUTE_BOOL(IsAutoDir)
	IMPL_ATTRIBUTE_FLOAT(ScaleFactor)
	IMPL_ATTRIBUTE_FLOAT(ConstraintStiffness)
	IMPL_ATTRIBUTE_STRING(NodeIndicesA)
	IMPL_ATTRIBUTE_STRING(NodeIndicesB)


	DECLARE_SERIALIZATION_CLASS(CXMLWrapperAdhesionCluster)
};

