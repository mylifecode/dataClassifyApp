#pragma once
#include "BasicTraining.h"
#include "OgreSceneManager.h"
#include "CollisionTest.h"
#include "MisNewTraining.h"
//#include "Painting.h"
class MisMedicRigidPrimtive;
class SYCameraSkillA : public MisNewTraining
{
public:
	enum TRAINSTATE
	{
		TT_NOTFOCUS,
		TT_INFOCUS,
		TT_INDISAPPEAR,
	};
	struct SphereMeshData : public GFPhysTriangleProcessor
	{
		SphereMeshData()
		{
			m_WorldMesh  = "";
			m_MeshMaterial = "";

			m_MeshVertex = 0;
			m_MeshTriangles = 0;
			m_MeshData = 0;
			m_CollisionShape = 0;
		}

		SphereMeshData(std::string  worldMesh, std::string material) : m_WorldMesh(worldMesh), m_MeshMaterial(material)
		{
			m_MeshVertex = 0;
			m_MeshTriangles = 0;
			m_MeshData = 0;
			m_CollisionShape = 0;
			Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().load(worldMesh, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
			BuildCollisionData(mesh);
		}

		~SphereMeshData()
		{
			if (m_MeshVertex)
				delete[]m_MeshVertex;
			
			if (m_MeshTriangles)
				delete[]m_MeshTriangles;
			
			if (m_MeshData)
				delete[]m_MeshData;
			
			if (m_CollisionShape)
				delete m_CollisionShape;
		}

		void BuildCollisionData(Ogre::MeshPtr mesh)
		{
			if (m_CollisionShape == 0)
			{
				std::vector<Ogre::Vector3>  vertices;
				std::vector<Ogre::Vector2>  textures;
				std::vector<unsigned int>   indices;
				MisMedicOrganInterface::ExtractOgreMeshInfo(mesh, vertices, textures, indices);

				m_MeshVertex = new GFPhysVector3[vertices.size()];
				m_MeshTriangles = new GFPhysMeshDataTriangle[indices.size() / 3];

				for (int v = 0; v < vertices.size(); v++)
				{
					m_MeshVertex[v] = OgreToGPVec3(vertices[v]);
				}

				for (int f = 0; f < indices.size() / 3; f++)
				{
					m_MeshTriangles[f].m_VertexIndex[0] = indices[f * 3 + 0];
					m_MeshTriangles[f].m_VertexIndex[1] = indices[f * 3 + 1];
					m_MeshTriangles[f].m_VertexIndex[2] = indices[f * 3 + 2];
				}

				m_MeshData = new GFPhysTriangleMesh();

				m_MeshData->m_Vertices = (unsigned char *)m_MeshVertex;

				m_MeshData->m_Triangles = m_MeshTriangles;

				m_MeshData->m_NumVertices = vertices.size();

				m_MeshData->m_NumTriangles = indices.size() / 3;

				m_MeshData->m_vertexStride = sizeof(GFPhysVector3);

				m_CollisionShape = new GFPhysBvhTriMeshShape(m_MeshData, false, true);
			}
		}

		bool isRayIntersect(const GFPhysVector3 & source, const GFPhysVector3 & target)
		{
			m_RaySource = source;
			m_RayTarget = target;
			m_IsIntersecRay = false;
			m_CollisionShape->CastRay(this, source, target);

			return m_IsIntersecRay;
		}

		void ProcessTriangle(GFPhysVector3* triangleVerts, int partId, int triangleIndex, void * UserData)
		{
			Real  Rayweight;
			
			Real  triangleWeight[3];
			
			GFPhysVector3  intersectpt;

			bool intersect = LineIntersectTriangle(triangleVerts[0],triangleVerts[1],triangleVerts[2],
				                                   m_RaySource , m_RayTarget, Rayweight ,intersectpt ,triangleWeight);

			if (intersect && Rayweight >= 0 && Rayweight <= 1)
			{
				m_IsIntersecRay = true;
			}
		}
		std::string m_WorldMesh;
		std::string m_MeshMaterial;

		GFPhysVector3 * m_MeshVertex;
		GFPhysMeshDataTriangle * m_MeshTriangles;
		GFPhysTriangleMesh * m_MeshData;

		GFPhysBvhTriMeshShape * m_CollisionShape;

		GFPhysVector3 m_RaySource;
		GFPhysVector3 m_RayTarget;
		bool m_IsIntersecRay;
	};

	struct SceneSphereInstance
	{
		SceneSphereInstance(Ogre::SceneNode * node, Ogre::Entity * entity, std::string  name) :m_SceneNode(node), m_Entity(entity), m_name(name)
		{
			m_FocusTime = 0;
			m_FailedNum = 0;
		}
		Ogre::SceneNode * m_SceneNode;
		Ogre::Entity * m_Entity;
		std::string m_name;

		//logic
		float m_FocusTime;
		float m_FailedNum;

	};
	TRAINSTATE m_state;

	//float m_focuseTime;

	float m_ScaleTime;

	SYCameraSkillA(MXOgreWrapper::CameraState eCS);
	
	virtual ~SYCameraSkillA(void);
	
	virtual bool Update(float dt);
	
	void UpdateState(float dt);

	void NextSphere();

	void FinishTraining();

	virtual bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);

	void CreateAllSpheresRandomly();

	void CreateSphereInRandomPos(const std::string & letter,
		                         float minTheta , float maxTheta,
								 float minAlpha , float maxAlpha );
	void CreateSphereInHemiSphere(const std::string & letter, float theta, float alpha);

	void OnSaveTrainingReport();
private:

	SYScoreTable * GetScoreTable();

	Ogre::Vector3  GetCameraPivot();

	Ogre::SceneNode * CreateCameraRect(const std::string & rectMesh);

	Ogre::SceneNode * CreateWorldSphere(const std::string & letter, 
									    const Ogre::Vector3 & letterPos);
	Ogre::SceneNode * m_CamLetterNode;

	Ogre::Entity * m_CamLetterEntity;

	MisMedicRigidPrimtive * m_CollisonMesh;

	std::vector<SceneSphereInstance> m_SceneSpheres;//only visible one in same time

	std::vector<SceneSphereInstance> m_ElimitedSpheres;

	std::map<std::string, SphereMeshData*> m_SphereMeshMap;

	int m_NumSphere;
};
