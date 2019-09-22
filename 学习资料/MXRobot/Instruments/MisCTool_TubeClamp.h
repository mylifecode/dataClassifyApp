#ifndef _MISCTOOL_TUBECLAMP_
#define _MISCTOOL_TUBECLAMP_
#include "MXOgreWrapper.h"
#include "ITraining.h"
#include "ToolsMgr.h"
#include "OgreMaxScene.hpp"
#include "MXOgreWrapper.h"
#include "IObjDefine.h"

class MisCTool_PluginClamp;
class ACTubeShapeObject;

class MisCTool_TubeClamp
{
public:
	struct ClampedTubeSegment
	{
		GFPhysSoftTubeSegment * m_segMent;
		GFPhysSoftTube * m_Tube;
		ACTubeShapeObject * m_Object;
		GFPhysVector3 m_RegCoord[2];
		int m_Reg[2];
		GFPhysQuaternion m_SegQuatRelToTool;

		GFPhysVector3 m_RotLambda;
	};
	MisCTool_TubeClamp(MisNewTraining * train ,
		               MisCTool_PluginClamp * clampPlugin);

	virtual ~MisCTool_TubeClamp();

	bool CheckTubeBeClamped(std::vector<ACTubeShapeObject*> & tubeObject);

	void ReleaseClampedSegment();

	void PrepareSolve(float dt);

	void Solve(float dt , float thickNess);

protected:
	float  ClipSegmentByCells(GFPhysSoftTubeSegment & segMent);

	int    TestSegmentContactClampReg(GFPhysSoftTubeSegment & segMent , float segRadius);

	MisNewTraining * m_train;
	MisCTool_PluginClamp * m_ClampPlugin;
	GFPhysVectorObj<ClampedTubeSegment> m_SegmentsClamped;

};
#endif