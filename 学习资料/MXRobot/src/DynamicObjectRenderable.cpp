#include "stdafx.h"
#include <hash_map>
#include "DynamicObjectRenderable.h"
#include "PerformanceDebug.h"

DynamicObjectRenderable::DynamicObjectRenderable(const Ogre::String & name):Ogre::MovableObject(name),
	mVertexDeclDirty(true),
	mBuffersNeedRecreating(true),
	mBoundsDirty(true),
	mIndexContentDirty(true)

{
	mVertexData = OGRE_NEW Ogre::VertexData();

	mIndexData = OGRE_NEW Ogre::IndexData();

	m_VertexCount = 0;

	m_IndexCount  = 0;

	m_MainBody = OGRE_NEW_T(DynamicObjRenerSection, Ogre::MEMCATEGORY_GENERAL)(this);

	m_FaceCutted =  OGRE_NEW_T(DynamicObjRenerSection, Ogre::MEMCATEGORY_GENERAL)(this);

	m_subdivdface =  OGRE_NEW_T(DynamicObjRenerSection, Ogre::MEMCATEGORY_GENERAL)(this);
}

DynamicObjectRenderable::~DynamicObjectRenderable()
{
	if(mVertexData)
	{
		OGRE_DELETE mVertexData;
		mVertexData = 0;
	}

	if(mIndexData)
	{
		OGRE_DELETE mIndexData;
		mIndexData = 0;
	}

	if(m_MainBody)
	{
		OGRE_DELETE_T(m_MainBody, DynamicObjRenerSection, Ogre::MEMCATEGORY_GENERAL);
		m_MainBody = 0;
	}

	if(m_FaceCutted)
	{
		OGRE_DELETE_T(m_FaceCutted, DynamicObjRenerSection, Ogre::MEMCATEGORY_GENERAL);
		m_FaceCutted = 0;
	}

	if(m_subdivdface)
	{
		OGRE_DELETE_T(m_subdivdface, DynamicObjRenerSection, Ogre::MEMCATEGORY_GENERAL);
		m_subdivdface = 0;
	}
}
//-----------------------------------------------------------------------
void DynamicObjectRenderable::_notifyCurrentCamera(Ogre::Camera* cam)
{
	Ogre::MovableObject::_notifyCurrentCamera(cam);
}
//-----------------------------------------------------------------------
Ogre::Real DynamicObjectRenderable::getBoundingRadius(void) const
{
	//mRadius = mAABB.getHalfSize().length();
	float maxx = fabsf(mAABB.getMinimum().x) > fabsf(mAABB.getMaximum().x) ? fabsf(mAABB.getMinimum().x):fabsf(mAABB.getMaximum().x);
	float maxy = fabsf(mAABB.getMinimum().y) > fabsf(mAABB.getMaximum().y) ? fabsf(mAABB.getMinimum().y):fabsf(mAABB.getMaximum().y);
	float maxz = fabsf(mAABB.getMinimum().z) > fabsf(mAABB.getMaximum().z) ? fabsf(mAABB.getMinimum().z):fabsf(mAABB.getMaximum().z);

	return sqrtf(maxx*maxx+maxy*maxy+maxz*maxz);
}
//-----------------------------------------------------------------------
const Ogre::AxisAlignedBox& DynamicObjectRenderable::getBoundingBox(void) const
{
	return mAABB;
}
//-----------------------------------------------------------------------
const Ogre::String& DynamicObjectRenderable::getMovableType(void) const
{
	return DynamicObjectRenderableFactory::FACTORY_TYPE_NAME;
}
//-----------------------------------------------------------------------
void DynamicObjectRenderable::_updateRenderQueue(Ogre::RenderQueue* queue)
{
	if (m_MainBody->m_Actived == true && m_MainBody->m_VertexCount > 0)
	{
		if (mRenderQueuePrioritySet)
			queue->addRenderable(m_MainBody, mRenderQueueID, queue->getDefaultRenderablePriority()+1);
		else if (mRenderQueueIDSet)
			queue->addRenderable(m_MainBody, mRenderQueueID);
		else
			queue->addRenderable(m_MainBody);
	}

	if (m_FaceCutted->m_Actived == true && m_FaceCutted->m_VertexCount > 0)
	{
		if (mRenderQueuePrioritySet)
			queue->addRenderable(m_FaceCutted, mRenderQueueID, queue->getDefaultRenderablePriority()+1);
		else if (mRenderQueueIDSet)
			queue->addRenderable(m_FaceCutted, mRenderQueueID);
		else
			queue->addRenderable(m_FaceCutted);
	}

	if (m_subdivdface->m_Actived == true && m_subdivdface->m_VertexCount > 0)
	{
		if (mRenderQueuePrioritySet)
			queue->addRenderable(m_subdivdface, mRenderQueueID, queue->getDefaultRenderablePriority()+1);
		else if (mRenderQueueIDSet)
			queue->addRenderable(m_subdivdface, mRenderQueueID);
		else
			queue->addRenderable(m_subdivdface);
	}


}
/*
//-----------------------------------------------------------------------
void DynamicObjectRenderable::getRenderOperation(Ogre::RenderOperation& op)
{
	if(m_IndexCount > 0)
	{
		op.indexData = mIndexData;
		op.useIndexes = true;
	}
	else
	{
		op.indexData = 0;
		op.useIndexes = false;
	}
	op.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
	op.srcRenderable = this;
	op.vertexData = mVertexData;
}
*/
/*
//-----------------------------------------------------------------------
void DynamicObjectRenderable::getWorldTransforms(Ogre::Matrix4* xform) const
{
	*xform = _getParentNodeFullTransform();
}
//-----------------------------------------------------------------------
const Ogre::LightList& DynamicObjectRenderable::getLights(void) const
{
	return queryLights();
}
*/
//---------------------------------------------------------------------
void DynamicObjectRenderable::visitRenderables(Ogre::Renderable::Visitor* visitor, 
								 bool debugRenderables)
{
	// only one renderable
	//if (mMaterial.isNull())
		//return;

	if(m_MainBody->mMaterial.isNull() == false)
		visitor->visit(m_MainBody, 0, false);
}

class CtrlEleCachedData
{
public:
	Ogre::Vector3 m_e0;
	Ogre::Vector3 m_e1;
	Ogre::Vector3 m_e2;
	Ogre::Vector3 m_origin;
};

class CtrlEleCacheKey
{
public:
	CtrlEleCacheKey(int v0 , int v1 ,int v2)
	{
		m_v0 = v0;
		m_v1 = v1;
		m_v2 = v2;
	}
	size_t hash() const
	{
		return ((m_v0 & 0xFF) << 24) | ((m_v1 & 0xFFF) << 12) | ((m_v2 & 0xFFF));
	}
	
	bool operator==(const CtrlEleCacheKey & other) const
	{
		return (m_v0 == other.m_v0 && m_v1 == other.m_v1 && m_v2 == other.m_v2);
	}

	bool operator < (const CtrlEleCacheKey & other) const
	{
		if (m_v0 == other.m_v0)
		{
			if (m_v1 == other.m_v1)
				return m_v2 < other.m_v2;
			else
				return m_v1 < other.m_v1;
		}
		else
			return m_v0 < other.m_v0;
	}

	operator size_t() const
	{
		return hash();
	}
	
	int m_v0;
	int m_v1;
	int m_v2;
};

DynamicSurfaceFreeDeformObject::DynamicSurfaceFreeDeformObject(const Ogre::String & name)
		: DynamicObjectRenderable(name)
{

	m_useffd = false;
}
#define MAXMANIPULATEDVERTXNUM 2500
struct TempVertexBTNStruct
{
	   Ogre::Vector3  normal;
	   Ogre::Vector3  tangent;
	   Ogre::Vector3  bionormal;
	   float  btnweight;
};

void DynamicSurfaceFreeDeformObject::LoadFFDFile(const char * filename)
{
		//read from file
		//std::ifstream inputstream;
		//inputstream.open(filename , ios::binary);
		Ogre::DataStreamPtr inputstream;

		inputstream = Ogre::ResourceGroupManager::getSingleton().openResource(filename);

		int manivertposcount;
		inputstream->read(&manivertposcount , sizeof(int));

		int maniverttexcount;
		inputstream->read(&maniverttexcount , sizeof(int));

		int rendvertcount;
		inputstream->read(&rendvertcount , sizeof(int));

		int rendfacecount;
		inputstream->read(&rendfacecount , sizeof(int));

		//read texture coordinate
		for(size_t t = 0 ; t < maniverttexcount ; t++)
		{
				float x , y;
				inputstream->read(&x , sizeof(float));
				inputstream->read(&y , sizeof(float));
				m_texcoords.push_back(Ogre::Vector2(x , y));
		}

		//read rend vertex 
		for(size_t v = 0 ; v < rendvertcount ; v++)
		{
				int posid , texid;
				inputstream->read(&posid , sizeof(int));
				inputstream->read(&texid , sizeof(int));
				RendVertexData rendvertdata;
				rendvertdata.manivertid = posid;
				rendvertdata.texcoorid = texid;
				rendvertdata.texcoord  = m_texcoords[texid];//precache
				m_rendvertexs.push_back(rendvertdata);
		}

		//read face data
		for(size_t f = 0 ; f < rendfacecount ; f++)
		{
				RendFaceData rendface;

				int rid0 , rid1 , rid2;

				inputstream->read(&rid0 , sizeof(int));
				inputstream->read(&rid1 , sizeof(int));
				inputstream->read(&rid2 , sizeof(int));

				rendface.rendvertexindex[0] = rid0;
				rendface.rendvertexindex[1] = rid1;
				rendface.rendvertexindex[2] = rid2;

				rendface.manivertindex[0]  = m_rendvertexs[rid0].manivertid;
				rendface.manivertindex[1]  = m_rendvertexs[rid1].manivertid;
				rendface.manivertindex[2]  = m_rendvertexs[rid2].manivertid;
				
				rendface.texcoordindex[0] = m_rendvertexs[rid0].texcoorid;
				rendface.texcoordindex[1] = m_rendvertexs[rid1].texcoorid;
				rendface.texcoordindex[2] = m_rendvertexs[rid2].texcoorid;
				
				rendface.texcoord[0] = m_texcoords[rendface.texcoordindex[0]];
				rendface.texcoord[1] = m_texcoords[rendface.texcoordindex[1]];
				rendface.texcoord[2] = m_texcoords[rendface.texcoordindex[2]];

				m_rendfaces.push_back(rendface);
		}
		
		//read control face
		int ctrlfacenum;
		inputstream->read(&ctrlfacenum , sizeof(int));

		for (size_t e = 0; e < ctrlfacenum ; e++)
		{
				CtrlFace facectrl;
				inputstream->read(&(facectrl.vid[0]) , sizeof(int));
				inputstream->read(&(facectrl.vid[1]) , sizeof(int));
				inputstream->read(&(facectrl.vid[2]) , sizeof(int));
				m_ctrlfaces.push_back(facectrl);
		}

		for (size_t p = 0 ; p < manivertposcount ; p++)
		{
				int controlelenum;
				inputstream->read(&controlelenum , sizeof(int));

				ManipulatedVertex manivertex;
				manivertex.m_ctrldatavec.resize(controlelenum);
				
				float totalweight = 0;

				for(int e = 0 ; e < controlelenum ; e++)
				{
						ElementCtrlData & eledata = manivertex.m_ctrldatavec[e];

						int ctrlfaceid;

						inputstream->read(&ctrlfaceid , sizeof(int));
						
						eledata.m_ctrlelefaceid = ctrlfaceid;
					
						//closet point coordinate
						inputstream->read(&(eledata.m_closetlocalcoord.x) , sizeof(float));
						inputstream->read(&(eledata.m_closetlocalcoord.y) , sizeof(float));
						inputstream->read(&(eledata.m_closetlocalcoord.z) , sizeof(float));

						//project to closet point coordinate
						inputstream->read(&(eledata.m_p2clocalcoord.x) , sizeof(float));
						inputstream->read(&(eledata.m_p2clocalcoord.y) , sizeof(float));
						eledata.m_p2clocalcoord.z = 0;

						//warp to project
						inputstream->read(&(eledata.m_w2plocalcoord.z) , sizeof(float));
						eledata.m_w2plocalcoord.x = eledata.m_w2plocalcoord.y = 0;

						//write offset dist
						inputstream->read(&eledata.m_p2cdist , sizeof(float));
						inputstream->read(&eledata.m_w2pdist , sizeof(float));
						inputstream->read(&eledata.m_w2cdist , sizeof(float));

						float local = 1.5f;//1.5f;//1.5f;

						float localweight = 1.0f / (1+pow(eledata.m_w2cdist , local));
						eledata.m_weight = localweight;

						totalweight += localweight;
				}
				//calculate weight
				for(int e = 0 ; e < controlelenum ; e++)
				{
					ElementCtrlData & eledata = manivertex.m_ctrldatavec[e];
					eledata.m_weight /= totalweight;
				}
				m_manivertex.push_back(manivertex);
		}
		inputstream->close();
}
/*

void DynamicSurfaceFreeDeformObject::updateS4m(CDynamicObject * dynobj)
{
	   if(m_useffd == true)
	   {
		   updateS4mFFD(dynobj);
	   }
	   else
	   {
		   DynamicObjectRenderable::updateS4m(dynobj);
	   }
}
void DynamicSurfaceFreeDeformObject::updateS4mFFD(CDynamicObject * dynobj)
{
		MXASSERT(dynobj != 0 && dynobj->m_pS4mTriMesh != 0);

		MXASSERT( dynobj->m_vecSurfaceEliminateFaces.size() == 0 );

		MXASSERT( dynobj->m_vecSurfaceSubdivideFaces.size() == 0 );

		Ogre::Vector3 manivertpos[MAXMANIPULATEDVERTXNUM];
		
		TempVertexBTNStruct manivertbtn[MAXMANIPULATEDVERTXNUM];

		dtkPoints::Ptr controlpoints = dynobj->m_pS4mTriMesh->GetPoints(); 
	
		int numctrlpoints = controlpoints->GetNumberOfPoints();

		//update all control element's local coordinate
		for(size_t e = 0; e < m_ctrlfaces.size() ;e++)
		{
				CtrlFace & ctrlface = m_ctrlfaces[e];
				int cvid0 = ctrlface.vid[0];
				int cvid1 = ctrlface.vid[1];
				int cvid2 = ctrlface.vid[2];

				CtrlEleCacheKey ctrlkey(cvid0 , cvid1 , cvid2);
				
				const GK::Point3 ctrlpt0 = controlpoints->GetPoint(cvid0);
				const GK::Point3 ctrlpt1 = controlpoints->GetPoint(cvid1);
				const GK::Point3 ctrlpt2 = controlpoints->GetPoint(cvid2);

				Ogre::Vector3 e0 = Ogre::Vector3(ctrlpt1.x()-ctrlpt0.x() , ctrlpt1.y()-ctrlpt0.y() , ctrlpt1.z()-ctrlpt0.z());
				Ogre::Vector3 e1 = Ogre::Vector3(ctrlpt2.x()-ctrlpt0.x() , ctrlpt2.y()-ctrlpt0.y() , ctrlpt2.z()-ctrlpt0.z());
				Ogre::Vector3 e2 = e0.crossProduct(e1);
				e2.normalise();

				ctrlface.m_e0 = e0;
				ctrlface.m_e1 = e1;
				ctrlface.m_e2 = e2;
				ctrlface.m_origin = Ogre::Vector3(ctrlpt0.x() , ctrlpt0.y() , ctrlpt0.z());
		}
	
		//update all manipulated vertex first
		MXASSERT((int)m_manivertex.size() < MAXMANIPULATEDVERTXNUM);

		for (size_t v = 0 ; v < m_manivertex.size(); v++)
		{
				ManipulatedVertex & vertex = m_manivertex[v];
		
				Ogre::Vector3 finaldeformpos = Ogre::Vector3(0 , 0 , 0);
		
				for (size_t c = 0 ; c < vertex.m_ctrldatavec.size() ; c++)
				{
						ElementCtrlData & ctrldata = vertex.m_ctrldatavec[c];
						
						const CtrlFace & cachectrldata = m_ctrlfaces[ctrldata.m_ctrlelefaceid];

						Ogre::Vector3 closetpointpos = cachectrldata.m_e0*ctrldata.m_closetlocalcoord.x
																	  +cachectrldata.m_e1*ctrldata.m_closetlocalcoord.y
																	  +cachectrldata.m_e2*ctrldata.m_closetlocalcoord.z;
						closetpointpos = closetpointpos+cachectrldata.m_origin;

						Ogre::Vector3 deformedpos;

						if(ctrldata.m_p2cdist < FLT_EPSILON)
						{
								deformedpos = closetpointpos + ctrldata.m_w2plocalcoord.z*cachectrldata.m_e2;//directly add 
						}
						else
						{
								Ogre::Vector3 offsetpc = cachectrldata.m_e0*ctrldata.m_p2clocalcoord.x
																+cachectrldata.m_e1*ctrldata.m_p2clocalcoord.y;//+cachectrldata.m_e2*ctrldata.m_p2clocalcoord.z;
							
								offsetpc.normalise();
								offsetpc = offsetpc*ctrldata.m_p2cdist;

								deformedpos = closetpointpos + offsetpc + ctrldata.m_w2plocalcoord.z*cachectrldata.m_e2;
						}
						
						finaldeformpos += deformedpos*ctrldata.m_weight;
				}
				vertex.m_Currposition = Ogre::Vector3(finaldeformpos.x, finaldeformpos.z ,-finaldeformpos.y);//finaldeformpos;
				manivertpos[v] = Ogre::Vector3(finaldeformpos.x, finaldeformpos.z ,-finaldeformpos.y);
		}


		//update normal binormal tangent
		int btninfosize = sizeof(TempVertexBTNStruct);
		btninfosize *= m_manivertex.size();

		
		memset(&manivertbtn[0] , 0  , btninfosize);

		for(size_t f = 0 ; f < m_rendfaces.size() ; f++)
		{
				 int vid1 = m_rendfaces[f].manivertindex[0];
				 int vid2 = m_rendfaces[f].manivertindex[1];
				 int vid3 = m_rendfaces[f].manivertindex[2];

				 Ogre::Vector3 v1 = manivertpos[vid1];
				 Ogre::Vector3 v2 = manivertpos[vid2];
				 Ogre::Vector3 v3 = manivertpos[vid3];

				 Ogre::Vector3 v21 = v2 - v1;
				 v21.normalise();

				 Ogre::Vector3 v31 = v3 - v1;
				 v31.normalise();

				 Ogre::Vector3 v32 = v3 - v2;
				 v32.normalise();

				 float weightA = 1-(v21.dotProduct(v31));  
				 float weightB = 1-((-v21).dotProduct(v32));  
				 float weightC = 1-((-v31).dotProduct(-v32));  

				// manivertbtn[vid1].btnweight  += weightA;//no use
				// manivertbtn[vid2].btnweight  += weightB;
				// manivertbtn[vid3].btnweight  += weightC;
		 
				 Ogre::Vector3 side0 = v1 - v2;

				 Ogre::Vector3 side1 = v3 - v1;

				 Ogre::Vector3 normal = side1.crossProduct(side0);

				// normal.normalise();

				 manivertbtn[vid3].normal += normal;//*weightC;
				 manivertbtn[vid2].normal += normal;//*weightB;
				 manivertbtn[vid1].normal += normal;//*weightA;

				 //tangent bionormal
				 Ogre::Vector2 uv1 = m_rendfaces[f].texcoord[0];
				 Ogre::Vector2 uv2 = m_rendfaces[f].texcoord[1];
				 Ogre::Vector2 uv3 = m_rendfaces[f].texcoord[2];

				 Ogre::Real deltaV0 = uv1.y - uv2.y;

				 Ogre::Real deltaV1 = uv3.y - uv1.y;

				 Ogre::Real deltaU0 = uv1.x - uv2.x;

				 Ogre::Real deltaU1 = uv3.x - uv1.x;

				 Ogre::Vector3 tangent = deltaV1 * side0 - deltaV0 * side1;
				 tangent.normalise();

				 Ogre::Vector3 binormal = deltaU1 * side0 - deltaU0 * side1;
				 binormal.normalise();

				 Ogre::Vector3 tangentCross = tangent.crossProduct(binormal);
				 if (tangentCross.dotProduct(normal) < 0.0f)
				{
						tangent = -tangent;
						binormal = -binormal;
				}

				 manivertbtn[vid1].bionormal += binormal*weightA;
				 manivertbtn[vid2].bionormal += binormal*weightB;
				 manivertbtn[vid3].bionormal += binormal*weightC;

				 manivertbtn[vid1].tangent += tangent*weightA;
				 manivertbtn[vid2].tangent += tangent*weightB;
				 manivertbtn[vid3].tangent += tangent*weightC;
		}

		for (size_t v  = 0; v < m_manivertex.size(); v++)
		{
				manivertbtn[v].normal.normalise();

				Ogre::Real bionormlength  = manivertbtn[v].bionormal.length();

				Ogre::Real tannormlength  = manivertbtn[v].tangent .length();

				if (bionormlength < 1e-06 || tannormlength < 1e-06)
				{
						manivertbtn[v].bionormal = Ogre::Vector3(1, 0 , 0);
						manivertbtn[v].tangent = Ogre::Vector3(0, 1 , 0);
						manivertbtn[v].normal = Ogre::Vector3(0, 0 , 1);
				}
				else
				{
						bionormlength = 1.0f / bionormlength;
						tannormlength = 1.0f / tannormlength;
						manivertbtn[v].bionormal *= bionormlength;
						manivertbtn[v].tangent *= tannormlength;
				}
				//manivertbtn[v].bionormal = manivertbtn[v].tangent.crossProduct(manivertbtn[v].normal);  
				//manivertbtn[v].bionormal.normalise();
		}

		//construct vertex buffer	
		size_t NeedVertexCount  = m_rendvertexs.size();
		
		size_t NeedIndexCount   = (int)m_rendfaces.size()*3;

		if (NeedVertexCount > m_MainBody->mVertexData->vertexCount)
		{
				m_MainBody->m_VertexCount = NeedVertexCount;
				m_MainBody->mBuffersNeedRecreating = true;
		}

		if (NeedIndexCount > m_MainBody->mIndexData->indexCount)
		{
				m_MainBody->m_IndexCount = NeedIndexCount;
				m_MainBody->mBuffersNeedRecreating = true;
		}

		m_MainBody->setupBuffers();

		Ogre::String strMtrl = dynobj->GetNativeMaterial();

		m_MainBody->setMaterialName(strMtrl);

		m_MainBody->m_Actived = true;

		Ogre::HardwareVertexBufferSharedPtr pBuffer = m_MainBody->mVertexData->vertexBufferBinding->getBuffer(0);
		
		Ogre::HardwareIndexBufferSharedPtr  pIndexBuffer = m_MainBody->mIndexData->indexBuffer;

		try
		{
				float * pBufferStart = (float*)pBuffer->lock(Ogre::HardwareBuffer::HBL_NORMAL);
				
				Ogre::uint16 * pIndexStart = (Ogre::uint16*)pIndexBuffer->lock(Ogre::HardwareBuffer::HBL_NORMAL);
			
				//fill vertex buffer
				for (size_t v = 0 ; v < m_rendvertexs.size(); v++)
				{
							 int vid = m_rendvertexs[v].manivertid;
							 Ogre::Vector2 texcoord = m_rendvertexs[v].texcoord;
						
							//position
							pBufferStart[0] = manivertpos[vid].x;
							pBufferStart[1] = manivertpos[vid].y;
							pBufferStart[2] = manivertpos[vid].z;

							//normal
							pBufferStart[3] = manivertbtn[vid].normal.x;
							pBufferStart[4] = manivertbtn[vid].normal.y;
							pBufferStart[5] = manivertbtn[vid].normal.z;
							
							//texture coordinate
							pBufferStart[6] = texcoord.x;
							pBufferStart[7] = texcoord.y;

							//texture coordinate
							pBufferStart[8]   = manivertbtn[vid].bionormal.x;
							pBufferStart[9]   = manivertbtn[vid].bionormal.y;
							pBufferStart[10] = manivertbtn[vid].bionormal.z;

							//texture coordinate
							pBufferStart[11] = manivertbtn[vid].tangent.x;
							pBufferStart[12] = manivertbtn[vid].tangent.y;
							pBufferStart[13] = manivertbtn[vid].tangent.z;

							pBufferStart += 14;
				}

				//fill index buffer
				for ( size_t f = 0; f < m_rendfaces.size(); f++ )
				{
							pIndexStart[0] = m_rendfaces[f].rendvertexindex[0];
							pIndexStart[1] = m_rendfaces[f].rendvertexindex[1];
							pIndexStart[2] = m_rendfaces[f].rendvertexindex[2];
							pIndexStart += 3;
				}

				pBuffer->unlock();

				pIndexBuffer->unlock();

		}
		catch(...)
		{

		}
}
*/
void DynamicSurfaceFreeDeformObject::InitalizeUndeformedPos(GFPhysSoftBody * sb)
{
	//Ogre::Vector3 manivertpos[MAXMANIPULATEDVERTXNUM];

	std::vector<Ogre::Vector3> controlpoints;

	GFPhysSoftBodyNode * nodelist  = sb->GetNodeList();

	while(nodelist != 0)
	{
		GFPhysVector3 NodePos = nodelist->m_UnDeformedPos;
		controlpoints.push_back(Ogre::Vector3(NodePos.x() , -NodePos.z() , NodePos.y()));
		nodelist = nodelist->m_Next;
	}

	int numctrlpoints = controlpoints.size();

	//update all control element's local coordinate
	int ctrlfacesNum = (int)m_ctrlfaces.size();

	CtrlFace * ctrlfaceArray = &m_ctrlfaces[0];

	for(int e = 0; e < ctrlfacesNum ;e++)
	{
		CtrlFace & ctrlface = ctrlfaceArray[e];//m_ctrlfaces[e];

		int cvid0 = ctrlface.vid[0];
		int cvid1 = ctrlface.vid[1];
		int cvid2 = ctrlface.vid[2];

		const Ogre::Vector3 ctrlpt0 = controlpoints[cvid0];
		const Ogre::Vector3 ctrlpt1 = controlpoints[cvid1];
		const Ogre::Vector3 ctrlpt2 = controlpoints[cvid2];

		Ogre::Vector3 e0 = ctrlpt1-ctrlpt0;//Ogre::Vector3(ctrlpt1.x()-ctrlpt0.x() , ctrlpt1.y()-ctrlpt0.y() , ctrlpt1.z()-ctrlpt0.z());
		Ogre::Vector3 e1 = ctrlpt2-ctrlpt0;//Ogre::Vector3(ctrlpt2.x()-ctrlpt0.x() , ctrlpt2.y()-ctrlpt0.y() , ctrlpt2.z()-ctrlpt0.z());
		Ogre::Vector3 e2 = e0.crossProduct(e1);
		e2.normalise();

		ctrlface.m_e0 = e0;
		ctrlface.m_e1 = e1;
		ctrlface.m_e2 = e2;
		ctrlface.m_origin = Ogre::Vector3(ctrlpt0.x , ctrlpt0.y , ctrlpt0.z);
	}

	//update all manipulated vertex first
	int manivertexCount = (int)m_manivertex.size();
	SY_ASSERT(manivertexCount < MAXMANIPULATEDVERTXNUM);

	ManipulatedVertex * maniVertexArray = &m_manivertex[0];

	//#pragma omp parallel for
	for (int v = 0 ; v < manivertexCount; v++)
	{
		ManipulatedVertex & vertex = maniVertexArray[v];//m_manivertex[v];

		Ogre::Vector3 finaldeformpos = Ogre::Vector3(0 , 0 , 0);

		for (size_t c = 0 ; c < vertex.m_ctrldatavec.size() ; c++)
		{
			ElementCtrlData & ctrldata = vertex.m_ctrldatavec[c];

			const CtrlFace & cachectrldata = m_ctrlfaces[ctrldata.m_ctrlelefaceid];

			Ogre::Vector3 closetpointpos = cachectrldata.m_e0*ctrldata.m_closetlocalcoord.x
				+cachectrldata.m_e1*ctrldata.m_closetlocalcoord.y
				+cachectrldata.m_e2*ctrldata.m_closetlocalcoord.z;
			closetpointpos = closetpointpos+cachectrldata.m_origin;

			Ogre::Vector3 deformedpos;

			if(ctrldata.m_p2cdist < FLT_EPSILON)
			{
				deformedpos = closetpointpos + ctrldata.m_w2plocalcoord.z*cachectrldata.m_e2;//directly add 
			}
			else
			{
				Ogre::Vector3 offsetpc = cachectrldata.m_e0*ctrldata.m_p2clocalcoord.x
					+cachectrldata.m_e1*ctrldata.m_p2clocalcoord.y;//+cachectrldata.m_e2*ctrldata.m_p2clocalcoord.z;

				offsetpc.normalise();
				offsetpc = offsetpc*ctrldata.m_p2cdist;

				deformedpos = closetpointpos + offsetpc + ctrldata.m_w2plocalcoord.z*cachectrldata.m_e2;
			}

			finaldeformpos += deformedpos*ctrldata.m_weight;
		}
		vertex.m_UnDeformedPos = Ogre::Vector3(finaldeformpos.x, finaldeformpos.z ,-finaldeformpos.y);
	}
}
void DynamicSurfaceFreeDeformObject::updateS4mFFD(GFPhysSoftBody * sb, Ogre::String matName)
{
		//MXASSERT(dynobj != 0 && dynobj->m_pS4mTriMesh != 0);

		//MXASSERT( dynobj->m_vecSurfaceEliminateFaces.size() == 0 );

		//MXASSERT( dynobj->m_vecSurfaceSubdivideFaces.size() == 0 );

		Ogre::Vector3 manivertpos[MAXMANIPULATEDVERTXNUM];
		
		TempVertexBTNStruct manivertbtn[MAXMANIPULATEDVERTXNUM];

		static std::vector<Ogre::Vector3> controlpoints;
		controlpoints.clear();

		GFPhysSoftBodyNode * nodelist  = sb->GetNodeList();

		while(nodelist != 0)
		{
			GFPhysVector3 NodePos = nodelist->m_CurrPosition;
			controlpoints.push_back(Ogre::Vector3(NodePos.x() , -NodePos.z() , NodePos.y()));
			nodelist = nodelist->m_Next;
		}

		int numctrlpoints = controlpoints.size();

		//update all control element's local coordinate
		int ctrlfacesNum = (int)m_ctrlfaces.size();

		CtrlFace * ctrlfaceArray = &m_ctrlfaces[0];
		
		for(int e = 0; e < ctrlfacesNum ;e++)
		{
				CtrlFace & ctrlface = ctrlfaceArray[e];//m_ctrlfaces[e];
				
				int cvid0 = ctrlface.vid[0];
				int cvid1 = ctrlface.vid[1];
				int cvid2 = ctrlface.vid[2];
	
				const Ogre::Vector3 ctrlpt0 = controlpoints[cvid0];
				const Ogre::Vector3 ctrlpt1 = controlpoints[cvid1];
				const Ogre::Vector3 ctrlpt2 = controlpoints[cvid2];

				Ogre::Vector3 e0 = ctrlpt1-ctrlpt0;//Ogre::Vector3(ctrlpt1.x()-ctrlpt0.x() , ctrlpt1.y()-ctrlpt0.y() , ctrlpt1.z()-ctrlpt0.z());
				Ogre::Vector3 e1 = ctrlpt2-ctrlpt0;//Ogre::Vector3(ctrlpt2.x()-ctrlpt0.x() , ctrlpt2.y()-ctrlpt0.y() , ctrlpt2.z()-ctrlpt0.z());
				Ogre::Vector3 e2 = e0.crossProduct(e1);
				e2.normalise();

				ctrlface.m_e0 = e0;
				ctrlface.m_e1 = e1;
				ctrlface.m_e2 = e2;
				ctrlface.m_origin = Ogre::Vector3(ctrlpt0.x , ctrlpt0.y , ctrlpt0.z);
		}
	
		//update all manipulated vertex first
		int manivertexCount = (int)m_manivertex.size();
		SY_ASSERT(manivertexCount < MAXMANIPULATEDVERTXNUM);

		ManipulatedVertex * maniVertexArray = &m_manivertex[0];
		
	//#pragma omp parallel for
		for (int v = 0 ; v < manivertexCount; v++)
		{
				ManipulatedVertex & vertex = maniVertexArray[v];//m_manivertex[v];
		
				Ogre::Vector3 finaldeformpos = Ogre::Vector3(0 , 0 , 0);
		
				for (size_t c = 0 ; c < vertex.m_ctrldatavec.size() ; c++)
				{
						ElementCtrlData & ctrldata = vertex.m_ctrldatavec[c];
						
						const CtrlFace & cachectrldata = m_ctrlfaces[ctrldata.m_ctrlelefaceid];

						Ogre::Vector3 closetpointpos = cachectrldata.m_e0*ctrldata.m_closetlocalcoord.x
																	  +cachectrldata.m_e1*ctrldata.m_closetlocalcoord.y
																	  +cachectrldata.m_e2*ctrldata.m_closetlocalcoord.z;
						closetpointpos = closetpointpos+cachectrldata.m_origin;

						Ogre::Vector3 deformedpos;

						if(ctrldata.m_p2cdist < FLT_EPSILON)
						{
								deformedpos = closetpointpos + ctrldata.m_w2plocalcoord.z*cachectrldata.m_e2;//directly add 
						}
						else
						{
								Ogre::Vector3 offsetpc = cachectrldata.m_e0*ctrldata.m_p2clocalcoord.x
																+cachectrldata.m_e1*ctrldata.m_p2clocalcoord.y;//+cachectrldata.m_e2*ctrldata.m_p2clocalcoord.z;
							
								offsetpc.normalise();
								offsetpc = offsetpc*ctrldata.m_p2cdist;

								deformedpos = closetpointpos + offsetpc + ctrldata.m_w2plocalcoord.z*cachectrldata.m_e2;
						}
						
						finaldeformpos += deformedpos*ctrldata.m_weight;
				}
				vertex.m_Currposition = finaldeformpos;
				manivertpos[v] = Ogre::Vector3(finaldeformpos.x, finaldeformpos.z ,-finaldeformpos.y);
		}


		//update normal binormal tangent
		int btninfosize = sizeof(TempVertexBTNStruct);
		btninfosize *= m_manivertex.size();

		/*zero btn's first*/
		memset(&manivertbtn[0] , 0  , btninfosize);

		for(size_t f = 0 ; f < m_rendfaces.size() ; f++)
		{
				 int vid1 = m_rendfaces[f].manivertindex[0];
				 int vid2 = m_rendfaces[f].manivertindex[1];
				 int vid3 = m_rendfaces[f].manivertindex[2];

				 Ogre::Vector3 v1 = manivertpos[vid1];
				 Ogre::Vector3 v2 = manivertpos[vid2];
				 Ogre::Vector3 v3 = manivertpos[vid3];

				 Ogre::Vector3 v21 = v2 - v1;
				 v21.normalise();

				 Ogre::Vector3 v31 = v3 - v1;
				 v31.normalise();

				 Ogre::Vector3 v32 = v3 - v2;
				 v32.normalise();

				 float weightA = 1-(v21.dotProduct(v31));  
				 float weightB = 1-((-v21).dotProduct(v32));  
				 float weightC = 1-((-v31).dotProduct(-v32));  

				// manivertbtn[vid1].btnweight  += weightA;//no use
				// manivertbtn[vid2].btnweight  += weightB;
				// manivertbtn[vid3].btnweight  += weightC;
		 
				 Ogre::Vector3 side0 = v1 - v2;

				 Ogre::Vector3 side1 = v3 - v1;

				 Ogre::Vector3 normal = side1.crossProduct(side0);

				// normal.normalise();

				 manivertbtn[vid3].normal += normal;//*weightC;
				 manivertbtn[vid2].normal += normal;//*weightB;
				 manivertbtn[vid1].normal += normal;//*weightA;

				 //tangent bionormal
				 Ogre::Vector2 uv1 = m_rendfaces[f].texcoord[0];
				 Ogre::Vector2 uv2 = m_rendfaces[f].texcoord[1];
				 Ogre::Vector2 uv3 = m_rendfaces[f].texcoord[2];

				 Ogre::Real deltaV0 = uv1.y - uv2.y;

				 Ogre::Real deltaV1 = uv3.y - uv1.y;

				 Ogre::Real deltaU0 = uv1.x - uv2.x;

				 Ogre::Real deltaU1 = uv3.x - uv1.x;

				 Ogre::Vector3 tangent = deltaV1 * side0 - deltaV0 * side1;
				 tangent.normalise();

				 Ogre::Vector3 binormal = deltaU1 * side0 - deltaU0 * side1;
				 binormal.normalise();

				 Ogre::Vector3 tangentCross = tangent.crossProduct(binormal);
				 if (tangentCross.dotProduct(normal) < 0.0f)
				{
						tangent = -tangent;
						binormal = -binormal;
				}

				 manivertbtn[vid1].bionormal += binormal*weightA;
				 manivertbtn[vid2].bionormal += binormal*weightB;
				 manivertbtn[vid3].bionormal += binormal*weightC;

				 manivertbtn[vid1].tangent += tangent*weightA;
				 manivertbtn[vid2].tangent += tangent*weightB;
				 manivertbtn[vid3].tangent += tangent*weightC;
		}

		for (size_t v  = 0; v < m_manivertex.size(); v++)
		{
				manivertbtn[v].normal.normalise();

				Ogre::Real bionormlength  = manivertbtn[v].bionormal.length();

				Ogre::Real tannormlength  = manivertbtn[v].tangent .length();

				if (bionormlength < 1e-06 || tannormlength < 1e-06)
				{
						manivertbtn[v].bionormal = Ogre::Vector3(1, 0 , 0);
						manivertbtn[v].tangent = Ogre::Vector3(0, 1 , 0);
						manivertbtn[v].normal = Ogre::Vector3(0, 0 , 1);
				}
				else
				{
						bionormlength = 1.0f / bionormlength;
						tannormlength = 1.0f / tannormlength;
						manivertbtn[v].bionormal *= bionormlength;
						manivertbtn[v].tangent *= tannormlength;
				}
				//manivertbtn[v].bionormal = manivertbtn[v].tangent.crossProduct(manivertbtn[v].normal);  
				//manivertbtn[v].bionormal.normalise();
		}

		//construct vertex buffer	
		size_t NeedVertexCount  = m_rendvertexs.size();
		
		size_t NeedIndexCount   = (int)m_rendfaces.size()*3;

		if (NeedVertexCount > m_MainBody->mVertexData->vertexCount)
		{
				m_MainBody->m_VertexCount = NeedVertexCount;
				m_MainBody->mBuffersNeedRecreating = true;
		}

		if (NeedIndexCount > m_MainBody->mIndexData->indexCount)
		{
				m_MainBody->m_IndexCount = NeedIndexCount;
				m_MainBody->mBuffersNeedRecreating = true;
		}

		m_MainBody->setupBuffers();

		Ogre::String strMtrl = matName;//dynobj->GetNativeMaterial();

		m_MainBody->setMaterialName(strMtrl);

		m_MainBody->m_Actived = true;

		Ogre::HardwareVertexBufferSharedPtr pBuffer = m_MainBody->mVertexData->vertexBufferBinding->getBuffer(0);
		
		Ogre::HardwareIndexBufferSharedPtr  pIndexBuffer = m_MainBody->mIndexData->indexBuffer;

		try
		{
				float * pBufferStart = (float*)pBuffer->lock(Ogre::HardwareBuffer::HBL_NORMAL);
				
				Ogre::uint16 * pIndexStart = (Ogre::uint16*)pIndexBuffer->lock(Ogre::HardwareBuffer::HBL_NORMAL);
			
				//fill vertex buffer
				for (size_t v = 0 ; v < m_rendvertexs.size(); v++)
				{
							 int vid = m_rendvertexs[v].manivertid;
							 Ogre::Vector2 texcoord = m_rendvertexs[v].texcoord;
						
							//position
							pBufferStart[0] = manivertpos[vid].x;
							pBufferStart[1] = manivertpos[vid].y;
							pBufferStart[2] = manivertpos[vid].z;

							//normal
							pBufferStart[3] = manivertbtn[vid].normal.x;
							pBufferStart[4] = manivertbtn[vid].normal.y;
							pBufferStart[5] = manivertbtn[vid].normal.z;
							
							//texture coordinate
							pBufferStart[6] = texcoord.x;
							pBufferStart[7] = texcoord.y;

							//texture coordinate
							pBufferStart[8]   = manivertbtn[vid].bionormal.x;
							pBufferStart[9]   = manivertbtn[vid].bionormal.y;
							pBufferStart[10] = manivertbtn[vid].bionormal.z;

							//texture coordinate
							pBufferStart[11] = manivertbtn[vid].tangent.x;
							pBufferStart[12] = manivertbtn[vid].tangent.y;
							pBufferStart[13] = manivertbtn[vid].tangent.z;

							pBufferStart += 14;
				}

				//fill index buffer
				for ( size_t f = 0; f < m_rendfaces.size(); f++ )
				{
							pIndexStart[0] = m_rendfaces[f].rendvertexindex[0];
							pIndexStart[1] = m_rendfaces[f].rendvertexindex[1];
							pIndexStart[2] = m_rendfaces[f].rendvertexindex[2];
							pIndexStart += 3;
				}

				pBuffer->unlock();

				pIndexBuffer->unlock();

		}
		catch(...)
		{

		}

		GFPhysTransform identity;
		identity.SetIdentity();
		GFPhysVector3 aabbmin;
		GFPhysVector3 aabbmax;
		sb->GetCollisionShape()->GetAabb(identity , aabbmin , aabbmax);

		mAABB.setMinimum(aabbmin.x() , aabbmin.y() , aabbmin.z());
		mAABB.setMaximum(aabbmax.x() , aabbmax.y() , aabbmax.z());

		//make update bound box
		if (mParentNode)
			mParentNode->needUpdate();
}
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
Ogre::String DynamicObjectRenderableFactory::FACTORY_TYPE_NAME = "DynamicObjRendable";

DynamicObjectRenderableFactory::~DynamicObjectRenderableFactory()
{

}
//-----------------------------------------------------------------------
const Ogre::String& DynamicObjectRenderableFactory::getType(void) const
{
	return FACTORY_TYPE_NAME;
}
//-----------------------------------------------------------------------
Ogre::MovableObject* DynamicObjectRenderableFactory::createInstanceImpl( const Ogre::String& name,
														  const Ogre::NameValuePairList* params)
{

	return OGRE_NEW DynamicSurfaceFreeDeformObject(name);

}
//-----------------------------------------------------------------------
void DynamicObjectRenderableFactory::destroyInstance( Ogre::MovableObject* obj)
{
	OGRE_DELETE  obj;
}

static DynamicObjectRenderableFactory * s_dynamicRendfactory = NULL;

void DynamicObjectRenderableFactory::Initalize()
{
	if(s_dynamicRendfactory == NULL)
	{
		s_dynamicRendfactory = OGRE_NEW DynamicObjectRenderableFactory();
		Ogre::Root::getSingleton().addMovableObjectFactory(s_dynamicRendfactory);
	}

}

void DynamicObjectRenderableFactory::Terminate()
{
	if (s_dynamicRendfactory)
	{
		Ogre::Root::getSingleton().removeMovableObjectFactory(s_dynamicRendfactory);
		s_dynamicRendfactory = NULL;
	}
}

DynamicObjectRenderable::DynamicObjRenerSection::DynamicObjRenerSection(DynamicObjectRenderable*movobj)
		: mVertexDeclDirty(true),
		  mBuffersNeedRecreating(true)
{
	m_moveableobj = movobj;
	
	mVertexData = OGRE_NEW Ogre::VertexData();

	mIndexData = OGRE_NEW Ogre::IndexData();

	m_VertexCount = 0;

	m_IndexCount  = 0;

	m_Actived = false;
}
DynamicObjectRenderable::DynamicObjRenerSection::~DynamicObjRenerSection()
{
	if(mVertexData)
	{
		OGRE_DELETE mVertexData;
		mVertexData = 0;
	}

	if(mIndexData)
	{
		OGRE_DELETE mIndexData;
		mIndexData = 0;
	}
}

//-----------------------------------------------------------------------
void DynamicObjectRenderable::DynamicObjRenerSection::setupVertexDeclaration(void)
{
	if (mVertexDeclDirty)
	{
		Ogre::VertexDeclaration* decl = mVertexData->vertexDeclaration;
		decl->removeAllElements();

		size_t offset = 0;
		// Add a description for the buffer of the positions of the vertices
		decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
		offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

		decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);
		offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

		decl->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES, 0);
		offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT2);

		decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_TEXTURE_COORDINATES, 1);
		offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

		decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_TEXTURE_COORDINATES, 2);
		offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

		mVertexDeclDirty = false;
	}
}
//-----------------------------------------------------------------------
void DynamicObjectRenderable::DynamicObjRenerSection::setupBuffers(void)
{
	setupVertexDeclaration();

	if (mBuffersNeedRecreating)
	{
		mIndexData->indexBuffer.setNull();

		mVertexData->vertexBufferBinding->unsetAllBindings();
		mVertexData->vertexBufferBinding->setBinding(0, Ogre::HardwareVertexBufferSharedPtr());

		if(m_VertexCount > 0)//dest vertex number > 0 
		{
				mVertexData->vertexCount = m_VertexCount;

				mIndexData->indexCount = 0;

				Ogre::HardwareVertexBufferSharedPtr pBuffer =
					Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
					mVertexData->vertexDeclaration->getVertexSize(0),
					mVertexData->vertexCount,
					Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

					// (re)Bind the buffer
					// Any existing buffer will lose its reference count and be destroyed
				mVertexData->vertexBufferBinding->setBinding(0, pBuffer);
		}
		if(m_IndexCount > 0)//dest index number > 0
		{
				mIndexData->indexCount = m_IndexCount;
				
				mIndexData->indexBuffer = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
					Ogre::HardwareIndexBuffer::IT_16BIT,
					m_IndexCount, // max we can use
					Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
					// NB we don't set the indexCount on IndexData here since we will
					// probably use less than the maximum number of indices
				
		}
		mBuffersNeedRecreating = false;
	}
}

void DynamicObjectRenderable::DynamicObjRenerSection::getRenderOperation(Ogre::RenderOperation& op)
{
	if(m_IndexCount > 0)
	{
		op.indexData = mIndexData;
		op.useIndexes = true;
	}
	else
	{
		op.indexData = 0;
		op.useIndexes = false;
	}
	op.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
	op.srcRenderable = this;
	op.vertexData = mVertexData;
}

void DynamicObjectRenderable::DynamicObjRenerSection::getWorldTransforms(Ogre::Matrix4* xform) const
{
	*xform = m_moveableobj->_getParentNodeFullTransform();
}

Ogre::Real DynamicObjectRenderable::DynamicObjRenerSection::getSquaredViewDepth(const Ogre::Camera* cam) const
{
	Ogre::AxisAlignedBox mAABB = m_moveableobj->mAABB;
	Ogre::Vector3 min, max, mid, dist;
	min = mAABB.getMinimum();
	max = mAABB.getMaximum();
	mid = ((max - min) * 0.5) + min;
	dist = cam->getDerivedPosition() - mid;

	return dist.squaredLength();
}

const Ogre::LightList& DynamicObjectRenderable::DynamicObjRenerSection::getLights(void) const
{
	return m_moveableobj->queryLights();
}

const Ogre::MaterialPtr& DynamicObjectRenderable::DynamicObjRenerSection::getMaterial(void) const
{
	return mMaterial;
}
//-----------------------------------------------------------------------
void DynamicObjectRenderable::DynamicObjRenerSection::setMaterialName( const Ogre::String& name, const Ogre::String& groupName /* = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME */)
{
	mMaterialName = name;
	mMaterial = Ogre::MaterialManager::getSingleton().getByName(mMaterialName, groupName);

	if (mMaterial.isNull())
	{
		Ogre::LogManager::getSingleton().logMessage("Can't2 assign material " + name +
			" because this "
			"Material does not exist. Have you forgotten to define it in a "
			".material script?");
		mMaterial = Ogre::MaterialManager::getSingleton().getByName("BaseWhiteNoLighting");
		if (mMaterial.isNull())
		{
			OGRE_EXCEPT(Ogre::Exception::ERR_INTERNAL_ERROR, "Can't assign default material "
				"to BillboardChain of Did "
				"you forget to call MaterialManager::initialise()?",
				"BillboardChain.setMaterialName");
		}
	}
	// Ensure new material loaded (will not load again if already loaded)
	mMaterial->load();
}