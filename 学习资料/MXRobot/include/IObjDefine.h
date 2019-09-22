#pragma once
#include "IMXDefine.h"

typedef enum MeshType
{
	EMT_NO_MESH = -1,
	EMT_SIMPLE_MESH,
	EMT_NORMAL_MESH,
	EMT_DATA_MESH
}TEMT;

typedef enum DynamicObjType
{
	EDOT_NO_TYPE = -1,
	EDOT_BRAVERY_ARTERY = 0,	// ���Ҷ���
	EDOT_COMMON_BILE_DUCT = 1,	// ���ܹ�
	EDOT_CYSTIC_DUCT = 2,		// ���ҹ�
	EDOT_GALLBLADDER = 3,		// ����
	EDOT_HEPATIC_ARTERY = 4,	// �ζ���
	EDOT_LIVER = 5,				// ��
	EDOT_VEIN = 6,				// ��������

	EDOT_RENAL_ARTERY = 7,
	EDOT_RENAL_VEIN = 8,
	EODT_URETER = 9,
	EODT_VEINCONNECT = 10,
	EODT_VEINBOTTOMCONNECT = 11,
	EODT_UTERUS = 12,
	EDOT_APPENDIX = 13,
	EDOT_APPENDMENSTORY = 14,
	EDOT_LARGEINTESTINE = 15,	//��
	EODT_PEITAI = 16,    
	EODT_DOME_ACTIVE=17,		//��̬����
    EDOT_UPPERGI = 18,			// ��������
    EDOT_SIGMOIDCUTPART = 19,
	EDOT_SIGMOIDNOCUTPART = 20,
    EDOT_IMA = 21,
	EDOT_IMV = 22,
    
    EDOT_SIGMOIDCONNECT = 24,    

    EDOT_UPPERLOBE_LEFT = 26,//�����Ҷ
	EDOT_LOWERLOBE_LEFT = 27,//�����Ҷ
	EDOT_UPPERLOBE_RIGHT = 28,//�����Ҷ
	EDOT_LOWERLOBE_RIGHT = 29,//�����Ҷ
	EDOT_OULMONARYARTERY = 30,//�ζ���
	EDOT_BRONCHIAL = 31, //����
    EDOT_LUNGVEIN  = 32, //�ξ���
    EDOT_LUNG_FASICA = 33,
	EDOT_LUNG_DIAPHRAGM = 34,//diaphragm;


    EDOT_ADHESION = 35,
    
	EDOT_ORGAN_MAX = 36,

	EDOT_HELPER_OBJECT0 = 37,

	EDOT_TOOL_OBJECT = 38,
    EDOT_LUNG_YEJIANLIE = 39,
    EDOT_GONADAL = 40,

    EDOT_GEROTAS = 50,
    EDOT_KIDNEY_VESSELS = 51,
    EDOT_MESOCOLON = 52,
    EDOT_SPLEEN = 53,
    EDOT_SPLEENNECK = 54,
    EDOT_KIDNEYCONNECT_STRONG = 55,
    EDOT_KIDNEYCONNECT_WEAK = 56,
    
	EDOT_ROUND_LIGMENT = 57,//��Բ�ʹ�

	//���Ʒֲ�
	EDOT_WOMB_BROAD_MID = 80,
	EDOT_WOMB_BROAD_TOP = 81,
	Ovarian_Ligament_Left = 82,
	Ovary_Left = 83,
	Ovarian_Ligament_Right = 84,
	Ovary_Right = 85,

	EDOT_ORGAN_LIMIT = 1000,
}TEDOT;

enum OrganPropertyName
{
	/// ���ٱ���
	OPN_AlreadyBeCut,
	/// ��������������
	OPN_BeCutOnIncorrectArea,
	/// ��ɼ�����
	OPN_FinishCut
};
