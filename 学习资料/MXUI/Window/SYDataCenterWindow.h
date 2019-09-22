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

	/** �������½ǵ�ѵ����������ͼ */
	void updateTrainTimesGraph();

	/** �������Ϸ��ı�״ͼ */
	void updatePieGraph();

	void addTrainAnalysisWindow();

	void updateTrainAnalysisWindow();

private slots:
	void on_trainRecordBtn_clicked();

	void on_skillingTrainAnalysisBtn_clicked();

	void on_surgeryTrainAnalisisBtn_clicked();

private:
	Ui::DataCenterWindow ui;

	/// ��ɶ��Ͻ�
	static const int m_finishProgressUpper;

	/// ѵ�������Ͻ�
	static const int m_trainTimesUpper;

	///ѵ��ʱ���Ͻ�,��λ��Сʱ
	static const int m_trainTimeUpper;

	int m_userId;

	std::vector<QSqlRecord> m_allRecords;
	std::vector<QSqlRecord> m_skillingTrainRecords;
	std::vector<QSqlRecord> m_surgeryTrainRecords;
	
	/// ѵ����������ͼ��ͼ
	QChartView* m_trainTimesGraphView;
	QChart* m_trainTimesChart;
	QBarSet* m_trainTimesBarSet;
	QBarSeries* m_trainTimesBarSeries;
	int m_simulationTrainTimes;
	int m_skillingTrainTimes;
	int m_surgeryTrainTimes;
	int m_totalTrainTimes;

	/// ѵ������������ѵ������������ѵ��������״ͼ
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

