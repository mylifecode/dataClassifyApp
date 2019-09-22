#ifndef _VOLUMEBLOOD_
#define _VOLUMEBLOOD_

#include "Ogre.h"
#include <QTime>

#include "collision\BroadPhase\GoPhysDynBVTree.h"
#include "MisNewTraining.h"

class VolumeBlood
{
public:

	MisMedicOrgan_Ordinary* pOrgan;
	int faceID;
	float faceWeight[3];

	float* disturb;
	float ts;		// Ê±¼ä(s)

	float *splineArr;
	float *shapeArr;
	float *scaleArr;
	int* fceArr;
	float* normArr;
	float* vtxArr;
	float* tgArr;
	float* uvArr;


	float area;
	float flow;
	float coneCoeff;
	float pos[3];
	float norm[3];
	float gravity[3];
	float bloodLen;

	int vQuality;
	int hQuality;

	int collideSecIdx;
	MisMedicOrgan_Ordinary *collideSecOrgan, *collideCheckOrgan;
	int collideNodeID[3];
	float collideFaceTexCoord[2];

	VolumeBlood(float gravity_[3], int vQuality_, int hQuality_, float bloodLen_);
	void setGravity(float gravity_[3]);
	void step(float dt);
	void setCutInfo(float cutRadius, float flow_);
	void setCutInfo(float cutRadius, float flow_, float* pos_, float* norm_);
	void setCutInfo(float cutRadius, float flow_, MisMedicOrgan_Ordinary* pOrgan_, int faceID_, float* weight_);
	int updateCutInfoByOrganFace();
	void draw(ITraining *pTraining);
	~VolumeBlood();
	
	// Collision
	GFPhysDBVTree m_SegmentTree;
	void updateCollideInfo(ITraining *pTraining);


private:
	int m_id;
	Ogre::String m_materialName;
	Ogre::String m_manualObjectName;
	Ogre::GpuProgramParametersSharedPtr m_vertexProgramParamPtr;

};

#define cw_pi()		3.14159265358979323846
#define cw_max(a, b)       ( ((a) > (b)) ? (a) : (b) )
#define cw_min(a, b)       ( ((a) <= (b)) ? (a) : (b) )
#define cw_clamp(val, minVal, maxVal) cw_min(maxVal, cw_max(minVal, val))
#define cw_sub3(a, b, c)	{ (c)[0]=(a)[0]-(b)[0]; (c)[1]=(a)[1]-(b)[1]; (c)[2]=(a)[2]-(b)[2]; }
#define cw_subscale3(a, b, v, c)	{ c[0]=(a[0]-b[0])*v; c[1]=(a[1]-b[1])*v; c[2]=(a[2]-b[2])*v; }
#define cw_add3(a, b, c)	{ (c)[0]=(a)[0]+(b)[0]; (c)[1]=(a)[1]+(b)[1]; (c)[2]=(a)[2]+(b)[2]; }
#define cw_dotprod3(a, b)	( (a)[0]*(b)[0]+(a)[1]*(b)[1]+(a)[2]*(b)[2])
#define cw_mix(a, b, eta)	(eta*a+(1-eta)*b)
#define cw_scale3(a, b, c)	{ (c)[0]=(a)[0]*(b); (c)[1]=(a)[1]*(b); (c)[2]=(a)[2]*(b); }
#define cw_copy3(a, b)	{ (b)[0]=(a)[0]; (b)[1]=(a)[1]; (b)[2]=(a)[2]; }
#define cw_addsum3(a, b, c)	{ (c)[0]+=(b)*(a)[0]; (c)[1]+=(b)*(a)[1]; (c)[2]+=(b)*(a)[2]; }
#define cw_lerp(a, b, w)  (a)*(1.0-(w))+(b)*(w)
#define cw_int(b)	(int)((b)+(b)>0?0.5:-0.5);
#define cw_cross3(a, b, c)	{ (c)[0]=(a)[1]*(b)[2]-(b)[1]*(a)[2]; (c)[1]=(a)[2]*(b)[0]-(b)[2]*(a)[0]; (c)[2]=(a)[0]*(b)[1]-(b)[0]*(a)[1]; }
#define cw_ifloor(b)	((int)floor(b))
#define cw_vec3(a, b, c, v)	{ (v)[0]=a; (v)[1]=b; (v)[2]=c; }
#define cw_addsum3(a, b, c)	{ (c)[0]+=(b)*(a)[0]; (c)[1]+=(b)*(a)[1]; (c)[2]+=(b)*(a)[2]; }

#endif