#include "SYUserData.h"


SYUserData::SYUserData(SYUserData* parent)
	:m_parent(parent),
	m_dataType(UDT_UserData),
	m_dataHasChanged(false),
	m_codeHasChanged(false),
	m_flag(0)
{

}

SYUserData::SYUserData(const QString& code)
	:m_parent(nullptr),
	m_code(code),
	m_oldCode(code),
	m_dataType(UDT_UserData),
	m_dataHasChanged(false),
	m_codeHasChanged(false),
	m_flag(0)
{

}

SYUserData::~SYUserData()
{

}

//////////////////////////////////////////////////////////////////////////
SYTrainData::SYTrainData()
{
	m_dataType = UDT_Train;
}

SYTrainData::SYTrainData(const QString& code)
	:SYUserData(code)
{
	m_dataType = UDT_Train;
}

//////////////////////////////////////////////////////////////////////////
SYStepData::SYStepData()
	:m_isKeyStep(1),
	m_oldKeyStepValue(1),
	m_keyStepChanged(false)

{
	m_dataType = UDT_Step;
}

SYStepData::SYStepData(const QString& code)
	:SYUserData(code),
	m_isKeyStep(1),
	m_oldKeyStepValue(1),
	m_keyStepChanged(false)
{
	m_dataType = UDT_Step;
}

//////////////////////////////////////////////////////////////////////////
SYScoreItemData::SYScoreItemData()
	:m_scoreValue(0)
{
	m_dataType = UDT_ScoreItem;
}

SYScoreItemData::SYScoreItemData(const QString& code)
	:SYUserData(code),
	m_scoreValue(0)
{
	m_dataType = UDT_ScoreItem;
}

//////////////////////////////////////////////////////////////////////////
SYScorePointDetailData::SYScorePointDetailData()
	:m_scoreValue(0),
	//m_type(0),
	//m_isKeyStep(0),
	m_abilitys(0)
{
	m_dataType = UDT_ScorePointDetail;
}

SYScorePointDetailData::SYScorePointDetailData(const QString& code)
	:SYUserData(code),
	m_scoreValue(0),
	//m_type(0),
	//m_isKeyStep(0),
	m_abilitys(0)
{
	m_dataType = UDT_ScorePointDetail;
}

