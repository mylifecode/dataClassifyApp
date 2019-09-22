#pragma once
#include <QWidget>
#include <QTableWidget>
#include <vector>
#include <QSqlRecord>
#include "ui_SYTrainRecordWindow.h"

class SYTrainRecordDetailWindow;
class SYEntityTrainReportFormWindow;
class SYScreenshotDisplayer;
class RbMoviePlayer;

class SYTrainRecordWindow : public QWidget
{
	Q_OBJECT
public:
	SYTrainRecordWindow(const std::vector<QSqlRecord>& records,QWidget* parent = nullptr);

	~SYTrainRecordWindow();

public:
	void updateTableWidget();

private slots:
	void onActiveComboBoxItem(int index);

	void onItemClicked(int row, int col);

	void onBackToScoreTableWidget();

private:
	void addRecord(int trainType);

protected:
	void showEvent(QShowEvent* event);

private:
	Ui::trainRecordWindow ui;

	const std::vector<QSqlRecord>& m_records;
	QTableWidget* m_tableWidget;

	SYTrainRecordDetailWindow* m_trainRecordDetailWindow;
	SYEntityTrainReportFormWindow* m_entityTrainReportFormWindow;
	SYScreenshotDisplayer* m_screenshotDisplayer;
	RbMoviePlayer* m_moviePlayer;
};

