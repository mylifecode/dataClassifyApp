#ifndef  _APPENDECTOMYTRIANING_
#define  _APPENDECTOMYTRIANING_

#include "MisNewTraining.h"
#include "MisNewTraining.h"
#include "PhysicsWrapper.h"
#include "MisMedicOrganOrdinary.h"

#include "TrainScoreSystem.h"
#include "MisMedicObjectUnion.h"
#include "Instruments/Knotter.h"
class MisMedicOrgan_Ordinary;
class MisMedicOrgan_Tube;
class VeinConnectObject;
class CElectricHook;
class CScissors;
class MisMedicBindedRope;

class DrawObject;


class OrganAppendixVessel
{
public:
	GFPhysAlignedVectorObj<GFPhysVector3> m_PointsUndeformed;
};

class CAppendectomyTraining :public MisNewTraining , public CKnotter::KnotterLoopControlRegion
{
public:
	enum AreaType
	{
		AT_AppendCutPart,// ��β���в���
		
		AT_AppendTakeOutPart,//	��βȡ������
		
		AT_AppendBurnWhitePart,// ��β�հײ���
		
		AT_MENSCutPart,// ��β���в���
		
		AT_MENSTakeOutPart,// ��βȡ������
		
		AT_MENSBurnWhitePart,// ��β�հײ���
		
		AT_NoneTakeablePart,

		AT_TOTALNUM,

		/// ϵģ����
		AT_Mentary = 0x00300000,
		/// ϵģ������Ĳ���
		AT_SeparatedMentary = 0x00C00000,
		/// ���ܴ�������
		AT_CannotKnotArea = 0x0000C000,
		/// �������
		AT_KnotArea = 0x00003000,
		/// ��������
		AT_CutArea = 0x000000C0,
		/// �˵�������
		AT_AppendixEndArea = 0x00000030
	};

	CAppendectomyTraining(const Ogre::String & strName);

	virtual ~CAppendectomyTraining(void);

	void SetCustomParamBeforeCreatePhysicsPart(MisMedicOrgan_Ordinary * organ);

	MisMedicOrganInterface * LoadOrganism( MisMedicDynObjConstructInfo & cs, MisNewTraining* ptrain);

	bool Update(float dt);

	virtual void SerializerReadFinish(MisMedicOrgan_Ordinary * organ , MisMedicObjetSerializer & serialize);

	void ReadAppendHelperLine();

	bool CanControlLoop(CKnotter * knotter);

	//@overriden itrain
	virtual void OnToolCreated(ITool * tool ,int side);
	//@overriden itrain
	virtual void OnToolRemoved(ITool * tool);

	void CheckAppendixSeperated();
	
	uint32 GetPixelFromAreaBuffer(float u, float v);

	bool ReadTrainParam(const std::string& strFileName);

protected:
	virtual void ReadCustomDataFile(const Ogre::String & customDataFile);

	virtual bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);

	virtual void CreateTrainingScene(CXMLWrapperTraining * pTrainingConfig);

	void OnTrainingIlluminated();

    void OnCameraStateChanged(bool bFixed);

	void OnHandleEvent(MxEvent* pEvent);

    void FacesBeRemoved(const GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & faces, GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & createdfaces, MisMedicOrgan_Ordinary * orgn);

	void RemoveMentaryNode(GFPhysSoftBodyNode * node , MisMedicOrgan_Ordinary * organ);

	void OnSaveTrainingReport();

private:
	bool BeginRendOneFrame(float timeelapsed);

	void InitializePlane();

	bool IsMarkedArea(AreaType type,const Ogre::Vector2& texCoord);

	void DealWithOrganBinded(MisMedicOrgan_Ordinary* pOrgan,MisMedicBindedRope* pRope);

	void DealWithCutOrgan(MisMedicOrgan_Ordinary* pOrgan,CTool* pTool);

	void GetEndpointRope(std::vector<MisMedicOrganAttachment*>& attachments,MisMedicBindedRope* & farRope,MisMedicBindedRope* & nearRope);

	void GetDisOfRopeToEndPoint(MisMedicBindedRope* pRope,float & minDis,float & maxDis);

	void GetDisOfCutCrossFaceToEndPoint(const std::vector<GFPhysSoftBodyFace*>& cutCrossFace,float& minDis,float& maxDis,GFPhysVector3& minPos,GFPhysVector3& maxPos);

	bool PosAtRopeRight(MisMedicBindedRope* pRope,const GFPhysVector3& pos);

	void UpdateNeedBurnFaces(MisMedicBindedRope* pNearRope,const std::vector<GFPhysSoftBodyFace*>& cutCrossFace);

	void SetRootPartNodes(MisMedicBindedRope* pNearRope);

	void ClearInvalidateCutFace();

	int GetNumberOfActiveBleedPoint();
public:

	bool PositionBetweenInKnotPlane(const GFPhysVector3& pos);

	bool PositionInAppendixArea(const GFPhysVector3& pos);

	void RemapAppendixToMorphPos(std::string & name);
public:

	void MarkTetraInVessel();

	//MisMedicObjectUnion m_StaticDynamicUnion;

	//Ogre::MeshPtr m_StaticDomeMeshPtr;

	GFPhysVector3 m_AppendIntestineSepPlane[3];
	
	//knot region is a line segment in appendix
	//any  point's dist to this segment shorter than m_KnotRadius
	//is valid;
	GFPhysVector3 m_KnotValidRegionPoints[2];
	
	GFPhysVector3 m_AppendixDirection;

	OrganAppendixVessel m_Vessels;

	/// ����̽�鸹ǻʱ�����ĵ�
	std::vector<Ogre::Vector3> m_CameraCheckPoints;
	std::vector<float> m_CameraCheckValues;
	bool m_CameraCanCheck;
	bool m_CameraCheckFinish;
	float m_InitCheckValue;

	/// ��Ҫ�������ϵģ���
	std::set<GFPhysSoftBodyNode*> m_SeparatedNodes;
	/// ������Ľ����
	int m_NumSeparatedNode;
	/// �������
	float m_SeparaeRate;
	/// ��Ч����β����
	int m_ValidNumSectionOfAppendix;

	/// ������
	int m_OrganBindedTimes;
	/// ��Ч�Ĵ�����,������Ч�����ϴ��Ĵ���
	int m_ValidKnotTimes;
	/// ������
	bool m_OrganBindedFinish;
	

	/// ��β�ʹ����ӵ�
	Ogre::Vector3 m_AppendixEndPoint;
	bool m_OrganCutFinish;

	/// ��β������Ҫ���������Ƭ
	std::set<GFPhysSoftBodyFace*> m_NeedBurnedFaceSet;
	std::set<GFPhysSoftBodyFace*> m_AlreadyBurnedFaceSet;
	float m_BurnFaceTotalTime;
	int m_NumNeedBurnFace;
	bool m_CanDealWithAppendixRoot;
	bool m_DealWithAppendixRootFinish;

	std::set<GFPhysSoftBodyNode*> m_RootPartNodes;

	MisMedicOrgan_Ordinary * m_Appendix;

	
	/// ��β�ײ��ָ�ƽ��
	Ogre::Vector3 m_AppendixBottomSplitePlanePos;
	Ogre::Vector3 m_AppendixBottomSplitePlaneNormal;
	/// ��β�Ҳ�ָ�ƽ��
	Ogre::Vector3 m_AppendixRightSideSplitePlanePos;
	Ogre::Vector3 m_AppendixRightSideSplitePlaneNormal;
	/// ��β����������ƽ��
	Ogre::Vector3 m_AppendixKnoteLeftPlanePos;
	Ogre::Vector3 m_AppendixKonteLeftPlaneNormal;
	/// ��β��������Ҳ�ƽ��
	Ogre::Vector3 m_AppendixKnoteRightPlanePos;
	Ogre::Vector3 m_AppendixKnoteRightPlaneNormal;

	float m_MinDamageInterval;
	/// ��β���˴���
	int m_DamageAppendixTimes;
	float m_LastDamgeAppendixTime;

	/// �����˴���
	int m_DamageLargeIntestineTimes;
	float m_LastDamgeLargeIntestineTime;

	/// �����������˴���
	int m_DamageOtherOrganTimes;
	float m_LastDamageOtherOrganTime;

	/// ���г�Ѫ����
	int m_BleedTimes;

	/// �Ƿ���������β����
	int m_KnotAppendixAtErrorAreaTimes;

	bool m_TakeOffAppendixFinish;


	float m_KnotTetrasWeights[2][4];

	float m_KnotRadius;

	Ogre::TexturePtr m_AreaTexPtr;
	
	int	  m_AreaWidth;
	
	int	  m_AreaHeight;

	Ogre::uint32 * m_AreaBuffer;

	bool	m_bToDebugDisplayMode;
	
	Ogre::String m_AreaMarkTextureName;

	Ogre::String m_AppendMaterialName;

	Ogre::String m_trainName;

	DrawObject * m_pDrawObject;
};

#endif