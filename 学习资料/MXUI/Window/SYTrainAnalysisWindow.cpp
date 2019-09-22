#include "SYTrainAnalysisWindow.h"
#include "SYTrainScoreAnalysisWindow.h"

SYTrainAnalysisWindow::SYTrainAnalysisWindow(QWidget* parent)
	:QWidget(parent),
	m_scoreAnalysisWindow(nullptr)
{
	QVBoxLayout* layout = new QVBoxLayout;

	m_scoreAnalysisWindow = new SYTrainScoreAnalysisWindow;
	layout->addWidget(m_scoreAnalysisWindow);
	layout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding));
	layout->setMargin(0);
	layout->setSpacing(0);

	setLayout(layout);
}

SYTrainAnalysisWindow::~SYTrainAnalysisWindow()
{

}


void SYTrainAnalysisWindow::updateContent(int userId,int trainTypeCode, const std::vector<QSqlRecord>& records)
{
	m_scoreAnalysisWindow->updateContent(userId, trainTypeCode, records);
}