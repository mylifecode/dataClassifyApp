#pragma once
#include <QWidget>
#include <QSet>
#include <QPushButton>
#include <QHBoxLayout>

#include "ui_FixedToolsMenu.h"

class SYTrainWindow;
class ITool;


class RbFixedToolsMenu : public QWidget
{
	Q_OBJECT
public:
	RbFixedToolsMenu(QWidget* pParent = NULL);
	~RbFixedToolsMenu(void);
	
	void setCurCaseOperationWidget(SYTrainWindow* caseOperationWidget) 
	{ 
		m_caseOperationWidget = caseOperationWidget;
	}

	void ClearAllFixedTool();

private slots:
	void on_fixLeftBtn_clicked();
	void on_fixRightBtn_clicked();

	void on_clearToolsBtn_clicked();

private:
	void AddOneFixedTool(ITool* fixedTool,bool isLeftHand);

private:
	Ui::FixedToolsMenu m_ui;
	SYTrainWindow* m_caseOperationWidget;
	QSet<QPushButton*> m_fixedToolsBtn;
};
