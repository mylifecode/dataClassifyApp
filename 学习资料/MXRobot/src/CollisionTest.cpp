#include "CollisionTest.h"
#include "collision\NarrowPhase\GoPhysPrimitiveTest.h"
#include "MXOgreGraphic.h"
#include "collision\CollisionShapes\GoPhysTriangleShape.h"

class GFPhysRayTestCallback : public GFPhysNodeOverlapCallback
{
public:
    GFPhysRayTestCallback(const GFPhysVector3 & raySource , const GFPhysVector3 & rayTarget)
    {       
        m_raySource = raySource;
        m_rayTarget = rayTarget;
        m_bingo = false;
    }

    void ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData)
    {        
        GFPhysTriangleShape  * triangle = (GFPhysTriangleShape*)UserData;

        Real weight;

        Real TriangleWeight[3];

        GFPhysVector3 instersectpt;

        bool succed = false;

        succed = LineIntersectTriangle(
            triangle->m_vertices1[0] , 
            triangle->m_vertices1[1] ,                  
            triangle->m_vertices1[2] , 
            m_raySource , 
            m_rayTarget , 
            weight ,
            instersectpt , 
            TriangleWeight);

        if (succed && weight >=0.005f && weight <= 1)
        {     
            m_bingo = true;
        }
    }

    GFPhysVector3 m_raySource;
    GFPhysVector3 m_rayTarget;
    bool m_bingo;
};
CollisionTest::CollisionTest( void )
{
}

CollisionTest::CollisionTest(Ogre::SceneManager* pSceneManager)
{
}

CollisionTest::~CollisionTest( void )
{
    for (std::vector<Geoinfo*>::iterator s = m_objects.begin(); s != m_objects.end(); ++s)
    {
        delete *s;
    }
    m_objects.clear();
}

void CollisionTest::AddStaticEntity(Ogre::String name, Ogre::Entity* pEntity )
{
    std::vector<Ogre::Vector3> vertices;
    std::vector<unsigned int> indices;

    vertices.clear();
    indices.clear();

    vertices.reserve(10000);
    indices.reserve(10000);

    bool added_shared = false;
    size_t current_offset = 0;
    size_t shared_offset = 0;
    size_t next_offset = 0;
    size_t index_offset = 0;

    int vertex_count = 0;
    int index_count = 0;

    Ogre::MeshPtr mesh = pEntity->getMesh();
    for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh( i );

        // We only need to add the shared vertices once
        if(submesh->useSharedVertices)
        {
            if( !added_shared )
            {
                vertex_count += mesh->sharedVertexData->vertexCount;
                added_shared = true;
            }
        }
        else
        {
            vertex_count += submesh->vertexData->vertexCount;
        }

        // Add the indices
        index_count += submesh->indexData->indexCount;
    }

    added_shared = false;

    // Run through the submeshes again, adding the data into the arrays
    for ( unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh(i);

        Ogre::VertexData* vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;

        if((!submesh->useSharedVertices)||(submesh->useSharedVertices && !added_shared))
        {
            if(submesh->useSharedVertices)
            {
                added_shared = true;
                shared_offset = current_offset;
            }

            const Ogre::VertexElement* posElem =
                vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);

            Ogre::HardwareVertexBufferSharedPtr vbuf =
                vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());

            unsigned char* vertex =
                static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

            // There is _no_ baseVertexPointerToElement() which takes an Ogre::Real or a double
            //  as second argument. So make it float, to avoid trouble when Ogre::Real will
            //  be comiled/typedefed as double:
            //      Ogre::Real* pReal;
            float* pReal;

            for( size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
            {
                posElem->baseVertexPointerToElement(vertex, &pReal);

                Ogre::Vector3 pt(pReal[0], pReal[1], pReal[2]);
                
                vertices.push_back(pt);
            }

            vbuf->unlock();
            next_offset += vertex_data->vertexCount;
        }


        Ogre::IndexData* index_data = submesh->indexData;
        size_t numTris = index_data->indexCount / 3;
        Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;

        bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);

        unsigned long*  pLong = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);


        size_t offset = (submesh->useSharedVertices)? shared_offset : current_offset;

        if ( use32bitindexes )
        {
            for ( size_t k = 0; k < numTris*3; ++k)
            {
                //indices[index_offset++] = pLong[k] + static_cast<unsigned long>(offset);
                indices.push_back(pLong[k] + static_cast<unsigned long>(offset));
            }
        }
        else
        {
            for ( size_t k = 0; k < numTris*3; ++k)
            {
                //indices[index_offset++] = static_cast<unsigned long>(pShort[k]) + static_cast<unsigned long>(offset);
                indices.push_back(static_cast<unsigned long>(pShort[k]) + static_cast<unsigned long>(offset));
            }
        }

        ibuf->unlock();
        current_offset = next_offset;
    }

    //////////////////////////////////////////////////////////////////////////
    GFPhysDBVTree* NodeTree = new GFPhysDBVTree();
    int numFaces = indices.size() / 3;
    unsigned int i1,i2,i3 = 0;   
    for(std::size_t f = 0;f < numFaces; ++f)
    {
        i1 = indices[f * 3];
        i2 = indices[f * 3 + 1];
        i3 = indices[f * 3 + 2];

        GFPhysVector3 aabbmin = OgreToGPVec3(vertices[i1]);
        GFPhysVector3 aabbmax = OgreToGPVec3(vertices[i1]);

        aabbmin.SetMin(OgreToGPVec3(vertices[i2]));
        aabbmin.SetMin(OgreToGPVec3(vertices[i3]));
        aabbmax.SetMax(OgreToGPVec3(vertices[i2]));
        aabbmax.SetMax(OgreToGPVec3(vertices[i3]));

        GFPhysDBVNode * dbvn = NodeTree->InsertAABBNode(aabbmin,aabbmax);

        GFPhysTriangleShape* triangle= new GFPhysTriangleShape(
            OgreToGPVec3(vertices[i1]),
            OgreToGPVec3(vertices[i2]),
            OgreToGPVec3(vertices[i3]));
        dbvn->m_UserData = triangle;
    }

    //////////////////////////////////////////////////////////////////////////
    Geoinfo* info = new Geoinfo(name,NodeTree);
    m_objects.push_back(info);
}

void CollisionTest::RemoveStaticEntity( Ogre::Entity* pEntity )
{
}

const Ogre::String CollisionTest::RayTest( const Ogre::Ray& ray, const Ogre::Real fDistance )
{
    Ogre::String temp = "NULL";
    Ogre::Vector3 origin = ray.getPoint(0);
    Ogre::Vector3 target = ray.getPoint(fDistance);
    GFPhysRayTestCallback nodeCallback(OgreToGPVec3(origin) , OgreToGPVec3(target));

#if 1      
    for (int i = 0 ; i < m_objects.size(); ++i)
    {
        m_objects[i]->CollisionTree->TraverseTreeAgainstRay(&nodeCallback, OgreToGPVec3(origin),OgreToGPVec3(target));

        if (nodeCallback.m_bingo)
        {
            return m_objects[i]->objName;
        }          
    }
         
    return temp;
#else

    for (int i = 0 ; i < m_objects.size(); i++)
    {
        Ogre::String pname = m_objects[i]->Name;

        int numFaces = m_objects[i]->Indices.size() / 3;
        unsigned int i1,i2,i3 = 0;

        for(std::size_t f = 0;f < numFaces;++f)
        {
            i1 = m_objects[i]->Indices[f * 3];
            i2 = m_objects[i]->Indices[f * 3 + 1];
            i3 = m_objects[i]->Indices[f * 3 + 2];        

            Real weight;
            Real TriangleWeight[3];
            GFPhysVector3 instersectpt;

            bool succed = false;
            succed = LineIntersectTriangle( 
                OgreToGPVec3(m_objects[i]->Vertices[i1]),
                OgreToGPVec3(m_objects[i]->Vertices[i2]),
                OgreToGPVec3(m_objects[i]->Vertices[i3]),
                OgreToGPVec3(origin),
                OgreToGPVec3(target),
                weight ,
                instersectpt , 
                TriangleWeight);
            if(succed && weight >=0.005f && weight <= 1)//ray intersect
            {
                return pname;
            }
        }
    }
    
    return temp;
#endif

    

}

//const Ogre::String CollisionTest::SphereTest(const Ogre::Sphere& sphere)
//{
//	m_pCollisionContext->reset();
//	OgreOpcode::CollisionPair** ppCollisionPair = NULL;
//	m_pCollisionContext->sphereCheck(sphere, OgreOpcode::COLLTYPE_EXACT, OgreOpcode::COLLTYPE_ALWAYS_EXACT, ppCollisionPair );
//	Ogre::String name = "NULL";
//	Ogre::Real d=0.0;
//	while (*ppCollisionPair)
//	{
//		name = (*ppCollisionPair)->this_object->getName();
//		ppCollisionPair++;
//		d++;
//	}
//	return name;
//}