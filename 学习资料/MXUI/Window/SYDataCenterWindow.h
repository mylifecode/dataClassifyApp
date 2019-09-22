#pragma once
#include <QWidget>
#include <QSqlRecord>
#include "ui_SYDataCenterWindow.h"
#include <QChartView>
#include <QChart>
#include <QBarSet>
#include <QBarSeries>
#include <QBarCategoryAxis>
#include <QPieSlice>
#include <QPieSeries>

using namespace QtCharts;

class SYTrainRecordWindow;
class SYTrainAnalysisWindow;

class SYDataCenterWindow : public QWidget
{
	Q_OBJECT
public:
	SYDataCenterWindow(QWidget* parent = nullptr);

	~SYDataCenterWindow();

protected:
	void showEvent(QShowEvent* event);

private:
	void setPieLabels();

	void updateRecords();

	void updateTrainOverview();

	/** 更新左下角的训练次数条形图 */
	void updateTrainTimesGraph();

	/** 更新右上方的饼状图 */
	void updatePieGraph();

	void addTrainAnalysisWindow();

	void updateTrainAnalysisWindow();

private slots:
	void on_trainRecordBtn_clicked();

	void on_skillingTrainAnalysisBtn_clicked();

	void on_surgeryTrainAnalisisBtn_clicked();

private:
	Ui::DataCenterWindow ui;

	/// 完成度上界
	static const int m_finishProgressUpper;

	/// 训练次数上界
	static const int m_trainTimesUpper;

	///训练时长上界,单位：小时
	static const int m_trainTimeUpper;

	int m_userId;

	std::vector<QSqlRecord> m_allRecords;
	std::vector<QSqlRecord> m_skillingTrainRecords;
	std::vector<QSqlRecord> m_surgeryTrainRecords;
	
	/// 训练次数条形图视图
	QChartView* m_trainTimesGraphView;
	QChart* m_trainTimesChart;
	QBarSet* m_trainTimesBarSet;
	QBarSeries* m_trainTimesBarSeries;
	int m_simulationTrainTimes;
	int m_skillingTrainTimes;
	int m_surgeryTrainTimes;
	int m_totalTrainTimes;

	/// 训练次数、技能训练分数、手术训练分数饼状图
	QChartView* m_pieGraphView;
	QChart* m_pieGraph;
	QPieSeries* m_trainTimesPieSeries;
	QPieSeries* m_skillingTrainScorePieSeries;
	QPieSeries* m_surgeryTrainScorePieSeries;

	///
	SYTrainRecordWindow* m_trainRecordWindow;
	SYTrainAnalysisWindow* m_skillingTrainAnalysisWindow;
	SYTrainAnalysisWindow* m_surgeryTrainAnalysisWindow;
};

