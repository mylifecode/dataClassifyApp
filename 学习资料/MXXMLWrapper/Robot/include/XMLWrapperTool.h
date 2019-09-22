/**Author:zx**/
#pragma once
#include "XMLSerialize.h"

class CXMLWrapperMeshNode;
class CXMLWrapperTool : public CXMLSerialize
{
public:
	CXMLWrapperTool(void);
	~CXMLWrapperTool(void);

	IMPL_ATTRIBUTE_STRING(Name)
	IMPL_ATTRIBUTE_STRING(Type)
	IMPL_ATTRIBUTE_STRING(SubType)
	IMPL_ATTRIBUTE_STRING(Path)
	IMPL_ATTRIBUTE_STRING(Root)

	//vector<CXMLWrapperMeshNode *> m_Mesh;
	//bool m_flag_Mesh;

	//void __stdcall Set_Mesh(Variant value);
	//void __stdcall Get_Mesh(Variant * pValue);
	
	IMPL_ATTRIBUTE_STRING(Left)
	IMPL_ATTRIBUTE_STRING(Right)
	IMPL_ATTRIBUTE_STRING(Left_a1)
	IMPL_ATTRIBUTE_STRING(Right_b1)
	IMPL_ATTRIBUTE_STRING(Left_a2)
	IMPL_ATTRIBUTE_STRING(Right_b2)
	IMPL_ATTRIBUTE_STRING(Center)

	//physics
	IMPL_ATTRIBUTE_STRING(Lefts4m)
	IMPL_ATTRIBUTE_STRING(Rights4m)
	IMPL_ATTRIBUTE_STRING(Kernels4m)
	IMPL_ATTRIBUTE_STRING(Lefts2m)
	IMPL_ATTRIBUTE_STRING(Rights2m)
	IMPL_ATTRIBUTE_STRING(OuterLefts2m)
	IMPL_ATTRIBUTE_STRING(OuterRights2m)
	IMPL_ATTRIBUTE_STRING(Kernels2m)

	IMPL_ATTRIBUTE_BOOL(NeedTriangleDetection)

	IMPL_ATTRIBUTE_FLOAT(Mass)
	IMPL_ATTRIBUTE_FLOAT(Stiffness)
	IMPL_ATTRIBUTE_FLOAT(Damp)
	IMPL_ATTRIBUTE_FLOAT(PointDamp)
	IMPL_ATTRIBUTE_FLOAT(PointResistence)
	IMPL_ATTRIBUTE_FLOAT(Strength)
	IMPL_ATTRIBUTE_FLOAT(Thickness)
	IMPL_ATTRIBUTE_FLOAT(HeadLength)
	IMPL_ATTRIBUTE_FLOAT(CollisionRadius)

	IMPL_ATTRIBUTE_FLOAT3(S4mShift)
	IMPL_ATTRIBUTE_FLOAT3(TriangleShift)

	// usage
	IMPL_ATTRIBUTE_BOOL(CanPuncture)
	IMPL_ATTRIBUTE_BOOL(CanCut)
	IMPL_ATTRIBUTE_BOOL(CanClamp)
	IMPL_ATTRIBUTE_BOOL(CanRelease)
	IMPL_ATTRIBUTE_BOOL(CanFire)
	IMPL_ATTRIBUTE_BOOL(CanSeparate)
	IMPL_ATTRIBUTE_BOOL(CanClosed)		//是否能够夹闭
	IMPL_ATTRIBUTE_BOOL(ExcuHeldPoints)

	IMPL_ATTRIBUTE_BOOL(CanDistance)

	IMPL_ATTRIBUTE_LONG(ElecPriority) //通电优先级
	IMPL_ATTRIBUTE_BOOL(IsIgnoreElecPriority) //使用电按键 但非通电器械可用



	// Clip Applier attribute
	IMPL_ATTRIBUTE_FLOAT(ClipApplierHeadHalfWidth)
	IMPL_ATTRIBUTE_FLOAT(ClipApplierHeadLength)

	//Scissors attribute
	IMPL_ATTRIBUTE_FLOAT(ToolUpRelaxAngle)
	IMPL_ATTRIBUTE_FLOAT(ToolDownRelaxAngle)
	IMPL_ATTRIBUTE_FLOAT(ToolUpTightAngle)
	IMPL_ATTRIBUTE_FLOAT(ToolDownTightAngle)
	IMPL_ATTRIBUTE_FLOAT(ToolHeadAngle)
	IMPL_ATTRIBUTE_FLOAT(ToolUpLength)
	IMPL_ATTRIBUTE_FLOAT(ToolDownLength)	

	//ElectricTime
	IMPL_ATTRIBUTE_FLOAT(ElectricTime)
    //能否产生出血效果
	IMPL_ATTRIBUTE_BOOL(CanBlood)
	IMPL_ATTRIBUTE_BOOL(CanStopBlood)

	//是否绘制图形
	IMPL_ATTRIBUTE_BOOL(CanDrawS2M)
	IMPL_ATTRIBUTE_BOOL(CanDrawS4M)
	IMPL_ATTRIBUTE_BOOL(CanDrawTriangle)

	IMPL_ATTRIBUTE_FLOAT3(HookPosition)

	IMPL_ATTRIBUTE_BOOL(CanAdhesionForce)

	IMPL_ATTRIBUTE_LONG(CutNumber)

	IMPL_ATTRIBUTE_FLOAT(MaxShaftAside)

	DECLARE_SERIALIZATION_CLASS(CXMLWrapperTool)
};

