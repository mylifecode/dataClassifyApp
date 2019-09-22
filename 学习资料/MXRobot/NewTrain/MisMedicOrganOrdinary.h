#ifndef _MISMEDICORGANORDINARY_
#define _MISMEDICORGANORDINARY_
#include "MisMedicOrganInterface.h"
#include "Topology/GoPhysSoftBodyCutter.h"
#include "MisMedicObjectSerializer.h"
#include "MisOrganAnimation.h"
#include <OgreMatrix4.h>
#include "MisMedicOrganRender.h"
#include "MisMedicOrganAttachment.h"
#include "MisMedicObjLink_Approach.h"
#include "VolumeBlood.h"

class VolumeBlood;

//#include "Painting.h"
#define USEOLDRENDOBJECT 0

class CTool;
struct Tetrahedron
{
	int vertid[4];
};
//application  data for physics node
struct PhysNode_Data
{
public:
	PhysNode_Data();
	GFPhysSoftBodyNode * m_PhysNode;
	int m_ShadowNodeIndex;
	//GFPhysSoftBodyNode * m_PosRedirectNode;//
	Ogre::Vector3 m_AvgNormal;
	float m_LayerBlendFactor;

	int m_NextFreed;
	int   m_TempBuildId;//for rebuild rend node use
	float m_ForceFeedScale;
	//float m_bloodValue;
	//float m_burnValue;
	//bool  m_NodeInCutEdge;
	bool  m_HasError;
	//Ogre::Vector2 m_NodeTextureCoord;
	//Ogre::Vector2 m_TextureCoord;//temp
	int   m_Dist;//use by train 11 12
	//std::vector<int> m_AdjNodeIndex;//相邻节点的索引
	//int m_SepPartID;
	//int m_NextPartIndex;
	bool m_AddSMForce;

	int m_NodeBeFixed;//include soft fix and hard fix
	
	std::map<Ogre::String,int> m_AttributeMap;
};

//application data for physics tetra
struct PhysTetra_Data
{
public:
	PhysTetra_Data();
	PhysTetra_Data(GFPhysSoftBodyTetrahedron* phystetra , int organId , bool ismens);
	GFPhysSoftBodyTetrahedron* m_PhysTetra;
	//Ogre::Vector2 m_NodeTexture[4];
	//bool m_IsTextureSetted;

	int   m_Layer;//for multilayer
	int   m_OrganID;//
	bool  m_IsMenstary;
	bool  m_ContainsVessel;
	bool  m_HasError;
	int m_NextFreed;

};


//每个器官的连通子集，切割后会重新计算
/*class OrganSubPart
{
public:
	OrganSubPart()
	{
		m_PartStateFlag = 0;
	}
	std::vector<GFPhysSoftBodyNode*> m_Nodes;
	std::vector<GFPhysSoftBodyTetrahedron*> m_Tetras;
	int m_SPartId;
	unsigned int m_PartStateFlag;
	//GFPhysVector3 m_GravityMoveDist;
};*/

struct TitanicClipInfo
{
	TitanicClipInfo() : m_IsClip(false) {}
	TitanicClipInfo(int nodeIndex , float maxDistance) : m_NeedClipNodeIndex(nodeIndex)  , m_maxDistance(maxDistance) , m_IsClip(false) {}
	//TitanicClipInfo(double minv , double maxv) : m_minv(minv)  , m_maxv(min) , m_IsClip(false) {}
	int m_NeedClipNodeIndex;					//需要检测距离的node
	float m_maxDistance;							//与node相距距离在此范围内为成功施放钛夹
	double m_minv,m_maxv;
	bool m_IsClip;										//
	static bool s_clipInValidReg;					//用来标记上一次是否成功施放
	static int  s_clipEmptyCount;						//掉落的钛夹数量
	std::vector<GFPhysSoftBodyFace *> m_facesSatisfied;
};

struct ElecCutInfo
{
	ElecCutInfo():m_isCut(false){}
	ElecCutInfo(int nodeIndex , float maxDistance) : m_DistNodeIndex(nodeIndex)  , m_maxDistance(maxDistance) , m_isCut(false) {}
	//ElecCutInfo(double minv , double maxv) : m_minv(minv)  , m_maxv(maxv) ,m_isCut(false) {}
	int m_DistNodeIndex;					//需要检测距离的node
	double m_maxDistance;
	double m_minv,m_maxv;
	bool m_isCut;
	static bool s_cutSucceed;
	std::vector<GFPhysSoftBodyFace *> m_facesSatisfied;
};


class FaceInScatterBlood
{
public:
	FaceInScatterBlood(GFPhysSoftBodyFace * PhysFace)
	{
		m_PhysFace = PhysFace;
		m_BloodTime = 0;
	}

	bool operator == (const FaceInScatterBlood & rth) const
	{
		return rth.m_PhysFace == m_PhysFace;
	}

	bool operator < (const FaceInScatterBlood & rth) const 
	{
		return rth.m_PhysFace < m_PhysFace;
	}

	//int   m_FaceIndex;
	GFPhysSoftBodyFace * m_PhysFace;
	float m_BloodTime;
};

class OrganInjuryPoint
{
public:
	OrganInjuryPoint()
	{
		m_Face = 0;
		m_FaceUID = -1;
		m_VolumeBlood = 0;
	}
	OrganInjuryPoint(GFPhysSoftBodyFace * Face,
		             float weights[3],
					 VolumeBlood * vblood
		             )
	{
		m_Face = Face;
		m_FaceUID = Face->m_uid;
		m_weights[0] = weights[0];
		m_weights[1] = weights[1];
		m_weights[2] = weights[2];
		m_BleedTexCoord.x = Face->m_TexCoordU[0] * weights[0]
			              + Face->m_TexCoordU[1] * weights[1]
			              + Face->m_TexCoordU[2] * weights[2];

		m_BleedTexCoord.y = Face->m_TexCoordV[0] * weights[0]
			              + Face->m_TexCoordV[1] * weights[1]
			              + Face->m_TexCoordV[2] * weights[2];

		m_OriginPos = Face->m_Nodes[0]->m_UnDeformedPos * weights[0]
			        + Face->m_Nodes[1]->m_UnDeformedPos * weights[1]
			        + Face->m_Nodes[2]->m_UnDeformedPos * weights[2];
		m_VolumeBlood = vblood;
	}
	Ogre::Vector2 m_BleedTexCoord;
	GFPhysVector3 m_OriginPos;
	GFPhysSoftBodyFace * m_Face;
	float m_weights[3];
	int m_FaceUID;
	VolumeBlood * m_VolumeBlood;
};


class OrganSurfaceBloodTextureTrack;
class CVesselBleedEffect;
class DynamicBloodPoint;

class MisMedicOrgan_Ordinary : public MisMedicOrganInterface//, public GFPhysSoftBodyCutter::GFPhysSoftBodyCutListener
{
	friend class MisMedicObjetSerializer;
public:
	struct	SBleedPoint
	{
		GFPhysSoftBodyNode* node;
		CVesselBleedEffect* effect;
		SBleedPoint(GFPhysSoftBodyNode* n, CVesselBleedEffect* e)
		{
			node = n;
			effect = e;
		}
	};
	MisMedicOrgan_Ordinary(DynamicObjType organtype, int organId,CBasicTraining * ownertraing);

	~MisMedicOrgan_Ordinary();

	int GetNumSubParts();

	GFPhysSubConnectPart * GetSubPart(int index);
	//********************************************************************************************************
	//@@@@ Cut & topology modify Function Start 
	//@@ Note! call your cut , tear remove operation of tissue by using these function!! do not do your self
	//********************************************************************************************************
	virtual void CutOrganByTool(CTool * tool);//cut this organ by a tool , the tool should have blade define see tool

	virtual void DestroyTissueAroundNode(GFPhysSoftBodyFace * physFace , float weights[3] ,/*const GFPhysVector3 & centerPos ,*/ bool indeformspace =true);//remove tetrahedron share same node

	virtual void TearOrganBySemiInfinteQuad(GFPhysVector3 quadVerts[4] , bool InDeformSpace = true);//tear this organ across the quad vertex composed plane

	virtual void TearOrganByCustomedGeomtry(GFPhysISoftBodyCutSurface * cutsurface , bool InDeformSpace = true);//tear this organ across the customed cut surface
	
	virtual void EliminateTetras(const GFPhysVectorObj<GFPhysSoftBodyTetrahedron *> & tetrasToRemove);

	virtual void RemoveSubParts(int subPartIndex);

	void SetTetraCollapseParam(float ratio , int minCutTimes);
	//********************************************************************************************************
	//@@ Cut Function End
	//***********************************************************

	void ShrinkTetrahedrons(const GFPhysVectorObj<GFPhysSoftBodyTetrahedron*> & TetrasToShrink ,
							const GFPhysVector3 & shrinkDirInMaterialSpace ,
							float shrinkFactor = 0.98f);
	
	void SetEletricCutParameter(float cutRadius , int maxcutCount);


	//@@ Get Last Cut or Tetrahedron Eliminate operation Generated Cut Cross Face
	void GetLastTimeCutCrossFaces(std::vector<GFPhysSoftBodyFace*> & result);

	virtual MisMedicTitaniumClampV2 * CreateTitanic(float invalidClipLen,
		                       Ogre::Vector3 clipAxis[3] ,
							   Ogre::Vector2 clipBound[3],
							   GFPhysSoftBodyFace * attachFace ,
							   int toolFlag);

	virtual void AddOrganAttachment(MisMedicOrganAttachment * attach);

	virtual void RemoveOrganAttachment(MisMedicOrganAttachment * attach);//simple remove but not delete

	virtual Ogre::Vector2 GetTextureCoord(GFPhysSoftBodyFace * face , float weights[3]);
	
	virtual void PreUpdateScene(float dt , Ogre::Camera * camera);

	virtual void UpdateScene(float dt, Ogre::Camera * camera);

	void UpdateNonWoundBlood( float dt );

	virtual void StopAroundTexBloodGradual(const Ogre::Vector2 & centerTexCoord);
	//touch type 0--电凝 touch type 1---电切  touch type 2 ---电断器官间连接部分 
	virtual void Tool_InElec_TouchFacePoint(ITool * tool , GFPhysSoftBodyFace * face , float weights[3],int touchtype , float dt);

	virtual void InjectSomething(ITool *tool , GFPhysSoftBodyFace * face , float weights[3] , float dt , std::vector<Ogre::Vector2> & resultUv);
		
	virtual void ToolElectricClampedFaces(ITool * tool , const std::vector<Ogre::Vector2> & touchfaces, const std::vector<Ogre::Vector2> & TFUV , float dt);
	
	//virtual void ToolWithElectricTouched_uv_3(ITool * tool , const std::vector<Ogre::Vector2> & touchfaces, const std::vector<Ogre::Vector2> & TFUV , float dt , int n = 3 , Ogre::Real dU = 0.005 , Ogre::Real dV = 0.005);

	virtual void ToolPunctureSurface(ITool * tool , GFPhysSoftBodyFace * face , const float weights[3]);
	virtual void RemovePhysicsPart();
	virtual void RemoveGraphicPart();

	virtual void ReadOrganObjectFile(MisMedicDynObjConstructInfo & constructInfo);
	virtual void Create(MisMedicDynObjConstructInfo & constructInfo);
	virtual void PostLoadScene();


	PhysNode_Data & GetPhysNodeData(GFPhysSoftBodyNode * node);

	PhysTetra_Data & GetPhysTetraAppData(GFPhysSoftBodyTetrahedron * tera);

	//@ get origin MMO face
	MMO_Face & GetMMOFace_OriginPart(int FaceIndex);
	
	//@get cut MMO face
	MMO_Face & GetMMOFace_CutPart(int FaceIndex);
	
    MMO_Face & GetMMOFace(GFPhysSoftBodyFace * physface);

	//@Get MMONode
	const MMO_Node & GetMMONode(int NodeIndex);

	//@Get MMONode Position
	Ogre::Vector3 getMMO_NODE_Pos(int index);

	//@Get 3 Nodes' texture coordinate of this face
	bool GetMMOFaceNodesTextureCoord(int faceindex , Ogre::Vector2 textureCoord[3]);
	
	bool ApplyEffect_BurnWhite(CTool * hostTool,
							   GFPhysSoftBodyFace * face , 
							   float weights[3] , 
							   float EffectValue , 
							   Ogre::Vector2 & resultTexCoord , 
							   float KeepTime);


	void ApplyEffect_Bleeding(Ogre::Vector2 texturecoord, float radius, float heatvalue, Ogre::TexturePtr bleedingPointTex);

	void ApplyEffect_VolumeBlood(int faceID, float* pFaceWeight);
	void StopEffect_VolumeBlood();
	void SetVolumeBloodParameter(float radius, float flow);

	

	void SetTimeNeedToEletricCut(float timeneed);
	void SetBloodScatterValue(float);
	void SetBloodScatterRadius(float);

	//获取当前器官上的纹理血流数量
	int  GetBloodTrackCount();//包括在剪断的面片产生的流血效果

	//获取一条纹理血流
	OrganSurfaceBloodTextureTrack * GetBloodTrack(int streamIndex);
	
	//获取血流开始的MMO_Face索引
	int GetBloodTrackStartFaceID(int streamIndex);

	//在任意的面片上增加一个动态流血效果
	bool AddBloodTrackAtFace(int faceID);

	void SetBleedRadius(float radius);

	void AddInjuryPoint(GFPhysSoftBodyFace * Face, float weights[3], VolumeBlood * vblood);

	//void RemoveInjuryPoint(int index);

	void RemoveInjuryPointsOnFaces(GFPhysVectorObj<GFPhysSoftBodyFace*> & faces);

	void RemoveInjuryPoints(const Ogre::Vector2 & tex , float range);

	int GetNumOfDynamicBlood();

	/**
		获取当前器官上所有流血点中最长的流血时间
	*/ 
	float GetCurMaxBleedTime();

	static int   GetOriginFaceIndexFromUsrData(GFPhysSoftBodyFace * face);
	static int   GetCrossFaceIndexFromUsrData(GFPhysSoftBodyFace * face);
	static void  ExtractFaceIdAndMaterialIdFromUsrData(GFPhysSoftBodyFace * face , int & materialid , int & faceid);
	static void  ExtractFaceIdAndMaterialIdFromUsrData(int userData , int & materialid , int & faceid);
	//iswound = false 时 非创伤 流一段时间自动停止
	bool createBloodTrack(GFPhysSoftBodyFace * face , const float Roughweights[3], int nMaxNum = 3 , bool IsWound = true , float bleedingTime = 2.0f);

	OrganSurfaceBloodTextureTrack *  createBloodTrack(GFPhysSoftBodyFace * face , const float Roughweights[3], float fDir, bool bPos , bool IsWound = true , float bleedingTime = 2.0f);
	//__/__创建器官表面的一个水流
	OrganSurfaceBloodTextureTrack *  createWaterTrack(GFPhysSoftBodyFace * face , const float Roughweights[3], float fDir, bool bPos );
	//在指定的面片上创建一个动态出血点效果
	DynamicBloodPoint * createDynamicBloodPoint(GFPhysSoftBodyFace * pFace,const float weights[3]);
	//移除指定面片上的动态出血点
	bool removeDynamicBloodPoint(GFPhysSoftBodyFace * pFace);

	void SetMaxBloodTrackExistsTime(float time);

	void SetBloodPointDensity(float density);
	void SetMaxBloodTrackCount(int cmax);


	//void CreateAdditionalBendingForce();

	
	void CollectTetrasAroundPoint(const GFPhysVector3 & point ,
								  float radius , 
								  std::vector<GFPhysSoftBodyTetrahedron*> & tetraInrange , 
								  int Catageory = (~0),
								  bool indeformspace = true);
	
	void HeatTetrahedrons(const GFPhysVectorObj<GFPhysSoftBodyTetrahedron*> & tetrasToHeat , float deltavalue);

	void HeatAroundUndeformedPoint(const GFPhysVector3 & point , float radius , float heatValue);
	
	void SelectPhysFaceAroundPoint(std::vector<GFPhysSoftBodyFace*> & ResultFaces , const GFPhysVector3 & pointPos , float radius , bool deformedspace = false);
    
	void DisRigidCollAroundPoint(std::vector<GFPhysSoftBodyFace*>& aroundfaces, const GFPhysVector3 & pointPos, Real threshold, bool deformedspace = false);
	
	void CollectFacesAroundPoint(std::vector<GFPhysSoftBodyFace*>& aroundfaces, const GFPhysVector3 & pointPos, Real threshold, bool deformedspace = false);
	
    void CopyBurnTexture();
	uint32 GetBurnTexPixel(const Ogre::Vector2 & texCoord);

	void SetBloodSystemGravityDir(Ogre::Vector3 & gravity);

	void SetIsolatedPartGravity(float gvalue);

	void CalcCurrentComAndOrient(GFPhysVector3 & currcom , GFPhysMatrix3 & rotateMat , GFPhysVector3 & restcom);

	static void CalcCurrentComAndOrient(GFPhysVector3 & currcom , GFPhysMatrix3 & rotateMat , GFPhysVector3 & restcom , std::vector<GFPhysSoftBodyNode*> & nodeCluods);

	//void EnableFakeGravity(float gvalue);

	//used for translate the organ after the initialization of the training 
	void TranslateUndeformedPosition(GFPhysVector3 & offset);

	
	//设置变换矩阵
	//对模型的原始顶点数据进行变换矩阵（平移、旋转、缩放.），在create函数调用以前设置才有效
	inline void setTransfromMatrix(const Ogre::Matrix4& matrix)
	{
		m_transformMatrix = matrix;
		m_bUseTransformForMaxData = true;
	}

	bool RemoveAndDestoryChild(const Ogre::String& name);

	void SetBurnNodeColor(Ogre::ColourValue & colorValue);
	Ogre::SceneNode* getSceneNode();
	Ogre::String getMaterialName();
	//void Reset();

	void CreateBloodStreamInCutFace(const GFPhysVector3 & cutPlaneNorm);

	void CreatePoseRigidForce();

	void TestLinkingArea(int MarkFlag , std::vector<Ogre::Vector2> & reginInfo);

	virtual void setAttchmentFlag(int flag, bool isRemove);

	void ApplyEffect_Soak(Ogre::Vector3 porigin, Ogre::Vector3 pnormal);//器官浸入血池的血渍纹理

	void stopVesselBleedEffect();
	void resumeVesselBleedEffect();
	//添加一个流血点,改点将显示动态流血效果
	//bool addBleedPoint(GFPhysSoftBodyNode * pNode);

	void setVesselBleedEffectTempalteName(Ogre::String templateName);

	void SetBleedWhenStripBreak(bool bleed)
	{
		 m_BleedWhenStripBreak = bleed;
	}

	bool IsBleedWhenStripBreak()
	{
		 return m_BleedWhenStripBreak;
	}

	void GetSortedNodeIndex(std::vector<int> &results,int &nodeNum);

	void setVisible(bool vis)
	{
		m_Visible = vis;
		if (m_pManualObject)
		{
			m_pManualObject->setVisible(vis);
		}
	}

	void SetOrdinaryMatrial(const Ogre::String& matiralName , int layer = 0);

	void ChangeTexture(Ogre::TexturePtr texture,const Ogre::String& textureunitname);

	void ChangeTexture(const Ogre::String & texname , const Ogre::String& textureunitname);

	inline	std::vector<SBleedPoint>& GetBleedPoint(){return m_bleednodes;}
	
	
	//
	void ScaleSerializerNodeByDir(const float factor , const GFPhysVector3 & dir);

	virtual void CreateSerializerNodeTree(const float extend);

	virtual const GFPhysDBVTree & GetSerializerNodeTree();

	virtual const GFPhysDBVTree & GetSerializerFaceTree();

	virtual const GFPhysDBVTree & GetSerializerTetraTree();

	GFPhysVector3 GetSerializerNodePos(int index);

	const MisMedicObjetSerializer::MisSerialFace & GetSerializerFace(int index);

	const MisMedicObjetSerializer::MisSerialTetra & GetSerializerTetra(int index);

    inline MisMedicObjetSerializer& GetSerializer(){return m_Serializer;}

	void SetBaseTextureMix(float fMix);

    GFPhysSoftBodyFace * GetRayIntersectFace(const Ogre::Vector3 rayStart ,const Ogre::Vector3 rayEnd, Real& dist);

    
    
	void SetBloodColor(Ogre::ColourValue & colorValue);

	float m_BurnShrinkRate;

	float m_CutWidthScale;

	void BuildTetrahedronNodeTextureCoord(const GFPhysVector3 & posFaceDir, bool usePosFaceDir);

	int m_frameCount;

	MisMedicEndoGiaClips* GetEndoGiaClips();
	void MapSurfaceToMorphedMesh(Ogre::MeshPtr & meshToMap);
	
	MisMedicOrganRender * GetRender(){ return m_pManualObject; }

private:

	void SetBloodAndBurnColorForMaterial();
	
    virtual void RefreshDirtyData();

	virtual void ReDistributeMass();

	virtual void InternalSimulateStart(int currStep , int TotalStep , Real dt);

	virtual void InternalSimulateEnd(int currStep , int TotalStep , Real dt);

	//@CutBySemiInfinitQuadImp do cut and build relation ship from physics face to origin face
	//but not build rend vertex and index
	void CutBySemiInfinitQuadImp(float cutWidth , GFPhysVector3 quadVerts[4] , bool InDeformSpace);

	//@ElectricCutMensetaryImp do cut and eliminate tetra and build relation ship from physics face to origin face
	//but not build rend vertex and index
	//void ElectricCutMensetaryImp(CTool * toolcut);

	//call when face change by cut etc
	void PostModifyTopology(const GFPhysAlignedVectorObj<GFPhysCuttedNewFace> & splitfaceData , 
						    const GFPhysAlignedVectorObj<GFPhysCuttedNewFace> & newCutfaceData ,
						    const GFPhysAlignedVectorObj<GFPhysNodeCreatedBySplit> & nodesCreateData ,
						    std::vector<FaceSplitByCut> & FacesSplitted);//,
						    //std::vector<Ogre::Vector2>  & organTexCoord);

	void ExtractCutInformation(std::vector<Ogre::Vector2>  & cuttedTexCoord , GFPhysVector3 cutQuad[4]);

	//void CreateBloodScatterPointInCutCrossFace();

	virtual void PerformElectricCut(ITool * tool , GFPhysSoftBodyFace * face , float weights[3]);

	const MMO_Face & AddNewCuttedMMO_FaceInternal(GFPhysSoftBodyFace * face , int & ResultIndex);

	const MMO_Face & AddSplittedMMO_FaceInternal(const MMO_Face & splitFromData , GFPhysSoftBodyFace * splitface , bool InOriginPart);
	
	const MMO_Face & AddOriginMMO_FaceInternal( GFPhysSoftBodyFace * face ,
												const int vid[3] , 
												const GFPhysVector3 vertUndeformPos[3] , 
												const Ogre::Vector2 texCoord[3]
												);
    void RebuildVeinConnect(std::vector<FaceSplitByCut> & FacesSplitted,
                            GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & FacesDeleted);

	void CreateEffectRender(int Width , int Height , Ogre::String name);

	void InitializeEffectSystem();

	void UpdateMesh(const std::vector<ShadowNodeForLinkage> & LinkNodes);

	void CreateFFDObject(Ogre::String name , Ogre::String ffdfile);
	
	void UpdateBTNs();

	
	void CreateOrganObjectFromSerialize(const MisMedicObjetSerializer & serializer , Ogre::String organObjName);

	void CreatePhysicsPart(const MisMedicDynObjConstructInfo & CreateInfo , int topologytype = 0);//float mass , bool distributemass , const std::vector<int> & fixpoints ,float stiffness , int contactmode , float veldamping,float collidefricoeff , float collideharness, bool hardfixpoint = false , float surfacemassmult = 1.0f);

	void SetCanClamp();

	void SetCanBeGrasp(bool canBeGrasp);

	void UpdateBurn(float dt);    

	//@overridden cut listener
	//preview add split face to MMO_Face according to where they split from
	//i.e split from origin will add to m_OriginFace split from cut Cross will add to m_CutCross
	void onTopologyChanged(GFPhysSoftBodyTopologyModifier & cData );//
						//const GFPhysAlignedVectorObj<GFPhysCuttedNewFace> & splitfaceData , 
						//const GFPhysAlignedVectorObj<GFPhysCuttedNewFace> & newCutfaceData ,
						//const GFPhysAlignedVectorObj<GFPhysNodeCreatedBySplit> & nodesCreateData);

	//void onRemoveOldTetrahedron(const GFPhysVectorObj<GFPhysSoftBodyTetrahedron*> & removedtetras);

	//call when element delete or added
	void NewTetrasAdded(const GFPhysAlignedVectorObj<GFPhysSoftBodyTetrahedron*> & tetras);

	void NewNodesAdded(const GFPhysAlignedVectorObj<GFPhysSoftBodyNode *> & nodes);

	void TetrasRemoved(const GFPhysAlignedVectorObj<GFPhysSoftBodyTetrahedron*> & tetras);
	
	void FacesRemoved(const GFPhysAlignedVectorObj<GFPhysSoftBodyFace *> & face);
	
	void NodesRemoved(const GFPhysAlignedVectorObj<GFPhysSoftBodyNode *> & nodes);

	//
	void FreeOrginFace(int index);
	void FreeCutFace(int index);

	//rebuild all organ face vi[3] value to point to rend nodes
	//call this when a cut finish or after create all organ physics file
	void RebuildRendVertexAndIndex();

	//call this when undeformed position changed like
	//use rest shape modify
	void RefreshRendFacesUndeformedPosition();

	//void EliminateTetrasInternal(const GFPhysVectorObj<GFPhysSoftBodyTetrahedron *> & tetrasToRemove);

	//变换模型的原始顶点数据
	void transformForModelData();

	//计算该器官分离的部分
	void CalculateSeperatePart();

	void AddBleedingRecord(int trackId) ;

	void OnStopBleeding(int trackId , bool isAutoStopped = false) ;

	void OnStopBleeding(std::vector<int>& trackIds , bool isAutoStopped = false) ;

	float GetBleedingVolume();

    float GetNailRadius();

	
public:
	float m_MinSubPartVolThresHold;

	float m_ElecCutRadius;
	int   m_MaxCutCount;
	Ogre::Vector2 m_TexMaxGrad;

	std::pair<float, int> m_BadTetCollapseParam;
	//testend
	float m_DragForceRate;

	bool  m_FixVertexColor;

	float m_IsolatedPartGravityValue;

	//float m_CutThickNess;
	//int   m_MaxCutTimeForTetra;

	//float m_MaxCutThickNessPercent;//0.02f , 0.1f
	//float m_MaxSoftNodeSpeed;
	//float m_MaxSoftNodeMove;
	bool  m_CanBindThread;

	int   m_EleCatogeryCanCut;

	GFPhysSoftBody * m_physbody;

	std::vector<GFPhysSoftBodyNode*> pulseNodes;
	float pulseForce;

	GFPhysDiscreteDynamicsWorld * m_physDynWorld;

	DynamicSurfaceFreeDeformObject * m_FFDObject;    //class DynamicObjectRenderable : public Ogre::MovableObject

	MisOrganAnimation m_AnimationData;

	GFPhysAlignedVectorObj<PhysNode_Data>  m_PhysNodeData;//first element is for error reference so always has size > 0
	int m_FreePhysNodeDataHead;

	GFPhysAlignedVectorObj<PhysTetra_Data> m_PhysTetraData;
	int m_FreePhysTetraDataHead;

	std::vector<MMO_Node> m_OrganRendNodes;

	GFPhysAlignedVectorObj<MMO_Face> m_OriginFaces;
	int m_FreeOriginFaceHead;

	GFPhysAlignedVectorObj<MMO_Face> m_CutCrossfaces;
	int m_FreeCutFaceHead;
	//std::set<GFPhysSoftBodyFace*> m_FaceDeleteByCut;
	
	bool m_CanBeShrink;

	//std::vector<OrganSubPart*> m_OrganSubParts;

	//std::set<GFPhysSoftBodyNode*> m_NodesInCutCrossFaces;//node be in cut cross face since last cut

	std::set<GFPhysSoftBodyFace*> m_CutCrossFacesInLastCut;//last time generated cut cross face like (cut , eliminate tetrahedrons)
	std::set<GFPhysSoftBodyFace*> m_SplitFacesInLastCut;

	std::set<FaceInScatterBlood> m_BloodPointCutCrossFaces;

	float m_ElecCutThreshold;

	std::vector<TitanicClipInfo> m_titanicClipInfos;
	std::vector<ElecCutInfo> m_elecCutInfos;
	GFPhysSoftBodyFace* m_lastEleCutFace;

	int m_OrganIDFurtherStiffness;

    GFPhysVector3 * m_Serializer_NodeInitPositions_copy;
    int m_Serializer_NodeNum;


	Real m_TimeSinceLastElectricMelt;

	VolumeBlood* m_VolumeBlood;
	MisMedicEndoGiaClips* m_EndoGiaClips;
	
	
private:

	bool  m_BleedWhenStripBreak;//附着的连接断掉的时候是否流血
	float m_BleedRadius;
	GFPhysAlignedVectorObj<OrganInjuryPoint> m_InjuryPoints;

    //PaintingTool m_painting;
	bool m_HasVolTexCoors;
	std::map<OrganSurfaceBloodTextureTrack*,float> m_NonWoundedBloodTimeRecord;
	std::map<OrganSurfaceBloodTextureTrack*,float> m_NonWoundedBloodLimitTime;


	Ogre::ColourValue m_BurnNodeColor;//m_CutFaceColor;
	Ogre::ColourValue m_BloodNodeColor;
	MisMedicObjetSerializer m_Serializer;

	Ogre::SceneNode * m_pSceneNode;

#if USEOLDRENDOBJECT
	Ogre::ManualObject * m_pManualObject;
#else
	MisMedicOrganRender * m_pManualObject;
#endif

	Ogre::SceneNode * m_pSceneNodeDebug;
	Ogre::ManualObject *m_pDebugMO;

	//std::vector<Ogre::Vector3> m_BloodedPoints;
	float m_AccmCrossFaceBloodTime;
	
	float m_FrameElapsedTime;

	float m_TotalElapsedTime;

	float m_lastBloodEmitTime;

	float m_simulateTime;

	float m_BloodIntervalTime;

	float m_TimeNeedToEleCut;

	int   m_MaxBloodTrackCount;//最多存在血流数量

	float m_BloodTrackPointDensity;//越小越密

	float m_MaxBloodTrackExistsTime;

	float m_ContiueElecCutTime;//持续在某点电切的时间

	float m_BloodScatterRadius;//
	float m_BloddScatterValue;

	GFPhysVector3 m_RefBurnPoint;//保存的几乎没有移动烧白点的位置

	bool m_isSetGravity;


	bool  m_IsThisFrameInElecCogTouch;//这一帧是否电凝状态
	
	bool  m_IsThisFrameInElecCutTouch;//这一帧是否电切状态

	int		m_nToolsAffectedType;	// 目前类似于当点使用的tools type

	int		m_bleedEffectFlag;

	Ogre::uint32 *m_burnRecord;

	Ogre::String m_strVesselBleedEffectTemplateName;					//设置流血效果的粒子模板名

	std::vector<SBleedPoint>  m_bleednodes;

	int		m_cutActionNum;

	float			m_ElecCutKeepTime;//电切板踩下的时间

	float		    m_ElecBurnKeepTime;//电凝板踩下的时间

	bool			m_bIsResetAction;

	bool  m_bUseTransformForMaxData;		//决定是否对原始数据进行变换：平移、旋转、缩放  . 默认false 在创建物理部分之前设置才有效

	bool  m_TopologyDirty;

	bool  m_IsInnerTexCoordSetted;

	Ogre::Matrix4 m_transformMatrix;        
};

//获取器官上最近点的位置
//此函数没有优化，遍历所有的面选取最近点 慎用！！
void ClosetFaceToPoint(MisMedicOrgan_Ordinary * meshordinary , 
								 const GFPhysVector3 &testpoint,
								 float &closetDist,
								 GFPhysSoftBodyFace * & closetFace,
								 GFPhysVector3 &closetPoint);
#endif