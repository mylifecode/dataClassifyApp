#include "ParticleOrganFaceCollideAlgorithm.h"
#include "DynamicObjectRenderable.h"

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
//======================================================================================================
PresetParticleOrganFaceCollideAlogrithm::~PresetParticleOrganFaceCollideAlogrithm()
{
	//clear incident map first
	stdext::hash_map<unsigned int , VertexIncidentFace*>::iterator uitor = m_vertIncidentFaces.begin();
	while(uitor != m_vertIncidentFaces.end())
	{
		delete uitor->second;
		uitor++;
	}
	m_vertIncidentFaces.clear();

	stdext::hash_map<GFPhysSoftBodyNode * , VertexIncidentFace*>::iterator sitor = m_SoftNodeIncidentFaces.begin();
	while(sitor != m_SoftNodeIncidentFaces.end())
	{
		delete sitor->second;
		sitor++;
	}
	m_SoftNodeIncidentFaces.clear();
}
//======================================================================================================
void PresetParticleOrganFaceCollideAlogrithm::Initialize(MisMedicOrgan_Ordinary * object)
{
	m_HostObject = object;
	
	m_isadjinfocollected = false;

	CollectFVAdjacent(m_HostObject);
}
//======================================================================================================
void PresetParticleOrganFaceCollideAlogrithm::CollectFVAdjacent(MisMedicOrgan_Ordinary * object)
{
	if (m_isadjinfocollected == true)
		return;

	//clear incident map first
	stdext::hash_map<unsigned int , VertexIncidentFace*>::iterator uitor = m_vertIncidentFaces.begin();
	while(uitor != m_vertIncidentFaces.end())
	{
		delete uitor->second;
		uitor++;
	}
	m_vertIncidentFaces.clear();

	stdext::hash_map<GFPhysSoftBodyNode * , VertexIncidentFace*>::iterator sitor = m_SoftNodeIncidentFaces.begin();
	while(sitor != m_SoftNodeIncidentFaces.end())
	{
		delete sitor->second;
		sitor++;
	}
	m_SoftNodeIncidentFaces.clear();

	if(object->m_FFDObject)//skin object use skin faces to build adjacent information
	{
		std::vector<DynamicSurfaceFreeDeformObject::RendFaceData> & rendFaces = object->m_FFDObject->m_rendfaces;
		
		for(size_t f = 0 ; f < rendFaces.size() ; f++)
		{
			DynamicSurfaceFreeDeformObject::RendFaceData & rendFace = rendFaces[f];
			int vid[3];
			vid[0]  = rendFace.manivertindex[0];
			vid[1]  = rendFace.manivertindex[1];
			vid[2]  = rendFace.manivertindex[2];

			for(size_t c = 0 ; c < 3; c++)
			{
				VertexIncidentFace * incident = NULL;

				stdext::hash_map<unsigned int , VertexIncidentFace*>::iterator itor = m_vertIncidentFaces.find(vid[c]);

				if(itor != m_vertIncidentFaces.end())
					incident = itor->second;
				else
				{
					incident = new VertexIncidentFace();
					m_vertIncidentFaces.insert(std::make_pair(vid[c], incident));
				}
				incident->m_Incidentfaces.push_back(f);
			}
		}
		m_isadjinfocollected = true;
	}
	else//use physics data
	{
		for(size_t f = 0 ; f < m_HostObject->m_OriginFaces.size() ; f++)
		{
			MMO_Face & rendFace = m_HostObject->m_OriginFaces[f];

			if(rendFace.m_physface)
			{
				GFPhysSoftBodyNode * softnodes[3];

				softnodes[0]  = rendFace.m_physface->m_Nodes[0];

				softnodes[1]  = rendFace.m_physface->m_Nodes[1];

				softnodes[2]  = rendFace.m_physface->m_Nodes[2];

				for(size_t c = 0 ; c < 3; c++)
				{
					VertexIncidentFace * incident = NULL;

					stdext::hash_map<GFPhysSoftBodyNode* , VertexIncidentFace*>::iterator itor = m_SoftNodeIncidentFaces.find(softnodes[c]);

					if(itor != m_SoftNodeIncidentFaces.end())
					   incident = itor->second;
					else
					{
					   incident = new VertexIncidentFace();
					   m_SoftNodeIncidentFaces.insert(std::make_pair(softnodes[c], incident));
					}
					incident->m_Incidentfaces.push_back(f);
				}
			}
		}
		m_isadjinfocollected = true;
	}
}
//===============================================================================================================================
int  PresetParticleOrganFaceCollideAlogrithm::GetBloodFaceCount()
{
	if(m_HostObject == 0)
	   return 0;

	if(m_HostObject->m_FFDObject)
	{
		std::vector<DynamicSurfaceFreeDeformObject::RendFaceData> & rendFaces = m_HostObject->m_FFDObject->m_rendfaces;
		return (int)rendFaces.size();
	}
	else 
	{
		return (int)m_HostObject->m_OriginFaces.size();
	}
}
bool PresetParticleOrganFaceCollideAlogrithm::IsFaceRemoved(int faceid)
{
	if(m_HostObject->m_FFDObject)
	{
		return false;
	}
	else
	{
		MMO_Face face = m_HostObject->GetMMOFace_OriginPart(faceid);//m_OriginFaces[faceid];

		if(face.m_physface == 0)//face may be cuttedd or removed
		   return true;//face removed
		else
		   return false;
	}
}
//===============================================================================================================================
bool PresetParticleOrganFaceCollideAlogrithm::GetBloodFaceVertexPosition(int faceid , Ogre::Vector3 vertexPos[3])
{
	if(m_HostObject->m_FFDObject)
	{
		std::vector<DynamicSurfaceFreeDeformObject::RendFaceData> & rendFaces = m_HostObject->m_FFDObject->m_rendfaces;

		std::vector<DynamicSurfaceFreeDeformObject::ManipulatedVertex> & maniVertex = m_HostObject->m_FFDObject->m_manivertex;

		DynamicSurfaceFreeDeformObject::RendFaceData & rendFace = rendFaces[faceid];
		int vid[3];
		vid[0]  = rendFace.manivertindex[0];
		vid[1]  = rendFace.manivertindex[1];
		vid[2]  = rendFace.manivertindex[2];

		Ogre::Vector3 temp = maniVertex[vid[0]].m_Currposition;
		vertexPos[0] = Ogre::Vector3(temp.x , temp.z , -temp.y);

		temp = maniVertex[vid[1]].m_Currposition;
		vertexPos[1] = Ogre::Vector3(temp.x , temp.z , -temp.y);

		temp = maniVertex[vid[2]].m_Currposition;
		vertexPos[2] = Ogre::Vector3(temp.x , temp.z , -temp.y);
	}
	else
	{
		MMO_Face face = m_HostObject->m_OriginFaces[faceid];

		if(face.m_physface)//face may be cuttedd or removed
		{
			GFPhysVector3 temp = face.m_physface->m_Nodes[0]->m_CurrPosition;
			vertexPos[0] = Ogre::Vector3(temp.m_x , temp.m_y , temp.m_z);

			temp = face.m_physface->m_Nodes[1]->m_CurrPosition;
			vertexPos[1] = Ogre::Vector3(temp.m_x , temp.m_y , temp.m_z);

			temp = face.m_physface->m_Nodes[2]->m_CurrPosition;
			vertexPos[2] = Ogre::Vector3(temp.m_x , temp.m_y , temp.m_z);
		}
	}

	return true;
}
//===============================================================================================================================
bool PresetParticleOrganFaceCollideAlogrithm::GetBloodFaceVertexIndex(int faceid , int vertexIndex[3])
{
	if(m_HostObject->m_FFDObject)//skin object use skin face's data
	{
		std::vector<DynamicSurfaceFreeDeformObject::RendFaceData> & rendFaces = m_HostObject->m_FFDObject->m_rendfaces;

		std::vector<DynamicSurfaceFreeDeformObject::ManipulatedVertex> & maniVertex = m_HostObject->m_FFDObject->m_manivertex;

		DynamicSurfaceFreeDeformObject::RendFaceData & rendFace = rendFaces[faceid];

		vertexIndex[0]  = rendFace.manivertindex[0];

		vertexIndex[1]  = rendFace.manivertindex[1];

		vertexIndex[2]  = rendFace.manivertindex[2];
	}
	else//use origin physics data
	{
		MMO_Face face = m_HostObject->m_OriginFaces[faceid];

		vertexIndex[0]  = face.vi[0];

		vertexIndex[1]  = face.vi[1];

		vertexIndex[2]  = face.vi[2];
	}
	return true;
}
//======================================================================================================================
void PresetParticleOrganFaceCollideAlogrithm::GetCollidedFacesIncidentSoftNode(GFPhysSoftBodyNode * node , std::vector<ParticleCollideOrganFaceData> & CollideFacesId)
{
	stdext::hash_map<GFPhysSoftBodyNode * , VertexIncidentFace*>::iterator itor = m_SoftNodeIncidentFaces.find(node);

	if(itor != m_SoftNodeIncidentFaces.end())
	{
		VertexIncidentFace * IncidFace = itor->second;

		for(size_t f = 0 ; f < IncidFace->m_Incidentfaces.size() ; f++)
		{
			int faceID = IncidFace->m_Incidentfaces[f];

			ParticleCollideOrganFaceData collideface;

			collideface.m_FaceID = faceID;

			collideface.m_collideWeights[0] = collideface.m_collideWeights[1] = collideface.m_collideWeights[2] = 0;

			GFPhysSoftBodyFace * physface = m_HostObject->m_OriginFaces[faceID].m_physface;
			
			GFPhysSoftBodyNode * softbodynodes[3];
			softbodynodes[0] = physface->m_Nodes[0];
			softbodynodes[1] = physface->m_Nodes[1];
			softbodynodes[2] = physface->m_Nodes[2];

			int incidentcount = 0;

			for(int v = 0 ; v < 3 ; v++)
			{
				if(softbodynodes[v] == node)
				{
					collideface.m_collideWeights[v] = 1.0f;
					incidentcount++;
				}
				else
				{
					collideface.m_collideWeights[v] = 0.0f;
				}
			}
			if(incidentcount > 0)
			{
				CollideFacesId.push_back(collideface);
			}
		}
	}
}
//===============================================================================================================================
void PresetParticleOrganFaceCollideAlogrithm::GetCollidedFacesIncidentVertex(int vertexID , std::vector<ParticleCollideOrganFaceData> & CollideFacesId)
{
	stdext::hash_map<unsigned int , VertexIncidentFace*>::iterator itor = m_vertIncidentFaces.find(vertexID);

	if(itor != m_vertIncidentFaces.end())
	{
		VertexIncidentFace * IncidFace = itor->second;

		for(size_t f = 0 ; f < IncidFace->m_Incidentfaces.size() ; f++)
		{
			int faceID = IncidFace->m_Incidentfaces[f];

			ParticleCollideOrganFaceData collideface;

			collideface.m_FaceID = faceID;

			collideface.m_collideWeights[0] = collideface.m_collideWeights[1] = collideface.m_collideWeights[2] = 0;

			int vertexindex[3];
			GetBloodFaceVertexIndex(faceID , vertexindex);

			for(int v = 0 ; v < 3 ; v++)
			{
				if(vertexindex[v] == vertexID)
				{
					collideface.m_collideWeights[v] = 1.0f;
					CollideFacesId.push_back(collideface);
				}
			}
		}
	}
}
//=============================================================================================
void PresetParticleOrganFaceCollideAlogrithm::GetCollidedFacesIncidentEdge(GFPhysSoftBodyNode * node0 , GFPhysSoftBodyNode * node1 , float weight0 , std::vector<ParticleCollideOrganFaceData> & CollideFacesId)
{
	stdext::hash_map<GFPhysSoftBodyNode * , VertexIncidentFace*>::iterator itor = m_SoftNodeIncidentFaces.find(node0);

	if(itor != m_SoftNodeIncidentFaces.end())
	{
		VertexIncidentFace * IncidFace = itor->second;

		for(size_t f = 0 ; f < IncidFace->m_Incidentfaces.size() ; f++)
		{
			int faceID = IncidFace->m_Incidentfaces[f];

			ParticleCollideOrganFaceData cdface;
			cdface.m_FaceID = faceID;
			cdface.m_collideWeights[0] = cdface.m_collideWeights[1] = cdface.m_collideWeights[2] = 0;

			//get 3 nodes of this face
			GFPhysSoftBodyFace * physface = m_HostObject->m_OriginFaces[faceID].m_physface;

			GFPhysSoftBodyNode * softbodynodes[3];
			softbodynodes[0] = physface->m_Nodes[0];
			softbodynodes[1] = physface->m_Nodes[1];
			softbodynodes[2] = physface->m_Nodes[2];

			int incidentVertexCount = 0;

			for(int i = 0 ; i < 3 ; i++)
			{
				if(softbodynodes[i] == node0)
				{
					cdface.m_collideWeights[i] = weight0;
					incidentVertexCount++;
				}
				else if(softbodynodes[i] == node1)
				{
					cdface.m_collideWeights[i] = 1-weight0;
					incidentVertexCount++;
				}
				else
				{
					cdface.m_collideWeights[i] = 0;
				}
			}

			if(incidentVertexCount == 2)
			   CollideFacesId.push_back(cdface);
		}
	}
}
//===============================================================================================================================
void PresetParticleOrganFaceCollideAlogrithm::GetCollidedFacesIncidentEdge(int vid0 , int vid1 , float weight0 , std::vector<ParticleCollideOrganFaceData> & CollideFacesId)
{
	stdext::hash_map<unsigned int , VertexIncidentFace*>::iterator itor = m_vertIncidentFaces.find(vid0);

	if(itor != m_vertIncidentFaces.end())
	{
		VertexIncidentFace * IncidFace = itor->second;

		for(size_t f = 0 ; f < IncidFace->m_Incidentfaces.size() ; f++)
		{
			int faceID = IncidFace->m_Incidentfaces[f];

			int vertexindex[3];

			ParticleCollideOrganFaceData cdface;
			cdface.m_FaceID = faceID;
			cdface.m_collideWeights[0] = cdface.m_collideWeights[1] = cdface.m_collideWeights[2] = 0;

			GetBloodFaceVertexIndex(faceID , vertexindex);

			int incidentVertexCount = 0;

			for(int i = 0 ; i < 3 ; i++)
			{
				if(vertexindex[i] == vid0)
				{
					cdface.m_collideWeights[i] = weight0;
					incidentVertexCount++;
				}

				if(vertexindex[i] == vid1)
				{
					cdface.m_collideWeights[i] = 1-weight0;
					incidentVertexCount++;
				}
			}

			if(incidentVertexCount == 2)
				CollideFacesId.push_back(cdface);
		}
	}
}
//===============================================================================================================================
void PresetParticleOrganFaceCollideAlogrithm::GetCollidedFaces(int FaceId ,
															   const float Faceweigths[3] , 
															   std::vector<ParticleCollideOrganFaceData> & CollideFaces
															   )
{
	   if(m_HostObject->m_FFDObject)
	   {
			int FaceVertIndex[3];

			GetBloodFaceVertexIndex(FaceId , FaceVertIndex);

			if(Faceweigths[0] > 0.99f)//this particle is located in vertex 0
			   GetCollidedFacesIncidentVertex(FaceVertIndex[0] , CollideFaces);

			else if(Faceweigths[1] > 0.99f)//this particle is located in vertex 1
			   GetCollidedFacesIncidentVertex(FaceVertIndex[1] , CollideFaces);

			else if(Faceweigths[2] > 0.99f)//this particle is located in vertex 2
			   GetCollidedFacesIncidentVertex(FaceVertIndex[2] , CollideFaces);

			else if(Faceweigths[0] < 0.001f)//this particle is located in edge 12
			   GetCollidedFacesIncidentEdge(FaceVertIndex[1] , FaceVertIndex[2] , Faceweigths[1] , CollideFaces);

			else if(Faceweigths[1] < 0.001f)//this particle is located in edge 20
			   GetCollidedFacesIncidentEdge(FaceVertIndex[2] , FaceVertIndex[0] , Faceweigths[2] , CollideFaces);

			else if(Faceweigths[2] < 0.001f)//this particle is located in edge 01
			   GetCollidedFacesIncidentEdge(FaceVertIndex[0] , FaceVertIndex[1] , Faceweigths[0] , CollideFaces);
	   }
	   else
	   {
		   GFPhysSoftBodyNode * FaceNodes[3];
		   
		   GFPhysSoftBodyFace * physface = m_HostObject->m_OriginFaces[FaceId].m_physface;

		   FaceNodes[0] = physface->m_Nodes[0];
		   FaceNodes[1] = physface->m_Nodes[1];
		   FaceNodes[2] = physface->m_Nodes[2];

		   if(Faceweigths[0] > 0.99f)//this particle is located in vertex 0
			  GetCollidedFacesIncidentSoftNode(FaceNodes[0] , CollideFaces);

		   else if(Faceweigths[1] > 0.99f)//this particle is located in vertex 1
			  GetCollidedFacesIncidentSoftNode(FaceNodes[1] , CollideFaces);

		   else if(Faceweigths[2] > 0.99f)//this particle is located in vertex 2
			  GetCollidedFacesIncidentSoftNode(FaceNodes[2] , CollideFaces);

		   else if(Faceweigths[0] < 0.001f)//this particle is located in edge 12
			  GetCollidedFacesIncidentEdge(FaceNodes[1] , FaceNodes[2] , Faceweigths[1] , CollideFaces);

		   else if(Faceweigths[1] < 0.001f)//this particle is located in edge 20
			  GetCollidedFacesIncidentEdge(FaceNodes[2] , FaceNodes[0] , Faceweigths[2] , CollideFaces);

		   else if(Faceweigths[2] < 0.001f)//this particle is located in edge 01
			  GetCollidedFacesIncidentEdge(FaceNodes[0] , FaceNodes[1] , Faceweigths[0] , CollideFaces);

	   }

}
//============================================================================================================
bool PresetParticleOrganFaceCollideAlogrithm::SelectNextFaceBloodMoveTo( int CurrFaceId , 
																	     const float CurrWeights[],
																	     const Ogre::Vector3 &CurrVel,
																	     const Ogre::Vector3 &Gravitydir,
																	     ParticleCollideOrganFaceData & NextFace)
{

	float minangledot = -FLT_MAX;

	std::vector<ParticleCollideOrganFaceData> CollidedFaces;

	//get faces that collided with current particle
	GetCollidedFaces(CurrFaceId , CurrWeights , CollidedFaces);

	//choose "next" face this particle will move to
	bool NextFaceFinded = false;

	for(size_t i = 0 ; i < CollidedFaces.size() ; i++)
	{
		ParticleCollideOrganFaceData FaceCollide = CollidedFaces[i];

		int AdjFaceId = FaceCollide.m_FaceID;

		if(CurrFaceId == AdjFaceId)//skip last face
			continue;

		Ogre::Vector3 CollideFaceVert[3];

		GetBloodFaceVertexPosition(CurrFaceId , CollideFaceVert);

		Ogre::Vector3 side0 = CollideFaceVert[0] - CollideFaceVert[1];//need optimize save once

		Ogre::Vector3 side1 = CollideFaceVert[2] - CollideFaceVert[0];

		Ogre::Vector3 normal = side1.crossProduct(side0);

		normal.normalise();

		Ogre::Vector3 gravitydir = Gravitydir;

		Ogre::Vector3 movedir;//speed or gravity in face 

		if(CurrVel.length() > FLT_EPSILON && normal.dotProduct(gravitydir) < 0)//upward face
		   movedir = CurrVel-CurrVel.dotProduct(normal)*normal;
		else
		   movedir = gravitydir-gravitydir.dotProduct(normal)*normal;

		movedir.normalise();//may be no need

		float * weights = FaceCollide.m_collideWeights;

		int launchvert = -1;

		int launchedge = -1;

		float threshold = 0.001f;

		Ogre::Vector3 correctedmovedir = movedir;

		//the follow code segment will check
		//case 1.if the move start from an face edge and the direction is point to the same side of the opposite vertex
		//then use this move direction 

		//case 2.if the move start from an face edge and the direction is point to the opposite side of the opposite vertex
		//then choose the edge as the move direction 

		//case 3.if we start from an vertex and the move direction is inside the 2 edge incident to this vertex
		//then use this move direction 

		//case 4.if we start from an vertex and the move direction is not inside the 2 edge incident to this vertex
		//we choose the edge with the minimum angle with move direction as direction

		for(int v = 0 ; v < 3 ; v++)
		{
			if(weights[v] < threshold)
			{
				int e0 = (v+1)%3;
				int e1 = (v+2)%3;

				Ogre::Vector3 end0 = CollideFaceVert[e0];
				Ogre::Vector3 end1 = CollideFaceVert[e1];

				bool sameside = isSameSide(end0 , end1, CollideFaceVert[v] , end0+movedir);

				if(sameside == false)
				{
					if(weights[e0] >= 1-threshold)//this mean we start from a vertex: case 4
						correctedmovedir = end1-end0;

					else if(weights[e1] >= 1-threshold)//this mean we start from a vertex: case 4
						correctedmovedir = end0-end1;

					else//ok we start from an edge choose the edge as the move direction:case 2
					{
						correctedmovedir = end1-end0;
						if(correctedmovedir.dotProduct(movedir) < 0)
							correctedmovedir *= -1.0f;
					}
					correctedmovedir.normalise();
				}
			}
		}	

		float angledot = correctedmovedir.dotProduct(movedir);

		if(angledot > minangledot)
		{
			minangledot = angledot;
			NextFace = FaceCollide;
			NextFace.m_FaceNormal = normal;
			NextFaceFinded = true;
		}
	}

	return NextFaceFinded;
}
