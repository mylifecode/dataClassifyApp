#include <hash_map>
#include "MisOrganAnimation.h"
#include "MisMedicOrganOrdinary.h"
#include "Dynamic/Solver/GoPhysParallelSolver.h"
#include "MXOgreGraphic.h"
static void ExtractPositionFromVertexBuffer( const Ogre::Mesh * mesh,
											 Ogre::HardwareVertexBufferSharedPtr vbuf,
											 size_t &vertex_count,
											 Ogre::Vector3* &vertices,
											 const Ogre::Vector3 & offset
											 )
{
	if(mesh->getNumSubMeshes() != 1)
	   return;

	Ogre::SubMesh* submesh = mesh->getSubMesh(0);

	if(submesh->useSharedVertices)
	{
		vertex_count = mesh->sharedVertexData->vertexCount;
	}
	else
	{
		vertex_count = submesh->vertexData->vertexCount;
	}

	if(vertex_count != vbuf->getNumVertices())
	   return;

	// Allocate space for the vertices and indices
	vertices = new Ogre::Vector3[vertex_count];

	Ogre::VertexData * vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;

	//Read Position
	const Ogre::VertexElement * posElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);

	unsigned char * vertex = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

	float* pReal;

	for( size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
	{
		posElem->baseVertexPointerToElement(vertex, &pReal);
		Ogre::Vector3 pt(pReal[0], pReal[1], pReal[2]);
		vertices[j] = pt+offset ;
	}
	vbuf->unlock();
}
//===============================================================================
MisOrganAnimation::MisOrganAnimation()
{
	m_TimeElapsed = 0;
	m_FrameCurrent = 0;
}
//===============================================================================
MisOrganAnimation::~MisOrganAnimation()
{
	if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
	   PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(this);

	for(size_t f = 0 ; f < m_ResultControlData.size() ; f++)
	{
		delete m_ResultControlData[f];
	}
	m_ResultControlData.clear();
}
//===============================================================================
void MisOrganAnimation::PrepareSolveConstraint(Real Stiffness , Real dt)
{
	/*
	float interval = 0.4f;//modify this to scale or shrink the animation!

	m_TimeElapsed += dt;
	
	int TotalFrame = m_ResultControlData.size();
	
	int frameStart = floor(m_TimeElapsed / interval);
	
	float percent  = (m_TimeElapsed-interval*frameStart) / interval;

	GPClamp(percent , 0.0f , 1.0f);

	frameStart   = frameStart % TotalFrame;

	int FrameEnd = (frameStart+1) % TotalFrame;

	FrameControlData * frame0 = m_ResultControlData[frameStart];
	
	FrameControlData * frame1 = m_ResultControlData[FrameEnd];

	GFPhysParallelPositionConstraintSolver * solver = PhysicsWrapper::GetSingleTon().m_dynamicsWorld->GetParallelPositionContraintSolver();
	
	for(size_t t = 0 ; t < m_AnimatedTetras.size() ; t++)
	{
		float vol0  = m_AnimatedTetras[t].m_RestVol * frame0->m_TetraEnlarge[t];
		float vol1  = m_AnimatedTetras[t].m_RestVol * frame1->m_TetraEnlarge[t];

		float finalVol = vol0 * (1-percent) + vol1 * percent;
		GFPhysParallelSolveUnit * solverunit = solver->GetSolverUnit(m_AnimatedTetras[t].m_PhysTetra);
		solverunit->m_RestValue = finalVol;//
	}

	for(size_t e = 0 ; e < m_AnimatedEdges.size() ; e++)
	{
		float len0  = m_AnimatedEdges[e].m_RestLen * frame0->m_EdgeEnlarge[e];
		float len1  = m_AnimatedEdges[e].m_RestLen * frame1->m_EdgeEnlarge[e];

		float finalLen = len0 * (1-percent) + len1 * percent;
		GFPhysParallelSolveUnit * solverunit = solver->GetSolverUnit(m_AnimatedEdges[e].m_physEdge);
		solverunit->m_RestValue = finalLen;//
	}
	*/
}
//===============================================================================
void MisOrganAnimation::SolveConstraint(Real Stiffness , Real dt)
{
}
//===============================================================================
//void MisOrganAnimation::SetPhysicsFaces(std::vector<GFPhysSoftBodyFace*> & CachedFaces)
//{
//	m_CachedFaces = CachedFaces;
//}
//===============================================================================
void MisOrganAnimation::SerializeFromCookedAnimFile(const Ogre::String & filename , GFPhysSoftBody * DestSb)
{
	//try open file if exists
	Ogre::DataStreamPtr streammms;
	try
	{
		streammms = Ogre::ResourceGroupManager::getSingleton().openResource(filename);
	}
	catch(...)
	{
		streammms.setNull();
	}
	int framecount , tetracount , edgecount;

	Ogre::String linestr = streammms->getLine();
	
	Ogre::vector<Ogre::String>::type tempve = Ogre::StringUtil::split(linestr, " ");

	framecount = Ogre::StringConverter::parseInt(tempve[0]);
	tetracount = Ogre::StringConverter::parseInt(tempve[1]);
	edgecount  = Ogre::StringConverter::parseInt(tempve[2]);

	std::vector<GFPhysSoftBodyNode*> SoftNodes;
	GFPhysSoftBodyNode * Node = DestSb->GetNodeList();
	while(Node)
	{
		SoftNodes.push_back(Node);
		Node = Node->m_Next;
	}

	//read edge
	linestr = streammms->getLine();
	for(int e = 0 ; e < edgecount ; e++)
	{
		linestr = streammms->getLine();
		tempve = Ogre::StringUtil::split(linestr, " ");
		
		int vid0 = Ogre::StringConverter::parseInt(tempve[0]);
		int vid1 = Ogre::StringConverter::parseInt(tempve[1]);
		
		GFPhysSoftBodyEdge * edge = DestSb->GetSoftBodyShape().GetEdge(SoftNodes[vid0] , SoftNodes[vid1]);
		m_AnimatedEdges.push_back(AnimatedEdge(edge));
	}

	//read tetra
	linestr = streammms->getLine();
	for(int t = 0 ; t < tetracount ; t++)
	{
		linestr = streammms->getLine();
		tempve = Ogre::StringUtil::split(linestr, " ");
		
		int vid0 = Ogre::StringConverter::parseInt(tempve[0]);
		int vid1 = Ogre::StringConverter::parseInt(tempve[1]);
		int vid2 = Ogre::StringConverter::parseInt(tempve[2]);
		int vid3 = Ogre::StringConverter::parseInt(tempve[3]);

		GFPhysSoftBodyTetrahedron * tetra = DestSb->GetSoftBodyShape().GetTetrahedron(SoftNodes[vid0] , SoftNodes[vid1] , SoftNodes[vid2] , SoftNodes[vid3]);
		m_AnimatedTetras.push_back(AnimatedTetra(tetra));
	}

	for(int frame = 0 ; frame < framecount ; frame++)
	{
		Ogre::String frameidlinestr = streammms->getLine();
		
		FrameControlData * framectrlData = new FrameControlData();
		
		framectrlData->m_EdgeEnlarge.resize(m_AnimatedEdges.size());
		
		framectrlData->m_TetraEnlarge.resize(m_AnimatedTetras.size());
		
		m_ResultControlData.push_back(framectrlData);

		//volume expand
		for(size_t t = 0 ; t < m_AnimatedTetras.size() ; t++)
		{
			Ogre::String linestr = streammms->getLine();//read face id line

			linestr = streammms->getLine();
			
			Real VolScale = Ogre::StringConverter::parseReal(linestr);
			
			framectrlData->m_TetraEnlarge[t] = VolScale;
		}

		//edge expand
		for(size_t e = 0 ; e < m_AnimatedEdges.size() ; e++)
		{
			Ogre::String linestr = streammms->getLine();//read face id line

			linestr = streammms->getLine();

			Real edgeScale = Ogre::StringConverter::parseReal(linestr);

			framectrlData->m_EdgeEnlarge[e] = edgeScale;
		}
	}
	PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(this);
	if(m_ResultControlData.size() > 20)
	{
		MessageBoxA(0 , "animation frame greater than 20" , "warning!!" , 0);
	}
}
//===============================================================================
void MisOrganAnimation::LoadFromAnimedMesh( Ogre::Mesh * animedMesh , GFPhysSoftBody * DestSb , const Ogre::Vector3 & offset)
{
	int NumAnim = animedMesh->getNumAnimations();

	if(NumAnim > 0)
	{
		Ogre::Animation * animation = animedMesh->getAnimation(0);//get first animation as default animation
		
		Ogre::Animation::VertexTrackList trackList = animation->_getVertexTrackList();

		Ogre::VertexAnimationTrack * track = trackList.begin()->second;
		
		if(track->getAnimationType() == Ogre::VAT_MORPH)
		{
			int kframeCount = track->getNumKeyFrames();
			
			if(kframeCount > 0)//first frame as init frame
			{
				Ogre::VertexMorphKeyFrame * kframe = track->getVertexMorphKeyFrame(0);
				
				Ogre::HardwareVertexBufferSharedPtr FramePosBuf = kframe->getVertexBuffer();

				size_t vertex_count = 0;

				Ogre::Vector3 * InitFrameVertices = 0;

				ExtractPositionFromVertexBuffer( animedMesh , FramePosBuf, vertex_count, InitFrameVertices , offset);
				
				//construct mapping node
				std::vector<GFPhysSoftBodyNode *> PhysNodeArray;
				std::vector<int> PhysNodeMapArray;
				stdext::hash_map<GFPhysSoftBodyNode * , int> PhysNodeIndexHash;

				GFPhysSoftBodyNode * softNode = DestSb->GetNodeList();
				while(softNode)
				{
					GFPhysVector3 softPos = softNode->m_UnDeformedPos;

					int RefIndex = -1;
					
					for(size_t c = 0 ; c < vertex_count ; c++)
					{
						Ogre::Vector3 refpos = InitFrameVertices[c];
						float dist = (OgreToGPVec3(refpos) - softPos).Length();
						if(dist < 0.01f)
						{
							RefIndex = c;
							break;
						}
					}

					PhysNodeIndexHash.insert(std::make_pair(softNode , PhysNodeArray.size()));
					PhysNodeArray.push_back(softNode);
					PhysNodeMapArray.push_back(RefIndex);

					if(RefIndex < 0 && softNode->m_insurface)
					{
						MessageBoxA(0, "mms surface point not match  animation mesh !!" , 0 , 0);
					}
					softNode = softNode->m_Next;
				 }
				 delete []InitFrameVertices;
				 
				 //construct animated edge and tetra
				 GFPhysSoftBodyEdge * edge = DestSb->GetEdgeList();
				 while(edge)
				 {
					 GFPhysSoftBodyNode * n0 = edge->m_Nodes[0];
					 GFPhysSoftBodyNode * n1 = edge->m_Nodes[1];

					 int index0 = PhysNodeIndexHash[n0];
					 int index1 = PhysNodeIndexHash[n1];
					
					 AnimatedEdge animEdge(edge);
					 animEdge.m_NodeIndex[0] = index0;
			         animEdge.m_NodeIndex[1] = index1;
					 m_AnimatedEdges.push_back(animEdge);

					 edge = edge->m_Next;
				 }

				 //GFPhysSoftBodyTetrahedron * tetra = DestSb->GetTetrahedronList();
				 // while(tetra)
				 for(size_t th = 0 ; th < DestSb->GetNumTetrahedron() ; th++)
				 {
					 GFPhysSoftBodyTetrahedron * tetra = DestSb->GetTetrahedronAtIndex(th);

					 GFPhysSoftBodyNode * n0 = tetra->m_TetraNodes[0];
					 GFPhysSoftBodyNode * n1 = tetra->m_TetraNodes[1];
					 GFPhysSoftBodyNode * n2 = tetra->m_TetraNodes[2];
					 GFPhysSoftBodyNode * n3 = tetra->m_TetraNodes[3];

					 int index0 = PhysNodeIndexHash[n0];
					 int index1 = PhysNodeIndexHash[n1];
					 int index2 = PhysNodeIndexHash[n2];
					 int index3 = PhysNodeIndexHash[n3];

					 AnimatedTetra animTetra(tetra);
					 animTetra.m_NodeIndex[0] = index0;
					 animTetra.m_NodeIndex[1] = index1;
					 animTetra.m_NodeIndex[2] = index2;
					 animTetra.m_NodeIndex[3] = index3;
					 m_AnimatedTetras.push_back(animTetra);

					 //tetra = tetra->m_Next;
				 }

				 //now process every frame
				 GFPhysAlignedVectorObj<GFPhysVector3> NodeFramePhysPos;
				 NodeFramePhysPos.resize(PhysNodeArray.size());
				
				 int kframeCount = track->getNumKeyFrames();
				 
				 for(int k = 0 ; k < kframeCount ; k++)
				 {
					 //reset all include inner physics node posision
					 for(size_t c = 0 ; c  < PhysNodeArray.size() ; c++)
					 {
						 NodeFramePhysPos[c]  = PhysNodeArray[c]->m_UnDeformedPos;
					 }

					 FrameControlData * framectrlData = new FrameControlData();
					 framectrlData->m_EdgeEnlarge.resize(m_AnimatedEdges.size());
					 framectrlData->m_TetraEnlarge.resize(m_AnimatedTetras.size());
					 m_ResultControlData.push_back(framectrlData);

					 //retrieve all physics position of this frame
					 Ogre::VertexMorphKeyFrame * kframe = track->getVertexMorphKeyFrame(k);

					 Ogre::HardwareVertexBufferSharedPtr FramePosBuf = kframe->getVertexBuffer();

					 size_t vertex_count = 0;

					 Ogre::Vector3 * vertices = 0;

					 ExtractPositionFromVertexBuffer( animedMesh , FramePosBuf, vertex_count, vertices , offset);

					 for(int n = 0 ; n < NodeFramePhysPos.size() ; n++)
					 {
						 if(PhysNodeMapArray[n] >= 0)//only refresh valid point
						    NodeFramePhysPos[n] = OgreToGPVec3(vertices[PhysNodeMapArray[n]]);
					 }
					 
					 //calculate all Edge Length of this frame
					 for(size_t e = 0 ; e < m_AnimatedEdges.size() ; e++)
					 {
						 GFPhysSoftBodyEdge * edge = m_AnimatedEdges[e].m_physEdge;

						 int index0 = m_AnimatedEdges[e].m_NodeIndex[0];
						 int index1 = m_AnimatedEdges[e].m_NodeIndex[1];

						 GFPhysVector3 pos0 = NodeFramePhysPos[index0];
						 GFPhysVector3 pos1 = NodeFramePhysPos[index1];

						 float CurrLen = (pos0-pos1).Length();
						 float RestLen = edge->m_RestLength;

						 float expand = CurrLen / RestLen;
						 framectrlData->m_EdgeEnlarge[e] = expand;
					 }

					 //calculate all Edge Length of this frame
					 for(size_t e = 0 ; e < m_AnimatedTetras.size() ; e++)
					 {
						 GFPhysSoftBodyTetrahedron * tetra = m_AnimatedTetras[e].m_PhysTetra;

						 int index0 = m_AnimatedTetras[e].m_NodeIndex[0];
						 int index1 = m_AnimatedTetras[e].m_NodeIndex[1];
						 int index2 = m_AnimatedTetras[e].m_NodeIndex[2];
						 int index3 = m_AnimatedTetras[e].m_NodeIndex[3];

						 GFPhysVector3 pos0 = NodeFramePhysPos[index0];
						 GFPhysVector3 pos1 = NodeFramePhysPos[index1];
						 GFPhysVector3 pos2 = NodeFramePhysPos[index2];
						 GFPhysVector3 pos3 = NodeFramePhysPos[index3];

						 float CurrVol = (pos1-pos0).Cross(pos2-pos0).Dot(pos3-pos0) / 6.0f;

						 float RestVol = tetra->m_RestSignedVolume;

						 float expand = CurrVol / RestVol;
					     framectrlData->m_TetraEnlarge[e] = expand;
					 }

					 delete[]vertices;
				 }
				 PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(this);
				 if(m_ResultControlData.size() > 20)
				 {
					 MessageBoxA(0 , "animation frame greater than 20" , "warning!!" , 0);
				 }
				 return;
			}
		}
	}

}