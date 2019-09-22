#pragma once
#include "ui_SYAdminEntityTrainAnalysisWindow.h"
#include <QTableWidgetItem>
#include <QSqlRecord>
#include <map>

class SYEntityTrainReportFormWindow;
class SYAdminScreenshotDisplayer;
class RbMoviePlayer;

class SYAdminEntityTrainAnalysisWindow : public QWidget
{
	Q_OBJECT
public:
	SYAdminEntityTrainAnalysisWindow(QWidget* parent = nullptr);

	~SYAdminEntityTrainAnalysisWindow();

	void LoadTrainStatisticData();

private:
	static void SetTableWidgetItemAttribute(QTableWidgetItem* item,const QColor& color,int recordIndex,bool needUnderline = false);

	void ClearRankTable();

	void LoadRankData();

	void LoadTrainRecords();

	void InitTrainRecordTableWidget();

private slots:
	void on_userFilter_currentIndexChanged(int index);

	void on_classFilter_currentIndexChanged(int index);

	void on_groupFilter_currentIndexChanged(int index);

	void updateTable(const std::vector<int>& recordIndices);

	void onClickedTrainRecordCell(int row, int col);

private:
	Ui::SYAdminEntityTrainAnalysisWindow ui;

	std::vector<QSqlRecord> m_allRecords;
	std::vector<int> m_recordIndices;

	std::map<int, std::vector<int>> m_userRecordMapById;
	std::map<QString, std::vector<int>> m_userRecordMapByClass;
	std::map<QString, std::vector<int>> m_userRecordMapByGroup;

	bool m_canUpdateData;

	SYEntityTrainReportFormWindow* m_trainReportFormWindow;
	SYAdminScreenshotDisplayer* m_screenshotDisplayer;
	RbMoviePlayer* m_moviePlayer;
};

