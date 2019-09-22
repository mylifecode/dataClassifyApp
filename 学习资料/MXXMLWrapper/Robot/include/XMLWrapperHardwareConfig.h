#pragma once
#include "XMLSerialize.h"

class CXMLWrapperHardwareConfig : public CXMLSerialize
{
public:
    CXMLWrapperHardwareConfig(void);
    ~CXMLWrapperHardwareConfig(void);

	IMPL_ATTRIBUTE_STRING(Use_ShaftLeft)
	IMPL_ATTRIBUTE_STRING(Use_ShaftRight)
	IMPL_ATTRIBUTE_STRING(Use_CameraSmall)
	IMPL_ATTRIBUTE_STRING(Use_CameraLarge)
	//////////////////////////////////////////////////////////////////////////
    IMPL_ATTRIBUTE_BOOL(Use_MisRobot)
    IMPL_ATTRIBUTE_BOOL(Use_MisRobot_Spec)
    IMPL_ATTRIBUTE_BOOL(Use_MisRobot_Left)
    IMPL_ATTRIBUTE_BOOL(Use_MisRobot_Right)

	//////////////////////////////////////////////////////////////////////////
    IMPL_ATTRIBUTE_BOOL(Use_Phantom_SelectTool)
    IMPL_ATTRIBUTE_BOOL(Adjust_Parameter)

	IMPL_ATTRIBUTE_FLOAT3(Left_Angle_K)
	IMPL_ATTRIBUTE_FLOAT3(Left_Movement_K)
    IMPL_ATTRIBUTE_FLOAT(Left_Shaft_K)
    IMPL_ATTRIBUTE_FLOAT(Left_Shaft_B)

	IMPL_ATTRIBUTE_FLOAT(ClAppliper_Left_Shaft_K)
	IMPL_ATTRIBUTE_FLOAT(ClAppliper_Left_Shaft_B)

	IMPL_ATTRIBUTE_FLOAT3(Right_Angle_K)
	IMPL_ATTRIBUTE_FLOAT3(Right_Movement_K)
    IMPL_ATTRIBUTE_FLOAT(Right_Shaft_K)
    IMPL_ATTRIBUTE_FLOAT(Right_Shaft_B)

	IMPL_ATTRIBUTE_FLOAT(ClAppliper_Right_Shaft_K)
	IMPL_ATTRIBUTE_FLOAT(ClAppliper_Right_Shaft_B)

	IMPL_ATTRIBUTE_FLOAT3(Camera_Angle_K)
	IMPL_ATTRIBUTE_FLOAT3(Camera_Movement_K)

    IMPL_ATTRIBUTE_FLOAT(Force_X_K)
    IMPL_ATTRIBUTE_FLOAT(Force_X_B)
    IMPL_ATTRIBUTE_FLOAT(Force_Y_K)
    IMPL_ATTRIBUTE_FLOAT(Force_Y_B)
    IMPL_ATTRIBUTE_FLOAT(Force_Z_K)
    IMPL_ATTRIBUTE_FLOAT(Force_Z_B)

	IMPL_ATTRIBUTE_FLOAT(Force_Transport_Multiplier)
	IMPL_ATTRIBUTE_FLOAT(Force_Impulse_Multiplier)
    IMPL_ATTRIBUTE_BOOL(Switch_Distance)
	IMPL_ATTRIBUTE_FLOAT(Force_Release_Limit)
	IMPL_ATTRIBUTE_FLOAT(Force_Release_Limit_ElectricHook)
	IMPL_ATTRIBUTE_FLOAT(Distance_Release_Limit)
	IMPL_ATTRIBUTE_FLOAT(Distance_Release_Limit_ElectricHook)
	IMPL_ATTRIBUTE_FLOAT(Force_Radius)
	IMPL_ATTRIBUTE_FLOAT(Force_Strength)
	
	IMPL_ATTRIBUTE_FLOAT(Stiffness)
	IMPL_ATTRIBUTE_FLOAT(Clamp_Stiffness)


    DECLARE_SERIALIZATION_CLASS(CXMLWrapperHardwareConfig)
};
