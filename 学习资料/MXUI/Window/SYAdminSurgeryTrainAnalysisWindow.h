#pragma once
#include "SYAdminSkillTrainAnalysisWindow.h"

class SYAdminSurgeryTrainAnalysisWindow : public SYAdminSkillTrainAnalysisWindow
{
public:
	SYAdminSurgeryTrainAnalysisWindow(QWidget* parent = nullptr);

	~SYAdminSurgeryTrainAnalysisWindow();

	virtual void LoadTrainStatisticData();
};

