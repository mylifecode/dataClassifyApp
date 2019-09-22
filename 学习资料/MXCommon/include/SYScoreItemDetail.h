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
	/// ���ֱ����
	QString m_scoreTableCode;

	/// ���������
	QString m_scoreCode;

	/// ����ʱ�䣬��λΪ�룬�����ѵ����ʼʱ��
	int m_time;
};

