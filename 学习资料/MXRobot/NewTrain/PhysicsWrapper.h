#ifndef _PHYSICWRAPPER_
#define _PHYSICWRAPPER_

#include "collision/GoPhysCollisionlib.h"
#include "Dynamic/GoPhysDynamicLib.h"
#include "collision/NarrowPhase/GoPhysPrimitiveTest.h"
#include "collision/CollisionShapes/GoPhysCompoundShape.h"
#include "Ogre.h"
//#include "BasicPrimitive.h"
using namespace GoPhys;
#define COMBCONVEXSHAPSUPPORT 1
class PhysicsWrapper
{
public:
		static PhysicsWrapper & GetSingleTon();

		void InitializePhysicsWorld(int solveriteratornum = 7, 
									int solverthreadnum = 2,
                                    int strainiteratornum = 0);

		void DestoryPhysicsWorld();

		int  UpdateWorld(float dt);

		void Terminate();

		
		//create collision shape
		GFPhysCompoundShape * CreateCompoundCollisionShape();

		GFPhysCollideShape * CreateBoxCollideShape(Ogre::Vector3 extends , 
												   bool convexhull = false);

        GFPhysCollideShape * CreateCynlinderCollideShape(Ogre::Vector3 pointA, Ogre::Vector3 pointB, Real radius);
        
		GFPhysCollideShape * CreateConvexCollidesShape(const std::vector<Ogre::Vector3> & ConvexVertex,
			                                           const std::vector<int> & FaceVertIndex,
			                                           const std::vector<int> & FaceVertNum
													   );

		void DestoryCollisionShape(GFPhysCollideShape * cdshape);


		GFPhysRigidBody * CreateSphere(Ogre::Vector3 center, float radius, float mass);

		GFPhysRigidBody * CreateDynamicBox(Ogre::Vector3 center , Ogre::Vector3 extends , float mass, bool convexhull = false);
	
		GFPhysRigidBody * CreateDynamicCynlinder(Ogre::Vector3 pointA , Ogre::Vector3 pointB , float radius , float mass);

		GFPhysRigidBody * CreateRigidBody(Real Mass,GFPhysCollideShape* Shape,const GFPhysVector3 & localInertia,const GFPhysTransform & Transform);

		void DestoryRigidBody(GFPhysRigidBody * rb);

		void SetSimulationFrequency(float freq);
	
		void SetActive(bool set);

		void ResetAllSoftBodyToOrigin();

		GFPhysDiscreteDynamicsWorld*		m_dynamicsWorld;

		GFPhysBPhaseInterface*	m_overlappingPairCache;
		GFPhysCollisionDispatch * m_CollisionDispatch;
		GFPhysConstraintSolver*	m_solver;
		GFPhysDefaultCollisionConfig * m_CollisionConfig;
		float m_SimulateFrequency;
		
protected:
		static PhysicsWrapper s_physwrapper;
		PhysicsWrapper();

		bool  m_Initialized;
		bool  m_Active;
		//std::vector<BasicPrimitive*> m_primtives;
		void CreateStaticBox(Ogre::Vector3 center , Ogre::Vector3 extends);

};
#endif