#include "collision/CollisionShapes/GoPhysCylinderShape.h"
#include "collision/CollisionShapes/GoPhysCapsuleshape.h"
#include "collision/CollisionShapes/GoPhysTriangleShape.h"
#include  "collision/CollisionShapes/GoPhysTriangleMesh.h"
#include "collision/CollisionShapes/GoPhysBVHTriMeshShape.h"
#include "collision/CollisionDispatch/GoPhysConvexConcaveCollision.h"
#include "collision/GoPhysWorldCaster.h"
#include "collision/BroadPhase/GoPhysCollisionSpaceHash.h"
#include "Dynamic/fluids/GoPhysSphFluid.h"
#include "Dynamic/fluids/GoPhysSphFluidsStream.h"
#include "Dynamic/Solver/GoPhysParallelSolver.h"
#include "Math/GoPhysMatrixDecompose.h"
#include "GoPhysGlobalConfig.h"
#include "physicsWrapper.h"
#include <windows.h>
const int maxProxies = 32766;
using namespace GoPhys;

PhysicsWrapper PhysicsWrapper::s_physwrapper;

PhysicsWrapper & PhysicsWrapper::GetSingleTon()
{
		return s_physwrapper;
}
PhysicsWrapper::PhysicsWrapper()
{
	m_dynamicsWorld = 0;
	m_Initialized = false;
	m_SimulateFrequency = 80.0f;
	m_Active = true;
}

void PhysicsWrapper::SetSimulationFrequency(float freq)
{
	m_SimulateFrequency = freq;
}

void PhysicsWrapper::SetActive(bool set)
{
	m_Active = set;
}
void PhysicsWrapper::Terminate()
{
	if(m_Initialized == true)
	{
	   DestoryPhysicsWorld();
	   GFPhysGlobalConfig::GetGlobalConfig().Terminate();
	   m_Initialized = false;
	}
}

void PhysicsWrapper::ResetAllSoftBodyToOrigin()
{
	
}
void PhysicsWrapper::DestoryPhysicsWorld()
{
	if(m_dynamicsWorld)
	{
	   delete m_dynamicsWorld;
	   m_dynamicsWorld = 0;
	}

	if(m_solver)
	{
		delete m_solver;
		m_solver = 0;
	}

	if(m_overlappingPairCache)
	{
		delete m_overlappingPairCache;
		m_overlappingPairCache = 0;
	}

	if(m_CollisionDispatch)
	{
		delete m_CollisionDispatch;
		m_CollisionDispatch = 0;
	}

	if(m_CollisionConfig)
	{
		delete m_CollisionConfig;
		m_CollisionConfig = 0;
	}
}

DWORD GetProcessorCoreCount()
{
#if (_WIN32_WINNT < 0x0600) // [zyl910] 低版本的Windows SDK没有定义 RelationProcessorPackage 等常量
	#define RelationProcessorPackage 3
	#define RelationGroup 4
#endif
	typedef BOOL (WINAPI *LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION , PDWORD);
	
	LPFN_GLPI glpi = (LPFN_GLPI) GetProcAddress(GetModuleHandle(TEXT("kernel32")), "GetLogicalProcessorInformation");
	
	if (NULL == glpi)
		return 0;
	
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
	
	DWORD returnLength = 0;
	
	DWORD processorCoreCount = 0;
	
	while (true)
	{
		DWORD rc = glpi(buffer, &returnLength);
		if (FALSE == rc)
		{
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				if (buffer)
					free(buffer);
				buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(
					returnLength);
				if (NULL == buffer)
					return 0;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			break;
		}
	}
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = buffer;
	DWORD byteOffset = 0;
	while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength)
	{
		switch (ptr->Relationship)
		{
		case RelationProcessorCore:
			++processorCoreCount;
			break;
		default:
			break;
		}
		byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
		++ptr;
	}
	free(buffer);
	return processorCoreCount;
}
void PhysicsWrapper::InitializePhysicsWorld(int solveriteratornum, 
											int solverthreadnum,
                                            int strainiteratornum)
{
		//std::ifstream file( "lic.lics" );
		//std::string strLics;
		//file >> strLics;
		//file.close();

	    FILE * fp;
	    fopen_s(&fp, "SYLicense.lis", "r");
		if (NULL == fp)
		{
			MessageBox(NULL, TEXT("no license file！"), TEXT("错误"), MB_OK);
			return;
		}

	    fseek(fp, 0L, SEEK_END);
	    int size = ftell(fp);
	  
	    char * newstr = new char[size + 1];
	    fseek(fp, 0L, SEEK_SET);
	    fread(newstr, size, 1, fp);
	    newstr[size] = 0;
	    fclose(fp);
	    std::string strLics = newstr;
		delete[]newstr;

		SYSTEM_INFO si;  		GetSystemInfo(&si);  
		int cpucorecount = GetProcessorCoreCount();//si.dwNumberOfProcessors;  //

		bool initilized = GFPhysGlobalConfig::GetGlobalConfig().Initialize( (char*)strLics.c_str() );
		if (!initilized)
		{
			MessageBox(NULL,TEXT("非法的使用权限！"),TEXT("错误"),MB_OK);
			exit(1);
		}

		GFPhysGlobalConfig::GetGlobalConfig().enableMultiThreads(cpucorecount);
		GFPhysGlobalConfig::GetGlobalConfig().m_SoftSolverItorNum = solveriteratornum;//7;
        //GFPhysGlobalConfig::GetGlobalConfig().m_SoftSolverStrainItorNum = strainiteratornum;//1;


		if(solverthreadnum == 0)//this means disable multithreadsolver
		   GFPhysGlobalConfig::GetGlobalConfig().m_ParallelPosSolverThreadCount = 0;
		else
		   GFPhysGlobalConfig::GetGlobalConfig().m_ParallelPosSolverThreadCount = (solverthreadnum > cpucorecount-2 ? solverthreadnum : cpucorecount-2) ;//solverthreadnum;//2;
		
		GFPhysGlobalConfig::GetGlobalConfig().m_useMT_SoftRigidCollide = true;
		GFPhysGlobalConfig::GetGlobalConfig().m_useOBBFilterInRSCollision = true;
		GFPhysGlobalConfig::GetGlobalConfig().m_UseMT_UpdateTree = true;
		GFPhysGlobalConfig::GetGlobalConfig().m_UseNewSRContact = true;
		GFPhysGlobalConfig::GetGlobalConfig().m_UseMergedSoftRigidSolver = true;
		//GFPhysGlobalConfig::GetGlobalConfig().m_UseMemOptimizedParallelSolver = true;
		///collision configuration contains default setup for memory, collision setup
		m_CollisionConfig = new GFPhysDefaultCollisionConfig();
		//m_CollisionConfig->DestoryCreateFunc(SOFTBODY_SHAPE_PROXYTYPE , SOFTBODY_SHAPE_PROXYTYPE);//this will force use collision hash

		m_CollisionDispatch = new GFPhysCollisionDispatch(m_CollisionConfig);

		///the maximum size of the collision world. Make sure objects stay within these boundaries
		///Don't make the world AABB size too large, it will harm simulation quality and performance
		GFPhysVector3 worldAabbMin(-1000,-1000,-1000);

		GFPhysVector3 worldAabbMax(1000,1000,1000);

		m_overlappingPairCache = new GFPhysSAP(worldAabbMin,worldAabbMax,1000);

		m_solver = new GFPhysQuickStepConstraintSolver;//sol;

		m_dynamicsWorld = new GFPhysDiscreteDynamicsWorld(m_CollisionDispatch,m_overlappingPairCache,m_solver,NULL,m_CollisionConfig);

		m_dynamicsWorld->SetGravity(GFPhysVector3(0 , 3 , 0));

		m_dynamicsWorld->GetSolverParameter().m_PositionCorrect = false;               

		m_dynamicsWorld->SoftCollisionHashGridSize(0.8f);

		//m_dynamicsWorld->GetParallelPositionContraintSolver()->m_UseEnhanceRSCollide = false;

		m_Initialized = true;
}

int PhysicsWrapper::UpdateWorld(float dt)
{
	if(m_Active)
	{
	   if(m_Initialized == false)
			return 0;

	   int stepcount = m_dynamicsWorld->StepSimulation(dt , 1 , /*1.0f / 80.0f*/1.0f / m_SimulateFrequency);

	   //for(size_t i = 0 ; i < m_primtives.size() ; i++)
	   //{
			//BasicPrimitive * pm = m_primtives[i];
			//pm->Update(dt);
	  // }

	   return stepcount;
	}
	else
	   return 0;

}

void PhysicsWrapper::CreateStaticBox(Ogre::Vector3 center , Ogre::Vector3 extends)
{
	if(m_Initialized == false)
		return;

	GFPhysCollideShape * groundShape = new GFPhysBoxShape(GFPhysVector3(extends.x , extends.y,extends.z));
	GFPhysTransform groundTransform;
	groundTransform.SetIdentity();
	groundTransform.SetOrigin(GFPhysVector3(center.x,center.y,center.z));

	float mass(0.);

	bool isDynamic = (mass != 0.f);

	GFPhysVector3 localInertia(0,0,0);
	if (isDynamic)
		groundShape->CalculateLocalInertia(mass,localInertia);

	CreateRigidBody(mass,groundShape , localInertia , groundTransform);
}

GFPhysCompoundShape * PhysicsWrapper::CreateCompoundCollisionShape()
{
	if(m_Initialized == false)
		return 0;

	GFPhysCompoundShape * CombinedConvexShape = 0;

	CombinedConvexShape = new GFPhysCompoundShape();
	CombinedConvexShape->SetMargin(0);

	return CombinedConvexShape;//CreateRigidBody(mass , CombinedConvexShape , localInertia , rigidTransform);
}
GFPhysCollideShape * PhysicsWrapper::CreateCynlinderCollideShape(Ogre::Vector3 pointA, Ogre::Vector3 pointB, Real radius)
{
    if (m_Initialized == false)
        return 0;

    GFPhysCollideShape * groundShape = 0;

    float HalfLen = (pointA - pointB).length()*0.5f;

    Ogre::Vector3 cynlindercenter = (pointA + pointB)*0.5f;

    Ogre::Vector3 cynlinderextends = Ogre::Vector3(radius, radius, HalfLen);

    groundShape = new GFPhysCylinderShapeZ(GFPhysVector3(radius, radius, HalfLen));   

    groundShape->SetMargin(0);

    return groundShape;
}
GFPhysCollideShape * PhysicsWrapper::CreateBoxCollideShape(Ogre::Vector3 extends , bool convexhull)
{
	if(m_Initialized == false)
		return 0;

	GFPhysCollideShape * collisionShape = 0;

	if(convexhull == false)
	{
		collisionShape = new GFPhysBoxShape(GFPhysVector3(extends.x, extends.y, extends.z));
		collisionShape->SetMargin(0);
	}
	else
	{
		GFPhysPolyhedraVert hullVerts[8];
		hullVerts[0] = GFPhysVector3(-extends.x , -extends.y , -extends.z);
		hullVerts[1] = GFPhysVector3(-extends.x ,  extends.y , -extends.z);
		hullVerts[2] = GFPhysVector3(extends.x  ,  extends.y,  -extends.z);
		hullVerts[3] = GFPhysVector3(extends.x  , -extends.y,  -extends.z);

		hullVerts[4] = GFPhysVector3(-extends.x , -extends.y, extends.z);
		hullVerts[5] = GFPhysVector3(-extends.x ,  extends.y, extends.z);
		hullVerts[6] = GFPhysVector3(extends.x  ,  extends.y, extends.z);
		hullVerts[7] = GFPhysVector3(extends.x  , -extends.y, extends.z);


		GFPhysPolyhedraFace hullfaces[6];
		//bottom
		hullfaces[0].m_VertsIndex[0] = 0;
		hullfaces[0].m_VertsIndex[1] = 3;
		hullfaces[0].m_VertsIndex[2] = 2;
		hullfaces[0].m_VertsIndex[3] = 1;

		//top
		hullfaces[1].m_VertsIndex[0] = 4;
		hullfaces[1].m_VertsIndex[1] = 5;
		hullfaces[1].m_VertsIndex[2] = 6;
		hullfaces[1].m_VertsIndex[3] = 7;

		//front
		hullfaces[2].m_VertsIndex[0] = 7;
		hullfaces[2].m_VertsIndex[1] = 6;
		hullfaces[2].m_VertsIndex[2] = 2;
		hullfaces[2].m_VertsIndex[3] = 3;

		//back
		hullfaces[3].m_VertsIndex[0] = 4;
		hullfaces[3].m_VertsIndex[1] = 0;
		hullfaces[3].m_VertsIndex[2] = 1;
		hullfaces[3].m_VertsIndex[3] = 5;

		//left
		hullfaces[4].m_VertsIndex[0] = 7;
		hullfaces[4].m_VertsIndex[1] = 3;
		hullfaces[4].m_VertsIndex[2] = 0;
		hullfaces[4].m_VertsIndex[3] = 4;

		//right
		hullfaces[5].m_VertsIndex[0] = 6;
		hullfaces[5].m_VertsIndex[1] = 5;
		hullfaces[5].m_VertsIndex[2] = 1;
		hullfaces[5].m_VertsIndex[3] = 2;

		for(int i = 0 ; i < 6 ; i++)
		{
			int vid0 = hullfaces[i].m_VertsIndex[0];
			int vid1 = hullfaces[i].m_VertsIndex[1];
			int vid2 = hullfaces[i].m_VertsIndex[2];

			hullfaces[i].m_Normal = (hullVerts[vid1]-hullVerts[vid0]).Cross(hullVerts[vid2]-hullVerts[vid0]);
			hullfaces[i].m_Normal.Normalize();
			hullfaces[i].m_NumVerts = 4;
			hullfaces[i].m_Dist = hullVerts[vid0].Dot(hullfaces[i].m_Normal);
		}

		GFPhysConvexHullShape * convex = new GFPhysConvexHullShape(hullVerts, 8, hullfaces, 6);
		convex->SetMargin(0);
		convex->RecalcLocalAabb();
		collisionShape = convex;
	}

	return collisionShape;
}
//===========================================================================================================================================
GFPhysCollideShape * PhysicsWrapper::CreateConvexCollidesShape(const std::vector<Ogre::Vector3> & ConvexVertex,
															   const std::vector<int> & FaceVertIndex,
															   const std::vector<int> & FacesVertNum
															   )
{
	if(m_Initialized == false)
	   return 0;

	GFPhysPolyhedraVert * hullVerts = new GFPhysPolyhedraVert[ConvexVertex.size()];

	for(size_t v = 0 ; v < ConvexVertex.size() ; v++)
	{
		Ogre::Vector3 position  = ConvexVertex[v];
		hullVerts[v] = GFPhysVector3(position.x , position.y , position.z);
	}

	GFPhysPolyhedraFace * hullfaces = new GFPhysPolyhedraFace[FacesVertNum.size()];
	int c = 0;
	for(size_t f = 0 ; f < FacesVertNum.size() ; f++)
	{
		for(int v = 0 ; v < FacesVertNum[f] ; v++)
		{
			hullfaces[f].m_VertsIndex[v] = FaceVertIndex[c];
			c++;
		}

		//faces normal
		int vid0 = hullfaces[f].m_VertsIndex[0];
		int vid1 = hullfaces[f].m_VertsIndex[1];
		int vid2 = hullfaces[f].m_VertsIndex[2];

		hullfaces[f].m_Normal = (hullVerts[vid1]-hullVerts[vid0]).Cross(hullVerts[vid2]-hullVerts[vid0]);
		hullfaces[f].m_Normal.Normalize();
		hullfaces[f].m_NumVerts = FacesVertNum[f];
		hullfaces[f].m_Dist  = hullVerts[vid0].Dot(hullfaces[f].m_Normal);
	}

	GFPhysConvexHullShape * convexHullShape = new GFPhysConvexHullShape(hullVerts , ConvexVertex.size() , hullfaces , FacesVertNum.size() );
	convexHullShape->SetMargin(0);
	convexHullShape->RecalcLocalAabb();

	delete []hullVerts;
	delete []hullfaces;

	return convexHullShape;
}
//===========================================================================================================================================
void PhysicsWrapper::DestoryCollisionShape(GFPhysCollideShape * cdshape)
{
	delete cdshape;
}
//=======================================================================================================================================
GFPhysRigidBody  * PhysicsWrapper::CreateSphere(Ogre::Vector3 center, float radius, float mass)
{
	if (m_Initialized == false)
		return 0;

	GFPhysCollideShape * sphereShape = new GFPhysSphereShape(radius);// (GFPhysVector3(extends.x, extends.y, extends.z));
	//sphereShape->SetMargin(0);
	
	GFPhysTransform sphereTransform;
	sphereTransform.SetIdentity();
	sphereTransform.SetOrigin(GFPhysVector3(center.x, center.y, center.z));

	bool isDynamic = (mass != 0.f);

	GFPhysVector3 localInertia(0, 0, 0);
	if (isDynamic)
		sphereShape->CalculateLocalInertia(mass, localInertia);

	return CreateRigidBody(mass, sphereShape, localInertia, sphereTransform);
}
//===========================================================================================================================================
GFPhysRigidBody  * PhysicsWrapper::CreateDynamicBox(Ogre::Vector3 center , Ogre::Vector3 extends , float mass, bool convexhull)
{
	if(m_Initialized == false)
		return 0;

	GFPhysCollideShape * groundShape = 0;

	if(convexhull == false)
	{
		groundShape = new GFPhysBoxShape(GFPhysVector3(extends.x , extends.y,extends.z));
	}
	else
	{
		GFPhysPolyhedraVert hullVerts[8];
		hullVerts[0] = GFPhysVector3(-extends.x , -extends.y , -extends.z);
		hullVerts[1] = GFPhysVector3(-extends.x ,  extends.y , -extends.z);
		hullVerts[2] = GFPhysVector3(extends.x  ,  extends.y,  -extends.z);
		hullVerts[3] = GFPhysVector3(extends.x  , -extends.y,  -extends.z);

		hullVerts[4] = GFPhysVector3(-extends.x , -extends.y, extends.z);
		hullVerts[5] = GFPhysVector3(-extends.x ,  extends.y, extends.z);
		hullVerts[6] = GFPhysVector3(extends.x  ,  extends.y, extends.z);
		hullVerts[7] = GFPhysVector3(extends.x  , -extends.y, extends.z);


		GFPhysPolyhedraFace hullfaces[6];
		//bottom
		hullfaces[0].m_VertsIndex[0] = 0;
		hullfaces[0].m_VertsIndex[1] = 3;
		hullfaces[0].m_VertsIndex[2] = 2;
		hullfaces[0].m_VertsIndex[3] = 1;

		//top
		hullfaces[1].m_VertsIndex[0] = 4;
		hullfaces[1].m_VertsIndex[1] = 5;
		hullfaces[1].m_VertsIndex[2] = 6;
		hullfaces[1].m_VertsIndex[3] = 7;

		//front
		hullfaces[2].m_VertsIndex[0] = 7;
		hullfaces[2].m_VertsIndex[1] = 6;
		hullfaces[2].m_VertsIndex[2] = 2;
		hullfaces[2].m_VertsIndex[3] = 3;

		//back
		hullfaces[3].m_VertsIndex[0] = 4;
		hullfaces[3].m_VertsIndex[1] = 0;
		hullfaces[3].m_VertsIndex[2] = 1;
		hullfaces[3].m_VertsIndex[3] = 5;

		//left
		hullfaces[4].m_VertsIndex[0] = 7;
		hullfaces[4].m_VertsIndex[1] = 3;
		hullfaces[4].m_VertsIndex[2] = 0;
		hullfaces[4].m_VertsIndex[3] = 4;

		//right
		hullfaces[5].m_VertsIndex[0] = 6;
		hullfaces[5].m_VertsIndex[1] = 5;
		hullfaces[5].m_VertsIndex[2] = 1;
		hullfaces[5].m_VertsIndex[3] = 2;

		for(int i = 0 ; i < 6 ; i++)
		{
			int vid0 = hullfaces[i].m_VertsIndex[0];
			int vid1 = hullfaces[i].m_VertsIndex[1];
			int vid2 = hullfaces[i].m_VertsIndex[2];

			hullfaces[i].m_Normal = (hullVerts[vid1]-hullVerts[vid0]).Cross(hullVerts[vid2]-hullVerts[vid0]);
			hullfaces[i].m_Normal.Normalize();
			hullfaces[i].m_NumVerts = 4;
			hullfaces[i].m_Dist = hullVerts[vid0].Dot(hullfaces[i].m_Normal);
		}

		groundShape = new GFPhysConvexHullShape(hullVerts , 8  , hullfaces , 6 );
	}

	groundShape->SetMargin(0);
	GFPhysTransform groundTransform;
	groundTransform.SetIdentity();
	groundTransform.SetOrigin(GFPhysVector3(center.x,center.y,center.z));

		bool isDynamic = (mass != 0.f);

			GFPhysVector3 localInertia(0,0,0);
			if (isDynamic)
				groundShape->CalculateLocalInertia(mass,localInertia);

			return CreateRigidBody(mass,groundShape , localInertia , groundTransform);
}

GFPhysRigidBody * PhysicsWrapper::CreateDynamicCynlinder(Ogre::Vector3 pointA , Ogre::Vector3 pointB , float radius , float mass)
{
	if(m_Initialized == false)
	    return 0;
	
	float HalfLen = (pointA-pointB).length()*0.5f;

	Ogre::Vector3 center = (pointA+pointB)*0.5f;

	GFPhysCylinderShapeZ * cylinderZ = new GFPhysCylinderShapeZ(GFPhysVector3(radius , radius , HalfLen));
	cylinderZ->SetMargin(0.04);
	
	Ogre::Vector3 Axis = pointA-pointB;

	Ogre::Quaternion quat0 = Ogre::Vector3::UNIT_Z.getRotationTo(Axis.normalisedCopy());
	
	Ogre::Quaternion quat1 = Ogre::Vector3::UNIT_Z.getRotationTo(-Axis.normalisedCopy());

	Ogre::Quaternion nearRotate = (fabsf(quat0.w) < fabsf(quat1.w) ? quat0 : quat1);

	GFPhysTransform rigidTransform;
	rigidTransform.SetOrigin(GFPhysVector3(center.x , center.y , center.z));
	rigidTransform.SetRotation(GFPhysQuaternion(nearRotate.x , nearRotate.y , nearRotate.z , nearRotate.w));
	
	GFPhysVector3 localInertia(0,0,0);
	
	bool isDynamic = (mass != 0.f);

	if (isDynamic)
		cylinderZ->CalculateLocalInertia(mass,localInertia);
	
	return CreateRigidBody(0 , cylinderZ , localInertia , rigidTransform);

	/*
	GFPhysCollideShape *cynlinderShape = new GFPhysCapsuleShapeZ(extends.x , extends.z*2);//GFPhysCylinderShapeZ(GFPhysVector3(extends.x , extends.x , extends.z));
	cynlinderShape->SetMargin(0.02);
			
	GFPhysTransform groundTransform;
	groundTransform.SetIdentity();
	groundTransform.SetOrigin(GFPhysVector3(center.x,center.y,center.z));

	bool isDynamic = (mass != 0.f);

	GFPhysVector3 localInertia(0,0,0);
	if (isDynamic)
		cynlinderShape->CalculateLocalInertia(mass,localInertia);

	return CreateRigidBody(mass , cynlinderShape , localInertia , groundTransform);
	*/
}
GFPhysRigidBody * PhysicsWrapper::CreateRigidBody(Real Mass,GFPhysCollideShape* Shape,const GFPhysVector3 & localInertia,const GFPhysTransform & Transform)
{
	if(m_Initialized == false)
		return 0;
	
	GFPhysRigidBody::GFPhysRigidBodyConstructInfo rbInfo(Mass,NULL,Shape,localInertia);
	rbInfo.m_startWorldTransform = Transform;
	rbInfo.m_friction = 1.0f;
	GFPhysRigidBody* body = new GFPhysRigidBody(rbInfo);
	//add to the dynamics world
	m_dynamicsWorld->AddRigidBody(body);
	return body;
}
void PhysicsWrapper::DestoryRigidBody(GFPhysRigidBody * rb)
{
	if(rb)
	{
	   if(m_dynamicsWorld)
	      m_dynamicsWorld->RemoveRigidBody(rb);

	   /*
	   GFPhysCollideShape * cdshape = rb->GetCollisionShape();
	   if(cdshape)
	   {
		   GFPhysCompoundShape * compoundshape = dynamic_cast<GFPhysCompoundShape * >(cdshape);
		   if(compoundshape)
		   {
			   for(int c = 0 ; c < compoundshape->GetNumComponent() ; c++)
			   {
					GFPhysCollideShape * subshape = compoundshape->GetComponentShape(c);
					if(subshape)
					{
						delete subshape;
					}
			   }
		   }
		   delete cdshape;
	   }*/
	   delete rb;
	   
	}
}