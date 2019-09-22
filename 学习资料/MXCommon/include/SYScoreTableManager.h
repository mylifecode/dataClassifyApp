#pragma once
#include "MXCommon.h"
#include <QVector>
#include <QString>

class SYScoreTable;

class MXCOMMON_API SYScoreTableManager
{
	SYScoreTableManager();

	~SYScoreTableManager();
public :
	static  SYScoreTableManager* GetInstance();

	void Initialize();

	SYScoreTable* CreateTable();

	/** 根据评分表编码获取相应评分表对象 */
	SYScoreTable* GetTable(const QString& tableCode);

	void DeleteTable(SYScoreTable* table);

	QVector<SYScoreTable*> AllTables() const { return m_scoreTables; }

private:
	
	QVector<SYScoreTable*> m_scoreTables;
};

