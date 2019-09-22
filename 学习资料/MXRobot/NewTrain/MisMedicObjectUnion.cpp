#include "MisMedicObjectUnion.h"
#include "CallBackForUnion.h"
#include "PhysicsWrapper.h"
#include "MisMedicOrganOrdinary.h"
#include "MisMedicObjectSerializer.h"
#include "CustomConstraint.h"
#include "MisNewTraining.h"
#include "XMLWrapperAdhere.h"
#include "Math/GoPhysTransformUtil.h"
#define CHECKUNIONEDFACECONSIST 1

using namespace GoPhys;

void MisMedicObjectUnion::CreateMergedObject(MisMedicObjetSerializer & DstSerializer , 
											  int ObjAID,
											  int ObjBID,
											  Ogre::String fileA ,
											  Ogre::String fileB ,
											  Ogre::Vector3 offset ,
											  float NodeThresHold ,
											  const std::vector<int> & fixindexA,
											  const std::vector<int> & fixindexB,
											  std::vector<int> & dstfixindex)
{
	MisMedicObjetSerializer serializeA , serializeB;
	serializeA.ReadFromOrganObjectFile(ObjAID , fileA , "" ,  "" ,  offset);
	serializeB.ReadFromOrganObjectFile(ObjBID , fileB , "" ,  "" ,  offset );
	
	GFPhysVector3 Extend(NodeThresHold , NodeThresHold , NodeThresHold);
	GFPhysDBVTree NodeTreeA;
	GFPhysDBVTree NodeTreeB;

	//insert undeformed aabb to treeA
	for(int n = 0 ; n < (int)serializeA.m_NodeInitPositions.size() ; n++)
	{
		GFPhysVector3 NodePos = serializeA.m_NodeInitPositions[n];
		GFPhysDBVNode * dbvn = NodeTreeA.InsertAABBNode(NodePos-Extend , NodePos+Extend);
		dbvn->m_UserData = (void*)n;
	}

	//insert undeformed aabb to treeB
	for (int n = 0; n < serializeB.m_NodeInitPositions.size(); n++)
	{
		GFPhysVector3 NodePos = serializeB.m_NodeInitPositions[n];
		GFPhysDBVNode * dbvn = NodeTreeB.InsertAABBNode(NodePos-Extend , NodePos+Extend);
		dbvn->m_UserData = (void*)n;
	}

	//remapping from A index to B
	int * AToBRemap = new int[serializeA.m_NodeInitPositions.size()];
	
	int * BBeRemapTag = new int[serializeB.m_NodeInitPositions.size()];
	
	for (int i = 0; i < serializeA.m_NodeInitPositions.size(); i++)
	{
		 AToBRemap[i] = -1;
	}
	for (int i = 0; i < serializeB.m_NodeInitPositions.size(); i++)
	{
		 BBeRemapTag[i] = -1;
	}
	
	//test coincide node with node threshold
	NodeNearestCallBack nodeCallBack(NodeThresHold);
	nodeCallBack.m_AToBRemap = AToBRemap;
	nodeCallBack.m_BRemappedTag = BBeRemapTag;
	nodeCallBack.m_NodesPosA = &(serializeA.m_NodeInitPositions[0]);
	nodeCallBack.m_NodesPosB = &(serializeB.m_NodeInitPositions[0]);

	NodeTreeA.CollideWithDBVTree(NodeTreeB , &nodeCallBack);
	
	//remapping has constructed
	//
	int ValidANum = 0;
	for (int i = 0; i < serializeA.m_NodeInitPositions.size(); i++)
	{
		if(AToBRemap[i] == -1)
		   ValidANum++;
	}

	//construct final buffer and remapping array
	DstSerializer.m_NodeInitPositions.resize(ValidANum + serializeB.m_NodeInitPositions.size());
	
	GFPhysVector3 * CombinedNodesPosition = &(DstSerializer.m_NodeInitPositions[0]);//for convenient

	int CombinedNodeNum = 0;

	int * FinalRemappingA = new int[serializeA.m_NodeInitPositions.size()];
	
	int * FinalRemappingB = new int[serializeB.m_NodeInitPositions.size()];
	
	for (int n = 0; n < (int)serializeA.m_NodeInitPositions.size(); n++)
	{
		int IndexInBlocal = AToBRemap[n];
		if(IndexInBlocal == -1)
		{
		   CombinedNodesPosition[CombinedNodeNum] = serializeA.m_NodeInitPositions[n];
		   FinalRemappingA[n] = CombinedNodeNum;
		   CombinedNodeNum++;
		}
		else
		{
		   FinalRemappingA[n] = ValidANum + IndexInBlocal;//RemapingToB[i];
		}
	}

	assert(CombinedNodeNum == ValidANum);
	for (int n = 0; n < serializeB.m_NodeInitPositions.size(); n++)
	{
		CombinedNodesPosition[CombinedNodeNum] = serializeB.m_NodeInitPositions[n];
		FinalRemappingB[n] = CombinedNodeNum;//ValidANum+n;
		CombinedNodeNum++;
	}

	//rebuild fix index
	dstfixindex.clear();
	for(size_t p = 0 ; p < fixindexA.size() ; p++)
	{
		int originindex = fixindexA[p];
		int finalindex = FinalRemappingA[originindex];
		dstfixindex.push_back(finalindex);
	}

	for(size_t p = 0 ; p < fixindexB.size() ; p++)
	{
		int originindex = fixindexB[p];
		int finalindex = FinalRemappingB[originindex];
		dstfixindex.push_back(finalindex);
	}
#if(CHECKUNIONEDFACECONSIST)
	std::vector<MisMedicObjetSerializer::MisSerialFace> m_FaceUninedInA;
	std::vector<MisMedicObjetSerializer::MisSerialFace> m_FaceUninedInB;
#endif

	//now rebuild face and point
	DstSerializer.m_InitFaces.resize(serializeA.m_InitFaces.size() + serializeB.m_InitFaces.size());// = new MisMedicObjetSerializer::MisSerialFace[serializeA.m_FaceInitNum + serializeB.m_FaceInitNum];
	
	int InitFaceNum = 0;
	
	for (int f = 0; f < serializeA.m_InitFaces.size(); f++)
	{
		const MisMedicObjetSerializer::MisSerialFace & faceInA = serializeA.m_InitFaces[f];
		int originVID0 = faceInA.m_Index[0];
		int originVID1 = faceInA.m_Index[1];
		int originVID2 = faceInA.m_Index[2];

		if(AToBRemap[originVID0] == -1
		 ||AToBRemap[originVID1] == -1
         ||AToBRemap[originVID2] == -1)//only face not shared by a and b can add to final face
		{
			MisMedicObjetSerializer::MisSerialFace & DstFace = DstSerializer.m_InitFaces[InitFaceNum];
			DstFace.m_Index[0] = FinalRemappingA[originVID0];
			DstFace.m_Index[1] = FinalRemappingA[originVID1];
			DstFace.m_Index[2] = FinalRemappingA[originVID2];
#if(CHECKUNIONEDFACECONSIST)
			if(DstFace.m_Index[0] == DstFace.m_Index[1] || DstFace.m_Index[0] == DstFace.m_Index[2]
		   	 ||DstFace.m_Index[1] == DstFace.m_Index[2])
			{
				MessageBoxA(0,"face degenerated!!" , "",0);
			}
#endif
			DstFace.m_TextureCoord[0] = faceInA.m_TextureCoord[0];
			DstFace.m_TextureCoord[1] = faceInA.m_TextureCoord[1];
			DstFace.m_TextureCoord[2] = faceInA.m_TextureCoord[2];
			
			InitFaceNum++;
		}
#if(CHECKUNIONEDFACECONSIST)
		else
		{
			m_FaceUninedInA.push_back(faceInA);
		}
#endif
	}
	for (int f = 0; f < serializeB.m_InitFaces.size(); f++)
	{
		const MisMedicObjetSerializer::MisSerialFace & faceInB = serializeB.m_InitFaces[f];
		int originVID0 = faceInB.m_Index[0];
		int originVID1 = faceInB.m_Index[1];
		int originVID2 = faceInB.m_Index[2];

		if(BBeRemapTag[originVID0] == -1
		 ||BBeRemapTag[originVID1] == -1
		 ||BBeRemapTag[originVID2] == -1)
		//if(1)
		{
			MisMedicObjetSerializer::MisSerialFace & DstFace = DstSerializer.m_InitFaces[InitFaceNum];
			DstFace.m_Index[0] = FinalRemappingB[originVID0];
			DstFace.m_Index[1] = FinalRemappingB[originVID1];
			DstFace.m_Index[2] = FinalRemappingB[originVID2];
#if(CHECKUNIONEDFACECONSIST)
			if(DstFace.m_Index[0] == DstFace.m_Index[1]
			 ||DstFace.m_Index[0] == DstFace.m_Index[2]
			 ||DstFace.m_Index[1] == DstFace.m_Index[2])
			{
				MessageBoxA(0,"face degenerated!!" ,"",0);
			}
#endif
			DstFace.m_TextureCoord[0] = faceInB.m_TextureCoord[0];
			DstFace.m_TextureCoord[1] = faceInB.m_TextureCoord[1];
			DstFace.m_TextureCoord[2] = faceInB.m_TextureCoord[2];

			InitFaceNum++;
		}
#if(CHECKUNIONEDFACECONSIST)
		else
		{
			m_FaceUninedInB.push_back(faceInB);
		}
#endif
		DstSerializer.m_InitFaces.resize(InitFaceNum);
	}

#if(CHECKUNIONEDFACECONSIST)
		for(size_t fa = 0 ; fa < m_FaceUninedInA.size() ; fa++)
		{
			MisMedicObjetSerializer::MisSerialFace faceInA = m_FaceUninedInA[fa];
			
			int originVID0 = faceInA.m_Index[0];
			int originVID1 = faceInA.m_Index[1];
			int originVID2 = faceInA.m_Index[2];
			
			int FaceANIndex0 = FinalRemappingA[originVID0];
			int FaceANIndex1 = FinalRemappingA[originVID1];
			int FaceANIndex2 = FinalRemappingA[originVID2];

			bool findedcoincide = false;
			for(size_t fb = 0 ; fb < m_FaceUninedInB.size() ; fb++)
			{
				MisMedicObjetSerializer::MisSerialFace faceInB = m_FaceUninedInB[fb];
				
				int originVID0 = faceInB.m_Index[0];
				int originVID1 = faceInB.m_Index[1];
				int originVID2 = faceInB.m_Index[2];

				int FaceBNIndex0 = FinalRemappingB[originVID0];
				int FaceBNIndex1 = FinalRemappingB[originVID1];
				int FaceBNIndex2 = FinalRemappingB[originVID2];

				Order3(FaceANIndex0, FaceANIndex1, FaceANIndex2);
				Order3(FaceBNIndex0, FaceBNIndex1, FaceBNIndex2);

				if(FaceANIndex0 == FaceBNIndex0 && FaceANIndex1 == FaceBNIndex1 && FaceANIndex2 == FaceBNIndex2)
				{
					findedcoincide = true;
					break;
				}
			}
			if(findedcoincide == false)
			{
				int i = 0;
				int j = i+1;
				//MessageBoxA(0, "find unconincide facec" , "" ,0);
			}
		}
#endif

	//now add tetrahedron
	DstSerializer.m_InitTetras.resize(serializeA.m_InitTetras.size() + serializeB.m_InitTetras.size());// = new MisMedicObjetSerializer::MisSerialTetra[serializeA.m_TetraInitNum + serializeB.m_TetraInitNum];
	
	int DstTetraInitNum = 0;
	
	for (int t = 0; t < (int)serializeA.m_InitTetras.size(); t++)
	{
		const MisMedicObjetSerializer::MisSerialTetra & tetra = serializeA.m_InitTetras[t];

		MisMedicObjetSerializer::MisSerialTetra & DstTetra = DstSerializer.m_InitTetras[DstTetraInitNum];
		DstTetra = tetra;

		DstTetra.m_Index[0] = FinalRemappingA[tetra.m_Index[0]];
		DstTetra.m_Index[1] = FinalRemappingA[tetra.m_Index[1]];
		DstTetra.m_Index[2] = FinalRemappingA[tetra.m_Index[2]];
		DstTetra.m_Index[3] = FinalRemappingA[tetra.m_Index[3]];
		
		if(tetra.m_unionObjectID >= 0)
		   DstTetra.m_unionObjectID = tetra.m_unionObjectID;
		else
		   DstTetra.m_unionObjectID = ObjAID;

		DstTetraInitNum++;
	}

	for (int t = 0; t < (int)serializeB.m_InitTetras.size(); t++)
	{
		const MisMedicObjetSerializer::MisSerialTetra & tetra = serializeB.m_InitTetras[t];

		MisMedicObjetSerializer::MisSerialTetra & DstTetra = DstSerializer.m_InitTetras[DstTetraInitNum];
		DstTetra = tetra;

		DstTetra.m_Index[0] = FinalRemappingB[tetra.m_Index[0]];
		DstTetra.m_Index[1] = FinalRemappingB[tetra.m_Index[1]];
		DstTetra.m_Index[2] = FinalRemappingB[tetra.m_Index[2]];
		DstTetra.m_Index[3] = FinalRemappingB[tetra.m_Index[3]];

		if(tetra.m_unionObjectID >= 0)
		   DstTetra.m_unionObjectID = tetra.m_unionObjectID;
		else
		   DstTetra.m_unionObjectID = ObjBID;

		DstTetraInitNum++;
	}
	DstSerializer.m_InitTetras.resize(DstTetraInitNum);

	delete []AToBRemap;
	delete []BBeRemapTag;
	delete []FinalRemappingA;
	delete []FinalRemappingB;
}

void MisMedicObjectUnion::MergeObjectToExist(int LayerIndex, 
	                                         MisMedicObjetSerializer & DstSerializer,
	                                         int ObjAID, Ogre::String fileA,
	                                         Ogre::Vector3 offset, float NodeThresHold,
						                     const std::vector<int> & fixindexB, 
						                     std::vector<int> & dstfixindex)
{
	MisMedicObjetSerializer serializeA;
	
	serializeA.ReadFromOrganObjectFile(ObjAID, fileA, "", "", offset);
	
	GFPhysVector3 Extend(NodeThresHold, NodeThresHold, NodeThresHold);
	GFPhysDBVTree NodeTreeA;
	GFPhysDBVTree NodeTreeB;

	//insert undeformed aabb to treeA
	for (int n = 0; n < (int)serializeA.m_NodeInitPositions.size(); n++)
	{
		GFPhysVector3 NodePos = serializeA.m_NodeInitPositions[n];
		GFPhysDBVNode * dbvn = NodeTreeA.InsertAABBNode(NodePos - Extend, NodePos + Extend);
		dbvn->m_UserData = (void*)n;
	}

	//insert undeformed aabb to treeB
	for (int n = 0; n < (int)DstSerializer.m_NodeInitPositions.size(); n++)
	{
		GFPhysVector3 NodePos = DstSerializer.m_NodeInitPositions[n];
		GFPhysDBVNode * dbvn = NodeTreeB.InsertAABBNode(NodePos - Extend, NodePos + Extend);
		dbvn->m_UserData = (void*)n;
	}

	//remapping from A index to B
	int * AToBRemap = new int[serializeA.m_NodeInitPositions.size()];

	int * BBeRemapTag = new int[DstSerializer.m_NodeInitPositions.size()];

	for (int i = 0; i < serializeA.m_NodeInitPositions.size(); i++)
	{
		AToBRemap[i] = -1;
	}
	for (int i = 0; i < DstSerializer.m_NodeInitPositions.size(); i++)
	{
		BBeRemapTag[i] = -1;
	}

	//test coincide node with node threshold
	NodeNearestCallBack nodeCallBack(NodeThresHold);
	nodeCallBack.m_AToBRemap = AToBRemap;
	nodeCallBack.m_BRemappedTag = BBeRemapTag;
	nodeCallBack.m_NodesPosA = &(serializeA.m_NodeInitPositions[0]);
	nodeCallBack.m_NodesPosB = &(DstSerializer.m_NodeInitPositions[0]);

	NodeTreeA.CollideWithDBVTree(NodeTreeB, &nodeCallBack);

	//remapping has constructed
	//
	int ValidANum = 0;
	for (int i = 0; i < serializeA.m_NodeInitPositions.size(); i++)
	{
		if (AToBRemap[i] == -1)
			ValidANum++;
	}

	int DstNodeBeforeMerge = DstSerializer.m_NodeInitPositions.size();
	
	//construct final buffer and remapping array
	DstSerializer.m_NodeInitPositions.resize(DstNodeBeforeMerge + ValidANum);
	
	GFPhysVector3 * CombinedNodesPosition = &(DstSerializer.m_NodeInitPositions[0]);//for convenient

	int * FinalRemappingA = new int[serializeA.m_NodeInitPositions.size()];

	int * FinalRemappingB = new int[DstNodeBeforeMerge];

	for (int n = 0; n < DstNodeBeforeMerge; n++)
	{
		 FinalRemappingB[n] = n;//the dest part not change map to itself
	}

	int CombinedNodeNum = DstNodeBeforeMerge;

	for (int n = 0; n < (int)serializeA.m_NodeInitPositions.size(); n++)
	{
		 int IndexInBlocal = AToBRemap[n];
		
		 if (IndexInBlocal == -1)
		 {
			 CombinedNodesPosition[CombinedNodeNum] = serializeA.m_NodeInitPositions[n];
			 FinalRemappingA[n] = CombinedNodeNum;
			 CombinedNodeNum++;
		 }
		 else
		 {
			 FinalRemappingA[n] = IndexInBlocal;
		 }
	}

	//rebuild fix index
	for (int p = 0; p < (int)fixindexB.size(); p++)
	{
		int originindex = fixindexB[p];
		int finalindex = FinalRemappingB[originindex];
		dstfixindex.push_back(finalindex);
	}

	GFPhysHashMap<MisMedicObjetSerializer::HashSharedFaces, int> ObjASharedFaces;
	GFPhysHashMap<MisMedicObjetSerializer::HashSharedFaces, int> ObjBSharedFaces;

	//mark face coincide as not build inner texture
	for (int f = 0; f < serializeA.m_InitFaces.size(); f++)
	{
		 MisMedicObjetSerializer::MisSerialFace & faceInA = serializeA.m_InitFaces[f];
		 int originVID0 = faceInA.m_Index[0];
		 int originVID1 = faceInA.m_Index[1];
		 int originVID2 = faceInA.m_Index[2];

		 if (AToBRemap[originVID0] == -1 || AToBRemap[originVID1] == -1 || AToBRemap[originVID2] == -1)//only face not shared by a and b can add to final face
		 {
			 faceInA.m_IsCoincideFaces = false;
		 }
		 else
		 {
			 if (LayerIndex == 0)
			     faceInA.m_IsCoincideFaces = true;
			 else
		         faceInA.m_IsCoincideFaces = false;

			 ObjASharedFaces.insert(MisMedicObjetSerializer::HashSharedFaces(AToBRemap[originVID0],
				                                                             AToBRemap[originVID1], 
																		     AToBRemap[originVID2]), 
																		     f);
		 }
	}
	serializeA.GenerateInnerTexture();

	//
#if(CHECKUNIONEDFACECONSIST)
	std::vector<MisMedicObjetSerializer::MisSerialFace> m_FaceUninedInA;
	std::vector<MisMedicObjetSerializer::MisSerialFace> m_FaceUninedInB;
#endif

	//now rebuild face and point
	//DstSerializer.m_InitFaces = new MisMedicObjetSerializer::MisSerialFace[serializeA.m_FaceInitNum + serializeB.m_FaceInitNum];
	//DstSerializer.m_FaceInitNum = 0;
	// DstSerializer.m_InitFaces.size();
	std::vector<MisMedicObjetSerializer::MisSerialFace> InitFaces = DstSerializer.m_InitFaces;

	DstSerializer.m_InitFaces.resize(InitFaces.size() + serializeA.m_InitFaces.size());
	
	int DstFaceNum = 0;
	
	for (int f = 0; f < InitFaces.size(); f++)
	{
		const MisMedicObjetSerializer::MisSerialFace & faceInB = InitFaces[f];
		int originVID0 = faceInB.m_Index[0];
		int originVID1 = faceInB.m_Index[1];
		int originVID2 = faceInB.m_Index[2];

		bool isSharedFace = false;

		if (BBeRemapTag[originVID0] == -1 || BBeRemapTag[originVID1] == -1 || BBeRemapTag[originVID2] == -1)//at least one node is not shared by object A
		{
			isSharedFace = false;
		}
		else//all node is shared by objectA we check whether objectA has the same face
		{
			int * index = ObjASharedFaces.find(MisMedicObjetSerializer::HashSharedFaces(originVID0, originVID1, originVID2));
			if (index == 0)
			{
				isSharedFace = false;
			}
			else
			{
				isSharedFace = true;
			}
		}

		if (isSharedFace == false)//only face not shared by a and b can add to final face
		{
			MisMedicObjetSerializer::MisSerialFace & DstFace = DstSerializer.m_InitFaces[DstFaceNum];
			DstFace.m_Index[0] = FinalRemappingB[originVID0];
			DstFace.m_Index[1] = FinalRemappingB[originVID1];
			DstFace.m_Index[2] = FinalRemappingB[originVID2];
#if(CHECKUNIONEDFACECONSIST)
			if (DstFace.m_Index[0] == DstFace.m_Index[1]
			 || DstFace.m_Index[0] == DstFace.m_Index[2]
			 || DstFace.m_Index[1] == DstFace.m_Index[2])
			{
				MessageBoxA(0, "face degenerated!!", "", 0);
			}
#endif
			DstFace.m_TextureCoord[0] = faceInB.m_TextureCoord[0];
			DstFace.m_TextureCoord[1] = faceInB.m_TextureCoord[1];
			DstFace.m_TextureCoord[2] = faceInB.m_TextureCoord[2];

			DstFaceNum++;
		}
#if(CHECKUNIONEDFACECONSIST)
		else
		{
			m_FaceUninedInB.push_back(faceInB);
			ObjBSharedFaces.insert(MisMedicObjetSerializer::HashSharedFaces(FinalRemappingB[originVID0],
				                                                            FinalRemappingB[originVID1],
				                                                            FinalRemappingB[originVID2]),
				                                                            f);
		}
#endif
	}

	for (int f = 0; f < serializeA.m_InitFaces.size(); f++)
	{
		const MisMedicObjetSerializer::MisSerialFace & faceInA = serializeA.m_InitFaces[f];
		int originVID0 = faceInA.m_Index[0];
		int originVID1 = faceInA.m_Index[1];
		int originVID2 = faceInA.m_Index[2];

		bool isSharedFace = false;

		if (AToBRemap[originVID0] == -1 || AToBRemap[originVID1] == -1 || AToBRemap[originVID2] == -1)//at least one node is not shared by object A
		{
			isSharedFace = false;
		}
		else//all node is shared by objectA we check whether objectA has the same face
		{
			int * index = ObjBSharedFaces.find(MisMedicObjetSerializer::HashSharedFaces(FinalRemappingA[originVID0], 
				                                                                        FinalRemappingA[originVID1],
																						FinalRemappingA[originVID2]));
			if (index == 0)
			{
				isSharedFace = false;
			}
			else
			{
				isSharedFace = true;
			}
		}

		if (isSharedFace == false)
		{
			MisMedicObjetSerializer::MisSerialFace & DstFace = DstSerializer.m_InitFaces[DstFaceNum];
			DstFace.m_Index[0] = FinalRemappingA[originVID0];
			DstFace.m_Index[1] = FinalRemappingA[originVID1];
			DstFace.m_Index[2] = FinalRemappingA[originVID2];
#if(CHECKUNIONEDFACECONSIST)
			if (DstFace.m_Index[0] == DstFace.m_Index[1] 
			 || DstFace.m_Index[0] == DstFace.m_Index[2]
			 || DstFace.m_Index[1] == DstFace.m_Index[2])
			{
				MessageBoxA(0, "face degenerated!!", "", 0);
			}
#endif
			DstFace.m_TextureCoord[0] = faceInA.m_TextureCoord[0];
			DstFace.m_TextureCoord[1] = faceInA.m_TextureCoord[1];
			DstFace.m_TextureCoord[2] = faceInA.m_TextureCoord[2];

			DstFaceNum++;
		}
#if(CHECKUNIONEDFACECONSIST)
		else
		{
			m_FaceUninedInA.push_back(faceInA);
		}
#endif
	}
	DstSerializer.m_InitFaces.resize(DstFaceNum);

#if(CHECKUNIONEDFACECONSIST)
	/*
	for (size_t fa = 0; fa < m_FaceUninedInA.size(); fa++)
	{
		MisMedicObjetSerializer::MisSerialFace faceInA = m_FaceUninedInA[fa];

		int originVID0 = faceInA.m_Index[0];
		int originVID1 = faceInA.m_Index[1];
		int originVID2 = faceInA.m_Index[2];

		int FaceANIndex0 = FinalRemappingA[originVID0];
		int FaceANIndex1 = FinalRemappingA[originVID1];
		int FaceANIndex2 = FinalRemappingA[originVID2];

		bool findedcoincide = false;
		for (size_t fb = 0; fb < m_FaceUninedInB.size(); fb++)
		{
			MisMedicObjetSerializer::MisSerialFace faceInB = m_FaceUninedInB[fb];

			int originVID0 = faceInB.m_Index[0];
			int originVID1 = faceInB.m_Index[1];
			int originVID2 = faceInB.m_Index[2];

			int FaceBNIndex0 = FinalRemappingB[originVID0];
			int FaceBNIndex1 = FinalRemappingB[originVID1];
			int FaceBNIndex2 = FinalRemappingB[originVID2];

			Order3(FaceANIndex0, FaceANIndex1, FaceANIndex2);
			Order3(FaceBNIndex0, FaceBNIndex1, FaceBNIndex2);

			if (FaceANIndex0 == FaceBNIndex0 && FaceANIndex1 == FaceBNIndex1 && FaceANIndex2 == FaceBNIndex2)
			{
				findedcoincide = true;
				break;
			}
		}
		if (findedcoincide == false)
		{
			int i = 0;
			int j = i + 1;
			//MessageBoxA(0, "find unconincide facec" , "" ,0);
		}
	}
	*/
#endif

	//now add tetrahedron
	int DstTetraNum = DstSerializer.m_InitTetras.size();

	DstSerializer.m_InitTetras.resize(DstTetraNum + serializeA.m_InitTetras.size());// = new MisMedicObjetSerializer::MisSerialTetra[serializeA.m_TetraInitNum + serializeB.m_TetraInitNum];
	
	for (int t = 0; t < serializeA.m_InitTetras.size(); t++)
	{
		 const MisMedicObjetSerializer::MisSerialTetra & tetra = serializeA.m_InitTetras[t];

		 MisMedicObjetSerializer::MisSerialTetra & DstTetra = DstSerializer.m_InitTetras[DstTetraNum];
		 DstTetra = tetra;

		 DstTetra.m_Index[0] = FinalRemappingA[tetra.m_Index[0]];
		 DstTetra.m_Index[1] = FinalRemappingA[tetra.m_Index[1]];
		 DstTetra.m_Index[2] = FinalRemappingA[tetra.m_Index[2]];
		 DstTetra.m_Index[3] = FinalRemappingA[tetra.m_Index[3]];

		 if (tetra.m_unionObjectID >= 0)
			 DstTetra.m_unionObjectID = tetra.m_unionObjectID;
		 else
			 DstTetra.m_unionObjectID = ObjAID;

		 DstTetra.m_LayerIndex = LayerIndex;
		 DstTetraNum++;
	}
	DstSerializer.m_InitTetras.resize(DstTetraNum);

	delete[]AToBRemap;
	delete[]BBeRemapTag;
	delete[]FinalRemappingA;
	delete[]FinalRemappingB;
}

void GetOgreMeshVertexIndex(  const Ogre::MeshPtr mesh , 
							  size_t & vertex_count,
							  SubMeshVertex * & vertices,
							  size_t & index_count,
							  unsigned long* &indices,
							  const Ogre::Vector3 & position,
							  const Ogre::Quaternion & orient,
							  const Ogre::Vector3 & scale ,
							  bool IsGetIndices)
{
	bool added_shared = false;
	size_t current_offset = 0;
	size_t shared_offset = 0;
	size_t next_offset = 0;
	size_t index_offset = 0;

	vertex_count = index_count = 0;

	// Calculate how many vertices and indices we're going to need
	for ( unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
	{
		Ogre::SubMesh* submesh = mesh->getSubMesh(i);
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

	// Allocate space for the vertices and indices
	vertices = new SubMeshVertex[vertex_count];
	indices  = new unsigned long[index_count];

	added_shared = false;

	// Run through the submeshes again, adding the data into the arrays
	for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
	{
		Ogre::SubMesh* submesh = mesh->getSubMesh(i);

		Ogre::VertexData* vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;

		if ((!submesh->useSharedVertices) || (submesh->useSharedVertices && !added_shared))
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
			// as second argument. So make it float, to avoid trouble when Ogre::Real will
			// be comiled/typedefed as double:
			//Ogre::Real* pReal;
			float* pReal;

			for( size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
			{
				posElem->baseVertexPointerToElement(vertex, &pReal);
				Ogre::Vector3 pt(pReal[0], pReal[1], pReal[2]);
				Ogre::Vector3 worldPos = (orient * (pt * scale)) + position;
				vertices[current_offset + j] = SubMeshVertex(i , j , worldPos , submesh->useSharedVertices);

				pReal[0] = worldPos.x;
				pReal[1] = worldPos.y;
				pReal[2] = worldPos.z;
			}

			vbuf->unlock();
			next_offset += vertex_data->vertexCount;
		}

		if(IsGetIndices)
		{
			Ogre::IndexData* index_data = submesh->indexData;
			size_t numTris = index_data->indexCount / 3;
			Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;

			bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);

			unsigned long* pLong = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
			unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);

			size_t offset = (submesh->useSharedVertices)? shared_offset : current_offset;

			if ( use32bitindexes )
			{
				for ( size_t k = 0; k < numTris*3; ++k)
				{
					indices[index_offset++] = pLong[k] + static_cast<unsigned long>(offset);
				}
			}
			else
			{
				for ( size_t k = 0; k < numTris*3; ++k)
				{
					indices[index_offset++] = static_cast<unsigned long>(pShort[k]) +
						static_cast<unsigned long>(offset);
				}
			}
			ibuf->unlock();
		}
		current_offset = next_offset;
	}

}

//==============================================================================================
void MisMedicObjectUnion::AttachStaticMeshToDynamicOrgan(Ogre::MeshPtr meshptr, 
	                                                     Ogre::Node * nodestatic,
														 Ogre::Vector3 position,
														 Ogre::Quaternion orient,
														 Ogre::Vector3 scale,
														 MisMedicOrgan_Ordinary * organObj,
														 float NodeThresHold)
{
	m_StaticDynamicSharedVertex.clear();
	m_VertexToDynMap.clear();
	GFPhysVector3 Extend(NodeThresHold , NodeThresHold , NodeThresHold);
	GFPhysDBVTree StaticMeshNodeTree;
	GFPhysDBVTree DynamicMeshNodeTree;

	size_t vertex_count;
	SubMeshVertex * StaticMeshVertex;
	
	size_t index_count;
	unsigned long * indices;
	
    GetOgreMeshVertexIndex( meshptr ,
						   vertex_count , 
						   StaticMeshVertex ,
						   index_count, 
						   indices , 
						   position , 
						   orient ,
						   scale);


	//insert undeformed aabb to static mesh
	for(size_t n = 0 ; n < vertex_count ; n++)
	{
		GFPhysVector3 NodePos = StaticMeshVertex[n].m_Position;
		GFPhysDBVNode * dbvn = StaticMeshNodeTree.InsertAABBNode(NodePos-Extend , NodePos+Extend);
		dbvn->m_UserData = (void*)n;
	}

	//insert undeformed aabb to dynamic mesh
	GFPhysSoftBodyNode * SoftNode = organObj->m_physbody->GetNodeList();
	while(SoftNode)
	{
		GFPhysVector3 NodePos = SoftNode->m_UnDeformedPos;
		GFPhysDBVNode * dbvn = DynamicMeshNodeTree.InsertAABBNode(NodePos-Extend , NodePos+Extend);
		dbvn->m_UserData = SoftNode;
		SoftNode = SoftNode->m_Next;
	}

	//check attached nodes
	S2DNearestCallBack nodeCallBack(NodeThresHold);
	nodeCallBack.m_StaticPos = StaticMeshVertex;
	StaticMeshNodeTree.CollideWithDBVTree(DynamicMeshNodeTree , &nodeCallBack);
#if 0
    m_StaticDynamicSharedVertex = nodeCallBack.m_SharedVertex;
#else
    for (int i = 0; i < nodeCallBack.m_SharedVertex.size();i++)
    {
        m_StaticDynamicSharedVertex.push_back(nodeCallBack.m_SharedVertex[i]);
    }
#endif
    

	//

	m_VertexToDynMap.resize(meshptr->getNumSubMeshes());
	for(size_t n = 0 ; n < m_StaticDynamicSharedVertex.size() ; n++)
	{
		DynStaticSharedVertex & dynvert = m_StaticDynamicSharedVertex[n];
		for(size_t p = 0 ; p < dynvert.m_NodesInStatic.size() ; p++)
		{
			SubMeshVertex & svert = dynvert.m_NodesInStatic[p];

			unsigned int combIndex = ((n & 0xFFFF) << 16) | (p & 0xFFFF);

			unsigned int t0 = combIndex & 0xFFFF;
			unsigned int t1 = (combIndex >> 16) & 0xFFFF;
			
			m_VertexToDynMap[svert.m_SubMesh].push_back(combIndex);
		}
	}

	//after union set position orientation to identity
	meshptr->_setBounds(Ogre::AxisAlignedBox(Ogre::Vector3(-1000, -1000, -1000), Ogre::Vector3(1000, 1000, 1000)));//temply
	nodestatic->setPosition(Ogre::Vector3::ZERO);
	nodestatic->setOrientation(Ogre::Quaternion::IDENTITY);
	nodestatic->setScale(1, 1, 1);

	//

	std::set<GFPhysSoftBodyNode*> fixNodes;
	for (size_t n = 0; n < m_StaticDynamicSharedVertex.size(); n++)
	{
		DynStaticSharedVertex & shareVertex = m_StaticDynamicSharedVertex[n];

		GFPhysSoftBodyNode * fixnode = shareVertex.m_DynamicNode;

		PhysNode_Data & physNodeData = organObj->GetPhysNodeData(fixnode);

		if (!physNodeData.m_HasError)
		{
			physNodeData.m_NodeBeFixed = true;
		}

		fixNodes.insert(fixnode);
	}

	std::set<GFPhysSoftBodyNode*>::iterator itor = fixNodes.begin();
	while (itor != fixNodes.end())
	{
		GFPhysSoftBodyNode * fixnode = (*itor);
		fixnode->SetMass(0);
		itor++;
	}
}

void MisMedicObjectUnion::UpdateStaticVertexByDynamic(Ogre::MeshPtr StaticMeshptr)
{
	int NumSubMesh = StaticMeshptr->getNumSubMeshes();

	for (int s = 0; s < NumSubMesh; s++)
	{
		Ogre::SubMesh * submesh = StaticMeshptr->getSubMesh(s);
		// Run through the sub-meshes again, adding the data into the arrays
		Ogre::VertexData * vertex_data = submesh->useSharedVertices ? StaticMeshptr->sharedVertexData : submesh->vertexData;

		const Ogre::VertexElement * posElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);

		const Ogre::VertexElement * norElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_NORMAL);

		Ogre::HardwareVertexBufferSharedPtr vbuf = vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());

		unsigned char * VBufferStart = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));

		float * pReal;

		const std::vector<unsigned int> & SubVertDynMap = m_VertexToDynMap[s];

		for (size_t n = 0; n < SubVertDynMap.size(); n++)
		{
			int combIndex = SubVertDynMap[n];

			unsigned int tp = combIndex & 0xFFFF;
			unsigned int tn = (combIndex >> 16) & 0xFFFF;

			DynStaticSharedVertex & shareVertex = m_StaticDynamicSharedVertex[tn];

			GFPhysVector3 DstPos = shareVertex.m_DynamicNode->m_CurrPosition;
			GFPhysVector3 DstNorm = shareVertex.m_DynamicNode->m_Normal;

			int offset = shareVertex.m_NodesInStatic[tp].m_VertexOffset;

			unsigned char * vertex = VBufferStart + vbuf->getVertexSize()*offset;

			norElem->baseVertexPointerToElement(vertex, &pReal);
			pReal[0] = DstNorm.m_x;
			pReal[1] = DstNorm.m_y;
			pReal[2] = DstNorm.m_z;

			posElem->baseVertexPointerToElement(vertex, &pReal);
			pReal[0] = DstPos.m_x;
			pReal[1] = DstPos.m_y;
			pReal[2] = DstPos.m_z;
		}
		vbuf->unlock();
	}
}
//===================================================================================================================================
void MisMedicObjectAdhersion::AttachNodesCenterToBody(const std::vector<GFPhysSoftBodyNode*> & nodes , GFPhysSoftBody * attacbody)
{
	GFPhysVector3 centerPos(0,0,0);
	
	for(size_t n = 0 ; n < nodes.size(); n++)
	{
		GFPhysVector3 nodepos = nodes[n]->m_UnDeformedPos;

		float minDist = FLT_MAX;

		GFPhysSoftBodyTetrahedron * minTetra = 0;

		GFPhysVector3 closetPointInSoft;

		//GFPhysSoftBodyTetrahedron * tetra = attacbody->GetTetrahedronList();
	
		//while(tetra)
		for(size_t th = 0 ; th < attacbody->GetNumTetrahedron() ; th++)
		{
			GFPhysSoftBodyTetrahedron * tetra = attacbody->GetTetrahedronAtIndex(th);

			GFPhysVector3 closetPoint = ClosetPtPointTetrahedron(nodepos, 
																 tetra->m_TetraNodes[0]->m_UnDeformedPos,
																 tetra->m_TetraNodes[1]->m_UnDeformedPos,
																 tetra->m_TetraNodes[2]->m_UnDeformedPos,
																 tetra->m_TetraNodes[3]->m_UnDeformedPos);

			float dist = (closetPoint-nodepos).Length();

			if(dist < minDist)
			{
				minDist = dist;
				closetPointInSoft = closetPoint;
				minTetra = tetra;
			}

			//tetra = tetra->m_Next;
		}
		if(minDist < FLT_MAX)
		{
			float weights[4];
			bool  gettedf = GetPointBarycentricCoordinate(  minTetra->m_TetraNodes[0]->m_UnDeformedPos,
															minTetra->m_TetraNodes[1]->m_UnDeformedPos,
															minTetra->m_TetraNodes[2]->m_UnDeformedPos,
															minTetra->m_TetraNodes[3]->m_UnDeformedPos,
															closetPointInSoft,
															weights);
			if(gettedf)
			{
				TetrahedronAttachConstraint * cs = new TetrahedronAttachConstraint(nodes[n] , 
																				   minTetra,//->m_TetraNodes,
																				   weights);

				cs->SetStiffness(0.99f);
				PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(cs);
				m_constraints.push_back(cs);
			}
		}
	}
}

//=============================================================================================================
MisMedicObjectAdhersion::MisMedicObjectAdhersion()
{
	m_ConnectOrganA = 0;
	m_ConnectOrganB = 0;

	m_HideLinkFace = true;
	if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
	   PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(this);
	m_Stiffness = 1.0f;
}
//=============================================================================================================
MisMedicObjectAdhersion::~MisMedicObjectAdhersion()
{
	for(size_t c = 0 ; c < m_constraints.size(); c++)
	{
		if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
		   PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(m_constraints[c]);
		delete m_constraints[c];
	}
	if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
	   PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(this);
}
//=============================================================================================================
void MisMedicObjectAdhersion::PrepareSolveConstraint(Real Stiffness,Real TimeStep)\
{

}
//=============================================================================================================
void MisMedicObjectAdhersion::SolveConstraint(Real Stiffness,Real TimeStep)
{
	/*
	for(size_t c = 0 ; c < m_NodeToNodeLinks.size() ; c++)
	{
		MM_NodeToNodeLinkPair & nnlink = m_NodeToNodeLinks[c];
		
		if(nnlink.m_IsValid == false)
		   continue;

		Real w1 = (nnlink.m_UseCustomWeight ? nnlink.m_CustomedWeight[0] : nnlink.m_NodeInA->m_InvM);
		
		Real w2 = (nnlink.m_UseCustomWeight ? nnlink.m_CustomedWeight[1] : nnlink.m_NodeInB->m_InvM);
		
		Real w = w1+w2;
		
		if(w <= GP_EPSILON)
		   continue;

		GFPhysVector3 Diff = nnlink.m_NodeInA->m_CurrPosition - nnlink.m_NodeInB->m_CurrPosition;
		
		Real Length = Diff.Length();

		Real diffLen = Length-0;

		if(Length > GP_EPSILON)
		{
			Real Temp = diffLen / Length;
			GFPhysVector3 Deta1 = -(w1 / (w1+w2)) * Temp * Diff;
			GFPhysVector3 Deta2 =  (w2 / (w1+w2)) * Temp * Diff;
			nnlink.m_NodeInA->m_CurrPosition += (Deta1 * nnlink.m_LinksStiffness);
			nnlink.m_NodeInB->m_CurrPosition += (Deta2 * nnlink.m_LinksStiffness);
		}
	}*/

	//
	for(size_t c = 0 ; c < m_NodeToTetraLinks.size() ; c++)
	{
		if(m_NodeToTetraLinks[c].m_IsValid)
		{
		   m_NodeToTetraLinks[c].SolveConstraint(Stiffness , TimeStep);
		}
	}

	for(size_t c = 0 ; c < m_FaceToEdgeLinks.size() ; c++)
	{
		if(m_FaceToEdgeLinks[c].m_IsValid)
		{
		   m_FaceToEdgeLinks[c].SolveConstraint(Stiffness , TimeStep);
		}
	}
}
//=============================================================================================================
bool MisMedicObjectAdhersion::BuildObjectAdhesion(CXMLWrapperAdhere * adhereconfig,
												  MisNewTraining * mistrain)
{
	
	m_HideLinkFace = adhereconfig->m_HideLinkFace;

	MisMedicOrganInterface * tmp = mistrain->GetOrgan(adhereconfig->m_Object1ID);

	m_ConnectOrganA = dynamic_cast<MisMedicOrgan_Ordinary*>(tmp);

	tmp = mistrain->GetOrgan(adhereconfig->m_Object2ID);

	m_ConnectOrganB = dynamic_cast<MisMedicOrgan_Ordinary*>(tmp);
		
	Ogre::String & strType = adhereconfig->m_AdhereType;
	    
	if(strType == "Insert")
	{
		m_AdhType = ADH_INSERT;
		if (m_ConnectOrganB->GetCreateInfo().m_objTopologyType == DOT_VOLMESH)
		    BuildUniversalLinkFromAToB(*m_ConnectOrganA,*m_ConnectOrganB,0.99f);
		else if (m_ConnectOrganB->GetCreateInfo().m_objTopologyType == DOT_MEMBRANE)
			BuildNodeSurfaceLinkFromAToB(*m_ConnectOrganA, *m_ConnectOrganB, 0.99f, 0.05f);
		
		return true;
	}
	else
	{
		return false;			
	}
}

//=============================================================================================================
/*
void MisMedicObjectAdhersion::BuildAdhersionBetWeenOrgan( MisMedicOrgan_Ordinary & organA , 
														  MisMedicOrgan_Ordinary & organB , 
														 float NodeThresHold , 
														 float ratio)
{
	GFPhysSoftBody * bodyA = organA.m_physbody;
	GFPhysSoftBody * bodyB = organB.m_physbody;

	GFPhysVector3 Extend(NodeThresHold , NodeThresHold , NodeThresHold);
	GFPhysDBVTree NodeTreeA;
	GFPhysDBVTree NodeTreeB;
	// The inflated organ is placed in front of pair.
    GFPhysVector3 * UnDeformedA = new GFPhysVector3[organA.m_Serializer_NodeNum];
    if (organA.m_Serializer_NodeNum)
    {
        GFPhysVector3 * SrAInitPos = organA.m_Serializer_NodeInitPositions_copy;   
        int i =0;
        GFPhysSoftBodyNode * NodeAinit = bodyA->GetNodeList();
        while(NodeAinit )
        {
            UnDeformedA[i] = NodeAinit->m_UnDeformedPos; //充气后
            NodeAinit->m_UnDeformedPos = SrAInitPos[i];//充气前
            NodeAinit = NodeAinit->m_Next;
            i++;
        } 
    }
	//insert undeformed aabb to treeA
	GFPhysSoftBodyNode * NodeA = bodyA->GetNodeList();
	while(NodeA)
	{
		GFPhysVector3 NodePos = NodeA->m_UnDeformedPos;
		GFPhysDBVNode * dbvn = NodeTreeA.InsertAABBNode(NodePos-Extend , NodePos+Extend);
		dbvn->m_UserData = NodeA;
		NodeA = NodeA->m_Next;
	}

	GFPhysSoftBodyNode * NodeB = bodyB->GetNodeList();
	while(NodeB)
	{
		GFPhysVector3 NodePos = NodeB->m_UnDeformedPos;
		GFPhysDBVNode * dbvn = NodeTreeB.InsertAABBNode(NodePos-Extend , NodePos+Extend);
		dbvn->m_UserData = NodeB;
		NodeB = NodeB->m_Next;
	}


	//test coincide node with node threshold
	SoftBodyAdhersionCallBack nodeCallBack(NodeThresHold);
	NodeTreeA.CollideWithDBVTree(NodeTreeB , &nodeCallBack);

	const std::map<GFPhysSoftBodyNode *, NearestNode> & AdherNodeMap = nodeCallBack.m_NearestNodes;
	std::map<GFPhysSoftBodyNode *, NearestNode>::const_iterator itor = AdherNodeMap.begin();

	float customweights[2] = { 0.5f , 0.5f};

	float weight1 = ratio;
	float weight2 = 1;

	customweights[0] = weight1;
	customweights[1] = weight2;


	while(itor != AdherNodeMap.end())
	{
		GFPhysSoftBodyNode * NodeA = itor->first;
		GFPhysSoftBodyNode * NodeB = itor->second.m_Node;

		NodeA->m_CurrPosition  = NodeB->m_CurrPosition;
		NodeA->m_UnDeformedPos = NodeB->m_UnDeformedPos;

		NodeA->m_StateFlag |= GPSESF_CONNECTED;
		NodeB->m_StateFlag |= GPSESF_CONNECTED;


	    MM_NodeToNodeLinkPair nn_linkpair(NodeA , NodeB);
		if(ratio > FLT_EPSILON)
		{
		   nn_linkpair.SetCustomWeights(customweights[0] , customweights[1]);
		}
		
		//for quick find when node removed
		m_NodeToNodeLinks.push_back(nn_linkpair);
		
		//m_NodesLinkMap.insert(std::make_pair(NodeA , m_NodeToNodeLinks.size()-1));
		//
		//m_NodesLinkMap.insert(std::make_pair(NodeB , m_NodeToNodeLinks.size()-1));

		itor++;
	}

	for(size_t f = 0 ; f < organA.m_OriginFaces.size() ; f++)
	{
		MMO_Face & OriginFaceA = organA.m_OriginFaces[f];
		if(OriginFaceA.m_NeedRend == true)//this face not processed yet
		{
		   GFPhysSoftBodyFace * physfaceA = organA.m_OriginFaces[f].m_physface;
		   GFPhysSoftBodyNode * NodeA[3];
		   GFPhysSoftBodyNode * NodeB[3];
		   
		   NodeA[0] = physfaceA->m_Nodes[0];
		   NodeA[1] = physfaceA->m_Nodes[1];
		   NodeA[2] = physfaceA->m_Nodes[2];

		   NodeB[0] = NodeB[1] = NodeB[2] = 0;

		   bool connect0 = (physfaceA->m_Nodes[0]->m_StateFlag & GPSESF_CONNECTED);
		   bool connect1 = (physfaceA->m_Nodes[1]->m_StateFlag & GPSESF_CONNECTED);
		   bool connect2 = (physfaceA->m_Nodes[2]->m_StateFlag & GPSESF_CONNECTED);
		   if(connect0 && connect1 && connect2)
		   {
			   for(int n = 0 ; n < 3 ; n++)
			   {
				   std::map<GFPhysSoftBodyNode *, NearestNode>::const_iterator itor = AdherNodeMap.find(NodeA[n]);
				   if(itor != AdherNodeMap.end())
				   {
					   NodeB[n] = itor->second.m_Node;
				   }
			   }
			   if(NodeB[0] && NodeB[1] && NodeB[2])
			   {	
				   GFPhysSoftBodyFace * physfaceB = bodyB->GetSoftBodyShape().GetSurFace(NodeB[0] , NodeB[1] , NodeB[2]);
				   if(physfaceB)
				   {
					  if(m_HideLinkFace)
					     OriginFaceA.m_NeedRend = false;
					  OriginFaceA.m_physface->DisableCollideWithSoft();

					  
					  int bindex = organB.GetOriginFaceIndexFromUsrData(physfaceB);
					  
					  MMO_Face & OriginFaceB = organB.GetMMOFace_OriginPart(bindex);
					  
					  if(!OriginFaceB.m_HasError)
					  {
						  if(m_HideLinkFace)
						     OriginFaceB.m_NeedRend = false;
						  OriginFaceB.m_physface->DisableCollideWithSoft();
					  }
				   }
			   }
		   }
		}
	}
	if (organA.m_Serializer_NodeNum)
    {
        int i = 0;
        GFPhysSoftBodyNode * NodeAend = bodyA->GetNodeList();
        while(NodeAend )
        {
            NodeAend->m_UnDeformedPos = UnDeformedA[i];
            NodeAend = NodeAend->m_Next;
            i++;
        }       
    }
    delete []UnDeformedA;

	
	//for(size_t f = 0 ; f < m_ConnectOrganB.m_OriginFaces.size() ; f++)
	//{
	//	if(m_ConnectOrganB.m_OriginFaces[f].m_NeedRend == true)//this face not processed yet
	//	{
	//		GFPhysSoftBodyFace * physface = m_ConnectOrganB.m_OriginFaces[f].m_physface;
	//		bool connect0 = (physface->m_Nodes[0]->m_StateFlag & GPSESF_CONNECTED);
	//		bool connect1 = (physface->m_Nodes[1]->m_StateFlag & GPSESF_CONNECTED);
	//		bool connect2 = (physface->m_Nodes[2]->m_StateFlag & GPSESF_CONNECTED);
	//		if(connect0 && connect1 && connect2)
	//		{
	//			connectFaceB.insert(f);
	//		}
	//	}
	//}
}
*/
void MisMedicObjectAdhersion::BuildUniversalLinkFromAToB(const MisMedicOrgan_Ordinary & organA , const MisMedicOrgan_Ordinary & organB, float stiffness , float threshold )
{
	float StickThresHold = threshold;

	const GFPhysDBVTree & tetraBTrees = organB.m_physbody->GetSoftBodyShape().GetTetrahedronBVTree(false);
	
	GFPhysDBVTree NodeTreeA;

	GFPhysVector3 Extend(StickThresHold , StickThresHold , StickThresHold);
	
	//insert undeformed aabb to treeA
	GFPhysSoftBodyNode * NodeA = organA.m_physbody->GetNodeList();
	while(NodeA)
	{
		GFPhysVector3 NodePos = NodeA->m_UnDeformedPos;
		GFPhysDBVNode * dbvn = NodeTreeA.InsertAABBNode(NodePos-Extend , NodePos+Extend);
		dbvn->m_UserData = NodeA;
		NodeA = NodeA->m_Next;
	}

	//check those node lie in the threshold of A' s tetrahedron
	UniverseLinkCallBack NodesInsideCB(StickThresHold);
	
	NodeTreeA.CollideWithDBVTree(tetraBTrees , &NodesInsideCB);
	
	std::map<GFPhysSoftBodyNode * , NodeLieInTetra>::iterator nitor = NodesInsideCB.m_StickNodes.begin();
	
	while(nitor != NodesInsideCB.m_StickNodes.end())
	{
		NodeLieInTetra temp = nitor->second;

		GFPhysSoftBodyTetrahedron * tetra = temp.m_tetra;

		float TetraWeights[4];

		bool  gettedf = GetPointBarycentricCoordinate(  tetra->m_TetraNodes[0]->m_UnDeformedPos,
														tetra->m_TetraNodes[1]->m_UnDeformedPos,
														tetra->m_TetraNodes[2]->m_UnDeformedPos,
														tetra->m_TetraNodes[3]->m_UnDeformedPos,
														temp.m_ClosetPoint,
														TetraWeights);
		if(gettedf)
		{
			//temp.m_Node->m_InvM *= 0.2f;//increase mass
			//TetrahedronAttachConstraint * cs = new TetrahedronAttachConstraint( temp.m_Node , 
																				//tetra->m_TetraNodes,
																				//TetraWeights,
																				//4,
																				//1);

			//cs->SetStiffness(stiffness);
			//PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(cs);
			//m_constraints.push_back(cs);
			tetra->m_StateFlag |= GPSESF_CONNECTED;
			TetrahedronAttachConstraint tetracs(temp.m_Node , 
												tetra,//->m_TetraNodes,
												TetraWeights);
			tetracs.m_Stiffness = stiffness;
			m_NodeToTetraLinks.push_back(tetracs);

		}
		nitor++;
	}


	//now for every edge check whether intersect organ
	GFPhysSoftBodyEdge * edge = organA.m_physbody->GetEdgeList();
	while(edge)
	{
		bool isNode0Inside = false;
		

		bool isNode1Inside = false;

		if(NodesInsideCB.m_StickNodes.find(edge->m_Nodes[0]) != NodesInsideCB.m_StickNodes.end())
		{
			isNode0Inside = true;
		}

		if(NodesInsideCB.m_StickNodes.find(edge->m_Nodes[1]) != NodesInsideCB.m_StickNodes.end())
		{
			isNode1Inside = true;
		}
		GFPhysSoftBodyNode * edgeNodes[2];

		bool testRay = false;

		if(isNode0Inside == true && isNode1Inside == false)
		{
			edgeNodes[0] = edge->m_Nodes[0];
			edgeNodes[1] = edge->m_Nodes[1];
			testRay = true;
		}
		else if(isNode0Inside == false && isNode1Inside == true)
		{
			edgeNodes[0] = edge->m_Nodes[1];
			edgeNodes[1] = edge->m_Nodes[0];
			testRay = true;
		}

		//if(testRay)
		{
			edgeNodes[0] = edge->m_Nodes[0];
			edgeNodes[1] = edge->m_Nodes[1];
			GFPhysVector3  startRay = edgeNodes[0]->m_UnDeformedPos;
			
			GFPhysVector3  EndRay   = edgeNodes[1]->m_UnDeformedPos;
			
			UniverseEdgeFaceCallBack encallback(startRay , EndRay);

			organB.m_physbody->GetSoftBodyShape().TraverseFaceTreeAgainstRay(&encallback , startRay , EndRay , false);
			
			if(encallback.m_intersected == true)
			{
				//GFPhysSoftBodyNode * faceNodes[3];
				
				//faceNodes[0] = encallback.m_face->m_Nodes[0];
				//faceNodes[1] = encallback.m_face->m_Nodes[1];
				//faceNodes[2] = encallback.m_face->m_Nodes[2];

				FaceEdgePointAttachConstraint  cs(encallback.m_face , edge ,  encallback.m_FaceWeights , encallback.m_LineWeight , 1.0f , 1.0f);
				cs.m_Stiffness = stiffness;
				//PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(cs);
				
				//cs->SetStiffness(stiffness);
	
				m_FaceToEdgeLinks.push_back(cs);
			}
		}

		edge = edge->m_Next;
	}
}
void MisMedicObjectAdhersion::BuildNodeSurfaceLinkFromAToB(const MisMedicOrgan_Ordinary & organA, const MisMedicOrgan_Ordinary & organB, float stiffness, float threshold )
{
	float StickThresHold = threshold;

	GFPhysVectorObj<GFPhysDBVTree*> & faceBVTrees = organB.m_physbody->GetSoftBodyShape().GetFaceBVTrees(false);

	GFPhysDBVTree NodeTreeA;

	GFPhysVector3 Extend(StickThresHold, StickThresHold, StickThresHold);

	//insert undeformed aabb to treeA
	GFPhysSoftBodyNode * NodeA = organA.m_physbody->GetNodeList();
	while (NodeA)
	{
		GFPhysVector3 NodePos = NodeA->m_UnDeformedPos;
		GFPhysDBVNode * dbvn = NodeTreeA.InsertAABBNode(NodePos - Extend, NodePos + Extend);
		dbvn->m_UserData = NodeA;
		NodeA = NodeA->m_Next;
	}

	//check those node lie in the threshold of A' s tetrahedron
	UniverseLinkCallBack NodesInsideCB(StickThresHold , 1);

	for (int c = 0; c < faceBVTrees.size(); c++)
	{
		NodeTreeA.CollideWithDBVTree((*faceBVTrees[c]), &NodesInsideCB);
	}
	std::map<GFPhysSoftBodyNode *, NodeLieInTetra>::iterator nitor = NodesInsideCB.m_StickNodes.begin();

	while (nitor != NodesInsideCB.m_StickNodes.end())
	{
		NodeLieInTetra temp = nitor->second;

		GFPhysSoftBodyFace * face = temp.m_Face;

		float FaceWeights[3];

		CalcBaryCentric(face->m_Nodes[0]->m_UnDeformedPos,
			            face->m_Nodes[1]->m_UnDeformedPos,
			            face->m_Nodes[2]->m_UnDeformedPos,
						temp.m_ClosetPoint, FaceWeights[0], FaceWeights[1], FaceWeights[2]);
			
		
	
		face->m_StateFlag |= GPSESF_CONNECTED;
		
		TetrahedronAttachConstraint tetracs(temp.m_Node , face, FaceWeights);
		
		tetracs.m_Stiffness = stiffness;
	    
		m_NodeToTetraLinks.push_back(tetracs);

		
		nitor++;
	}

}
//=============================================================================================================
void MisMedicObjectAdhersion::BuildSpaceKeepLink(const MisMedicOrgan_Ordinary & organA , const MisMedicOrgan_Ordinary & organB , float stiffness , float threshold)
{
	m_AdhType = ADH_SPACEKEEP;

	float StickThresHold = threshold;

	const GFPhysDBVTree & FaceBTrees = organB.m_physbody->GetSoftBodyShape().GetFaceBVTree(false);

	GFPhysDBVTree NodeTreeA;

	GFPhysVector3 Extend(StickThresHold , StickThresHold , StickThresHold);

	//insert undeformed aabb to treeA
	//GFPhysSoftBodyFace * FaceA = organA.m_physbody->GetFaceList();
	//while(FaceA)
	for(size_t f = 0 ; f < organA.m_physbody->GetNumFace() ; f++)
	{
		GFPhysSoftBodyFace * FaceA = organA.m_physbody->GetFaceAtIndex(f);
		GFPhysVector3 CenterPos = (FaceA->m_Nodes[0]->m_UnDeformedPos
								  +FaceA->m_Nodes[1]->m_UnDeformedPos
								  +FaceA->m_Nodes[2]->m_UnDeformedPos)*0.333f;
		GFPhysDBVNode * dbvn = NodeTreeA.InsertAABBNode(CenterPos-Extend , CenterPos+Extend);
		dbvn->m_UserData = FaceA;
		//FaceA = FaceA->m_Next;
	}

	//check those node lie in the threshold of A' s tetrahedron
	FaceFaceCenterDistCallBack NodesFaceRangeCB(StickThresHold);

	NodeTreeA.CollideWithDBVTree(FaceBTrees , &NodesFaceRangeCB);

	std::map<GFPhysSoftBodyFace * , FaceClosetFace>::iterator nitor = NodesFaceRangeCB.m_ClostPairs.begin();

	std::set<GFPhysSoftBodyNode*> NodesInCs;

	int csNum = 0;
	while(nitor != NodesFaceRangeCB.m_ClostPairs.end())
	{
		FaceClosetFace temp = nitor->second;

		GFPhysSoftBodyFace * FaceA = temp.m_FaceA;
		GFPhysSoftBodyFace * FaceB = temp.m_FaceB;

		int existNum = 0;

		if(NodesInCs.find(FaceA->m_Nodes[0]) != NodesInCs.end())
		   existNum++;
		if(NodesInCs.find(FaceA->m_Nodes[1]) != NodesInCs.end())
		   existNum++;
		if(NodesInCs.find(FaceA->m_Nodes[2]) != NodesInCs.end())
		   existNum++;

		if(existNum <= 1)
		{
			Face_FaceConnection * FFCS = new Face_FaceConnection(FaceA , FaceB , 0.95f , 0.95f);
			PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(FFCS);
			m_constraints.push_back(FFCS);

			NodesInCs.insert(FaceA->m_Nodes[0]);
			NodesInCs.insert(FaceA->m_Nodes[1]);
			NodesInCs.insert(FaceA->m_Nodes[2]);
			csNum++;
		}
		nitor++;
	}
}
//=====================================================================================================================

void MisMedicObjectAdhersion::OnNodesRemoved(const GFPhysAlignedVectorObj<GFPhysSoftBodyNode*> & nodes)
{
	for(size_t c = 0 ; c < nodes.size() ; c++)
	{
		GFPhysSoftBodyNode * node = nodes[c];
		/*for(size_t t = 0 ; t < m_NodeToNodeLinks.size() ; t++)//temply brute force may add more good struct data
		{
			if((m_NodeToNodeLinks[t].m_NodeInA == node) || (m_NodeToNodeLinks[t].m_NodeInB == node))
			{
				m_NodeToNodeLinks[t].m_IsValid = false;
				m_NodeToNodeLinks[t].m_NodeInA = m_NodeToNodeLinks[t].m_NodeInB = 0;
			}
		}*/

		for(size_t t = 0 ; t < m_NodeToTetraLinks.size() ; t++)//temply brute force may add more good struct data
		{
			if(m_NodeToTetraLinks[t].m_AttachNode == node)
			{
			   m_NodeToTetraLinks[t].m_IsValid = false;
			   m_NodeToTetraLinks[t].m_AttachNode = 0;
			}
		}

		for(size_t c = 0 ; c < m_FaceToEdgeLinks.size() ; c++)
		{
			if(m_FaceToEdgeLinks[c].m_EdgeNodes[0] == node || m_FaceToEdgeLinks[c].m_EdgeNodes[1] == node)
			{	
			   m_FaceToEdgeLinks[c].m_IsValid = false;
			   m_FaceToEdgeLinks[c].m_AttachFace = 0;
			}
		}
	}
}
//=====================================================================================================================
void MisMedicObjectAdhersion::OnTetrasRemoved(const GFPhysAlignedVectorObj<GFPhysSoftBodyTetrahedron*> & tetras)
{
	for(size_t c = 0 ; c < tetras.size() ; c++)
	{
		GFPhysSoftBodyTetrahedron * tetra = tetras[c];
	
		for(size_t t = 0 ; t < m_NodeToTetraLinks.size() ; t++)//temply brute force may add more good struct data
		{
			if(m_NodeToTetraLinks[t].m_Tetra == tetra)
			{
				m_NodeToTetraLinks[t].m_IsValid = false;
				m_NodeToTetraLinks[t].m_AttachNode = 0;
				m_NodeToTetraLinks[t].m_Tetra = 0;
			}
		}
	}
}
//=====================================================================================================================

void MisMedicObjectAdhersion::OnFaceRemoved(GFPhysSoftBodyFace * face)
{
	if(m_AdhType == ADH_SPACEKEEP)
	{
		std::vector<GFPhysPositionConstraint*>::iterator itor = m_constraints.begin();
		while(itor != m_constraints.end())
		{
			GFPhysPositionConstraint * ps = (*itor);
			Face_FaceConnection * FFCS = dynamic_cast<Face_FaceConnection*>(ps);
			if(FFCS)
			{
				if(FFCS->m_Face[0] == face || FFCS->m_Face[1] == face)
				{
					PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(FFCS);
					delete FFCS;
					itor = m_constraints.erase(itor);
					continue;
				}
			}
			itor++;
		}
	}

	for(size_t c = 0 ; c < m_FaceToEdgeLinks.size() ; c++)
	{
		if(m_FaceToEdgeLinks[c].m_AttachFace == face)
		{	
		   m_FaceToEdgeLinks[c].m_IsValid = false;
		   m_FaceToEdgeLinks[c].m_AttachFace = 0;
		}
	}
}
//=============================================================================================================
//float NodeDistConstraintWrapper::GetCurrentDist()
//{
	//return m_pConstraint->m_Nodes[0]->m_CurrPosition.Distance(m_pConstraint->m_Nodes[1]->m_CurrPosition); 
//}

int MisMedicObjectEnvelop::NodeInfo::FindFaceIndex(std::vector<FaceInfo>& faces , GFPhysSoftBodyFace * pFace)
{
	for(size_t f = 0 ; f < m_OwnerFacesIndices.size() ; f++)
	{
		if(faces[m_OwnerFacesIndices[f]].m_pFace == pFace)
			return m_OwnerFacesIndices[f];
	}
	return -1;
}

void MisMedicObjectEnvelop::NodeInfo::OnFaceRemoved(int faceIndex)
{
	std::vector<int>::iterator itor = m_OwnerFacesIndices.begin();
	for( ; itor != m_OwnerFacesIndices.end() ; ++itor)
	{
		if(*itor == faceIndex) {
			m_OwnerFacesIndices.erase(itor);
			break;
		}
	}
	if(m_OwnerFacesIndices.empty())
		m_IsFacesAllRemoved = true;
}

void MisMedicObjectEnvelop::NodeInfo::RestoreStateOfFaces(std::vector<FaceInfo>& faces , bool isUnhide /* = true */)
{
	if(m_IsProcessed)
		return;

	m_IsProcessed = true;

	for(size_t f = 0 ; f < m_OwnerFacesIndices.size() ; f++)
	{
		FaceInfo & faceInfo =  faces[m_OwnerFacesIndices[f]];

		if(faceInfo.m_IsValid)
		{
			faceInfo.SetValid(false);

			GFPhysSoftBodyFace * pFace = faceInfo.m_pFace;

			pFace->EnableCollideWithRigid();

			pFace->EnableCollideWithSoft();
			
			if(isUnhide)
			{
				int index = m_pOrgan->GetOriginFaceIndexFromUsrData(pFace);

				MMO_Face & originFace = m_pOrgan->GetMMOFace_OriginPart(index);

				if(!originFace.m_HasError) {
					originFace.m_NeedRend = true;
					m_pOrgan->GetRender()->DirtyIndexBuffer();
				}
			}
			
		}
	}
}

MisMedicObjectEnvelop::MisMedicObjectEnvelop()
: m_OrganA(NULL) ,
	m_OrganB(NULL) ,
	m_IsValid(true) ,
	m_NumOfConsRemoved(0) , 
	m_NumOfConsRemovedWithFace(0) , 
	m_OrganABleedCount(0) , 
	m_OrganBBleedCount(0)
{
	PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(this);
};

MisMedicObjectEnvelop::~MisMedicObjectEnvelop()
{
	if(m_IsValid)
		RemoveConstraints();

	PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(this);
	
}

void MisMedicObjectEnvelop::PrepareSolveConstraint(Real Stiffness, Real TimeStep)
{
	for (size_t c = 0; c < m_NodeDistConstraints.size(); c++)
		m_NodeDistConstraints[c].m_Lambda = 0.0f;
}

void MisMedicObjectEnvelop::SolveConstraint(Real Stiffness, Real TimeStep)
{
	for (size_t c = 0; c < m_NodeDistConstraints.size(); c++)
	{
		NodeDistConstraintWrapper & distcs = m_NodeDistConstraints[c];
		
		if (distcs.IsValid() == false)
			continue;
		Real w1 = (distcs.m_Nodes[0]->m_InvM);
		
		Real w2 = (distcs.m_Nodes[1]->m_InvM);
		
		Real w = w1 + w2;
		
		assert(w1 >= 0 && w2 >= 0);

		if (w <= GP_EPSILON)
			continue;

		GFPhysVector3 Diff = distcs.m_Nodes[0]->m_CurrPosition - distcs.m_Nodes[1]->m_CurrPosition;
	
		float dlambda = Diff.Length() / (w1 + w2);
		distcs.m_Lambda += dlambda;

		GFPhysVector3 Deta1 = -(w1 / (w1 + w2)) * Diff;
		GFPhysVector3 Deta2 =  (w2 / (w1 + w2)) * Diff;

		distcs.m_Nodes[0]->m_CurrPosition += Deta1 * distcs.m_Stiffness;
		distcs.m_Nodes[1]->m_CurrPosition += Deta2 * distcs.m_Stiffness;
	}
}
//void MisMedicObjectEnvelop::FrameStarted()
//{

//}

void MisMedicObjectEnvelop::BuildEnvelop(MisMedicOrgan_Ordinary & organA , 
										 MisMedicOrgan_Ordinary & organB , 
										 float nodeThresHold ,
										 float ratio , 
										 bool isHideAFace /* = true */, 
										 bool isHideBFace /* = true */,
										 int organABleedCount /*= 0 */, 
										 int organBBleedCount/* = 0*/)
{
	m_OrganA = &organA;
	m_OrganB = &organB;

	BuildNodeDistConstraint(nodeThresHold , ratio);

	BuildNodesInfo();

	ProcessConnectedFace(isHideAFace , isHideBFace);

	m_OrganABleedCount = organABleedCount;
	m_OrganBBleedCount = organBBleedCount;

	//ConnectNodeWithFace();
}

float MisMedicObjectEnvelop::CheckTearConnect(float forceTear)
{
	float maxConnectForce = 0.0f;

	if(!m_IsValid)
		return maxConnectForce;

	for(size_t c = 0 ; c < m_NodeDistConstraints.size() ; c++)
	{
		NodeDistConstraintWrapper & distCons = m_NodeDistConstraints[c];

		if(distCons.IsValid())
		{
			float connectForce = distCons.m_Lambda;

			maxConnectForce = max(maxConnectForce, connectForce);

			if (connectForce >= forceTear)
			{
				distCons.SetValid(false);

				m_NumOfConsRemoved++;

				for(int n = 0 ; n < 2 ; n++)
				{
					GFPhysSoftBodyNode * pNode = distCons.m_Nodes[n];

					NodeInfoMap::iterator nodeIt= m_NodesInfoMap.find(pNode);

					if(nodeIt != m_NodesInfoMap.end())
					{
						NodeInfo & nodeInfo = nodeIt->second;

						nodeInfo.RestoreStateOfFaces(m_FacesInfoVec);
					}
				}
			}
		}
	}
	return maxConnectForce;
}

void MisMedicObjectEnvelop::RemoveConstraints()
{
	for(size_t c = 0 ; c < m_NodeDistConstraints.size() ; c++)
	{
		NodeDistConstraintWrapper & distCons = m_NodeDistConstraints[c];

		if(distCons.IsValid())
		{
			//PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(distCons.m_pConstraint);

			distCons.SetValid(false);

			for(int n = 0 ; n < 2 ; n++)
			{
				GFPhysSoftBodyNode * pNode = distCons.m_Nodes[n];

				NodeInfoMap::iterator nodeIt= m_NodesInfoMap.find(pNode);

				if(nodeIt != m_NodesInfoMap.end())
				{
					NodeInfo & nodeInfo = nodeIt->second;
					
					nodeInfo.RestoreStateOfFaces(m_FacesInfoVec);
				}
			}
		}
	}
	m_IsValid = false;
	m_ConnectFaceMap.clear();
	m_NumOfConsRemoved = m_NodeDistConstraints.size();
}

void MisMedicObjectEnvelop::OnFaceRemoved(GFPhysSoftBodyFace * pFace , GFPhysSoftBodyShape * pHostshape)
{
	if(!m_IsValid)
		return;

	int faceIndex = FindFaceIndex(pFace);

	if(faceIndex < 0)
		return;

// 	static const float weights[3] = {0.333 , 0.333 , 0.333};
// 	if(pHostshape->m_HostBody == m_OrganA->m_physbody && m_OrganABleedCount > 0)
// 	{
// 		m_OrganA->createBloodTrack(pFace, weights);// , 3 , false , 10.0f);
// 		m_OrganABleedCount--;
// 	}
// 	else if(pHostshape->m_HostBody == m_OrganB->m_physbody && m_OrganBBleedCount > 0)
// 	{
// 		m_OrganB->createBloodTrack(pFace, weights);// , 3 , false , 10.0f);
// 		m_OrganBBleedCount--;
// 	}

#if 0
	int removeCount = 0;
	for(size_t n = 0 ; n < 3; n++)
	{
		GFPhysSoftBodyNode * pNode =  pFace->m_Nodes[n];
		removeCount += RemoveConstraintWithNode(pNode);
	}
	m_NumOfConsRemoved += removeCount;
	m_NumOfConsRemovedWithFace += removeCount;
#else
	int removeCount = 0;
	for(size_t n = 0 ; n < 3; n++)
	{
		GFPhysSoftBodyNode * pNode =  pFace->m_Nodes[n];
		NodeInfoMap::iterator itor = m_NodesInfoMap.find(pNode);
		if(itor != m_NodesInfoMap.end())
		{
			NodeInfo & nodeInfo = itor->second;
			nodeInfo.OnFaceRemoved(faceIndex);
			if(nodeInfo.m_IsFacesAllRemoved)
				removeCount += RemoveConstraintWithNode(pNode);
		}
	}

	m_NumOfConsRemoved += removeCount;
	m_NumOfConsRemovedWithFace += removeCount;
	//直接使用连接的face
	FaceMap::iterator itor = m_ConnectFaceMap.find(pFace);
	if(itor != m_ConnectFaceMap.end())
	{
		GFPhysSoftBodyFace * pAnotherFace = itor->second;
		pFace->EnableCollideWithRigid();
		pFace->EnableCollideWithSoft();

		pAnotherFace->EnableCollideWithRigid();
		pAnotherFace->EnableCollideWithSoft();

		MisMedicOrgan_Ordinary *pOrgan = NULL;
		MisMedicOrgan_Ordinary *pAnotherOrgan = NULL;

		if(pHostshape->m_HostBody == m_OrganA->m_physbody) {
			pOrgan = m_OrganA;
			pAnotherOrgan = m_OrganB;
		} else {
			pOrgan = m_OrganB;
			pAnotherOrgan = m_OrganA;
		}

		int index1 = pOrgan->GetOriginFaceIndexFromUsrData(pFace);

		MMO_Face & OriginFace1 = pOrgan->GetMMOFace_OriginPart(index1);

		if(!OriginFace1.m_HasError) {
			OriginFace1.m_NeedRend = true;
		}

		int index2 = pAnotherOrgan->GetOriginFaceIndexFromUsrData(pAnotherFace);

		MMO_Face & OriginFace2 = pAnotherOrgan->GetMMOFace_OriginPart(index2);

		if(!OriginFace2.m_HasError) { 
			OriginFace2.m_NeedRend = true;
		}

		m_ConnectFaceMap.erase(itor);

		itor = m_ConnectFaceMap.find(pAnotherFace);
		if(itor != m_ConnectFaceMap.end()) {
			m_ConnectFaceMap.erase(itor);
		}
	

		int faceInfoIndex1 = FindFaceIndex(pFace);
		int faceInfoIndex2 = FindFaceIndex(pAnotherFace);

		if(faceInfoIndex1 >= 0)
			m_FacesInfoVec[faceInfoIndex1].SetValid(false);
		if(faceInfoIndex2 >= 0)
			m_FacesInfoVec[faceInfoIndex2].SetValid(false);

		pAnotherOrgan->GetRender()->DirtyIndexBuffer();
	}
#endif

}

void MisMedicObjectEnvelop::OnNodeRemoved(GFPhysSoftBodyNode * pNode, GFPhysSoftBodyShape * pHostshape)
{

}
void MisMedicObjectEnvelop::BuildNodeDistConstraint(float nodeThreshold , float ratio)
{

	GFPhysSoftBody * bodyA = m_OrganA->m_physbody;
	GFPhysSoftBody * bodyB = m_OrganB->m_physbody;

	GFPhysVector3 Extend(nodeThreshold , nodeThreshold , nodeThreshold);
	GFPhysDBVTree NodeTreeA;
	GFPhysDBVTree NodeTreeB;

	//insert undeformed aabb to treeA
	GFPhysSoftBodyNode * NodeA = bodyA->GetNodeList();
	while(NodeA)
	{
		GFPhysVector3 NodePos = NodeA->m_UnDeformedPos;
		GFPhysDBVNode * dbvn = NodeTreeA.InsertAABBNode(NodePos-Extend , NodePos+Extend);
		dbvn->m_UserData = NodeA;
		NodeA = NodeA->m_Next;
	}

	GFPhysSoftBodyNode * NodeB = bodyB->GetNodeList();
	while(NodeB)
	{
		GFPhysVector3 NodePos = NodeB->m_UnDeformedPos;
		GFPhysDBVNode * dbvn = NodeTreeB.InsertAABBNode(NodePos-Extend , NodePos+Extend);
		dbvn->m_UserData = NodeB;
		NodeB = NodeB->m_Next;
	}

	LinkedOrganNodeCallBack nodeCallBack(nodeThreshold);
	NodeTreeA.CollideWithDBVTree(NodeTreeB , &nodeCallBack);

	const std::map<GFPhysSoftBodyNode *, NearestNode> & AdherNodeMap = nodeCallBack.m_NearestNodes;
	std::map<GFPhysSoftBodyNode *, NearestNode>::const_iterator itor = AdherNodeMap.begin();

	float customWeights[2] = { ratio  , 1.0f};

	while(itor != AdherNodeMap.end())
	{
		GFPhysSoftBodyNode * NodeA = itor->first;
		GFPhysSoftBodyNode * NodeB = itor->second.m_Node;

		//GFPhysSoftBodyNodeDistConstraint * cs = new GFPhysSoftBodyNodeDistConstraint();

		NodeA->m_CurrPosition  = NodeB->m_CurrPosition;
		NodeA->m_UnDeformedPos = NodeB->m_UnDeformedPos;

		NodeA->m_StateFlag |= GPSESF_CONNECTED;
		NodeB->m_StateFlag |= GPSESF_CONNECTED;

		//cs->SetNodes(NodeA  , NodeB , 0);
		//cs->SetStiffness(1.0f, 1.0f);
		
		//PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(cs);

		m_NodeDistConstraints.push_back(NodeDistConstraintWrapper(NodeA, NodeB , 1.0f));

		m_NearestNodeMap.insert(std::make_pair(NodeA  , NodeB));

		//m_NodesBeConnect.push_back(std::make_pair(NodeA  , NodeB));
		itor++;
	}
}

void MisMedicObjectEnvelop::BuildNodesInfo()
{
	for(size_t c = 0 ; c < m_NodeDistConstraints.size() ; c++)
	{
		NodeDistConstraintWrapper & constraint = m_NodeDistConstraints[c];
		for(size_t n = 0 ; n < 2 ; n++)
		{
			GFPhysSoftBodyNode * pNode = constraint.m_Nodes[n];
			NodeInfoMap::iterator itor = m_NodesInfoMap.find(pNode);
			if(itor != m_NodesInfoMap.end()) {
				NodeInfo & nodeInfo = itor->second;
				nodeInfo.m_ConstraintIndices.push_back(c);
			} else {
				NodeInfo nodeInfo;
				nodeInfo.m_ConstraintIndices.push_back(c);
				m_NodesInfoMap.insert(std::make_pair(pNode ,nodeInfo));
			}
		}
	}

}

void MisMedicObjectEnvelop::ProcessConnectedFace(bool isHideAFace /* = true */, bool isHideBFace /* = true */)
{
	GFPhysSoftBody * bodyA = m_OrganA->m_physbody;
	GFPhysSoftBody * bodyB = m_OrganB->m_physbody;

	for(size_t f = 0 ; f < m_OrganA->m_OriginFaces.size() ; f++)
	{
		MMO_Face & OriginFaceA = m_OrganA->m_OriginFaces[f];
		if(OriginFaceA.m_NeedRend == true)//this face not processed yet
		{
			GFPhysSoftBodyFace * physfaceA = m_OrganA->m_OriginFaces[f].m_physface;
			GFPhysSoftBodyNode * NodeA[3];
			GFPhysSoftBodyNode * NodeB[3];

			NodeA[0] = physfaceA->m_Nodes[0];
			NodeA[1] = physfaceA->m_Nodes[1];
			NodeA[2] = physfaceA->m_Nodes[2];

			NodeB[0] = NodeB[1] = NodeB[2] = 0;

			bool connect0 = (physfaceA->m_Nodes[0]->m_StateFlag & GPSESF_CONNECTED);
			bool connect1 = (physfaceA->m_Nodes[1]->m_StateFlag & GPSESF_CONNECTED);
			bool connect2 = (physfaceA->m_Nodes[2]->m_StateFlag & GPSESF_CONNECTED);

			if(connect0 && connect1 && connect2)
			{
				for(int n = 0 ; n < 3 ; n++)
				{
					NearestNodeMap::iterator itor = m_NearestNodeMap.find(NodeA[n]);
					if(itor != m_NearestNodeMap.end())
					{
						NodeB[n] = itor->second;
					}
				}
				if(NodeB[0] && NodeB[1] && NodeB[2])
				{	
					GFPhysSoftBodyFace * physfaceB = bodyB->GetSoftBodyShape().GetSurFace(NodeB[0] , NodeB[1] , NodeB[2]);
					if(physfaceB)
					{
						if(isHideAFace)
							OriginFaceA.m_NeedRend = false;

						OriginFaceA.m_physface->DisableCollideWithSoft();
						OriginFaceA.m_physface->DisableCollideWithRigid();

						m_FacesInfoVec.push_back(FaceInfo(OriginFaceA.m_physface));

						m_ConnectFaceMap.insert(std::make_pair(physfaceA , physfaceB));
						m_ConnectFaceMap.insert(std::make_pair(physfaceB , physfaceA));

						for(size_t nA = 0 ; nA < 3 ; nA++)
						{
							NodeInfoMap::iterator itor = m_NodesInfoMap.find(NodeA[nA]);
							if(itor != m_NodesInfoMap.end()){
								NodeInfo & nodeInfo = itor->second;
								nodeInfo.m_pOrgan = m_OrganA;
								//node保存拥有它的面
								nodeInfo.m_OwnerFacesIndices.push_back(m_FacesInfoVec.size() - 1);		
							}
						}

						int bindex = m_OrganB->GetOriginFaceIndexFromUsrData(physfaceB);

						MMO_Face & OriginFaceB = m_OrganB->GetMMOFace_OriginPart(bindex);

						if(!OriginFaceB.m_HasError)
						{
							if(isHideBFace)
								OriginFaceB.m_NeedRend = false;
							OriginFaceB.m_physface->DisableCollideWithSoft();
							OriginFaceB.m_physface->DisableCollideWithRigid();

							m_FacesInfoVec.push_back(FaceInfo(OriginFaceB.m_physface));

							for(size_t nB = 0 ; nB < 3 ; nB++)
							{
								NodeInfoMap::iterator itor = m_NodesInfoMap.find(NodeB[nB]);
								if(itor != m_NodesInfoMap.end()){
									NodeInfo & nodeInfo = itor->second;
									nodeInfo.m_pOrgan = m_OrganB;
									//node保存拥有它的面
									nodeInfo.m_OwnerFacesIndices.push_back(m_FacesInfoVec.size() - 1);
								}
							}
						}
					}
				}
			}
		}
	}
}

void MisMedicObjectEnvelop::ConnectNodeWithFace()
{
	for(size_t f = 0 ; f < m_FacesInfoVec.size() ; f++)
	{
		FaceInfo & faceInfo = m_FacesInfoVec[f];
		GFPhysSoftBodyFace * pFace = faceInfo.m_pFace;
		for(size_t n = 0 ; n < 3 ; n++)
		{
			GFPhysSoftBodyNode *pNode = pFace->m_Nodes[n];
			NodeInfoMap::iterator itor =  m_NodesInfoMap.find(pNode);
			if(itor != m_NodesInfoMap.end())
			{
				NodeInfo & nodeInfo = itor->second;
				nodeInfo.m_OwnerFacesIndices.push_back(f);
			}
		}
	}
}

int MisMedicObjectEnvelop::RemoveConstraintWithNode(GFPhysSoftBodyNode *pNode)
{
	int removeCount = 0;
	NodeInfoMap::iterator itor = m_NodesInfoMap.find(pNode);
	if(itor != m_NodesInfoMap.end())
	{
		NodeInfo & nodeInfo = itor->second;

		nodeInfo.RestoreStateOfFaces(m_FacesInfoVec);

		std::vector<int> & consIndices = nodeInfo.m_ConstraintIndices;

		for(size_t c = 0 ; c < consIndices.size() ; c++)
		{
			NodeDistConstraintWrapper & distCons = m_NodeDistConstraints[consIndices[c]];
			if(distCons.IsValid())
			{
				distCons.SetValid(false);

				removeCount++;

				//PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(distCons.m_pConstraint);

				GFPhysSoftBodyNode * pAnotherNode = (distCons.m_Nodes[0] == pNode ? distCons.m_Nodes[1] : distCons.m_Nodes[0]);
				
				NodeInfoMap::iterator anotherItor = m_NodesInfoMap.find(pAnotherNode);
				if(anotherItor != m_NodesInfoMap.end()){
					anotherItor->second.RestoreStateOfFaces(m_FacesInfoVec);
				}
			}
		}
	}
	return removeCount;
}

int MisMedicObjectEnvelop::RemoveConstraintWithNode(MisMedicObjectEnvelop::NodeInfo & nodeInfo)
{
	int removeCount = 0;

	nodeInfo.RestoreStateOfFaces(m_FacesInfoVec);

	std::vector<int> & consIndices = nodeInfo.m_ConstraintIndices;

	for(size_t c = 0 ; c < consIndices.size() ; c++)
	{
		NodeDistConstraintWrapper & distCons = m_NodeDistConstraints[consIndices[c]];
		if(distCons.IsValid())
		{
			distCons.SetValid(false);

			removeCount++;

			//PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(distCons.m_pConstraint);

			GFPhysSoftBodyNode *pAnotherNode = (distCons.m_Nodes[0] == NULL ? distCons.m_Nodes[1] : distCons.m_Nodes[0]);

			NodeInfoMap::iterator anotherItor = m_NodesInfoMap.find(pAnotherNode);
			if(anotherItor != m_NodesInfoMap.end()){
				anotherItor->second.RestoreStateOfFaces(m_FacesInfoVec);
			}
		}
	}
	return removeCount;
}

int MisMedicObjectEnvelop::FindFaceIndex(GFPhysSoftBodyFace * pFace)
{
	if(pFace != NULL)
	{
		for(size_t n = 0 ; n < 3 ; n++)
		{
			GFPhysSoftBodyNode *pNode = pFace->m_Nodes[n];
			NodeInfoMap::iterator itor = m_NodesInfoMap.find(pNode);
			if(itor != m_NodesInfoMap.end())
			{
				NodeInfo & nodeInfo = itor->second;
				int indexFound = nodeInfo.FindFaceIndex(m_FacesInfoVec , pFace);
				if(indexFound >= 0)
					return indexFound;
			}
		}
	}
	return -1;
}