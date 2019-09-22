#pragma once
#include <QWidget>
#include "ui_RbTrainingDebugWindow.h"
#include "EditorTool/EditCamera.h"

class MisNewTraining;
class RbVirtualKeyboard;

class RbPointToolWidget;
class RbVeinConnEditorWidget;
class RbHotchpotchEditorWidget;
class RbVeinConnEditorV2Widget;
class RbAdjustShaderParametersWidget;

class MxTrainingDebugWindow : public QWidget
{
	Q_OBJECT
public:
	MxTrainingDebugWindow(QWidget* parent,  MisNewTraining * trainInEdit);
	~MxTrainingDebugWindow(void);

	void setTraining(MisNewTraining * pTraining);

public:
	void keyPressEvent(QKeyEvent *e){__super::keyPressEvent(e);}
	void keyReleaseEvent(QKeyEvent*e){__super::keyPressEvent(e);}
	void hideAllSubWindow();

protected:
	bool eventFilter(QObject *, QEvent *);
	void hideEvent(QHideEvent *);

private slots:
	void onSendBtn();
	void onHideBtn();
	void onClearListViewBtn();
	/** 导出调试信息 */
	void onExportMessageBtn();

	/**
		将此调试信息添加到调试信息显示列表中
	*/
	void onAddOneDebugInfo(const std::string& debugInfo);

	void onClickedSubToolBtn();
	
	void onClickedEditCamera();

private:
	/**
		此函数负责创建子工具窗口，如果要添加你自定义的工具类窗口，请在此函数中写相应代码！！！
	*/
	void createSubToolWindow();


private:
	Ui::TrainingDebugWindow m_ui;
	MisNewTraining * m_pTraining;
	RbVirtualKeyboard * m_pVirtualKeyboard;

	RbPointToolWidget * m_pointInputEditWidget;
	RbHotchpotchEditorWidget * m_hotchpotchEditorWidget;
	RbVeinConnEditorWidget *m_veinConnEditWidget;
	RbVeinConnEditorV2Widget *m_veinConnEditorV2Widget;
	RbAdjustShaderParametersWidget *m_adjustShaderParamWidget;

	int m_nextRow;
	int m_nextCol;
	const int m_maxCol;

	/// 有效的调试信息记录数
	int m_numRecord;

	/// 调试信息显示前缀
	const QString m_debugMessageShowPrefix;


};
