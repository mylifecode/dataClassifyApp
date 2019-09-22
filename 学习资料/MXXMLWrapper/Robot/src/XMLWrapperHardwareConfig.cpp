#include "StdAfx.h"
#include "XMLWrapperHardwareConfig.h"


BEGIN_IMPL_SERIALIZATION_CLASS(CXMLWrapperHardwareConfig)
REGISTER_CLASS(HardwareConfig)
//////////////////////////////////////////////////////////////////////////
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig,Use_ShaftLeft, VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig,Use_ShaftRight,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig,Use_CameraSmall,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig,Use_CameraLarge,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig,Use_MisRobot,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig,Use_MisRobot_Spec,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig,Use_MisRobot_Left,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig,Use_MisRobot_Right,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig,Use_Phantom_SelectTool,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig,Adjust_Parameter,VALUE)

REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig, Left_Angle_K, VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig, Left_Movement_K, VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig, Left_Shaft_K,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig, Left_Shaft_B,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig, ClAppliper_Left_Shaft_K,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig, ClAppliper_Left_Shaft_B,VALUE)

REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig, Right_Angle_K, VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig, Right_Movement_K, VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig, Right_Shaft_K,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig, Right_Shaft_B,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig, ClAppliper_Right_Shaft_K,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig, ClAppliper_Right_Shaft_B,VALUE)

REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig, Camera_Angle_K, VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig, Camera_Movement_K, VALUE)

REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig,Force_X_K,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig,Force_X_B,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig,Force_Y_K,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig,Force_Y_B,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig,Force_Z_K,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig,Force_Z_B,VALUE)

REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig,Force_Transport_Multiplier,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig,Force_Impulse_Multiplier,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig,Switch_Distance,VALUE)

REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig,Distance_Release_Limit,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig,Distance_Release_Limit_ElectricHook,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig,Force_Radius,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig,Force_Strength,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig,Stiffness,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperHardwareConfig,Clamp_Stiffness,VALUE)

END_IMPL_SERIALIZATION_CLASS

CXMLWrapperHardwareConfig::CXMLWrapperHardwareConfig(void)
{
	INIT_ATTRIBUTE_STRING(Use_ShaftLeft)
	INIT_ATTRIBUTE_STRING(Use_ShaftRight)
	INIT_ATTRIBUTE_STRING(Use_CameraSmall)
	INIT_ATTRIBUTE_STRING(Use_CameraLarge)
    INIT_ATTRIBUTE_BOOL(Use_MisRobot)
    INIT_ATTRIBUTE_BOOL(Use_MisRobot_Spec)
    INIT_ATTRIBUTE_BOOL(Use_MisRobot_Left)
    INIT_ATTRIBUTE_BOOL(Use_MisRobot_Right)
    INIT_ATTRIBUTE_BOOL(Use_Phantom_SelectTool)
    INIT_ATTRIBUTE_BOOL(Adjust_Parameter)

	INIT_ATTRIBUTE_FLOAT3_VALUE(Left_Angle_K, 1.0f, 1.0f, 1.0f)
	INIT_ATTRIBUTE_FLOAT3_VALUE(Left_Movement_K, 1.0f, 1.0f, 1.0f)
    INIT_ATTRIBUTE_FLOAT(Left_Shaft_K)
    INIT_ATTRIBUTE_FLOAT(Left_Shaft_B)
	INIT_ATTRIBUTE_FLOAT(ClAppliper_Left_Shaft_K)
	INIT_ATTRIBUTE_FLOAT(ClAppliper_Left_Shaft_B)

	INIT_ATTRIBUTE_FLOAT3_VALUE(Right_Angle_K, 1.0f, 1.0f, 1.0f)
	INIT_ATTRIBUTE_FLOAT3_VALUE(Right_Movement_K, 1.0f, 1.0f, 1.0f)
    INIT_ATTRIBUTE_FLOAT(Right_Shaft_K)
    INIT_ATTRIBUTE_FLOAT(Right_Shaft_B)
	INIT_ATTRIBUTE_FLOAT(ClAppliper_Right_Shaft_K)
	INIT_ATTRIBUTE_FLOAT(ClAppliper_Right_Shaft_B)

	INIT_ATTRIBUTE_FLOAT3_VALUE(Camera_Angle_K, 1.0f, 1.0f, 1.0f)
	INIT_ATTRIBUTE_FLOAT3_VALUE(Camera_Movement_K, 1.0f, 1.0f, 1.0f)

    INIT_ATTRIBUTE_FLOAT(Force_X_K)
    INIT_ATTRIBUTE_FLOAT(Force_X_B)
    INIT_ATTRIBUTE_FLOAT(Force_Y_K)
    INIT_ATTRIBUTE_FLOAT(Force_Y_B)
    INIT_ATTRIBUTE_FLOAT(Force_Z_K)
    INIT_ATTRIBUTE_FLOAT(Force_Z_B)

	INIT_ATTRIBUTE_FLOAT(Force_Transport_Multiplier)
	INIT_ATTRIBUTE_FLOAT_VALUE(Force_Transport_Multiplier, 10.0)
	INIT_ATTRIBUTE_FLOAT(Force_Impulse_Multiplier)
	INIT_ATTRIBUTE_FLOAT_VALUE(Force_Impulse_Multiplier, 1.0)
    INIT_ATTRIBUTE_BOOL(Switch_Distance)

	INIT_ATTRIBUTE_FLOAT(Distance_Release_Limit)
	INIT_ATTRIBUTE_FLOAT(Distance_Release_Limit_ElectricHook)
	INIT_ATTRIBUTE_FLOAT(Force_Radius)
	m_Force_Radius = 5.0f;
	INIT_ATTRIBUTE_FLOAT(Force_Strength)
	m_Force_Strength = 1.0f;
	
	INIT_ATTRIBUTE_FLOAT_VALUE(Stiffness, 0.1)
	INIT_ATTRIBUTE_FLOAT_VALUE(Clamp_Stiffness, 0.1)
}


CXMLWrapperHardwareConfig::~CXMLWrapperHardwareConfig(void)
{
}
