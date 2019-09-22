#include "SYAdminSurgeryTrainAnalysisWindow.h"
#include <MxDefine.h>
#include "SYStringTable.h"

SYAdminSurgeryTrainAnalysisWindow::SYAdminSurgeryTrainAnalysisWindow(QWidget* parent)
	:SYAdminSkillTrainAnalysisWindow(parent)
{
	m_trainType = TT_SurgeryTrain;
}

SYAdminSurgeryTrainAnalysisWindow::~SYAdminSurgeryTrainAnalysisWindow()
{

}

void SYAdminSurgeryTrainAnalysisWindow::LoadTrainStatisticData()
{
	ui.trainAnalysisWindow->SetTrainType(m_trainType);
	ui.trainAnalysisWindow->SetChartTitle(SYStringTable::GetString(SYStringTable::STR_RECENT6MONTHTRAINTIMES),
										  SYStringTable::GetString(SYStringTable::STR_RECENT6MONTHTRAINTIME));
	ui.trainAnalysisWindow->LoadData();
}