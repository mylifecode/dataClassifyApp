#pragma once
#include "ui_SYAdminTrainStatisticWindow.h"
#include <QChart>
#include <QChartView>
#include <QBarSet>
#include <QBarSeries>
#include <QBarCategoryAxis>
#include <QValueAxis>

using namespace QtCharts;

enum TrainType;

class SYAdminTrainStatisticWindow : public QWidget
{
public:
	SYAdminTrainStatisticWindow(QWidget* parent = nullptr);

	~SYAdminTrainStatisticWindow();

	void SetTrainType(TrainType type);

	void LoadData();

	void SetChartTitle(const QString& leftTile, const QString& rightTile);

private:
	void UpdateChart1();

	void UpdateChart2();

	/**
		axisY1RangLow、axisY1RangUp：左侧条形图Y轴的范围
		axisY2RangLow、axisY2RangUp：又侧条形图Y轴的范围
	*/
	void SetAxisData(const std::vector<int>& timesDatas,const std::vector<int>& totalTimeDatas,const std::vector<QString>& barLabels,
					 int axisY1RangLow,int axisY1RangUp,
					 int axisY2RangLow,int axisY2RangUp,
					 bool enforceBarLabels = false);

	void SetAxisColor(QBarCategoryAxis* xAxis, QValueAxis* yAxis);

private:
	Ui::SYAdminTrainStatisticWindow ui;

	TrainType m_trainType;

	QChartView* m_chartViews[2];
	QChart* m_charts[2];
	QBarSet* m_barSets[2];
	QBarSeries* m_barSeries[2];
};

