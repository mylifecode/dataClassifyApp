#pragma once
#include <OgreRoot.h>
#include <OgreConfigFile.h>
#include <OgreRenderSystem.h>

#include <OgrePrerequisites.h>
#include <OgreEntity.h>
#include <OgreVector3.h>
#include <OgreOverlay.h>
#include <OgreOverlayContainer.h>
#include <OgreOverlayElement.h>
#include <OgreOverlayManager.h>

#define CALL(p, member)	if (p) { p->member; } 
#define CALLEX(p, member, ret) if ( p ) { ret = p->member; }

#undef	SAFE_DELETE
#define SAFE_DELETE(p)          { if(p) { delete (p); (p) = 0; } }

#undef	SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p)    { if(p) { delete[] (p); (p) = 0; } }

#undef	SAFE_RELEASE
#define SAFE_RELEASE(p)         { if(p) { (p)->Release(); (p) = 0; } }

enum 
{
	SINGLE_PORT_MODEL=0,//单孔模式
	MULTIL_PORT_MODEL//多孔模式
};
// Config File Path Define
#define CONFIG_FILE_PATH "..\\Config\\"
#define DATA_FILE_PATH "..\\Data\\"

#define MODULE_PATH  DATA_FILE_PATH##"module\\"
//#define SINGLEPORT_CONFIG_FILE_PATH "..\\Config\\SinglePortConfig\\"
//#define MULTIPORT_CONFIG_FILE_PATH "..\\Config\\Train\\"

//#define SINGLEPORT_GLOBAL_CONFIG_FILE SINGLEPORT_CONFIG_FILE_PATH##"global.xml"
#define MULTIPORT_GLOBAL_CONFIG_FILE MODULE_PATH##"global.xml"

#define TOOL_CONFIG_FILE DATA_FILE_PATH##"InstrumentAttriConfig.xml"
#define PARTICLES_CONIFG_FILE DATA_FILE_PATH##"particles.xml"
#define HARDWARE_CONFIG_FILE CONFIG_FILE_PATH##"hardware.xml"

#define TRAINING_FILE(file_name) CONFIG_FILE_PATH##"training_"##file_name##".xml"
#define GALLBLADDER_FSM_XLS CONFIG_FILE_PATH##"GallbladderFSM.xls" // Gallbladder Training xls
#define GALLBLADDER_FSM_BINARY CONFIG_FILE_PATH##"GallbladderFSM.binary" // Gallbladder Training binary




// Render Window Name
#define RENDER_WINDOW_LARGE "TestOgreWidget_1"
#define RENDER_WINDOW_SMALL "TestOgreWidget_2"

// Camera Name
#define CAMERA_NAME_1 "Camera001$"
#define CAMERA_NAME_2 "Camera002$"
#define CAMERA_NAME_3 "Camera003$"
#define CAMERA_NAME_4 "Camera004$"

// Effect Type Define
//#define PT_SMOKE_1 "Particle/smoke1"
//#define PT_WATER_1 "Particle/water1"
//#define PT_BLOOD_1 "Particle/blood1"

//#define PT_BLOOD_00 "Effect/Blood"
//#define PT_BLOOD_01 "Effect/Blood1"
//#define PT_BLOOD_02 "Effect/Blood2"

//#define PT_SMOKE_00 "Effect/Smoke"

#define PT_SPARK_00 "Effect/SPark"

#define PT_BLEED_00 "Effect/bleed"
#define PT_BLEED_01 "Effect/duct"
#define PT_BLEED_02 "Effect/bleed02"
#define PT_BLEED_BIGVESSEL "Effect/BleedForBigVessel"
#define PT_BLEED_SMALLVESSEL "Effect/BleedForSmallVessel"



#define CANVAS "canvas.png"
#define DEFAULT_CANVAS_MATERIAL "gallbladde"

// Tool Type Define & Sub Type Define
#define TT_AIRNEEDLE																"AirNeedle"
#define TT_ELECTRIC_HOOK														"ElectricHook"
#define TT_T_HOOK_ELECTRODE												"T-Hook Electrode"
#define TT_T_NEEDLE_ELECTRODE											"T-Needle Electrode"
#define TT_ELECTRIC_SPATULA												"ElectricSpatula"
#define TT_GRASPING_FORCEPS												"GraspingForceps"
#define TT_GALL_FORCEPS												    "GallForceps"
#define TT_DISSECTING_FORCEPS											"DissectingForceps"
#define TT_DISSECTING_FORCEPS_SIZE10									"DissectingForceps_Size10"
#define TT_BIGHEMOCLIP														"BigHemoClip"
#define TT_HEMOCLIP																"HemoClip"
#define TT_NAIL																			"Nail"
#define TT_KNOTTER																	"Knotter"
#define TT_LINEARCUTTINGSTAPLER										"LinearCuttingStapler"
#define TT_CAUTERY																	"Cautery"
#define TT_HARMONICSCALPEL  											"Ultrasonic scalpe"
#define TT_BIOPSYFORCEPS														"BiopsyForceps"
#define TT_OBLIQUE_EVEN_MOUTH_BIPOLAR						"Oblique Even mouth bipolar"
#define TT_ULTRASONIC_SCALPE 											"Ultrasonic scalpe"

#define TT_SCISSORS																"Scissors"
#define TT_T_STRAIGHT_SCISSORS											"T-Straight Scissors"
#define TT_CURVEDSCISSORS											"Curved Scissors"
#define TT_DOUBLEHOOKSCISSORS										"DoubleHookScissors"
#define TT_STRAIGHTMINISCISSORS										"StraightMiniScissors"
#define TT_T_DOUBLE_HOOK_SCISSORS								"T-Double Hook Scissors"
#define TT_T_STRAIGHT_MINI_SCISSORS								"T-Straight Mini Scissors"

#define TT_STRAW																	"Straw"
#define TT_TRIELCON																"Trielcon"
#define TT_TROCHAR																"Trochar"

#define TT_CLIPAPPLIER															"ClipApplier"
#define TT_CLIP_APPLICATOR													"Clip applicator"
#define TT_NEEDLEHOLDER														"NeedleHolder"
#define TT_SILVERCLIP																"SilverClip"
#define TT_BIPOLARELECFORCEPS											"BipolarElecForceps"
#define TT_SILVERCLIPHOLDER												"SilverClipHolder"
#define TT_Y_CLIP_APPLIER														"Y-Clip applier"
#define TT_LOSSLESSFORCEPS													"LosslessForceps" 
#define TT_RINGFORCEPS													"RingForceps" 

#define TT_HOOKCLIPPLIERS													"HookClipPliers"
#define TT_HEM_O_LOK_PLIERS												"hem-o-lok_pliers"
#define TT_HOOKCLIP																"HookClip"
#define TT_THINMENBRANEFORCEPS									"ThinMenbraneForceps"
#define TT_THIN_MENBRANE_DISSENCTOR							"thin menbrane dissenctor"
#define TT_INJECT_NEEDLE														"Inject Needle"
#define TT_SUCTION_AND_IRRIGATION_TUBE						"Suction And Irrigation Tube"
#define TT_SPECIMENBAG														"SpecimenBag"



#define  NORMAL_PATH 256

#define  USE_NATIVE_NORMALS

#define GREAT_THAN_ORGAN_BOUND 100
#define OGAN_ID_BOUND GREAT_THAN_ORGAN_BOUND
#define GREAT_THAN_THREAD_BOUND 200
#define MAX_FIXED_BOUND GREAT_THAN_THREAD_BOUND

#define BIGGLWIDGET_WIDTH  1038.0f 
#define BIGGLWIDGET_HEIGHT 839.0f 

class CFSMachine;
typedef void (*EventFunc)(CFSMachine * pFSM, std::wstring strParam);