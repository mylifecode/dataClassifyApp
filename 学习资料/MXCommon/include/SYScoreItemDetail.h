#pragma once
#include "MXCommon.h"
#include <QString>

class MXCOMMON_API SYScoreItemDetail
{
public:
	SYScoreItemDetail();

	SYScoreItemDetail(const QString& scoreTableCode, const QString& scoreCode,int time = -1);

	~SYScoreItemDetail();

	const QString& GetScoreTableCode() const { return m_scoreTableCode; }
	void SetScoreTableCode(const QString& code) { m_scoreTableCode = code; }

	const QString& GetScoreCode() const { return m_scoreCode; }
	void SetScoreCode(const QString& code) { m_scoreCode = code; }

	int GetTime() const { return m_time; }
	void SetTime(int time) { m_time = time; }


private:
	/// 评分表编码
	QString m_scoreTableCode;

	/// 评分项编码
	QString m_scoreCode;

	/// 触发时间，单位为秒，相对于训练开始时间
	int m_time;
};

