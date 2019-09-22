#pragma once
using namespace std;

struct TipInfo
{
	//提示位置  
	enum TipPosition
	{
		TP_NONE,
		TP_LeftUp,
		TP_LeftDown,
		TP_RightUp,
		TP_RightDown,
		TP_Center,
		TP_PopUp,
		TP_CustomPos
	};

	enum TipIconType
	{
		TIT_SCORE_INFO,
		TIT_SCORE_WARNING,
		TIT_TIP_INFO,
		TIT_TIP_WARNING,
		TIT_GUIDE,
		TIT_TIP,
		TIT_TIP_OTHER
	};

	TipPosition pos;
	std::string title;
	QString str;
	int nDuration;
	int msLast;//持续时间
	int msDelay;//延时提示
	TipIconType eIconType;
	int nCustomPosX;
	int nCustomPosY;

	TipInfo()
	{
		pos = TP_NONE;
		title = "";
		str = "";
		msLast = 0;
		msDelay = 0;
		eIconType = TIT_TIP_INFO;
		nCustomPosX = 0;
		nCustomPosY = 0;
	}

	TipInfo(TipPosition pos, const std::string & title, const QString & str, int msLast, int msDelay, TipIconType eIconType, int nCustomPosX, int nCustomPosY)
	{
		this->pos = pos;
		this->title = title;
		this->str = str;
		this->msLast = msLast;
		this->msDelay = msDelay;
		this->eIconType = eIconType;
		this->nCustomPosX = nCustomPosX;
		this->nCustomPosY = nCustomPosY;
	}
};