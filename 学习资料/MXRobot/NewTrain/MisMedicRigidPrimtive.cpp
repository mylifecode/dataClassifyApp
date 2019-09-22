#include "MisMedicRigidPrimtive.h"
#include "collision/CollisionShapes/GoPhysBVHTriMeshShape.h"
#include "PhysicsWrapper.h"
#include "MisMedicObjectSerializer.h"
#include "TrainingMgr.h"
#include "MisNewTraining.h"
#include "MisMedicOrganInterface.h"
#include "MisMedicOrganOrdinary.h"
#include "MXOgreGraphic.h"
#include "OgreManualObject.h"
#include "ToolSpenetrateMgr.h"
MisMedicRigidPrimtive::MisMedicRigidPrimtive(int index , CBasicTraining * ownertrain)
	: MisMedicOrganInterface(DOT_RIGIDBODY, EDOT_NO_TYPE, index, ownertrain),
	m_body(0),
	m_node(0),
	m_entity(0),
    m_RendObject(0),
    m_serializer(0),
	m_MeshVertex(0),
	m_MeshLocalVertex(0),
	m_MeshTriangles(0),
	m_MeshData(0),
	m_RigidType(RT_UNKNOW)
{
	
}
MisMedicRigidPrimtive::~MisMedicRigidPrimtive()
{
	ToolSpenetrateMgr::GetInstance()->RemoveStaticObject(this);
    delete m_serializer;
    m_serializer = 0;
}

void MisMedicRigidPrimtive::SetMass(float mass)
{
	GFPhysVector3 vec = GFPhysVector3(0, 0, 0);
	m_body->SetMassProps(mass, vec);
}

void MisMedicRigidPrimtive::SetEntityMaterial(Ogre::String materialName)
{
	if (m_entity)
	{
		m_entity->setMaterialName(materialName);
	}

}

void MisMedicRigidPrimtive::SetEntityPos(Ogre::Vector3 pos)
{
	if (m_node)
	{
		m_node->setPosition(pos);
	}
}

void MisMedicRigidPrimtive::SetEntityRot(Ogre::Quaternion ori)
{
	if (m_node)
	{
		m_node->setOrientation(ori);
	}
}

void MisMedicRigidPrimtive::SetEntitySize(Ogre::Vector3 size)
{
	if (m_node)
	{
		if (size.x <= 0 || size.y <= 0 ||size.z <= 0 )
		{
			m_node->scale(Ogre::Vector3(1, 1, 1));
			return;
		}
			m_node->scale(size);
	}
}

void updateMesh(const Ogre::MeshPtr mesh, Ogre::Vector3 offset)  
{  
	bool added_shared = false;   
	size_t current_offset = 0;   
	size_t shared_offset = 0;   
	size_t next_offset = 0;   
	// foreach SubMesh   
	for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)   
	{   
		Ogre::SubMesh* submesh = mesh->getSubMesh(i);   
		Ogre::VertexData* vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;   
		if ((!submesh->useSharedVertices) || (submesh->useSharedVertices && !added_shared))   
		{   
			if (submesh->useSharedVertices)   
			{   
				added_shared = true;   
				shared_offset = current_offset;   
			}   
			// the real vertex data is wrapped into lots of classes   
			const Ogre::VertexElement* posElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);   
			Ogre::HardwareVertexBufferSharedPtr vbuf = vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());   
			unsigned char* vertex = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));   
			float* pReal;   
			for (size_t j = 0; j < vertex_data->vertexCount; j++, vertex += vbuf->getVertexSize())   
			{   
				// get vertex data   
				posElem->baseVertexPointerToElement(vertex, &pReal);   
				// modify vertex data   
				pReal[0] += offset.x;   
				pReal[1] += offset.y;   
				pReal[2] += offset.z;   
			}   
			vbuf->unlock();   
			next_offset += vertex_data->vertexCount;   
		}   
	}  
}

//============================================================================================
//
void MisMedicRigidPrimtive::CreateRigidBox(Ogre::String entityName, Ogre::SceneManager * scenemgr, Ogre::Vector3 center, Ogre::Vector3 size, float mass)
{
	Ogre::SceneNode * cubenode = scenemgr->getRootSceneNode()->createChildSceneNode(entityName + "Node");
	if (m_CreateInfo.m_s3mfilename.empty())
	{
		m_entity = scenemgr->createEntity(entityName, Ogre::SceneManager::PT_CUBE);
		m_entity->setMaterialName("MisMedical/MetalBox");
		cubenode->attachObject(m_entity);
		cubenode->setPosition(center);
		cubenode->setScale(size* 0.02);
	}
 	else
 	{
 		m_entity = scenemgr->createEntity(entityName, m_CreateInfo.m_s3mfilename);
 		m_entity->setMaterialName("MisMedical/MetalBox");
 
 		updateMesh(m_entity->getMesh(), m_CreateInfo.m_SceneMeshInitPosOffset);
 
 		cubenode->attachObject(m_entity);
 		cubenode->setPosition(center);
 		cubenode->setScale(m_CreateInfo.m_SceneMeshInitScaleOffset);
 	}

	m_node = cubenode;

	if (mass > 0)
	{
		m_body->SetGravity(OgreToGPVec3(m_CreateInfo.m_CustomGravityDir*m_CreateInfo.m_GravityValue));
	}

	m_body = PhysicsWrapper::GetSingleTon().CreateDynamicBox(center, size, mass, false);
	m_body->m_TubeCollideSolvePriority += 1;

}
//==========================================================================================================================================
void MisMedicRigidPrimtive::CreateRigidSphere(Ogre::String meshName,
	                                          Ogre::SceneManager * scenemgr,
											  Ogre::Vector3 center,float radius,float mass)
{
	static int ballId = 0;
	ballId++;

	Ogre::String NodeName   = "ballNode" + Ogre::StringConverter::toString(ballId);
	
	Ogre::String EntityName = "EntityBall" + Ogre::StringConverter::toString(ballId);

	m_node = scenemgr->getRootSceneNode()->createChildSceneNode(NodeName);
	
	m_entity = scenemgr->createEntity(EntityName, meshName);
	m_node->attachObject(m_entity);
	m_node->setPosition(center);

	//physics
	m_body = PhysicsWrapper::GetSingleTon().CreateSphere(center, radius , mass);
	m_body->SetFriction(0.3f);
	if (mass > 0)
	{
		m_body->SetGravity(OgreToGPVec3(m_CreateInfo.m_CustomGravityDir*m_CreateInfo.m_GravityValue));
	}
}
//==========================================================================================================================================
void MisMedicRigidPrimtive::CreateRigidCylinderY(Ogre::String entityName, Ogre::SceneManager * scenemgr, Ogre::Vector3 center, Ogre::Vector3 size, float mass)
{
	//y-direction cylinder
	Ogre::SceneNode * cubenode = scenemgr->getRootSceneNode()->createChildSceneNode(entityName + "Node");
	if (m_CreateInfo.m_s3mfilename.empty())
	{
		m_entity = scenemgr->createEntity(entityName, Ogre::SceneManager::PT_CUBE);
		m_entity->setMaterialName("jinshu");
		cubenode->attachObject(m_entity);
		cubenode->setPosition(center);
		cubenode->setScale(size* 0.02);
	}
	else
	{
		m_entity = scenemgr->createEntity(entityName, m_CreateInfo.m_s3mfilename);
		m_entity->setMaterialName("MisMedical/MetalBox");

		updateMesh(m_entity->getMesh(), m_CreateInfo.m_SceneMeshInitPosOffset);

		cubenode->attachObject(m_entity);
		cubenode->setPosition(center);
		cubenode->setScale(m_CreateInfo.m_SceneMeshInitScaleOffset);
	}

	m_node = cubenode;

	Ogre::Vector3 pointA = center + Ogre::Vector3(0 , size.y , 0);
	
	Ogre::Vector3 pointB = center - Ogre::Vector3(0 , size.y , 0);
	
	m_body = PhysicsWrapper::GetSingleTon().CreateDynamicCynlinder(pointA, pointB , size.x , mass);
	m_body->m_TubeCollideSolvePriority += 1;

	if (mass > 0)
	{
		m_body->SetGravity(OgreToGPVec3(m_CreateInfo.m_CustomGravityDir*m_CreateInfo.m_GravityValue));
	}
}

void MisMedicRigidPrimtive::RemovePhysicsPart()
{
	if(m_body)
	{
		if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
		   PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemoveRigidBody(m_body);

		GFPhysCollideShape * collideShape = m_body->GetCollisionShape();
		if(collideShape)
		{
			delete collideShape;
		}
		delete m_body;
		m_body = 0;

		if (m_MeshVertex)
		{
			delete m_MeshVertex;
			m_MeshVertex = 0;
		}

		if (m_MeshLocalVertex)
		{
			delete m_MeshLocalVertex;
			m_MeshLocalVertex = 0;
		}
		if (m_MeshTriangles)
		{
			delete m_MeshTriangles;
			m_MeshTriangles = 0;
		}

		if (m_MeshData)
		{
			delete m_MeshData;
			m_MeshData = 0;
		}
	}
}

void MisMedicRigidPrimtive::RemoveGraphicPart()
{
	Ogre::SceneManager * scenemgr = MXOgreWrapper::Get()->GetDefaultSceneManger();
    if(m_node)
    {
	    if(m_entity)
	    {
		    m_node->detachObject(m_entity);
		    scenemgr->destroyEntity(m_entity);
            m_entity = 0;
	    }
        if(m_RendObject)
        {
            m_node->detachObject(m_RendObject);
            scenemgr->destroyManualObject(m_RendObject);
            m_RendObject = 0;
        }

        scenemgr->destroySceneNode(m_node);
        m_node = 0;
    }
}
////////////////////////////////////////////////////////////////////////// 
// inherit base class MisMedicOrganInterface
Ogre::Vector2 MisMedicRigidPrimtive::GetTextureCoord(GFPhysSoftBody * sb ,GFPhysSoftBodyFace * face , float weights[3])
{
	
	return Ogre::Vector2(1);
};

void MisMedicRigidPrimtive::Update(float dt , Ogre::SceneManager * scenemgr , Ogre::Camera * camera)
{

};

//=======================================================================================================
void MisMedicRigidPrimtive::UpdateScene(float dt, Ogre::Camera * camera)
{
	//if (m_CreateInfo.m_mass != 0)
    if (m_body && m_RendObject)
	{
		GFPhysTransform trans = m_body->GetWorldTransform();

		GFPhysVector3 physpos = trans.GetOrigin();

		GFPhysQuaternion Quaternion = trans.GetRotation();

		m_node->setOrientation(Quaternion.w(),Quaternion.x(),Quaternion.y(),Quaternion.z());
		m_node->setPosition(physpos.x() , physpos.y(), physpos.z());

        ////////////////////////////////////////////        
       
        m_RendObject->setDynamic(false);
        m_RendObject->setVisible(true);
        m_RendObject->clear();
        m_RendObject->begin("ToolObjectFlatGreen");
        
        for(int f = 0 ; f < m_serializer->m_InitFaces.size() ; f++)
        {
            //MisSerialFace face = m_serializer->m_InitFaces[f];
            GFPhysVector3 v0 = m_serializer->m_NodeInitPositions[m_serializer->m_InitFaces[f].m_Index[0]];
            GFPhysVector3 v1 = m_serializer->m_NodeInitPositions[m_serializer->m_InitFaces[f].m_Index[1]];
            GFPhysVector3 v2 = m_serializer->m_NodeInitPositions[m_serializer->m_InitFaces[f].m_Index[2]];

            Ogre::Vector3 pos0(v0.m_x , v0.m_y , v0.m_z);
            Ogre::Vector3 pos1(v1.m_x , v1.m_y , v1.m_z);
            Ogre::Vector3 pos2(v2.m_x , v2.m_y , v2.m_z);

            Ogre::Vector3 normal = (pos1-pos0).crossProduct(pos2-pos0).normalisedCopy();

            m_RendObject->position(pos0);
            m_RendObject->normal(normal);

            m_RendObject->position(pos1);
            m_RendObject->normal(normal);

            m_RendObject->position(pos2);
            m_RendObject->normal(normal);
        }
        m_RendObject->end();
        ////////////////////////////////////////////
	}
	else if (m_body && m_entity && m_body->GetInvMass() > 0)
	{
		GFPhysTransform trans = m_body->GetWorldTransform();

		GFPhysVector3 physpos = trans.GetOrigin();

		GFPhysQuaternion Quaternion = trans.GetRotation();

		if (m_node)
		{
			m_node->setOrientation(Quaternion.w(), Quaternion.x(), Quaternion.y(), Quaternion.z());
			m_node->setPosition(physpos.x(), physpos.y(), physpos.z());
		}
	}
	MisMedicOrganInterface::UpdateScene(dt,camera);
}

bool MisMedicRigidPrimtive::CanBeGrasp()
{

	return true;
};

float MisMedicRigidPrimtive::GetForceFeedBackRation()
{

	return 0;
};

void  MisMedicRigidPrimtive::SetForceFeedBackRation(float ration)
{

};

// void MisMedicRigidPrimtive::ToolWithElectricTouched(ITool * tool , GFPhysSoftBodyFace * face , float weights[3])
// {
// 
// };

void MisMedicRigidPrimtive::ToolPunctureSurface(ITool * tool , GFPhysSoftBodyFace * face , const float weights[3])
{

};

void MisMedicRigidPrimtive::NotifyRigidBodyRemovedFromWorld(GFPhysRigidBody * rb)
{

};

void MisMedicRigidPrimtive::Create( MisMedicDynObjConstructInfo & constructInfo,
	                                Ogre::SceneManager * sceneMgr,
	                                Ogre::String & entityName)
{
	m_CreateInfo = constructInfo;
	
	m_CreateInfo.m_objTopologyType = DOT_RIGIDBODY;
	
	float mass = m_CreateInfo.m_mass;
	
	Ogre::String rigidType = m_CreateInfo.m_RigidType;
	
	Ogre::String rigidName = m_CreateInfo.m_name;
	
	transform(rigidType.begin(), rigidType.end(), rigidType.begin(), ::tolower);
	
	transform(rigidName.begin(), rigidName.end(), rigidName.begin(), ::tolower);

	int rtype = rigidType.find("box");
	if ((int)rigidType.find("box") >= 0)
	{
		CreateRigidBox(entityName,sceneMgr,
			           m_CreateInfo.m_Position, 
					   m_CreateInfo.m_InitSize,
			           mass);

		m_node->setVisible(m_CreateInfo.m_Visible);

		SetEntityMaterial(m_CreateInfo.m_materialname[0]);

		m_body->SetDamping(6, 6);

		m_RigidType = RT_BOX;

		return;
	}
	else if ((int)rigidType.find("sphere") >= 0)
	{

		CreateRigidSphere(m_CreateInfo.m_s3mfilename, sceneMgr,
			              m_CreateInfo.m_Position,
			              m_CreateInfo.m_InitSize.x,
			              mass);

		m_node->setVisible(m_CreateInfo.m_Visible);

		SetEntityMaterial(m_CreateInfo.m_materialname[0]);

		m_body->SetDamping(6, 6);

		m_RigidType = RT_SPHERE;
		
		return;
	}
	else if ((int)rigidType.find("cylindery") >= 0)
	{
		
		CreateRigidCylinderY(entityName, sceneMgr,
			                 m_CreateInfo.m_Position,
			                 m_CreateInfo.m_InitSize,
			                 mass);

		m_node->setVisible(m_CreateInfo.m_Visible);

		SetEntityMaterial(m_CreateInfo.m_materialname[0]);

		m_body->SetDamping(6, 6);

		m_RigidType = RT_CYLINDER;

		return;
	}
	else if ((int)rigidType.find("trimesh") >= 0)
	{
		CreateRigidMeshFromMMSFile(sceneMgr,m_CreateInfo.m_s3mfilename,OgreToGPVec3(m_CreateInfo.m_Position));
		//add rigid object to correct tool's position

		if (m_CreateInfo.m_CollideWithTool)
		    ToolSpenetrateMgr::GetInstance()->AddStaticObject(this);

		m_RigidType = RT_TRIMESH;

		return;
	}
}
void MisMedicRigidPrimtive::SetPosition(const GFPhysVector3 & pos)
{
	if (m_RigidType != RT_TRIMESH)
	{
		m_body->GetWorldTransform().SetOrigin(pos);
	}
	else/// for triangle mesh we need refit (not rebuild) tree which is a little expensive so be carefully call this
	{
		GFPhysBvhTriMeshShape * bvshape = dynamic_cast<GFPhysBvhTriMeshShape*>(m_body->GetCollisionShape());
		
		GFPhysTriangleMesh * meshData = bvshape->GetMeshData();
		
		for (int v = 0; v < meshData->m_NumVertices; v++)
		{
			m_MeshVertex[v] = m_MeshLocalVertex[v] + pos;
		}

		bvshape->ReFitBVTree();
	}
}

void MisMedicRigidPrimtive::SetTransform(const GFPhysTransform & trans)
{
	if (m_RigidType != RT_TRIMESH)
	{
		m_body->SetWorldTransform(trans);
	}
	else/// for triangle mesh we need refit (not rebuild) tree which is a little expensive so be carefully call this
	{
		GFPhysBvhTriMeshShape * bvshape = dynamic_cast<GFPhysBvhTriMeshShape*>(m_body->GetCollisionShape());

		GFPhysTriangleMesh * meshData = bvshape->GetMeshData();

		for (int v = 0; v < meshData->m_NumVertices; v++)
		{
			m_MeshVertex[v] = trans(m_MeshLocalVertex[v]);
		}

		bvshape->ReFitBVTree();
	}
}
GFPhysBvhTriMeshShape * MisMedicRigidPrimtive::GetCollisionTriMesh()
{
	GFPhysBvhTriMeshShape * triMesh = dynamic_cast<GFPhysBvhTriMeshShape*>(m_body->GetCollisionShape());
	return triMesh;
}

void MisMedicRigidPrimtive::CreateRigidMeshFromMMSFile(Ogre::SceneManager * scenemgr , 
	                                                   const std::string & mmsfilename ,
													   GFPhysVector3 & position)
{
	if (m_serializer)
	{
		delete m_serializer;
	}
	m_serializer = new MisMedicObjetSerializer();
	m_serializer->ReadFromOrganObjectFile(-1, mmsfilename, mmsfilename, mmsfilename, Ogre::Vector3::ZERO);
	CreateRigidMeshFromSerialize(*m_serializer, scenemgr, position);
}

void MisMedicRigidPrimtive::CreateRigidMeshFromSerialize(MisMedicObjetSerializer& serializer,
	                                                     Ogre::SceneManager * scenemgr,
														 GFPhysVector3 & position)
{
	m_MeshVertex = new GFPhysVector3[serializer.m_NodeInitPositions.size()];

	m_MeshLocalVertex = new GFPhysVector3[serializer.m_NodeInitPositions.size()];

	m_MeshTriangles = new GFPhysMeshDataTriangle[serializer.m_InitFaces.size()];

	for (int v = 0; v < serializer.m_NodeInitPositions.size(); v++)
	{
		 m_MeshVertex[v] = serializer.m_NodeInitPositions[v] + position;
		 m_MeshLocalVertex[v] = serializer.m_NodeInitPositions[v];
	}

	for (int f = 0; f < serializer.m_InitFaces.size(); f++)
	{
		m_MeshTriangles[f].m_VertexIndex[0] = serializer.m_InitFaces[f].m_Index[0];
		m_MeshTriangles[f].m_VertexIndex[1] = serializer.m_InitFaces[f].m_Index[1];
		m_MeshTriangles[f].m_VertexIndex[2] = serializer.m_InitFaces[f].m_Index[2];
	}
	m_MeshData = new GFPhysTriangleMesh();

	m_MeshData->m_Vertices = (unsigned char *)m_MeshVertex;

	m_MeshData->m_Triangles = m_MeshTriangles;

	m_MeshData->m_NumVertices = serializer.m_NodeInitPositions.size();

	m_MeshData->m_NumTriangles = serializer.m_InitFaces.size();

	m_MeshData->m_vertexStride = sizeof(GFPhysVector3);

	GFPhysCollideShape * bvhMeshShape = new GFPhysBvhTriMeshShape(m_MeshData, false, true);
	bvhMeshShape->SetMargin(0.02);

	GFPhysTransform startTransform;
	startTransform.SetIdentity();
	//startTransform.SetOrigin(position);
	
	GFPhysVector3 localInertia(0, 0, 0);
	
	m_body = PhysicsWrapper::GetSingleTon().CreateRigidBody(0, bvhMeshShape, localInertia, startTransform);
}

