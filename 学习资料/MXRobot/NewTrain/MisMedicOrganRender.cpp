#include "MisMedicOrganRender.h"


MisMedicOrganRender::OrganRendSection::OrganRendSection(MisMedicOrganRender * rendobj)
{		
	m_RenderObj = rendobj;
	m_SharedIndexData = OGRE_NEW Ogre::IndexData();
	m_IndexCount  = 0;
	m_IndexNeedRecreating = true;
	m_Actived = false;
	m_Visible = true;
}

MisMedicOrganRender::OrganRendSection::~OrganRendSection()
{
	if(m_SharedIndexData)
	{
		OGRE_DELETE m_SharedIndexData;
		m_SharedIndexData = 0;
	}
}

const Ogre::MaterialPtr& MisMedicOrganRender::OrganRendSection::getMaterial(void) const
{
	return mMaterial;
}

void MisMedicOrganRender::OrganRendSection::getRenderOperation(Ogre::RenderOperation& op)
{
	if(m_IndexCount > 0)
	{
		op.indexData = m_SharedIndexData;
		op.useIndexes = true;
	}
	else
	{
		op.indexData = 0;
		op.useIndexes = false;
	}
	op.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
	op.srcRenderable = this;
	op.vertexData = m_RenderObj->m_SharedVertexData;
}

void MisMedicOrganRender::OrganRendSection::getWorldTransforms(Ogre::Matrix4* xform) const
{
	*xform = m_RenderObj->_getParentNodeFullTransform();
}

Ogre::Real MisMedicOrganRender::OrganRendSection::getSquaredViewDepth(const Ogre::Camera* cam) const
{
	//Ogre::AxisAlignedBox mAABB = m_RenderObj->mAABB;
	//Ogre::Vector3 min, max, mid, dist;
	//min = mAABB.getMinimum();
	//max = mAABB.getMaximum();
	//mid = ((max - min) * 0.5) + min;
	Ogre::Vector3 dist = cam->getDerivedPosition() - m_RenderObj->m_Center;

	return dist.squaredLength();
}

const Ogre::LightList& MisMedicOrganRender::OrganRendSection::getLights(void) const
{
	return m_RenderObj->queryLights();
}

void MisMedicOrganRender::OrganRendSection::setMaterialName( const Ogre::String& name, const Ogre::String& groupName)
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


MisMedicOrganRender::MisMedicOrganRender(const Ogre::String & name)
{
	OrganRendSection * OriginlPart = OGRE_NEW_T(OrganRendSection, Ogre::MEMCATEGORY_GENERAL)(this);
	OriginlPart->setMaterialName(name);
	m_OriginlParts.push_back(OriginlPart);

	OrganRendSection * CuttedPart = OGRE_NEW_T(OrganRendSection, Ogre::MEMCATEGORY_GENERAL)(this);
	CuttedPart->setMaterialName(name);
	m_CuttedParts.push_back(CuttedPart);

	m_SharedVertexData = OGRE_NEW Ogre::VertexData();
	m_SharedVertexData->vertexCount = 0;
	m_VertexCount = 0;
	m_BuffersNeedRecreating = true;
	m_VertexDeclDirty = true;
	m_IndexBuffDirty = false;
	mBoundsDirty = true;
	mRadius = 0;
	mAABB.setNull();
}

MisMedicOrganRender::~MisMedicOrganRender()
{
	if(m_SharedVertexData)
	{
		OGRE_DELETE m_SharedVertexData;
		m_SharedVertexData = 0;
	}

	for (size_t c = 0; c < m_OriginlParts.size(); c++)
	{
		OGRE_DELETE_T(m_OriginlParts[c], OrganRendSection, Ogre::MEMCATEGORY_GENERAL);
	}

	for (size_t c = 0; c < m_CuttedParts.size(); c++)
	{
		OGRE_DELETE_T(m_CuttedParts[c], OrganRendSection, Ogre::MEMCATEGORY_GENERAL);
	}
	m_OriginlParts.clear();
	m_CuttedParts.clear();
}

int  MisMedicOrganRender::AddLayer(const Ogre::String & matName)
{
	OrganRendSection * OriginlPart = OGRE_NEW_T(OrganRendSection, Ogre::MEMCATEGORY_GENERAL)(this);
	OriginlPart->setMaterialName(matName);
	m_OriginlParts.push_back(OriginlPart);

	OrganRendSection * CuttedPart = OGRE_NEW_T(OrganRendSection, Ogre::MEMCATEGORY_GENERAL)(this);
	CuttedPart->setMaterialName(matName);
	m_CuttedParts.push_back(CuttedPart);

	return m_OriginlParts.size() - 1;
}

void MisMedicOrganRender::setupVertexDeclaration(void)
{
	if (m_VertexDeclDirty)
	{
		Ogre::VertexDeclaration* decl = m_SharedVertexData->vertexDeclaration;
		decl->removeAllElements();

		size_t offset = 0;
		// Add a description for the buffer of the positions of the vertices
		decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
		offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

		decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);
		offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

		decl->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES,0);
		offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT2);

		decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_TANGENT);
		offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

		decl->addElement(0, offset, Ogre::VET_COLOUR, Ogre::VES_DIFFUSE);
		offset += Ogre::VertexElement::getTypeSize(Ogre::VET_COLOUR);

		m_VertexDeclDirty = false;
	}
}

void MisMedicOrganRender::setupVertexBuffers(void)
{
	setupVertexDeclaration();

	if(m_VertexCount > m_SharedVertexData->vertexCount && m_BuffersNeedRecreating)
	{
		m_SharedVertexData->vertexBufferBinding->unsetAllBindings();
		m_SharedVertexData->vertexCount = m_VertexCount;

		Ogre::HardwareVertexBufferSharedPtr pBuffer =
			Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
			m_SharedVertexData->vertexDeclaration->getVertexSize(0),
			m_VertexCount,
			Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

		// (re)Bind the buffer
		// Any existing buffer will lose its reference count and be destroyed
		m_SharedVertexData->vertexBufferBinding->setBinding(0, pBuffer);
		m_BuffersNeedRecreating = false;
	}

	
}

void MisMedicOrganRender::setupIndexBuffers(void)
{
	for (size_t c = 0; c < m_OriginlParts.size(); c++)
	{
		OrganRendSection * originPart = m_OriginlParts[c];
		
		int currIndexsize = (originPart->m_SharedIndexData->indexBuffer.isNull() ? 0 : originPart->m_SharedIndexData->indexBuffer->getNumIndexes());

		if (originPart->m_IndexCount > currIndexsize && originPart->m_IndexNeedRecreating)//dest index number > 0
		{
			originPart->m_SharedIndexData->indexCount = 0;//wait to be recalculate
			originPart->m_SharedIndexData->indexBuffer.setNull();
			originPart->m_SharedIndexData->indexBuffer = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
				Ogre::HardwareIndexBuffer::IT_16BIT,
				originPart->m_IndexCount, // max we can use
				Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
			// NB we don't set the indexCount on IndexData here since we will
			// probably use less than the maximum number of indices
			originPart->m_IndexNeedRecreating = false;
		}
	}

	for (size_t c = 0; c < m_CuttedParts.size(); c++)
	{
		OrganRendSection * cuttedPart = m_CuttedParts[c];

		int currIndexsize = (cuttedPart->m_SharedIndexData->indexBuffer.isNull() ? 0 : cuttedPart->m_SharedIndexData->indexBuffer->getNumIndexes());

		if (cuttedPart->m_IndexCount > currIndexsize && cuttedPart->m_IndexNeedRecreating)//dest index number > 0
		{
			cuttedPart->m_SharedIndexData->indexCount = 0;//wait to be recalculate
			cuttedPart->m_SharedIndexData->indexBuffer.setNull();
			cuttedPart->m_SharedIndexData->indexBuffer = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
				Ogre::HardwareIndexBuffer::IT_16BIT,
				cuttedPart->m_IndexCount, // max we can use
				Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
			// NB we don't set the indexCount on IndexData here since we will
			// probably use less than the maximum number of indices
			cuttedPart->m_IndexNeedRecreating = false;
		}
	}
}
void MisMedicOrganRender::UpdateVertexBuffer(const std::vector<MMO_Node> & rendVertex,
									         const Ogre::Vector3 & aabbMin,
									         const Ogre::Vector3 & aabbMax)
{
	size_t NeedVertexCount  = rendVertex.size();
	
	if (NeedVertexCount > m_SharedVertexData->vertexCount)
	{
		m_VertexCount = NeedVertexCount;
		m_BuffersNeedRecreating = true;
	}
	
	setupVertexBuffers();
                                              
	Ogre::HardwareVertexBufferSharedPtr pBuffer = m_SharedVertexData->vertexBufferBinding->getBuffer(0);
	
	if (!pBuffer.isNull())
	{
		float * pBufferStart = (float*)pBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD);
		for (size_t v = 0 ; v < rendVertex.size(); v++)
		{
			//position
			*pBufferStart++ = rendVertex[v].m_CurrPos.x;
			*pBufferStart++ = rendVertex[v].m_CurrPos.y;
			*pBufferStart++ = rendVertex[v].m_CurrPos.z;

			//normal
			*pBufferStart++ = rendVertex[v].m_Normal.x;
			*pBufferStart++ = rendVertex[v].m_Normal.y;
			*pBufferStart++ = rendVertex[v].m_Normal.z;

			//texture coordinate
			*pBufferStart++ = rendVertex[v].m_TextureCoord.x;
			*pBufferStart++ = rendVertex[v].m_TextureCoord.y;

			//tangent
			*pBufferStart++   = rendVertex[v].m_Tangent.x;
			*pBufferStart++   = rendVertex[v].m_Tangent.y;
			*pBufferStart++  = rendVertex[v].m_Tangent.z;

			//colour
			*((Ogre::ARGB*)(pBufferStart)) = rendVertex[v].m_Color.getAsARGB();
			pBufferStart = (float*)((Ogre::uint8*)pBufferStart+sizeof(Ogre::ARGB));
		}
		pBuffer->unlock();
	}
	
	m_Center = (aabbMin + aabbMax)*0.5f;

	mAABB.setMinimum(-1000, -1000, -1000);
	mAABB.setMaximum(+1000, +1000, +1000);
}

void MisMedicOrganRender::UpdateIndexBuffer(const GFPhysAlignedVectorObj<MMO_Face> & originFace,
	                                        const GFPhysAlignedVectorObj<MMO_Face> & cuttedFace)
{
	//some face may need twice via inverse order multi-2
	//size_t cNeedIndexCount = cuttedFace.size() * 3 * 2;
	for (size_t c = 0; c < m_OriginlParts.size(); c++)
	{
		m_OriginlParts[c]->m_BuildFacesIndex.clear();
		m_OriginlParts[c]->m_Actived = false;
	
		m_CuttedParts[c]->m_BuildFacesIndex.clear();
		m_CuttedParts[c]->m_Actived = false;
	}
	
	for (size_t c = 0; c < originFace.size(); c++)
	{
		 int layer = originFace[c].m_LayerIndex;
		 m_OriginlParts[layer]->m_BuildFacesIndex.push_back(c);
	}

	for (size_t c = 0; c < cuttedFace.size(); c++)
	{
		 int layer = cuttedFace[c].m_LayerIndex;
		 m_CuttedParts[layer]->m_BuildFacesIndex.push_back(c);
	}
	
	for (size_t c = 0; c < m_OriginlParts.size(); c++)
	{
		//origin part of this layer
		int FaceNum = m_OriginlParts[c]->m_BuildFacesIndex.size();
		if (FaceNum > 0)
		{
			m_OriginlParts[c]->m_Actived = true;
			size_t oNeedIndexCount = FaceNum * 3 * 1.5;//some face may need twice via inverse order multi-2
			if (oNeedIndexCount > m_OriginlParts[c]->m_SharedIndexData->indexCount)
			{
				m_OriginlParts[c]->m_IndexCount = oNeedIndexCount;
				m_OriginlParts[c]->m_IndexNeedRecreating = true;
			}
		}
	
		//cut part of this layer
		FaceNum = m_CuttedParts[c]->m_BuildFacesIndex.size();
		if (FaceNum > 0)
		{
			m_CuttedParts[c]->m_Actived = true;
			size_t cNeedIndexCount = FaceNum * 3 * 1.5;
			if (cNeedIndexCount > m_CuttedParts[c]->m_SharedIndexData->indexCount)
			{
				m_CuttedParts[c]->m_IndexCount = cNeedIndexCount;
				m_CuttedParts[c]->m_IndexNeedRecreating = true;
			}
		}
	}

	setupIndexBuffers();
	
	for (size_t c = 0; c < m_OriginlParts.size(); c++)
	{
		Ogre::HardwareIndexBufferSharedPtr  poIndexBuffer = m_OriginlParts[c]->m_SharedIndexData->indexBuffer;

		Ogre::HardwareIndexBufferSharedPtr  pcIndexBuffer = m_CuttedParts[c]->m_SharedIndexData->indexBuffer;

		if (!poIndexBuffer.isNull())
		{
			Ogre::uint16 * poIndexStart = (Ogre::uint16*)poIndexBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD);
			Ogre::uint16 * oldptr = poIndexStart;
			for (size_t i = 0; i < m_OriginlParts[c]->m_BuildFacesIndex.size(); i++)
			{
				int f = m_OriginlParts[c]->m_BuildFacesIndex[i];

				if (originFace[f].m_physface && originFace[f].m_NeedRend)
				{
					poIndexStart[0] = originFace[f].vi[0];
					poIndexStart[1] = originFace[f].vi[1];
					poIndexStart[2] = originFace[f].vi[2];
					poIndexStart += 3;

					if (originFace[f].m_NeedDoubleFaces)
					{
						poIndexStart[0] = originFace[f].vi[0];
						poIndexStart[1] = originFace[f].vi[2];
						poIndexStart[2] = originFace[f].vi[1];
						poIndexStart += 3;
					}
				}
			}
			poIndexBuffer->unlock();
			m_OriginlParts[c]->m_SharedIndexData->indexCount = poIndexStart - oldptr;
		}

		if (!pcIndexBuffer.isNull())
		{
			Ogre::uint16 * pcIndexStart = (Ogre::uint16*)pcIndexBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD);
			Ogre::uint16 * oldptr = pcIndexStart;
			for (size_t i = 0; i < m_CuttedParts[c]->m_BuildFacesIndex.size();i++)
			{
				int f = m_CuttedParts[c]->m_BuildFacesIndex[i];
				const MMO_Face & face = cuttedFace[f];
				if (face.m_physface)
				{
					pcIndexStart[0] = face.vi[0];
					pcIndexStart[1] = face.vi[1];
					pcIndexStart[2] = face.vi[2];
					pcIndexStart += 3;

					if (face.m_NeedDoubleFaces)
					{
						pcIndexStart[0] = face.vi[0];
						pcIndexStart[1] = face.vi[2];
						pcIndexStart[2] = face.vi[1];
						pcIndexStart += 3;
					}
				}
			}
			pcIndexBuffer->unlock();
			m_CuttedParts[c]->m_SharedIndexData->indexCount = pcIndexStart - oldptr;
		}
	}
	m_IndexBuffDirty = false;
}
// Overridden members follow
void MisMedicOrganRender::_notifyCurrentCamera(Ogre::Camera* cam)
{
	Ogre::MovableObject::_notifyCurrentCamera(cam);
}

Ogre::Real MisMedicOrganRender::getBoundingRadius(void) const
{
	mRadius = Ogre::Math::Sqrt(
		std::max(mAABB.getMinimum().squaredLength(),
		mAABB.getMaximum().squaredLength()));
	return mRadius;
}

const Ogre::AxisAlignedBox& MisMedicOrganRender::getBoundingBox(void) const
{
	return mAABB;
}

//const Ogre::MaterialPtr& getMaterial(void) const;
const Ogre::String& MisMedicOrganRender::getMovableType(void) const
{
	return MisMedicOrganRenderFactory::FACTORY_TYPE_NAME;
}

void MisMedicOrganRender::_updateRenderQueue(Ogre::RenderQueue * queue)
{
	for (size_t c = 0; c < m_OriginlParts.size(); c++)
	{
		if (m_OriginlParts[c]->m_Actived && m_OriginlParts[c]->m_Visible)
		{
			if (mRenderQueuePrioritySet)
				queue->addRenderable(m_OriginlParts[c], mRenderQueueID, queue->getDefaultRenderablePriority() + 1);
			else if (mRenderQueueIDSet)
				queue->addRenderable(m_OriginlParts[c], mRenderQueueID);
			else
				queue->addRenderable(m_OriginlParts[c]);
		}

		if (m_CuttedParts[c]->m_Actived && m_CuttedParts[c]->m_Visible)
		{
			if (mRenderQueuePrioritySet)
				queue->addRenderable(m_CuttedParts[c], mRenderQueueID, queue->getDefaultRenderablePriority() + 1);
			else if (mRenderQueueIDSet)
				queue->addRenderable(m_CuttedParts[c], mRenderQueueID);
			else
				queue->addRenderable(m_CuttedParts[c]);
		}
	}
}

/// @copydoc MovableObject::visitRenderables
void MisMedicOrganRender::visitRenderables(Ogre::Renderable::Visitor* visitor, bool debugRenderables)
{
	for (size_t c = 0; c < m_OriginlParts.size(); c++)
	{
		if (m_OriginlParts[c]->mMaterial.isNull() == false && m_OriginlParts[c]->m_Actived && m_OriginlParts[c]->m_Visible)
		{
			visitor->visit(m_OriginlParts[c], 0, false);
		}
		if (m_CuttedParts[c]->mMaterial.isNull() == false && m_CuttedParts[c]->m_Actived && m_CuttedParts[c]->m_Visible)
		{
			visitor->visit(m_CuttedParts[c], 0, false);
		}
	}
}

void MisMedicOrganRender::setMaterialName(int layer, int rendsection, Ogre::String nameMaterial)
{
	if (rendsection == 0)
	{
		m_OriginlParts[layer]->setMaterialName(nameMaterial);
	}
	else if (rendsection == 1)
	{
		m_CuttedParts[layer]->setMaterialName(nameMaterial);
	}
}

Ogre::String MisMedicOrganRender::GetMaterialName(int layer, int rendsection)
{
	if (rendsection == 0)
	{
		return m_OriginlParts[layer]->getMaterial()->getName();
	}
	else if (rendsection == 1)
	{
		return m_CuttedParts[layer]->getMaterial()->getName();
	}
	return Ogre::String("");
}
void MisMedicOrganRender::SetOriginPartVisibility(bool visible)
{
	for (size_t c = 0; c < m_OriginlParts.size(); c++)
	{
		m_OriginlParts[c]->m_Visible = visible;
	}
}

void MisMedicOrganRender::SetCutPartVisibility(bool visible)
{
	for (size_t c = 0; c < m_CuttedParts.size(); c++)
	{
		m_CuttedParts[c]->m_Visible = visible;
	}
}

void MisMedicOrganRender::DirtyIndexBuffer()
{
	m_IndexBuffDirty = true;
}

bool MisMedicOrganRender::IsIndexBufferDirty()
{
	return m_IndexBuffDirty;
}
//-----------------------------------------------------------------------
Ogre::String MisMedicOrganRenderFactory::FACTORY_TYPE_NAME = "MisMedicOrganRender";
MisMedicOrganRenderFactory* MisMedicOrganRenderFactory::s_dynamicRendfactory = NULL;

MisMedicOrganRenderFactory::~MisMedicOrganRenderFactory()
{
}

const Ogre::String& MisMedicOrganRenderFactory::getType(void) const
{
	return FACTORY_TYPE_NAME;
}

Ogre::MovableObject* MisMedicOrganRenderFactory::createInstanceImpl( const Ogre::String& name,
																		const Ogre::NameValuePairList* params)
{
	return OGRE_NEW MisMedicOrganRender(name);
}

void MisMedicOrganRenderFactory::destroyInstance( Ogre::MovableObject* obj)
{
	OGRE_DELETE  obj;
}

void MisMedicOrganRenderFactory::Initalize()
{
	if(s_dynamicRendfactory == NULL)
	{
		s_dynamicRendfactory = OGRE_NEW MisMedicOrganRenderFactory();
		Ogre::Root::getSingleton().addMovableObjectFactory(s_dynamicRendfactory);
	}
}

void MisMedicOrganRenderFactory::Terminate()
{
	if (s_dynamicRendfactory)
	{
		Ogre::Root::getSingleton().removeMovableObjectFactory(s_dynamicRendfactory);
		s_dynamicRendfactory = NULL;
	}
}