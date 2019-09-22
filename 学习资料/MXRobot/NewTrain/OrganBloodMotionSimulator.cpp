#include "math/GophysTransformUtil.h"
#include "../MXCommon/stdafx.h"
#include "OrganBloodMotionSimulator.h"
#include "DynamicParticle.h"
#include "MisMedicOrganOrdinary.h"
//#include "MisMedicOrganTube.h"
#include "DynamicObjectRenderable.h"
// Helper function
static Ogre::Vector3 transToOgreVec(GoPhys::GFPhysVector3 & gpvec)
{
	return Ogre::Vector3(gpvec.m_x , gpvec.m_y , gpvec.m_z);
}
static void ExtractPointWeights(Ogre::Vector3 facevert[3] , Ogre::Vector3 p , float weights[3])
{
	Ogre::Vector3 a = facevert[0];

	Ogre::Vector3 b = facevert[1];

	Ogre::Vector3 c = facevert[2];

	Ogre::Vector3 v0 = b-a;

	Ogre::Vector3 v1 = c-a;

	Ogre::Vector3 v2 = p-a;

	double d00 = v0.dotProduct(v0);
	double d01 = v0.dotProduct(v1);
	double d11 = v1.dotProduct(v1);
	double d20 = v2.dotProduct(v0);

	double d21 = v2.dotProduct(v1);
	double denom = d00*d11-d01*d01;

	float v = 0;
	float w = 0;
	if(fabs(denom) > 1e-10F)
	{
		v = (float)((d11 * d20 - d01 * d21) / denom);
		w =  (float)((d00 * d21 - d01 * d20) / denom);
	}

	float u = 1.0f-v-w;

	if(u < 0) u = 0;
	if(u > 1) u = 1;

	if(v < 0) v = 0;
	if(v > 1) v = 1;

	if(w < 0) w = 0;
	if(w > 1) w = 1;

	float sum = u+v+w;
	weights[0] = u / sum;
	weights[1] = v / sum;
	weights[2] = w / sum;
}

static void ExtractVectorWeights(Ogre::Vector3 facevert[3] , Ogre::Vector3 vector , float weights[3])
{
	float u , v , w;
	u = v = w = 0.3333f;

	Ogre::Vector3 v0 = facevert[1]-facevert[0];

	Ogre::Vector3 v1 = facevert[2]-facevert[0];

	double d00 = v0.dotProduct(v0);
	double d01 = v0.dotProduct(v1);
	double d11 = v1.dotProduct(v1);
	double denom = d00*d11-d01*d01;

	double deta20 = vector.dotProduct(v0);
	double deta21 = vector.dotProduct(v1);
	if(fabs(denom) > 1e-10F)
	{
		v = (float)((d11 * deta20 - d01 * deta21) / denom);
		w = (float)((d00 * deta21 - d01 * deta20) / denom);
		u = -v-w;
	}
	weights[0] = u;
	weights[1] = v;
	weights[2] = w;
}

static int PointToTriangleDist( Ogre::Vector3 trianglevert[3], Ogre::Vector3 point , float weights[3] , float & dist)
{
	GoPhys::GFPhysVector3 a(trianglevert[0].x , trianglevert[0].y , trianglevert[0].z);
	GoPhys::GFPhysVector3 b(trianglevert[1].x , trianglevert[1].y , trianglevert[1].z);
	GoPhys::GFPhysVector3 c(trianglevert[2].x , trianglevert[2].y , trianglevert[2].z);

	GoPhys::GFPhysVector3 p(point.x , point.y , point.z);

	// Check if P in vertex region outside A
	GoPhys::GFPhysVector3 ab = b - a;
	GoPhys::GFPhysVector3 ac = c - a;
	GoPhys::GFPhysVector3 ap = p - a;
	float d1 = ab.Dot(ap); 
	float d2 = ac.Dot(ap);
	if (d1 <= 0.0f && d2 <= 0.0f)// barycentric coordinates (1,0,0)
	{
		weights[0] = 1.0f; weights[1] = 0.0f; weights[2] = 0.0f;
		GoPhys::GFPhysVector3 distvec = (p-a);
		dist = distvec.Length();
		return 1;
	}

	// Check if P in vertex region outside B
	GoPhys::GFPhysVector3 bp = p - b;
	float d3 = ab.Dot(bp);
	float d4 = ac.Dot(bp);
	if (d3 >= 0.0f && d4 <= d3) // barycentric coordinates (0,1,0)
	{
		weights[0] = 0.0f; weights[1] = 1.0f; weights[2] = 0.0f;
		GoPhys::GFPhysVector3 distvec = (p-b);
		dist = distvec.Length();
		return 1;
	}

	// Check if P in edge region of AB, if so return projection of P onto AB
	float vc = d1*d4 - d3*d2;

	if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
		float v = d1 / (d1 - d3);
		weights[0] = 1-v; weights[1] = v; weights[2] = 0.0f;
		GoPhys::GFPhysVector3 distvec = (p-(a*(1-v)+b*v));
		dist = distvec.Length();//a + v * ab; // barycentric coordinates (1-v,v,0)
		return 1;
	}

	// Check if P in vertex region outside C
	GoPhys::GFPhysVector3 cp = p - c;
	float d5 = ab.Dot(cp);
	float d6 = ac.Dot(cp);
	if (d6 >= 0.0f && d5 <= d6) // barycentric coordinates (0,0,1)
	{
		weights[0] = 0; weights[1] = 0; weights[2] = 1;
		GoPhys::GFPhysVector3 distvec = (p-c);
		dist = distvec.Length();
		return 1;
	}

	// Check if P in edge region of AC, if so return projection of P onto AC
	float vb = d5*d2 - d1*d6;
	if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {// barycentric coordinates (1-w,0,w)
		float w = d2 / (d2 - d6);
		weights[0] = 1-w; weights[1] = 0; weights[2] = w;
		GoPhys::GFPhysVector3 distvec = (p-(a*(1-w)+c*w));
		dist = distvec.Length();
		return 1;
	}

	// Check if P in edge region of BC, if so return projection of P onto BC
	float va = d3*d6 - d5*d4;
	if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {// barycentric coordinates (0,1-w,w)
		float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
		weights[0] = 0; weights[1] = 1-w; weights[2] = w;
		GoPhys::GFPhysVector3 distvec = (p-(b*(1-w)+c*w));
		dist = distvec.Length();
		return 1;
	}

	// P inside face region. Compute Q through its barycentric coordinates (u,v,w)
	float denom = 1.0f / (va + vb + vc);
	float v = vb * denom;
	float w = vc * denom;
	float u = 1.0f-v-w;
	weights[0] = u; weights[1] = v; weights[2] = w;
	GoPhys::GFPhysVector3 distvec = (p-(a*u + b*v + c*w));
	dist = distvec.Length();//a + ab * v + ac * w; // = u*a + v*b + w*c, u = va * denom = 1.0f - v - w
	return -1;
}

static bool isSameSide(Ogre::Vector3 & endpointa, Ogre::Vector3 & endpointb, Ogre::Vector3 p0 , Ogre::Vector3 p1)
{
	Ogre::Vector3 edge = endpointa-endpointb;
	edge.normalise();

	Ogre::Vector3 vec = p1-p0;

	Ogre::Vector3 perpendvec = vec-edge*vec.dotProduct(edge);

	float v0 = (p0-endpointa).dotProduct(perpendvec);
	float v1 = (p1-endpointa).dotProduct(perpendvec);
	if((v0 < 0 && v1 > 0) || (v0 > 0 && v1 < 0))
		return false;
	else
		return true;
}
//================================================================================================================
DynamicBloodParticle::DynamicBloodParticle(Ogre::Vector3 origin ,  float mass , float radius) : DynamicParticle(origin ,  mass ,  radius)
{
	reset();
}
//================================================================================================================
void DynamicBloodParticle::reset()
{
	DynamicParticle::reset();

	m_fadeoutalpha = 0.4f;

	m_initalpha = 0.4f;

	//m_StartQuantity = 35.0f; 
	m_StartQuantity = 100.0f;

	m_StopQuanity = 30;

	m_QuantityDecPermove = 0.7f;

	m_CurrQuanity = m_StartQuantity;

	m_Startradius = 3.0f;

	m_Endradius = 2.5f;//2.5f;
	m_Organ = 0;
}

void DynamicBloodParticle::setParameterForDynamicInCutFace()
{
	m_CurrQuanity = m_StartQuantity = 15;
	m_QuantityDecPermove = 0.9f;
	m_StopQuanity = 14.5;
}
//============================================================================================================
BloodMoveData::BloodMoveData(float slipVelAcc, float maxSlipVel)
{
	reset( slipVelAcc,  maxSlipVel);
}
//============================================================================================================
void  BloodMoveData::reset(float slipVelAcc, float maxSlipVel)
{
	m_stickedOrgan = NULL;

	m_stickedfaceid = 0xFFFFFFFF;

	m_IsBloodMoveable = true;

	m_exceededface = false;

	m_currfacenorm = Ogre::Vector3::UNIT_Z;

	m_randtangent = false;

	m_positionweights[0] = m_positionweights[1] = m_positionweights[2] = 0;

	m_linearvelweights[0] = m_linearvelweights[1] = m_linearvelweights[2] = 0;

	m_SlipAccelerate = slipVelAcc;// 2.0f;

	m_MaxSlipspeed   = maxSlipVel;// 2.5f;
}

void BloodMoveData::SetStickFaceId(unsigned int sid)
{
	if(sid == 0xFFFFFFFF)
	{
	   m_IsBloodMoveable = false;
	}
	else
	{
	   m_stickedfaceid = sid;
	}
}

/*
BloodInVeinConnect::BloodInVeinConnect()
{
m_isactive = false;
m_clusterID = -1;
m_PairID = -1;
m_randtangent = false;
m_PositionWeight = 0.0f;
m_currslipspeed = 0.01f;
m_slipaccelerate = 0.06f;
m_maxslipspeed = 0.08f;
m_veinconnectobj = 0;
}
*/

//============================================================================================================
OrganBloodMotionSimulator::OrganBloodMotionSimulator()
{
	m_AccmulateTime = 0;
	m_gravitydir = Ogre::Vector3(0, -1, 1 );
	m_gravitydir.normalise();
	m_activeparticlehead = 0;
	m_activeparticleend = 0;
	m_recycledhead = 0;
	m_activeparticlecount = 0;
	m_maxparticlenum = 150;
	m_paritcletangentrate = 0.008f;
	m_HostObject = 0;
	///
	m_bIsTest = true;
}
//=================================================================================================================
OrganBloodMotionSimulator::~OrganBloodMotionSimulator()
{
	DestoryAllParticles();
	for(size_t s = 0 ; s < m_listeners.size() ; s++)
		m_listeners[s]->onSimulatorDestoryed();

}

//=================================================================================================================
void OrganBloodMotionSimulator::Initialize(MisMedicOrgan_Ordinary * bloodobject , bool gravityFlag)
{
	if(!gravityFlag)
		setGravityDir(Ogre::Vector3(0,0,-1));

	m_HostObject = bloodobject;

	if(m_HostObject)
		m_collidealgo.Initialize(m_HostObject);

	//check exists particle to see if the stick face is removed
	//remove dead particles will do this 
	removeDeadParticles(-1);

}
//=================================================================================================================
bool OrganBloodMotionSimulator::Update(float dt)
{
	return updateParticles(dt);
}
//=================================================================================================================
void OrganBloodMotionSimulator::setGravityDir(Ogre::Vector3 gravitydir)
{
	m_gravitydir = gravitydir;
	m_gravitydir.normalise();
}
//=================================================================================================================
void  OrganBloodMotionSimulator::setSlipAccelerate(float slipacc)
{
	//m_slipaccelerate = slipacc;
}
//=================================================================================================================
void  OrganBloodMotionSimulator::setMaxSpeed(float maxspeed)
{
	//m_maxmovespeed = maxspeed;
}
//=================================================================================================================
void  OrganBloodMotionSimulator::setMaxParticleNum(int parnum)
{
	m_maxparticlenum = parnum;
}
//=================================================================================================================
void OrganBloodMotionSimulator::GetBloodParticles(std::vector<DynamicBloodParticle*> & particles)
{
	particles.clear();

	DynamicBloodParticle * particle = m_activeparticlehead;

	while(particle)
	{
		particles.push_back(particle);
		particle = (DynamicBloodParticle*)particle->m_activednext;
	}
}
//=================================================================================================================
bool  OrganBloodMotionSimulator::MoveInCurrentFace(BloodMoveData & movedata , float deltatime, Ogre::Vector3 & endspeed, float &usedtime , bool cleartangent)
{
	unsigned int stickedfaceid = movedata.m_stickedfaceid;

	if(movedata.m_IsBloodMoveable == false)
		return false;

	MisMedicOrgan_Ordinary * stickedOrgan = movedata.m_stickedOrgan;

	Ogre::Vector3 facenormal;

	Ogre::Vector3 facevertex[3];

	Ogre::Vector3 slipdir;

	Ogre::Vector3 tangdir;

	Ogre::Vector3 currentpos;

	Ogre::Vector3 currentvel;

	if(stickedfaceid >= 0 && stickedfaceid < m_collidealgo.GetBloodFaceCount())
	{
		m_collidealgo.GetBloodFaceVertexPosition(stickedfaceid , facevertex);

		Ogre::Vector3 side0 = facevertex[0] - facevertex[1];

		Ogre::Vector3 side1 = facevertex[2] - facevertex[0];

		facenormal = side1.crossProduct(side0);

		facenormal.normalise();//can be optimize one frame for save in one triangle

		slipdir = movedata.m_vectorDir-movedata.m_vectorDir.dotProduct(facenormal)*facenormal;

		slipdir.normalise();//can be optimize one frame for save in one triangle

		tangdir = slipdir.crossProduct(facenormal) ;//can be optimize one frame for save in one triangle

		float * posweights = movedata.m_positionweights;

		float * velweights = movedata.m_linearvelweights;

		currentpos  =  facevertex[0]*posweights[0] + facevertex[1]*posweights[1] + facevertex[2]*posweights[2];

		currentvel  =  facevertex[0]*velweights[0] + facevertex[1]*velweights[1] + facevertex[2]*velweights[2];
	}
	else
	{
		usedtime  = deltatime;
		endspeed = Ogre::Vector3(0,0,0);
		return false;
	}

	float slipspeedstart  = currentvel.dotProduct(slipdir);

	float tangspeedstart  = currentvel.dotProduct(tangdir);

	if (movedata.m_randtangent)//random tangent speed to simulate wrinkle in surface
	{
		float randpercent = (rand()%10-5)/ 5.0f;
		tangspeedstart = 0.8f*randpercent;
	}

	//slip direction speed
	if(slipspeedstart < FLT_EPSILON)//first time 
		slipspeedstart = movedata.m_SlipAccelerate*0.1f;

	float slipspeedend = slipspeedstart+movedata.m_SlipAccelerate*deltatime; //
	if(fabsf(slipspeedend) > movedata.m_MaxSlipspeed)
		slipspeedend *= (movedata.m_MaxSlipspeed / fabsf(slipspeedend));

	//calculate tangent dir speed
	float tangspeedend = tangspeedstart;
	if(fabsf(tangspeedend) > movedata.m_MaxSlipspeed)
		tangspeedend *= (movedata.m_MaxSlipspeed /fabsf(tangspeedend));

	//see if we need clear tangent
	if(cleartangent)
		tangspeedstart = tangspeedend = 0;

	float tangMoveDist = (tangspeedstart+tangspeedend)*deltatime*0.5f;

	float slipMoveDist = (slipspeedstart+slipspeedend)*deltatime*0.5f;

	Ogre::Vector3  predictedmovedvec = slipdir*slipMoveDist+tangdir*tangMoveDist;

	float predictedmovedlen = predictedmovedvec.length();

	Ogre::Vector3  destpos = currentpos+predictedmovedvec;

	//check if we exceed face
	float closepointweights[3];

	float closedist;

	int   closetype = PointToTriangleDist(facevertex, destpos ,  closepointweights ,  closedist);

	//put destination position to closest point in triangle any way
	movedata.m_positionweights[0] = closepointweights[0];
	movedata.m_positionweights[1] = closepointweights[1];
	movedata.m_positionweights[2] = closepointweights[2];
	movedata.m_currfacenorm  = facenormal;

	bool ExceedFace = false;
	if(closetype < 0)//the particle is inside the triangle face
	{
		//get end speed in 3d space
		endspeed = tangdir*tangspeedend+slipdir*slipspeedend;	
		usedtime = deltatime;

		//if(isDownWardFace && VelGravityAngle < 0)//this is  a cheat!! I have no time to impelment a robust algo
		//	endspeed *= -1;

		movedata.m_exceededface = false;
		ExtractVectorWeights( facevertex , endspeed , movedata.m_linearvelweights);
		//return false;
	}
	else//the particle exceed this triangle face
	{
		Ogre::Vector3 finalpos = closepointweights[0]*facevertex[0]+closepointweights[1]*facevertex[1]+closepointweights[2]*facevertex[2];

		float lengthadvanced = (finalpos-currentpos).length();

		float percent = (predictedmovedlen > FLT_EPSILON ? lengthadvanced / predictedmovedlen : 0);
		if( percent > 1)
			percent = 1;

		//get end speed in 3d space
		endspeed = tangdir*tangspeedend+slipdir*slipspeedend;
		usedtime = deltatime*percent;

		//if(facenormal.dotProduct(m_gravitydir) > 0 && endspeed.dotProduct(m_gravitydir) < 0)//this is  a cheat!! I have no time to impelment a robust algo
		//  endspeed *= -1;

		movedata.m_exceededface = true;
		movedata.m_ExceedPosWeights[0] = closepointweights[0];
		movedata.m_ExceedPosWeights[1] = closepointweights[1];
		movedata.m_ExceedPosWeights[2] = closepointweights[2];

		ExtractVectorWeights( facevertex , endspeed , movedata.m_linearvelweights);
		//return true;
	}

	bool  isDownWardFace  = (facenormal.dotProduct(m_gravitydir) > 0 ? true : false);

	float VelGravityAngle = endspeed.normalisedCopy().dotProduct(m_gravitydir);

	if(isDownWardFace && VelGravityAngle < 0)//this is  a cheat!! I have no time to implement a robust algorithm
	{
		movedata.SetStickFaceId(0xFFFFFFFF);
		//	endspeed *= -1;
	}

	return movedata.m_exceededface;
}

//==========================================================================================================
bool OrganBloodMotionSimulator::MoveToNextFace(BloodMoveData & movedata)
{
	int BloodFaceCount = m_collidealgo.GetBloodFaceCount();

	if(movedata.m_stickedfaceid < 0 || movedata.m_stickedfaceid >= BloodFaceCount || (movedata.m_IsBloodMoveable == false))//movedata.m_stickedmesh->temptricount
	   return false;

	//int   laststickedfaceID = movedata.m_stickedfaceid;

	//float minangledot = -FLT_MAX;

	Ogre::Vector3 CurrFaceVertex[3];

	//get previous organ face's vertex position
	m_collidealgo.GetBloodFaceVertexPosition(movedata.m_stickedfaceid , CurrFaceVertex);

	Ogre::Vector3 nextfacenorm;

	//current particle velocity in space
	Ogre::Vector3 currentvel = CurrFaceVertex[0]*movedata.m_linearvelweights[0]+
		CurrFaceVertex[1]*movedata.m_linearvelweights[1]+
		CurrFaceVertex[2]*movedata.m_linearvelweights[2];

	ParticleCollideOrganFaceData NextFace;

	bool NextFaceFinded = m_collidealgo.SelectNextFaceBloodMoveTo(movedata.m_stickedfaceid , 
		movedata.m_ExceedPosWeights,
		currentvel,
		m_gravitydir,
		NextFace);


	if(NextFaceFinded)//find set to move data
	{
		nextfacenorm = NextFace.m_FaceNormal;
		movedata.SetStickFaceId(NextFace.m_FaceID);//m_stickedfaceid = NextFace.m_FaceID;
		movedata.m_positionweights[0] = NextFace.m_collideWeights[0];
		movedata.m_positionweights[1] = NextFace.m_collideWeights[1];
		movedata.m_positionweights[2] = NextFace.m_collideWeights[2];
		currentvel -= nextfacenorm*currentvel.dotProduct(nextfacenorm);

		Ogre::Vector3 nextfacevert[3];
		m_collidealgo.GetBloodFaceVertexPosition(NextFace.m_FaceID , nextfacevert);

		if(nextfacenorm.dotProduct(m_gravitydir) > 0 && currentvel.dotProduct(m_gravitydir) < 0)//this is  a cheat!! I have no time to impelment a robust algo
			currentvel = Ogre::Vector3::ZERO;

		ExtractVectorWeights( nextfacevert, currentvel , movedata.m_linearvelweights);

		movedata.m_currfacenorm = nextfacenorm;
	}

	return NextFaceFinded;
}
//===========================================================================================================
Ogre::Vector3 OrganBloodMotionSimulator::CalculateCurrentPos(BloodMoveData & movedata)
{
	if(movedata.m_stickedfaceid >= 0 && movedata.m_stickedfaceid < m_collidealgo.GetBloodFaceCount() && movedata.m_IsBloodMoveable)
	{
		Ogre::Vector3 vertexPos[3];
		m_collidealgo.GetBloodFaceVertexPosition(movedata.m_stickedfaceid , vertexPos);
		return vertexPos[0]*movedata.m_positionweights[0]+vertexPos[1]*movedata.m_positionweights[1]+vertexPos[2]*movedata.m_positionweights[2];
	}
	else
	{
		//MXASSERT(0 && "error mesh");
		return Ogre::Vector3::ZERO;
	}
}
//===========================================================================================================
bool OrganBloodMotionSimulator::updateParticlePosition(float fixintervalinsecond , int stepCount , BloodMoveData & movedata ,  Ogre::Vector3 & finalresult)
{
	//the particle move to local minimum point of the surface
	//which should drop freely from the surface
	if(movedata.m_IsBloodMoveable == false)
		return false;

	//if particle is in exceed plane choose next face to move out
	if(movedata.m_exceededface)
	{//MXASSERT(movedata.m_stickedfaceid != 0xFFFFFFFF);

		bool hasnextface = MoveToNextFace(movedata);

		if(hasnextface)
			movedata.m_exceededface = false;
		else//change nothing
		{
			finalresult = CalculateCurrentPos(movedata);
			return true;
		}
	}

	int c = 0;
	while(c < stepCount)
	{
		float deltatime = fixintervalinsecond;

		Ogre::Vector3 endspeedinspace;

		float usedtime;

		bool intersected = MoveInCurrentFace(movedata , deltatime , endspeedinspace , usedtime , false);

		finalresult = CalculateCurrentPos(movedata);

		//the particle move to local minimum point of the surface
		//which should drop freely from the surface
		if(movedata.m_IsBloodMoveable == false)//m_stickedfaceid == 0xFFFFFFFF)
			break;

		//for nearly stationary particle
		if(usedtime < FLT_EPSILON && intersected == false)
			break;

		//collide with the face's edge or vertex
		if(intersected == true)
			break;

		deltatime = deltatime-usedtime;
		c++;
	}
	return true;
}

//==================================================================================================================================
DynamicBloodParticle * OrganBloodMotionSimulator::createBloodParticle(MisMedicOrgan_Ordinary * organ,
																	  float weights[3] ,  
																	  float mass , 
																	  int initfaceid, 
																	  float radius,
																	  float BloodSlipAccelrate, float BloodMaxSlipVel,
																	  float fDir
																	  )
{
	DynamicBloodParticle * particle = GenNewParticle(GoPhys::GFPhysVector3(0, 0, 0), mass, radius,  BloodSlipAccelrate,  BloodMaxSlipVel);  //new PhysicsParticle( origin ,   mass ,  radius);
	particle->m_Organ = organ;
	addToActiveList(particle);

	BloodMoveData * parmovedata = (BloodMoveData *)particle->m_usedata;
	parmovedata->m_paritcle = particle;
	parmovedata->m_stickedfaceid = initfaceid;
	parmovedata->m_stickedOrgan = organ;
	parmovedata->m_randtangent = true;

	parmovedata->m_positionweights[0] = weights[0];
	parmovedata->m_positionweights[1] = weights[1];
	parmovedata->m_positionweights[2] = weights[2];

	Ogre::Vector3 facenormal;
	Ogre::Vector3 facevertex[3];
	if(initfaceid >= 0 && initfaceid < m_collidealgo.GetBloodFaceCount())
	{
		m_collidealgo.GetBloodFaceVertexPosition(initfaceid , facevertex);
		Ogre::Vector3 side0 = facevertex[0] - facevertex[1];
		Ogre::Vector3 side1 = facevertex[2] - facevertex[0];
		facenormal = side1.crossProduct(side0);
		Ogre::Vector3 vectorLeft = m_gravitydir.crossProduct(facenormal);
		vectorLeft.normalise();
		parmovedata->m_vectorDir = vectorLeft*fDir+m_gravitydir;
		parmovedata->m_vectorDir.normalise();
	}

	//Ogre::Vector2 textureCoords[3];
	// GetBloodFaceTextureCoordinate(parmovedata->m_stickedfaceid , textureCoords);
	// particle->m_TextureUV = textureCoords[0]*parmovedata->m_positionweights[0] 
	//	+textureCoords[1]*parmovedata->m_positionweights[1] 
	//	+textureCoords[2]*parmovedata->m_positionweights[2];

	for(size_t s = 0 ; s < m_listeners.size() ; s++)
		m_listeners[s]->onParticleCreated(particle);

	return particle;
}
//===================================================================================================================
DynamicBloodParticle * OrganBloodMotionSimulator::allocParticle(const GoPhys::GFPhysVector3 & origin ,  float mass , float radius)
{
	DynamicBloodParticle * particle = new DynamicBloodParticle(Ogre::Vector3(origin.x() , origin.y() , origin.z()) ,   mass ,  radius);
	return particle;
}
//===========================================================================================================
void OrganBloodMotionSimulator::RecycleParticle(DynamicBloodParticle * particle)
{
	particle->m_recyclenext = m_recycledhead;
	m_recycledhead = particle;
}
//===========================================================================================================
DynamicBloodParticle * OrganBloodMotionSimulator::GenNewParticle(const GoPhys::GFPhysVector3 &origin ,  
	                                                             float mass , float radius ,
																 float BloodSlipAccelrate, float BloodMaxSlipVel)
{
	if(m_recycledhead != 0)
	{
		DynamicBloodParticle * temp = m_recycledhead;
		m_recycledhead = (DynamicBloodParticle*)m_recycledhead->m_recyclenext;
		temp->reset();
		((BloodMoveData *)temp->m_usedata)->reset(BloodSlipAccelrate, BloodMaxSlipVel);

		temp->m_position = Ogre::Vector3(origin.x() , origin.y() , origin.z());
		temp->m_radius = radius;

		return temp;
	}
	else
	{
		DynamicBloodParticle * particle = allocParticle(origin ,   mass ,  radius);	
		particle->reset();

		BloodMoveData * parmovedata = new BloodMoveData(BloodSlipAccelrate, BloodMaxSlipVel);
		particle->m_usedata = parmovedata;

		return particle;
	}
}
//===========================================================================================================
void OrganBloodMotionSimulator::addToActiveList(DynamicBloodParticle * movedparticle)
{
	//add to active list
	if(m_activeparticleend != 0)
	{
		m_activeparticleend->m_activednext = movedparticle;
		movedparticle->m_activedprev = m_activeparticleend;
		m_activeparticleend = movedparticle;
	}
	else
	{
		m_activeparticleend = m_activeparticlehead = movedparticle;
	}
	m_activeparticlecount++;
}
//===========================================================================================================
void OrganBloodMotionSimulator::onRemoveParticle(DynamicBloodParticle * particle)
{

}
//===========================================================================================================
void OrganBloodMotionSimulator::removeDeadParticles(int maxparnum)
{
	DynamicBloodParticle * movedparticle = m_activeparticlehead;

	while(movedparticle != 0)
	{
		DynamicBloodParticle * nextparticle = (DynamicBloodParticle*)movedparticle->m_activednext;

		bool recycle = false;

		BloodMoveData & partdata = *((BloodMoveData*)movedparticle->m_usedata);

		//recycle particle which quantity is small then stop quantity or which move to 
		//the local minimum of the surface
		if(movedparticle->m_CurrQuanity <= movedparticle->m_StopQuanity || partdata.m_IsBloodMoveable == false) //m_stickedfaceid == 0xFFFFFFFF)//recycle this 
		   recycle = true;

		else if(m_collidealgo.IsFaceRemoved(partdata.m_stickedfaceid))
		   recycle = true;

		if(recycle == true)
		{
			if(movedparticle == m_activeparticlehead)
				m_activeparticlehead = (DynamicBloodParticle*)movedparticle->m_activednext;
			else
				movedparticle->m_activedprev->m_activednext = movedparticle->m_activednext;

			if(movedparticle == m_activeparticleend)
				m_activeparticleend = (DynamicBloodParticle*)movedparticle->m_activedprev;
			else
				movedparticle->m_activednext->m_activedprev = movedparticle->m_activedprev;

			for(size_t c = 0 ; c < m_listeners.size() ; c++)
				m_listeners[c]->onParticleRemoved(movedparticle);

			//for(size_t s = 0 ; s < m_OrganSurfaceBloodStreams.size() ; s++)
			//m_OrganSurfaceBloodStreams[s]->OnBloodParticleRemoved(movedparticle);

			onRemoveParticle(movedparticle);

			RecycleParticle(movedparticle);

			m_activeparticlecount--;

		}

		movedparticle = nextparticle;
	}

	//MXASSERT(m_activeparticlecount >=0);
}
void OrganBloodMotionSimulator::DestoryAllParticles()
{
	while(m_activeparticlehead != 0)
	{
		DynamicBloodParticle * delparticle =  m_activeparticlehead;

		for(size_t s = 0 ; s < m_listeners.size() ; s++)
			m_listeners[s]->onParticleRemoved(delparticle);

		m_activeparticlehead = (DynamicBloodParticle*)m_activeparticlehead->m_activednext;

		BloodMoveData * data = (BloodMoveData*)delparticle->m_usedata;

		delete data;
		delete delparticle;
	}

	DynamicBloodParticle * delparticle = m_recycledhead;
	while(delparticle != 0)
	{
		DynamicBloodParticle * nextparticle = (DynamicBloodParticle*)delparticle->m_recyclenext;
		delete delparticle;
		delparticle = nextparticle;
	}
	m_recycledhead = 0;
}
//===============================================================================================
void OrganBloodMotionSimulator::AddListener(OrganBloodMotionSimulator::Listener * listener)
{
	for(size_t s = 0 ; s < m_listeners.size() ; s++)
	{
		if(m_listeners[s] == listener)
			return;
	}
	m_listeners.push_back(listener);
}
//===============================================================================================
void OrganBloodMotionSimulator::RemoveListener(OrganBloodMotionSimulator::Listener * listener)
{
	for(size_t s = 0 ; s < m_listeners.size() ; s++)
	{
		if(m_listeners[s] == listener)
		{
			m_listeners.erase(m_listeners.begin()+s);
			return;
		}
	}
}
//===============================================================================================
bool OrganBloodMotionSimulator::updateParticles(float dt)
{
	if(m_HostObject == 0)
		return false;

	const float fixedTimeStep = 1.0f / 30.0f;

	m_AccmulateTime += dt;

	int stepcount = (int)(m_AccmulateTime / fixedTimeStep); 

	if(stepcount <= 0)
		return false;

	if(stepcount > 3)
	{
		stepcount = 3;
		m_AccmulateTime = 0;
	}
	else
	{
		m_AccmulateTime = m_AccmulateTime-fixedTimeStep*stepcount;
	}

	DynamicBloodParticle * particle = m_activeparticlehead;

	int BloodFaceCount = m_collidealgo.GetBloodFaceCount();

	while(particle)
	{
		//first update barycentric coordinate to space coordinate
		BloodMoveData & partdata = *((BloodMoveData*)particle->m_usedata);

		int stickfaceid = partdata.m_stickedfaceid;

		if(stickfaceid >=0 && stickfaceid < BloodFaceCount)
		{
			Ogre::Vector3 facevert[3];

			m_collidealgo.GetBloodFaceVertexPosition(stickfaceid , facevert);

			Ogre::Vector3 pos = partdata.m_positionweights[0]*facevert[0]
			+partdata.m_positionweights[1]*facevert[1]
			+partdata.m_positionweights[2]*facevert[2];

			partdata.m_paritcle->m_position = pos;

			partdata.m_paritcle->m_timelived += dt;
		}
		particle = (DynamicBloodParticle*)particle->m_activednext;
	}

	particle = m_activeparticlehead;

	while(particle)
	{
		Ogre::Vector3 particlepos;

		BloodMoveData & partdata = *((BloodMoveData*)particle->m_usedata);

		bool particleMoved = updateParticlePosition(fixedTimeStep , stepcount , partdata , particlepos);

		if(particleMoved)
		{
			particle->m_position = particlepos;
			particle->m_CurrQuanity -= particle->m_QuantityDecPermove*stepcount;
		}
		//
		particle = (DynamicBloodParticle*)particle->m_activednext;
	}

	//remove blood particle whose quantity is small than stop quantity
	removeDeadParticles(-1);

	//for(int s = 0 ; s < (int)m_OrganSurfaceBloodStreams.size() ; s++)
	//{
	//m_OrganSurfaceBloodStreams[s]->Update(dt);
	//}

	if(m_activeparticlehead == 0)// && m_OrganSurfaceBloodStreams.size() == 0)
		return false;
	else
		return true;
}
