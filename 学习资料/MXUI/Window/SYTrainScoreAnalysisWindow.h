#pragma once
#include "ui_SYTrainScoreAnalysisWindow.h"
#include <vector>
#include <QSqlRecord>

class SYTrainScoreAnalysisWindow : public QWidget
{
public:
	SYTrainScoreAnalysisWindow(QWidget* parent = nullptr);

	~SYTrainScoreAnalysisWindow();

	void updateContent(int userId, int trainTypeCode,const std::vector<QSqlRecord>& records);

private:
	Ui::SYTrainScoreAnalysisWindow ui;
};

