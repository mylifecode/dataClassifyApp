#include "MisMedicObjectSerializer.h"
#include "MisMedicOrganOrdinary.h"
#include "Utility/GoPhysSoftBodyUtil.h"
#include "tinyxml.h"
#include <hash_map>
#include <QDebug>
//========================================================================================================
MisMedicObjetSerializer::MisSerialFace MisMedicObjetSerializer::MisSerialFace::sInvalidFace(false);

MisMedicObjetSerializer::MisSerialTetra MisMedicObjetSerializer::MisSerialTetra::sInvalidTetra(false);


MisMedicObjetSerializer::MisSerialFace::operator size_t() const
{
	return (m_Index[0] + m_Index[1] + m_Index[2]);
}

bool MisMedicObjetSerializer::MisSerialFace::operator == (const MisSerialFace & rth) const 
{
	int myindex[3];
	int otherindex[3];

	myindex[0] = m_Index[0];
	myindex[1] = m_Index[1];
	myindex[2] = m_Index[2];

	otherindex[0] = rth.m_Index[0];
	otherindex[1] = rth.m_Index[1];
	otherindex[2] = rth.m_Index[2];

	GoPhys::Order3(myindex[0] , myindex[1] , myindex[2]);
	GoPhys::Order3(otherindex[0] , otherindex[1] , otherindex[2]);

	if(myindex[0] == otherindex[0] && myindex[1] == otherindex[1] && myindex[2] == otherindex[2])
		return true;
	else
		return false;
}

bool MisMedicObjetSerializer::MisSerialFace::operator < (const MisSerialFace & rth) const 
{
	int myindex[3];
	int otherindex[3];
    
	myindex[0] = m_Index[0];
	myindex[1] = m_Index[1];
	myindex[2] = m_Index[2];

	otherindex[0] = rth.m_Index[0];
	otherindex[1] = rth.m_Index[1];
	otherindex[2] = rth.m_Index[2];

	GoPhys::Order3(myindex[0] , myindex[1] , myindex[2]);
	GoPhys::Order3(otherindex[0] , otherindex[1] , otherindex[2]);

	if(myindex[0] != otherindex[0])
	{
		return myindex[0] < otherindex[0];
	}
	else
	{ 
		if(myindex[1] != otherindex[1])
		{
			return myindex[1] < otherindex[1];
		}
		else
		{
			return myindex[2] < otherindex[2];
		}
	}
}

MisMedicObjetSerializer::MisMedicObjetSerializer()
{
	//m_NodeInitPositions = 0;
	//m_InitFaces = 0;
	//m_InitTetras = 0;
}
MisMedicObjetSerializer::~MisMedicObjetSerializer()
{
	//if(m_NodeInitPositions)
	//{
	//	delete []m_NodeInitPositions;
	//	m_NodeInitPositions = 0;
	//}

	//if(m_InitFaces)
	//{
	//	delete []m_InitFaces;
	//	m_InitFaces = 0;
	//}
//
	//if(m_InitTetras)
	//{
		//delete []m_InitTetras;
		//m_InitTetras = 0;
	//}
}
////float MisMedicObjetSerializer::GetMergedObjectStiffness(int objID)
//{
	//////////////////////////////////////////////////////////////////////////
	//std::map<int, float>::iterator itor = m_MergedObjectStiffness.find(objID);
	//if(itor != m_MergedObjectStiffness.end())
	//   return itor->second;
	//else
	//   return -1.0f;
//}
//========================================================================================================
void MisMedicObjetSerializer::ReadFromOrganObjectFile(int objID , Ogre::String s3mfile , Ogre::String s4mfile , Ogre::String t2file ,  Ogre::Vector3 offset)
{
	//try open mms file if exists
	Ogre::DataStreamPtr streammms;
	try
	{
		Ogre::String newOrganfile = s3mfile;

		std::size_t found = newOrganfile.rfind(".");

		int replen = newOrganfile.length()-found;

		newOrganfile.replace(found , replen,".mms");

		streammms = Ogre::ResourceGroupManager::getSingleton().openResource(newOrganfile);
	}
	catch(...)
	{
		streammms.setNull();
	}

	int pointnum, maxID;
	Ogre::String linestr;

	if(streammms.isNull() == false)
	{
		//read physics position
		linestr = streammms->getLine();
		pointnum = Ogre::StringConverter::parseInt(linestr);

		linestr = streammms->getLine();
		maxID = Ogre::StringConverter::parseInt(linestr);

		//m_NodeInitNum = pointnum;//organobject.m_NodeInitNum = pointnum;
		m_NodeInitPositions.resize(pointnum);// = new GFPhysVector3[m_NodeInitNum];//organobject.m_NodeInitPositions = new GFPhysVector3[organobject.m_NodeInitNum];

		for ( int i = 0; i < pointnum; ++i )
		{
			linestr = streammms->getLine();

			Ogre::vector<Ogre::String>::type tempve = Ogre::StringUtil::split(linestr, " ");
			int tempid = Ogre::StringConverter::parseInt(tempve[0]);
			float x = Ogre::StringConverter::parseReal(tempve[1]);
			float y = Ogre::StringConverter::parseReal(tempve[2]);
			float z = Ogre::StringConverter::parseReal(tempve[3]);
			m_NodeInitPositions[i] = GFPhysVector3(x+offset.x , z+offset.y , -y+offset.z);//organobject.m_NodeInitPositions[i] = GFPhysVector3(x+offset.x , z+offset.y , -y+offset.z);
			if(tempve.size() > 4)
			{
			   float tu = Ogre::StringConverter::parseReal(tempve[4]);
			   float tv = Ogre::StringConverter::parseReal(tempve[5]);
			   m_NodeTexCoord.push_back(Ogre::Vector2(tu , tv));
			}
		}

		//read texture coordinate 
		std::vector<Ogre::Vector2> Texcoords;

		linestr = streammms->getLine();
		int texcoordnum = Ogre::StringConverter::parseInt(linestr);

		linestr = streammms->getLine();
		maxID = Ogre::StringConverter::parseInt(linestr);

		Texcoords.resize(texcoordnum);
		for ( int t = 0 ; t < texcoordnum ; ++t )
		{
			linestr = streammms->getLine();
			Ogre::vector<Ogre::String>::type tempve = Ogre::StringUtil::split(linestr, " ");
			int tempid = Ogre::StringConverter::parseInt(tempve[0]);
			float tx = Ogre::StringConverter::parseReal(tempve[1]);
			float ty = Ogre::StringConverter::parseReal(tempve[2]);
			Texcoords[t] = Ogre::Vector2(tx , ty);
		}

		//read face
		linestr = streammms->getLine();
		int FaceInitNum= Ogre::StringConverter::parseInt(linestr);
		m_InitFaces.resize(FaceInitNum);// = new MisSerialFace[m_FaceInitNum];
		
		stdext::hash_map<MisSerialFace , int> existsFaceMap;//prevent tool generated redundant face
		int realFaceNum = 0;
		for (int i = 0; i < FaceInitNum; ++i)
		{
			MisSerialFace face;

			linestr = streammms->getLine();

			Ogre::vector<Ogre::String>::type tempve = Ogre::StringUtil::split(linestr, " ");

			int a = Ogre::StringConverter::parseInt(tempve[0]);

			int b = Ogre::StringConverter::parseInt(tempve[1]);

			int c = Ogre::StringConverter::parseInt(tempve[2]);

			int ta = Ogre::StringConverter::parseInt(tempve[3]);

			int tb = Ogre::StringConverter::parseInt(tempve[4]);

			int tc = Ogre::StringConverter::parseInt(tempve[5]);

			face.m_Index[0] = a;
			face.m_Index[1] = b;
			face.m_Index[2] = c;

			face.m_TextureCoord[0] = Texcoords[ta];
			face.m_TextureCoord[1] = Texcoords[tb];
			face.m_TextureCoord[2] = Texcoords[tc];

			if(existsFaceMap.find(face) == existsFaceMap.end())
			{
				m_InitFaces[realFaceNum++] = face;
				existsFaceMap.insert(std::make_pair(face , 1));
			}
		}
		m_InitFaces.resize(realFaceNum);
	    //m_FaceInitNum = realFaceNum;


		//read tetra
		linestr = streammms->getLine();
		int TetraInitNum = Ogre::StringConverter::parseInt(linestr);
		m_InitTetras.resize(TetraInitNum);// = new MisSerialTetra[m_TetraInitNum];
		
		for (int i = 0; i < TetraInitNum; ++i)
		{
			MisSerialTetra tera;

			linestr = streammms->getLine();

			Ogre::vector<Ogre::String>::type tempve = Ogre::StringUtil::split(linestr, " ");

			tera.m_Index[0] = Ogre::StringConverter::parseInt(tempve[0]);

			tera.m_Index[1] = Ogre::StringConverter::parseInt(tempve[1]);

			tera.m_Index[2] = Ogre::StringConverter::parseInt(tempve[2]);

			tera.m_Index[3] = Ogre::StringConverter::parseInt(tempve[3]);

			//if(tempve.size() > 4)
			//{
			  // int layer = Ogre::StringConverter::parseInt(tempve[4]);
		      // tera.m_Layer = layer;
			//}
			
			tera.m_unionObjectID = objID;
			m_InitTetras[i] = tera;
		}

		//read attributes ...
		int size = streammms->size() - streammms->tell();
		Ogre::String content(size + 1,0);
		
		if(streammms->read((void*)content.c_str(),size))
		{
			TiXmlDocument doc;
			if(doc.Parse(content.c_str()))
			{
				TiXmlElement * rootElement = doc.RootElement();
				if(rootElement)
				{
					TiXmlNode* pTopNode = rootElement->FirstChild();
					while(pTopNode)
					{
						std::map<int,std::vector<std::pair<std::string,int>>> * pAttributeMap = NULL;
						std::pair<std::string,int> attribute;

						if(pTopNode->Value() == Ogre::String("FaceAttributes"))
							pAttributeMap = &m_FaceAttributeMap;
						else if(pTopNode->Value() == Ogre::String("NodeAttributes"))
							pAttributeMap = &m_NodeAttributeMap;

						if(pAttributeMap)
						{
							TiXmlElement *pAttributeElement = pTopNode->FirstChildElement();
							while(pAttributeElement)
							{
								int id = -1;

								const char * pName = pAttributeElement->Value();
								//id
								pAttributeElement->Attribute("Id",&id);
								//attribute name
								const char * pAttributeName = pAttributeElement->Attribute("AttributeName");
								//attribute value
								const char * pValue = pAttributeElement->Attribute("AttributeValue");

								attribute.first = pAttributeName;
								attribute.second = Ogre::StringConverter::parseInt(pValue);

								if(id != -1)
								{
									(*pAttributeMap)[id].push_back(attribute);
								}

								pAttributeElement = pAttributeElement->NextSiblingElement();
							}
						}

						pTopNode = pTopNode->NextSiblingElement();
					}
				}
			}
		}
	}
	else//open s3m+s4m+t2
	{

	}
}

void MisMedicObjetSerializer::Rearrange(const GFPhysVector3& offset)
{
	for (int t = 0; t < (int)m_NodeInitPositions.size(); t++)
    {
        m_NodeInitPositions[t] += offset;
    }
}


static void GetProjectWeighsInTriangle(const GFPhysVector3 & faceVert0,
	const GFPhysVector3 & faceVert1,
	const GFPhysVector3 & faceVert2,
	const GFPhysVector3 & extpos,
	Real weights[3])
{
	GFPhysVector3 faceNorml = (faceVert1 - faceVert0).Cross(faceVert2 - faceVert0).Normalized();
	GFPhysVector3 prjPos = extpos + faceNorml * (faceVert0 - extpos).Dot(faceNorml);

	CalcBaryCentric(faceVert0,
		faceVert1,
		faceVert2,
		extpos,
		weights[0],
		weights[1],
		weights[2]);
}


void MisMedicObjetSerializer::GenerateInnerTexture()
{
	//class NodeAttribute
	//{
	//public:
		//NodeAttribute()
		//{
		//	m_inSurface = false;
		//}
		//bool m_inSurface;
	//};
	m_NodeInfos.clear();
	m_NodeInfos.resize(m_NodeInitPositions.size());

	GFPhysHashMap<HashSharedFaces, ShareFace> ShareFacesHash;//tetrahedron's faces

	//build share faces
	for(int t = 0; t < m_InitTetras.size(); t++)
	{
		MisMedicObjetSerializer::MisSerialTetra & serialTetra = m_InitTetras[t];

		for (int n = 0; n < 4; n++)
		{
			int n0 = serialTetra.m_Index[n];
			int n1 = serialTetra.m_Index[(n + 1) % 4];
			int n2 = serialTetra.m_Index[(n + 2) % 4];

			HashSharedFaces faceHash(n0, n1, n2);

			ShareFace * shareface = ShareFacesHash.find(faceHash);
			if (shareface == 0)
			{
				ShareFace newface(n0, n1, n2);
				newface.addTetra(t);
				ShareFacesHash.insert(faceHash, newface);
			}
			else
			{
				shareface->addTetra(t);
			}
		}
	}

	//
	std::vector<int> tetraQueue;

	for (int c = 0; c < m_InitFaces.size(); c++)
	{
		MisMedicObjetSerializer::MisSerialFace & serialFace = m_InitFaces[c];

		HashSharedFaces faceHash(serialFace.m_Index[0], serialFace.m_Index[1], serialFace.m_Index[2]);

		ShareFace * shareface = ShareFacesHash.find(faceHash);

		if (serialFace.m_IsCoincideFaces == true)
			continue;
		
		if (shareface->m_NumTetra == 1)
		{
			int tIndex = shareface->m_NeiborTetras[0];

			MisMedicObjetSerializer::MisSerialTetra & serialTetra = m_InitTetras[tIndex];

			for (int n = 0; n < 4; n++)
			{
				if (serialTetra.m_Index[n] == serialFace.m_Index[0])
				{
					serialTetra.m_TextureCoord[n] = serialFace.m_TextureCoord[0];
				}

				else if (serialTetra.m_Index[n] == serialFace.m_Index[1])
				{
					serialTetra.m_TextureCoord[n] = serialFace.m_TextureCoord[1];
				}

				else if (serialTetra.m_Index[n] == serialFace.m_Index[2])
				{
					serialTetra.m_TextureCoord[n] = serialFace.m_TextureCoord[2];
				}
				else
				{
					//project
					int nIndex = serialTetra.m_Index[n];

					GFPhysVector3 nodePos = m_NodeInitPositions[nIndex];
					GFPhysVector3 faceNodePos[3];
					faceNodePos[0] = m_NodeInitPositions[serialFace.m_Index[0]];
					faceNodePos[1] = m_NodeInitPositions[serialFace.m_Index[1]];
					faceNodePos[2] = m_NodeInitPositions[serialFace.m_Index[2]];

					GFPhysVector3 nodeNormal = (faceNodePos[1] - faceNodePos[0]).Cross(faceNodePos[2] - faceNodePos[0]).Normalized();
					if ((nodePos - faceNodePos[0]).Dot(nodeNormal) < 0)
						nodeNormal *= -1.0f;

					
					float maxDot = -FLT_MAX;
					int   useExistIndex = -1;
					for (int c = 0; c < m_NodeInfos[nIndex].m_NumTexAssigned; c++)
					{
						GFPhysVector3 existNorm = m_NodeInfos[nIndex].m_Normals[c];
						if (existNorm.Dot(nodeNormal) > maxDot)
						{
							maxDot = existNorm.Dot(nodeNormal);
							useExistIndex = c;
						}
					}
					if (0)//useExistIndex >= 0 && maxDot > 0)
					{
						serialTetra.m_TextureCoord[n] = m_NodeInfos[nIndex].m_Textures[useExistIndex];
					}
					else
					{

						float weights[3];
						GetProjectWeighsInTriangle(faceNodePos[0],
							faceNodePos[1],
							faceNodePos[2],
							nodePos,
							weights);

						serialTetra.m_TextureCoord[n] = serialFace.m_TextureCoord[0] * weights[0]
							+ serialFace.m_TextureCoord[1] * weights[1]
							+ serialFace.m_TextureCoord[2] * weights[2];

						m_NodeInfos[nIndex].AddTexture(serialTetra.m_TextureCoord[n], nodeNormal);
					}
				}

			}

			serialTetra.m_IsTextureSetted = true;
			tetraQueue.push_back(tIndex);
		}
		else
		{//shouldn't move here
			int i = 0;
			int j = i + 1;
		}
	}

	//process queued
	while (tetraQueue.size() > 0)
	{
		int numt = (int)tetraQueue.size();

		for (int c = 0; c < numt; c++)
		{
			int ParentTetraIndex = tetraQueue[c];

			MisMedicObjetSerializer::MisSerialTetra & ParentTetra = m_InitTetras[ParentTetraIndex];

			MisMedicObjetSerializer::MisSerialTetra * neighborTetra;

			for (int f = 0; f < 4; f++)
			{
				int n0 = ParentTetra.m_Index[f];

				int n1 = ParentTetra.m_Index[(f + 1) % 4];

				int n2 = ParentTetra.m_Index[(f + 2) % 4];

				ShareFace * shareface = ShareFacesHash.find(HashSharedFaces(n0, n1, n2));

				if (shareface->m_NumTetra < 2)//surface
					continue;

				int NeigborTetraIndex = -1;

				if (shareface->m_NeiborTetras[0] == ParentTetraIndex)
					NeigborTetraIndex = shareface->m_NeiborTetras[1];
				else
					NeigborTetraIndex = shareface->m_NeiborTetras[0];

				neighborTetra = &(m_InitTetras[NeigborTetraIndex]);

				if (neighborTetra->m_IsTextureSetted == false)
				{
					GFPhysVector3 faceNodes[3];
					Ogre::Vector2 facetexcoord[3];
					int nodeIndex = -1;
					int cc = 0;
					for (int n = 0; n < 4; n++)
					{
						if (neighborTetra->m_Index[n] == ParentTetra.m_Index[0])
						{
							neighborTetra->m_TextureCoord[n] = ParentTetra.m_TextureCoord[0];

							faceNodes[cc] = m_NodeInitPositions[ParentTetra.m_Index[0]];
							facetexcoord[cc] = ParentTetra.m_TextureCoord[0];

							cc++;
						}
						else if (neighborTetra->m_Index[n] == ParentTetra.m_Index[1])
						{
							neighborTetra->m_TextureCoord[n] = ParentTetra.m_TextureCoord[1];

							faceNodes[cc] = m_NodeInitPositions[ParentTetra.m_Index[1]];

							facetexcoord[cc] = ParentTetra.m_TextureCoord[1];
							cc++;
						}
						else if (neighborTetra->m_Index[n] == ParentTetra.m_Index[2])
						{
							neighborTetra->m_TextureCoord[n] = ParentTetra.m_TextureCoord[2];

							faceNodes[cc] = m_NodeInitPositions[ParentTetra.m_Index[2]];

							facetexcoord[cc] = ParentTetra.m_TextureCoord[2];
							cc++;
						}
						else if (neighborTetra->m_Index[n] == ParentTetra.m_Index[3])
						{
							neighborTetra->m_TextureCoord[n] = ParentTetra.m_TextureCoord[3];

							faceNodes[cc] = m_NodeInitPositions[ParentTetra.m_Index[3]];

							facetexcoord[cc] = ParentTetra.m_TextureCoord[3];
							cc++;
						}
						else
						{
							nodeIndex = n;
						}
					}
					if (nodeIndex >= 0)
					{
						int nodeGlobalIndex = neighborTetra->m_Index[nodeIndex];
						GFPhysVector3 nodePos = m_NodeInitPositions[nodeGlobalIndex];

						GFPhysVector3 nodeNormal = (faceNodes[1] - faceNodes[0]).Cross(faceNodes[2] - faceNodes[0]).Normalized();
						if ((nodePos - faceNodes[0]).Dot(nodeNormal) < 0)
							nodeNormal *= -1.0f;


						float maxDot = -FLT_MAX;
						int   useExistIndex = -1;
						for (int c = 0; c < m_NodeInfos[nodeGlobalIndex].m_NumTexAssigned; c++)
						{
							GFPhysVector3 existNorm = m_NodeInfos[nodeGlobalIndex].m_Normals[c];
							if (existNorm.Dot(nodeNormal) > maxDot)
							{
								maxDot = existNorm.Dot(nodeNormal);
								useExistIndex = c;
							}
						}
						if (0)//useExistIndex >= 0 && maxDot > 0)
						{
							neighborTetra->m_TextureCoord[nodeIndex] = m_NodeInfos[nodeGlobalIndex].m_Textures[useExistIndex];
						}
						else
						{
							float weights[3];
							GetProjectWeighsInTriangle(faceNodes[0],
								faceNodes[1],
								faceNodes[2],
								m_NodeInitPositions[neighborTetra->m_Index[nodeIndex]],
								weights);

							neighborTetra->m_TextureCoord[nodeIndex] = facetexcoord[0] * weights[0] + facetexcoord[1] * weights[1] + facetexcoord[2] * weights[2];
						
							m_NodeInfos[nodeGlobalIndex].AddTexture(neighborTetra->m_TextureCoord[nodeIndex], nodeNormal);
						}
					}

					neighborTetra->m_IsTextureSetted = true;
					tetraQueue.push_back(NeigborTetraIndex);
				}

			}
		}

		tetraQueue.erase(tetraQueue.begin(), tetraQueue.begin() + numt);
	}

}