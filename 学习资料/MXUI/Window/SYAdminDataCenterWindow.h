#pragma once
#include "ui_SYAdminDataCenterWindow.h"
#include <QPushButton>
#include <QStackedWidget>

class SYAdminEntityTrainAnalysisWindow;
class SYAdminSkillTrainAnalysisWindow;
class SYAdminSurgeryTrainAnalysisWindow;

class SYAdminDataCenterWindow : public QWidget
{
	Q_OBJECT
public:
	SYAdminDataCenterWindow(QWidget* parent = nullptr);

	~SYAdminDataCenterWindow();

private:
	void InitLayout();

private slots:
	void onEntityTrainAnalysisBtn();

	void onSkillTrainAnalysisBtn();

	void onSurgeryTrainAnalysisBtn();

private:
	Ui::SYAdminDataCenterWindow ui;
	QPushButton* m_entityTrainAnalysisBtn;
	QPushButton* m_skillTrainAnalysisBtn;
	QPushButton* m_surgeryTrainAnalysisBtn;
	QPushButton* m_curSelectedBtn;

	QStackedWidget* m_stackedWidget;

	SYAdminEntityTrainAnalysisWindow* m_entityTrainAnalysisWindow;
	SYAdminSkillTrainAnalysisWindow* m_skillTrainAnalysisWindow;
	SYAdminSurgeryTrainAnalysisWindow* m_surgeryTrainAnalysisWindow;
};
