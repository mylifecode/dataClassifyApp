#pragma once
#include "ui_SYAdminTrainAnalysisWindow.h"
#include <QTableWidgetItem>

struct TrainAbilityCounter;
enum TrainType;

class SYAdminTrainAnalysisWindow : public QWidget
{
	Q_OBJECT
public:
	SYAdminTrainAnalysisWindow(QWidget* parent = nullptr);

	~SYAdminTrainAnalysisWindow();

	void SetTrainAbilityCounter(TrainType trainType,TrainAbilityCounter* trainAbilityCounter) 
	{ 
		m_trainType = trainType;
		m_trainAbilityCounter = trainAbilityCounter; 
	}

	void UpdateContent();

private:
	void InitTable();

	void ClearTableContent();

	void SetItemAttribute(QTableWidgetItem* item,const QString& text,Qt::Alignment alignment);

	void SetEliteListInfo();

	void SetScoreRankInfo();

	void SetErrorInfo();

protected:
	void showEvent(QShowEvent* showEvent);

	bool eventFilter(QObject* object, QEvent* event);

private:
	Ui::SYAdminTrainAnalysisWindow ui;

	TrainType m_trainType;
	TrainAbilityCounter* m_trainAbilityCounter;
	QLabel* m_bottomBlueLabel;

	const std::size_t m_nMaxRowOfScoreRankTable;
};

