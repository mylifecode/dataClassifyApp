#pragma once
#include <QWidget>
#include <vector>
#include <QSqlRecord>

class SYTrainScoreAnalysisWindow;

class SYTrainAnalysisWindow : public QWidget
{
public:
	SYTrainAnalysisWindow(QWidget* parent = nullptr);

	~SYTrainAnalysisWindow();

	void updateContent(int userId, int trainTypeCode,const std::vector<QSqlRecord>& records);

private:
	SYTrainScoreAnalysisWindow* m_scoreAnalysisWindow;
};

