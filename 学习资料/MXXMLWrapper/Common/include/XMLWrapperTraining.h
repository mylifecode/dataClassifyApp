/**Author:zx**/
#pragma once
#include "XMLSerialize.h"

class CXMLWrapperLight;
class CXMLWrapperToolPlace;
class CXMLWrapperDataForDeviceCandidate;
class CXMLWrapperTool;
class CXMLWrapperStaticScene;
class CXMLWrapperOrgan;
class CXMLWrapperConnect;
class CXMLWrapperAdhere;
class CXMLWrapperAdhesionCluster;
class CXMLWrapperCollision;
class CXMLWrapperPart;
class CXMLWrapperHardwareConfig;
class CXMLWrapperPursue;
class CXMLWrapperMovie;
class CXMLWrapperScore;
class CXMLWrapperOnLineGrade;
class CXMLWrapperTip;
class CXMLWrapperShadow;
class CXMLWrapperSphere;
class CXMLWrapperMucous;
class CXMLWrapperDetector;
class CXMLWrapperOrganTranslation;
class CXMLWrapperOperateItem;
class CXMLWrapperWaterPool;
class CXMLWrapperToolForTask;
class CXMLWrapperViewDetection;

class CXMLWrapperTraining : public CXMLSerialize
{
public:
	CXMLWrapperTraining(void);
	~CXMLWrapperTraining(void);

	vector<CXMLWrapperLight *> m_Lights;
	bool m_flag_Lights;

	void __stdcall Set_Lights(Variant value);
	void __stdcall Get_Lights(Variant * pValue);

	vector<CXMLWrapperToolPlace *> m_ToolPlaces;
	bool m_flag_ToolPlaces;

	void __stdcall Set_ToolPlaces(Variant value);
	void __stdcall Get_ToolPlaces(Variant * pValue);

    vector<CXMLWrapperDataForDeviceCandidate *> m_DataForDeviceCandidates;
    bool m_flag_DataForDeviceCandidates;

    void __stdcall Set_DataForDeviceCandidates(Variant value);
    void __stdcall Get_DataForDeviceCandidates(Variant * pValue);

	vector<CXMLWrapperToolForTask *> m_ToolForTasks;
	bool m_flag_ToolForTasks;

	void __stdcall Set_ToolForTasks(Variant value);
	void __stdcall Get_ToolForTasks(Variant * pValue);

	vector<CXMLWrapperMovie *> m_Movies;
	bool m_flag_Movies;

	void __stdcall Set_Movies(Variant value);
	void __stdcall Get_Movies(Variant * pValue);

	vector<CXMLWrapperScore *> m_Scores;
	bool m_flag_Scores;

	void __stdcall Set_Scores(Variant value);
	void __stdcall Get_Scores(Variant * pValue);

	vector<CXMLWrapperOnLineGrade *> m_OnLineGrades;
	bool m_flag_OnLineGrades;

	void __stdcall Set_OnLineGrades(Variant value);
	void __stdcall Get_OnLineGrades(Variant * pValue);

	vector<CXMLWrapperOperateItem*> m_OperateItems;
	bool m_flag_OperateItems;

	void __stdcall Set_OperateItems(Variant value);
	void __stdcall Get_OperateItems(Variant * pValue);

	vector<CXMLWrapperOperateItem*> m_CommonOperateItems;
	bool m_flag_CommonOperateItems;

	void __stdcall Set_CommonOperateItems(Variant value);
	void __stdcall Get_CommonOperateItems(Variant * pValue);

	vector<CXMLWrapperTip *> m_Tips;
	bool m_flag_Tips;

	void __stdcall Set_Tips(Variant value);
	void __stdcall Get_Tips(Variant * pValue);

	vector<CXMLWrapperOrgan *> m_DynamicScene;
	bool m_flag_DynamicScene;

	void __stdcall Set_DynamicScene(Variant value);
	void __stdcall Get_DynamicScene(Variant * pValue);

	vector<CXMLWrapperConnect *> m_ConnectObject;
	bool m_flag_ConnectObject;

	void __stdcall Set_ConnectObject(Variant value);
	void __stdcall Get_ConnectObject(Variant * pValue);

	vector<CXMLWrapperPart *> m_PartScene;
	bool m_flag_PartScene;

	void __stdcall Set_PartScene(Variant value);
	void __stdcall Get_PartScene(Variant * pValue);

    vector<CXMLWrapperAdhere *> m_AdhereObject;
    bool m_flag_AdhereObject;

    void __stdcall Set_AdhereObject(Variant value);
    void __stdcall Get_AdhereObject(Variant * pValue);

	vector<CXMLWrapperAdhesionCluster*> m_AdhesionClusters;
	bool m_flag_AdhesionClusters;

	void __stdcall Set_AdhesionClusters(Variant value);
	void __stdcall Get_AdhesionClusters(Variant * pValue);

    vector<CXMLWrapperCollision *> m_CollisionObject;
    bool m_flag_CollisionObject;

    void __stdcall Set_CollisionObject(Variant value);
    void __stdcall Get_CollisionObject(Variant * pValue);

	vector<CXMLWrapperPursue *> m_PursueObject;
	bool m_flag_PursueObject;
	void __stdcall Set_PursueObject(Variant value);
	void __stdcall Get_PursueObject(Variant *pValue);

	vector< CXMLWrapperMucous* > m_vecMucousObject;
	void __stdcall Set_MucousObject( Variant value );
	void __stdcall Get_MucousObject( Variant *pValue );

	CXMLWrapperStaticScene * m_StaticScene;
	bool m_flag_StaticScene;

	void __stdcall Set_StaticScene(Variant value);
	void __stdcall Get_StaticScene(Variant * pValue);

	vector<CXMLWrapperShadow *> m_Shadows;
	bool m_flag_Shadows;

	void __stdcall Set_Shadows(Variant value);
	void __stdcall Get_Shadows(Variant * pValue);

	vector<CXMLWrapperSphere *> m_Spheres;
	bool m_flag_Spheres;

	void __stdcall Set_Spheres(Variant value);
	void __stdcall Get_Spheres(Variant * pValue);

	vector<CXMLWrapperDetector*> m_Detectors;
	bool m_flag_Detectors;
	
	void __stdcall Set_Detectors(Variant value);
	void __stdcall Get_Detectors(Variant * pValue);

	vector<CXMLWrapperOrganTranslation*> m_OrganTranslations;
	bool m_flag_OrganTranslations;

	void __stdcall Set_OrganTranslations(Variant value);
	void __stdcall Get_OrganTranslations(Variant * pValue);

	vector<CXMLWrapperWaterPool*> m_WaterPools;
	bool m_flag_WaterPools;

	void __stdcall Set_WaterPools(Variant value);
	void __stdcall Get_WaterPools(Variant * pValue);

	vector<CXMLWrapperViewDetection*> m_ViewDetections;
	bool m_flag_ViewDetections;
	
	void __stdcall Set_ViewDetections(Variant value);
	void __stdcall Get_ViewDetections(Variant * pValue);

	IMPL_ATTRIBUTE_STRING(Name)
	IMPL_ATTRIBUTE_STRING(CustomDataFile)
	IMPL_ATTRIBUTE_STRING(ShowName)
	IMPL_ATTRIBUTE_STRING(MainCatogery)
	IMPL_ATTRIBUTE_STRING(SubCatogery)
	IMPL_ATTRIBUTE_LONG(DifficultLevel)
	IMPL_ATTRIBUTE_STRING(Type)
	IMPL_ATTRIBUTE_STRING(AddOgreResLocation)
	IMPL_ATTRIBUTE_BOOL(AutoLoad)
	IMPL_ATTRIBUTE_BOOL(CustomLoadDynamicScene)
	IMPL_ATTRIBUTE_STRING(XML)
	IMPL_ATTRIBUTE_BOOL(SupportFSM)
	IMPL_ATTRIBUTE_BOOL(StaticCamera)

	// fsm info
	IMPL_ATTRIBUTE_STRING(FSMXlsFile)
	IMPL_ATTRIBUTE_STRING(FSMBinaryfile)
	IMPL_ATTRIBUTE_STRING(InitialState)

	// skin alpha
	IMPL_ATTRIBUTE_STRING(SkinModelName)

	//hdr
	IMPL_ATTRIBUTE_BOOL(HDR);    

	//training time
	IMPL_ATTRIBUTE_LONG(TrainTime);
	IMPL_ATTRIBUTE_FLOAT(SimulationFreqency);
	IMPL_ATTRIBUTE_LONG(SolverItertorNum);
    IMPL_ATTRIBUTE_LONG(StrainItertorNum);
	IMPL_ATTRIBUTE_LONG(SolverThreadNum);

	IMPL_ATTRIBUTE_LONG(GraspMode);
	IMPL_ATTRIBUTE_STRING(SigMoviePath);
	IMPL_ATTRIBUTE_LONG(SigMovieWidth);
	IMPL_ATTRIBUTE_LONG(SigMovieHeight);
	IMPL_ATTRIBUTE_STRING(MoviePath);
	IMPL_ATTRIBUTE_LONG(MovieWidth);
	IMPL_ATTRIBUTE_LONG(MovieHeight);
	IMPL_ATTRIBUTE_LONG(MovieOverlayWidth);
	IMPL_ATTRIBUTE_LONG(MovieOverlayHeight);
	IMPL_ATTRIBUTE_FLOAT(MovieOverlayPosX);
	IMPL_ATTRIBUTE_FLOAT(MovieOverlayPosY);

	IMPL_ATTRIBUTE_FLOAT(DistanceLimit);

	IMPL_ATTRIBUTE_FLOAT(CollisionRadius);
	IMPL_ATTRIBUTE_BOOL(CanStaticCollision);
	IMPL_ATTRIBUTE_BOOL(CanSSAO);
	IMPL_ATTRIBUTE_FLOAT3(BloomFactor);
	IMPL_ATTRIBUTE_FLOAT3(CCFactor);
	IMPL_ATTRIBUTE_BOOL(NeedFixTool);
	IMPL_ATTRIBUTE_BOOL(UseNewBlood);

	IMPL_ATTRIBUTE_STRING(DataForDeviceWorkspaceLeft);
	IMPL_ATTRIBUTE_BOOL(DebugForDeviceWorkspaceLeft);
	IMPL_ATTRIBUTE_STRING(DataForDeviceWorkspaceRight);
	IMPL_ATTRIBUTE_BOOL(DebugForDeviceWorkspaceRight);

    IMPL_ATTRIBUTE_BOOL(UseNewRender)

    IMPL_ATTRIBUTE_FLOAT3(GravityDir)
    //for suture
    IMPL_ATTRIBUTE_FLOAT(ThreadRSLEN);
    IMPL_ATTRIBUTE_LONG(ThreadNodeNum);
    IMPL_ATTRIBUTE_STRING(NeedleSkeleton);
    //SimpleUI
    IMPL_ATTRIBUTE_STRING(SimpleUI)

	IMPL_ATTRIBUTE_STRING(SheetCode)
	DECLARE_SERIALIZATION_CLASS(CXMLWrapperTraining)
};

