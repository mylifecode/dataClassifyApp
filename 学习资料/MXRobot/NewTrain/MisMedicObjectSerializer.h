#ifndef _MISMEDICOBJECTSERIALIZER_
#define _MISMEDICOBJECTSERIALIZER_
#include "Ogre.h"
#include "Math/GoPhysVector3.h"
#include "Memory/GoPhysAlignedObjectArray.h"
using namespace GoPhys;

class MisMedicOrgan_Ordinary;

class MisMedicObjetSerializer
{
public:
	class MisSerialNode
	{
	public:
		MisSerialNode()
		{
			m_NumTexAssigned = 0;
		}

		void AddTexture(const Ogre::Vector2 & text , const GFPhysVector3 & norm)
		{
			if (m_NumTexAssigned < 19)
			{
				m_Textures[m_NumTexAssigned] = text;
				m_Normals[m_NumTexAssigned] = norm;
				m_NumTexAssigned++;
			}
		}
		Ogre::Vector2 m_Textures[20];
		GFPhysVector3 m_Normals[20];
		int m_NumTexAssigned;

	};
	class MisSerialFace
	{
	public:
		static MisSerialFace sInvalidFace;
		
		operator size_t() const;

		bool operator == (const MisSerialFace & rth) const;

		bool operator < (const MisSerialFace & rth) const;

		MisSerialFace(bool isValid = true) : m_IsValid(isValid) {}
		int m_Index[3];
		Ogre::Vector2 m_TextureCoord[3];
		bool m_IsValid;
		bool m_IsCoincideFaces;
	};

	class MisSerialTetra
	{
	public:
		static MisSerialTetra sInvalidTetra;

		MisSerialTetra(bool isValid = true)
		{
			m_unionObjectID = -1;
			//massScale = 1.0f;
			m_IsValid = isValid;
			m_LayerIndex = 0;
			m_IsTextureSetted = false;
			m_IsMenstary = false;
		}
		//float massScale;//default 1
		int m_Index[4];
		Ogre::Vector2 m_TextureCoord[4];
		bool m_IsTextureSetted;

		int  m_unionObjectID;//四面体属于unioned object的哪一个
		bool m_IsMenstary;//是否脂肪
		bool m_IsValid;
		int  m_LayerIndex;//for multi-layer
	};

	class ShareFace
	{
	public:
		ShareFace(int n0, int n1, int n2)
		{
			nodeIndex[0] = n0;
			nodeIndex[1] = n1;
			nodeIndex[2] = n2;
			m_NumTetra = 0;
		}
		void addTetra(int tetraIndex)
		{
			if (m_NumTetra < 2)
			{
				m_NeiborTetras[m_NumTetra] = tetraIndex;
				m_NumTetra++;
			}
			else
			{
				int i = 0;
				int j = i + 1;
			}
		}
		int m_NeiborTetras[2];
		int nodeIndex[3];
		int m_NumTetra;
	};

	class HashSharedFaces
	{
	public:
		HashSharedFaces(int nodeA, int nodeB, int nodeC)
		{
			m_nodeA = nodeA;
			m_nodeB = nodeB;
			m_nodeC = nodeC;

			Order3(m_nodeA, m_nodeB, m_nodeC);
		}

		bool equals(const HashSharedFaces & other) const
		{
			return (m_nodeA == other.m_nodeA && m_nodeB == other.m_nodeB && m_nodeC == other.m_nodeC);
		}
		//to our success
		GP_FORCE_INLINE	size_t getHash()const
		{
#if(GOPHYS_ARCH_TYPE == GOPHYS_ARCHITECTURE_32)
			size_t keyA = (size_t)m_nodeA;
			size_t keyB = (size_t)m_nodeB;
			size_t key = ((keyA << 16) | (keyB & 0xFFFF));
			// Thomas Wang's hash
			key += ~(key << 15);	key ^= (key >> 10);	key += (key << 3);	key ^= (key >> 6);	key += ~(key << 11);	key ^= (key >> 16);
			return key;
#else if(GOPHYS_ARCH_TYPE == GOPHYS_ARCHITECTURE_64)
			size_t keyA = (size_t)m_nodeA;
			size_t keyB = (size_t)m_nodeB;
			size_t key = ((keyA << 32) | (keyB & 0xFFFFFFFF));
			// Thomas Wang's hash
			key += ~(key << 30);	key ^= (key >> 20);	key += (key << 6);	key ^= (key >> 12); key += ~(key << 22);	key ^= (key >> 32);
			return key;
#endif
		}
		int m_nodeA, m_nodeB, m_nodeC;
	};

	MisMedicObjetSerializer();

	~MisMedicObjetSerializer();

	void ReadFromOrganObjectFile(int objID , Ogre::String s3mfile , Ogre::String s4mfile , Ogre::String t2file ,  Ogre::Vector3 offset);
    
    //float GetMergedObjectStiffness(int objID);
    void  Rearrange(const GFPhysVector3& offset);
	
	void  GenerateInnerTexture();

	GFPhysAlignedVectorObj<GFPhysVector3> m_NodeInitPositions;//init node position from file
	GFPhysAlignedVectorObj<MisSerialNode> m_NodeInfos;
	//int m_NodeInitNum;

	std::vector<Ogre::Vector2> m_NodeTexCoord;//node texture coord if generate volume texture coord
	
    std::map<int,GFPhysVector3> m_NodeExpandPositions;

	std::vector<MisSerialFace> m_InitFaces;//init face from file
	//int m_FaceInitNum;

	std::vector<MisSerialTetra> m_InitTetras;//init tetra from file
	//int m_TetraInitNum;

	//std::map<int, float> m_MergedObjectStiffness;

	std::map<int,std::vector<std::pair<std::string,int>>> m_NodeAttributeMap;
	std::map<int,std::vector<std::pair<std::string,int>>> m_FaceAttributeMap;
};
#endif