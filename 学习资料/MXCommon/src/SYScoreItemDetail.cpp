#include "..\include\SYScoreItemDetail.h"


SYScoreItemDetail::SYScoreItemDetail()
	:m_scoreTableCode(),
	m_scoreCode(),
	m_time(-1)
{

}

SYScoreItemDetail::SYScoreItemDetail(const QString& scoreTableCode, const QString& scoreCode,int time)
	:m_scoreTableCode(scoreTableCode),
	m_scoreCode(scoreCode),
	m_time(time)
{

}

SYScoreItemDetail::~SYScoreItemDetail()
{

}
