
#ifndef _MISMEDCDYBANESH_
#define _MISMEDCDYBANESH_
#include <Ogre.h>
#include "collision/GoPhysCollisionlib.h"
#include "Dynamic/GoPhysDynamicLib.h"
#include "Dynamic/Constraint/GoPhysSoftBodyDistConstraint.h"
#include "MisMedicEffectRender.h"
#include "MXOgreWrapper.h"
#include "IObjDefine.h"
#include "MxVariant.h"

using namespace GoPhys;

class PhysicsWrapper;
class DynamicSurfaceFreeDeformObject;
class MisMedicOrganAttachment;
class OrganBloodMotionSimulator;
class TextureBloodTrackEffect;
class TextureWaterTrackEffect;//__/__
class CXMLWrapperTraining;
class CXMLWrapperOrgan;
class MisMedicOrgan_Ordinary;
class MisMedicOrgan_Tube;
class ITool;
class CTool;
class CBasicTraining;
class OrganActionListener;
class VeinConnectObject;
enum MisOrganAttachmentType;

enum  DynObjType
{
	DOT_UNDEF = -1,
	DOT_VOLMESH,//体积网格（默认）
	DOT_MEMBRANE,//膜
	DOT_TUBE,//绳子和线类
	DOT_VEINCONNECT,//组织连接
	DOT_RIGIDBODY,
	DOT_AreaMark,
	DOT_NEWVEINCONNECT,
	DOT_VEINCONNECTV2,
};
enum ObjPhysCatogry
{
	OPC_THREADLOOPLEFTOOL  = 0x01 << 5,
	OPC_THREADLOOPRIGHTOOL = 0x01 << 6,
	OPC_THREADUNIVERSAL = 0x01 << 7,
    OPC_TUBEUNIVERSAL = 0x01 << 8,
    OPC_RIGIDDOME =  0x01 << 9,

	OPC_USERCUSTOMSTART = 0x01 << 20
};


struct FaceSplitByCut
{
	FaceSplitByCut(int oldfaceid , int oldmateril , GFPhysSoftBodyFace * newface)
	{
		m_OldFaceID = oldfaceid;
		m_MaterialID = oldmateril;
		m_NewFace = newface;
	}
	int m_OldFaceID;//被切割的面在mmo_face数组中的索引
	int m_MaterialID;//是originface 还是 cutface面
	GFPhysSoftBodyFace * m_NewFace;//新产生的面
};

struct BleedingRecord
{
	float m_RemainderTimeAtBleed;
	float m_RemainderTimeAtStop;
	float m_BleedingTime;
	bool m_IsAutoStopped;
	bool m_IsStopped;
	int m_TrackId;
	int m_OrganId;
    bool m_HavingBeenCount;
	BleedingRecord(int trackId = -1 , int organId = -1) : m_BleedingTime(0) ,
		m_IsAutoStopped(false) , 
		m_IsStopped(false),
		m_TrackId(trackId),
		m_OrganId(organId),
        m_HavingBeenCount(false){}
};

class MisMedicNodeAdhesion
{
public:
	MisMedicOrgan_Ordinary * m_AdhesionObjectA;
	
	MisMedicOrgan_Ordinary * m_AdhesionObjectB;
	
	std::vector<GFPhysSoftBodyNode*> m_NodesInA;
	
	std::vector<GFPhysSoftBodyNode*> m_NodesInB;
};

class SerializerDBVTrees
{
public:
	SerializerDBVTrees() : m_IsCreateNodeTree(false) , m_IsCreateFaceTree(false) , m_IsCreateTetraTree(false) {}
	bool m_IsCreateNodeTree;
	bool m_IsCreateFaceTree;
	bool m_IsCreateTetraTree;
	
	GFPhysDBVTree m_NodeTree;
	GFPhysDBVTree m_FaceTree;
	GFPhysDBVTree m_TetraTree;
};

class OrganLayer
{
public:
	OrganLayer(const std::string & layername, const std::string & matName, int layerindex)
	{
		m_LayerName = layername;
		m_MaterialName = matName;
		m_layerIndex = layerindex;
	}
	std::string m_LayerName;
	std::string m_MaterialName;
	std::string m_CloneMaterialName;
	Ogre::MaterialPtr m_ClonedMatPtr;
	int m_layerIndex;
};
class MisMedicDynObjConstructInfo
{
public:
	MisMedicDynObjConstructInfo();

	void ReadParameter(CXMLWrapperOrgan * organ);//(CXMLWrapperTraining * pTrainingConfig , unsigned int uiIndex );

	void ReadParameter(CXMLWrapperOrgan * organmain, CXMLWrapperOrgan * organunioned);

	Ogre::String m_RigidType;
	bool         m_CollideWithTool;
	bool		m_Visible;
	bool		m_enableSSCollide;
	bool		m_CanCut;
	bool        m_CanHook;
	bool        m_CanBlood;
	bool		m_IsMensentary;
	std::string m_animationfile;
	std::string m_s3mfilename;
	std::string m_s4mfilename;
	std::string m_t2filename;

	std::string m_pulsePoints;
	std::string m_LayerName;

	std::vector<std::string> m_MergedObjMMSFileNames;

	std::vector<std::string> m_MergedObjLayerNames;
	std::vector<std::string> m_MergedObjLayerMaterials;
	std::vector<int> m_MergedObjIDS;
	//std::string m_unionedmmsfilename;
	//int  m_unionedobjid;
	//float m_unionedobjstiffness;
	//float m_unionobjAdditionBendForce;
	//bool  m_unionobjMenstary;

	std::string m_materialname[4];
	std::string m_name;
	std::string m_FFDFile;

//  	Ogre::SceneManager * m_pSceneMgr;
	Ogre::Vector3 m_Position;
	float m_GravityValue;
	Ogre::Vector3 m_CustomGravityDir;
	bool m_HasCustomGravityDir;
	Ogre::Vector3 m_InitSize;
	Ogre::Vector3 m_SceneMeshInitPosOffset;
	Ogre::Vector3 m_SceneMeshInitScaleOffset;

	float m_AdditionBendForce;

	DynamicObjType   m_OrganType;
	int m_OrganId;
	int   m_objTopologyType;//tube or mesh
	float m_mass;//mass of the body
	float m_stiffness;//0 - 1;

	float m_invEdgePhysStiff;
	float m_invTetraPhysStiff;
	float m_invFacePhysStiff;

	float m_EdgePhysDamping;
	float m_TetraPhysDamping;
	float m_FacePhysDamping;

    float m_poissonrate;//0 - 0.5;
	float m_FurtherStiffness;//
	float m_veldamping;//velocity damping
	float m_perelementdamping;//
	float m_lineardamping;
	float m_angulardampint;
	float m_frictcoeff;//0 - 1 more large more friction will apply (usually use 0.1-0.2)
	float m_collidehardness;//0 - 1 more large collide seems more rigid (usually use 0.4-0.6)
	float m_collideRSMargin;
	float m_ConnectMass;
	bool  m_hardfixpoint;

	float m_RestScale;
	GFPhysVector3 m_FixPointStiffness;    
    float m_ExpandValue;         
    GFPhysVector3 m_ExpandCenterPos;
    GFPhysVector3 m_ExpandPlanePoint1;
    GFPhysVector3 m_ExpandPlanePoint2;
    GFPhysVector3 m_ExpandPlanePoint3;
	float m_bleedingSpeed;
	float m_surfacemassmultifactor;//
	int   m_contactmode;//(0 or 1) usually use 0
	bool  m_DoubleFaceCollide;//whether use double face soft-rigid collision default false
	int   m_collidealgo;
	float m_RSMaxPenetrate;
	float m_HomingForce;

	bool  m_distributemass;
	
	float m_ForceFeedBackRation;

	bool  m_bCanPuncture;
	bool  m_bCanBeGrasp;
	float m_effTexWid;
	float m_effTexHei;
	float m_BurnRadius;
	float m_BurnRation;
	Ogre::String m_BloodTex;
	float m_BloodRadius;
	float m_WaterRadius;
	float m_BloodFlowVelocity;
	float m_TubeRadius;
	float m_TubeRendRadius;
	//int   m_TubeFixSection;
	//int   m_TubeAttachObject;
	//int   m_TubeAttachSection;
	//bool  m_IncTubeRootRadius;
	//bool  m_IncTubeRootWeight;
	bool  m_CheckToolPenetrateForce;
	bool  m_IsStaticObject;
	float m_CutThreshold;
	float m_CrossDir;
	bool  m_bCanBluntDissection;
	//std::string m_CutMaterialName;
	int   m_RigidFactor;//only for tube
	//MisMedicOrgan_Ordinary * m_TubeAttach;
	//MisMedicOrgan_Tube * m_TubeAttTube;

	std::vector<int> m_FixPointsIndex;    
    std::vector<int> m_ExpandPointsIndex;    

	int	m_CutActionParticleParam[3];	// [0]无夹出血时间 [1]有夹出血时间 [2]流出粒子种类，（0 胆汁 1血液）

	//std::vector<MisMedicNodeAdhesion> m_NodeAdhesion;

	//used for the new vein connect
	float m_GallSideWeight;
	float m_LiverSideWeight;
	float m_ConnectStiffnessScale;
	float m_ConnectNodeDistanceStiffness;
	float m_CoonectTetraVolumeStiffness;

	//used for the new vein connect
	bool m_VeinObjNewMode;
	long m_ConnStayNum;
	long m_ConnReduceNum;
    std::string m_AttachStaticMesh;
    float m_AttachStaticMeshThresHold;

	std::vector<OrganLayer> m_LayerVecs;
};

//@ MMOI_AttchmentFlag is set for per MismedicOrgan object
//
enum	MMOI_AttchmentFlag
{
	EMMOI_Reset = 0,
	EMMOI_WithTitanic_On_Tube = 1,
	EMMOI_WithTitanic_On_Ordinary = (1 << 1),
	EMMOI_Touching_By_Tool_Left = (1 << 4),//0x10,
	EMMOI_Touching_By_Tool_Right = (1 << 5),//0x20,
	EMMOI_Touching_By_Tool_Left_Lock = (1 << 6),//0x40,
	EMMOI_Touching_By_Tool_Right_Lock = (1 << 7),//0x80,
	EMMOI_Color_Change_Lock_0 = (1 << 8),//0x100,
	EMMOI_Color_Change_Lock_1 = (1 << 9),//0x200,
	EMMOI_Color_Change_Lock_2 = (1 << 10),//0x300,

	EMMOI_Remove_node	=	0x10000

};

//@@ Note !!MMPhys_NodeStateFlag is set for per GFPhysSoftBodyNode's m_StateFlag control
//it present the node's state like be clamped attached and so on
//see definition of GPSESF_USERDEFSTART
enum MMPhys_NodeStateFlag
{
	EMMP_InCrossFace = (GPSESF_USERDEFSTART << 1),//node lie in cut cross face
	EMMP_ClampByTool = (GPSESF_USERDEFSTART << 2),//node is clamped by tool
	
};

enum MMPhys_TetraStateFlag
{
	EMMT_InElecHollowCenter = (GPSESF_USERDEFSTART << 1),
	EMMT_LayerMembrane = (GPSESF_USERDEFSTART << 2),
	EMMT_LayerTissue   = (GPSESF_USERDEFSTART << 3),
};

//Vertex For Rend
struct MMO_Node
{
	static MMO_Node s_errorNode;//(true);

	MMO_Node(bool haserror = false);
	bool m_HasError;

	GFPhysSoftBodyNode * m_PhysNode;
	//bool m_InCutEdge;

	Ogre::Vector2 m_TextureCoord;
	Ogre::Vector3 m_CurrPos;
	Ogre::ColourValue m_Color;
	Ogre::Vector3 m_Normal;
	Ogre::Vector3 m_Tangent;
	//Ogre::Vector3 m_Bionormal;

	int m_NodeDataIndex;//index reference to "PhysNode_Data"
};

//Face For Rend index point to MMO_Node



struct MMO_Face
{
	static MMO_Face s_errorFace;//(true);

	MMO_Face(bool haserror = false);

	void BuildConstParameter();

	inline Ogre::Vector2 GetTextureCoord(int index) const
	{
		if (m_physface == 0)
			return Ogre::Vector2(0, 0);

		if (index >= 0 && index < 3)
			return Ogre::Vector2(m_physface->m_TexCoordU[index]
			                   , m_physface->m_TexCoordV[index]);
		else
			return Ogre::Vector2(0, 0);
	}

	inline Ogre::Vector2 GetTextureCoord(float weights[3]) const
	{
		 Ogre::Vector2 p1(m_physface->m_TexCoordU[0] , m_physface->m_TexCoordV[0]);
		 Ogre::Vector2 p2(m_physface->m_TexCoordU[1], m_physface->m_TexCoordV[1]);
		 Ogre::Vector2 p3(m_physface->m_TexCoordU[2], m_physface->m_TexCoordV[2]);
		 return p1*weights[0] + p2*weights[1] + p3*weights[2];
	}

	inline void RemoveStripConnectInfo(VeinConnectObject * veinobj , int clusterID)
	{
		 std::vector<MMO_Face::VeinInfo>::iterator iter = m_VeinInfoVector.begin();

		 while (iter != m_VeinInfoVector.end())
		 {
			if (iter->valid && iter->veinobj == veinobj && iter->clusterId == clusterID)
			{
				iter = m_VeinInfoVector.erase(iter);
			}
			else
			{
				iter++;
			}
		}
	}

	float m_constAngle[3];

	float m_deltaV1 , m_deltaV0  , m_deltaU1 , m_deltaU0 ;

	bool m_HasError;//for return face wrong
	int vi[3];//索引到3个MMO_NOde(渲染顶点)
	GFPhysSoftBodyFace * m_physface;//在GPSDK中对应的面
	int m_NextFreed;
	GFPhysVector3 m_NodeUndeformPos[3];//3个顶点未形变时的坐标
	//Ogre::Vector2 m_TextureCoord[3];//
	bool m_NeedRend;
	bool m_NeedDoubleFaces;
	char m_LayerIndex;

	float m_BurnValue;
	std::map<Ogre::String,int> m_AttributeMap;

    struct VeinClusterInfo
    {        
        VeinConnectObject * veinobj;
        int clusterId;
        bool operator  < (const VeinClusterInfo& other) const
        {
            if (this->veinobj != other.veinobj)
            {
                return this->veinobj < other.veinobj;
            }
            else
            {
                return this->clusterId < other.clusterId;
            }            
        }
    };
    //////////////////////////////////////////////////////////////////////////
    enum  VeinconnPosLocal
    {
        A,
        B,
        C,
        D
    };
    struct VeinInfo
    {        
        VeinConnectObject * veinobj;
        int clusterId;
        VeinconnPosLocal local;
        bool valid;
    };
    std::vector<VeinInfo> m_VeinInfoVector;//索引到渲染面的筋膜信息
};



class MisSoftBodyHomingForce : public GFPhysSoftBodyConstraint
{
public:
	MisSoftBodyHomingForce(GFPhysSoftBody * softBody);
	
	virtual ~MisSoftBodyHomingForce();

	virtual void PrepareSolveConstraint(Real Stiffness,Real TimeStep);

	virtual void SolveConstraint(Real Stiffness,Real TimeStep);

	GFPhysSoftBody  * m_SoftBody;
};

class MisSoftBodyNodesFixForce : public GFPhysSoftBodyConstraint
{
public:
	MisSoftBodyNodesFixForce(const std::vector<GFPhysSoftBodyNode*> & nodes);

	virtual ~MisSoftBodyNodesFixForce();

	virtual void PrepareSolveConstraint(Real Stiffness,Real TimeStep);

	virtual void SolveConstraint(Real Stiffness,Real TimeStep);

	GFPhysVector3 m_AsyStiff;
	std::vector<GFPhysSoftBodyNode*> m_Nodes;
};

class MisSoftBodyExpandForce : public GFPhysSoftBodyConstraint
{
public:
    MisSoftBodyExpandForce(const std::vector<GFPhysSoftBodyNode*> & nodes,const GFPhysAlignedVectorObj<GFPhysVector3> & nodesPos);

    virtual ~MisSoftBodyExpandForce();

    virtual void PrepareSolveConstraint(Real Stiffness,Real TimeStep);

    virtual void SolveConstraint(Real Stiffness,Real TimeStep);

    std::vector<GFPhysSoftBodyNode*> m_Nodes;
    GFPhysAlignedVectorObj<GFPhysVector3>  m_NodesPos;
};

class MisMedicOrganInterface
{
	friend class MisNewTraining;
public:
	MisMedicOrganInterface(DynObjType type, DynamicObjType organtype, int organId, CBasicTraining * ownertraing);

	virtual ~MisMedicOrganInterface();

	virtual int  GetAttachmentCount(MisOrganAttachmentType type);

	void GetAttachment(MisOrganAttachmentType type, std::vector<MisMedicOrganAttachment*> & attachments);

	virtual void CutOrganByTool(CTool * tool);

	virtual void SetBurnWhiteNoiseTexture(Ogre::TexturePtr burnTexture);

	virtual void SetBurnWhiteColor(Ogre::ColourValue color0, Ogre::ColourValue color1);

	virtual Ogre::Vector2 GetTextureCoord(GFPhysSoftBody * sb, GFPhysSoftBodyFace * face, float weights[3]);

	virtual void PreUpdateScene(float dt, Ogre::Camera * camera);

	virtual void UpdateScene(float dt, Ogre::Camera * camera);

	virtual void setVisible(bool vis)
	{
		m_Visible = vis;
	}

	virtual bool CanBeGrasp();

	virtual bool CanBeCut();

	virtual bool CanBeLoop();

	virtual bool CanBeHook();

	inline  float GetMinPunctureDist()
	{
		return m_MinPuncturedDist;
	}

	inline  void SetMinPunctureDist(float minPuncDist)
	{
		m_MinPuncturedDist = minPuncDist;
	}
	virtual float GetForceFeedBackRation();

	virtual void  SetForceFeedBackRation(float ration);

	virtual void ToolPunctureSurface(ITool * tool, GFPhysSoftBodyFace * face, const float weights[3]);

	virtual void Tool_InElec_TouchFacePoint(ITool * tool, GFPhysSoftBodyFace * face, float weights[3], int touchtype, float dt);

	virtual void InjectSomething(ITool *tool, GFPhysSoftBodyFace * face, float weights[3], float dt, std::vector<Ogre::Vector2> & resultUv);

	virtual void HeatTetrahedrons(const GFPhysVectorObj<GFPhysSoftBodyTetrahedron*> & tetrasToHeat, float deltavalue);

	virtual void HeatAroundUndeformedPoint(const GFPhysVector3 & point, float radius, float heatValue);

	virtual void NotifyRigidBodyRemovedFromWorld(GFPhysRigidBody * rb);

	virtual	Ogre::SceneNode* getSceneNode() { return NULL; }

	virtual void Create(MisMedicDynObjConstructInfo & constructInfo) = 0;

	virtual void PostLoadScene()
	{

	}

	virtual void InternalSimulateStart(int currStep, int TotalStep, Real dt)
	{

	}
	virtual void InternalSimulateEnd(int currStep, int TotalStep, Real dt)
	{

	}

	virtual void RefreshDirtyData()
	{

	}

	//	void SetContulateRadius(float radius);

	virtual void setEffectRender_deviateScale(float deviateScale)
	{
		m_EffectRender.m_deviateScale = deviateScale;
	}
	MisMedicDynObjConstructInfo & GetCreateInfo()
	{
		return m_CreateInfo;
	}
	MisMedicEffectRender & GetEffectRender()
	{
		return  m_EffectRender;
	}

	TextureBloodTrackEffect * GetBloodTextureEffect()
	{
		return m_BloodTextureEffect;
	}
	//__/__
	TextureWaterTrackEffect * GetWaterTextureEffect()
	{
		return m_WaterTextureEffect;
	}

	int getAttchmentFlag()
	{
		return m_AttchmentsFlag;
	}

	void SetHomingForce(float stiffness, GFPhysSoftBody *sb);
	void SetNodeFixForce(const GFPhysVector3 & stifness, const std::vector<GFPhysSoftBodyNode*> & fixNodes);
	void SetNodeExpandForce(const std::vector<GFPhysSoftBodyNode*> & expandNodes, const GFPhysAlignedVectorObj<GFPhysVector3> & expandNodesPos);
	virtual void setAttchmentFlag(int flag, bool isRemove = false);
	virtual void setFlag(int flag, bool isRemove);
	virtual int  getFlag_MaterialId();


	virtual std::string getName()
	{
		return m_CreateInfo.m_name;
	}

	std::string getMaterialName()
	{
		return m_CreateInfo.m_materialname[0];
	}
	void setMaterialName(std::string str)
	{
		m_CreateInfo.m_materialname[0] = str;
	}

	Ogre::MaterialPtr GetOwnerMaterialPtr()
	{
		return m_OwnerMaterialPtr;
	}

	std::string GetOwnerMaterialName()
	{
		return m_OwnerMaterialPtr->getName();
	}

	virtual void refreshMaterial(const std::string& str)
	{
		m_CreateInfo.m_materialname[0] = str;
	}

	void setProperty(OrganPropertyName propertyName, const MxVariant& propertyValue)
	{
		m_organPropertyValueMap[propertyName] = propertyValue;
	}

	MxVariant getProperty(OrganPropertyName propertyName)
	{
		std::map<OrganPropertyName, MxVariant>::iterator itr = m_organPropertyValueMap.find(propertyName);
		if(itr != m_organPropertyValueMap.end())
			return itr->second;
		else
			return MxVariant();
	}

	virtual float GetBleedingVolume() { return 0.f; }

	//virtual void OnAddTetra(GFPhysSoftBodyTetrahedron * tetra)
	//{}

	//virtual void OnAddEdge(GFPhysSoftBodyEdge * edge)
	//{}

	//virtual void OnAddFace(GFPhysSoftBodyFace * face)
	//{}

	//virtual void OnAddNode(GFPhysSoftBodyNode * node)
	//{}

	//virtual void OnRemoveTetra(GFPhysSoftBodyTetrahedron * tetra)
	//{}

	//virtual void OnRemoveEdge(GFPhysSoftBodyEdge * edge)
	//{}
	//virtual void OnRemoveFace(GFPhysSoftBodyFace * face)
	//{}

	//virtual void OnRemoveNode(GFPhysSoftBodyNode * node)
	//{}

	virtual void CreateSerializerNodeTree(const float extend) {}

	virtual const GFPhysDBVTree & GetSerializerNodeTree()
	{
		return m_SerializerDBVTrees.m_NodeTree;
	}

	virtual const GFPhysDBVTree & GetSerializerFaceTree()
	{
		return m_SerializerDBVTrees.m_FaceTree;
	}

	virtual const GFPhysDBVTree & GetSerializerTetraTree()
	{
		return m_SerializerDBVTrees.m_TetraTree;
	}

	//MisSoftBodyPoseRigidForce * GetPoseRigidForce()
	//{
	//return m_PoseRigidForce;
	//}

	virtual float GetCurMaxBleedTime(){ return 0.f; }

	int GetNumberOfActiveBleedPoint();

	inline DynamicObjType GetOrganType() { return m_OrganType; }

	void AddOrganActionListener(OrganActionListener * listener);

	void RemoveOrganActionListener(OrganActionListener * listener);

	static void ExtractOgreMeshInfo(Ogre::MeshPtr mesh,

							 std::vector<Ogre::Vector3> & vertices,
							 std::vector<Ogre::Vector2> & textures,
							 std::vector<unsigned int> & indices);

	bool IsClamped()
	{
		return m_BeClampedByLTool || m_BeClampedByRTool;
	}
	bool IsClampedByLTool()
	{
		return m_BeClampedByLTool;
	}

	bool IsClampedByRTool()
	{
		return m_BeClampedByRTool;
	}

	void SetClampedByLTool(bool set)
	{
		m_BeClampedByLTool = set;
	}

	void SetClampedByRTool(bool set)
	{
		m_BeClampedByRTool = set;
	}

	Ogre::Vector3 GetGravity()
	{
		return m_CreateInfo.m_CustomGravityDir*m_CreateInfo.m_GravityValue;
	}

	//bool		m_MultiMassToCalcContactForce;
	bool		m_Visible;
	bool        m_Transparent;
	// 	DynObjType m_TopolgyType;

	int m_OrganID;//器官ID

	float m_ConnectMass;//当连接在胆囊连接上时的权重

	std::vector<MisMedicOrganAttachment*> m_OrganAttachments;
	//出血点记录
	std::vector<BleedingRecord> m_BleedingRecords;

	int m_RSCollideAlgoType;

	//是否被抓住 temp
	//bool m_IsGrasped;
	

	float m_FaceMoveIncrementInClamp;

	float m_FaceMoveSpeedInClamp;

	Ogre::String m_ClampInstrumentType;

	//是否被吸住 temp
	bool m_IsSucked;

	float m_FaceMoveIncrementInSuction;

	float m_FaceMoveSpeedInSuction;

	bool m_IsInContainer;
protected:

	virtual void RemovePhysicsPart();
	virtual void RemoveGraphicPart();
	virtual void RemoveFromWorld()
	{
		RemovePhysicsPart();
		RemoveGraphicPart();
	}

	Ogre::String m_materialname;

	Ogre::MaterialPtr m_OwnerMaterialPtr;

	MisMedicEffectRender m_EffectRender;

	OrganBloodMotionSimulator * m_BloodSystem;
	TextureBloodTrackEffect *  m_BloodTextureEffect;
	//__/__
	OrganBloodMotionSimulator * m_WaterSystem;
	TextureWaterTrackEffect *  m_WaterTextureEffect;

	SerializerDBVTrees m_SerializerDBVTrees;

	MisMedicDynObjConstructInfo m_CreateInfo;

	bool m_IsAlreadyReadFile;
	
	float m_MinPuncturedDist;
	bool  m_CanBePuncture;
	bool  m_CanBeGrasp;//是否可以抓取

	float m_ForceFeedBackRation;//力反馈系数
	
	float m_congulateradius;//电凝斑大小


	int	m_AttchmentsFlag;
	int	m_Flag;

	CBasicTraining * m_OwnerTrain;

	MisSoftBodyNodesFixForce * m_NodesFixForces;

	MisSoftBodyHomingForce * m_homingforcecs;

	//MisSoftBodyPoseRigidForce * m_PoseRigidForce;

    MisSoftBodyExpandForce * m_ExpandForce;

	bool m_CanBeLoop;

	DynamicObjType m_OrganType;

	std::map<OrganPropertyName,MxVariant> m_organPropertyValueMap;

	std::vector<OrganActionListener *> m_OrganActionListeners;

	bool m_BeClampedByLTool;//for convient use

	bool m_BeClampedByRTool;//for convient use
};


typedef std::map<int , MisMedicOrganInterface*> DynObjMap;

class VeinRigidCollisionListener
{
public:
	virtual void  OnVeinConnectPairCollide(MisMedicOrganInterface * veinobject ,
		GFPhysCollideObject * convexobj,
		int cluster , 
		int pair,
		const GFPhysVector3 & collidepoint) = 0;
};

class OrganActionListener
{
public:
	virtual void OnOrganBleeding(int organId , int trackId) {};
	virtual void OnOrganStopBleeding(int organId , int trackId) {};
	virtual void OnOrganPhysicsPartCreated(MisMedicOrganInterface * pOrgan) {};
};
#endif