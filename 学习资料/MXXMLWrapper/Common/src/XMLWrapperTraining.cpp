/**Author:zx**/
#include "StdAfx.h"
#include "XMLWrapperTraining.h"
#include "XMLWrapperLight.h"
#include "XMLWrapperOrgan.h"
#include "XMLWrapperSceneNode.h"
#include "XMLWrapperTool.h"
#include "XMLWrapperToolPlace.h"
#include "XMLWrapperDataForDeviceCandidate.h"
#include "XMLWrapperShadow.h"
#include "XMLWrapperOrganTranslation.h"
#include "XMLWrapperOperateItem.h"
#include "XMLWrapperToolForTask.h"
BEGIN_IMPL_SERIALIZATION_CLASS(CXMLWrapperTraining)
	REGISTER_CLASS(Training)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,Name,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,Type,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,CustomDataFile,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining, ShowName, VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining, MainCatogery, VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining, SubCatogery, VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining, DifficultLevel, VALUE)

	REGISTER_ATTRIBUTE(CXMLWrapperTraining,AutoLoad,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,CustomLoadDynamicScene,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,AddOgreResLocation,VALUE)
	
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,StaticCamera,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,XML,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,ToolPlaces,ARRAY)
    REGISTER_ATTRIBUTE(CXMLWrapperTraining,DataForDeviceCandidates,ARRAY)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,ToolForTasks,ARRAY)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,Lights,ARRAY)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,DynamicScene,ARRAY)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,StaticScene,OBJECT)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,PartScene,ARRAY)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,Movies,ARRAY)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,Scores,ARRAY)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining, OnLineGrades, ARRAY)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,OperateItems,ARRAY)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,CommonOperateItems,ARRAY)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,Tips,ARRAY)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,ConnectObject,ARRAY)
    REGISTER_ATTRIBUTE(CXMLWrapperTraining,AdhereObject,ARRAY)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,AdhesionClusters,ARRAY)
    REGISTER_ATTRIBUTE(CXMLWrapperTraining,CollisionObject,ARRAY)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,PursueObject,ARRAY)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining, MucousObject, ARRAY)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,Shadows,ARRAY)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,Spheres,ARRAY)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,Detectors,ARRAY)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,OrganTranslations,ARRAY)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,WaterPools,ARRAY)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,ViewDetections,ARRAY)


	REGISTER_ATTRIBUTE(CXMLWrapperTraining,FSMXlsFile,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,FSMBinaryfile,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,InitialState,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,SupportFSM,VALUE)

	REGISTER_ATTRIBUTE(CXMLWrapperTraining,SkinModelName,VALUE)

	REGISTER_ATTRIBUTE(CXMLWrapperTraining,HDR,VALUE)     
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,TrainTime,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,SimulationFreqency,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,SolverItertorNum,VALUE)
    REGISTER_ATTRIBUTE(CXMLWrapperTraining,StrainItertorNum,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,SolverThreadNum,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,GraspMode,VALUE)
	
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,SigMoviePath,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,SigMovieWidth,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,SigMovieHeight,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,MoviePath,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,MovieWidth,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,MovieHeight,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,MovieOverlayWidth,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,MovieOverlayHeight,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,MovieOverlayPosX,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,MovieOverlayPosY,VALUE)

	REGISTER_ATTRIBUTE(CXMLWrapperTraining,DistanceLimit,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,NeedFixTool,VALUE)
	
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,CollisionRadius,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,CanStaticCollision,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,CanSSAO,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining, BloomFactor, VALUE);
    REGISTER_ATTRIBUTE(CXMLWrapperTraining, CCFactor, VALUE);

	REGISTER_ATTRIBUTE(CXMLWrapperTraining,UseNewBlood,VALUE)

	REGISTER_ATTRIBUTE(CXMLWrapperTraining,DataForDeviceWorkspaceLeft,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,DebugForDeviceWorkspaceLeft,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,DataForDeviceWorkspaceRight,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,DebugForDeviceWorkspaceRight,VALUE)

	REGISTER_ATTRIBUTE(CXMLWrapperTraining,UseNewRender,VALUE)
    REGISTER_ATTRIBUTE(CXMLWrapperTraining,ThreadRSLEN, VALUE)    
    REGISTER_ATTRIBUTE(CXMLWrapperTraining,ThreadNodeNum, VALUE)
    REGISTER_ATTRIBUTE(CXMLWrapperTraining, NeedleSkeleton, VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining,GravityDir,VALUE)

	REGISTER_ATTRIBUTE(CXMLWrapperTraining,SimpleUI,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperTraining, SheetCode, VALUE)

END_IMPL_SERIALIZATION_CLASS

CXMLWrapperTraining::CXMLWrapperTraining(void)
{
	INIT_ATTRIBUTE_STRING(Name)
	INIT_ATTRIBUTE_STRING(CustomDataFile)
	INIT_ATTRIBUTE_STRING(ShowName)
	INIT_ATTRIBUTE_STRING(MainCatogery)
	INIT_ATTRIBUTE_STRING(SubCatogery)
	INIT_ATTRIBUTE_LONG(DifficultLevel)
	INIT_ATTRIBUTE_STRING(Type)
	INIT_ATTRIBUTE_BOOL(AutoLoad)
	INIT_ATTRIBUTE_BOOL(CustomLoadDynamicScene)
	INIT_ATTRIBUTE_STRING(AddOgreResLocation)
	INIT_ATTRIBUTE_BOOL(StaticCamera)
	INIT_ATTRIBUTE_STRING(XML)
	INIT_ATTRIBUTE_BOOL(SupportFSM)

	INIT_ATTRIBUTE_STRING(FSMXlsFile)
	INIT_ATTRIBUTE_STRING(FSMBinaryfile)
	INIT_ATTRIBUTE_STRING(InitialState)

	INIT_ATTRIBUTE_STRING(SkinModelName)

	INIT_ATTRIBUTE_BOOL(HDR)    

	INIT_ATTRIBUTE_LONG(TrainTime)

	INIT_ATTRIBUTE_FLOAT(SimulationFreqency)

	INIT_ATTRIBUTE_LONG_VALUE(SolverItertorNum,7)
    INIT_ATTRIBUTE_LONG_VALUE(StrainItertorNum,1)
	INIT_ATTRIBUTE_LONG_VALUE(SolverThreadNum,2)

	INIT_ATTRIBUTE_LONG(GraspMode)

	INIT_ATTRIBUTE_STRING(SigMoviePath)
	INIT_ATTRIBUTE_LONG(SigMovieWidth)
	INIT_ATTRIBUTE_LONG(SigMovieHeight)
	INIT_ATTRIBUTE_STRING(MoviePath)
	INIT_ATTRIBUTE_LONG(MovieWidth)
	INIT_ATTRIBUTE_LONG(MovieHeight)
	INIT_ATTRIBUTE_LONG(MovieOverlayWidth)
	INIT_ATTRIBUTE_LONG(MovieOverlayHeight)
	INIT_ATTRIBUTE_FLOAT(MovieOverlayPosX)
	INIT_ATTRIBUTE_FLOAT(MovieOverlayPosY)

	INIT_ATTRIBUTE_FLOAT_VALUE(CollisionRadius,2.5)
	INIT_ATTRIBUTE_BOOL(CanStaticCollision)
	INIT_ATTRIBUTE_BOOL_VALUE(CanSSAO, true)
	INIT_ATTRIBUTE_FLOAT3_VALUE(BloomFactor, 0.8f, 1.0f, 1.0f)
	INIT_ATTRIBUTE_FLOAT3_VALUE(CCFactor,1.2f,1.0f,1.0f)

	INIT_ATTRIBUTE_BOOL_VALUE(NeedFixTool, false)
	INIT_ATTRIBUTE_BOOL(UseNewBlood)

	INIT_ATTRIBUTE_STRING(DataForDeviceWorkspaceLeft)
	INIT_ATTRIBUTE_BOOL(DebugForDeviceWorkspaceLeft)
	INIT_ATTRIBUTE_STRING(DataForDeviceWorkspaceRight)
	INIT_ATTRIBUTE_BOOL(DebugForDeviceWorkspaceRight)
	
	INIT_ATTRIBUTE_BOOL(UseNewRender)
	INIT_ATTRIBUTE_FLOAT3(GravityDir)

    INIT_ATTRIBUTE_STRING(SimpleUI)
    INIT_ATTRIBUTE_STRING(NeedleSkeleton)

	m_flag_StaticScene = false;
	m_flag_Lights = false;
	m_flag_ToolForTasks = false;
	m_flag_DynamicScene = false;
	m_flag_PartScene = false;
	m_flag_Movies = false;
	m_flag_Scores = false;
	m_flag_OnLineGrades = false;
	m_flag_OperateItems = false;
	m_flag_CommonOperateItems = false;
	m_flag_Tips = false;
	m_flag_Shadows=false;
	m_flag_OrganTranslations = false;
	m_flag_WaterPools = false;
	m_flag_AdhesionClusters = false;
	m_flag_ViewDetections = false;

	INIT_ATTRIBUTE_FLOAT(DistanceLimit)
	m_DistanceLimit=2.0;
	
	INIT_ATTRIBUTE_STRING(SheetCode)
}

CXMLWrapperTraining::~CXMLWrapperTraining(void)
{

}

void __stdcall CXMLWrapperTraining::Set_Lights(Variant value)
{
	if (value.vt != SYVT_I4 && value.lVal != 0)
	{
		return;	
	}
	
	m_Lights.push_back((CXMLWrapperLight *)value.lVal);
	m_flag_Lights = true;
}

void __stdcall CXMLWrapperTraining::Get_Lights(Variant * pValue)
{

}

void __stdcall CXMLWrapperTraining::Set_ToolPlaces(Variant value)
{
	if (value.vt != SYVT_I4 && value.lVal != 0)
	{
		return;	
	}

	m_ToolPlaces.push_back((CXMLWrapperToolPlace *)value.lVal);
	m_flag_ToolPlaces = true;
}

void __stdcall CXMLWrapperTraining::Get_ToolPlaces(Variant * pValue)
{

}

void __stdcall CXMLWrapperTraining::Set_DataForDeviceCandidates(Variant value)
{
    if (value.vt != SYVT_I4 && value.lVal != 0)
    {
        return;	
    }

    m_DataForDeviceCandidates.push_back((CXMLWrapperDataForDeviceCandidate *)value.lVal);
    m_flag_DataForDeviceCandidates = true;
}

void __stdcall CXMLWrapperTraining::Get_DataForDeviceCandidates(Variant * pValue)
{

}

void __stdcall CXMLWrapperTraining::Set_ToolForTasks(Variant value)
{
	if (value.vt != SYVT_I4 && value.lVal != 0)
	{
		return;	
	}
	
	m_ToolForTasks.push_back((CXMLWrapperToolForTask *)value.lVal);
	m_flag_ToolForTasks = true;
}

void __stdcall CXMLWrapperTraining::Get_ToolForTasks(Variant * pValue)
{

}

void __stdcall CXMLWrapperTraining::Set_DynamicScene(Variant value)
{
	if (value.vt != SYVT_I4 && value.lVal != 0)
	{
		return;	
	}
	
	m_DynamicScene.push_back((CXMLWrapperOrgan *)value.lVal);
	m_flag_DynamicScene = true;
}

void __stdcall CXMLWrapperTraining::Get_DynamicScene(Variant * pValue)
{

}

void __stdcall CXMLWrapperTraining::Set_StaticScene(Variant value)
{
	if (value.vt != SYVT_I4 && value.lVal != 0)
	{
		return;	
	}
	
	m_StaticScene = (CXMLWrapperStaticScene *)value.lVal;
	m_flag_StaticScene = true;
}

void __stdcall CXMLWrapperTraining::Get_StaticScene(Variant * pValue)
{

}

void __stdcall CXMLWrapperTraining::Set_ConnectObject( Variant value )
{
	if (value.vt != SYVT_I4 && value.lVal != 0)
	{
		return;	
	}

	m_ConnectObject.push_back((CXMLWrapperConnect *)value.lVal);
	m_flag_ConnectObject = true;

}

void __stdcall CXMLWrapperTraining::Get_ConnectObject( Variant * pValue )
{

}

void __stdcall CXMLWrapperTraining::Set_AdhereObject( Variant value )
{
    if (value.vt != SYVT_I4 && value.lVal != 0)
    {
        return;	
    }

    m_AdhereObject.push_back((CXMLWrapperAdhere *)value.lVal);
    m_flag_AdhereObject = true;

}

void __stdcall CXMLWrapperTraining::Get_AdhereObject( Variant * pValue )
{

}

void __stdcall CXMLWrapperTraining::Set_AdhesionClusters( Variant value )
{
	if (value.vt != SYVT_I4 && value.lVal != 0)
	{
		return;	
	}

	m_AdhesionClusters.push_back((CXMLWrapperAdhesionCluster *)value.lVal);
	m_flag_AdhesionClusters = true;

}

void __stdcall CXMLWrapperTraining::Get_AdhesionClusters( Variant * pValue )
{

}

void __stdcall CXMLWrapperTraining::Set_CollisionObject( Variant value )
{
    if (value.vt != SYVT_I4 && value.lVal != 0)
    {
        return;	
    }

    m_CollisionObject.push_back((CXMLWrapperCollision *)value.lVal);
    m_flag_CollisionObject = true;

}

void __stdcall CXMLWrapperTraining::Get_CollisionObject( Variant * pValue )
{

}

void __stdcall CXMLWrapperTraining::Set_PartScene( Variant value )
{
	if (value.vt != SYVT_I4 && value.lVal != 0)
	{
		return;	
	}

	m_PartScene.push_back((CXMLWrapperPart *)value.lVal);
	m_flag_PartScene = true;
}

void __stdcall CXMLWrapperTraining::Get_PartScene( Variant * pValue )
{

}

void __stdcall CXMLWrapperTraining::Set_PursueObject(Variant value)
{
	if (value.vt != SYVT_I4 && value.lVal != 0)
	{
		return;	
	}

	m_PursueObject.push_back((CXMLWrapperPursue *)value.lVal);
	m_flag_PursueObject = true;
}

void __stdcall CXMLWrapperTraining::Get_PursueObject( Variant* pValue )
{

}

void __stdcall CXMLWrapperTraining::Set_MucousObject( Variant value )
{
	if (value.vt != SYVT_I4 && value.lVal != 0)
	{
		return;	
	}
	m_vecMucousObject.push_back( ( CXMLWrapperMucous* )value.lVal );
}
void __stdcall CXMLWrapperTraining::Get_MucousObject( Variant* pValue )
{

}


void __stdcall CXMLWrapperTraining::Set_Movies( Variant value )
{
	if (value.vt != SYVT_I4 && value.lVal != 0)
	{
		return;	
	}

	m_Movies.push_back((CXMLWrapperMovie *)value.lVal);
	m_flag_Movies = true;
}

void __stdcall CXMLWrapperTraining::Get_Movies( Variant * pValue )
{

}

void __stdcall CXMLWrapperTraining::Set_Scores(Variant value)
{
	if (value.vt != SYVT_I4 && value.lVal != 0)
	{
		return;
	}

	m_Scores.push_back((CXMLWrapperScore *)value.lVal);
	m_flag_Scores = true;
}

void __stdcall CXMLWrapperTraining::Get_Scores(Variant * pValue)
{

}

void __stdcall CXMLWrapperTraining::Set_OnLineGrades(Variant value)
{
	if (value.vt != SYVT_I4 && value.lVal != 0)
	{
		return;
	}

	m_OnLineGrades.push_back((CXMLWrapperOnLineGrade *)value.lVal);
	m_flag_OnLineGrades = true;
}

void __stdcall CXMLWrapperTraining::Get_OnLineGrades(Variant * pValue)
{

}

void __stdcall CXMLWrapperTraining::Set_OperateItems(Variant value)
{
	if (value.vt != SYVT_I4 && value.lVal != 0)
	{
		return;
	}

	m_OperateItems.push_back((CXMLWrapperOperateItem *)value.lVal);
	m_flag_OperateItems = true;
}

void __stdcall CXMLWrapperTraining::Get_OperateItems(Variant * pValue)
{

}

void __stdcall CXMLWrapperTraining::Set_CommonOperateItems(Variant value)
{
	if (value.vt != SYVT_I4 && value.lVal != 0)
	{
		return;
	}

	m_CommonOperateItems.push_back((CXMLWrapperOperateItem *)value.lVal);
	m_flag_CommonOperateItems = true;
}

void __stdcall CXMLWrapperTraining::Get_CommonOperateItems(Variant * pValue)
{

}

void __stdcall CXMLWrapperTraining::Set_Tips(Variant value)
{
	if (value.vt != SYVT_I4 && value.lVal != 0)
	{
		return;
	}

	m_Tips.push_back((CXMLWrapperTip *)value.lVal);
	m_flag_Tips = true;
}

void __stdcall CXMLWrapperTraining::Get_Tips(Variant * pValue)
{

}

void __stdcall CXMLWrapperTraining::Set_Shadows(Variant value)
{
	if (value.vt != SYVT_I4 && value.lVal != 0)
	{
		return;	
	}

	m_Shadows.push_back((CXMLWrapperShadow *)value.lVal);
	m_flag_Shadows = true;
}

void __stdcall CXMLWrapperTraining::Get_Shadows(Variant * pValue)
{

}

void __stdcall CXMLWrapperTraining::Set_Spheres(Variant value)
{
	if (value.vt != SYVT_I4 && value.lVal != 0)
	{
		return;	
	}

	m_Spheres.push_back((CXMLWrapperSphere *)value.lVal);
	m_flag_Spheres = true;
}

void __stdcall CXMLWrapperTraining::Get_Spheres(Variant * pValue)
{

}

void __stdcall CXMLWrapperTraining::Set_Detectors(Variant value)
{
	if (value.vt != SYVT_I4 && value.lVal != 0)
	{
		return;	
	}

	m_Detectors.push_back((CXMLWrapperDetector *)value.lVal);
	m_flag_Detectors = true;
}

void __stdcall CXMLWrapperTraining::Get_Detectors(Variant * pValue)
{

}

void __stdcall CXMLWrapperTraining::Set_OrganTranslations(Variant value)
{
	if (value.vt != SYVT_I4 && value.lVal != 0)
	{
		return;	
	}

	m_OrganTranslations.push_back((CXMLWrapperOrganTranslation *)value.lVal);
	m_flag_OrganTranslations = true;
}

void __stdcall CXMLWrapperTraining::Get_OrganTranslations(Variant * pValue)
{

}

void __stdcall CXMLWrapperTraining::Set_WaterPools(Variant value)
{
	if (value.vt != SYVT_I4 && value.lVal != 0)
	{
		return;	
	}

	m_WaterPools.push_back((CXMLWrapperWaterPool *)value.lVal);
	m_flag_WaterPools = true;
}

void __stdcall CXMLWrapperTraining::Get_WaterPools(Variant * pValue)
{

}

void __stdcall CXMLWrapperTraining::Set_ViewDetections(Variant value)
{
	if (value.vt != SYVT_I4 && value.lVal != 0)
	{
		return;	
	}

	m_ViewDetections.push_back((CXMLWrapperViewDetection *)value.lVal);
	m_flag_ViewDetections = true;
}

void __stdcall CXMLWrapperTraining::Get_ViewDetections(Variant * pValue)
{

}

